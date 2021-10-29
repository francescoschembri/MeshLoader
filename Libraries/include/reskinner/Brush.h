#pragma once

#include <reskinner/Mesh.h>

#include <glm/glm.hpp>
#include <map>

class Brush {
public:
	std::string name;
	float radius;
	float smoothness;
	float impact;
	bool reverseNormal = false;

	Brush(std::string& name, float radius, float smoothness = 0.5f, float impact = 1.0f, bool reverseNormal = false);
	Brush(const char* name, float radius, float smoothness = 0.5f, float impact = 1.0f, bool reverseNormal = false);

	void ModifyMesh(Mesh& mesh, std::vector<int>& verIndices, std::map<int, float>& distancesFromCenter);
};