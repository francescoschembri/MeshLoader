#include <reskinner/StatusManager.h>

StatusManager::StatusManager(float screenWidth, float screenHeight)
	:
	camera(glm::vec3(0.0f, 0.0f, 3.0f)),
	lastFrame(glfwGetTime()),
	deltaTime(0.0f),
	mouseLastPos(glm::vec2(screenWidth / 2, screenHeight / 2))
{
	InitStatus();
}

bool StatusManager::IsRotationLocked() const
{
	return status[ROTATE];
}

bool StatusManager::IsPaused() const
{
	return status[PAUSE];
}

bool StatusManager::DrawLines() const
{
	return status[WIREFRAME];
}

bool StatusManager::DrawFaces() const
{
	return status[HIDDEN_LINE];
}

void StatusManager::AddAnimation(const char* path)
{
	animator.AddAnimation(Animation(filesystem::path(path).string(), model));
}

void StatusManager::UpdateDeltaTime()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void StatusManager::ProcessInput(GLFWwindow* window)
{
	// enable/disable wireframe mode
	if (glfwGetKey(window, WIREFRAME_KEY) == GLFW_PRESS && !status[WIREFRAME_KEY_PRESSED])
		status[WIREFRAME] = !status[WIREFRAME];
	// enable/disable hidden line
	if (glfwGetKey(window, HIDDEN_LINE_KEY) == GLFW_PRESS && !status[HIDDEN_LINE_KEY_PRESSED])
		status[HIDDEN_LINE] = !status[HIDDEN_LINE];
	// pause/unpause animation. If is a baked model there are no bones -> no animation -> always paused
	if (glfwGetKey(window, PAUSE_KEY) == GLFW_PRESS && !status[PAUSE_KEY_PRESSED] && !status[BAKED_MODEL])
		status[PAUSE] = !status[PAUSE];
	// bake the model in the current pose
	if (glfwGetKey(window, BAKE_MODEL_KEY) == GLFW_PRESS && !status[BAKED_MODEL]) {
		status[BAKED_MODEL] = status[PAUSE] = true;
		model = model.Bake();
	}
	// enable/disable hidden line
	if (glfwGetKey(window, SWITCH_ANIMATION_KEY) == GLFW_PRESS && !status[SWITCH_ANIMATION_KEY_PRESSED]) {
		SwitchAnimation();
	}
	// reset camera position
	if (glfwGetKey(window, RESET_CAMERA_KEY) == GLFW_PRESS) {
		camera.Reset();
	}
	// move camera position
	if (glfwGetKey(window, CAMERA_UP_KEY) == GLFW_PRESS) {
		camera.ProcessKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, CAMERA_DOWN_KEY) == GLFW_PRESS) {
		camera.ProcessKeyboard(DOWN, deltaTime);
	}
	if (glfwGetKey(window, CAMERA_LEFT_KEY) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, CAMERA_RIGHT_KEY) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	// keep rotating only if the rotation key is still pressed. 
	// This way it starts rotating when it is pressed and keeps rotating until is released
	status[ROTATE] = glfwGetMouseButton(window, ROTATE_KEY) == GLFW_PRESS;
	// reset status of keys released
	status[SWITCH_ANIMATION_KEY_PRESSED] = glfwGetKey(window, SWITCH_ANIMATION_KEY) == GLFW_PRESS;
	status[WIREFRAME_KEY_PRESSED] = glfwGetKey(window, WIREFRAME_KEY) == GLFW_PRESS;
	status[HIDDEN_LINE_KEY_PRESSED] = glfwGetKey(window, HIDDEN_LINE_KEY) == GLFW_PRESS;
	status[PAUSE_KEY_PRESSED] = glfwGetKey(window, PAUSE_KEY) == GLFW_PRESS;
}

void StatusManager::SwitchAnimation()
{
	animator.PlayNextAnimation();
}

void StatusManager::InitStatus()
{
	//init keys status
	status[WIREFRAME_KEY_PRESSED] = 0;
	status[HIDDEN_LINE_KEY_PRESSED] = 0;
	status[PAUSE_KEY_PRESSED] = 0;
	status[SWITCH_ANIMATION_KEY_PRESSED] = 0;
	//init status
	status[WIREFRAME] = 0;
	status[HIDDEN_LINE] = 1;
	status[ROTATE] = 0;
	status[PAUSE] = 0;
	status[BAKED_MODEL] = 0;
}
