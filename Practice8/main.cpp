#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

constexpr int QUADRANT_COUNT = 4;
constexpr float MIN_TRIANGLE_SIZE = 0.08f;
constexpr float MAX_TRIANGLE_SIZE = 0.28f;
constexpr float SIZE_STEP = 0.04f;

std::random_device rd;
std::mt19937 generator(rd());

struct Triangle
{
	float x;
	float y;
	float size;
	float R;
	float G;
	float B;
	float sizeDirection = 1.0f;
};

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};

std::array<Triangle, QUADRANT_COUNT> triangles;
std::vector<Vertex> vertices;

bool fillMode = true;

GLuint shaderProgram = 0;
GLuint VAO = 0;
GLuint VBO = 0;

float RandomFloat(float minValue, float maxValue);
std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void InitBuffer();
void ResetTriangles();
int GetQuadrant(float x, float y);
void SetTrianglePositionInQuadrant(Triangle& triangle, int quadrant, float x, float y);
void MakeRandomTriangle(int quadrant, float x, float y, bool useMousePosition);
void ScaleTriangle(int quadrant);
void AddVertex(float x, float y, float R, float G, float B);
void RebuildVertexData();
void GetMouseNDC(GLFWwindow* window, float& x, float& y);
void DrawScene();
void Cleanup();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, int button, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice8", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetMouseButtonCallback(window, mouseCallBack);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 800);

	shaderProgram = CreateShaderProgram(
		"shaders/vertex.glsl",
		"shaders/fragment.glsl");

	if (shaderProgram == 0)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	InitBuffer();
	ResetTriangles();

	while (!glfwWindowShouldClose(window))
	{
		DrawScene();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

float RandomFloat(float minValue, float maxValue)
{
	std::uniform_real_distribution<float> distribution(minValue, maxValue);
	return distribution(generator);
}

std::string ReadShaderFile(const char* filePath)
{
	std::ifstream shaderFile(filePath);
	if (!shaderFile.is_open())
	{
		std::cerr << "셰이더 파일을 열 수 없습니다: " << filePath << std::endl;
		return "";
	}
	std::stringstream shaderStream;
	shaderStream << shaderFile.rdbuf();
	return shaderStream.str();
}

GLuint CompileShader(GLenum shaderType, const char* filePath)
{
	std::string shaderSource = ReadShaderFile(filePath);
	if (shaderSource.empty())
		return 0;

	const char* source = shaderSource.c_str();
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char infoLog[1024] = {};
		glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
		std::cerr << filePath << " 컴파일 실패!\n" << infoLog << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
{
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexPath);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentPath);
	if (vertexShader == 0 || fragmentShader == 0)
		return 0;

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		char infoLog[1024] = {};
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		std::cerr << "셰이더 프로그램 링크 실패!\n" << infoLog << std::endl;
		glDeleteProgram(program);
		program = 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

void InitBuffer()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, x)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, R)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// 좌표가 어느 사분면인지 반환
int GetQuadrant(float x, float y)
{
	if (x < 0.0f && y >= 0.0f)
		return 0; // 2사분면
	if (x >= 0.0f && y >= 0.0f)
		return 1; // 1사분면
	if (x < 0.0f && y < 0.0f)
		return 2; // 3사분면
	return 3;     // 4사분면
}

void SetTrianglePositionInQuadrant(
	Triangle& triangle, int quadrant, float x, float y)
{
	float size = triangle.size;

	if (quadrant == 0 || quadrant == 2)
		triangle.x = std::clamp(x, -1.0f + size, -size);
	else
		triangle.x = std::clamp(x, size, 1.0f - size);

	if (quadrant == 0 || quadrant == 1)
		triangle.y = std::clamp(y, size, 1.0f - size);
	else
		triangle.y = std::clamp(y, -1.0f + size, -size);
}

// 지정한 사분면의 삼각형을 새 크기와 색상으로 교체
void MakeRandomTriangle(int quadrant, float x, float y, bool useMousePosition)
{
	Triangle& triangle = triangles[quadrant];
	triangle.size = RandomFloat(MIN_TRIANGLE_SIZE, MAX_TRIANGLE_SIZE);
	triangle.R = RandomFloat(0.10f, 0.90f);
	triangle.G = RandomFloat(0.10f, 0.90f);
	triangle.B = RandomFloat(0.10f, 0.90f);
	triangle.sizeDirection = 1.0f;

	if (!useMousePosition)
	{
		if (quadrant == 0 || quadrant == 2)
			x = RandomFloat(-1.0f + triangle.size, -triangle.size);
		else
			x = RandomFloat(triangle.size, 1.0f - triangle.size);

		if (quadrant == 0 || quadrant == 1)
			y = RandomFloat(triangle.size, 1.0f - triangle.size);
		else
			y = RandomFloat(-1.0f + triangle.size, -triangle.size);
	}

	SetTrianglePositionInQuadrant(triangle, quadrant, x, y);
}

void ResetTriangles()
{
	for (int quadrant = 0; quadrant < QUADRANT_COUNT; ++quadrant)
		MakeRandomTriangle(quadrant, 0.0f, 0.0f, false);

	RebuildVertexData();
}

// 오른쪽 클릭한 사분면의 삼각형 확대/축소
void ScaleTriangle(int quadrant)
{
	Triangle& triangle = triangles[quadrant];
	triangle.size += SIZE_STEP * triangle.sizeDirection;

	if (triangle.size >= MAX_TRIANGLE_SIZE)
	{
		triangle.size = MAX_TRIANGLE_SIZE;
		triangle.sizeDirection = -1.0f;
	}
	else if (triangle.size <= MIN_TRIANGLE_SIZE)
	{
		triangle.size = MIN_TRIANGLE_SIZE;
		triangle.sizeDirection = 1.0f;
	}

	SetTrianglePositionInQuadrant(
		triangle, quadrant, triangle.x, triangle.y);
	RebuildVertexData();
}

void AddVertex(float x, float y, float R, float G, float B)
{
	vertices.push_back({ x, y, 0.0f, R, G, B });
}

// 축과 네 개의 이등변삼각형을 VBO 데이터로 변환
void RebuildVertexData()
{
	vertices.clear();

	// x축과 y축: 정점 4개
	AddVertex(-1.0f, 0.0f, 0.20f, 0.45f, 0.80f);
	AddVertex(1.0f, 0.0f, 0.20f, 0.45f, 0.80f);
	AddVertex(0.0f, -1.0f, 0.20f, 0.45f, 0.80f);
	AddVertex(0.0f, 1.0f, 0.20f, 0.45f, 0.80f);

	for (const Triangle& triangle : triangles)
	{
		AddVertex(
			triangle.x, triangle.y + triangle.size,
			triangle.R, triangle.G, triangle.B);
		AddVertex(
			triangle.x - triangle.size, triangle.y - triangle.size,
			triangle.R, triangle.G, triangle.B);
		AddVertex(
			triangle.x + triangle.size, triangle.y - triangle.size,
			triangle.R, triangle.G, triangle.B);
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(Vertex),
		vertices.data(),
		GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GetMouseNDC(GLFWwindow* window, float& x, float& y)
{
	double mouseX = 0.0;
	double mouseY = 0.0;
	int windowWidth = 1;
	int windowHeight = 1;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	x = static_cast<float>(mouseX / windowWidth * 2.0 - 1.0);
	y = static_cast<float>(1.0 - mouseY / windowHeight * 2.0);
}

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);

	// 사분면을 나누는 x축과 y축
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, 4);

	// 축 다음에 저장된 삼각형 정점 12개
	glPolygonMode(
		GL_FRONT_AND_BACK,
		fillMode ? GL_FILL : GL_LINE);
	glDrawArrays(GL_TRIANGLES, 4, 12);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(0);
	glUseProgram(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_A)
		fillMode = true;
	else if (key == GLFW_KEY_B)
		fillMode = false;
	else if (key == GLFW_KEY_C)
		ResetTriangles();
	else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;
	if (action != GLFW_PRESS)
		return;

	float mouseX = 0.0f;
	float mouseY = 0.0f;
	GetMouseNDC(window, mouseX, mouseY);
	int quadrant = GetQuadrant(mouseX, mouseY);

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		MakeRandomTriangle(quadrant, mouseX, mouseY, true);
		RebuildVertexData();
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		ScaleTriangle(quadrant);
	}
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

void Cleanup()
{
	if (VBO != 0)
		glDeleteBuffers(1, &VBO);
	if (VAO != 0)
		glDeleteVertexArrays(1, &VAO);
	if (shaderProgram != 0)
		glDeleteProgram(shaderProgram);
}
