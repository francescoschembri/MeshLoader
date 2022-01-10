#include "Window.h"
#include "StatusManager.h"
#include "GUI.h"

#include <GLFW/glfw3.h>

#include <iostream>


int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = CreateWindow();
	//Sanity Check
	if (window == NULL) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	// create and attach a status to the window
	StatusManager status;
	glfwSetWindowUserPointer(window, (void*)&status);

	//create the projection matrix
	status.projection = glm::perspective(glm::radians(FOV), (float)SCREEN_INITIAL_WIDTH / (float)SCREEN_INITIAL_HEIGHT, NEAR_PLANE, FAR_PLANE);

	SetupImGui(window);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// TEST - load a model and an animation - remove in the final app
	std::string modelPath = std::string("./Animations/Capoeira/Capoeira.dae");
	status.LoadModel(modelPath);
	status.AddAnimation(modelPath.c_str());

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// glfw: poll IO events(keys pressed / released, mouse moved etc.)
		glfwPollEvents();

		status.Render();
		RenderGUI(status);
		// glfw: swap buffers
		glfwSwapBuffers(window);
	}

	// Clean memory
	//CloseImGui();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
