#pragma once

#include <bitset>
#include <optional>
#include <utility>

#include <glm/glm.hpp>
#include <reskinner/Camera.h>
#include <reskinner/Animator.h>
#include <reskinner/TextureManager.h>
#include <reskinner/Brush.h>

#include <GLFW/glfw3.h>

#include <filesystem>

namespace filesystem = std::filesystem;

// keys
constexpr int WIREFRAME_KEY = GLFW_KEY_W;
constexpr int HIDDEN_LINE_KEY = GLFW_KEY_H;
constexpr int ROTATE_KEY = GLFW_MOUSE_BUTTON_RIGHT; //
constexpr int RESET_CAMERA_KEY = GLFW_KEY_R;
constexpr int CAMERA_UP_KEY = GLFW_KEY_UP;
constexpr int CAMERA_DOWN_KEY = GLFW_KEY_DOWN;
constexpr int CAMERA_LEFT_KEY = GLFW_KEY_LEFT;
constexpr int CAMERA_RIGHT_KEY = GLFW_KEY_RIGHT;
constexpr int BAKE_MODEL_KEY = GLFW_KEY_B;
constexpr int PAUSE_KEY = GLFW_KEY_P; 
constexpr int CHANGE_MESH_KEY = GLFW_MOUSE_BUTTON_LEFT;
constexpr int SWITCH_ANIMATION_KEY = GLFW_KEY_S;
// keys status (indices in the bitset)
constexpr int WIREFRAME_KEY_PRESSED = 0;
constexpr int HIDDEN_LINE_KEY_PRESSED = 1;
constexpr int PAUSE_KEY_PRESSED = 2;
constexpr int SWITCH_ANIMATION_KEY_PRESSED = 3;
// status (indices in the bitset)
constexpr int WIREFRAME = 4;
constexpr int HIDDEN_LINE = 5;
constexpr int ROTATE = 6;
constexpr int BAKED_MODEL = 7;

class StatusManager
{
public:
	glm::vec2 mouseLastPos;
	Camera camera;
	Animator animator;
	std::optional<Model> animatedModel, bakedModel;
	TextureManager texMan;
	float lastFrame;
	float deltaTime;
	bool pause;
	bool wireframe;
	bool hiddenLine;
	bool pennello1 = false;
	bool sculptingMode;
	int lastVertexPicked = -1;
	int lastMeshPicked = -1;
	std::bitset<8> status;
	std::optional<Brush*> activeBrush;
	float aspect_ratio = 1.0f;
	float width = 800.0f;
	float height = 800.0f;

	StatusManager(float screenWidth = 800.0f, float screenHeight = 800.0f);

	bool IsRotationLocked() const;
	bool IsPaused() const;
	bool IsBaked() const;
	bool DrawLines() const;
	bool DrawFaces() const;
	void AddAnimation(const char* path);
	void Update(GLFWwindow* window);
	void UpdateDeltaTime();
	void ProcessInput(GLFWwindow* window);
	//Vertex* Picking(bool reload = true);
	std::pair<std::optional<Face>, int> FacePicking(bool reload = true);
	void BakeModel();
	void SwitchAnimation();
	void ChangeMesh();
	void LoadModel(std::string& path);
private:

	void InitStatus();
};