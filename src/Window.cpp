#include "Window.h"

GLFWwindow* CreateWindow() {
	GLFWwindow* window = glfwCreateWindow(SCREEN_INITIAL_WIDTH, SCREEN_INITIAL_HEIGHT, "Reskinner", NULL, NULL);
	if (window == NULL)
		return window;
	glfwMakeContextCurrent(window);

	//Enable vsync
	glfwSwapInterval(1);

	//Setup callbacks
	process_mouse_movement = &update_mouse_last_pos;
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_press_callback);
	glfwSetMouseButtonCallback(window, on_mouse_click_callback);

	return window;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);
	status->width = float(width);
	status->height = float(height);
	status->projection = glm::perspective(glm::radians(FOV), (float)width / (float)height, NEAR_PLANE, FAR_PLANE);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	process_mouse_movement(window, xpos, ypos);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);
	status->camera.ProcessMouseScroll(yoffset);
}

void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE)
		return;

	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);

	// move camera position
	bool cameraMove = false;
	if (glfwGetKey(window, CAMERA_UP_KEY) == GLFW_PRESS) {
		status->camera.ProcessKeyboard(UP, status->deltaTime);
		cameraMove = true;
	}
	if (glfwGetKey(window, CAMERA_DOWN_KEY) == GLFW_PRESS) {
		status->camera.ProcessKeyboard(DOWN, status->deltaTime);
		cameraMove = true;
	}
	if (glfwGetKey(window, CAMERA_LEFT_KEY) == GLFW_PRESS) {
		status->camera.ProcessKeyboard(LEFT, status->deltaTime);
		cameraMove = true;
	}
	if (glfwGetKey(window, CAMERA_RIGHT_KEY) == GLFW_PRESS) {
		status->camera.ProcessKeyboard(RIGHT, status->deltaTime);
		cameraMove = true;
	}

	if (action != GLFW_PRESS || cameraMove)
		return;

	// enable/disable wireframe mode
	if (key == WIREFRAME_KEY) {
		status->wireframeEnabled = !status->wireframeEnabled;
		return;
	}
	// pause/unpause animation. If is a baked model there are no bones -> no animation -> always paused
	if (key == PAUSE_KEY)
	{
		status->Pause();
		if (!status->pause)
			process_mouse_movement = &update_mouse_last_pos;
		return;
	}
	// bake the model in the current pose
	if (key == BAKE_MODEL_KEY) {
		status->BakeModel();
		process_mouse_movement = &picking;
		return;
	}
	// switch animation
	if (key == SWITCH_ANIMATION_KEY) {
		status->SwitchAnimation();
		return;
	}
	// reset camera position
	if (key == RESET_CAMERA_KEY) {
		status->camera.Reset();
		return;
	}

	//Undo change
	if (key == UNDO_CHANGE_KEY && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		status->Undo();
		return;
	}
	//Redo change
	if (key == REDO_CHANGE_KEY && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		status->Redo();
		return;
	}
}

void on_mouse_click_callback(GLFWwindow* window, int button, int action, int mods)
{
	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);

	if (action == GLFW_RELEASE && status->isModelBaked)
	{
		process_mouse_movement = &picking;
		if (button == GLFW_MOUSE_BUTTON_LEFT && status->currentChange.offset != glm::vec3(0.0f, 0.0f, 0.0f))
			status->EndChange();
		return;
	}

	if (action == GLFW_RELEASE && !status->isModelBaked) {
		process_mouse_movement = &update_mouse_last_pos;
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && status->pause) {
		status->SetPivot();
		status->info.hitPoint.reset();
		process_mouse_movement = &rotate;
		return;
	}

	if (!status->bakedModel)
		return;

	// selection
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		//check for multiple selection
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
			status->selectedVertices.clear();
		}
		if (!status->info.hitPoint)
			return;
		status->SelectHoveredVertex();
		status->StartChange();
		process_mouse_movement = &tweak;
		return;
	}
}

void rotate(GLFWwindow* window, float xpos, float ypos) {
	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);
	float xoffset = xpos - status->mouseLastPos.x;
	float yoffset = status->mouseLastPos.y - ypos; // reversed since y-coordinates go from bottom to top
	status->camera.ProcessMouseMovement(xoffset, yoffset);
}

void update_mouse_last_pos(GLFWwindow* window, float xpos, float ypos) {
	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);
	status->mouseLastPos.x = xpos;
	status->mouseLastPos.y = ypos;
}

void picking(GLFWwindow* window, float xpos, float ypos) {
	update_mouse_last_pos(window, xpos, ypos);
	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);
	status->info = status->Picking();
}

void tweak(GLFWwindow* window, float xpos, float ypos) {
	update_mouse_last_pos(window, xpos, ypos);
	StatusManager* status = (StatusManager*)glfwGetWindowUserPointer(window);
	status->TweakSelectedVertices();
}