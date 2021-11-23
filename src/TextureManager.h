#pragma once

#include "Texture.h"
#include "Shader.h"
//#include <glad/glad.h>
#include <stb/stb_image.h>
#include <assimp/scene.h>
#include <vector>

class TextureManager {
public:
	std::vector<Texture> textures;
	TextureManager();
	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required infos are returned as a vector of indices to use the textures of the texture manager
	std::vector<int> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string& typeName, std::string& directory);
	std::vector<int> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* typeName, std::string& directory);
	// loads the texture from a file and return the index in textures of the texture loaded 
	int LoadTextureFromFile(const char* filename, std::string type = "");
	// bind the corrisponding textures to the given shader
	void BindTextures(std::vector<int>& texIndices, const Shader& shader);
	// change the setting of stbi. Default flip = true;
	void FlipTextures(bool flip);
};