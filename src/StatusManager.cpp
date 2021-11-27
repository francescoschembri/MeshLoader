#include "StatusManager.h"

StatusManager::StatusManager(float screenWidth, float screenHeight)
	:
	mouseLastPos(glm::vec2(screenWidth / 2, screenHeight / 2)),
	camera(glm::vec3(0.0f, 0.0f, 3.0f)),
	lastFrame(glfwGetTime()),
	deltaTime(0.0f),
	pause(false),
	wireframeEnabled(false),
	isModelBaked(false),
	HVAO(0),
	HVBO(0),
	SVAO(0),
	SVBO(0),
	modelShader(Shader("./Shaders/animated_model_loading.vs", "./Shaders/animated_model_loading.fs")),
	wireframeShader(Shader("./Shaders/wireframe.vs", "./Shaders/wireframe.fs")),
	mouseShader(Shader("./Shaders/mouse_shader.vs", "./Shaders/mouse_shader.fs")),
	hoverShader(Shader("./Shaders/hover.vs", "./Shaders/hover.fs")),
	selectedShader(Shader("./Shaders/selected.vs", "./Shaders/selected.fs")),
	currentChange(Change(selectedVertices))
{
	// setup selected vertices vao
	glGenVertexArrays(1, &SVAO);
	glGenBuffers(1, &SVBO);

	// setup hovered vertices vao
	glGenVertexArrays(1, &HVAO);
	glGenBuffers(1, &HVBO);
	glBindVertexArray(HVAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, HVBO);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	// we only care about the position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);
}

void StatusManager::AddAnimation(const char* path)
{
	if (animatedModel)
		animator.AddAnimation(Animation(std::string(path), *animatedModel));
}

void StatusManager::Pause()
{
	pause = !pause;
	if (!pause)
		UnbakeModel();
}

void StatusManager::SetPivot()
{
	bool backup = isModelBaked;
	if (!backup)
		BakeModel();

	auto info = Picking();
	if (info.hitPoint) {
		auto indices = info.face.value().indices;
		Mesh& hittedMesh = bakedModel->meshes[info.meshIndex];
		int closestIndex = getClosestVertexIndex(info.hitPoint.value(), hittedMesh, indices[0], indices[1], indices[2]);
		camera.pivot = hittedMesh.vertices[closestIndex].Position;
	}
	else {
		camera.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	if (!backup)
		UnbakeModel();
}

void StatusManager::UpdateDeltaTime()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void StatusManager::Update()
{
	UpdateDeltaTime();
	if (!pause)
		animator.UpdateAnimation(deltaTime);
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

void StatusManager::UnbakeModel()
{
	info.hitPoint.reset();
	isModelBaked = false;
	bakedModel.reset();
	selectedVertices.clear();
}

PickingInfo StatusManager::Picking()
{
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(width, height)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);

	glm::mat4 toWorld = glm::inverse(projection * camera.viewMatrix);

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
			IntersectionInfo tmpInfo = rayPlaneIntersection(rayStartPos, dir, ver1, ver2, ver3);
			if (tmpInfo.distance > 0.0)
			{
				//object i has been clicked. probably best to find the minimum t1 (front-most object)
				if (!res.face || tmpInfo.distance < minDist)
				{
					res.hitPoint = tmpInfo.hitPoint;
					res.face.emplace(f);
					res.meshIndex = i;
					res.distance = minDist = tmpInfo.distance;
				}
			}
		}
	}

	return res;
}

void StatusManager::DrawSelectedVertices()
{
	if (selectedVertices.size() == 0) return;
	glBindVertexArray(SVAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, SVBO);
	glBufferData(GL_ARRAY_BUFFER, selectedVertices.size() * sizeof(Vertex), &selectedVertices[0], GL_STREAM_DRAW);
	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	// bone ids
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneData.BoneIDs[0]));
	// weights
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneData.Weights[0]));
	// num bones
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneData.NumBones));
	selectedShader.use();
	// model/view/projection transformations
	glm::mat4 modelView = camera.viewMatrix;
	selectedShader.setMat4("modelView", modelView);
	selectedShader.setMat4("projection", projection);
	// pass bones matrices to the shader
	auto transforms = animator.GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		selectedShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 0.0);

	glPointSize(5.0f);
	glDrawArrays(GL_POINTS, 0, selectedVertices.size());
	//unbind
	glBindVertexArray(0);
}

void StatusManager::Undo()
{
	if (changeIndex < changes.size()) {
		changes[changeIndex--].Undo();
		animatedModel.value().Reload();
	}
}

void StatusManager::Redo()
{

	int size = changes.size() - 1;
	if (changeIndex < size) {
		changes[++changeIndex].Apply();
		animatedModel.value().Reload();
	}
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

void StatusManager::SelectHoveredVertex()
{
	Mesh& bMesh = bakedModel.value().meshes[info.meshIndex];
	Face& f = info.face.value();
	Vertex v = getClosestVertex(info.hitPoint.value(), bMesh, f);;

	//avoid duplicates and allow removing selected vertices
	auto iter = std::find(selectedVertices.begin(), selectedVertices.end(), v);
	if (iter == selectedVertices.end()) {
		selectedVertices.push_back(v);;
	}
	else {
		selectedVertices.erase(iter);
	}
}

void StatusManager::StartChange()
{
	startChangingPos = info.hitPoint.value();
	rayLenghtOnChangeStart = info.distance;

	//remove changes that were rollbacked from the history
	int currSize = changes.size();
	for (int i = changeIndex + 1; i < currSize; i++) {
		changes.pop_back();
	}

	currentChange = Change(selectedVertices);
}

void StatusManager::EndChange()
{
	changes.push_back(currentChange);
	changeIndex++;
	//TODO change animated mesh
	//BakeModel();
}

void StatusManager::TweakSelectedVertices()
{
	// make a new ray
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(width, height)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);

	glm::mat4 modelView = camera.viewMatrix;

	glm::mat4 toWorld = glm::inverse(projection * modelView);

	rayStartPos = toWorld * rayStartPos;
	rayEndPos = toWorld * rayEndPos;

	rayStartPos /= rayStartPos.w;
	rayEndPos /= rayEndPos.w;

	glm::vec3 dir = glm::normalize(glm::vec3(rayEndPos - rayStartPos));

	glm::vec3 hotpoint = glm::vec3(rayStartPos) + dir * rayLenghtOnChangeStart;
	glm::vec3 offset = hotpoint - startChangingPos;
	currentChange.Modify(offset);
	animatedModel.value().Reload();
}

void StatusManager::Render()
{
	//TODO REVIEW
	glClearColor(1.0f, 0.5f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!animatedModel)
		return;
	Update();
	// draw wireframe if enabled
	if (wireframeEnabled)
		DrawWireframe();
	// draw model
	DrawModel();
	// render selected vertices
	DrawSelectedVertices();
	if (!info.hitPoint)
		return;
	DrawHoveredPoint();
	//DrawHoveredLine();
	//DrawHoveredFace();
}

void StatusManager::DrawWireframe() {
	wireframeShader.use();
	// model/view/projection transformations
	glm::mat4 modelView = camera.viewMatrix;
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
	glm::mat4 modelView = camera.viewMatrix;
	modelShader.setMat4("modelView", modelView);
	modelShader.setMat4("projection", projection);

	// pass bones matrices to the shader
	auto transforms = animator.GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		modelShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	if (!isModelBaked)
		animatedModel.value().Draw(modelShader);
	else
		bakedModel.value().Draw(modelShader);
}

void StatusManager::DrawHoveredFace() {
	Mesh& m = bakedModel.value().meshes[info.meshIndex];
	Face& f = info.face.value();

	//vertex
	float hoveredVertices[9] = {
		m.vertices[f.indices[0]].Position.x, m.vertices[f.indices[0]].Position.y, m.vertices[f.indices[0]].Position.z,
		m.vertices[f.indices[1]].Position.x, m.vertices[f.indices[1]].Position.y, m.vertices[f.indices[1]].Position.z,
		m.vertices[f.indices[2]].Position.x, m.vertices[f.indices[2]].Position.y, m.vertices[f.indices[2]].Position.z,
	};
	hoverShader.use();
	glBindVertexArray(HVAO);
	glBindBuffer(GL_ARRAY_BUFFER, HVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hoveredVertices), &hoveredVertices);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 0.0);

	// model/view/projection transformations
	glm::mat4 modelView = camera.viewMatrix;
	hoverShader.setMat4("modelView", modelView);
	hoverShader.setMat4("projection", projection);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	//unbind
	glBindVertexArray(0);
}

void StatusManager::DrawHoveredPoint() {
	Mesh& m = bakedModel.value().meshes[info.meshIndex];
	Face& f = info.face.value();
	int index = getClosestVertexIndex(info.hitPoint.value(), m, f);

	//vertex
	float hoveredVertices[3] = { m.vertices[index].Position.x, m.vertices[index].Position.y, m.vertices[index].Position.z };

	hoverShader.use();
	glBindVertexArray(HVAO);
	glBindBuffer(GL_ARRAY_BUFFER, HVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), &hoveredVertices);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 0.0);

	// model/view/projection transformations
	glm::mat4 modelView = camera.viewMatrix;
	hoverShader.setMat4("modelView", modelView);
	hoverShader.setMat4("projection", projection);

	glPointSize(8.0f);
	glDrawArrays(GL_POINTS, 0, 1);

	//unbind
	glBindVertexArray(0);
}



void StatusManager::DrawHoveredLine() {
	Mesh& m = bakedModel.value().meshes[info.meshIndex];
	Face& f = info.face.value();
	auto line = getClosestLineIndex(info.hitPoint.value(), m, f);

	//vertex
	float hoveredVertices[6] = {
		m.vertices[line.v1].Position.x, m.vertices[line.v1].Position.y, m.vertices[line.v1].Position.z,
		m.vertices[line.v2].Position.x, m.vertices[line.v2].Position.y, m.vertices[line.v2].Position.z
	};

	hoverShader.use();
	glBindVertexArray(HVAO);
	glBindBuffer(GL_ARRAY_BUFFER, HVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hoveredVertices), &hoveredVertices);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 0.0);

	// model/view/projection transformations
	glm::mat4 modelView = camera.viewMatrix;
	hoverShader.setMat4("modelView", modelView);
	hoverShader.setMat4("projection", projection);

	glLineWidth(3.0f);
	glDrawArrays(GL_LINES, 0, 2);
	glLineWidth(1.0f);

	//unbind
	glBindVertexArray(0);
}