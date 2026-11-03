#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

GLFWwindow* window = nullptr;
const int windowWidth = 800;
const int windowHeight = 600;

bool InitOpenGL();
void DrawScene();
void Cleanup();
void framebufferCallBack(GLFWwindow* currentWindow, int width, int height);

int main()
{
	if (!InitOpenGL()) return -1;

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		DrawScene();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();
	return 0;
}

bool InitOpenGL()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Study", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "OpenGL 3.3 윈도우 생성 실패!\n";
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW 초기화 실패!\n";
		Cleanup();
		return false;
	}

	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
	std::cout << "ESC: exit\n";
	return true;
}

void DrawScene()
{
	// GLM 헤더 연결도 확인한다. 이후 이곳에 실습 그리기를 추가한다.
	glm::vec4 backgroundColor(0.2f, 0.3f, 0.5f, 1.0f);
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void framebufferCallBack(GLFWwindow* currentWindow, int width, int height)
{
	(void)currentWindow;
	glViewport(0, 0, width, height);
}

void Cleanup()
{
	if (window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}
