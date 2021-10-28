#pragma once

#include <glm/glm.hpp>

#include <optional>
#include <reskinner/Mesh.h>

//struct PickingInfo {
//	std::optional<Vertex> closestVertex;
//	std::optional<Mesh> meshHitted;
//	std::optional<Face> faceHitted;
//	std::optional<IntersectionInfo> info;
//};

struct PickingInfo {
	std::optional<glm::vec3> hitPoint;
	std::optional<Face> face;
	int meshIndex = -1;
};

struct IntersectionInfo {
	std::optional<glm::vec3> hitPoint;
	float distance = -1.0f;
};

float magnitude(glm::vec3 v);
float raySphereIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, float radius);
IntersectionInfo rayPlaneIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, Vertex& v1, Vertex& v2, Vertex& v3);