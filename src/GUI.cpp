#include <reskinner/GUI.h>

void RenderGUI(StatusManager& status)
{
	bool show_demo_window = true;
	NewGUIFrame();
	RenderMenuBar(status);
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
	OpenMeshDialog(status);
	OpenAnimationDialog(status);
	RenderModelInfo(status);
	RenderCameraInfo(status);
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void SetupImGui(GLFWwindow* window)
{
	// initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	io.ConfigDockingWithShift = false;

	// When viewports are enabled we tweak WindowRounding 
	// WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	SetupFileDialog();
}

void CloseImGui() {
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void SetupFileDialog()
{
	// create a file browser instance
	ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
		GLuint tex;

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		return (void*)tex;
	};

	ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
		GLuint texID = (GLuint)tex;
		glDeleteTextures(1, &texID);
	};
}

void NewGUIFrame() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void OpenMeshDialog(StatusManager& status)
{
	if (ifd::FileDialog::Instance().IsDone("Mesh Open Dialog")) {
		if (ifd::FileDialog::Instance().HasResult()) {
			std::string res = ifd::FileDialog::Instance().GetResult().u8string();
			status.LoadModel(res);
			status.AddAnimation(res.c_str());
		}
		ifd::FileDialog::Instance().Close();
	}
}

void OpenAnimationDialog(StatusManager& status)
{
	if (ifd::FileDialog::Instance().IsDone("Animation Open Dialog")) {
		if (ifd::FileDialog::Instance().HasResult()) {
			std::string res = ifd::FileDialog::Instance().GetResult().u8string();
			status.AddAnimation(res.c_str());
		}
		ifd::FileDialog::Instance().Close();
	}
}

void RenderMenuBar(StatusManager& status)
{
	if (ImGui::BeginMainMenuBar())
	{
		RenderFileMenuSection(status);
		//RenderEditMenuSection(status);
		RenderWindowMenuSection(status);
		ImGui::EndMainMenuBar();
	}
}

void RenderFileMenuSection(StatusManager& status)
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Open new Model...", "CTRL+O")) {
			ifd::FileDialog::Instance().Open("Mesh Open Dialog", "Open a mesh", "Mesh file (*.dae){.dae},.*");
		}
		if (ImGui::MenuItem("Add new animation...", "CTRL+Shift+A") && status.animatedModel) {
			ifd::FileDialog::Instance().Open("Animation Open Dialog", "Open an animation", "Animation file (*.dae){.dae},.*");
		}
		ImGui::EndMenu();
	}
}

void RenderEditMenuSection(StatusManager& status)
{
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
		if (ImGui::MenuItem("Redo", "CTRL+Y")) {}
		ImGui::EndMenu();
	}
}

void RenderWindowMenuSection(StatusManager& status)
{
	if (ImGui::BeginMenu("Window"))
	{
		if (ImGui::MenuItem("Show Model Info")) {
			showModel = true;
		}
		if (ImGui::MenuItem("Show Camera Info")) {
			showCamera = true;
		}
		if (ImGui::MenuItem("Show Animator Info")) {}
		if (ImGui::MenuItem("Show Animations Info")) {}
		if (ImGui::MenuItem("Show Camera Info")) {}
		if (ImGui::MenuItem("Show Status Info")) {}
		ImGui::EndMenu();
	}
}

void RenderModelInfo(StatusManager& status)
{
	static bool uniform = true;
	static float scale = 1.0f;
	if (!(showModel && status.animatedModel))
		return;
	ImGui::Begin("Model", &showModel);
	ImGui::InputFloat3("Position", status.modelPos);
	ImGui::InputFloat3("Rotation", status.modelRot);
	status.modelRot[0] = std::clamp(status.modelRot[0], 0.0f, 360.0f);
	status.modelRot[1] = std::clamp(status.modelRot[1], 0.0f, 360.0f);
	status.modelRot[2] = std::clamp(status.modelRot[2], 0.0f, 360.0f);
	ImGui::Checkbox("Uniform Scale (RESETS SCALE TO 1.0)", &uniform);
	if (uniform)
	{
		ImGui::InputFloat("Scale", &scale);
		status.modelScale[0] = scale;
		status.modelScale[1] = scale;
		status.modelScale[2] = scale;
	}
	else {
		ImGui::InputFloat3("Scale", status.modelScale);
	}
	if (ImGui::Button("Reset Model Settings")) {
		scale = 1.0f;
		uniform = true;
		//Reset position
		status.modelPos[0] = 0.0f;
		status.modelPos[1] = 0.0f;
		status.modelPos[2] = 0.0f;
		//Reset rotation
		status.modelRot[0] = 0.0f;
		status.modelRot[1] = 0.0f;
		status.modelRot[2] = 0.0f;
		//Reset scaling
		status.modelScale[0] = scale;
		status.modelScale[1] = scale;
		status.modelScale[2] = scale;
	}
	RenderMeshesInfo(status);
	ImGui::End();
}

void RenderMeshTextureInfo(Mesh& mesh, int mIndex, int tIndex, TextureManager& texMan) {
	int texManIndex = mesh.texIndices[tIndex];
	Texture tex = texMan.textures[texManIndex];
	ImTextureID texID = (void*)(intptr_t)tex.id;
	std::pair p = std::pair(mIndex, tIndex);
	std::string textureName = tex.type + ": " +std::to_string(texManIndex);
	ImGui::Text(textureName.c_str());
	ImGui::PushID(mIndex * 100 + tex.id);
	if (ImGui::ImageButton(texID, ImVec2(50, 50))) {
		auto it = std::find(panelTex.begin(), panelTex.end(), p);
		if (it == panelTex.end()) {
			panelTex.push_back(p);
		}
	}
	ImGui::PopID();
	ImGui::SameLine();
	if (ImGui::Button("Remove")) {
		mesh.texIndices.erase(mesh.texIndices.begin() + tIndex);
	}

	if(std::find(panelTex.begin(), panelTex.end(), p) != panelTex.end())
		ShowTextureInPanel(mIndex, texManIndex, texID, 500, 500);
}

void ShowTextureInPanel(int mIndex, int tIndex, ImTextureID id, int width, int height)
{
	bool open = true;
	std::string name = "Mesh " + std::to_string(mIndex) + " Texture " + std::to_string(tIndex);
	ImGui::Begin(name.c_str(), &open);
	ImGui::Image(id, ImVec2(width, height));
	ImGui::End();
	if (!open)
		panelTex.erase(std::find(panelTex.begin(), panelTex.end(), std::pair(mIndex, tIndex)));
}

void RenderMeshesInfo(StatusManager& status)
{
	if (!ImGui::CollapsingHeader("Meshes Info"))
		return;
	int meshIndex = 0;
	for (Mesh& m : status.animatedModel->meshes)
	{
		std::string header = "Mesh " + std::to_string(++meshIndex);
		if (!ImGui::CollapsingHeader(header.c_str()))
			continue;
		for (int i = 0; i< m.texIndices.size(); i++)
		{
			RenderMeshTextureInfo(m, meshIndex, i, status.texMan);
		}
		if (!m.texIndices.size())
			ImGui::Text("There are no textures attached to this mesh.");
	}
}

void RenderCameraInfo(StatusManager& status)
{
	if (!showCamera)
		return;
	ImGui::Begin("Camera", &showCamera);
	// Camera Position
	float* cameraPos = glm::value_ptr(status.camera.Position);
	ImGui::InputFloat3("Position", cameraPos);
	status.camera.Position = glm::make_vec3(cameraPos);
	// Camera Rotation
	float yaw = status.camera.Yaw;
	ImGui::InputFloat("X rotation", &yaw, -360.0f, 360.0f);
	yaw = std::clamp(yaw, -360.0f, 360.0f);
	float pitch = status.camera.Pitch;
	ImGui::InputFloat("Y rotation", &pitch, -89.0f, 89.0f);
	pitch = std::clamp(pitch, -90.0f, 90.0f);
	if (status.camera.Yaw - yaw || status.camera.Pitch - pitch) {
		status.camera.Yaw = yaw;
		status.camera.Pitch = pitch;
		status.camera.updateCameraVectors();
	}
	// Camera Settings
	ImGui::InputFloat("Movement speed", &status.camera.MovementSpeed);
	ImGui::InputFloat("Rotation speed", &status.camera.MouseSensitivity);
	ImGui::InputFloat("Zoom speed", &status.camera.ZoomSpeed);
	if (ImGui::Button("Reset Camera Settings"))
		status.camera.Reset();
	ImGui::End();
}
