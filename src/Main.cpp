#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <FileBrowser/ImFileDialog.h>
#include <reskinner/GUI.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	SetupImGui(window);


	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("./Shaders/animated_model_loading.vs", "./Shaders/animated_model_loading.fs");
	// load models
	// -----------
	std::string modelPath = std::string("./Animations/Capoeira/Capoeira.dae");
	status.LoadModel(modelPath);
	status.AddAnimation(modelPath.c_str());

	// Our gui state
	float animTime = 0.0f;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// update the status before rendering based on user input
		if (status.animatedModel) {
			status.Update(window);
			animTime = status.animator.m_CurrentTime;
		}
		// render
		glClearColor(1.0f, 0.5f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		

		/*ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing()), ImGuiCond_Once);
		if (ImGui::Begin("Settings") && status.animatedModel)
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

		}*/


		if (status.animatedModel) {
			// don't forget to enable shader before setting uniforms
			ourShader.use();

			// model/view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(ZOOM), get_window_aspect_ratio(window), 0.1f, 100.0f);
			glm::mat4 view = status.camera.GetViewMatrix();
			glm::mat4 model = status.GetModelMatrix();
			ourShader.setMat4("model", model);
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", view);

			// pass bones matrices to the shader
			auto transforms = status.animator.GetFinalBoneMatrices();
			for (int i = 0; i < transforms.size(); ++i)
				ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

			// render the loaded model
			status.animatedModel->Draw(ourShader, status.DrawFaces(), status.DrawLines());
		}
		//status.fb.DrawTextureToScreen();


		RenderGUI(status);
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	CloseImGui();
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
