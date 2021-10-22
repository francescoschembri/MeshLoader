#pragma once

#include <reskinner/Texture.h>
#include <reskinner/Shader.h>
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
	// bind the corrisponding textures to the given shader
	void BindTextures(std::vector<int>& texIndices, Shader& shader);
	// change the setting of stbi. Default flip = true;
	void FlipTextures(bool flip);
private:
	// loads the texture from a file and return the id of the texture; 
	// NOTE: id texture =/= index of vector
	unsigned int TextureFromFile(const char* path, const std::string& directory);
};