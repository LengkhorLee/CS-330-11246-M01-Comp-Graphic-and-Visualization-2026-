#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

namespace
{
	const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones";

	GLFWwindow* g_Window = nullptr;

	SceneManager* g_SceneManager = nullptr;
	ShaderManager* g_ShaderManager = nullptr;
	ViewManager* g_ViewManager = nullptr;

	// Camera/input variables for milestone controls
	float g_CameraSpeed = 0.2f;
	bool g_PerspectiveView = true;
}

bool InitializeGLFW();
bool InitializeGLEW();
void ProcessInput(GLFWwindow* window);
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		std::cout << "Move camera forward\n";

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		std::cout << "Move camera backward\n";

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		std::cout << "Move camera left\n";

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		std::cout << "Move camera right\n";

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		std::cout << "Move camera down\n";

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		std::cout << "Move camera up\n";

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		g_PerspectiveView = true;
		std::cout << "Perspective view selected\n";
	}

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		g_PerspectiveView = false;
		std::cout << "Orthographic view selected\n";
	}
}

void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	g_CameraSpeed += static_cast<float>(yoffset) * 0.05f;

	if (g_CameraSpeed < 0.05f)
		g_CameraSpeed = 0.05f;

	std::cout << "Camera speed: " << g_CameraSpeed << std::endl;
}

int main(int argc, char* argv[])
{
	if (InitializeGLFW() == false)
	{
		return(EXIT_FAILURE);
	}

	g_ShaderManager = new ShaderManager();

	g_ViewManager = new ViewManager(g_ShaderManager);

	g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

	// Added for scroll wheel camera speed control
	glfwSetScrollCallback(g_Window, MouseScrollCallback);

	if (InitializeGLEW() == false)
	{
		return(EXIT_FAILURE);
	}

	g_ShaderManager->LoadShaders(
		"../../Utilities/shaders/vertexShader.glsl",
		"../../Utilities/shaders/fragmentShader.glsl");
	g_ShaderManager->use();

	g_SceneManager = new SceneManager(g_ShaderManager);
	g_SceneManager->PrepareScene();

	while (!glfwWindowShouldClose(g_Window))
	{
		// Added for keyboard input controls
		ProcessInput(g_Window);

		glEnable(GL_DEPTH_TEST);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		g_ViewManager->PrepareSceneView();

		g_SceneManager->RenderScene();

		glfwSwapBuffers(g_Window);

		glfwPollEvents();
	}

	if (NULL != g_SceneManager)
	{
		delete g_SceneManager;
		g_SceneManager = NULL;
	}
	if (NULL != g_ViewManager)
	{
		delete g_ViewManager;
		g_ViewManager = NULL;
	}
	if (NULL != g_ShaderManager)
	{
		delete g_ShaderManager;
		g_ShaderManager = NULL;
	}

	exit(EXIT_SUCCESS);
}

bool InitializeGLFW()
{
	glfwInit();

#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

	return(true);
}

bool InitializeGLEW()
{
	GLenum GLEWInitResult = GLEW_OK;

	GLEWInitResult = glewInit();
	if (GLEW_OK != GLEWInitResult)
	{
		std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
		return false;
	}

	std::cout << "INFO: OpenGL Successfully Initialized\n";
	std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

	return(true);
}