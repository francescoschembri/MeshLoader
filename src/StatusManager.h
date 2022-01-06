#pragma once

#include "Camera.h"
#include "Animator.h"
#include "TextureManager.h"
#include "Utility.h"
#include "Shader.h"
#include "Change.h"

#include <optional>
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
	//status of the render
	bool pause;
	bool wireframeEnabled;
	bool isModelBaked;
	//info about the window
	float width = 800.0f;
	float height = 800.0f;
	glm::mat4 projection = glm::mat4(1.0f);
	//additional info
	std::vector<Vertex> selectedVertices;
	std::vector<Vertex*> selectedVerticesPointers;
	PickingInfo info;
	//variables for tweaking
	Change currentChange;
	std::vector<Change> changes;
	int changeIndex = -1;
	glm::vec3 startChangingPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 hotPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	float rayLenghtOnChangeStart = -1.0f;
	//buffers to render hovered and selected stuffs
	GLuint HVBO, HVAO;
	GLuint SVBO, SVAO;
	//shaders to render the model and the gizmos
	Shader modelShader;
	Shader wireframeShader;
	Shader mouseShader;
	Shader hoverShader;
	Shader selectedShader;
	Shader numBonesShader;
	Shader currentBoneShader;
	int currentBoneID = -1;

	StatusManager(float screenWidth = 800.0f, float screenHeight = 800.0f);

	//loading functions
	void AddAnimation(const char* path);
	void LoadModel(std::string& path);

	//animation management
	void Pause();
	void BakeModel();
	void UnbakeModel();
	void SwitchAnimation();
	void Update();
	void UpdateDeltaTime();

	//utilities
	PickingInfo Picking();
	void SetPivot();
	bool SelectHoveredVertex();

	//tweaking
	void StartChange();
	void EndChange();
	void TweakSelectedVertices();
	void Undo();
	void Redo();

	//visual output
	void IncreaseCurrentBoneID();
	void DecreaseCurrentBoneID();
	void Render();
	

private:
	// rendering functions
	void DrawWireframe();
	void DrawModel();
	void DrawSelectedVertices();
	void DrawHoveredFace();
	void DrawHoveredLine();
	void DrawHoveredPoint();
	void DrawHotPoint();

	//utilities
	void UpdateSelectedVertices();
};