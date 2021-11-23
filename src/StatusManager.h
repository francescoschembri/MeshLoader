#pragma once

#include "Camera.h"
#include "Animator.h"
#include "TextureManager.h"
#include "Utility.h"
#include "Shader.h"
#include "Change.h"

#include <bitset>
#include <optional>
#include <tuple>
#include <utility>
#include <algorithm>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


class StatusManager
{
public:
	glm::vec2 mouseLastPos;
	Camera camera;
	Animator animator;
	TextureManager texMan;
	std::optional<Model> animatedModel, bakedModel;
	float lastFrame;
	float deltaTime;
	bool pause;
	bool wireframeEnabled;
	bool isModelBaked;
	bool rotating;
	bool changingMesh;
	std::bitset<7> keys;
	float width = 800.0f;
	float height = 800.0f;
	std::vector<Vertex> selectedVertices;
	//std::vector<Vertex*> selectedVerticesPointers;
	glm::mat4 projection = glm::mat4(1.0f);
	PickingInfo info;
	//std::vector<std::pair<int, int>> selectedVerticesIndices;
	std::vector<Change> changes;
	Change currentChange;
	int changeIndex = -1;
	glm::vec3 startChangingPos = glm::vec3(0.0f, 0.0f, 0.0f);
	float rayLenghtOnChangeStart = -1.0f;
	GLuint HVBO, HVAO;
	GLuint SVBO, SVAO;
	Shader modelShader;
	Shader wireframeShader;
	Shader mouseShader;
	Shader hoverShader;
	Shader selectedShader;

	StatusManager(float screenWidth = 800.0f, float screenHeight = 800.0f);

	void AddAnimation(const char* path);
	void Pause();
	void SetPivot();
	void Update();
	void UpdateDeltaTime();
	void BakeModel();
	void UnbakeModel();
	void SwitchAnimation();
	void LoadModel(std::string& path);
	void SelectHoveredVertex();
	void StartChange();
	void EndChange();
	void TweakSelectedVertices();
	PickingInfo FacePicking();
	glm::mat4 GetModelViewMatrix();
	void Undo();
	void Redo();

	// rendering functions
	void Render();
	void DrawWireframe();
	void DrawModel();
	void DrawSelectedVertices();
	void DrawHoveredFace(PickingInfo info);
	void DrawHoveredLine(PickingInfo info);
	void DrawHoveredPoint(PickingInfo info);
};