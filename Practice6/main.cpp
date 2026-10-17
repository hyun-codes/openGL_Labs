#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

std::random_device rd;
std::mt19937 generator(rd());

struct Rect
{
	float x, y;
	float halfSize;
	float R, G, B;

	bool moving = false;
	float directionX = 0.0f;
	float directionY = 0.0f;
	float speed = 0.0f;
	float shrinkSpeed = 0.0f;
	float colorSpeed = 0.0f;
	bool colorGetsBrighter = true;
};

std::vector<Rect> rects;
double lastFrameTime = 0.0;

float RandomFloat(float minValue, float maxValue);
int RandomInt(int minValue, int maxValue);
void ResetPractice();
void GetMouseNDC(GLFWwindow* window, float& x, float& y);
int FindClickedRect(float x, float y);
void AddFragment(const Rect& source, float startX, float startY,
float directionX, float directionY, float childHalfSize, bool getsBrighter);
void SplitRect(int index);
void UpdateAnimation(float deltaTime);
void DrawRect(const Rect& rect);
void DrawScene();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, int button, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	//GLFW 초기화
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	//OpenGL 버전 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	//윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Practice 6", nullptr, nullptr);
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
	lastFrameTime = glfwGetTime();

	//메인 루프
	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;

		// 창을 오래 멈췄다가 돌아왔을 때 한 번에 너무 멀리 이동하지 않게 한다.
		deltaTime = std::min(deltaTime, 0.05f);
		UpdateAnimation(deltaTime);
		DrawScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

//랜덤값 생성.
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
	rects.clear();

	int rectCount = RandomInt(5, 10);
	for (int i = 0; i < rectCount; ++i)
	{
		Rect rect;
		rect.halfSize = RandomFloat(0.07f, 0.15f);
		rect.x = RandomFloat(-1.0f + rect.halfSize, 1.0f - rect.halfSize);
		rect.y = RandomFloat(-1.0f + rect.halfSize, 1.0f - rect.halfSize);
		rect.R = RandomFloat(0.10f, 0.90f);
		rect.G = RandomFloat(0.10f, 0.90f);
		rect.B = RandomFloat(0.10f, 0.90f);
		rects.push_back(rect);
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

//클릭한 위치에서 가장 위에 그려진 사각형 찾기
int FindClickedRect(float x, float y)
{
	for (int i = static_cast<int>(rects.size()) - 1; i >= 0; --i)
	{
		const Rect& rect = rects[i];
		if (x >= rect.x - rect.halfSize && x <= rect.x + rect.halfSize &&
			y >= rect.y - rect.halfSize && y <= rect.y + rect.halfSize)
		{
			return i;
		}
	}
	return -1;
}

//움직이는 조각 한 개 추가
void AddFragment(const Rect& source, float startX, float startY,
	float directionX, float directionY, float childHalfSize, bool getsBrighter)
{
	float length = std::sqrt(directionX * directionX + directionY * directionY);
	if (length > 0.0f)
	{
		directionX /= length;
		directionY /= length;
	}

	Rect fragment;
	fragment.x = startX;
	fragment.y = startY;
	fragment.halfSize = childHalfSize;
	fragment.R = source.R;
	fragment.G = source.G;
	fragment.B = source.B;
	fragment.moving = true;
	fragment.directionX = directionX;
	fragment.directionY = directionY;
	//fragment의 속도가 랜덤값으로 설정이 되어있어서 전부 애니메이션의 속도는 전부 다르게 한다.
	//fragment.speed = RandomFloat(0.25f, 0.55f);	
	fragment.speed = 0.25f;
	// 크기와 관계없이 약 1.5~3초 동안 축소 애니메이션이 보이게 한다.
	fragment.shrinkSpeed = childHalfSize / RandomFloat(1.5f, 3.0f);
	fragment.colorSpeed = RandomFloat(0.25f, 0.55f);
	fragment.colorGetsBrighter = getsBrighter;
	rects.push_back(fragment);
}

// 선택한 사각형을 이동 방식에 따라 분할
void SplitRect(int index)
{
	if (index < 0 || index >= static_cast<int>(rects.size()))
		return;

	Rect source = rects[index];
	rects.erase(rects.begin() + index);

	//클릭할 때마다 네 가지 이동 방식 중 하나를 랜덤하게 선택한다.
	int moveType = RandomInt(0, 3);
	bool getsBrighter = RandomInt(0, 1) == 1;

	if (moveType == 3)
	{
		// 좌우상하와 대각선 8방향 이동
		const float directions[8][2] =
		{
			{-1.0f,  0.0f}, {1.0f,  0.0f},
			{ 0.0f, -1.0f}, {0.0f,  1.0f},
			{-1.0f, -1.0f}, {-1.0f, 1.0f},
			{ 1.0f, -1.0f}, {1.0f, 1.0f}
		};

		float childHalfSize = source.halfSize * 0.25f;
		for (const auto& direction : directions)
		{
			AddFragment(source,
				source.x + direction[0] * childHalfSize,
				source.y + direction[1] * childHalfSize,
				direction[0], direction[1], childHalfSize, getsBrighter);
		}
		return;
	}

	// 나머지 방식은 사각형을 정확히 사등분한다.
	float childHalfSize = source.halfSize * 0.5f;
	const float starts[4][2] =
	{
		{-childHalfSize,  childHalfSize},
		{ childHalfSize,  childHalfSize},
		{-childHalfSize, -childHalfSize},
		{ childHalfSize, -childHalfSize}
	};

	if (moveType == 0)
	{
		// 좌우상하 이동
		const float directions[4][2] =
		{
			{-1.0f, 0.0f}, {0.0f, 1.0f},
			{ 0.0f,-1.0f}, {1.0f, 0.0f}
		};

		for (int i = 0; i < 4; ++i)
		{
			AddFragment(source,
				source.x + starts[i][0], source.y + starts[i][1],
				directions[i][0], directions[i][1],
				childHalfSize, getsBrighter);
		}
	}
	else if (moveType == 1)
	{
		// 대각선 이동
		for (int i = 0; i < 4; ++i)
		{
			AddFragment(source,
				source.x + starts[i][0], source.y + starts[i][1],
				starts[i][0], starts[i][1],
				childHalfSize, getsBrighter);
		}
	}
	else
	{
		// 사등분된 사각형이 한 방향으로 같이 이동
		const float directions[8][2] =
		{
			{-1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f,-1.0f}, {0.0f,1.0f},
			{-1.0f,-1.0f}, {-1.0f,1.0f}, {1.0f,-1.0f}, {1.0f,1.0f}
		};
		int directionIndex = RandomInt(0, 7);

		for (int i = 0; i < 4; ++i)
		{
			AddFragment(source,
				source.x + starts[i][0], source.y + starts[i][1],
				directions[directionIndex][0], directions[directionIndex][1],
				childHalfSize, getsBrighter);
		}
	}
}

//조각 이동, 축소, 색상 변화
void UpdateAnimation(float deltaTime)
{
	for (Rect& rect : rects)
	{
		if (!rect.moving)
			continue;

		rect.x += rect.directionX * rect.speed * deltaTime;
		rect.y += rect.directionY * rect.speed * deltaTime;
		rect.halfSize -= rect.shrinkSpeed * deltaTime;

		if (rect.colorGetsBrighter)
		{
			rect.R = std::min(1.0f, rect.R + rect.colorSpeed * deltaTime);
			rect.G = std::min(1.0f, rect.G + rect.colorSpeed * deltaTime);
			rect.B = std::min(1.0f, rect.B + rect.colorSpeed * deltaTime);
		}
		else
		{
			rect.R = std::max(0.0f, rect.R - rect.colorSpeed * deltaTime);
			rect.G = std::max(0.0f, rect.G - rect.colorSpeed * deltaTime);
			rect.B = std::max(0.0f, rect.B - rect.colorSpeed * deltaTime);
		}
	}

	// 충분히 작아진 조각은 완전히 삭제한다.
	rects.erase(std::remove_if(rects.begin(), rects.end(),[](const Rect& rect)
		{
			return rect.halfSize <= 0.005f;
		}),rects.end());
}

//사각형 그리기
void DrawRect(const Rect& rect)
{
	glColor3f(rect.R, rect.G, rect.B);
	glBegin(GL_QUADS);
	glVertex2f(rect.x - rect.halfSize, rect.y - rect.halfSize);
	glVertex2f(rect.x + rect.halfSize, rect.y - rect.halfSize);
	glVertex2f(rect.x + rect.halfSize, rect.y + rect.halfSize);
	glVertex2f(rect.x - rect.halfSize, rect.y + rect.halfSize);
	glEnd();

	glColor3f(0.15f, 0.35f, 0.55f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(rect.x - rect.halfSize, rect.y - rect.halfSize);
	glVertex2f(rect.x + rect.halfSize, rect.y - rect.halfSize);
	glVertex2f(rect.x + rect.halfSize, rect.y + rect.halfSize);
	glVertex2f(rect.x - rect.halfSize, rect.y + rect.halfSize);
	glEnd();
}

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (const Rect& rect : rects)
		DrawRect(rect);
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

//마우스 입력
void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;

	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
		return;

	float mouseX = 0.0f;
	float mouseY = 0.0f;
	GetMouseNDC(window, mouseX, mouseY);

	int clickedIndex = FindClickedRect(mouseX, mouseY);
	if (clickedIndex >= 0)
		SplitRect(clickedIndex);
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}
