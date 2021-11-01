#include  <reskinner/Camera.h>

// constructor with vectors
Camera::Camera(glm::vec3 position)
	:
	movementSpeed(MOVEMENT_SPEED),
	zoomSpeed(ZOOM_SPEED),
	sPosition(position),
	position(position)
{}

glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(position, position+front, up);
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
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yoffset)
{
	position += front * yoffset * zoomSpeed;
}

void Camera::Reset()
{
	movementSpeed = MOVEMENT_SPEED;
	zoomSpeed = ZOOM_SPEED;
	position = sPosition;
}
