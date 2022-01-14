#include "Mesh.h"

// constructor
Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<Face>&& faces, std::vector<int>&& texIndices)
	:
	vertices(std::move(vertices)),
	faces(std::move(faces)),
	texIndices(std::move(texIndices))
{
	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	PropagateVerticesWeights();
	SendMeshToGPU();
}

// copy constructor
Mesh::Mesh(const Mesh& m) :
	vertices(m.vertices),
	faces(m.faces),
	texIndices(m.texIndices),
	graph(m.graph)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	SendMeshToGPU();
}

void Mesh::Bake(std::vector<glm::mat4>& matrices, std::vector<Vertex>& animatedVertices)
{
	// Modify the vertex data
	vertices.clear();
	for (int i = 0; i < animatedVertices.size(); i++) {
		Vertex& v = animatedVertices[i];
		glm::mat4 cumulativeMatrix = glm::mat4(0.0f);
		for (int i = 0; i < v.BoneData.NumBones; i++)
		{
			cumulativeMatrix += (v.BoneData.Weights[i] * matrices[v.BoneData.BoneIDs[i]]);
		}
		Vertex ver{};
		ver.originalVertex = &animatedVertices[i];
		ver.associatedWeightMatrix = cumulativeMatrix;
		ver.Position = glm::vec3(cumulativeMatrix * glm::vec4(v.Position, 1.0f));
		ver.Normal = glm::normalize(glm::vec3(cumulativeMatrix * glm::vec4(v.Normal, 0.0f)));
		ver.Tangent = glm::normalize(glm::vec3(cumulativeMatrix * glm::vec4(v.Tangent, 0.0f)));
		ver.Bitangent = glm::normalize(glm::vec3(cumulativeMatrix * glm::vec4(v.Bitangent, 0.0f)));
		ver.TexCoords = v.TexCoords;
		ver.BoneData.NumBones = 0;
		vertices.push_back(ver);
	}
}

// render the mesh
void Mesh::Draw()
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, faces.size() * sizeof(Face), GL_UNSIGNED_INT, 0);

	// always good practice to set everything back to defaults once configured.
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}


void Mesh::PropagateVerticesWeights()
{
	BuildGraph();
	// initialize the temp weights array [DENSE]
	std::vector<std::vector<double>> weights = std::vector<std::vector<double>>(vertices.size(), std::vector<double>(MAX_NUM_BONE, 0.0));
	for (int i = 0; i < vertices.size(); i++) {
		Vertex& v = vertices[i];
		for (int j = 0; j < v.BoneData.NumBones; j++) {
			weights[i][vertices[i].BoneData.BoneIDs[j]] = abs(vertices[i].BoneData.Weights[j]);
			//NOTE: we use abs because we are taking in consideration even meshes already modified with this tool
			//      that could have some negative weights
		}
	}
	float diag = GetDiagonalLenOfBoundingBox();
	//for each bone of each vertex propagate the associated weight
	bool changed = true;
	while (changed) {
		changed = false;
		for (int i = 0; i < vertices.size(); i++) {
			Vertex& v = vertices[i];
			for (int j = 0; j < MAX_NUM_BONE; j++) {
				if (weights[i][j] > -DBL_EPSILON && weights[i][j] < DBL_EPSILON) continue;
				for (auto ver: graph[i]) {
					// calculate the propagated weight
					double dist = glm::length(v.Position - vertices[ver].Position) / diag;
					double propagatedWeight = weights[i][j] / pow(1.1, dist);
					// check if the vertex exctracted from the frontier is already influenced by the bone
					if (propagatedWeight > weights[ver][j])
					{
						weights[ver][j] = propagatedWeight;
						changed = true;
					}
				}
			}
		}
	}


	// add new bones to the original ones
	for (int i = 0; i < vertices.size(); i++) {
		Vertex& v = vertices[i];
		for (int j = 0; j < v.BoneData.NumBones; j++) {
			weights[i][v.BoneData.BoneIDs[j]] = -1.0f;
		}
		while (v.BoneData.NumBones < MAX_BONE_INFLUENCE) {
			int boneID = std::distance(weights[i].begin(), std::max_element(weights[i].begin(), weights[i].end()));
			if (weights[i][boneID] > -DBL_EPSILON && weights[i][boneID] < DBL_EPSILON) break;
			weights[i][boneID] = -1.0f;
			v.BoneData.BoneIDs[v.BoneData.NumBones] = boneID;
			v.BoneData.Weights[v.BoneData.NumBones++] = 0.0f;
		}
	}
}

void Mesh::BuildGraph()
{
	graph = std::vector<std::set<int>>(vertices.size(), std::set<int>());
	for (Face& f : faces) {
		//v1
		graph[f.indices[0]].insert(f.indices[1]);
		graph[f.indices[0]].insert(f.indices[2]);
		//v2
		graph[f.indices[1]].insert(f.indices[0]);
		graph[f.indices[1]].insert(f.indices[2]);
		//v3
		graph[f.indices[2]].insert(f.indices[0]);
		graph[f.indices[2]].insert(f.indices[1]);
	}
}

// initializes all the buffer objects/arrays
void Mesh::SendMeshToGPU()
{
	glBindVertexArray(VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Face), &faces[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	// ids
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneData.BoneIDs[0]));
	// weights
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneData.Weights[0]));
	// num bones
	glEnableVertexAttribArray(7);
	glVertexAttribIPointer(7, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneData.NumBones));
	glBindVertexArray(0);
}

float Mesh::GetDiagonalLenOfBoundingBox()
{
	float minX = vertices[0].Position.x;
	float maxX = vertices[0].Position.x;
	float minY = vertices[0].Position.y;
	float maxY = vertices[0].Position.y;
	float minZ = vertices[0].Position.z;
	float maxZ = vertices[0].Position.z;
	for (Vertex& v : vertices) {
		if (v.Position.x < minX)
			minX = v.Position.x;
		else if (v.Position.x > maxX)
			maxX = v.Position.x;

		if (v.Position.y < minY)
			minY = v.Position.y;
		else if (v.Position.y > maxY)
			maxY = v.Position.y;

		if (v.Position.z < minZ)
			minZ = v.Position.z;
		else if (v.Position.z > maxZ)
			maxZ = v.Position.z;
	}
	float h = maxZ - minZ;
	float bl1 = maxX - minX;
	float bl2 = maxY - minY;
	return sqrt(h * h + bl1 * bl1 + bl2 * bl2);
}

