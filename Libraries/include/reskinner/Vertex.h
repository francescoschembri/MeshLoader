#pragma once

#include <glm/glm.hpp>
#include <reskinner/VertexBoneData.h>

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords; 
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
	// bone data
	VertexBoneData BoneData;
	// is the vertex hovered
	int Selected = 0;
};