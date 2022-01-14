#include  "Animator.h"


Animator::Animator() : m_CurrentTime(0.0f), currentAnimationIndex(0), m_FinalBoneMatrices(100, glm::mat4(1.0f))
{
	animations.reserve(5);
}

void Animator::UpdateAnimation(float dt)
{
	Animation currentAnimation = animations[currentAnimationIndex];
	if (&currentAnimation)
	{
		m_CurrentTime += currentAnimation.GetTicksPerSecond() * dt * currentAnimation.speed;
		m_CurrentTime = std::clamp(fmod(m_CurrentTime,currentAnimation.endAt), currentAnimation.startFrom, currentAnimation.endAt);
		CalculateBoneTransform(&currentAnimation.GetRootNode(), glm::mat4(1.0f));
	}
}

void Animator::PlayAnimation(Animation* animation)
{
	if (animation) {
		currentAnimationIndex = animations.size();
		AddAnimation(*animation);
		m_CurrentTime = 0.0f;
	}
}

void Animator::PlayAnimationIndex(int index)
{
	currentAnimationIndex = (index + animations.size()) % animations.size();
	m_CurrentTime = 0.0f;
}

void Animator::PlayNextAnimation()
{
	PlayAnimationIndex(currentAnimationIndex + 1);
}

void Animator::PlayPrevAnimation()
{
	PlayAnimationIndex(currentAnimationIndex - 1);
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{ // Worth farla iterativa?
	std::string nodeName = node->name;
	Animation currentAnimation = animations[currentAnimationIndex];
	glm::mat4 nodeTransform = currentAnimation.GetNodeTransform(node, m_CurrentTime);

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = currentAnimation.GetBoneInfoMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;
		m_FinalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++)
		CalculateBoneTransform(&node->children[i], globalTransformation);
}

std::vector<glm::mat4>& Animator::GetFinalBoneMatrices()
{
	return m_FinalBoneMatrices;
}

void Animator::AddAnimation(Animation animation)
{
	if (&animation)
	{
		animations.push_back(animation);
	}
}
