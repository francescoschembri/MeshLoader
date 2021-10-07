#pragma once

#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

#include <reskinner/Animation.h>
#include <reskinner/Bone.h>

class Animator
{
public:
	Animator(Animation* animation);
	void UpdateAnimation(float dt);
	void PlayAnimation(Animation* pAnimation);
	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
	std::vector<glm::mat4> GetFinalBoneMatrices();

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;

};

