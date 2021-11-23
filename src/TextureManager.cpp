#include "TextureManager.h"


TextureManager::TextureManager() {
	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<int> TextureManager::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string& typeName, std::string& directory)
{
	std::vector<int> texIndices;
	texIndices.reserve(mat->GetTextureCount(type));
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string path = directory + "/" + str.C_Str();
		texIndices.push_back(LoadTextureFromFile(path.c_str(), typeName));
	}
	return texIndices;
}

std::vector<int> TextureManager::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* typeName, std::string& directory)
{
	std::string name = std::string(typeName);
	return loadMaterialTextures(mat, type, name, directory);
}


int TextureManager::LoadTextureFromFile(const char* path, std::string type)
{
	// don't load the same texture twice.
	auto samePath = [&path](Texture& t) {return std::strcmp(t.path.data(), path) == 0; };
	auto iter = std::find_if(textures.begin(), textures.end(), samePath);
	if (iter != textures.end())
		return iter - textures.begin();

	// texture not already loaded -> load it.
	unsigned int textureID{};

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		glGenTextures(1, &textureID);
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << "\n";
		stbi_image_free(data);
	}

	Texture tex{ textureID, path, type };
	textures.emplace_back(tex);
	return textures.size() - 1;
}

void TextureManager::BindTextures(std::vector<int>& texIndices, const Shader& shader)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int ambientNr = 1;
	for (unsigned int i = 0; i < texIndices.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[texIndices[i]].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); // transfer unsigned int to stream
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to stream
		else if (name == "texture_ambient")
			number = std::to_string(ambientNr++); // transfer unsigned int to stream

		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.ID, (name).c_str()), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[texIndices[i]].id);
	}
}

void TextureManager::FlipTextures(bool flip)
{
	// tell stb_image.h if it has to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(flip);
}