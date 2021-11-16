#pragma once

#include <glm/glm.hpp>
#include <reskinner/Utility.h>

class Ray {
public:
	glm::vec3 origin;
	glm::vec3 direction;
	IntersectionInfo info;

	Ray(glm::vec2& mouseLastPos, float width, float height, glm::mat4& toWorld);

	void IntersectTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
	void IntersectTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
	void IntersectPlane(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
	void IntersectPlane(Vertex& v1, Vertex& v2, Vertex& v3);
};