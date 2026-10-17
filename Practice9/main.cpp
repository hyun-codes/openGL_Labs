#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

constexpr int TRIANGLE_COUNT = 4;
constexpr float MIN_TRIANGLE_SIZE = 0.08f;
constexpr float MAX_TRIANGLE_SIZE = 0.22f;
constexpr float SIZE_STEP = 0.03f;
constexpr float PI = 3.1415926535f;
constexpr int SPIRAL_PATH_SEGMENTS = 160;

std::random_device rd;
std::mt19937 generator(rd());

enum class MovementMode
{
	Bounce,
	HorizontalZigzag,
	VerticalZigzag,
	Spiral
};

struct Triangle
{
	float x;
	float y;
	float size;
	float R;
	float G;
	float B;
	float sizeDirection = 1.0f;

	float directionX = 1.0f;
	float directionY = 1.0f;
	float rowDirectionY = -1.0f;
	float speed = 0.25f;

	float spiralAngle = 0.0f;
	float spiralRadius = 0.0f;
	float angularSpeed = 1.0f;
	float radialSpeed = 0.08f;
};

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};

std::array<Triangle, TRIANGLE_COUNT> triangles;
std::vector<Vertex> vertices;
GLsizei spiralPathVertexCount = 0;

MovementMode movementMode = MovementMode::Bounce;
bool fillMode = true;
double lastFrameTime = 0.0;

GLuint shaderProgram = 0;
GLuint VAO = 0;
GLuint VBO = 0;

float RandomFloat(float minValue, float maxValue);
int RandomSign();
std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void InitBuffer();
int GetQuadrant(float x, float y);
void SetTrianglePositionInQuadrant(Triangle& triangle, int quadrant, float x, float y);
void MakeRandomTriangle(int quadrant, float x, float y, bool useMousePosition);
void ResetTriangles();
void ScaleTriangle(int quadrant);
void SetMovementMode(MovementMode newMode);
void UpdateBounce(Triangle& triangle, float deltaTime);
void UpdateHorizontalZigzag(Triangle& triangle, float deltaTime);
void UpdateVerticalZigzag(Triangle& triangle, float deltaTime);
void UpdateSpiral(Triangle& triangle, float deltaTime);
void UpdateAnimation(float deltaTime);
void AddVertex(float x, float y, float R, float G, float B);
float GetTriangleDirectionAngle(const Triangle& triangle);
void AddRotatedTriangleVertex(
	const Triangle& triangle, float localX, float localY, float angle);
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

	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice9", nullptr, nullptr);
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
	lastFrameTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
		deltaTime = std::min(deltaTime, 0.05f);

		UpdateAnimation(deltaTime);
		RebuildVertexData();
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

int RandomSign()
{
	return RandomFloat(0.0f, 1.0f) < 0.5f ? -1 : 1;
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

int GetQuadrant(float x, float y)
{
	if (x < 0.0f && y >= 0.0f)
		return 0;
	if (x >= 0.0f && y >= 0.0f)
		return 1;
	if (x < 0.0f && y < 0.0f)
		return 2;
	return 3;
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

void MakeRandomTriangle(int quadrant, float x, float y, bool useMousePosition)
{
	Triangle& triangle = triangles[quadrant];
	triangle.size = RandomFloat(MIN_TRIANGLE_SIZE, MAX_TRIANGLE_SIZE);
	triangle.R = RandomFloat(0.10f, 0.90f);
	triangle.G = RandomFloat(0.10f, 0.90f);
	triangle.B = RandomFloat(0.10f, 0.90f);
	triangle.sizeDirection = 1.0f;

	float directionAngle = RandomFloat(0.0f, 2.0f * PI);
	triangle.directionX = std::cos(directionAngle);
	triangle.directionY = std::sin(directionAngle);
	triangle.rowDirectionY = static_cast<float>(RandomSign());
	triangle.speed = RandomFloat(0.18f, 0.38f);
	triangle.angularSpeed =
		RandomFloat(0.8f, 1.8f) * static_cast<float>(RandomSign());
	triangle.radialSpeed = RandomFloat(0.05f, 0.11f);

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
	triangle.spiralAngle = std::atan2(triangle.y, triangle.x);
	triangle.spiralRadius =
		std::sqrt(triangle.x * triangle.x + triangle.y * triangle.y);
}

void ResetTriangles()
{
	for (int quadrant = 0; quadrant < TRIANGLE_COUNT; ++quadrant)
		MakeRandomTriangle(quadrant, 0.0f, 0.0f, false);

	if (movementMode == MovementMode::Spiral)
		SetMovementMode(MovementMode::Spiral);

	RebuildVertexData();
}

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
}

void SetMovementMode(MovementMode newMode)
{
	movementMode = newMode;

	if (movementMode == MovementMode::HorizontalZigzag)
	{
		for (Triangle& triangle : triangles)
		{
			triangle.directionX =
				triangle.directionX < 0.0f ? -1.0f : 1.0f;
		}
	}
	else if (movementMode == MovementMode::VerticalZigzag)
	{
		for (Triangle& triangle : triangles)
		{
			triangle.directionX =
				triangle.directionX < 0.0f ? -1.0f : 1.0f;
			triangle.directionY =
				triangle.directionY < 0.0f ? -1.0f : 1.0f;
		}
	}
	else if (movementMode == MovementMode::Spiral)
	{
		for (Triangle& triangle : triangles)
		{
			// 현재 위치를 극좌표로 바꾸므로 모드 전환 시 위치가 튀지 않는다.
			triangle.spiralAngle = std::atan2(triangle.y, triangle.x);
			triangle.spiralRadius =
				std::sqrt(triangle.x * triangle.x + triangle.y * triangle.y);

			float maxRadius = std::max(0.10f, 0.90f - triangle.size);
			if (triangle.spiralRadius >= maxRadius)
				triangle.radialSpeed = -std::abs(triangle.radialSpeed);
			else
				triangle.radialSpeed = std::abs(triangle.radialSpeed);
		}
	}
}

void UpdateBounce(Triangle& triangle, float deltaTime)
{
	float limit = 1.0f - triangle.size;
	triangle.x += triangle.directionX * triangle.speed * deltaTime;
	triangle.y += triangle.directionY * triangle.speed * deltaTime;

	if (triangle.x >= limit || triangle.x <= -limit)
	{
		triangle.x = std::clamp(triangle.x, -limit, limit);
		triangle.directionX *= -1.0f;
	}
	if (triangle.y >= limit || triangle.y <= -limit)
	{
		triangle.y = std::clamp(triangle.y, -limit, limit);
		triangle.directionY *= -1.0f;
	}
}

// 좌우로 이동하다 가장자리에서 한 줄 이동하고 반대 방향으로 진행
void UpdateHorizontalZigzag(Triangle& triangle, float deltaTime)
{
	float limit = 1.0f - triangle.size;
	triangle.x += triangle.directionX * triangle.speed * deltaTime;

	if (triangle.x >= limit || triangle.x <= -limit)
	{
		triangle.x = std::clamp(triangle.x, -limit, limit);
		triangle.directionX *= -1.0f;
		triangle.y += triangle.rowDirectionY * triangle.size * 1.5f;

		if (triangle.y >= limit || triangle.y <= -limit)
		{
			triangle.y = std::clamp(triangle.y, -limit, limit);
			triangle.rowDirectionY *= -1.0f;
		}
	}
}

// 위아래 경계에서 뾰족하게 방향을 바꾸며 가로로 진행
void UpdateVerticalZigzag(Triangle& triangle, float deltaTime)
{
	float limit = 1.0f - triangle.size;
	triangle.x += triangle.directionX * triangle.speed * 0.65f * deltaTime;
	triangle.y += triangle.directionY * triangle.speed * deltaTime;

	if (triangle.y >= limit || triangle.y <= -limit)
	{
		triangle.y = std::clamp(triangle.y, -limit, limit);
		triangle.directionY *= -1.0f;
	}
	if (triangle.x >= limit || triangle.x <= -limit)
	{
		triangle.x = std::clamp(triangle.x, -limit, limit);
		triangle.directionX *= -1.0f;
	}
}

void UpdateSpiral(Triangle& triangle, float deltaTime)
{
	float maxRadius = std::max(0.10f, 0.90f - triangle.size);
	float minRadius = 0.05f;

	triangle.spiralAngle += triangle.angularSpeed * deltaTime;
	triangle.spiralRadius += triangle.radialSpeed * deltaTime;

	if (triangle.spiralRadius >= maxRadius)
	{
		triangle.spiralRadius = maxRadius;
		triangle.radialSpeed = -std::abs(triangle.radialSpeed);
	}
	else if (triangle.spiralRadius <= minRadius)
	{
		triangle.spiralRadius = minRadius;
		triangle.radialSpeed = std::abs(triangle.radialSpeed);
	}

	triangle.x = std::cos(triangle.spiralAngle) * triangle.spiralRadius;
	triangle.y = std::sin(triangle.spiralAngle) * triangle.spiralRadius;
}

void UpdateAnimation(float deltaTime)
{
	for (Triangle& triangle : triangles)
	{
		if (movementMode == MovementMode::Bounce)
			UpdateBounce(triangle, deltaTime);
		else if (movementMode == MovementMode::HorizontalZigzag)
			UpdateHorizontalZigzag(triangle, deltaTime);
		else if (movementMode == MovementMode::VerticalZigzag)
			UpdateVerticalZigzag(triangle, deltaTime);
		else
			UpdateSpiral(triangle, deltaTime);
	}
}

void AddVertex(float x, float y, float R, float G, float B)
{
	vertices.push_back({ x, y, 0.0f, R, G, B });
}

// 이동 방향을 가장 가까운 상하좌우 방향으로 바꿔 삼각형 각도를 구한다.
float GetTriangleDirectionAngle(const Triangle& triangle)
{
	if (movementMode == MovementMode::Spiral)
		return 0.0f;

	float moveX = triangle.directionX;
	float moveY = triangle.directionY;
	if (movementMode == MovementMode::HorizontalZigzag)
		moveY = 0.0f;
	else if (movementMode == MovementMode::VerticalZigzag)
		moveX *= 0.65f;

	if (std::abs(moveX) > std::abs(moveY))
		return moveX > 0.0f ? -PI / 2.0f : PI / 2.0f;
	return moveY >= 0.0f ? 0.0f : PI;
}

void AddRotatedTriangleVertex(
	const Triangle& triangle, float localX, float localY, float angle)
{
	float cosine = std::cos(angle);
	float sine = std::sin(angle);
	float x = triangle.x + localX * cosine - localY * sine;
	float y = triangle.y + localX * sine + localY * cosine;
	AddVertex(x, y, triangle.R, triangle.G, triangle.B);
}

void RebuildVertexData()
{
	vertices.clear();
	spiralPathVertexCount = 0;

	// 실습 8과 같은 사분면 축
	AddVertex(-1.0f, 0.0f, 0.20f, 0.45f, 0.80f);
	AddVertex(1.0f, 0.0f, 0.20f, 0.45f, 0.80f);
	AddVertex(0.0f, -1.0f, 0.20f, 0.45f, 0.80f);
	AddVertex(0.0f, 1.0f, 0.20f, 0.45f, 0.80f);

	for (const Triangle& triangle : triangles)
	{
		float angle = GetTriangleDirectionAngle(triangle);
		AddRotatedTriangleVertex(triangle, 0.0f, triangle.size, angle);
		AddRotatedTriangleVertex(triangle, -triangle.size, -triangle.size, angle);
		AddRotatedTriangleVertex(triangle, triangle.size, -triangle.size, angle);
	}

	// 4번 모드에서 삼각형이 이동할 검은색 나선 경로를 추가한다.
	if (movementMode == MovementMode::Spiral)
	{
		for (int i = 0; i <= SPIRAL_PATH_SEGMENTS; ++i)
		{
			float ratio = static_cast<float>(i) / SPIRAL_PATH_SEGMENTS;
			float angle = ratio * 6.0f * PI;
			float radius = 0.05f + ratio * 0.78f;
			AddVertex(
				std::cos(angle) * radius,
				std::sin(angle) * radius,
				0.0f, 0.0f, 0.0f);
			++spiralPathVertexCount;
		}
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

	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, 4);

	glPolygonMode(
		GL_FRONT_AND_BACK,
		fillMode ? GL_FILL : GL_LINE);
	glDrawArrays(GL_TRIANGLES, 4, 12);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (movementMode == MovementMode::Spiral)
	{
		glLineWidth(2.0f);
		glDrawArrays(GL_LINE_STRIP, 16, spiralPathVertexCount);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_1)
		SetMovementMode(MovementMode::Bounce);
	else if (key == GLFW_KEY_2)
		SetMovementMode(MovementMode::HorizontalZigzag);
	else if (key == GLFW_KEY_3)
		SetMovementMode(MovementMode::VerticalZigzag);
	else if (key == GLFW_KEY_4)
		SetMovementMode(MovementMode::Spiral);
	else if (key == GLFW_KEY_A)
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
		if (movementMode == MovementMode::Spiral)
			SetMovementMode(MovementMode::Spiral);
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
