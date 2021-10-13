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
	return status[PAUSE] || status[BAKED_MODEL];
}

bool StatusManager::IsBaked() const
{
	return status[BAKED_MODEL];
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
		BakeModel();
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

void StatusManager::BakeModel(){
	status[BAKED_MODEL] = true;
	status[PAUSE] = true;
	model = model.Bake(animator.GetFinalBoneMatrices());
}

float intersectSphere(glm::vec3 rayOrigin, glm::vec3 direction, glm::vec3 center, float radius) {
	glm::vec3 oc = rayOrigin - center;
	float a = glm::dot(direction, direction);
	float b = 2.0 * glm::dot(oc, direction);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) {
		return -1.0f;
	}
	else {
		float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
		if (t1 > 0.0f) return t1;
		return (-b + sqrt(discriminant)) / (2.0f * a);
	}
}

void StatusManager::Picking()
{
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(800.0f,800.0f)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::mat4 proj = glm::perspective(glm::radians(ZOOM), 1.0f, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	glm::mat4 toWorld = glm::inverse(proj * view);

	glm::vec3 dir = glm::normalize(glm::vec3(toWorld * glm::vec4(mousePos, 1.0f, 1.0f)));

	Vertex* ver = nullptr;
	float minDist = 0.0f;

	for (Mesh& m: model.meshes)
	{
		for (Vertex& v : m.vertices) {
			v.Selected = 0;
			float t = intersectSphere(camera.Position, dir, v.Position, 0.01f);
			if (t > 0.0f)
			{
				//object i has been clicked. probably best to find the minimum t1 (front-most object)
				if (ver == nullptr || t < minDist)
				{
					minDist = t;
					ver = &v;
				}
			}
		}
	}
	if (ver) {
		std::cout << "trovato"<< ver->Position.x << ", " << ver->Position.y << ", " << ver->Position.z << "\n";
		ver->Selected = 1;
	}
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
