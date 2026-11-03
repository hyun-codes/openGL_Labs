#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct Camera
{
	glm::vec3 position{0.0f, 0.0f, 5.0f};
	glm::vec3 target{0.0f, 0.0f, 0.0f};
	glm::vec3 up{0.0f, 1.0f, 0.0f};
	float fovDegrees = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};

GLFWwindow* window = nullptr;
GLuint shaderProgram = 0;
Camera camera;
const int windowWidth = 800;
const int windowHeight = 600;
int framebufferWidth = windowWidth;
int framebufferHeight = windowHeight;

bool InitOpenGL();
std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
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
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	framebufferCallBack(window, framebufferWidth, framebufferHeight);
	glEnable(GL_DEPTH_TEST);
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (!shaderProgram)
	{
		Cleanup();
		return false;
	}
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
	glUseProgram(shaderProgram);

	// 도형 변환, 카메라, 투영 행렬을 각각 셰이더로 전달한다.
	glm::mat4 modelTransform(1.0f);
	glm::mat4 viewTransform = glm::lookAt(camera.position, camera.target, camera.up);
	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
	glm::mat4 projectionTransform = glm::perspective(
		glm::radians(camera.fovDegrees), aspect, camera.nearPlane, camera.farPlane);

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(modelTransform));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewTransform"), 1, GL_FALSE, glm::value_ptr(viewTransform));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectionTransform"), 1, GL_FALSE, glm::value_ptr(projectionTransform));
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 1.0f, 1.0f);

	// 이 아래에 VAO 연결과 glDrawArrays 등의 도형 그리기를 추가한다.
}

void framebufferCallBack(GLFWwindow* currentWindow, int width, int height)
{
	(void)currentWindow;
	framebufferWidth = std::max(width, 1);
	framebufferHeight = std::max(height, 1);
	glViewport(0, 0, width, height);
}

std::string ReadShaderFile(const char* filePath)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "셰이더 파일을 열 수 없습니다: " << filePath << '\n';
		return {};
	}
	std::ostringstream source;
	source << file.rdbuf();
	return source.str();
}

GLuint CompileShader(GLenum shaderType, const char* filePath)
{
	std::string source = ReadShaderFile(filePath);
	if (source.empty()) return 0;
	GLuint shader = glCreateShader(shaderType);
	const char* sourceText = source.c_str();
	glShaderSource(shader, 1, &sourceText, nullptr);
	glCompileShader(shader);
	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar errorLog[2048]{};
		glGetShaderInfoLog(shader, sizeof(errorLog), nullptr, errorLog);
		std::cerr << filePath << " 컴파일 실패:\n" << errorLog << '\n';
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
{
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexPath);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentPath);
	if (!vertexShader || !fragmentShader)
	{
		if (vertexShader) glDeleteShader(vertexShader);
		if (fragmentShader) glDeleteShader(fragmentShader);
		return 0;
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	GLint success = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar errorLog[2048]{};
		glGetProgramInfoLog(program, sizeof(errorLog), nullptr, errorLog);
		std::cerr << "셰이더 프로그램 연결 실패:\n" << errorLog << '\n';
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

void Cleanup()
{
	if (shaderProgram)
	{
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
	}
	if (window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}
