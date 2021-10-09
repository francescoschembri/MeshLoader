#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <reskinner/Shader.h>
#include <reskinner/Face.h>
#include <reskinner/Vertex.h>
#include <reskinner/Texture.h>

#include <string>
#include <vector>

class Mesh {
public:
	// mesh Data
	std::vector<Vertex>       vertices;
	std::vector<Face> faces;
	std::vector<Texture>      textures;
	unsigned int VAO;

	// constructor
	Mesh(std::vector<Vertex>&& vertices, std::vector<Face>&& indices, std::vector<Texture>&& textures);
	// bake the mesh
	void Bake(std::vector<glm::mat4>& matrices);
	// render the mesh
	void Draw(Shader& shader, bool faces = true, bool lines = false);

private:
	// render data 
	unsigned int VBO, EBO;

	// initializes all the buffer objects/arrays
	void setupMesh();
};