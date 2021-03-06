#include "GUI.h"

void RenderGUI(StatusManager& status)
{
	bool show_demo_window = false;
	NewGUIFrame();
	RenderMenuBar(status);
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
	OpenMeshDialog(status);
	OpenAnimationDialog(status);
	RenderModelInfo(status);
	RenderCameraInfo(status);
	RenderSelectionInfo(status);
	RenderAnimatorInfo(status);
	RenderLightingInfo(status);
	RenderVisualModeInfo(status);
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
		if (ImGui::MenuItem("Show Animator Info")) {
			showAnimator = true;
		}
		if (ImGui::MenuItem("Show Render Info")) {
			showRenderInfo = true;
		}
		ImGui::EndMenu();
	}
}

void RenderModelInfo(StatusManager& status)
{
	if (!showModel)
		return;
	ImGui::Begin("Model", &showModel);

	ImGui::Checkbox("Show Wireframe", &status.wireframeEnabled);

	RenderMeshesInfo(status);
	ImGui::End();
}

void RenderMeshTextureInfo(Mesh& mesh, int mIndex, int tIndex, TextureManager& texMan) {
	int texManIndex = mesh.texIndices[tIndex];
	Texture tex = texMan.textures[texManIndex];
	ImTextureID texID = (void*)(intptr_t)tex.id;
	std::string textureName = tex.type + ": " + std::to_string(texManIndex);
	ImGui::Text(textureName.c_str());
	int hashedIndex = mIndex * 100 + tIndex;
	ImGui::PushID(hashedIndex);
	if (ImGui::ImageButton(texID, ImVec2(50, 50))) {
		auto it = std::find(panelTex.begin(), panelTex.end(), hashedIndex);
		if (it == panelTex.end()) {
			panelTex.push_back(hashedIndex);
		}
	}
	ImGui::PopID();
	ImGui::SameLine();
	std::string removeBtnName = "Enable##" + std::to_string(mIndex * 100 + tex.id);
	if (ImGui::Button(removeBtnName.c_str())) {
		mesh.texIndices.erase(mesh.texIndices.begin() + tIndex);
	}

	if (std::find(panelTex.begin(), panelTex.end(), hashedIndex) != panelTex.end())
		ShowTextureInPanel(mIndex, tIndex, texID, 500, 500);
}

void ShowTextureInPanel(int mIndex, int tIndex, ImTextureID id, int width, int height)
{
	bool open = true;
	std::string name = "Mesh " + std::to_string(mIndex) + " Texture " + std::to_string(tIndex);
	ImGui::Begin(name.c_str(), &open);
	ImGui::Image(id, ImVec2(width, height));
	ImGui::End();
	int hashedIndex = mIndex * 100 + tIndex;
	if (!open)
		panelTex.erase(std::find(panelTex.begin(), panelTex.end(), hashedIndex));
}

void RenderAnimatorInfo(StatusManager& status)
{
	if (!(showAnimator && status.animatedModel))
		return;
	ImGui::Begin("Animator", &showAnimator);
	int animIndex = status.animator.currentAnimationIndex;
	Animation& anim = status.animator.animations[animIndex];
	std::string currentAnim = "Current animation: " + anim.name;
	ImGui::Text(currentAnim.c_str());
	std::string currentIndex = "Current index: " + std::to_string(animIndex);
	ImGui::Text(currentIndex.c_str());

	ImGui::SameLine();
	if (ImGui::Button(status.pause ? "Play" : "Pause")) {
		status.pause = !status.pause;
	}

	float animDuration = anim.GetDuration();
	std::string durationText = "Duration: " + std::to_string(animDuration);
	ImGui::Text(durationText.c_str());

	ImGui::Separator();
	float currentTime = status.animator.m_CurrentTime;
	ImGui::SliderFloat("Current Time", &currentTime, 0.0f, animDuration);
	float& start = anim.startFrom;
	float& end = anim.endAt;
	ImGui::DragFloatRange2("Current Range", &start, &end, 1.0f, 0.0f, animDuration);
	currentTime = std::clamp(currentTime, start, end);
	if (currentTime != status.animator.m_CurrentTime)
	{
		status.animator.m_CurrentTime = currentTime;
		status.animator.UpdateAnimation(0.0f);
	}

	if (ImGui::Button("Previous Animation")) {
		status.animator.PlayPrevAnimation();
	}
	ImGui::SameLine();
	if (ImGui::Button("Next Animation")) {
		status.animator.PlayNextAnimation();
	}

	int size = status.animator.animations.size();
	std::string otherAnim = "Other animations (" + std::to_string(size - 1) + ")";
	if (!ImGui::CollapsingHeader(otherAnim.c_str()))
	{
		ImGui::End();
		return;
	}

	for (int i = 0; i < status.animator.animations.size(); i++)
	{
		if (animIndex == i)
			continue;
		ShowAnimationNInfo(status.animator, i);
		ImGui::Separator();
	}
	ImGui::End();
}

void RenderVisualModeInfo(StatusManager& status)
{
	if (!showVisualMode) return;
	ImGui::Begin("Visual mode", &showVisualMode);
	ImGui::Text("Visual mode:");
	if (ImGui::RadioButton("Normal", status.visualMode == Mode_Texture)) { status.visualMode = Mode_Texture; }
	if (ImGui::RadioButton("Num bones influece", status.visualMode == Mode_NumBones)) { status.visualMode = Mode_NumBones; }
	if (ImGui::RadioButton("Current Bone ID", status.visualMode == Mode_CurrentBoneIDInfluence)) { status.visualMode = Mode_CurrentBoneIDInfluence; }
	ImGui::InputInt("Current bone ID", &status.currentBoneID);
	ImGui::SameLine();
	if (ImGui::Button("+")) { status.currentBoneID++; }
	ImGui::SameLine();
	if (ImGui::Button("-")) { status.currentBoneID--; }
	ImGui::End();
}

void ShowAnimationNInfo(Animator& animator, int n) {
	Animation& anim = animator.animations[n];
	std::string animNameText = "Animation name: " + anim.name;
	ImGui::Text(animNameText.c_str());
	std::string animIndexText = "Animation index: " + std::to_string(n);
	ImGui::Text(animIndexText.c_str());

	ImGui::SameLine();
	std::string buttonText = "Play Animation##" + std::to_string(n);
	if (ImGui::Button(buttonText.c_str())) {
		animator.PlayAnimationIndex(n);
	}

	float animDuration = anim.GetDuration();
	std::string durationText = "Duration: " + std::to_string(animDuration);
	ImGui::Text(durationText.c_str());

	std::string ticks = "Tick per second: " + std::to_string(anim.GetTicksPerSecond());
	ImGui::Text(ticks.c_str());
}

//void RenderRenderInfo(StatusManager& status)
//{
//	if (!showRenderInfo)
//		return;
//	ImGui::Begin("Render Info", &showRenderInfo);
//	ImGui::End();
//}

void RenderMeshesInfo(StatusManager& status)
{
	if (!ImGui::CollapsingHeader("Meshes Info"))
		return;
	int meshIndex = 0;
	for (Mesh& m : status.animatedModel->meshes)
	{
		std::string header = "Mesh " + std::to_string(++meshIndex);
		if (!ImGui::CollapsingHeader(header.c_str())) continue;
		std::string enabled = header + " enabled";
		ImGui::Checkbox(enabled.c_str(), &m.enabled);
		for (int i = 0; i < m.texIndices.size(); i++)
		{
			RenderMeshTextureInfo(m, meshIndex, i, status.texMan);
		}
		if (!m.texIndices.size())
			ImGui::Text("There are no textures attached to this mesh.");
	}
}

void RenderSelectionInfo(StatusManager& status)
{
	if (!showSelection)
		return;
	ImGui::Begin("Selection", &showSelection);
	ImGui::Text("Selection mode:");
	if (ImGui::RadioButton("Vertex", status.selectionMode == Mode_Vertex)) { status.selectionMode = Mode_Vertex; }
	if (ImGui::RadioButton("Edge", status.selectionMode == Mode_Edge)) { status.selectionMode = Mode_Edge; }
	if (ImGui::RadioButton("Face", status.selectionMode == Mode_Face)) { status.selectionMode = Mode_Face; }
	ImGui::Checkbox("Remove if double", &status.removeIfDouble);
	ImGui::End();
}

void RenderLightingInfo(StatusManager& status)
{
	if (!showLighting)
		return;
	ImGui::Begin("Lighting", &showLighting);
	ImGui::Text("Lighting mode:");
	if (ImGui::RadioButton("Flat", status.lightingMode == Mode_Flat)) { status.lightingMode = Mode_Flat; }
	if (ImGui::RadioButton("Smooth", status.lightingMode == Mode_Smooth)) { status.lightingMode = Mode_Smooth; }
	if (ImGui::RadioButton("No lighting", status.lightingMode == Mode_None)) { status.lightingMode = Mode_None; }
	ImGui::InputFloat3("Light position", glm::value_ptr(status.lightPos));
	ImGui::End();
}

void RenderCameraInfo(StatusManager& status)
{
	if (!showCamera)
		return;
	ImGui::Begin("Camera", &showCamera);
	if (ImGui::Button("Reset Camera Settings"))
		status.camera.Reset();
	ImGui::End();
}
