#pragma once

#include <glad/glad.h>
#include <reskinner/VBO.h>

class VAO
{
public:
	// ID reference for the Vertex Array Object
	unsigned int ID;
	// Constructor that generates a VAO ID
	VAO();

	// Links a VBO Attribute such as a position or color to the VAO
	void LinkAttribute(VBO& VBO, unsigned int layout, unsigned int numComponents, GLenum type, int stride, void* offset);
	// Binds the VAO
	void Bind();
	// Unbinds the VAO
	void Unbind();
	// Deletes the VAO
	void Delete();
};