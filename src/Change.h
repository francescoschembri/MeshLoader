#pragma once

#include "Vertex.h"

#include <vector>


class Change {
public:
	glm::vec3 offset;

	Change(std::vector<Vertex>& changedVertices);
	void Apply();
	void Undo();
	void Modify(glm::vec3 newoffset);
private:
	std::vector<Vertex> changedVertices;
};