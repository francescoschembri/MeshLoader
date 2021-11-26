#include "Mesh.h"

// constructor
Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<Face>&& faces, std::vector<int>&& texIndices)
	:
	loaded(true),
	vertices(std::move(vertices)),
	faces(std::move(faces)),
	texIndices(std::move(texIndices))
{
	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	setupMesh();
}

void Mesh::Bake(std::vector<glm::mat4>& matrices, std::vector<Vertex> animatedVertices)
{
	// Modify the vertex data
	std::vector<Vertex> bakedVertices = std::vector<Vertex>();
	for (Vertex& v : animatedVertices) {
		glm::mat4 cumulativeMatrix = glm::mat4(0.0f);
		for (int i = 0; i < v.BoneData.NumBones; i++)
		{
			cumulativeMatrix += (v.BoneData.Weights[i] * matrices[v.BoneData.BoneIDs[i]]);
		}
		Vertex ver{};
		ver.originalVertex = &v;
		ver.associatedWeightMatrix = cumulativeMatrix;
		ver.Position = glm::vec3(cumulativeMatrix * glm::vec4(v.Position, 1.0f));
		ver.Normal = glm::normalize(glm::vec3(cumulativeMatrix * glm::vec4(v.Normal, 0.0f)));
		ver.Tangent = glm::normalize(glm::vec3(cumulativeMatrix * glm::vec4(v.Tangent, 0.0f)));
		ver.Bitangent = glm::normalize(glm::vec3(cumulativeMatrix * glm::vec4(v.Bitangent, 0.0f)));
		ver.BoneData.NumBones = 0;
		bakedVertices.push_back(ver);
	}
	vertices = bakedVertices;
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

void Mesh::Reload()
{
	setupMesh();
}

// initializes all the buffer objects/arrays
void Mesh::setupMesh()
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

