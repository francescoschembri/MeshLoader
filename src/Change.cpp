#include "Change.h"

Change::Change(std::vector<Vertex>& changedVertices) 
	:
	changedVertices(changedVertices), 
	offset(glm::vec3(0.0f, 0.0f, 0.0f))
{}

void Change::Apply() {
	for (Vertex& v : changedVertices) {
		v.Position += offset;
		glm::mat4 inverse = glm::inverse(v.associatedWeightMatrix);
		v.originalVertex->Position += glm::vec3((inverse * glm::vec4(offset, 0.0f)));
	}
}

void Change::Undo() {
	for (Vertex& v : changedVertices) {
		glm::mat4 inverse = glm::inverse(v.associatedWeightMatrix);
		v.originalVertex->Position -= glm::vec3((inverse * glm::vec4(offset, 0.0f)));
	}
}

void Change::Modify(glm::vec3 newoffset)
{
	offset = newoffset - offset;
	Apply();
	offset = newoffset;
}
