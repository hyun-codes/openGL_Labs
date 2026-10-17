#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

std::random_device rd;
std::mt19937 generator(rd());

constexpr float SQUARE_HALF_SIZE = 0.035f;
constexpr float FIRST_ERASER_HALF_SIZE = SQUARE_HALF_SIZE * 2.0f;
constexpr float MIN_ERASER_HALF_SIZE = 0.01f;
constexpr float MAX_ERASER_HALF_SIZE = 0.40f;
constexpr int MAX_NEW_SQUARES = 10;

struct Square
{
	float x, y;
	float halfSize;
	float R, G, B;
	bool erased = false;
};

std::vector<Square> squares;
bool eraserActive = false;
float eraserX = 0.0f;
float eraserY = 0.0f;
// 오른쪽 클릭으로만 바뀌는 다음 드래그의 시작 크기
float baseEraserHalfSize = FIRST_ERASER_HALF_SIZE;
// 현재 드래그 중 충돌에 따라 커지는 실제 지우개 크기
float eraserHalfSize = FIRST_ERASER_HALF_SIZE;
float eraserR = 0.0f;
float eraserG = 0.0f;
float eraserB = 0.0f;
int newSquareCount = 0;

float RandomFloat(float minValue, float maxValue);
int RandomInt(int minValue, int maxValue);
void ResetPractice();
void RestoreErasedSquares();
void AddSquare(float x, float y);
void UpdateEraserCollision();
void GetMouseNDC(GLFWwindow* window, float& x, float& y);
void DrawSquare(const Square& square);
void DrawEraser();
void DrawScene();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, int button, int action, int mods);
void cursorCallBack(GLFWwindow* window, double xpos, double ypos);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	//GLFW 초기화
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	// OpenGL 버전 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	// 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Practice 5", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	//컨텍스트 및 콜백 설정
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetMouseButtonCallback(window, mouseCallBack);
	glfwSetCursorPosCallback(window, cursorCallBack);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);

	//GLEW 초기화
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);
	ResetPractice();

	// 메인 루프
	while (!glfwWindowShouldClose(window))
	{
		DrawScene();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//종료 처리
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

//처음 상태로 초기화
void ResetPractice()
{
	squares.clear();
	newSquareCount = 0;
	eraserActive = false;
	baseEraserHalfSize = FIRST_ERASER_HALF_SIZE;
	eraserHalfSize = FIRST_ERASER_HALF_SIZE;
	eraserR = eraserG = eraserB = 0.0f;

	int squareCount = RandomInt(20, 40);
	for (int i = 0; i < squareCount; ++i)
	{
		Square square;
		square.halfSize = SQUARE_HALF_SIZE;
		square.x = RandomFloat(-1.0f + square.halfSize, 1.0f - square.halfSize);
		square.y = RandomFloat(-1.0f + square.halfSize, 1.0f - square.halfSize);
		square.R = RandomFloat(0.10f, 0.90f);
		square.G = RandomFloat(0.10f, 0.90f);
		square.B = RandomFloat(0.10f, 0.90f);
		squares.push_back(square);
	}
}

//드래그가 끝나면 지워진 사각형 복구
void RestoreErasedSquares()
{
	for (Square& square : squares)
		square.erased = false;
}

//오른쪽 마우스로 새로운 사각형 생성
void AddSquare(float x, float y)
{
	if (newSquareCount >= MAX_NEW_SQUARES)
		return;

	Square square;
	square.halfSize = SQUARE_HALF_SIZE;
	square.x = std::clamp(x, -1.0f + square.halfSize, 1.0f - square.halfSize);
	square.y = std::clamp(y, -1.0f + square.halfSize, 1.0f - square.halfSize);
	square.R = RandomFloat(0.10f, 0.90f);
	square.G = RandomFloat(0.10f, 0.90f);
	square.B = RandomFloat(0.10f, 0.90f);
	squares.push_back(square);
	++newSquareCount;

	baseEraserHalfSize = std::max(
		MIN_ERASER_HALF_SIZE,
		baseEraserHalfSize - SQUARE_HALF_SIZE * 0.15f);

	// 왼쪽 버튼도 누른 상태라면 화면에 보이는 지우개도 함께 줄인다.
	if (eraserActive)
	{
		eraserHalfSize = std::max(
			MIN_ERASER_HALF_SIZE,
			eraserHalfSize - SQUARE_HALF_SIZE * 0.15f);
	}
	else
	{
		eraserHalfSize = baseEraserHalfSize;
	}
}

//지우개와 닿은 사각형 삭제 처리
void UpdateEraserCollision()
{
	if (!eraserActive)
		return;

	for (Square& square : squares)
	{
		if (square.erased)
			continue;

		bool overlapX = std::abs(eraserX - square.x)
			<= eraserHalfSize + square.halfSize;
		bool overlapY = std::abs(eraserY - square.y)
			<= eraserHalfSize + square.halfSize;

		if (overlapX && overlapY)
		{
			square.erased = true;
			//부딪친 사각형과 같은 색상으로 변경
			eraserR = square.R;
			eraserG = square.G;
			eraserB = square.B;
			eraserHalfSize = std::min(
				MAX_ERASER_HALF_SIZE,
				eraserHalfSize + SQUARE_HALF_SIZE * 0.25f);
		}
	}
}

//마우스 화면 좌표를 OpenGL 좌표로 변경
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

//사각형 그리기
void DrawSquare(const Square& square)
{
	if (square.erased)
		return;

	glColor3f(square.R, square.G, square.B);
	glBegin(GL_QUADS);
	glVertex2f(square.x - square.halfSize, square.y - square.halfSize);
	glVertex2f(square.x + square.halfSize, square.y - square.halfSize);
	glVertex2f(square.x + square.halfSize, square.y + square.halfSize);
	glVertex2f(square.x - square.halfSize, square.y + square.halfSize);
	glEnd();

	glColor3f(0.15f, 0.35f, 0.55f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(square.x - square.halfSize, square.y - square.halfSize);
	glVertex2f(square.x + square.halfSize, square.y - square.halfSize);
	glVertex2f(square.x + square.halfSize, square.y + square.halfSize);
	glVertex2f(square.x - square.halfSize, square.y + square.halfSize);
	glEnd();
}

//지우개 사각형 그리기
void DrawEraser()
{
	if (!eraserActive)
		return;

	glColor3f(eraserR, eraserG, eraserB);
	glBegin(GL_QUADS);
	glVertex2f(eraserX - eraserHalfSize, eraserY - eraserHalfSize);
	glVertex2f(eraserX + eraserHalfSize, eraserY - eraserHalfSize);
	glVertex2f(eraserX + eraserHalfSize, eraserY + eraserHalfSize);
	glVertex2f(eraserX - eraserHalfSize, eraserY + eraserHalfSize);
	glEnd();

	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(eraserX - eraserHalfSize, eraserY - eraserHalfSize);
	glVertex2f(eraserX + eraserHalfSize, eraserY - eraserHalfSize);
	glVertex2f(eraserX + eraserHalfSize, eraserY + eraserHalfSize);
	glVertex2f(eraserX - eraserHalfSize, eraserY + eraserHalfSize);
	glEnd();
	glLineWidth(1.0f);
}

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (const Square& square : squares)
		DrawSquare(square);

	DrawEraser();
}

//키보드 입력
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_R)
		ResetPractice();
	else if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

//마우스 버튼 입력
void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		GetMouseNDC(window, eraserX, eraserY);
		eraserActive = true;
		// 이전 드래그에서 커졌던 크기는 이어받지 않는다.
		eraserHalfSize = baseEraserHalfSize;
		eraserR = eraserG = eraserB = 0.0f;
		UpdateEraserCollision();
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		eraserActive = false;
		eraserHalfSize = baseEraserHalfSize;
		RestoreErasedSquares();
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		float mouseX = 0.0f;
		float mouseY = 0.0f;
		GetMouseNDC(window, mouseX, mouseY);
		AddSquare(mouseX, mouseY);
		UpdateEraserCollision();
	}
}

//마우스 이동
void cursorCallBack(GLFWwindow* window, double xpos, double ypos)
{
	(void)xpos;
	(void)ypos;

	if (!eraserActive)
		return;

	GetMouseNDC(window, eraserX, eraserY);
	UpdateEraserCollision();
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}
