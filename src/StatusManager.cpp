#include <reskinner/StatusManager.h>

StatusManager::StatusManager(float screenWidth, float screenHeight)
	:
	mouseLastPos(glm::vec2(screenWidth / 2, screenHeight / 2)),
	camera(glm::vec3(0.0f, 0.0f, 3.0f)),
	lastFrame(glfwGetTime()),
	deltaTime(0.0f),
	pause(false),
	wireframeEnabled(false),
	isModelBaked(false),
	rotating(false),
	changingMesh(false),
	keys(0)
{}

void StatusManager::AddAnimation(const char* path)
{
	if (animatedModel)
		animator.AddAnimation(Animation(filesystem::path(path).string(), *animatedModel));
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
	if (glfwGetKey(window, WIREFRAME_KEY) == GLFW_PRESS && !keys[WIREFRAME_KEY_PRESSED])
		wireframeEnabled = !wireframeEnabled;

	// pause/unpause animation. If is a baked model there are no bones -> no animation -> always paused
	if (glfwGetKey(window, PAUSE_KEY) == GLFW_PRESS && !keys[PAUSE_KEY_PRESSED]) {
		pause = !pause;
	}
	// bake the model in the current pose
	if (glfwGetKey(window, BAKE_MODEL_KEY) == GLFW_PRESS && !isModelBaked) {
		BakeModel();
	}
	// enable/disable hidden line
	if (glfwGetKey(window, SWITCH_ANIMATION_KEY) == GLFW_PRESS && !keys[SWITCH_ANIMATION_KEY_PRESSED]) {
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

	// controlla tweaking
	/*if (glfwGetMouseButton(window, CHANGE_MESH_KEY) == GLFW_PRESS) {
		changingMesh = true;
		ChangeMesh();
	}
	if (changingMesh && glfwGetMouseButton(window, CHANGE_MESH_KEY) == GLFW_RELEASE) {
		changingMesh = false;
		BakeModel();
	}*/
	// keep rotating only if the rotation key is still pressed. 
	// This way it starts rotating when it is pressed and keeps rotating until is released
	rotating = glfwGetMouseButton(window, ROTATE_KEY) == GLFW_PRESS;
	if (rotating && !keys[ROTATION_KEY_PRESSED])
	{
		bool bakupBaked = isModelBaked;
		BakeModel();
		auto info = FacePicking(false);
		if (info.hitPoint) {
			auto indices = info.face.value().indices;
			Mesh& hittedMesh = bakedModel->meshes[info.meshIndex];
			int closestIndex = getClosestVertexIndex(info.hitPoint.value(), hittedMesh, indices[0], indices[1], indices[2]);
			animatedModel.value().pivot = hittedMesh.vertices[closestIndex].Position;
		}
		else {
			animatedModel.value().pivot = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		//restore status
		isModelBaked = bakupBaked;
		if (!isModelBaked) {
			bakedModel.reset();
		}
	}
	// reset status of keys released
	keys[SWITCH_ANIMATION_KEY_PRESSED] = glfwGetKey(window, SWITCH_ANIMATION_KEY) == GLFW_PRESS;
	keys[WIREFRAME_KEY_PRESSED] = glfwGetKey(window, WIREFRAME_KEY) == GLFW_PRESS;
	keys[PAUSE_KEY_PRESSED] = glfwGetKey(window, PAUSE_KEY) == GLFW_PRESS;
	keys[ROTATION_KEY_PRESSED] = glfwGetMouseButton(window, ROTATE_KEY) == GLFW_PRESS;
}

void StatusManager::Update(GLFWwindow* window)
{
	UpdateDeltaTime();
	ProcessInput(window);
	if (!pause) {
		bakedModel.reset();
		isModelBaked = false;
		animator.UpdateAnimation(deltaTime);
	}
}

void StatusManager::SwitchAnimation()
{
	animator.PlayNextAnimation();
}

void StatusManager::BakeModel() {
	isModelBaked = true;
	pause = true;
	bakedModel.reset();
	bakedModel.emplace(animatedModel->Bake(animator.GetFinalBoneMatrices()));
}

void StatusManager::ChangeMesh()
{
	auto info = FacePicking(false);
}

PickingInfo StatusManager::FacePicking(bool reload)
{
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(width, height)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);

	glm::mat4 proj = glm::perspective(glm::radians(FOV), aspect_ratio, NEAR_PLANE, FAR_PLANE);
	glm::mat4 modelView = GetModelViewMatrix();

	glm::mat4 toWorld = glm::inverse(proj * modelView);

	rayStartPos = toWorld * rayStartPos;
	rayEndPos = toWorld * rayEndPos;

	rayStartPos /= rayStartPos.w;
	rayEndPos /= rayEndPos.w;

	glm::vec3 dir = glm::normalize(glm::vec3(rayEndPos - rayStartPos));

	float minDist = 0.0f;

	PickingInfo res{};
	for (int i = 0; i < animatedModel->meshes.size(); i++)
	{
		Mesh& m = bakedModel->meshes[i];
		for (Face& f : m.faces)
		{
			Vertex& ver1 = m.vertices[f.indices[0]];
			Vertex& ver2 = m.vertices[f.indices[1]];
			Vertex& ver3 = m.vertices[f.indices[2]];
			IntersectionInfo info = rayPlaneIntersection(rayStartPos, dir, ver1, ver2, ver3);
			if (info.distance > 0.0)
			{
				//object i has been clicked. probably best to find the minimum t1 (front-most object)
				if (!res.face || info.distance < minDist)
				{
					res.hitPoint = info.hitPoint;
					res.face.emplace(f);
					res.meshIndex = i;
					minDist = info.distance;
				}
			}
		}
	}

	return res;
}

glm::mat4 StatusManager::GetModelViewMatrix()
{
	glm::mat4 model = glm::mat4(1.0f);
	if (animatedModel) {
		model = animatedModel.value().GetModelMatrix();
	}
	return camera.GetViewMatrix() * model;
}

void StatusManager::LoadModel(std::string& path)
{
	animatedModel.reset();
	bakedModel.reset();
	pause = isModelBaked = false;
	animator.animations.clear();
	animator.currentAnimationIndex = 0;
	texMan.textures.clear();
	animatedModel.emplace(Model(path, texMan));
	animatedModel.value().pivot = glm::vec3(-0.3f, 1.3f, 0.3f);
}
