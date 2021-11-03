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
	HVAO(0),
	HVBO(0),
	keys(0),
	modelShader(Shader("./Shaders/animated_model_loading.vs", "./Shaders/animated_model_loading.fs")),
	wireframeShader(Shader("./Shaders/wireframe.vs", "./Shaders/wireframe.fs")),
	mouseShader(Shader("./Shaders/mouse_shader.vs", "./Shaders/mouse_shader.fs")),
	hoverShader(Shader("./Shaders/hover.vs", "./Shaders/hover.fs"))
{
	glGenVertexArrays(1, &HVAO);
	glGenBuffers(1, &HVBO);
	glBindVertexArray(HVAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, HVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(hoveredVertices), NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);
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
		auto info = FacePicking();
		if (info.hitPoint) {
			auto indices = info.face.value().indices;
			Mesh& hittedMesh = bakedModel->meshes[info.meshIndex];
			int closestIndex = getClosestVertexIndex(info.hitPoint.value(), hittedMesh, indices[0], indices[1], indices[2]);
			camera.pivot = hittedMesh.vertices[closestIndex].Position;
		}
		else {
			camera.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
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
	auto info = FacePicking();
}

PickingInfo StatusManager::FacePicking()
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
	return camera.GetViewMatrix();
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
	camera.pivot = glm::vec3(-0.3f, 1.3f, 0.3f);
}

void StatusManager::Render()
{
	glClearColor(1.0f, 0.5f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!animatedModel)
		return;
	// draw wireframe if enabled
	if (wireframeEnabled)
		DrawWireframe();
	// draw model
	DrawModel();
	// check if something is hovered and shoudl be drawn
	if (rotating || !bakedModel)
		return;
	auto hoveredInfo = FacePicking();
	if (!hoveredInfo.hitPoint)
		return;
	Mesh& m = bakedModel.value().meshes[hoveredInfo.meshIndex];
	Face& f = hoveredInfo.face.value();
	//vertex 1
	hoveredVertices[0] = m.vertices[f.indices[0]].Position.x;
	hoveredVertices[1] = m.vertices[f.indices[0]].Position.y;
	hoveredVertices[2] = m.vertices[f.indices[0]].Position.z;
	//vertex 2
	hoveredVertices[3] = m.vertices[f.indices[1]].Position.x;
	hoveredVertices[4] = m.vertices[f.indices[1]].Position.y;
	hoveredVertices[5] = m.vertices[f.indices[1]].Position.z;
	//vertex 3
	hoveredVertices[6] = m.vertices[f.indices[2]].Position.x;
	hoveredVertices[7] = m.vertices[f.indices[2]].Position.y;
	hoveredVertices[8] = m.vertices[f.indices[2]].Position.z;
	DrawHoveredFace();
}

void StatusManager::DrawWireframe() {
	wireframeShader.use();
	// model/view/projection transformations
	glm::mat4 projection = glm::perspective(glm::radians(FOV), aspect_ratio, NEAR_PLANE, FAR_PLANE);
	glm::mat4 modelView = GetModelViewMatrix();
	wireframeShader.setMat4("modelView", modelView);
	wireframeShader.setMat4("projection", projection);

	// pass bones matrices to the shader
	auto transforms = animator.GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		wireframeShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	animatedModel.value().Draw(wireframeShader);
}

void StatusManager::DrawModel() {
	modelShader.use();
	// model/view/projection transformations
	glm::mat4 projection = glm::perspective(glm::radians(FOV), aspect_ratio, NEAR_PLANE, FAR_PLANE);
	glm::mat4 modelView = GetModelViewMatrix();
	modelShader.setMat4("modelView", modelView);
	modelShader.setMat4("projection", projection);

	// pass bones matrices to the shader
	auto transforms = animator.GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		modelShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	animatedModel.value().Draw(modelShader);
}

void StatusManager::DrawHoveredFace() {
	glBindVertexArray(HVAO);
	glBindBuffer(GL_ARRAY_BUFFER, HVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hoveredVertices), &hoveredVertices);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	//unbind
	glBindVertexArray(0);
}