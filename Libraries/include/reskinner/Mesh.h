#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <reskinner/Shader.h>
#include <reskinner/Face.h>
#include <reskinner/Vertex.h>
#include <reskinner/TextureManager.h>

#include <string>
#include <vector>
#include <set>
#include <algorithm>

class Mesh {
public:
	// mesh Data
	std::vector<Vertex>       vertices;
	std::vector<Face> faces;
	std::vector<int> texIndices;
	unsigned int VAO;
	// render data 
	unsigned int VBO, EBO;
	bool loaded = false;

	// constructors
	Mesh() = default;
	Mesh(std::vector<Vertex>&& vertices, std::vector<Face>&& indices, std::vector<int>&& texIndices);
	// bake the mesh
	void Bake(std::vector<glm::mat4>& matrices);
	// render the mesh
	void Draw(Shader& shader, bool wireframeEnabled = false);
	// reload opengl data for the mesh
	void Reload();

private:

	// initializes all the buffer objects/arrays
	void setupMesh();
};