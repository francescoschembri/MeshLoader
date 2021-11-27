#pragma once

#include "VertexBoneData.h"

#include <glm/glm.hpp>

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
	// data usefull to refer the original vertex when the vertex is one of the vertices of the baked model
	Vertex* originalVertex = nullptr;
	glm::mat4 associatedWeightMatrix = glm::mat4(0.0f);

	friend bool operator==(const Vertex& v1, const Vertex& v2)
	{
		return v1.Position == v2.Position && v1.Normal == v2.Normal && v1.Bitangent == v2.Bitangent
			&& v1.TexCoords == v2.TexCoords && v1.Tangent == v2.Tangent && v1.BoneData == v2.BoneData;
	}
};