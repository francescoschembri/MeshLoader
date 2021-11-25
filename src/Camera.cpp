#include  "Camera.h"

// constructor with vectors
Camera::Camera(glm::vec3 position)
	:
	movementSpeed(MOVEMENT_SPEED),
	rotationSpeed(ROTATION_SPEED),
	zoomSpeed(ZOOM_SPEED),
	sPosition(position),
	position(position),
	yaw(YAW),
	pitch(PITCH),
	pivot(glm::vec3(0.0f, 0.0f, 0.0f))
{
	UpdateCameraDirs();
}

void Camera::UpdateViewMatrix()
{
	viewMatrix = glm::lookAt(position, position + front, up);
}

void Camera::UpdateCameraDirs()
{
	// needed for camera position update
	float fDist = glm::dot(position - pivot, front);
	float uDist = glm::dot(position - pivot, up);
	float rDist = glm::dot(position - pivot, right);

	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(f);
	// also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
	// update camera position
	position = pivot + front * fDist + up * uDist + right * rDist;
	UpdateViewMatrix();
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	if (direction == UP)
		position += up * velocity;
	if (direction == DOWN)
		position -= up * velocity;
	if (direction == LEFT)
		position -= right * velocity;
	if (direction == RIGHT)
		position += right * velocity;

	UpdateViewMatrix();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yoffset)
{
	position += front * yoffset * zoomSpeed;
	UpdateViewMatrix();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset)
{
	pitch += yoffset * rotationSpeed;
	yaw += xoffset * rotationSpeed;

	pitch = std::clamp(pitch, -89.9f, 89.9f);

	UpdateCameraDirs();
}

void Camera::Reset()
{
	movementSpeed = MOVEMENT_SPEED;
	rotationSpeed = ROTATION_SPEED;
	zoomSpeed = ZOOM_SPEED;
	yaw = YAW;
	pitch = PITCH;
	position = sPosition;
	pivot = glm::vec3(0.0f, 0.0f, 0.0f);
	UpdateCameraDirs();
}
