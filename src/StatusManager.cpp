#include <reskinner/StatusManager.h>

#include <math.h>       /* sqrt */

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
	if(animatedModel)
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
		BakeModel();
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
		status[BAKED_MODEL] = false;
		currentModel.reset();
		currentModel.emplace(*animatedModel);
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
	currentModel.reset();
	bakedModel.emplace(animatedModel->Bake(animator.GetFinalBoneMatrices()));
	currentModel.emplace(*bakedModel);
}

void StatusManager::ChangeMesh()
{
	std::cout << "Change mesh \n";
	if (Vertex* ver = Picking()) {
		std::cout << "normale " << ver->Normal.x << ", " << ver->Normal.y << ", " << ver->Normal.z << "\n";
		ver->Position += ver->Normal * 0.01f;
		std::cout << "nuova pos " << ver->Position.x << ", " << ver->Position.y << ", " << ver->Position.z << "\n";
		for (Mesh& m : currentModel->meshes)
			m.Reload();
	}
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

Vertex* StatusManager::Picking()
{
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(800.0f, 800.0f)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);

	glm::mat4 proj = glm::perspective(glm::radians(ZOOM), 1.0f, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	glm::mat4 toWorld = glm::inverse(proj * view);

	rayStartPos = toWorld * rayStartPos;
	rayEndPos = toWorld * rayEndPos;

	rayStartPos /= rayStartPos.w;
	rayEndPos /= rayEndPos.w;

	glm::vec3 dir = glm::normalize(glm::vec3(rayEndPos - rayStartPos));

	Vertex* ver = nullptr;
	float minDist = 0.0f;

	for (Mesh& m : currentModel->meshes)
	{
		for (Vertex& v : m.vertices) {
			v.Selected = 0;
			float t = intersectSphere(rayStartPos, dir, v.Position, 0.01f);
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
		std::cout << "trovato" << ver->Position.x << ", " << ver->Position.y << ", " << ver->Position.z << "\n";
		ver->Selected = 1;
		for (Mesh& m : currentModel->meshes)
		{
			for (Face& f : m.faces)
			{
				Vertex& v1 = m.vertices[f.indices[0]];
				Vertex& v2 = m.vertices[f.indices[1]];
				Vertex& v3 = m.vertices[f.indices[2]];
				if (v1.Selected == 1) {
					v2.Selected = v3.Selected = 2;
				}
				if (v2.Selected == 1) {
					v1.Selected = v3.Selected = 2;
				}
				if (v3.Selected == 1) {
					v2.Selected = v1.Selected = 2;
				}
			}
		}
	}
	for (Mesh& m : currentModel->meshes)
		m.Reload();
	return ver;
}

float magnitude(glm::vec3 v) {
	return sqrt(glm::dot(v, v));
}

float intersectPlane(glm::vec3 rayOrigin, glm::vec3 direction, Vertex& v1, Vertex& v2, Vertex& v3) {
	glm::vec3 e12 = glm::normalize(v2.Position - v1.Position);
	glm::vec3 e31 = glm::normalize(v1.Position - v3.Position);
	glm::vec3 e23 = glm::normalize(v3.Position - v2.Position);
	glm::vec3 normalPlane = glm::normalize(glm::cross(e12, e23));

	//check if not parallel
	float normalDotDirection = glm::dot(normalPlane, direction);
	if (normalDotDirection == 0.0f) //it means direction and normal of the plane are perpendicular, so the ray is parallel to the plane
		return -1.0f;

	// dot((P-P0), normal) = 0, normal<-->[a,b,c]
	// (x-x0)*a + (y-y0)*b + (z-z0)*c = 0 ---> ax + by + cz - dot(P0, normal) = 0, so d=- dot(P0, normal) 
	float d = - glm::dot(normalPlane, v1.Position);

	// the intersection point is the solution of this sistem
	// P-P0 = t*rayDirection <---> P(t) = P0+t*raydir , P0 is the ray origin
	// a*Px + b*Py + c*Pz + d = 0 <----> dot(normal, P) + d = 0
	// 
	// to find the t:
	// dot(normal, P0+t*raydir) + d = 0 <---> dot(normal,P0) + dot(normal, t*raydir) + d = 0
	// dot(normal, P0) + t*dot(normal,raydir) + d = 0
	// t = -[dot(normal,P0) + d]/dot(normal, raydir)
	float distRayOriginPlane = glm::dot(normalPlane, rayOrigin) + d;

	float distFollowingRayDirFromRayOrigin = -distRayOriginPlane / normalDotDirection;

	glm::vec3 intersectionPoint = rayOrigin + distFollowingRayDirFromRayOrigin * direction;
	// check if the point is in the triangle
	// let's say A and B are two edges of the triangle and B is on the left of A, than dot(cross(A,B),normal)>0
	// while if B in on the right of A, dot(cross(A,B), normal)<0
	// if instead of B we use the intersection point P, than we have to check for each edge if P is on the left of the edge
	
	bool checkEdge1 = glm::dot(glm::normalize(glm::cross(e12, intersectionPoint - v1.Position)), normalPlane)<0.9;
	if (checkEdge1) return -1.0f;
	bool checkEdge2 = glm::dot(glm::normalize(glm::cross(e23, intersectionPoint - v2.Position)), normalPlane) < 0.9f;
	if (checkEdge2) return -1.0f;
	bool checkEdge3 = glm::dot(glm::normalize(glm::cross(e31, intersectionPoint - v3.Position)), normalPlane) < 0.9f;
	if (checkEdge3) return -1.0f;

	
	return distFollowingRayDirFromRayOrigin;
}

void StatusManager::FacePicking()
{
	glm::vec2 mousePos = (mouseLastPos / glm::vec2(800.0f, 800.0f)) * 2.0f - 1.0f;
	mousePos.y = -mousePos.y; //origin is top-left and +y mouse is down

	glm::vec4 rayStartPos = glm::vec4(mousePos, 0.0f, 1.0f);
	glm::vec4 rayEndPos = glm::vec4(mousePos, 1.0f, 1.0f);

	glm::mat4 proj = glm::perspective(glm::radians(ZOOM), 1.0f, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	glm::mat4 toWorld = glm::inverse(proj * view);

	rayStartPos = toWorld * rayStartPos;
	rayEndPos = toWorld * rayEndPos;

	rayStartPos /= rayStartPos.w;
	rayEndPos /= rayEndPos.w;

	glm::vec3 dir = glm::normalize(glm::vec3(rayEndPos - rayStartPos));

	Vertex* v1 = nullptr;
	Vertex* v2 = nullptr;
	Vertex* v3 = nullptr;
	float minDist = 0.0f;

	for (Mesh& m : currentModel->meshes)
	{
		for (Vertex& v : m.vertices) {
			v.Selected = 0;
		}
		for (Face& f : m.faces)
		{
			Vertex& ver1 = m.vertices[f.indices[0]];
			Vertex& ver2 = m.vertices[f.indices[1]];
			Vertex& ver3 = m.vertices[f.indices[2]];
			float t = intersectPlane(rayStartPos, dir, ver1, ver2, ver3);
			if (t > 0.0f)
			{
				//object i has been clicked. probably best to find the minimum t1 (front-most object)
				if (v1 == nullptr || t < minDist)
				{
					minDist = t;
					v1 = &ver1;
					v2 = &ver2;
					v3 = &ver3;
				}
			}
		}
	}
	if (v1) {
		std::cout << "trovato1: " << v1->Position.x << ", " << v1->Position.y << ", " << v1->Position.z << "\n";
		std::cout << "trovato2: " << v2->Position.x << ", " << v2->Position.y << ", " << v2->Position.z << "\n";
		std::cout << "trovato3: " << v3->Position.x << ", " << v3->Position.y << ", " << v3->Position.z << "\n\n";
		v1->Selected = 1;
		v2->Selected = 1;
		v3->Selected = 1;
	}
	for (Mesh& m : currentModel->meshes)
		m.Reload();
}

void StatusManager::LoadModel(std::string& path)
{
	animatedModel.reset();
	animatedModel.emplace(Model(path, texMan));
	currentModel.emplace(*animatedModel);
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
