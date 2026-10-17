#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};

enum class ShapeType
{
	Triangle,
	Rectangle,
	InvertedTriangle
};

struct Board
{
	GLuint VAO = 0;
	GLuint VBO = 0;
	std::vector<Vertex> vertices;

	GLfloat left = -0.9f;
	GLfloat right = 0.9f;
	GLfloat bottom = -0.9f;
	GLfloat top = 0.9f;
	GLint cellCount = 20;
	GLfloat cellSize = (right - left) / cellCount;
};

struct Player
{
	GLuint VAO = 0;
	GLuint VBO = 0;
	ShapeType shapeType = ShapeType::Rectangle;

	int row = 0;
	int col = 0;

	float x = 0.0f;
	float y = 0.0f;
	float R = 1.0f;
	float G = 0.0f;
	float B = 0.0f;
	int horizontalDirection = 1;
	bool isMoving = false;
	double lastMoveTime = 0.0;
	double moveInterval = 0.1;
	bool collisionEffect = false;
	double collisionEffectStartTime = 0.0;
	double collisionEffectDuration = 0.3;

	std::vector<Vertex> vertices;
};

struct Obstacle
{
	GLuint VAO = 0;
	GLuint VBO = 0;
	ShapeType shapeType = ShapeType::Triangle;

	int row = 0;
	int col = 0;

	float x = 0.0f;
	float y = 0.0f;
	float halfWidth = 0.0f;
	float halfHeight = 0.0f;

	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	std::vector<Vertex> vertices;
};

GLuint shaderProgram = 0;

Board board;
Player player;
std::vector<Obstacle> obstacles;

std::mt19937 randomGenerator(std::random_device{}());

std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void InitBuffer();
void InitBoardBuffer();
void InitPlayerBuffer();
void InitObstacleData();
void InitObstacleBuffer();
void MakeShapeVertices(
	std::vector<Vertex>& vertices,
	ShapeType shapeType,
	float x,
	float y,
	float halfWidth,
	float halfHeight,
	float R,
	float G,
	float B);
void UpdateObstacleVertices(Obstacle& obstacle);
void UpdateObstacleBuffer(Obstacle& obstacle);
void UpdatePlayerPosition();
void UpdatePlayerVertices();
void UpdatePlayerBuffer();
void CheckPlayerCollision();
void StartCollisionEffect();
void UpdateCollisionEffect(double currentTime);
void UpdateAnimation();
void DrawScene();
void DrawBoard();
void DrawPlayer();
void DrawObstacles();
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
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice11", nullptr, nullptr);
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
	InitPlayerBuffer();
	InitObstacleData();
	InitObstacleBuffer();
}

void MakeShapeVertices(
	std::vector<Vertex>& vertices,
	ShapeType shapeType,
	float x,
	float y,
	float halfWidth,
	float halfHeight,
	float R,
	float G,
	float B)
{
	vertices.clear();

	switch (shapeType)
	{
	case ShapeType::Triangle:
		vertices = {
			{ x,             y + halfHeight, 0.0f, R, G, B },
			{ x - halfWidth, y - halfHeight, 0.0f, R, G, B },
			{ x + halfWidth, y - halfHeight, 0.0f, R, G, B }
		};
		break;

	case ShapeType::Rectangle:
		vertices = {
			{ x - halfWidth, y + halfHeight, 0.0f, R, G, B },
			{ x - halfWidth, y - halfHeight, 0.0f, R, G, B },
			{ x + halfWidth, y - halfHeight, 0.0f, R, G, B },

			{ x - halfWidth, y + halfHeight, 0.0f, R, G, B },
			{ x + halfWidth, y - halfHeight, 0.0f, R, G, B },
			{ x + halfWidth, y + halfHeight, 0.0f, R, G, B }
		};
		break;

	case ShapeType::InvertedTriangle:
		vertices = {
			{ x - halfWidth, y + halfHeight, 0.0f, R, G, B },
			{ x + halfWidth, y + halfHeight, 0.0f, R, G, B },
			{ x,             y - halfHeight, 0.0f, R, G, B }
		};
		break;
	}
}

void InitObstacleData()
{
	const int obstacleCount = 60;

	std::uniform_int_distribution<int> rowDistribution(
		0, board.cellCount - 1);
	std::uniform_int_distribution<int> colDistribution(
		0, board.cellCount - 1);
	std::uniform_int_distribution<int> shapeDistribution(0, 2);
	std::uniform_real_distribution<float> sizeDistribution(
		board.cellSize * 0.2f,
		board.cellSize * 0.4f);
	std::uniform_real_distribution<float> colorDistribution(
		0.15f, 0.85f);

	std::vector<bool> occupiedCells(
		board.cellCount * board.cellCount,
		false);

	// 플레이어가 시작하는 좌측 상단 칸은 비워둔다.
	occupiedCells[0] = true;

	obstacles.clear();
	obstacles.reserve(obstacleCount);

	while (static_cast<int>(obstacles.size()) < obstacleCount)
	{
		Obstacle obstacle;

		obstacle.row = rowDistribution(randomGenerator);
		obstacle.col = colDistribution(randomGenerator);

		int cellIndex =
			obstacle.row * board.cellCount + obstacle.col;

		if (occupiedCells[cellIndex])
			continue;

		occupiedCells[cellIndex] = true;

		if (obstacles.size() < 3)
		{
			obstacle.shapeType = static_cast<ShapeType>(
				obstacles.size());
		}
		else
		{
			obstacle.shapeType = static_cast<ShapeType>(
				shapeDistribution(randomGenerator));
		}

		obstacle.x = board.left
			+ (obstacle.col + 0.5f) * board.cellSize;
		obstacle.y = board.top
			- (obstacle.row + 0.5f) * board.cellSize;

		obstacle.halfWidth = sizeDistribution(randomGenerator);
		obstacle.halfHeight = sizeDistribution(randomGenerator);

		obstacle.R = colorDistribution(randomGenerator);
		obstacle.G = colorDistribution(randomGenerator);
		obstacle.B = colorDistribution(randomGenerator);

		UpdateObstacleVertices(obstacle);
		obstacles.push_back(obstacle);
	}
}

void UpdateObstacleVertices(Obstacle& obstacle)
{
	MakeShapeVertices(
		obstacle.vertices,
		obstacle.shapeType,
		obstacle.x,
		obstacle.y,
		obstacle.halfWidth,
		obstacle.halfHeight,
		obstacle.R,
		obstacle.G,
		obstacle.B
	);
}

void InitObstacleBuffer()
{
	for (Obstacle& obstacle : obstacles)
	{
		glGenVertexArrays(1, &obstacle.VAO);
		glBindVertexArray(obstacle.VAO);

		glGenBuffers(1, &obstacle.VBO);
		glBindBuffer(GL_ARRAY_BUFFER, obstacle.VBO);

		// 사각형으로 바뀌어도 사용할 수 있도록 정점 6개 크기를 확보한다.
		glBufferData(
			GL_ARRAY_BUFFER,
			6 * sizeof(Vertex),
			nullptr,
			GL_DYNAMIC_DRAW
		);

		UpdateObstacleBuffer(obstacle);

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
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void UpdateObstacleBuffer(Obstacle& obstacle)
{
	glBindBuffer(GL_ARRAY_BUFFER, obstacle.VBO);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		obstacle.vertices.size() * sizeof(Vertex),
		obstacle.vertices.data()
	);
}

void InitBoardBuffer()
{
	//세로선 만들기.
	for (int i = 0; i <= board.cellCount; ++i)
	{
		GLfloat x = board.left + i * board.cellSize;

		board.vertices.push_back(
			{ x, board.bottom, 0.0f, 0.0f, 0.0f, 0.0f });
		board.vertices.push_back(
			{ x, board.top, 0.0f, 0.0f, 0.0f, 0.0f });
	}

	//가로선 만들기.
	for (int i = 0; i <= board.cellCount; ++i)
	{
		GLfloat y = board.top - i * board.cellSize;

		board.vertices.push_back(
			{ board.left, y, 0.0f, 0.0f, 0.0f, 0.0f });
		board.vertices.push_back(
			{ board.right, y, 0.0f, 0.0f, 0.0f, 0.0f });
	}

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
}

void InitPlayerBuffer() {
	//player의 위치값 업데이트.
	UpdatePlayerPosition();
	//player의 버텍스들 업데이트.
	UpdatePlayerVertices();

	glGenVertexArrays(1, &player.VAO);
	glBindVertexArray(player.VAO);

	glGenBuffers(1, &player.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO);

	glBufferData(
		GL_ARRAY_BUFFER,
		player.vertices.size() * sizeof(Vertex),
		player.vertices.data(),
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
}

//플레이어의 위치값만 업데이트.
void UpdatePlayerPosition() {
	player.x = board.left + (player.col + 0.5f) * board.cellSize;
	player.y = board.top - (player.row + 0.5f) * board.cellSize;
}

void UpdatePlayerVertices() {
	float halfSize = board.cellSize * 0.4f;
	float playerR = player.R;
	float playerG = player.G;
	float playerB = player.B;

	if (player.collisionEffect)
	{
		playerR = 1.0f;
		playerG = 1.0f;
		playerB = 0.0f;
	}

	MakeShapeVertices(
		player.vertices,
		player.shapeType,
		player.x,
		player.y,
		halfSize,
		halfSize,
		playerR,
		playerG,
		playerB
	);
}

void UpdatePlayerBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		player.vertices.size() * sizeof(Vertex),
		player.vertices.data()
	);
}

//물체와 플레이어 충돌 확인.
void CheckPlayerCollision()
{
	for (Obstacle& obstacle : obstacles)
	{
		if (player.row != obstacle.row ||
			player.col != obstacle.col)
		{
			continue;
		}

		std::swap(player.shapeType, obstacle.shapeType);

		UpdateObstacleVertices(obstacle);
		UpdateObstacleBuffer(obstacle);

		StartCollisionEffect();
		break;
	}
}

void StartCollisionEffect()
{
	player.collisionEffect = true;
	player.collisionEffectStartTime = glfwGetTime();

	UpdatePlayerVertices();
	UpdatePlayerBuffer();
}

void UpdateCollisionEffect(double currentTime)
{
	if (!player.collisionEffect)
		return;

	if (currentTime - player.collisionEffectStartTime
		< player.collisionEffectDuration)
	{
		return;
	}

	player.collisionEffect = false;
	UpdatePlayerVertices();
	UpdatePlayerBuffer();
}

void UpdateAnimation()
{
	double currentTime = glfwGetTime();
	UpdateCollisionEffect(currentTime);

	if (!player.isMoving)
		return;

	if (currentTime - player.lastMoveTime < player.moveInterval)
		return;

	player.lastMoveTime = currentTime;
	bool moved = false;

	if (player.horizontalDirection == 1)
	{
		if (player.col < board.cellCount - 1)
		{
			player.col++;
			moved = true;
		}
		else if (player.row < board.cellCount - 1)
		{
			player.row++;
			player.horizontalDirection = -1;
			moved = true;
		}
		else
		{
			player.isMoving = false;
		}
	}
	else
	{
		if (player.col > 0)
		{
			player.col--;
			moved = true;
		}
		else if (player.row < board.cellCount - 1)
		{
			player.row++;
			player.horizontalDirection = 1;
			moved = true;
		}
		else
		{
			player.isMoving = false;
		}
	}

	if (!moved)
		return;

	UpdatePlayerPosition();
	UpdatePlayerVertices();
	UpdatePlayerBuffer();
	CheckPlayerCollision();
}

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	DrawBoard();
	DrawObstacles();
	DrawPlayer();
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

void DrawPlayer()
{
	glBindVertexArray(player.VAO);
	glDrawArrays(
		GL_TRIANGLES,
		0,
		static_cast<GLsizei>(player.vertices.size())
	);
	glBindVertexArray(0);
}

void DrawObstacles()
{
	for (const Obstacle& obstacle : obstacles)
	{
		glBindVertexArray(obstacle.VAO);
		glDrawArrays(
			GL_TRIANGLES,
			0,
			static_cast<GLsizei>(obstacle.vertices.size())
		);
	}

	glBindVertexArray(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			return;
		}

		if ((key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT)) ||
			key == GLFW_KEY_KP_ADD)
		{
			player.moveInterval -= 0.02;
			if (player.moveInterval < 0.02)
				player.moveInterval = 0.02;

			std::cout << "이동 속도: "
				<< 1.0 / player.moveInterval
				<< " 칸/초" << std::endl;
			return;
		}

		if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
		{
			player.moveInterval += 0.02;
			if (player.moveInterval > 0.50)
				player.moveInterval = 0.50;

			std::cout << "이동 속도: "
				<< 1.0 / player.moveInterval
				<< " 칸/초" << std::endl;
			return;
		}

		if (key == GLFW_KEY_SPACE)
		{
			player.isMoving = !player.isMoving;
			player.lastMoveTime = glfwGetTime();
			return;
		}

		bool moved = false;

		if (key == GLFW_KEY_W && player.row > 0)
		{
			player.row--;
			moved = true;
		}

		if (key == GLFW_KEY_S && player.row < board.cellCount - 1)
		{
			player.row++;
			moved = true;
		}

		if (key == GLFW_KEY_A && player.col > 0)
		{
			player.col--;
			moved = true;
		}

		if (key == GLFW_KEY_D && player.col < board.cellCount - 1)
		{
			player.col++;
			moved = true;
		}

		if (!moved)
			return;

		UpdatePlayerPosition();
		UpdatePlayerVertices();
		UpdatePlayerBuffer();
		CheckPlayerCollision();
	}
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

void Cleanup()
{
	for (Obstacle& obstacle : obstacles)
	{
		if (obstacle.VBO != 0)
			glDeleteBuffers(1, &obstacle.VBO);
		if (obstacle.VAO != 0)
			glDeleteVertexArrays(1, &obstacle.VAO);
	}

	if (board.VBO != 0)
		glDeleteBuffers(1, &board.VBO);
	if (board.VAO != 0)
		glDeleteVertexArrays(1, &board.VAO);
	if (player.VBO != 0)
		glDeleteBuffers(1, &player.VBO);
	if (player.VAO != 0)
		glDeleteVertexArrays(1, &player.VAO);
	if (shaderProgram != 0)
		glDeleteProgram(shaderProgram);
}
