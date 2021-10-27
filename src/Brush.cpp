#include <reskinner/Brush.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


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