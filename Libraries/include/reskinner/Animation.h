#pragma once

#include <vector>
#include <map>

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <reskinner/Bone.h>
#include <reskinner/BoneInfo.h>
#include <reskinner/Model.h>
#include <reskinner/AssimpNodeData.h>

class Animation
{
public:
	Animation(const std::string& animationPath, Model& model);

	Bone* FindBone(const std::string& name);

	float GetTicksPerSecond();
	float GetDuration();
	const AssimpNodeData& GetRootNode();
	const std::map<std::string, BoneInfo>& GetBoneInfoMap();
	glm::mat4 GetNodeTransform(const AssimpNodeData* node, float currentTime);

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model);
	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};

