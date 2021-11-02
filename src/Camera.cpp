#include  <reskinner/Camera.h>

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
	front(glm::vec3(0.0f, 0.0f, -1.0f)),
	up(glm::vec3(0.0f, 1.0f, 0.0f)),
	right(glm::vec3(1.0f, 0.0f, 0.0f)),
	pivot(glm::vec3(0.0f, 0.0f, 0.0f))
{
	UpdateViewMatrix();
}

glm::mat4 Camera::GetViewMatrix() {
	return viewCamera;
}

void Camera::UpdateViewMatrix()
{
	viewCamera = glm::mat4(1.0f);
	viewCamera = glm::translate(viewCamera, -position);
	viewCamera = glm::rotate(viewCamera, glm::radians(pitch), glm::vec3(1.0, 0.0, 0.0));
	viewCamera = glm::rotate(viewCamera, glm::radians(yaw), glm::vec3(0.0, 1.0, 0.0));
	viewCamera = glm::translate(viewCamera, -pivot);
}

void Camera::UpdateCameraDirs()
{
	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(f);
	// also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front, worldUp)); 
	up = glm::normalize(glm::cross(right, front));
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
	UpdateViewMatrix();
}

void Camera::Reset()
{
	movementSpeed = MOVEMENT_SPEED;
	rotationSpeed = ROTATION_SPEED;
	zoomSpeed = ZOOM_SPEED;
	yaw = YAW;
	pitch = PITCH,
	position = sPosition;
	UpdateCameraDirs();
	UpdateViewMatrix();
}
