#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<algorithm>
#include<random>
#include<vector>

#include <iostream>

float colors[4][3];

std::random_device rd;
std::mt19937 generator(rd());
std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
std::uniform_real_distribution<float> sizeDistribution(0.08f, 0.35f);
//위에거 랜덤 크기 생성.
struct Rectangle
{
	//center는 이제 아래 centerPositions에 맞춰주면 됨.ㅎ
	float centerX;
	float centerY;
	float halfWidth;
	float halfHeight;

	float red;
	float green;
	float blue;

	bool selected;
};

float centerPositions[4][2] =
{
	{-0.5f,  0.5f}, //왼쪽 위
	{ 0.5f,  0.5f}, //오른쪽 위
	{-0.5f, -0.5f}, //왼쪽 아래
	{ 0.5f, -0.5f}  //오른쪽 아래
};

std::vector<Rectangle>rectangles[4];	//각 영역 안에 있는 사각형들.

void DrawScene();
void RandomizeColors();

void keyCallBack(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods
);

void mouseCallBack(
	GLFWwindow* window,
	int button,
	int action,
	int mods
);

void AddRectangle(int area);
void ResizeSelectedRectangle(float amount);
void ChangeSelectedRectangleColor();

int main() {
	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	//--- 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
	if (!window) {
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}
	//--- 컨텍스트 설정
	glfwMakeContextCurrent(window);
	//키보드 콜백 등록.
	glfwSetKeyCallback(window, keyCallBack);
	//마우스 콜백 등록.
	glfwSetMouseButtonCallback(window, mouseCallBack);

	//--- GLEW 초기화
	glewExperimental = GL_TRUE; // 최신 기능 사용
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		return -1;
	}
	//--- 뷰포트 설정
	glViewport(0, 0, 800, 600);

	RandomizeColors();

	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		DrawScene();
		// 버퍼 교체
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//--- 종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void AddRectangle(int area)
{
	if (rectangles[area].size() >= 5)
		return;
	Rectangle rectangle;

	rectangle.halfWidth = sizeDistribution(generator);
	rectangle.halfHeight = sizeDistribution(generator);

	std::uniform_real_distribution<float> xPositionDistribution(
		centerPositions[area][0] - 0.5f + rectangle.halfWidth,
		centerPositions[area][0] + 0.5f - rectangle.halfWidth
	);
	std::uniform_real_distribution<float> yPositionDistribution(
		centerPositions[area][1] - 0.5f + rectangle.halfHeight,
		centerPositions[area][1] + 0.5f - rectangle.halfHeight
	);

	rectangle.centerX = xPositionDistribution(generator);
	rectangle.centerY = yPositionDistribution(generator);

	rectangle.red = distribution(generator);
	rectangle.green = distribution(generator);
	rectangle.blue = distribution(generator);

	rectangle.selected = false;

	rectangles[area].push_back(rectangle);
}

void DrawScene()
{
	glClear(GL_COLOR_BUFFER_BIT);

	//이건 전체 사각형 4등분 
	glColor3f(colors[0][0], colors[0][1], colors[0][2]);
	glRectf(-1.0f, 0.0f, 0.0f, 1.0f);

	glColor3f(colors[1][0], colors[1][1], colors[1][2]);
	glRectf(0.0f, 0.0f, 1.0f, 1.0f);

	glColor3f(colors[2][0], colors[2][1], colors[2][2]);
	glRectf(-1.0f, -1.0f, 0.0f, 0.0f);

	glColor3f(colors[3][0], colors[3][1], colors[3][2]);
	glRectf(0.0f, -1.0f, 1.0f, 0.0f);

	//이건 4등분된 것들 5개 사각형 그리기.
	for (int area = 0; area < 4; ++area)
	{
		for (const Rectangle& rectangle : rectangles[area])
		{
			glColor3f(
				rectangle.red,
				rectangle.green,
				rectangle.blue
			);

			glRectf(
				rectangle.centerX - rectangle.halfWidth,
				rectangle.centerY - rectangle.halfHeight,
				rectangle.centerX + rectangle.halfWidth,
				rectangle.centerY + rectangle.halfHeight
			);

			if (rectangle.selected)
			{
				//선택된 사각형은 흰색 테두리로 표시.
				glColor3f(1.0f, 1.0f, 1.0f);
				glLineWidth(4.0f);
				glBegin(GL_LINE_LOOP);
				glVertex2f(rectangle.centerX - rectangle.halfWidth, rectangle.centerY - rectangle.halfHeight);
				glVertex2f(rectangle.centerX + rectangle.halfWidth, rectangle.centerY - rectangle.halfHeight);
				glVertex2f(rectangle.centerX + rectangle.halfWidth, rectangle.centerY + rectangle.halfHeight);
				glVertex2f(rectangle.centerX - rectangle.halfWidth, rectangle.centerY + rectangle.halfHeight);
				glEnd();
				glLineWidth(1.0f);
			}
		}
	}
}

void RandomizeColors()
{
	for (int area = 0; area < 4; ++area)
	{
		colors[area][0] = distribution(generator);
		colors[area][1] = distribution(generator);
		colors[area][2] = distribution(generator);
	}
}

void ResizeSelectedRectangle(float amount)
{
	for (int area = 0; area < 4; ++area)
	{
		for (Rectangle& rectangle : rectangles[area])
		{
			if (!rectangle.selected)
				continue;

			if (amount > 0.0f)
			{
				float areaMinX = centerPositions[area][0] - 0.5f;
				float areaMaxX = centerPositions[area][0] + 0.5f;
				float areaMinY = centerPositions[area][1] - 0.5f;
				float areaMaxY = centerPositions[area][1] + 0.5f;

				float maxHalfWidth = std::min(rectangle.centerX - areaMinX, areaMaxX - rectangle.centerX);
				float maxHalfHeight = std::min(rectangle.centerY - areaMinY, areaMaxY - rectangle.centerY);

				rectangle.halfWidth = std::min(rectangle.halfWidth + amount, maxHalfWidth);
				rectangle.halfHeight = std::min(rectangle.halfHeight + amount, maxHalfHeight);
			}
			else
			{
				rectangle.halfWidth = std::max(rectangle.halfWidth + amount, 0.03f);
				rectangle.halfHeight = std::max(rectangle.halfHeight + amount, 0.03f);
			}

			return;
		}
	}
}

void ChangeSelectedRectangleColor()
{
	for (int area = 0; area < 4; ++area)
	{
		for (Rectangle& rectangle : rectangles[area])
		{
			if (!rectangle.selected)
				continue;

			rectangle.red = distribution(generator);
			rectangle.green = distribution(generator);
			rectangle.blue = distribution(generator);
			return;
		}
	}
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		RandomizeColors();
		for (int area = 0; area < 4; ++area)
			rectangles[area].clear();
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		AddRectangle(0);

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		AddRectangle(1);

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		AddRectangle(2);

	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		AddRectangle(3);

	if ((key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) && action == GLFW_PRESS)
		ResizeSelectedRectangle(0.03f);

	if ((key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) && action == GLFW_PRESS)
		ResizeSelectedRectangle(-0.03f);

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		ChangeSelectedRectangleColor();

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
		return;

	double mouseX;
	double mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	int windowWidth;
	int windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	if (windowWidth == 0 || windowHeight == 0)
		return;

	//마우스 좌표를 OpenGL 좌표(-1.0 ~ 1.0)로 변환.
	float openGLX = static_cast<float>(mouseX / windowWidth * 2.0 - 1.0);
	float openGLY = static_cast<float>(1.0 - mouseY / windowHeight * 2.0);

	//기존 선택 상태 해제.
	for (int area = 0; area < 4; ++area)
	{
		for (Rectangle& rectangle : rectangles[area])
			rectangle.selected = false;
	}

	//겹친 경우 화면에 가장 나중에 그려진 사각형을 선택.
	for (int area = 3; area >= 0; --area)
	{
		for (int index = static_cast<int>(rectangles[area].size()) - 1; index >= 0; --index)
		{
			Rectangle& rectangle = rectangles[area][index];

			if (openGLX >= rectangle.centerX - rectangle.halfWidth &&
				openGLX <= rectangle.centerX + rectangle.halfWidth &&
				openGLY >= rectangle.centerY - rectangle.halfHeight &&
				openGLY <= rectangle.centerY + rectangle.halfHeight)
			{
				rectangle.selected = true;
				return;
			}
		}
	}
}
