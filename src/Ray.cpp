#include "Ray.h"

Ray::Ray(glm::vec2& mouseLastPos, float width, float height, glm::mat4& toWorld)
{
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(width, height)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);

	rayStartPos = toWorld * rayStartPos;
	rayEndPos = toWorld * rayEndPos;

	rayStartPos /= rayStartPos.w;
	rayEndPos /= rayEndPos.w;

	direction = glm::normalize(glm::vec3(rayEndPos - rayStartPos));
	origin = glm::vec3(rayStartPos);
}

void Ray::IntersectTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	IntersectTriangle(v1.Position, v2.Position, v3.Position);
}

void Ray::IntersectTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
	info = IntersectionInfo{};
	glm::vec3 e12 = glm::normalize(v2 - v1);
	glm::vec3 e31 = glm::normalize(v1 - v3);
	glm::vec3 e23 = glm::normalize(v3 - v2);
	glm::vec3 normalPlane = glm::normalize(glm::cross(e12, e23));

	//check if not parallel
	float normalDotDirection = glm::dot(normalPlane, direction);
	if (normalDotDirection == 0.0f) //it means direction and normal of the plane are perpendicular, so the ray is parallel to the plane
		return;

	// dot((P-P0), normal) = 0, normal<-->[a,b,c]
	// (x-x0)*a + (y-y0)*b + (z-z0)*c = 0 ---> ax + by + cz - dot(P0, normal) = 0, so d=- dot(P0, normal) 
	float d = -glm::dot(normalPlane, v1);

	// the intersection point is the solution of this sistem
	// P-P0 = t*rayDirection <---> P(t) = P0+t*raydir , P0 is the ray origin
	// a*Px + b*Py + c*Pz + d = 0 <----> dot(normal, P) + d = 0
	// 
	// to find the t:
	// dot(normal, P0+t*raydir) + d = 0 <---> dot(normal,P0) + dot(normal, t*raydir) + d = 0
	// dot(normal, P0) + t*dot(normal,raydir) + d = 0
	// t = -[dot(normal,P0) + d]/dot(normal, raydir)
	float distRayOriginPlane = glm::dot(normalPlane, origin) + d;

	float distFollowingRayDirFromRayOrigin = -distRayOriginPlane / normalDotDirection;

	glm::vec3 intersectionPoint = origin + distFollowingRayDirFromRayOrigin * origin;
	info.hitPoint.emplace(intersectionPoint);
	// check if the point is in the triangle
	// let's say A and B are two edges of the triangle and B is on the left of A, than dot(cross(A,B),normal)>0
	// while if B in on the right of A, dot(cross(A,B), normal)<0
	// if instead of B we use the intersection point P, than we have to check for each edge if P is on the left of the edge

	bool checkEdge1 = glm::dot(glm::normalize(glm::cross(e12, intersectionPoint - v1)), normalPlane) < 0.9;
	if (checkEdge1) return;
	bool checkEdge2 = glm::dot(glm::normalize(glm::cross(e23, intersectionPoint - v2)), normalPlane) < 0.9f;
	if (checkEdge2) return;
	bool checkEdge3 = glm::dot(glm::normalize(glm::cross(e31, intersectionPoint - v3)), normalPlane) < 0.9f;
	if (checkEdge3) return;

	info.distance = distFollowingRayDirFromRayOrigin;
}

void Ray::IntersectPlane(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
	info = IntersectionInfo{};
	glm::vec3 e12 = glm::normalize(v2 - v1);
	glm::vec3 e31 = glm::normalize(v1 - v3);
	glm::vec3 e23 = glm::normalize(v3 - v2);
	glm::vec3 normalPlane = glm::normalize(glm::cross(e12, e23));

	//check if not parallel
	float normalDotDirection = glm::dot(normalPlane, direction);
	if (normalDotDirection == 0.0f) //it means direction and normal of the plane are perpendicular, so the ray is parallel to the plane
		return;

	// dot((P-P0), normal) = 0, normal<-->[a,b,c]
	// (x-x0)*a + (y-y0)*b + (z-z0)*c = 0 ---> ax + by + cz - dot(P0, normal) = 0, so d=- dot(P0, normal) 
	float d = -glm::dot(normalPlane, v1);

	// the intersection point is the solution of this sistem
	// P-P0 = t*rayDirection <---> P(t) = P0+t*raydir , P0 is the ray origin
	// a*Px + b*Py + c*Pz + d = 0 <----> dot(normal, P) + d = 0
	// 
	// to find the t:
	// dot(normal, P0+t*raydir) + d = 0 <---> dot(normal,P0) + dot(normal, t*raydir) + d = 0
	// dot(normal, P0) + t*dot(normal,raydir) + d = 0
	// t = -[dot(normal,P0) + d]/dot(normal, raydir)
	float distRayOriginPlane = glm::dot(normalPlane, origin) + d;

	float distFollowingRayDirFromRayOrigin = -distRayOriginPlane / normalDotDirection;

	glm::vec3 intersectionPoint = origin + distFollowingRayDirFromRayOrigin * origin;
	info.hitPoint.emplace(intersectionPoint);

	info.distance = distFollowingRayDirFromRayOrigin;
}

void Ray::IntersectPlane(Vertex& v1, Vertex& v2, Vertex& v3)
{
	IntersectPlane(v1.Position, v2.Position, v3.Position);
}

