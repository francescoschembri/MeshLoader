#include <reskinner/Brush.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <cmath>

#include <reskinner/Utility.h>


Brush::Brush(std::string& name, float radius, float smoothness, float impact, bool reverseNormal)
	:
	name(name),
	radius(radius),
	smoothness(smoothness),
	impact(impact),
	reverseNormal(reverseNormal)
{}

Brush::Brush(const char* name, float radius, float smoothness, float impact, bool reverseNormal)
	:
	name(std::string(name)),
	radius(radius),
	smoothness(smoothness),
	impact(impact),
	reverseNormal(reverseNormal)
{}

void Brush::ModifyMesh(Mesh& mesh, std::vector<int>& verIndices, std::map<int,float>& distancesFromCenter)
{
	for (int i : verIndices) {
		Vertex& v = mesh.vertices[i];
		float calculateSmoothnessEffect = smoothness + (1.0f - distancesFromCenter[i] / radius)*(1.0f-smoothness);
		float effectiveImpact = impact * (reverseNormal ? -1.0f : 1.0f) *calculateSmoothnessEffect;
		//v.Position += v.Normal * effectiveImpact;
		v.Selected = 1;
	}
	if(verIndices.size())
		mesh.Reload();
}

