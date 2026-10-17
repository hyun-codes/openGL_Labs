#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};

struct Board
{
	GLuint VAO = 0;
	GLuint VBO = 0;
	std::vector<Vertex> vertices;

	float laneBottom = -0.82f;
	float laneTop = 0.82f;
	float leftLaneLeft = -0.65f;
	float leftLaneRight = -0.35f;
	float rightLaneLeft = -0.20f;
	float rightLaneRight = 0.10f;
	float meetingBottom = -0.13f;
	float meetingTop = 0.13f;
};

struct Square
{
	float x = 0.0f;
	float y = 0.0f;
	float halfSize = 0.055f;

	float R = 1.0f;
	float G = 0.0f;
	float B = 0.0f;

	float verticalDirection = 1.0f;
	float verticalSpeed = 0.4f;
	float targetX = 0.0f;
	float targetY = 0.0f;
};

struct SquareRenderer
{
	GLuint VAO = 0;
	GLuint VBO = 0;
	std::vector<Vertex> vertices;
};

enum class AnimationState
{
	VerticalMoving,
	Aligning,
	MovingRight
};

GLuint shaderProgram = 0;

Board board;
Square leftSquare;
Square rightSquare;
SquareRenderer squareRenderer;
std::vector<Square> stackedSquares;

AnimationState animationState = AnimationState::VerticalMoving;
double lastFrameTime = 0.0;

std::mt19937 randomGenerator(std::random_device{}());

std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);

void InitBuffer();
void InitBoardBuffer();
void InitSquareBuffer();
void InitBoardVertices();
void UpdateSquareVertices();
void UpdateSquareBuffer();

void AppendLine(
	std::vector<Vertex>& vertices,
	float x1, float y1,
	float x2, float y2,
	float R, float G, float B);
void AppendRectangleOutline(
	std::vector<Vertex>& vertices,
	float left, float right,
	float bottom, float top,
	float R, float G, float B);
void AppendSquareVertices(
	std::vector<Vertex>& vertices,
	const Square& square);

void ResetGame();
void SpawnNewSquares();
void SetStackTarget(Square& square, int stackIndex);
void UpdateAnimation();
void UpdateVerticalSquare(Square& square, float deltaTime);
void UpdateAligning(float deltaTime);
void UpdateMovingRight(float deltaTime);
bool IsInsideMeetingSpace(const Square& square);
bool MoveSquareToTarget(Square& square, float deltaTime);
float MoveTowards(float current, float target, float maxDelta);
float RandomFloat(float minValue, float maxValue);

void DrawScene();
void DrawBoard();
void DrawSquares();
void Cleanup();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
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
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice12", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallBack);
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
	lastFrameTime = glfwGetTime();

	// 메인 루프
	while (!glfwWindowShouldClose(window))
	{
		UpdateAnimation();
		DrawScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
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
	{
		if (vertexShader != 0)
			glDeleteShader(vertexShader);
		if (fragmentShader != 0)
			glDeleteShader(fragmentShader);
		return 0;
	}

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
	InitBoardBuffer();
	ResetGame();
	InitSquareBuffer();
}

void InitBoardBuffer()
{
	InitBoardVertices();

	glGenVertexArrays(1, &board.VAO);
	glBindVertexArray(board.VAO);

	glGenBuffers(1, &board.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, board.VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		board.vertices.size() * sizeof(Vertex),
		board.vertices.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, x))
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, R))
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InitSquareBuffer()
{
	glGenVertexArrays(1, &squareRenderer.VAO);
	glBindVertexArray(squareRenderer.VAO);

	glGenBuffers(1, &squareRenderer.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, squareRenderer.VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		squareRenderer.vertices.size() * sizeof(Vertex),
		squareRenderer.vertices.data(),
		GL_DYNAMIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, x))
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, R))
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InitBoardVertices()
{
	board.vertices.clear();

	const float lineR = 0.20f;
	const float lineG = 0.45f;
	const float lineB = 0.75f;

	AppendRectangleOutline(
		board.vertices,
		board.leftLaneLeft,
		board.leftLaneRight,
		board.laneBottom,
		board.laneTop,
		lineR, lineG, lineB
	);

	AppendRectangleOutline(
		board.vertices,
		board.rightLaneLeft,
		board.rightLaneRight,
		board.laneBottom,
		board.laneTop,
		lineR, lineG, lineB
	);

	// 두 사각형이 함께 들어와야 하는 공간을 점선으로 표시한다.
	const float meetingLeft = board.leftLaneLeft - 0.12f;
	const float meetingRight = board.rightLaneRight + 0.12f;
	const float dashLength = 0.055f;
	const float dashGap = 0.035f;

	for (float x = meetingLeft; x < meetingRight; x += dashLength + dashGap)
	{
		float endX = x + dashLength;
		if (endX > meetingRight)
			endX = meetingRight;

		AppendLine(
			board.vertices,
			x, board.meetingBottom,
			endX, board.meetingBottom,
			lineR, lineG, lineB
		);
		AppendLine(
			board.vertices,
			x, board.meetingTop,
			endX, board.meetingTop,
			lineR, lineG, lineB
		);
	}

	AppendLine(
		board.vertices,
		meetingLeft, board.meetingBottom,
		meetingLeft, board.meetingTop,
		lineR, lineG, lineB
	);
	AppendLine(
		board.vertices,
		meetingRight, board.meetingBottom,
		meetingRight, board.meetingTop,
		lineR, lineG, lineB
	);
}

void AppendLine(
	std::vector<Vertex>& vertices,
	float x1, float y1,
	float x2, float y2,
	float R, float G, float B)
{
	vertices.push_back({ x1, y1, 0.0f, R, G, B });
	vertices.push_back({ x2, y2, 0.0f, R, G, B });
}

void AppendRectangleOutline(
	std::vector<Vertex>& vertices,
	float left, float right,
	float bottom, float top,
	float R, float G, float B)
{
	AppendLine(vertices, left, bottom, right, bottom, R, G, B);
	AppendLine(vertices, right, bottom, right, top, R, G, B);
	AppendLine(vertices, right, top, left, top, R, G, B);
	AppendLine(vertices, left, top, left, bottom, R, G, B);
}

void AppendSquareVertices(
	std::vector<Vertex>& vertices,
	const Square& square)
{
	float left = square.x - square.halfSize;
	float right = square.x + square.halfSize;
	float bottom = square.y - square.halfSize;
	float top = square.y + square.halfSize;

	vertices.push_back({ left, top, 0.0f, square.R, square.G, square.B });
	vertices.push_back({ left, bottom, 0.0f, square.R, square.G, square.B });
	vertices.push_back({ right, bottom, 0.0f, square.R, square.G, square.B });

	vertices.push_back({ left, top, 0.0f, square.R, square.G, square.B });
	vertices.push_back({ right, bottom, 0.0f, square.R, square.G, square.B });
	vertices.push_back({ right, top, 0.0f, square.R, square.G, square.B });
}

void UpdateSquareVertices()
{
	squareRenderer.vertices.clear();

	for (const Square& square : stackedSquares)
		AppendSquareVertices(squareRenderer.vertices, square);

	AppendSquareVertices(squareRenderer.vertices, leftSquare);
	AppendSquareVertices(squareRenderer.vertices, rightSquare);
}

void UpdateSquareBuffer()
{
	if (squareRenderer.VBO == 0)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, squareRenderer.VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		squareRenderer.vertices.size() * sizeof(Vertex),
		squareRenderer.vertices.data(),
		GL_DYNAMIC_DRAW
	);
}

void ResetGame()
{
	stackedSquares.clear();
	animationState = AnimationState::VerticalMoving;
	SpawnNewSquares();
	UpdateSquareVertices();
	UpdateSquareBuffer();
	lastFrameTime = glfwGetTime();
}

void SpawnNewSquares()
{
	leftSquare = Square{};
	rightSquare = Square{};
  
  //세로공간 중앙에 배치.
	leftSquare.x = (board.leftLaneLeft + board.leftLaneRight) * 0.5f;
	rightSquare.x = (board.rightLaneLeft + board.rightLaneRight) * 0.5f;

	float minY = board.laneBottom + leftSquare.halfSize;
	float maxY = board.laneTop - leftSquare.halfSize;

	leftSquare.y = RandomFloat(minY, maxY);
	rightSquare.y = RandomFloat(minY, maxY);

	leftSquare.R = RandomFloat(0.15f, 0.90f);
	leftSquare.G = RandomFloat(0.15f, 0.90f);
	leftSquare.B = RandomFloat(0.15f, 0.90f);

	rightSquare.R = RandomFloat(0.15f, 0.90f);
	rightSquare.G = RandomFloat(0.15f, 0.90f);
	rightSquare.B = RandomFloat(0.15f, 0.90f);

	leftSquare.verticalDirection =
		RandomFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
	rightSquare.verticalDirection =
		RandomFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;

	leftSquare.verticalSpeed = RandomFloat(0.32f, 0.48f);
	rightSquare.verticalSpeed = RandomFloat(0.32f, 0.48f);
}

void SetStackTarget(Square& square, int stackIndex)
{
	const int rowsPerColumn = 13;
	const int column = (stackIndex / rowsPerColumn) % 3;
	const int row = stackIndex % rowsPerColumn;
	const float spacing = 0.012f;

	square.targetX = 0.52f + column * 0.16f;
	square.targetY = board.laneBottom
		+ square.halfSize
		+ row * (square.halfSize * 2.0f + spacing);
}

void UpdateAnimation()
{
	double currentTime = glfwGetTime();
	float deltaTime = static_cast<float>(currentTime - lastFrameTime);
	lastFrameTime = currentTime;

	if (deltaTime > 0.05f)
		deltaTime = 0.05f;

	switch (animationState)
	{
	case AnimationState::VerticalMoving:
		UpdateVerticalSquare(leftSquare, deltaTime);
		UpdateVerticalSquare(rightSquare, deltaTime);
		break;

	case AnimationState::Aligning:
		UpdateAligning(deltaTime);
		break;

	case AnimationState::MovingRight:
		UpdateMovingRight(deltaTime);
		break;
	}

	UpdateSquareVertices();
	UpdateSquareBuffer();
}

void UpdateVerticalSquare(Square& square, float deltaTime)
{
	square.y += square.verticalDirection
		* square.verticalSpeed
		* deltaTime;

	float minY = board.laneBottom + square.halfSize;
	float maxY = board.laneTop - square.halfSize;

	if (square.y >= maxY)
	{
		square.y = maxY;
		square.verticalDirection = -1.0f;
	}
	else if (square.y <= minY)
	{
		square.y = minY;
		square.verticalDirection = 1.0f;
	}
}

void UpdateAligning(float deltaTime)
{
	const float targetY =
		(board.meetingBottom + board.meetingTop) * 0.5f;
	const float alignSpeed = 0.65f;
	const float maxDelta = alignSpeed * deltaTime;

	leftSquare.y = MoveTowards(leftSquare.y, targetY, maxDelta);
	rightSquare.y = MoveTowards(rightSquare.y, targetY, maxDelta);

	if (leftSquare.y == targetY && rightSquare.y == targetY)
	{
		int firstStackIndex = static_cast<int>(stackedSquares.size());
		SetStackTarget(leftSquare, firstStackIndex);
		SetStackTarget(rightSquare, firstStackIndex + 1);
		animationState = AnimationState::MovingRight;
	}
}

void UpdateMovingRight(float deltaTime)
{
	bool leftArrived = MoveSquareToTarget(leftSquare, deltaTime);
	bool rightArrived = MoveSquareToTarget(rightSquare, deltaTime);

	if (!leftArrived || !rightArrived)
		return;

	stackedSquares.push_back(leftSquare);
	stackedSquares.push_back(rightSquare);

	SpawnNewSquares();
	animationState = AnimationState::VerticalMoving;
}

//가운데 영역 판
bool IsInsideMeetingSpace(const Square& square)
{
	return square.y >= board.meetingBottom
		&& square.y <= board.meetingTop;
}

bool MoveSquareToTarget(Square& square, float deltaTime)
{
	const float moveSpeed = 0.75f;
	const float maxDelta = moveSpeed * deltaTime;

	square.x = MoveTowards(square.x, square.targetX, maxDelta);
	square.y = MoveTowards(square.y, square.targetY, maxDelta);

	return square.x == square.targetX
		&& square.y == square.targetY;
}

float MoveTowards(float current, float target, float maxDelta)
{
	float difference = target - current;
	if (std::fabs(difference) <= maxDelta)
		return target;

	if (difference > 0.0f)
		return current + maxDelta;

	return current - maxDelta;
}

float RandomFloat(float minValue, float maxValue)
{
	std::uniform_real_distribution<float> distribution(minValue, maxValue);
	return distribution(randomGenerator);
}

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	DrawBoard();
	DrawSquares();
	glUseProgram(0);
}

void DrawBoard()
{
	glBindVertexArray(board.VAO);
	glDrawArrays(
		GL_LINES,
		0,
		static_cast<GLsizei>(board.vertices.size())
	);
	glBindVertexArray(0);
}

void DrawSquares()
{
	glBindVertexArray(squareRenderer.VAO);
	glDrawArrays(
		GL_TRIANGLES,
		0,
		static_cast<GLsizei>(squareRenderer.vertices.size())
	);
	glBindVertexArray(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}

	if (key == GLFW_KEY_R)
	{
		ResetGame();
		return;
	}

	if (key == GLFW_KEY_ENTER
		&& animationState == AnimationState::VerticalMoving
		&& IsInsideMeetingSpace(leftSquare)
		&& IsInsideMeetingSpace(rightSquare))
	{
		animationState = AnimationState::Aligning;
	}
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

void Cleanup()
{
	if (squareRenderer.VBO != 0)
		glDeleteBuffers(1, &squareRenderer.VBO);
	if (squareRenderer.VAO != 0)
		glDeleteVertexArrays(1, &squareRenderer.VAO);

	if (board.VBO != 0)
		glDeleteBuffers(1, &board.VBO);
	if (board.VAO != 0)
		glDeleteVertexArrays(1, &board.VAO);

	if (shaderProgram != 0)
		glDeleteProgram(shaderProgram);
}
