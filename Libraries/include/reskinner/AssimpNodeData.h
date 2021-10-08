#pragma once

#include <glm/glm.hpp>

#include<vector>
#include<string>

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};