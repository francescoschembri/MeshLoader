
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
	//ImGui::StyleColorsClassic();

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
	status.model = Model(filesystem::path("./Animations/Capoeira/Capoeira.dae").string());
	// load animations
	status.AddAnimation("./Animations/Capoeira/Capoeira.dae");
	status.AddAnimation("./Animations/Flair/Flair.dae");
	status.AddAnimation("./Animations/Silly Dancing/Silly Dancing.dae");

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		status.UpdateDeltaTime();

		// input
		// -----
		status.ProcessInput(window);

		// animate the model
		if (!status.IsPaused()) status.animator.UpdateAnimation(status.deltaTime);

		// render
		// ------
		glClearColor(1.0f, 0.5f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


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
		ImGui::Render();
		status.model.Draw(ourShader, status.DrawFaces(), status.DrawLines());

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
		if(status.IsBaked())
			status.Picking();
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
	return float(width)/(float)height;
}
