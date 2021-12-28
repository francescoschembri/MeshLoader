#pragma once


#include "Mesh.h"
#include "Shader.h"
#include "assimp_glm_helpers.h"
#include "BoneInfo.h"
#include "Face.h"
#include "TextureManager.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

//#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class Model
{
public:
	// model data 
	std::vector<Mesh> meshes;
	std::string directory;
	TextureManager& texMan;
	bool gammaCorrection;

	// default constructor
	Model() = default;
	// copy constructor
	Model(const Model& m) = default;
	// move constructor
	Model(Model&& m) = default;
	// constructor, expects a filepath to a 3D model.
	Model(std::string& path, TextureManager& texMan, bool gamma = false);
	// bake the model
	Model Bake(std::vector<glm::mat4>& matrices);
	// draws the model, and thus all its meshes
	void Draw(const Shader& shader);
	std::map<std::string, BoneInfo> GetBoneInfoMap();
	const BoneInfo& AddBoneInfo(std::string&& name, glm::mat4 offset);
	void Reload();
private:

	std::map<std::string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;

	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(std::string& path);
	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene);

	void SetVertexBoneDataToDefault(Vertex& vertex);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
	void PropagateWeights();
};