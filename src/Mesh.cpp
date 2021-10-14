#include <reskinner/Mesh.h>

// constructor
Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<Face>&& faces, std::vector<Texture>&& textures) : loaded(true), vertices(std::move(vertices)), faces(std::move(faces)), textures(std::move(textures))
{
	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	setupMesh();
}

void Mesh::Bake(std::vector<glm::mat4>& matrices)
{
	// Modify the vertex data
	for (Vertex& v : vertices) {
		glm::vec4 totalPosition = glm::vec4(0.0f);
		glm::vec4 totalNormal = glm::vec4(0.0f);
		glm::vec4 totalTangent = glm::vec4(0.0f);
		glm::vec4 totalBitangent = glm::vec4(0.0f);
		for (int i = 0; i < v.BoneData.NumBones; i++)
		{
			if (v.BoneData.BoneIDs[i] == -1)
				continue;
			if (v.BoneData.BoneIDs[i] >= MAX_BONE_INFLUENCE)
			{
				totalPosition = glm::vec4(v.Position, 1.0f);
				totalNormal = glm::vec4(v.Normal, 1.0f);
				totalTangent = glm::vec4(v.Tangent, 1.0f);
				totalBitangent = glm::vec4(v.Bitangent, 1.0f);
				break;
			}
			glm::vec4 localPosition = matrices[v.BoneData.BoneIDs[i]] * glm::vec4(v.Position, 1.0f);
			glm::vec4 localNormal = matrices[v.BoneData.BoneIDs[i]] * glm::vec4(v.Normal, 1.0f);
			glm::vec4 localTangent = matrices[v.BoneData.BoneIDs[i]] * glm::vec4(v.Tangent, 1.0f);
			glm::vec4 localBitangent = matrices[v.BoneData.BoneIDs[i]] * glm::vec4(v.Bitangent, 1.0f);
			totalPosition += localPosition * v.BoneData.Weights[i];
			totalNormal += localNormal * v.BoneData.Weights[i];
			totalTangent += localTangent * v.BoneData.Weights[i];
			totalBitangent += localBitangent * v.BoneData.Weights[i];
		}
		v.Position = totalPosition;
		v.Normal = totalNormal;
		v.Tangent = totalTangent;
		v.Bitangent = totalBitangent;
	}
}

// render the mesh
void Mesh::Draw(Shader& shader, bool faces, bool lines)
{
	// bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); // transfer unsigned int to stream
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to stream
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // transfer unsigned int to stream

		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	//draw lines
	if (lines) {
		shader.use();
		shader.wireframeMode(true);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, this->faces.size() * sizeof(Face), GL_UNSIGNED_INT, 0);
	}

	// draw mesh aka faces
	if (faces || !lines) { //we want to draw at least something
		shader.use();
		shader.wireframeMode(false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, this->faces.size() * sizeof(Face), GL_UNSIGNED_INT, 0);
	}

	// always good practice to set everything back to defaults once configured.
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::Reload()
{
	if (loaded) {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
	setupMesh();
}

// initializes all the buffer objects/arrays
void Mesh::setupMesh()
{
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

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
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
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

