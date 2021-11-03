#pragma once

#include <glm/glm.hpp>

#include <optional>
#include <reskinner/Mesh.h>
#include <reskinner/Camera.h>

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

struct Line {
	int v1 = -1;
	int v2 = -1;
};

float magnitude(glm::vec3 v);
float raySphereIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, float radius);
IntersectionInfo rayPlaneIntersection(const glm::vec3 rayOrigin, const glm::vec3 rayDir, const Vertex& v1, const Vertex& v2, const Vertex& v3);
int getClosestVertexIndex(const glm::vec3 point, const Mesh& m, int v1, int v2, int v3);
int getClosestVertexIndex(const glm::vec3 point, const Mesh& m, Face& f);
Line getClosestLineIndex(const glm::vec3 point, const Mesh& m, int v1, int v2, int v3);
float getPointLineDist(const glm::vec3 l1, const glm::vec3 l2, const glm::vec3 point);
//void tweaking(Vertex& v, Camera& cam, float xoffset, float yoffset);