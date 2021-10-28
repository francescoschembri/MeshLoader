#include <reskinner/StatusManager.h>

StatusManager::StatusManager(float screenWidth, float screenHeight)
	:
	camera(glm::vec3(0.0f, 0.0f, 3.0f)),
	lastFrame(glfwGetTime()),
	deltaTime(0.0f),
	mouseLastPos(glm::vec2(screenWidth / 2, screenHeight / 2)),
	pause(false),
	wireframe(false),
	hiddenLine(true)
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
	if (activeBrush && glfwGetMouseButton(window, CHANGE_MESH_KEY) == GLFW_PRESS) {
		changingMesh = true;
		ChangeMesh();
	}
	if (changingMesh && glfwGetMouseButton(window, CHANGE_MESH_KEY) == GLFW_RELEASE) {
		changingMesh = false;
		BakeModel();
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
	if (activeBrush && !bakedModel) {
		BakeModel();
	}
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
	auto info = FacePicking(false);
	if (info.hitPoint) {
		Mesh& aMesh = animatedModel->meshes[info.meshIndex];
		std::vector<int> verticesToCheck;
		std::vector<int> exploredVertices;
		std::map<int, float> distances;
		verticesToCheck.push_back(info.face->indices[0]);
		verticesToCheck.push_back(info.face->indices[1]);
		verticesToCheck.push_back(info.face->indices[2]);
		exploredVertices.push_back(info.face->indices[0]);
		exploredVertices.push_back(info.face->indices[1]);
		exploredVertices.push_back(info.face->indices[2]);
		distances[info.face->indices[0]] = 0.0f;
		distances[info.face->indices[1]] = 0.0f;
		distances[info.face->indices[2]] = 0.0f;
		std::vector<int> verIndices;
		float radius = activeBrush.value()->radius;
		while (verticesToCheck.size() > 0) {
			int index = verticesToCheck.back();
			verticesToCheck.pop_back();
			float distance = magnitude(aMesh.vertices[index].Position - info.hitPoint.value()) + distances[index];
			//std::cout << "index: " << index << " , distance: " << distance << " , radius: " << radius << "\n";
			if (distance <= radius)
			{
				//std::cout << "picked index: " << index << "\n";
				distances[index] = distance;
				verIndices.push_back(index);
				std::set<int> closeVertices = aMesh.verticesPerVertex[index];
				for (int i : closeVertices) {
					auto iter = std::find(exploredVertices.begin(), exploredVertices.end(), i);
					bool notExplored = !(iter == exploredVertices.end());
					float distanceFromCurrentIndex = magnitude(aMesh.vertices[index].Position - aMesh.vertices[i].Position);
//					std::cout << "next index: " << i << " , distance parent (" << index<< "): "<< distances[index]
//						<< ", distance from parent: " << distanceFromCurrentIndex <<", Total: "<< distances[index] + distanceFromCurrentIndex;
//					if (!notExplored) std::cout << ", prev distance: " << distances[i];
//					std::cout << "\n";
					if (notExplored || distances[i] > distances[index] + distanceFromCurrentIndex) {
						//std::cout << "inserted next index: " << i <<"\n";
						exploredVertices.push_back(i);
						verticesToCheck.push_back(i);
						distances[i] = distance;
					}
				}
			}
		}
		std::cout << "\n\n";
		activeBrush.value()->ModifyMesh(aMesh, verIndices, info.hitPoint.value());
	}
}

PickingInfo StatusManager::FacePicking(bool reload)
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

	float minDist = 0.0f;

	if (lastMeshPicked >= 0)
		animatedModel->meshes[lastMeshPicked].vertices[lastVertexPicked].Selected = 0;

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
	if (res.face) {
		if (lastMeshPicked != res.meshIndex && lastMeshPicked >= 0)
			animatedModel->meshes[lastMeshPicked].Reload();
		lastMeshPicked = res.meshIndex;
		lastVertexPicked = res.face->indices[2];
		//mesh->vertices[face->indices[0]].Selected = 1;
		//mesh->vertices[face->indices[1]].Selected = 1;
		//lastMeshPicked->vertices[face->indices[0]].Selected = 1;
		//lastMeshPicked->vertices[face->indices[1]].Selected = 1;
		animatedModel->meshes[res.meshIndex].vertices[lastVertexPicked].Selected = 1;
		if (reload) animatedModel->meshes[res.meshIndex].Reload();
	}
	else if (lastMeshPicked >= 0 && reload) {
		animatedModel->meshes[lastMeshPicked].Reload();
		lastVertexPicked = -1;
		lastMeshPicked = -1;
	}

	return res;
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
