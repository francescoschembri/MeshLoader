#include "Utility.h"


float magnitude(glm::vec3 v) {
	return glm::length(v);
}

float raySphereIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, float radius)
{
	glm::vec3 oc = rayOrigin - sphereCenter;
	float a = glm::dot(rayDir, rayDir);
	float b = 2.0 * glm::dot(oc, rayDir);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;
	if (discriminant >= 0) {
		float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
		if (t1 > 0.0f) return t1;
		return (-b + sqrt(discriminant)) / (2.0f * a);
	}
	return -1.0f;
}

IntersectionInfo rayTriangleIntersection(const glm::vec3 rayOrigin, const glm::vec3 rayDir, const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	IntersectionInfo result{};
	glm::vec3 e12 = glm::normalize(v2.Position - v1.Position);
	glm::vec3 e31 = glm::normalize(v1.Position - v3.Position);
	glm::vec3 e23 = glm::normalize(v3.Position - v2.Position);
	glm::vec3 normalPlane = glm::normalize(glm::cross(e12, e23));

	//check if not parallel
	float normalDotDirection = glm::dot(normalPlane, rayDir);
	if (normalDotDirection >= -FLT_EPSILON && normalDotDirection <= FLT_EPSILON) //it means direction and normal of the plane are perpendicular, so the ray is parallel to the plane
		return result;

	// dot((P-P0), normal) = 0, normal<-->[a,b,c]
	// (x-x0)*a + (y-y0)*b + (z-z0)*c = 0 ---> ax + by + cz - dot(P0, normal) = 0, so d=- dot(P0, normal) 
	float d = -glm::dot(normalPlane, v1.Position);

	// the intersection point is the solution of this system
	// P-P0 = t*rayDirection <---> P(t) = P0+t*raydir , P0 is the ray origin
	// a*Px + b*Py + c*Pz + d = 0 <----> dot(normal, P) + d = 0
	// 
	// to find the t:
	// dot(normal, P0+t*raydir) + d = 0 <---> dot(normal,P0) + dot(normal, t*raydir) + d = 0
	// dot(normal, P0) + t*dot(normal,raydir) + d = 0
	// t = -[dot(normal,P0) + d]/dot(normal, raydir)
	float distRayOriginPlane = glm::dot(normalPlane, rayOrigin) + d;

	float distFollowingRayDirFromRayOrigin = -distRayOriginPlane / normalDotDirection;

	glm::vec3 intersectionPoint = rayOrigin + distFollowingRayDirFromRayOrigin * rayDir;
	result.hitPoint.emplace(intersectionPoint);
	// check if the point is in the triangle
	// let's say A and B are two edges of the triangle and B is on the left of A, than dot(cross(A,B),normal)>0
	// while if B in on the right of A, dot(cross(A,B), normal)<0
	// if instead of B we use the intersection point P, than we have to check for each edge if P is on the left of the edge

	bool checkEdge1 = glm::dot(glm::normalize(glm::cross(e12, intersectionPoint - v1.Position)), normalPlane) < 1.0f - FLT_EPSILON;
	if (checkEdge1) return result;

	bool checkEdge2 = glm::dot(glm::normalize(glm::cross(e23, intersectionPoint - v2.Position)), normalPlane) < 1.0f - FLT_EPSILON;
	if (checkEdge2) return result;

	bool checkEdge3 = glm::dot(glm::normalize(glm::cross(e31, intersectionPoint - v3.Position)), normalPlane) < 1.0f - FLT_EPSILON;
	if (checkEdge3) return result;

	result.distance = distFollowingRayDirFromRayOrigin;
	return result;
}

int getClosestVertexIndex(const glm::vec3 point, const Mesh& m, int v1, int v2, int v3)
{
	int res = v1;
	float v1dist = magnitude(m.vertices[v1].Position - point);
	float v2dist = magnitude(m.vertices[v2].Position - point);
	float v3dist = magnitude(m.vertices[v3].Position - point);
	float dist = v1dist;
	if (v2dist < dist) {
		res = v2;
		dist = v2dist;
	}
	if (v3dist < dist) {
		res = v3;
	}
	return res;
}

int getClosestVertexIndex(const glm::vec3 point, const Mesh& m, Face& f)
{
	return getClosestVertexIndex(point, m, f.indices[0], f.indices[1], f.indices[2]);
}

Vertex getClosestVertex(const glm::vec3 point, const Mesh& m, int v1, int v2, int v3)
{
	int res = getClosestVertexIndex(point, m, v1, v2, v3);
	return m.vertices[res];
}

Vertex getClosestVertex(const glm::vec3 point, const Mesh& m, Face& f)
{
	return getClosestVertex(point, m, f.indices[0], f.indices[1], f.indices[2]);
}

Line getClosestLineIndex(const glm::vec3 point, const Mesh& m, Face& f)
{
	return getClosestLineIndex(point, m, f.indices[0], f.indices[1], f.indices[2]);
}

Line getClosestLineIndex(const glm::vec3 point, const Mesh& m, int v1, int v2, int v3)
{
	Line res{};
	res.v1 = v1;
	res.v2 = v2;
	float e1dist = getPointLineDist(m.vertices[v1].Position, m.vertices[v2].Position, point);
	float e2dist = getPointLineDist(m.vertices[v1].Position, m.vertices[v3].Position, point);
	float e3dist = getPointLineDist(m.vertices[v2].Position, m.vertices[v3].Position, point);
	float dist = e1dist;
	if (e2dist < dist)
	{
		res.v2 = v3;
		dist = e2dist;
	}
	if (e3dist < dist)
	{
		res.v1 = v2;
		res.v2 = v3;
	}
	return res;
}

float getPointLineDist(const glm::vec3 l1, const glm::vec3 l2, const glm::vec3 point)
{
	glm::vec3 l12 = l2 - l1;
	glm::vec3 l1p = point - l1;
	float area = magnitude(glm::cross(l12, l1p));
	float base = magnitude(l12);
	return area / base;
}

//void tweaking(Vertex& v, Camera& cam, float xoffset, float yoffset)
//{
//	v.Position += cam.right * xoffset + cam.up * yoffset;
//}
