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
	float m_CurrentTime;
	std::vector<Animation> animations;
	int currentAnimationIndex = 0;

	Animator();
	void UpdateAnimation(float dt);
	void PlayAnimation(Animation* animation);
	void PlayAnimationIndex(int index);
	void PlayNextAnimation();
	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
	std::vector<glm::mat4>& GetFinalBoneMatrices();
	void AddAnimation(Animation animation);

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
};

