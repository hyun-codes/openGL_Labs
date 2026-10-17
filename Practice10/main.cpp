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

constexpr int WINDOW_WIDTH = 900;
constexpr int WINDOW_HEIGHT = 900;
constexpr float PI = 3.1415926535f;
constexpr float DIVIDER_X = 0.05f;

enum class ShapeType
{
	Square,
	EquilateralTriangle,
	RightTriangle
};

struct Piece
{
	ShapeType type;
	float x;
	float y;
	float halfWidth;
	float halfHeight;
	float angle;
	float R;
	float G;
	float B;
	bool locked = false;
	int patternIndex = -1;
	int slotIndex = -1;
};

struct Slot
{
	ShapeType type;
	float x;
	float y;
	float halfWidth;
	float halfHeight;
	float angle;
	bool occupied = false;
	int pieceIndex = -1;
};

struct Pattern
{
	float x;
	float y;
	float halfWidth;
	float halfHeight;
	std::vector<Slot> slots;
	bool completed = false;
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
	GLfloat lineWidth;
};

std::random_device rd;
std::mt19937 generator(rd());

std::vector<Piece> pieces;
std::vector<Pattern> patterns;
std::vector<Vertex> vertices;
std::vector<DrawCommand> drawCommands;

GLuint shaderProgram = 0;
GLuint VAO = 0;
GLuint VBO = 0;

int draggedIndex = -1;
float dragOffsetX = 0.0f;
float dragOffsetY = 0.0f;

float RandomFloat(float minValue, float maxValue);
int RandomInt(int minValue, int maxValue);
std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void InitBuffer();
void CreatePatterns();
void CreateRandomPieces();
void ResetGame();
void TrySnapPiece(int pieceIndex);
bool RunLogicSelfTest();
void RebuildVertexData();
void DrawScene();
void Cleanup();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, int button, int action, int mods);
void cursorCallBack(GLFWwindow* window, double mouseX, double mouseY);
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
	GLFWwindow* window = glfwCreateWindow(
		WINDOW_WIDTH, WINDOW_HEIGHT, "Practice10", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetMouseButtonCallback(window, mouseCallBack);
	glfwSetCursorPosCallback(window, cursorCallBack);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);

	// GLEW 초기화
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

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
	ResetGame();

#ifdef _DEBUG
	// 스냅, 모양판 완성, 완성 후 잠금 로직 자동 검수
	if (!RunLogicSelfTest())
	{
		std::cerr << "Practice10 자체 검수 실패!" << std::endl;
		Cleanup();
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	ResetGame();
#endif

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

int RandomInt(int minValue, int maxValue)
{
	std::uniform_int_distribution<int> distribution(minValue, maxValue);
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

// VAO, VBO 생성과 정점 속성 연결
void InitBuffer()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	// location 0: 정점 위치 x, y, z
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, x)));
	glEnableVertexAttribArray(0);

	// location 1: 정점 색상 R, G, B
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		reinterpret_cast<void*>(offsetof(Vertex, R)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void AddSlot(Pattern& pattern, ShapeType type,
	float x, float y, float halfWidth, float halfHeight, float angle)
{
	pattern.slots.push_back({
		type, x, y, halfWidth, halfHeight, angle, false, -1 });
}

// 오른쪽에 기본 모양판 3개와 추가 모양판 2개 생성
void CreatePatterns()
{
	patterns.clear();

	// 1. 작은 사각형 4개로 만드는 큰 사각형
	Pattern squareGrid{ 0.32f, 0.70f, 0.17f, 0.22f };
	const float squareSize = 0.035f;
	AddSlot(squareGrid, ShapeType::Square, 0.28f, 0.74f, squareSize, squareSize, 0.0f);
	AddSlot(squareGrid, ShapeType::Square, 0.36f, 0.74f, squareSize, squareSize, 0.0f);
	AddSlot(squareGrid, ShapeType::Square, 0.28f, 0.66f, squareSize, squareSize, 0.0f);
	AddSlot(squareGrid, ShapeType::Square, 0.36f, 0.66f, squareSize, squareSize, 0.0f);
	patterns.push_back(squareGrid);

	// 2. 직각삼각형 4개로 만드는 바람개비
	Pattern pinwheel{ 0.73f, 0.70f, 0.18f, 0.22f };
	const float pinHalfWidth = 0.060f;
	const float pinHalfHeight = 0.060f;
	for (int i = 0; i < 4; ++i)
	{
		float angle = i * PI / 2.0f;
		float cosine = std::cos(angle);
		float sine = std::sin(angle);
		float rightAngleX = -pinHalfWidth * cosine + pinHalfHeight * sine;
		float rightAngleY = -pinHalfWidth * sine - pinHalfHeight * cosine;
		AddSlot(pinwheel, ShapeType::RightTriangle,
			pinwheel.x - rightAngleX, pinwheel.y - rightAngleY,
			pinHalfWidth, pinHalfHeight, angle);
	}
	patterns.push_back(pinwheel);

	// 3. 직각삼각형 2개로 만드는 세로 직사각형
	Pattern diagonalRectangle{ 0.32f, 0.22f, 0.17f, 0.22f };
	AddSlot(diagonalRectangle, ShapeType::RightTriangle,
		0.32f, 0.22f, 0.060f, 0.120f, 0.0f);
	AddSlot(diagonalRectangle, ShapeType::RightTriangle,
		0.32f, 0.22f, 0.060f, 0.120f, PI);
	patterns.push_back(diagonalRectangle);

	// 4. 사용자가 정한 모양판: 정삼각형 4개로 만드는 큰 삼각형
	Pattern largeTriangle{ 0.73f, 0.22f, 0.18f, 0.22f };
	AddSlot(largeTriangle, ShapeType::EquilateralTriangle,
		0.73f, 0.285f, 0.050f, 0.0433f, 0.0f);
	AddSlot(largeTriangle, ShapeType::EquilateralTriangle,
		0.675f, 0.185f, 0.050f, 0.0433f, 0.0f);
	AddSlot(largeTriangle, ShapeType::EquilateralTriangle,
		0.785f, 0.185f, 0.050f, 0.0433f, 0.0f);
	AddSlot(largeTriangle, ShapeType::EquilateralTriangle,
		0.73f, 0.205f, 0.050f, 0.0433f, PI);
	patterns.push_back(largeTriangle);

	// 5. 사용자가 정한 모양판: 사각형과 정삼각형으로 만드는 집
	Pattern house{ 0.53f, -0.52f, 0.23f, 0.23f };
	AddSlot(house, ShapeType::Square,
		0.53f, -0.575f, 0.065f, 0.065f, 0.0f);
	AddSlot(house, ShapeType::EquilateralTriangle,
		0.53f, -0.445f, 0.080f, 0.0693f, 0.0f);
	patterns.push_back(house);
}

// 모양판을 완성할 수 있는 수량과 랜덤 여분 도형 생성
void CreateRandomPieces()
{
	pieces.clear();

	int squareCount = 0;
	int triangleCount = 0;
	int rightTriangleCount = 0;
	for (const Pattern& pattern : patterns)
	{
		for (const Slot& slot : pattern.slots)
		{
			if (slot.type == ShapeType::Square)
				++squareCount;
			else if (slot.type == ShapeType::EquilateralTriangle)
				++triangleCount;
			else
				++rightTriangleCount;
		}
	}

	// 필요한 개수에 종류별 0~2개의 여분을 더해 매번 개수가 달라진다.
	squareCount += RandomInt(0, 2);
	triangleCount += RandomInt(0, 2);
	rightTriangleCount += RandomInt(0, 2);

	std::vector<ShapeType> types;
	types.insert(types.end(), squareCount, ShapeType::Square);
	types.insert(types.end(), triangleCount, ShapeType::EquilateralTriangle);
	types.insert(types.end(), rightTriangleCount, ShapeType::RightTriangle);
	std::shuffle(types.begin(), types.end(), generator);

	// 왼쪽 영역의 격자 위치를 섞고 약간의 랜덤 오차를 더한다.
	std::vector<int> cells(25);
	for (int i = 0; i < static_cast<int>(cells.size()); ++i)
		cells[i] = i;
	std::shuffle(cells.begin(), cells.end(), generator);

	for (int i = 0; i < static_cast<int>(types.size()); ++i)
	{
		int cell = cells[i];
		int column = cell % 5;
		int row = cell / 5;

		Piece piece{};
		piece.type = types[i];
		piece.x = -0.88f + column * 0.18f + RandomFloat(-0.025f, 0.025f);
		piece.y = 0.82f - row * 0.41f + RandomFloat(-0.025f, 0.025f);
		piece.angle = RandomInt(0, 3) * PI / 2.0f;
		piece.R = RandomFloat(0.10f, 0.90f);
		piece.G = RandomFloat(0.10f, 0.90f);
		piece.B = RandomFloat(0.10f, 0.90f);

		if (piece.type == ShapeType::Square)
		{
			piece.halfWidth = 0.035f;
			piece.halfHeight = 0.035f;
		}
		else if (piece.type == ShapeType::EquilateralTriangle)
		{
			piece.halfWidth = 0.045f;
			piece.halfHeight = 0.0390f;
		}
		else
		{
			piece.halfWidth = 0.045f;
			piece.halfHeight = 0.055f;
		}

		pieces.push_back(piece);
	}
}

void ResetGame()
{
	draggedIndex = -1;
	CreatePatterns();
	CreateRandomPieces();
	RebuildVertexData();
}

void TransformPoint(float localX, float localY,
	float centerX, float centerY, float angle, float& x, float& y)
{
	float cosine = std::cos(angle);
	float sine = std::sin(angle);
	x = centerX + localX * cosine - localY * sine;
	y = centerY + localX * sine + localY * cosine;
}

void AddVertex(float x, float y, float R, float G, float B)
{
	vertices.push_back({ x, y, 0.0f, R, G, B });
}

void AddTransformedVertex(float localX, float localY,
	float centerX, float centerY, float angle, float R, float G, float B)
{
	float x;
	float y;
	TransformPoint(localX, localY, centerX, centerY, angle, x, y);
	AddVertex(x, y, R, G, B);
}

void AddDrawCommand(GLenum mode, GLint first, GLfloat lineWidth)
{
	drawCommands.push_back({
		mode,
		first,
		static_cast<GLsizei>(vertices.size()) - first,
		lineWidth });
}

void AppendFilledShape(ShapeType type, float x, float y,
	float halfWidth, float halfHeight, float angle, float R, float G, float B)
{
	GLint first = static_cast<GLint>(vertices.size());
	if (type == ShapeType::Square)
	{
		const float points[6][2] =
		{
			{-halfWidth, -halfHeight}, { halfWidth, -halfHeight}, { halfWidth, halfHeight},
			{-halfWidth, -halfHeight}, { halfWidth,  halfHeight}, {-halfWidth, halfHeight}
		};
		for (const auto& point : points)
			AddTransformedVertex(point[0], point[1], x, y, angle, R, G, B);
	}
	else if (type == ShapeType::EquilateralTriangle)
	{
		AddTransformedVertex(0.0f, halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(-halfWidth, -halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(halfWidth, -halfHeight, x, y, angle, R, G, B);
	}
	else
	{
		AddTransformedVertex(-halfWidth, -halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(halfWidth, -halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(-halfWidth, halfHeight, x, y, angle, R, G, B);
	}
	AddDrawCommand(GL_TRIANGLES, first, 1.0f);
}

void AppendOutlineShape(ShapeType type, float x, float y,
	float halfWidth, float halfHeight, float angle,
	float R, float G, float B, float lineWidth)
{
	GLint first = static_cast<GLint>(vertices.size());
	if (type == ShapeType::Square)
	{
		const float points[4][2] =
		{
			{-halfWidth, -halfHeight}, {halfWidth, -halfHeight},
			{ halfWidth,  halfHeight}, {-halfWidth, halfHeight}
		};
		for (const auto& point : points)
			AddTransformedVertex(point[0], point[1], x, y, angle, R, G, B);
	}
	else if (type == ShapeType::EquilateralTriangle)
	{
		AddTransformedVertex(0.0f, halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(-halfWidth, -halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(halfWidth, -halfHeight, x, y, angle, R, G, B);
	}
	else
	{
		AddTransformedVertex(-halfWidth, -halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(halfWidth, -halfHeight, x, y, angle, R, G, B);
		AddTransformedVertex(-halfWidth, halfHeight, x, y, angle, R, G, B);
	}
	AddDrawCommand(GL_LINE_LOOP, first, lineWidth);
}

void AppendPanel(const Pattern& pattern)
{
	float R = pattern.completed ? 0.10f : 0.20f;
	float G = pattern.completed ? 0.70f : 0.45f;
	float B = pattern.completed ? 0.20f : 0.75f;
	AppendOutlineShape(ShapeType::Square,
		pattern.x, pattern.y, pattern.halfWidth, pattern.halfHeight,
		0.0f, R, G, B, pattern.completed ? 4.0f : 2.0f);
}

// CPU의 도형 목록을 하나의 동적 VBO 데이터로 다시 만든다.
void RebuildVertexData()
{
	vertices.clear();
	drawCommands.clear();

	// 좌우 영역 구분선
	GLint dividerFirst = static_cast<GLint>(vertices.size());
	AddVertex(DIVIDER_X, -0.95f, 0.20f, 0.45f, 0.75f);
	AddVertex(DIVIDER_X, 0.95f, 0.20f, 0.45f, 0.75f);
	AddDrawCommand(GL_LINES, dividerFirst, 2.0f);

	// 모양판과 아직 비어 있는 슬롯
	for (const Pattern& pattern : patterns)
	{
		AppendPanel(pattern);
		for (const Slot& slot : pattern.slots)
		{
			if (!slot.occupied)
			{
				AppendOutlineShape(slot.type,
					slot.x, slot.y, slot.halfWidth, slot.halfHeight, slot.angle,
					0.20f, 0.45f, 0.75f, 2.0f);
			}
		}
	}

	// 작은 도형
	for (int i = 0; i < static_cast<int>(pieces.size()); ++i)
	{
		const Piece& piece = pieces[i];
		AppendFilledShape(piece.type,
			piece.x, piece.y, piece.halfWidth, piece.halfHeight, piece.angle,
			piece.R, piece.G, piece.B);

		// 현재 드래그 중인 도형은 노란 외곽선으로 표시
		if (i == draggedIndex)
		{
			AppendOutlineShape(piece.type,
				piece.x, piece.y, piece.halfWidth, piece.halfHeight, piece.angle,
				1.0f, 0.75f, 0.0f, 4.0f);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(Vertex),
		vertices.empty() ? nullptr : vertices.data(),
		GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GetMouseNDC(GLFWwindow* window, float& x, float& y)
{
	double mouseX;
	double mouseY;
	int windowWidth;
	int windowHeight;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	x = static_cast<float>(mouseX / windowWidth * 2.0 - 1.0);
	y = static_cast<float>(1.0 - mouseY / windowHeight * 2.0);
}

bool PointInTriangle(float x, float y,
	float x1, float y1, float x2, float y2, float x3, float y3)
{
	float denominator = (y2 - y3) * (x1 - x3) +
		(x3 - x2) * (y1 - y3);
	float a = ((y2 - y3) * (x - x3) +
		(x3 - x2) * (y - y3)) / denominator;
	float b = ((y3 - y1) * (x - x3) +
		(x1 - x3) * (y - y3)) / denominator;
	float c = 1.0f - a - b;
	return a >= 0.0f && b >= 0.0f && c >= 0.0f;
}

bool ContainsPoint(const Piece& piece, float x, float y)
{
	// 도형 회전의 반대 방향으로 마우스 좌표를 돌려 로컬 좌표로 변환
	float dx = x - piece.x;
	float dy = y - piece.y;
	float cosine = std::cos(piece.angle);
	float sine = std::sin(piece.angle);
	float localX = dx * cosine + dy * sine;
	float localY = -dx * sine + dy * cosine;

	if (piece.type == ShapeType::Square)
	{
		return std::abs(localX) <= piece.halfWidth &&
			std::abs(localY) <= piece.halfHeight;
	}

	if (piece.type == ShapeType::EquilateralTriangle)
	{
		return PointInTriangle(localX, localY,
			0.0f, piece.halfHeight,
			-piece.halfWidth, -piece.halfHeight,
			piece.halfWidth, -piece.halfHeight);
	}

	return PointInTriangle(localX, localY,
		-piece.halfWidth, -piece.halfHeight,
		piece.halfWidth, -piece.halfHeight,
		-piece.halfWidth, piece.halfHeight);
}

int FindTopPiece(float x, float y)
{
	for (int i = static_cast<int>(pieces.size()) - 1; i >= 0; --i)
	{
		if (!pieces[i].locked && ContainsPoint(pieces[i], x, y))
			return i;
	}
	return -1;
}

void RemovePieceFromSlot(Piece& piece)
{
	if (piece.patternIndex < 0 || piece.slotIndex < 0)
		return;

	Slot& slot = patterns[piece.patternIndex].slots[piece.slotIndex];
	slot.occupied = false;
	slot.pieceIndex = -1;
	patterns[piece.patternIndex].completed = false;
	piece.patternIndex = -1;
	piece.slotIndex = -1;
}

void CheckPatternCompleted(int patternIndex)
{
	Pattern& pattern = patterns[patternIndex];
	pattern.completed = std::all_of(
		pattern.slots.begin(), pattern.slots.end(),
		[](const Slot& slot) { return slot.occupied; });

	if (!pattern.completed)
		return;

	// 완성된 모양판 위의 모든 도형을 잠근다.
	for (const Slot& slot : pattern.slots)
	{
		if (slot.pieceIndex >= 0)
			pieces[slot.pieceIndex].locked = true;
	}
}

//마우스 놓았을떄 호출.
void TrySnapPiece(int pieceIndex)
{
	Piece& piece = pieces[pieceIndex];
	int bestPattern = -1;
	int bestSlot = -1;
	float bestDistance = 1000.0f;

	for (int patternIndex = 0;
		patternIndex < static_cast<int>(patterns.size()); ++patternIndex)
	{
		Pattern& pattern = patterns[patternIndex];
		if (pattern.completed)
			continue;

		for (int slotIndex = 0;
			slotIndex < static_cast<int>(pattern.slots.size()); ++slotIndex)
		{
			const Slot& slot = pattern.slots[slotIndex];
			if (slot.occupied || slot.type != piece.type)
				continue;

			float dx = piece.x - slot.x;
			float dy = piece.y - slot.y;
			float distance = std::sqrt(dx * dx + dy * dy);
			float snapDistance = std::max(slot.halfWidth, slot.halfHeight) * 1.8f + 0.025f;
			if (distance <= snapDistance && distance < bestDistance)
			{
				bestDistance = distance;
				bestPattern = patternIndex;
				bestSlot = slotIndex;
			}
		}
	}

	if (bestPattern < 0)
		return;

	Slot& slot = patterns[bestPattern].slots[bestSlot];
	piece.x = slot.x;
	piece.y = slot.y;
	piece.halfWidth = slot.halfWidth;
	piece.halfHeight = slot.halfHeight;
	piece.angle = slot.angle;
	piece.patternIndex = bestPattern;
	piece.slotIndex = bestSlot;
	slot.occupied = true;
	slot.pieceIndex = pieceIndex;

	CheckPatternCompleted(bestPattern);
}

// 마우스 조작과 독립적으로 핵심 맞추기 규칙을 검사한다.
bool RunLogicSelfTest()
{
	for (int patternIndex = 0;
		patternIndex < static_cast<int>(patterns.size()); ++patternIndex)
	{
		Pattern& pattern = patterns[patternIndex];
		for (int slotIndex = 0;
			slotIndex < static_cast<int>(pattern.slots.size()); ++slotIndex)
		{
			const Slot& slot = pattern.slots[slotIndex];
			int pieceIndex = -1;

			for (int i = 0; i < static_cast<int>(pieces.size()); ++i)
			{
				if (!pieces[i].locked && pieces[i].patternIndex < 0 &&
					pieces[i].type == slot.type)
				{
					pieceIndex = i;
					break;
				}
			}

			if (pieceIndex < 0)
				return false;

			// 슬롯 중심에 놓았을 때 해당 슬롯으로 정확히 스냅되어야 한다.
			pieces[pieceIndex].x = slot.x;
			pieces[pieceIndex].y = slot.y;
			TrySnapPiece(pieceIndex);
			if (pieces[pieceIndex].patternIndex != patternIndex)
				return false;
		}

		if (!pattern.completed)
			return false;

		for (const Slot& slot : pattern.slots)
		{
			if (!slot.occupied || slot.pieceIndex < 0 ||
				!pieces[slot.pieceIndex].locked)
			{
				return false;
			}
		}
	}

	return true;
}

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);

	for (const DrawCommand& command : drawCommands)
	{
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
	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_R)
		ResetGame();
	else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;
	if (button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	float mouseX;
	float mouseY;
	GetMouseNDC(window, mouseX, mouseY);

	if (action == GLFW_PRESS)
	{
		draggedIndex = FindTopPiece(mouseX, mouseY);
		if (draggedIndex < 0)
			return;

		Piece& piece = pieces[draggedIndex];
		RemovePieceFromSlot(piece);
		dragOffsetX = mouseX - piece.x;
		dragOffsetY = mouseY - piece.y;
		RebuildVertexData();
	}
	else if (action == GLFW_RELEASE && draggedIndex >= 0)
	{
		TrySnapPiece(draggedIndex);
		draggedIndex = -1;
		RebuildVertexData();
	}
}

void cursorCallBack(GLFWwindow* window, double mouseX, double mouseY)
{
	(void)mouseX;
	(void)mouseY;
	if (draggedIndex < 0)
		return;

	float x;
	float y;
	GetMouseNDC(window, x, y);
	Piece& piece = pieces[draggedIndex];
	piece.x = std::clamp(x - dragOffsetX, -0.96f, 0.96f);
	piece.y = std::clamp(y - dragOffsetY, -0.96f, 0.96f);
	RebuildVertexData();
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
