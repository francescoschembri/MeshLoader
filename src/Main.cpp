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
	glEnable(GL_BLEND);

	// build and compile shaders
	// -------------------------
	Shader ourShader("./Shaders/animated_model_loading.vs", "./Shaders/animated_model_loading.fs");
	Shader mouseShader("./Shaders/mouse_shader.vs", "./Shaders/mouse_shader.fs");
	// load models
	// -----------
	std::string modelPath = std::string("./Animations/Capoeira/Capoeira.dae");
	status.LoadModel(modelPath);
	status.AddAnimation(modelPath.c_str());

	// Our gui state
	float animTime = 0.0f;

	//mouse effect setup
	unsigned int mouseVAO, mouseVBO, mouseEBO;
	float screenVertices[] = {
		1.0f, 1.0f, 
		1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f, 1.0f
	};

	unsigned int screenIndices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &mouseVAO);
	glGenBuffers(1, &mouseVBO);
	glGenBuffers(1, &mouseEBO);
	glBindVertexArray(mouseVAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, mouseVBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices) * sizeof(float), &screenVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mouseEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenIndices), &screenIndices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glBindVertexArray(0);
	//end mouse effect setup

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

		//mouseShader.use();
		//mouseShader.setVec2("mousePos", status.mouseLastPos);
		//mouseShader.setVec2("resolution", status.width, status.height);
		//if (status.activeBrush)
		//	mouseShader.setFloat("radius", status.activeBrush.value()->radius);
		//else
		//	mouseShader.setFloat("radius", 1.0f);
		//glBindVertexArray(mouseVAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0);

		if (status.animatedModel) {
			// don't forget to enable shader before setting uniforms
			ourShader.use();

			// model/view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(ZOOM), status.aspect_ratio, 0.1f, 100.0f);
			glm::mat4 view = status.camera.GetViewMatrix();
			glm::mat4 model = status.animatedModel->GetModelMatrix();
			ourShader.setMat4("model", model);
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", view);

			// pass bones matrices to the shader
			auto transforms = status.animator.GetFinalBoneMatrices();
			for (int i = 0; i < transforms.size(); ++i)
				ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

			// render the loaded model
			status.animatedModel->Draw(ourShader, status.DrawFaces(), status.DrawLines());
			//if (status.bakedModel) {
			//	//std::cout << "print baked\n";
			//	status.bakedModel->Draw(ourShader, status.DrawFaces(), status.DrawLines());
			//}

		}


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
	status.aspect_ratio = (float)width / (float)height;
	status.width = float(width);
	status.height = float(height);
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

//float get_window_aspect_ratio(GLFWwindow* window)
//{
//	int width, height;
//	glfwGetWindowSize(window, &width, &height);
//	return float(width) / (float)height;
//}
