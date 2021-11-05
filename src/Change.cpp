#include <reskinner/Change.h>

Change::Change(Model& m, std::vector<std::pair<int, int>>& changedVertices, glm::vec3 offset, bool apply) 
	:
	m(m),
	changedVertices(changedVertices), 
	offset(offset)
{
	if (apply) Apply();
}

void Change::Apply() {
	std::set<int> modifiedMesh;
	for (auto p : changedVertices) {
		Vertex& v = m.meshes[p.first].vertices[p.second];
		v.Position += offset;
		modifiedMesh.insert(p.first);
	}
	for (int index : modifiedMesh) {
		m.meshes[index].Reload();
	}
}

void Change::Undo() {
	std::set<int> modifiedMesh;
	for (auto p : changedVertices) {
		Vertex& v = m.meshes[p.first].vertices[p.second];
		v.Position -= offset;
		modifiedMesh.insert(p.first);
	}
	for (int index : modifiedMesh) {
		m.meshes[index].Reload();
	}
}

void Change::Modify(glm::vec3 newoffset)
{
	offset = newoffset - offset;
	Apply();
	offset = newoffset;
}
