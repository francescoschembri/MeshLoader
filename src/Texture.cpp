#include <reskinner/Texture.h>
#include <glad/glad.h>

Texture::~Texture() {
	glDeleteTextures(1, &id);
}