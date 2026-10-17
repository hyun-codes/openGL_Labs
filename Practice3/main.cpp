#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

struct Rect
{
	float x;
	float y;
	float halfWidth;
	float halfHeight;
	float R;
	float G;
	float B;
	std::shared_ptr<const Rect> beforeMergeFirst;
	std::shared_ptr<const Rect> beforeMergeSecond;
};

std::vector<Rect> rects;
std::random_device rd;
std::mt19937 generator(rd());
std::uniform_real_distribution<float> colorDistribution(0.0f, 1.0f);
std::uniform_real_distribution<float> sizeDistribution(0.07f, 0.16f);

int createdByA = 0;
int draggedIndex = -1;
float dragOffsetX = 0.0f;
float dragOffsetY = 0.0f;
bool movedWhileDragging = false;

void RandomizeColor(Rect& rect);
void AddRect();
void drawRect();
bool GetMousePosition(GLFWwindow* window, float& x, float& y);
int FindTopRectAt(float x, float y);
bool IsOverlapping(const Rect& first, const Rect& second);
void MergeDraggedRect();
void SplitRect(int index);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, int button, int action, int mods);
void cursorCallBack(GLFWwindow* window, double mouseX, double mouseY);
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
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Practice 3", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	//컨텍스트 설정
	glfwMakeContextCurrent(window);
	//키보드/마우스 콜백 등록
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

	//뷰포트 설정
	int framebufferWidth;
	int framebufferHeight;
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	rects.reserve(20);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	//메인 루프
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		drawRect();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void RandomizeColor(Rect& rect)
{
	rect.R = colorDistribution(generator);
	rect.G = colorDistribution(generator);
	rect.B = colorDistribution(generator);
}

void AddRect()
{
	//A 키로는 최대 10개, 화면 전체에는 최대 20개까지 생성
	if (createdByA >= 10 || rects.size() >= 20)
		return;

	Rect rect{};
	rect.halfWidth = sizeDistribution(generator);
	rect.halfHeight = sizeDistribution(generator);
	std::uniform_real_distribution<float> xDistribution(-1.0f + rect.halfWidth, 1.0f - rect.halfWidth);
	std::uniform_real_distribution<float> yDistribution(-1.0f + rect.halfHeight, 1.0f - rect.halfHeight);
	rect.x = xDistribution(generator);
	rect.y = yDistribution(generator);
	RandomizeColor(rect);

	//뒤에 추가된 사각형을 마지막에 그려 맨 위에 표시
	rects.push_back(rect);
	++createdByA;
}

void drawRect()
{
	for (const Rect& rect : rects)
	{
		glColor3f(rect.R, rect.G, rect.B);
		glRectf(
			rect.x - rect.halfWidth,
			rect.y - rect.halfHeight,
			rect.x + rect.halfWidth,
			rect.y + rect.halfHeight
		);
	}
}

bool GetMousePosition(GLFWwindow* window, float& x, float& y)
{
	int windowWidth;
	int windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	if (windowWidth <= 0 || windowHeight <= 0)
		return false;

	double mouseX;
	double mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	x = static_cast<float>(mouseX / windowWidth * 2.0 - 1.0);
	y = static_cast<float>(1.0 - mouseY / windowHeight * 2.0);
	return true;
}

int FindTopRectAt(float x, float y)
{
	//나중에 만든 사각형이 위에 있으므로 뒤에서부터 검사
	for (int i = static_cast<int>(rects.size()) - 1; i >= 0; --i)
	{
		const Rect& rect = rects[i];
		if (x >= rect.x - rect.halfWidth && x <= rect.x + rect.halfWidth &&
			y >= rect.y - rect.halfHeight && y <= rect.y + rect.halfHeight)
			return i;
	}
	return -1;
}

bool IsOverlapping(const Rect& first, const Rect& second)
{
	return first.x - first.halfWidth < second.x + second.halfWidth &&
		first.x + first.halfWidth > second.x - second.halfWidth &&
		first.y - first.halfHeight < second.y + second.halfHeight &&
		first.y + first.halfHeight > second.y - second.halfHeight;
}

void MergeDraggedRect()
{
	if (draggedIndex < 0 || draggedIndex >= static_cast<int>(rects.size()))
		return;

	for (int i = static_cast<int>(rects.size()) - 1; i >= 0; --i)
	{
		if (i == draggedIndex || !IsOverlapping(rects[draggedIndex], rects[i]))
			continue;

		const Rect& dragged = rects[draggedIndex];
		const Rect& other = rects[i];
		float minX = std::min(dragged.x - dragged.halfWidth, other.x - other.halfWidth);
		float maxX = std::max(dragged.x + dragged.halfWidth, other.x + other.halfWidth);
		float minY = std::min(dragged.y - dragged.halfHeight, other.y - other.halfHeight);
		float maxY = std::max(dragged.y + dragged.halfHeight, other.y + other.halfHeight);

		Rect merged{};
		merged.x = (minX + maxX) / 2.0f;
		merged.y = (minY + maxY) / 2.0f;
		merged.halfWidth = (maxX - minX) / 2.0f;
		merged.halfHeight = (maxY - minY) / 2.0f;
		//RGB 각 성분의 평균으로 섞고, 분리할 때 되돌릴 두 색을 보관
		merged.R = (dragged.R + other.R) / 2.0f;
		merged.G = (dragged.G + other.G) / 2.0f;
		merged.B = (dragged.B + other.B) / 2.0f;
		merged.beforeMergeFirst = std::make_shared<Rect>(dragged);
		merged.beforeMergeSecond = std::make_shared<Rect>(other);

		//두 사각형을 제거하고 합친 사각형을 맨 위에 추가
		int highIndex = std::max(draggedIndex, i);
		int lowIndex = std::min(draggedIndex, i);
		rects.erase(rects.begin() + highIndex);
		rects.erase(rects.begin() + lowIndex);
		rects.push_back(merged);
		return;
	}
}

void SplitRect(int index)
{
	if (index < 0 || rects.size() >= 20)
		return;

	Rect original = rects[index];

	Rect left = original.beforeMergeFirst ? *original.beforeMergeFirst : original;
	Rect right = original.beforeMergeSecond ? *original.beforeMergeSecond : original;
	//이전 크기와 무관하게 두 사각형의 가로·세로를 각각 새로 뽑음
	left.halfWidth = sizeDistribution(generator);
	right.halfWidth = sizeDistribution(generator);
	left.halfHeight = sizeDistribution(generator);
	right.halfHeight = sizeDistribution(generator);

	//두 사각형을 좌우로 놓고 창 경계 안에 유지
	const float gap = 0.02f;
	float totalWidth = 2.0f * left.halfWidth + gap + 2.0f * right.halfWidth;
	float leftEdge = std::clamp(original.x - totalWidth / 2.0f, -1.0f, 1.0f - totalWidth);
	left.x = leftEdge + left.halfWidth;
	right.x = leftEdge + 2.0f * left.halfWidth + gap + right.halfWidth;
	left.y = std::clamp(original.y, -1.0f + left.halfHeight, 1.0f - left.halfHeight);
	right.y = std::clamp(original.y, -1.0f + right.halfHeight, 1.0f - right.halfHeight);
	//합쳤던 사각형이면 합치기 전 각각의 RGB를 복원
	if (!original.beforeMergeFirst || !original.beforeMergeSecond)
	{
		RandomizeColor(left);
		RandomizeColor(right);
	}

	//원래 사각형을 두 개의 새로운 사각형으로 교체
	rects.erase(rects.begin() + index);
	rects.push_back(left);
	rects.push_back(right);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_A)
		AddRect();

	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	float mouseX;
	float mouseY;
	if (!GetMousePosition(window, mouseX, mouseY))
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		draggedIndex = FindTopRectAt(mouseX, mouseY);
		movedWhileDragging = false;
		if (draggedIndex >= 0)
		{
			dragOffsetX = mouseX - rects[draggedIndex].x;
			dragOffsetY = mouseY - rects[draggedIndex].y;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		if (movedWhileDragging)
			MergeDraggedRect();
		draggedIndex = -1;
		movedWhileDragging = false;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (draggedIndex < 0)
			SplitRect(FindTopRectAt(mouseX, mouseY));
	}
}

void cursorCallBack(GLFWwindow* window, double mouseX, double mouseY)
{
	if (draggedIndex < 0 || draggedIndex >= static_cast<int>(rects.size()))
		return;

	float x;
	float y;
	if (!GetMousePosition(window, x, y))
		return;

	Rect& rect = rects[draggedIndex];
	float nextX = std::clamp(x - dragOffsetX, -1.0f + rect.halfWidth, 1.0f - rect.halfWidth);
	float nextY = std::clamp(y - dragOffsetY, -1.0f + rect.halfHeight, 1.0f - rect.halfHeight);
	if (std::fabs(nextX - rect.x) > 0.001f || std::fabs(nextY - rect.y) > 0.001f)
		movedWhileDragging = true;
	rect.x = nextX;
	rect.y = nextY;
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
