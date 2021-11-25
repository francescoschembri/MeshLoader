#pragma once

#include "Utility.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

// Default camera values
constexpr float MOVEMENT_SPEED = 2.5f;
constexpr float ROTATION_SPEED = 0.005f;
constexpr float ZOOM_SPEED = 0.2f;
constexpr float FOV = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// camera Attributes
	glm::vec3 position, sPosition;
	const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::mat4 viewMatrix;
	float yaw;
	float pitch;
	glm::vec3 pivot = glm::vec3(0.0f, 0.0f, 0.0f);
	// camera options
	float movementSpeed;
	float zoomSpeed;
	float rotationSpeed;

	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset);
	// processes mouse movement rotating the camera around the pivot
	void ProcessMouseMovement(float xoffset, float yoffset);

	// resets camera to starting values
	void Reset();
	// update camera versors
	void UpdateCameraDirs();

private:
	// update view matrix
	void UpdateViewMatrix();
};