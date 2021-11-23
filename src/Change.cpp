#include "Change.h"

Change::Change(std::vector<Vertex>& changedVertices, glm::vec3 offset, bool apply) 
	:
	changedVertices(changedVertices), 
	offset(offset)
{
	if (apply) Apply();
}

void Change::Apply() {
	for (Vertex v : changedVertices) {
		v.Position += offset;
	}
}

void Change::Undo() {
	for (Vertex v : changedVertices) {
		v.Position -= offset;
	}
}

void Change::Modify(glm::vec3 newoffset)
{
	offset = newoffset - offset;
	Apply();
	offset = newoffset;
}
