#pragma once

#include "Shader.h"
#include "Face.h"
#include "Vertex.h"
#include "TextureManager.h"

//#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>


class Mesh {
public:
	// mesh Data
	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	std::vector<int> texIndices;
	// render data 
	unsigned int VAO;
	unsigned int VBO, EBO;
	bool enabled = true;

	// constructors
	Mesh() = default;
	Mesh(std::vector<Vertex>&& vertices, std::vector<Face>&& indices, std::vector<int>&& texIndices);
	// copy constructor
	Mesh(const Mesh& m);
	// move constructor
	Mesh(Mesh&& m) = default;


	// bake the mesh
	void Bake(std::vector<glm::mat4>& matrices, std::vector<Vertex>& animatedVertices);
	// render the mesh
	void Draw();
	// send opengl data for the mesh to the gpu
	void SendMeshToGPU();

private:
	std::vector<std::set<int>> graph;
	void BuildGraph();
	float GetDiagonalLenOfBoundingBox();
	// propagate weights of the bones that influence the vertex to the next ones.
	void PropagateVerticesWeights();
};