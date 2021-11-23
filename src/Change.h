#pragma once

#include "Vertex.h"

#include <vector>


class Change {
public:
	glm::vec3 offset;

	Change(std::vector<Vertex>& changedVertices, glm::vec3 offset, bool apply = true);
	void Apply();
	void Undo();
	void Modify(glm::vec3 newoffset);
private:
	std::vector<Vertex> changedVertices;
};