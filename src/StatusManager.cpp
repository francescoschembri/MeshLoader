#include <reskinner/StatusManager.h>

#include <reskinner/Utility.h>

StatusManager::StatusManager(float screenWidth, float screenHeight)
	:
	camera(glm::vec3(0.0f, 0.0f, 3.0f)),
	lastFrame(glfwGetTime()),
	deltaTime(0.0f),
	mouseLastPos(glm::vec2(screenWidth / 2, screenHeight / 2)),
	pause(false),
	wireframe(false),
	hiddenLine(true),
	sculptingMode(false)
{
	InitStatus();
}

bool StatusManager::IsRotationLocked() const
{
	return status[ROTATE];
}

bool StatusManager::IsPaused() const
{
	return pause;
}

bool StatusManager::IsBaked() const
{
	return status[BAKED_MODEL];
}

bool StatusManager::DrawLines() const
{
	return wireframe;
}

bool StatusManager::DrawFaces() const
{
	return hiddenLine;
}

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
	if (glfwGetKey(window, WIREFRAME_KEY) == GLFW_PRESS && !status[WIREFRAME_KEY_PRESSED])
		wireframe = !wireframe;
	// enable/disable hidden line (possible only if wireframe enabled)
	if (glfwGetKey(window, HIDDEN_LINE_KEY) == GLFW_PRESS && !status[HIDDEN_LINE_KEY_PRESSED] && !wireframe)
		hiddenLine = !hiddenLine;
	// pause/unpause animation. If is a baked model there are no bones -> no animation -> always paused
	if (glfwGetKey(window, PAUSE_KEY) == GLFW_PRESS && !status[PAUSE_KEY_PRESSED]) {
		pause = !pause;
	}
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

	// controlla pennello
	if (pennello1 && glfwGetMouseButton(window, CHANGE_MESH_KEY) == GLFW_PRESS) {
		ChangeMesh();
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

void StatusManager::Update(GLFWwindow* window)
{
	UpdateDeltaTime();
	ProcessInput(window);
	if (!IsPaused()) {
		bakedModel.reset();
		status[BAKED_MODEL] = false;
		animator.UpdateAnimation(deltaTime);
	}
}

void StatusManager::SwitchAnimation()
{
	animator.PlayNextAnimation();
}

void StatusManager::BakeModel() {
	status[BAKED_MODEL] = true;
	pause = true;
	bakedModel.reset();
	bakedModel.emplace(animatedModel->Bake(animator.GetFinalBoneMatrices()));
}

void StatusManager::ChangeMesh()
{
	BakeModel();
	auto pair = FacePicking(false);
	if (pair.first) {
		Mesh& aMesh = animatedModel->meshes[pair.second];
		for (unsigned int i : pair.first->indices) {
			aMesh.vertices[i].Position += aMesh.vertices[i].Normal * 1.5f;
		}
		aMesh.Reload();
	}
}

//Vertex* StatusManager::Picking(bool reload)
//{
//	glm::vec2 mousePos = (mouseLastPos / glm::vec2(800.0f, 800.0f)) * 2.0f - 1.0f;
//	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down
//
//	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
//	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);
//
//	glm::mat4 proj = glm::perspective(glm::radians(ZOOM), 1.0f, 0.1f, 100.0f);
//	glm::mat4 view = camera.GetViewMatrix();
//
//	glm::mat4 toWorld = glm::inverse(proj * view);
//
//	rayStartPos = toWorld * rayStartPos;
//	rayEndPos = toWorld * rayEndPos;
//
//	rayStartPos /= rayStartPos.w;
//	rayEndPos /= rayEndPos.w;
//
//	glm::vec3 dir = glm::normalize(glm::vec3(rayEndPos - rayStartPos));
//
//	Vertex* ver = nullptr;
//	float minDist = 0.0f;
//
//	for (Mesh& m : bakedModel->meshes)
//	{
//		for (Vertex& v : m.vertices) {
//			v.Selected = 0;
//			float t = raySphereIntersection(rayStartPos, dir, v.Position, 0.01f);
//			if (t > 0.0f)
//			{
//				//object i has been clicked. probably best to find the minimum t1 (front-most object)
//				if (ver == nullptr || t < minDist)
//				{
//					minDist = t;
//					ver = &v;
//				}
//			}
//		}
//	}
//
//	if (ver) {
//		ver->Selected = 1;
//		for (Mesh& m : animatedModel->meshes)
//		{
//			bool changed = false;
//			for (Face& f : m.faces)
//			{
//				Vertex& v1 = m.vertices[f.indices[0]];
//				Vertex& v2 = m.vertices[f.indices[1]];
//				Vertex& v3 = m.vertices[f.indices[2]];
//				if (v1.Selected == 1) {
//					changed = true;
//					v2.Selected = v3.Selected = 2;
//				}
//				if (v2.Selected == 1) {
//					changed = true;
//					v1.Selected = v3.Selected = 2;
//				}
//				if (v3.Selected == 1)
//				{
//					changed = true;
//					v2.Selected = v1.Selected = 2;
//				}
//			}
//			if (changed && reload)
//			{
//				m.Reload();
//			}
//		}
//	}
//	return ver;
//}

std::pair<std::optional<Face>, int> StatusManager::FacePicking(bool reload)
{
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(width, height)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);

	glm::mat4 proj = glm::perspective(glm::radians(ZOOM), aspect_ratio, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = animatedModel->GetModelMatrix();

	glm::mat4 toWorld = glm::inverse(proj * view * model);

	rayStartPos = toWorld * rayStartPos;
	rayEndPos = toWorld * rayEndPos;

	rayStartPos /= rayStartPos.w;
	rayEndPos /= rayEndPos.w;

	glm::vec3 dir = glm::normalize(glm::vec3(rayEndPos - rayStartPos));

	std::optional<Face> face;
	int meshIndex = -1;
	float minDist = 0.0f;

	if (lastMeshPicked >= 0)
	{
		animatedModel->meshes[lastMeshPicked].vertices[lastVertexPicked].Selected = 0;
	}

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
				if (!face || info.distance < minDist)
				{
					minDist = info.distance;
					face.emplace(f);
					meshIndex = i;
				}
			}
		}
	}
	if (face) {
		if (lastMeshPicked != meshIndex && lastMeshPicked>=0)
			animatedModel->meshes[lastMeshPicked].Reload();
		lastMeshPicked = meshIndex;
		lastVertexPicked = face->indices[2];
		//mesh->vertices[face->indices[0]].Selected = 1;
		//mesh->vertices[face->indices[1]].Selected = 1;
		//lastMeshPicked->vertices[face->indices[0]].Selected = 1;
		//lastMeshPicked->vertices[face->indices[1]].Selected = 1;
		animatedModel->meshes[meshIndex].vertices[lastVertexPicked].Selected = 1;
		if (reload) animatedModel->meshes[meshIndex].Reload();
	}
	else if (lastMeshPicked >= 0 && reload) {
		animatedModel->meshes[lastMeshPicked].Reload();
		lastVertexPicked = -1;
		lastMeshPicked = -1;
	}

	return std::pair<std::optional<Face>, int>(face, meshIndex);
}

void StatusManager::LoadModel(std::string& path)
{
	animatedModel.reset();
	animator.animations.clear();
	animator.currentAnimationIndex = 0;
	animatedModel.emplace(Model(path, texMan));
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
	status[BAKED_MODEL] = 0;
}
