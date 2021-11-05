#pragma once

#include <bitset>
#include <optional>
#include <tuple>
#include <utility>

#include <glm/glm.hpp>
#include <reskinner/Camera.h>
#include <reskinner/Animator.h>
#include <reskinner/TextureManager.h>
#include <reskinner/Utility.h>
#include <reskinner/Shader.h>
#include <reskinner/Change.h>

#include <GLFW/glfw3.h>

#include <filesystem>

namespace filesystem = std::filesystem;

// keys
constexpr int WIREFRAME_KEY = GLFW_KEY_W;
constexpr int HIDDEN_LINE_KEY = GLFW_KEY_H;
constexpr int ROTATE_KEY = GLFW_MOUSE_BUTTON_RIGHT;
constexpr int RESET_CAMERA_KEY = GLFW_KEY_R;
constexpr int CAMERA_UP_KEY = GLFW_KEY_UP;
constexpr int CAMERA_DOWN_KEY = GLFW_KEY_DOWN;
constexpr int CAMERA_LEFT_KEY = GLFW_KEY_LEFT;
constexpr int CAMERA_RIGHT_KEY = GLFW_KEY_RIGHT;
constexpr int BAKE_MODEL_KEY = GLFW_KEY_B;
constexpr int PAUSE_KEY = GLFW_KEY_P;
constexpr int CHANGE_MESH_KEY = GLFW_MOUSE_BUTTON_LEFT;
constexpr int SELECT_KEY = GLFW_MOUSE_BUTTON_LEFT;
constexpr int SWITCH_ANIMATION_KEY = GLFW_KEY_S;
// keys status (indices in the bitset)
constexpr int WIREFRAME_KEY_PRESSED = 0;
constexpr int PAUSE_KEY_PRESSED = 1;
constexpr int SWITCH_ANIMATION_KEY_PRESSED = 2;
constexpr int SELECT_KEY_PRESSED = 3;
constexpr int ROTATION_KEY_PRESSED = 4;

constexpr float CHANGE_VELOCITY = 0.1f;

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
	std::bitset<5> keys;
	float aspect_ratio = 1.0f;
	float width = 800.0f;
	float height = 800.0f;
	std::vector<Vertex> selectedVertices;
	std::vector<std::pair<int, int>> selectedVerticesIndices;
	std::vector<Change> changes;
	glm::vec2 startChangingPos = glm::vec2(0.0, 0.0);
	GLuint HVBO, HVAO;
	GLuint SVBO, SVAO;
	Shader modelShader;
	Shader wireframeShader;
	Shader mouseShader;
	Shader hoverShader;
	Shader selectedShader;

	StatusManager(float screenWidth = 800.0f, float screenHeight = 800.0f);

	void AddAnimation(const char* path);
	void Update(GLFWwindow* window);
	void UpdateDeltaTime();
	void ProcessInput(GLFWwindow* window);
	void BakeModel();
	void SwitchAnimation();
	void ChangeMesh();
	void LoadModel(std::string& path);
	PickingInfo FacePicking();
	glm::mat4 GetModelViewMatrix();

	// rendering functions
	void Render();
	void DrawWireframe();
	void DrawModel();
	void DrawSelectedVertices();
	void DrawHoveredFace(PickingInfo info);
	void DrawHoveredLine(PickingInfo info);
	void DrawHoveredPoint(PickingInfo info);
};