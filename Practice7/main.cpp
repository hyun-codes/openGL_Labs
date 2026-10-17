#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

constexpr int MAX_SHAPES = 50;
constexpr float MOVE_STEP = 0.05f;
constexpr float PI = 3.1415926535f;

std::random_device rd;
std::mt19937 generator(rd());

enum class ShapeType
{
	Point,
	Line,
	Triangle,
	Rectangle
};

struct Shape
{
	ShapeType type;
	float x;
	float y;
	float size;
	float angle;
	float R;
	float G;
	float B;
	bool selected = false;
};

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};

struct DrawCommand
{
	GLenum mode;
	GLint first;
	GLsizei count;
	GLfloat pointSize;
	GLfloat lineWidth;
};

std::vector<Shape> shapes;
std::vector<Vertex> vertices;
std::vector<DrawCommand> drawCommands;

GLuint shaderProgram = 0;
GLuint VAO = 0;
GLuint VBO = 0;

float RandomFloat(float minValue, float maxValue);
std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void InitBuffer();
void RebuildVertexData();
void AddShape(ShapeType type);
void AddVertex(float x, float y, const Shape& shape);
void AppendShapeVertices(const Shape& shape);
void MoveSelected(float dx, float dy);
void MoveAll(float dx, float dy);
bool ContainsPoint(const Shape& shape, float x, float y);
void SelectShape(float x, float y);
void GetMouseNDC(GLFWwindow* window, float& x, float& y);
void DrawScene();
void Cleanup();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, int button, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	// GLFW 초기화
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	// OpenGL 3.3 Core Profile 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice7", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetMouseButtonCallback(window, mouseCallBack);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);

	// GLEW 초기화
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 800);

	// 셰이더 프로그램 생성
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

	// 메인 루프
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

// 셰이더 파일 읽기
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

// VAO, VBO와 정점 속성 설정
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

// 도형 한 개 추가
void AddShape(ShapeType type)
{
	if (shapes.size() >= MAX_SHAPES)
		return;

	Shape shape;
	shape.type = type;
	shape.angle = RandomFloat(0.0f, 2.0f * PI);
	shape.R = RandomFloat(0.10f, 0.90f);
	shape.G = RandomFloat(0.10f, 0.90f);
	shape.B = RandomFloat(0.10f, 0.90f);

	if (type == ShapeType::Point)
		shape.size = 0.025f;
	else if (type == ShapeType::Line)
		shape.size = RandomFloat(0.10f, 0.25f);
	else
		shape.size = RandomFloat(0.06f, 0.16f);

	float margin = std::max(shape.size, 0.04f);
	shape.x = RandomFloat(-1.0f + margin, 1.0f - margin);
	shape.y = RandomFloat(-1.0f + margin, 1.0f - margin);
	shapes.push_back(shape);
	RebuildVertexData();
}

//선택할시에
void AddVertex(float x, float y, const Shape& shape)
{
	float R = shape.selected ? 1.0f : shape.R;
	float G = shape.selected ? 1.0f : shape.G;
	float B = shape.selected ? 0.0f : shape.B;
	vertices.push_back({ x, y, 0.0f, R, G, B });
}

// 논리적인 Shape를 실제 VBO 정점으로 변환
void AppendShapeVertices(const Shape& shape)
{
	GLint first = static_cast<GLint>(vertices.size());

	if (shape.type == ShapeType::Point)
	{
		AddVertex(shape.x, shape.y, shape);
		drawCommands.push_back({
			GL_POINTS, first, 1,
			shape.selected ? 12.0f : 7.0f, 1.0f });
	}
	else if (shape.type == ShapeType::Line)
	{
		float dx = std::cos(shape.angle) * shape.size;
		float dy = std::sin(shape.angle) * shape.size;
		AddVertex(shape.x - dx, shape.y - dy, shape);
		AddVertex(shape.x + dx, shape.y + dy, shape);
		drawCommands.push_back({
			GL_LINES, first, 2, 1.0f,
			shape.selected ? 5.0f : 3.0f });
	}
	else if (shape.type == ShapeType::Triangle)
	{
		AddVertex(shape.x, shape.y + shape.size, shape);
		AddVertex(shape.x - shape.size, shape.y - shape.size, shape);
		AddVertex(shape.x + shape.size, shape.y - shape.size, shape);
		drawCommands.push_back({ GL_TRIANGLES, first, 3, 1.0f, 1.0f });
	}
	else
	{
		float left = shape.x - shape.size;
		float right = shape.x + shape.size;
		float bottom = shape.y - shape.size;
		float top = shape.y + shape.size;

		// 사각형은 삼각형 2개, 즉 정점 6개로 구성
		AddVertex(left, bottom, shape);
		AddVertex(right, bottom, shape);
		AddVertex(right, top, shape);
		AddVertex(left, bottom, shape);
		AddVertex(right, top, shape);
		AddVertex(left, top, shape);
		drawCommands.push_back({ GL_TRIANGLES, first, 6, 1.0f, 1.0f });
	}
}

// 현재 도형 목록으로 VBO를 다시 구성
void RebuildVertexData()
{
	vertices.clear();
	drawCommands.clear();

	for (const Shape& shape : shapes)
		AppendShapeVertices(shape);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(Vertex),
		vertices.empty() ? nullptr : vertices.data(),
		GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MoveSelected(float dx, float dy)
{
	for (Shape& shape : shapes)
	{
		if (!shape.selected)
			continue;

		float margin = std::max(shape.size, 0.04f);
		shape.x = std::clamp(shape.x + dx, -1.0f + margin, 1.0f - margin);
		shape.y = std::clamp(shape.y + dy, -1.0f + margin, 1.0f - margin);
	}
	RebuildVertexData();
}

void MoveAll(float dx, float dy)
{
	for (Shape& shape : shapes)
	{
		shape.selected = true;
		float margin = std::max(shape.size, 0.04f);
		shape.x = std::clamp(shape.x + dx, -1.0f + margin, 1.0f - margin);
		shape.y = std::clamp(shape.y + dy, -1.0f + margin, 1.0f - margin);
	}
	RebuildVertexData();
}

// 마우스 선택을 위한 도형 충돌 검사
bool ContainsPoint(const Shape& shape, float x, float y)
{
	float localX = x - shape.x;
	float localY = y - shape.y;

	if (shape.type == ShapeType::Point)
		return std::sqrt(localX * localX + localY * localY) <= 0.06f;

	if (shape.type == ShapeType::Rectangle)
		return std::abs(localX) <= shape.size && std::abs(localY) <= shape.size;

	if (shape.type == ShapeType::Triangle)
	{
		float x1 = 0.0f;
		float y1 = shape.size;
		float x2 = -shape.size;
		float y2 = -shape.size;
		float x3 = shape.size;
		float y3 = -shape.size;

		float denominator =
			(y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
		float a = ((y2 - y3) * (localX - x3) +
			(x3 - x2) * (localY - y3)) / denominator;
		float b = ((y3 - y1) * (localX - x3) +
			(x1 - x3) * (localY - y3)) / denominator;
		float c = 1.0f - a - b;
		return a >= 0.0f && b >= 0.0f && c >= 0.0f;
	}

	float dx = std::cos(shape.angle) * shape.size;
	float dy = std::sin(shape.angle) * shape.size;
	float lengthSquared = 4.0f * shape.size * shape.size;
	float t = ((x - (shape.x - dx)) * (2.0f * dx) +
		(y - (shape.y - dy)) * (2.0f * dy)) / lengthSquared;
	t = std::clamp(t, 0.0f, 1.0f);
	float nearestX = shape.x - dx + t * 2.0f * dx;
	float nearestY = shape.y - dy + t * 2.0f * dy;
	float distanceX = x - nearestX;
	float distanceY = y - nearestY;
	return distanceX * distanceX + distanceY * distanceY <= 0.0025f;
}

//클릭한 위치에 도형을 찾는다.
void SelectShape(float x, float y)
{
	int selectedIndex = -1;
	for (int i = static_cast<int>(shapes.size()) - 1; i >= 0; --i)
	{
		if (ContainsPoint(shapes[i], x, y))
		{
			selectedIndex = i;
			break;
		}
	}

	//선택한 도형 뺴고 전부 선택이 false가 됨.
	for (int i = 0; i < static_cast<int>(shapes.size()); ++i)
		shapes[i].selected = i == selectedIndex;

	RebuildVertexData();
}

// 마우스 좌표를 마우스 좌표에서 opengl좌표로 변환.
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

	for (const DrawCommand& command : drawCommands)
	{
		glPointSize(command.pointSize);
		glLineWidth(command.lineWidth);
		glDrawArrays(command.mode, command.first, command.count);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_P)
			AddShape(ShapeType::Point);
		else if (key == GLFW_KEY_E)
			AddShape(ShapeType::Line);
		else if (key == GLFW_KEY_T)
			AddShape(ShapeType::Triangle);
		else if (key == GLFW_KEY_R)
			AddShape(ShapeType::Rectangle);
		else if (key == GLFW_KEY_C)
		{
			shapes.clear();
			RebuildVertexData();
		}
		else if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if (key == GLFW_KEY_W)
		MoveSelected(0.0f, MOVE_STEP);
	else if (key == GLFW_KEY_A)
		MoveSelected(-MOVE_STEP, 0.0f);
	else if (key == GLFW_KEY_S)
		MoveSelected(0.0f, -MOVE_STEP);
	else if (key == GLFW_KEY_D)
		MoveSelected(MOVE_STEP, 0.0f);
	else if (key == GLFW_KEY_I)
		MoveSelected(-MOVE_STEP, MOVE_STEP);
	else if (key == GLFW_KEY_J)
		MoveSelected(MOVE_STEP, MOVE_STEP);
	else if (key == GLFW_KEY_K)
		MoveSelected(-MOVE_STEP, -MOVE_STEP);
	else if (key == GLFW_KEY_L)
		MoveSelected(MOVE_STEP, -MOVE_STEP);
	else if (key == GLFW_KEY_1)
		MoveAll(-MOVE_STEP, 0.0f);
	else if (key == GLFW_KEY_2)
		MoveAll(MOVE_STEP, 0.0f);
	else if (key == GLFW_KEY_3)
		MoveAll(0.0f, MOVE_STEP);
	else if (key == GLFW_KEY_4)
		MoveAll(0.0f, -MOVE_STEP);
}

void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		float mouseX = 0.0f;
		float mouseY = 0.0f;
		GetMouseNDC(window, mouseX, mouseY);
		SelectShape(mouseX, mouseY);
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
