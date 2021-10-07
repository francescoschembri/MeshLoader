#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <reskinner/Shader.h>
#include <reskinner/Face.h>

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 100

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {
public:
	// mesh Data
	std::vector<Vertex>       vertices;
	std::vector<Face> faces;
	std::vector<Texture>      textures;
	unsigned int VAO;

	// constructor
	Mesh(std::vector<Vertex>&& vertices, std::vector<Face>&& indices, std::vector<Texture>&& textures);

	// render the mesh
	void Draw(Shader& shader, bool faces = true, bool lines = false);

private:
	// render data 
	unsigned int VBO, EBO;

	// initializes all the buffer objects/arrays
	void setupMesh();
};