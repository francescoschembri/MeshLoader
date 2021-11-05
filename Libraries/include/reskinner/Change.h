#pragma once

#include <vector>
#include <set>
#include <iostream>

#include <reskinner/Model.h>

class Change {
public:
	glm::vec3 offset;

	Change(Model& m, std::vector<std::pair<int, int>>& changedVertices, glm::vec3 offset, bool apply = true);
	void Apply();
	void Undo();
	void Modify(glm::vec3 addoffest);
private:
	Model m;
	std::vector<std::pair<int, int>> changedVertices;
};