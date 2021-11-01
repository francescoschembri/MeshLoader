#include <reskinner/Utility.h>

#include <cmath>

float magnitude(glm::vec3 v) {
	return sqrt(glm::dot(v, v));
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

IntersectionInfo rayPlaneIntersection(const glm::vec3 rayOrigin, const glm::vec3 rayDir, const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	IntersectionInfo result{};
	glm::vec3 e12 = glm::normalize(v2.Position - v1.Position);
	glm::vec3 e31 = glm::normalize(v1.Position - v3.Position);
	glm::vec3 e23 = glm::normalize(v3.Position - v2.Position);
	glm::vec3 normalPlane = glm::normalize(glm::cross(e12, e23));

	//check if not parallel
	float normalDotDirection = glm::dot(normalPlane, rayDir);
	if (normalDotDirection == 0.0f) //it means direction and normal of the plane are perpendicular, so the ray is parallel to the plane
		return result;

	// dot((P-P0), normal) = 0, normal<-->[a,b,c]
	// (x-x0)*a + (y-y0)*b + (z-z0)*c = 0 ---> ax + by + cz - dot(P0, normal) = 0, so d=- dot(P0, normal) 
	float d = -glm::dot(normalPlane, v1.Position);

	// the intersection point is the solution of this sistem
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

	bool checkEdge1 = glm::dot(glm::normalize(glm::cross(e12, intersectionPoint - v1.Position)), normalPlane) < 0.9;
	if (checkEdge1) return result;
	bool checkEdge2 = glm::dot(glm::normalize(glm::cross(e23, intersectionPoint - v2.Position)), normalPlane) < 0.9f;
	if (checkEdge2) return result;
	bool checkEdge3 = glm::dot(glm::normalize(glm::cross(e31, intersectionPoint - v3.Position)), normalPlane) < 0.9f;
	if (checkEdge3) return result;

	result.distance = distFollowingRayDirFromRayOrigin;
	return result;
}

int getClosestVertexIndex(const glm::vec3 point, const Mesh& m, int v1, int v2, int v3)
{
	int res = v1;
	float dist = magnitude(point - m.vertices[v1].Position);
	if (magnitude(point - m.vertices[v2].Position) < dist) {
		res = v2;
	}
	if (magnitude(point - m.vertices[v3].Position)) {
		res = v3;
	}
	return res;
}

Line getClosestLineIndex(const glm::vec3 point, const Mesh& m, int v1, int v2, int v3)
{
	Line res{};
	res.v1 = v1;
	res.v2 = v2;
	float dist = getPointLineDist(m.vertices[v1].Position, m.vertices[v2].Position, point);
	if (getPointLineDist(m.vertices[v1].Position, m.vertices[v3].Position, point) < dist)
	{
		res.v2 = v3;
	}
	if (getPointLineDist(m.vertices[v2].Position, m.vertices[v3].Position, point) < dist)
	{
		res.v1 = v3;
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
