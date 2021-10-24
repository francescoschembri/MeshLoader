#include <reskinner/Model.h>

#include <algorithm>

// constructor, expects a filepath to a 3D model.
Model::Model(std::string& path, TextureManager& texManager, bool gamma) : gammaCorrection(gamma), texMan(texManager)
{
	loadModel(path);
	//JoinVertices();
}

void Model::JoinVertices() {
	std::vector<Vertex> mergedVertices;
	for (Mesh& m : meshes) {
		std::vector<bool> skip(m.vertices.size(), false);
		for (int i = 0; i < m.vertices.size(); i++)
		{
			std::cout << "Vertex: " << i << "\n";
			if (skip[i]) continue;
			for (int j = i+1; j < m.vertices.size(); j++)
			{
				if (skip[j]) continue;
				skip[j] = m.vertices[i].Position == m.vertices[j].Position 
					&& m.vertices[i].TexCoords == m.vertices[j].TexCoords;
				if (skip[j]) {
					std::cout << "Vertex 1: " << i << ", Vertex 2: "<< j<<"\n";
					for (Face& f : m.faces) {
						for (int k = 0; k < 3; k++)
						{
							if (f.indices[k] == j) f.indices[k] = i;
						}
					}
				}
			}
			mergedVertices.push_back(m.vertices[i]);
		}
		m.vertices = mergedVertices;
	}
}

Model Model::Bake(std::vector<glm::mat4>& matrices)
{
	Model m(*this);
	for (Mesh& mesh : m.meshes)
		mesh.Bake(matrices);
	return m;
}

// draws the model, and thus all its meshes
void Model::Draw(Shader& shader, bool faces, bool lines)
{
	for (unsigned int i = 0; i < meshes.size(); i++) {
		// Bind textures for the mesh
		texMan.BindTextures(meshes[i].texIndices, shader);
		// Draw the mesh
		meshes[i].Draw(shader, faces, lines);
	}
}

const BoneInfo& Model::AddBoneInfo(std::string&& name, glm::mat4 offset)
{
	if (m_BoneInfoMap.find(name) == m_BoneInfoMap.end())
	{
		BoneInfo info;
		info.id = m_BoneCounter++;
		info.offset = offset;
		m_BoneInfoMap[std::move(name)] = info;
		return info;
	}
	else {
		return m_BoneInfoMap[name];
	}

}

std::map<std::string, BoneInfo> Model::GetBoneInfoMap() { return m_BoneInfoMap; }

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::loadModel(std::string& path)
{
	// read file via ASSIMP
	Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS);
	importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace | 
		aiProcess_GenSmoothNormals |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_RemoveComponent |
		aiProcess_SplitLargeMeshes |
		aiProcess_GenUVCoords |
		aiProcess_SortByPType |
		aiProcess_FindDegenerates |
		aiProcess_FindInstances |
		aiProcess_ValidateDataStructure |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph |
		aiProcess_FixInfacingNormals |
		aiProcess_JoinIdenticalVertices
		);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	// retrieve the directory path of the filepath
	std::replace(path.begin(), path.end(), '\\', '/');
	directory = path.substr(0, path.find_last_of('/'));
	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}

}

void Model::SetVertexBoneDataToDefault(Vertex& vertex)
{
	vertex.BoneData.NumBones = 0;
	vertex.BoneData.BoneIDs[0] = -1;
	vertex.BoneData.Weights[0] = 0.0f;	
}


Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<int> texIndices;

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	std::vector<int> diffuseMaps = texMan.loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", directory);
	texIndices.insert(texIndices.end(), diffuseMaps.begin(), diffuseMaps.end());
	std::vector<int> specularMaps = texMan.loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", directory);
	texIndices.insert(texIndices.end(), specularMaps.begin(), specularMaps.end());
	std::vector<int> normalMaps = texMan.loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", directory);
	texIndices.insert(texIndices.end(), normalMaps.begin(), normalMaps.end());
	std::vector<int> ambientMaps = texMan.loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient", directory);
	texIndices.insert(texIndices.end(), ambientMaps.begin(), ambientMaps.end());

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		SetVertexBoneDataToDefault(vertex);
		vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
		vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}

		vertices.push_back(vertex);
	}
	std::vector<Face> faces;
	faces.reserve(mesh->mNumFaces);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		Face f{};
		aiFace face = mesh->mFaces[i];
		if (face.mNumIndices > 3) {
			std::cout << face.mNumIndices << "\n";
		}
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			f.indices[j] = face.mIndices[j];
		faces.push_back(f);
	}

	ExtractBoneWeightForVertices(vertices, mesh, scene);

	return Mesh(std::move(vertices), std::move(faces), std::move(texIndices));
}

void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
{
	if (vertex.BoneData.NumBones >= MAX_BONE_INFLUENCE) return;
	vertex.BoneData.Weights[vertex.BoneData.NumBones] = weight;
	vertex.BoneData.BoneIDs[vertex.BoneData.NumBones++] = boneID;
}


void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
	auto& boneInfoMap = m_BoneInfoMap;
	int& boneCount = m_BoneCounter;

	for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = boneCount;
			newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
			boneInfoMap[boneName] = newBoneInfo;
			boneID = boneCount;
			boneCount++;
		}
		else
		{
			boneID = boneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}