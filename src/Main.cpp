#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <FileBrowser/ImFileDialog.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <reskinner/Shader.h>
#include <reskinner/Model.h>
#include <reskinner/StatusManager.h>

#include <iostream>
#include <bitset>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
float get_window_aspect_ratio(GLFWwindow* window);

StatusManager status;

constexpr int SCREEN_INITIAL_WIDTH = 800;
constexpr int SCREEN_INITIAL_HEIGHT = 800;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCREEN_INITIAL_WIDTH, SCREEN_INITIAL_HEIGHT, "Animated Mesh Loader", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

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

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("./Shaders/animated_model_loading.vs", "./Shaders/animated_model_loading.fs");
	// load models
	// -----------
	//status.animatedModel = Model(filesystem::path("./Animations/Capoeira/Capoeira.dae").string());
	//status.currentModel = &status.animatedModel;
	// load animations
	//status.AddAnimation("./Animations/Capoeira/Capoeira.dae");
	//status.AddAnimation("./Animations/Flair/Flair.dae");
	//status.AddAnimation("./Animations/Silly Dancing/Silly Dancing.dae");

	// Our gui state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
	float animTime = 0.0f;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// update the status before rendering based on user input
		if (status.currentModel) {
			status.Update(window);
			animTime = status.animator.m_CurrentTime;
		}

		// render
		glClearColor(1.0f, 0.5f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show navbar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open...", "CTRL+O")) {
					ifd::FileDialog::Instance().Open("Mesh Open Dialog", "Open a mesh", "Mesh file (*.dae){.dae},.*");
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing()), ImGuiCond_Once);
		if (ImGui::Begin("Settings") && status.currentModel)
		{
			ImGui::Checkbox("Pause", &status.pause);
			ImGui::Checkbox("Wireframe", &status.wireframe);
			if (status.wireframe) {
				ImGui::Checkbox("Hidden line", &status.hiddenLine);
			}
			if (ImGui::Button("Bake"))
			{
				status.BakeModel();
			}
			if (ImGui::Button("Reset Camera"))
			{
				status.camera.Reset();
			}
			if (ImGui::Button("Switch Animation"))
			{
				status.SwitchAnimation();
			}
			float animDuration = status.animator.animations[status.animator.currentAnimationIndex].GetDuration();
			ImGui::InputFloat("Animation Time", &animTime);
			ImGui::SliderFloat("Animation Time", &animTime, 0.0f, animDuration);
			float deltaTime = animTime - status.animator.m_CurrentTime;
			if (deltaTime)
				status.animator.UpdateAnimation(deltaTime);
			if (ImGui::Button("Pennello 1"))
				status.pennello1 = true;

		}
		ImGui::End();

		if (ifd::FileDialog::Instance().IsDone("Mesh Open Dialog")) {
			if (ifd::FileDialog::Instance().HasResult()) {
				std::string res = ifd::FileDialog::Instance().GetResult().u8string();
				printf("OPEN[%s]\n", res.c_str());
				if (status.currentModel == nullptr) {
					status.animatedModel = Model(filesystem::path("./Animations/Capoeira/Capoeira.dae").string());
					status.currentModel = &status.animatedModel;
				}
				status.AddAnimation(res.c_str());
			}
			ifd::FileDialog::Instance().Close();
		}


		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}


		if (status.currentModel) {
			// don't forget to enable shader before setting uniforms
			ourShader.use();

			// model/view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(ZOOM), get_window_aspect_ratio(window), 0.1f, 100.0f);
			glm::mat4 view = status.camera.GetViewMatrix();
			glm::mat4 model = glm::mat4(1.0f);
			ourShader.setMat4("model", model);
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", view);

			// pass bones matrices to the shader
			auto transforms = status.animator.GetFinalBoneMatrices();
			for (int i = 0; i < transforms.size(); ++i)
				ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

			// render the loaded model
			status.currentModel->Draw(ourShader, status.DrawFaces(), status.DrawLines());
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (status.IsRotationLocked()) {
		float xoffset = xpos - status.mouseLastPos.x;
		float yoffset = status.mouseLastPos.y - ypos; // reversed since y-coordinates go from bottom to top

		status.camera.ProcessMouseMovement(xoffset, yoffset);
	}
	else {
		status.mouseLastPos.x = xpos;
		status.mouseLastPos.y = ypos;
		if (status.IsBaked())
			status.FacePicking();
	}
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	status.camera.ProcessMouseScroll(yoffset);
}

float get_window_aspect_ratio(GLFWwindow* window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	return float(width) / (float)height;
}
