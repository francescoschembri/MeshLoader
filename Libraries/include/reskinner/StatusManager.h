#pragma once

#include <bitset>

#include <glm/glm.hpp>
#include <reskinner/Camera.h>
#include <reskinner/Animator.h>

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
constexpr int PAUSE = 7;
constexpr int BAKED_MODEL = 8;

class StatusManager
{
public:
	glm::vec2 mouseLastPos;
	Camera camera;
	Animator animator;
	Model model;
	float lastFrame;
	float deltaTime;

	StatusManager(float screenWidth = 800.0f, float screenHeight = 800.0f);

	bool IsRotationLocked() const;
	bool IsPaused() const;
	bool DrawLines() const;
	bool DrawFaces() const;
	void AddAnimation(const char* path);
	void UpdateDeltaTime();
	void ProcessInput(GLFWwindow* window);
private:
	std::bitset<9> status;

	void InitStatus();
	void SwitchAnimation();
};