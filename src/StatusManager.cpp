#include "StatusManager.h"

StatusManager::StatusManager(float screenWidth, float screenHeight)
	:
	mouseLastPos(glm::vec2(screenWidth / 2, screenHeight / 2)),
	camera(glm::vec3(0.0f, 0.0f, 3.0f)),
	lastFrame(glfwGetTime()),
	deltaTime(0.0f),
	pause(false),
	wireframeEnabled(false),
	projection(glm::perspective(glm::radians(FOV), screenWidth / screenHeight, NEAR_PLANE, FAR_PLANE)),
	HVAO(0),
	HVBO(0),
	SVAO(0),
	SVBO(0),
	lightingMode(Mode_Flat),
	visualMode(Mode_Texture),
	modelFlatShader(Shader("./Shaders/animated_model_loading.vs", "./Shaders/animated_model_loading.fs", "./Shaders/animated_model_loading.gs")),
	modelNoLightShader(Shader("./Shaders/no_lighting_shader.vs", "./Shaders/no_lighting_shader.fs")),
	modelSmoothShader(Shader("./Shaders/smooth_lighting_shader.vs", "./Shaders/smooth_lighting_shader.fs")),
	wireframeShader(Shader("./Shaders/wireframe.vs", "./Shaders/wireframe.fs")),
	mouseShader(Shader("./Shaders/mouse_shader.vs", "./Shaders/mouse_shader.fs")),
	hoverShader(Shader("./Shaders/hover.vs", "./Shaders/hover.fs")),
	selectedShader(Shader("./Shaders/selected.vs", "./Shaders/selected.fs")),
	numBonesShader(Shader("./Shaders/num_bones_visualization.vs", "./Shaders/num_bones_visualization.fs")),
	currentBoneShader(Shader("./Shaders/influence_of_single_bone.vs", "./Shaders/influence_of_single_bone.fs")),
	currentChange(Change(selectedVerticesPointers)),
	changeIndex(-1)
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
	assert(animatedModel);
	animator.AddAnimation(Animation(std::string(path), *animatedModel));
}

void StatusManager::Pause()
{
	pause = !pause;
	if (pause)
		BakeModel();
	else
		UnbakeModel();
}

void StatusManager::SetPivot()
{
	assert(bakedModel.has_value());
	if (info.hitPoint) {
		Mesh& hittedMesh = bakedModel->meshes[info.meshIndex];
		int closestIndex = getClosestVertexIndex(info.hitPoint.value(), hittedMesh, info.face.value());
		camera.pivot = hittedMesh.vertices[closestIndex].Position;
		info = PickingInfo{};
	}
	else {
		camera.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
	}
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
	if (pause)
		return;
	animator.UpdateAnimation(deltaTime);
}

void StatusManager::NextAnimation()
{
	animator.PlayNextAnimation();
}

void StatusManager::PrevAnimation()
{
	animator.PlayPrevAnimation();
}

void StatusManager::BakeModel() {
	assert(!bakedModel.has_value());
	bakedModel.emplace(animatedModel->Bake(animator.GetFinalBoneMatrices()));
}

void StatusManager::UnbakeModel()
{
	//assert(bakedModel);
	info.hitPoint.reset();
	bakedModel.reset();
	selectedVertices.clear();
}

PickingInfo StatusManager::Picking()
{
	assert(bakedModel.has_value());
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
	for (int i = 0; i < bakedModel->meshes.size(); i++)
	{
		Mesh& m = bakedModel->meshes[i];
		if (!animatedModel->meshes[i].enabled) continue;
		for (Face& f : m.faces)
		{
			Vertex& ver1 = m.vertices[f.indices[0]];
			Vertex& ver2 = m.vertices[f.indices[1]];
			Vertex& ver3 = m.vertices[f.indices[2]];
			IntersectionInfo tmpInfo = rayTriangleIntersection(rayStartPos, dir, ver1, ver2, ver3);
			if (tmpInfo.distance > FLT_EPSILON)
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
	assert(bakedModel.has_value());
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
{ // CHANGE INDEX DOESN'T CHANGE WITH -- ???????
	if (changeIndex >= 0) {
		assert(changeIndex < changes.size());
		changes[changeIndex--].Undo();
		animatedModel.value().Reload();
	}
	UpdateSelectedVertices();
}

void StatusManager::Redo()
{
	if (changeIndex < changes.size() - 1) {
		assert(changes.size() > 0);
		assert(changeIndex >= -1);
		changes[++changeIndex].Apply();
		animatedModel.value().Reload();
	}
	UpdateSelectedVertices();
}

bool StatusManager::IsChanging()
{
	return currentChange.offset.x > FLT_EPSILON || currentChange.offset.x < -FLT_EPSILON
		|| currentChange.offset.y > FLT_EPSILON || currentChange.offset.y < -FLT_EPSILON
		|| currentChange.offset.z > FLT_EPSILON || currentChange.offset.z < -FLT_EPSILON;
}

void StatusManager::LoadModel(std::string& path)
{
	animator.animations.clear();
	animator.currentAnimationIndex = 0;
	texMan.ClearTextures();
	animatedModel.emplace(path, texMan);
	if (pause)
		BakeModel();
	else
		UnbakeModel();
	AddAnimation(path.c_str());
}

void StatusManager::CompleteLoad(std::string& path)
{
	animator.animations.clear();
	animator.currentAnimationIndex = 0;
	texMan.ClearTextures();
	animatedModel.emplace(path, texMan);
	if (pause)
		BakeModel();
	else
		UnbakeModel();
	std::string dir_name = path.substr(0, path.find_last_of("/"));
	dir_name = dir_name.substr(0, dir_name.find_last_of("/"));
	auto dir = std::filesystem::recursive_directory_iterator(dir_name);
	for (auto animation : dir) {
		if (animation.path().extension().u8string().compare(".dae") != 0)
			continue;
		dir_name = animation.path().u8string();
		std::replace(dir_name.begin(), dir_name.end(), '\\', '/');
		std::cout << dir_name << "\n";
		AddAnimation(dir_name.c_str());
	}
	
}

bool StatusManager::SelectHoveredVertex()
{
	assert(info.hitPoint.has_value());
	assert(bakedModel.has_value());

	Mesh& bMesh = bakedModel.value().meshes[info.meshIndex];
	Face& f = info.face.value();
	int verIndex = getClosestVertexIndex(info.hitPoint.value(), bMesh, f);
	Vertex* v = &bMesh.vertices[verIndex];
	if (bMesh.vertices[verIndex].originalVertex->BoneData.NumBones < 4) return false;
	//avoid duplicates and allow removing selected vertices
	auto iter = std::find(selectedVertices.begin(), selectedVertices.end(), *v);
	int index = iter - selectedVertices.begin();
	if (iter == selectedVertices.end()) {
		selectedVertices.push_back(*v);
		selectedVerticesPointers.push_back(v);
	}
	else if (removeIfDouble) {
		selectedVertices.erase(iter);
		selectedVerticesPointers.erase(selectedVerticesPointers.begin() + index);
	}
	return true;
}

bool StatusManager::SelectHoveredEdge()
{
	Mesh& bMesh = bakedModel.value().meshes[info.meshIndex];
	Face& f = info.face.value();
	Line closestLine = getClosestLineIndex(info.hitPoint.value(), bMesh, f);
	Vertex* v1 = &bMesh.vertices[closestLine.v1];
	Vertex* v2 = &bMesh.vertices[closestLine.v2];
	bool selected = false;
	if (bMesh.vertices[f.indices[0]].originalVertex->BoneData.NumBones == 4) {
		//avoid duplicates and allow removing selected vertices
		auto iter = std::find(selectedVertices.begin(), selectedVertices.end(), *v1);
		int index = iter - selectedVertices.begin();
		if (iter == selectedVertices.end()) {
			selectedVertices.push_back(*v1);
			selectedVerticesPointers.push_back(v1);
		}
		else if (removeIfDouble) {
			selectedVertices.erase(iter);
			selectedVerticesPointers.erase(selectedVerticesPointers.begin() + index);
		}
		selected = true;
	}
	if (bMesh.vertices[f.indices[1]].originalVertex->BoneData.NumBones == 4) {
		//avoid duplicates and allow removing selected vertices
		auto iter = std::find(selectedVertices.begin(), selectedVertices.end(), *v2);
		int index = iter - selectedVertices.begin();
		if (iter == selectedVertices.end()) {
			selectedVertices.push_back(*v2);
			selectedVerticesPointers.push_back(v2);
		}
		else if (removeIfDouble) {
			selectedVertices.erase(iter);
			selectedVerticesPointers.erase(selectedVerticesPointers.begin() + index);
		}
		selected = true;
	}
	return selected;
}

bool StatusManager::SelectHoveredFace()
{
	Mesh& bMesh = bakedModel.value().meshes[info.meshIndex];
	Face& f = info.face.value();
	Vertex* v1 = &bMesh.vertices[f.indices[0]];
	Vertex* v2 = &bMesh.vertices[f.indices[1]];
	Vertex* v3 = &bMesh.vertices[f.indices[2]];
	bool selected = false;
	if (bMesh.vertices[f.indices[0]].originalVertex->BoneData.NumBones == 4) {
		//avoid duplicates and allow removing selected vertices
		auto iter = std::find(selectedVertices.begin(), selectedVertices.end(), *v1);
		int index = iter - selectedVertices.begin();
		if (iter == selectedVertices.end()) {
			selectedVertices.push_back(*v1);
			selectedVerticesPointers.push_back(v1);
		}
		else if (removeIfDouble) {
			selectedVertices.erase(iter);
			selectedVerticesPointers.erase(selectedVerticesPointers.begin() + index);
		}
		selected = true;
	}
	if (bMesh.vertices[f.indices[1]].originalVertex->BoneData.NumBones == 4) {
		//avoid duplicates and allow removing selected vertices
		auto iter = std::find(selectedVertices.begin(), selectedVertices.end(), *v2);
		int index = iter - selectedVertices.begin();
		if (iter == selectedVertices.end()) {
			selectedVertices.push_back(*v2);
			selectedVerticesPointers.push_back(v2);
		}
		else if (removeIfDouble) {
			selectedVertices.erase(iter);
			selectedVerticesPointers.erase(selectedVerticesPointers.begin() + index);
		}
		selected = true;
	}
	if (bMesh.vertices[f.indices[2]].originalVertex->BoneData.NumBones == 4) {
		//avoid duplicates and allow removing selected vertices
		auto iter = std::find(selectedVertices.begin(), selectedVertices.end(), *v3);
		int index = iter - selectedVertices.begin();
		if (iter == selectedVertices.end()) {
			selectedVertices.push_back(*v3);
			selectedVerticesPointers.push_back(v3);
		}
		else if (removeIfDouble) {
			selectedVertices.erase(iter);
			selectedVerticesPointers.erase(selectedVerticesPointers.begin() + index);
		}
		selected = true;
	}
	return selected;
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

	currentChange = Change(selectedVerticesPointers);
	info = PickingInfo{};
}

void StatusManager::EndChange()
{
	changes.push_back(currentChange);
	changeIndex++;
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

	hotPoint = glm::vec3(rayStartPos) + dir * rayLenghtOnChangeStart;
	glm::vec3 offset = hotPoint - startChangingPos;
	currentChange.Modify(offset);
	animatedModel.value().Reload();
	UpdateSelectedVertices();
}

void StatusManager::IncreaseCurrentBoneID()
{
	currentBoneID++;
	std::cout << "bone ID: " << currentBoneID << "\n";
}

void StatusManager::DecreaseCurrentBoneID()
{
	currentBoneID--;
	std::cout << "bone ID: " << currentBoneID << "\n";
}

void StatusManager::Render()
{
	//TODO REVIEW
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!animatedModel)
		return;
	Update();

	// draw wireframe if enabled
	if (wireframeEnabled)
		DrawWireframe();
	if (visualMode == Mode_Texture) {
		// draw model
		if (lightingMode == Mode_Flat)
			DrawModel(modelFlatShader);
		else if (lightingMode == Mode_Smooth)
			DrawModel(modelSmoothShader);
		else
			DrawModel(modelNoLightShader);
	}
	else if (visualMode == Mode_CurrentBoneIDInfluence) {
		DrawModel(currentBoneShader);
	}
	else
	{
		DrawModel(numBonesShader);
	}

	// render selected vertices
	DrawSelectedVertices();
	if (!info.hitPoint)
		return;

	//DrawHotPoint();
	if (selectionMode == Mode_Vertex)
		DrawHoveredPoint();
	if (selectionMode == Mode_Edge)
		DrawHoveredLine();
	if (selectionMode == Mode_Face)
		DrawHoveredFace();
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

void StatusManager::DrawModel(Shader& modelShader) {
	modelShader.use();
	// model/view/projection transformations
	modelShader.setMat4("modelView", camera.viewMatrix);
	modelShader.setMat4("projection", projection);
	if (visualMode == Mode_Texture)
		modelShader.setVec3("light_pos", lightPos);
	else if (visualMode == Mode_CurrentBoneIDInfluence)
		modelShader.setInt("currentBoneID", currentBoneID);

	// pass bones matrices to the shader
	auto transforms = animator.GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		modelShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	/*if (pause)
		bakedModel.value().Draw(modelShader);
	else*/
	animatedModel.value().Draw(modelShader);
}

void StatusManager::DrawHoveredFace() {
	assert(bakedModel.has_value());
	assert(info.hitPoint.has_value());
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
	assert(bakedModel.has_value());
	assert(info.hitPoint.has_value());
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

void StatusManager::DrawHotPoint()
{
	float hotVertices[3] = { hotPoint.x, hotPoint.y, hotPoint.z };

	hoverShader.use();
	glBindVertexArray(HVAO);
	glBindBuffer(GL_ARRAY_BUFFER, HVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), &hotVertices);

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

void StatusManager::UpdateSelectedVertices()
{
	selectedVertices.clear();
	for (Vertex* v : selectedVerticesPointers)
		selectedVertices.push_back(*v);
}



void StatusManager::DrawHoveredLine() {
	assert(bakedModel.has_value());
	assert(info.hitPoint.has_value());
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