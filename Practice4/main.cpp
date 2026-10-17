#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<random>
#include<vector>
#include<algorithm>
#include <cmath>
#include <iostream>

std::random_device rd;
std::mt19937 generator(rd());
//색 랜덤값.
std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

enum class Edge
{
	Top,
	Right,
	Bottom,
	Left
};

struct Rect
{
	//사각형이 그려질 위치.
	float x;
	float y;
	float originalX; //처음 마우스로 그린 위치.
	float originalY;
	float halfSize = 0.1f;
	float sizeDirection = 1.0f;

	//키보드 입력 1
	float directionX = 1.0f;    //오른쪽이면 1, 왼쪽이면 -1
	float directionY = 1.0f;    //위쪽이면 1, 아래쪽이면 -1
	bool zigzagStopped = false;
	Edge edge = Edge::Top;
	bool approachingEdge = false;
	float targetX = 0.0f;
	float targetY = 0.0f;
  
	//사각형의 RGB
	float R;
	float G;
	float B;
};

Rect rects[5];
int rectCount = 0;  //Rect가 실제로 그려진 개수.

enum class MoveMode		//움직이는 패턴을 enum으로 관리.
{
	Stop,		//정지.
	Diagonal,	//대각선.
	Zigzag,		//지그재그.
	Perimeter,	//가장자리를 따라 시계방향.
	ReturnHome	//원래 그린 위치로 돌아가기.
}; 	

MoveMode moveMode = MoveMode::Stop;		//처음 모드는 stop으로 설정.
bool sizeChanging = false;
bool colorChanging = false;
float colorElapsed = 0.0f;

void RandomizeColor(int index);
void drawRect();
void UpdateDiagonal(float deltaTime);
void UpdateZigzag(float deltaTime);
void SetPerimeterDestination(Rect& rect);
void StartPerimeter();
void UpdatePerimeter(float deltaTime);
void UpdateSize(float deltaTime);
void UpdateColor(float deltaTime);
void UpdateReturnHome(float deltaTime);

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
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Practice 4", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	//컨텍스트 설정
	glfwMakeContextCurrent(window);
	//키보드 콜백 등록.
	glfwSetKeyCallback(window, keyCallBack);
	//마우스 콜백 등록.
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

	//뷰포트 설정
	int framebufferWidth;
	int framebufferHeight;
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);	//배경색은 짙은 회색.
	double previousTime = glfwGetTime();	//시간을 저장.

	//메인 루프
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		double currentTime = glfwGetTime();	//현재 시간 저장.
		float deltaTime = std::min(static_cast<float>(currentTime - previousTime), 0.05f);
		previousTime = currentTime;	//이전 시간을 현재 시간에 저장.

		glClear(GL_COLOR_BUFFER_BIT);	//바탕색.

		UpdateSize(deltaTime);
		UpdateDiagonal(deltaTime);
		UpdateZigzag(deltaTime);
		UpdatePerimeter(deltaTime);
		UpdateReturnHome(deltaTime);
		UpdateColor(deltaTime);
		drawRect();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

//하나의 사각형만 색갈 설정하도록 설정.
void RandomizeColor(int index)
{
    rects[index].R = distribution(generator);
    rects[index].G = distribution(generator);
    rects[index].B = distribution(generator);
}

void drawRect()
{
	for (int i = 0; i < rectCount; ++i)
	{
		glColor3f(
			rects[i].R,
			rects[i].G,
			rects[i].B
		);
		glRectf(
			rects[i].x - rects[i].halfSize,
			rects[i].y - rects[i].halfSize,
			rects[i].x + rects[i].halfSize,
			rects[i].y + rects[i].halfSize
		);
	}
}

void UpdateDiagonal(float deltaTime)
{
    if(moveMode != MoveMode::Diagonal)
      return;
    const float speed = 0.5f;
	for(int i = 0; i< rectCount; ++i)
    {
		Rect& rect = rects[i];
		const float limit = 1.0f - rect.halfSize;	//현재 크기를 고려한 중심 좌표 경계.

		rect.x += rect.directionX * speed * deltaTime;
		rect.y += rect.directionY * speed * deltaTime;

		if (rect.x >= limit)
		{
			rect.x = limit;
			rect.directionX = -1.0f;
		}
		else if (rect.x <= -limit)
		{
			rect.x = -limit;
			rect.directionX = 1.0f;
		}

		if (rect.y >= limit)
		{
			rect.y = limit;
			rect.directionY = -1.0f;
		}
		else if (rect.y <= -limit)
		{
			rect.y = -limit;
			rect.directionY = 1.0f;
		}

    }
}

void UpdateZigzag(float deltaTime)
{
	if (moveMode != MoveMode::Zigzag)
		return;

	const float speed = 0.5f;

	for (int i = 0; i < rectCount; ++i)
	{
		Rect& rect = rects[i];
		const float limit = 1.0f - rect.halfSize;
		const float rowStep = rect.halfSize * 2.0f; // 사각형 한 칸 높이

		if (rect.zigzagStopped)
			continue;

		rect.x += rect.directionX * speed * deltaTime;

		if (rect.x >= limit)
		{
			rect.x = limit;
			rect.directionX = -1.0f; // 다음에는 왼쪽으로
			rect.y -= rowStep;        // 한 칸 아래로
		}
		else if (rect.x <= -limit)
		{
			rect.x = -limit;
			rect.directionX = 1.0f;  // 다음에는 오른쪽으로
			rect.y -= rowStep;        // 한 칸 아래로
		}

		if (rect.y <= -limit)
		{
			rect.y = -limit;
			rect.zigzagStopped = true;
		}
	}
}

void SetPerimeterDestination(Rect& rect)
{
	//현재 위치에서 가장 가까운 가장자리를 목적지로 설정. 여기서는 아직 움직이지 않음.
	float limit = 1.0f - rect.halfSize;
	float nearest = std::abs(limit - rect.y);
	rect.edge = Edge::Top;

	if (std::abs(limit - rect.x) < nearest)
	{
		nearest = std::abs(limit - rect.x);
		rect.edge = Edge::Right;
	}
	if (std::abs(-limit - rect.y) < nearest)
	{
		nearest = std::abs(-limit - rect.y);
		rect.edge = Edge::Bottom;
	}
	if (std::abs(-limit - rect.x) < nearest)
		rect.edge = Edge::Left;

	rect.targetX = std::clamp(rect.x, -limit, limit);
	rect.targetY = std::clamp(rect.y, -limit, limit);

	if (rect.edge == Edge::Top)
		rect.targetY = limit;
	else if (rect.edge == Edge::Right)
		rect.targetX = limit;
	else if (rect.edge == Edge::Bottom)
		rect.targetY = -limit;
	else
		rect.targetX = -limit;

	rect.approachingEdge = true;
}

void StartPerimeter()
{
	for (int i = 0; i < rectCount; ++i)
		SetPerimeterDestination(rects[i]);
}

void UpdatePerimeter(float deltaTime)
{
	if (moveMode != MoveMode::Perimeter)
		return;

	const float speed = 0.5f;
	float step = speed * deltaTime;

	for (int i = 0; i < rectCount; ++i)
	{
		Rect& rect = rects[i];
		float limit = 1.0f - rect.halfSize;

		if (rect.approachingEdge)
		{
			//크기가 바뀌어도 목적지는 현재 사각형 크기에 맞는 가장자리로 보정.
			rect.targetX = std::clamp(rect.targetX, -limit, limit);
			rect.targetY = std::clamp(rect.targetY, -limit, limit);
			if (rect.edge == Edge::Top)
				rect.targetY = limit;
			else if (rect.edge == Edge::Right)
				rect.targetX = limit;
			else if (rect.edge == Edge::Bottom)
				rect.targetY = -limit;
			else
				rect.targetX = -limit;

			float dx = rect.targetX - rect.x;
			float dy = rect.targetY - rect.y;
			float distance = std::sqrt(dx * dx + dy * dy);
			if (distance <= step)
			{
				rect.x = rect.targetX;
				rect.y = rect.targetY;
				rect.approachingEdge = false;
			}
			else
			{
				rect.x += dx / distance * step;
				rect.y += dy / distance * step;
			}
			continue;
		}

		//위 → 오른쪽 → 아래 → 왼쪽 순서로 시계방향 이동.
		if (rect.edge == Edge::Top)
		{
			rect.y = limit;
			rect.x += step;
			if (rect.x >= limit)
			{
				rect.x = limit;
				rect.edge = Edge::Right;
			}
		}
		else if (rect.edge == Edge::Right)
		{
			rect.x = limit;
			rect.y -= step;
			if (rect.y <= -limit)
			{
				rect.y = -limit;
				rect.edge = Edge::Bottom;
			}
		}
		else if (rect.edge == Edge::Bottom)
		{
			rect.y = -limit;
			rect.x -= step;
			if (rect.x <= -limit)
			{
				rect.x = -limit;
				rect.edge = Edge::Left;
			}
		}
		else
		{
			rect.x = -limit;
			rect.y += step;
			if (rect.y >= limit)
			{
				rect.y = limit;
				rect.edge = Edge::Top;
			}
		}
	}
}

void UpdateSize(float deltaTime)
{
	if (!sizeChanging)
		return;

	for (int i = 0; i < rectCount; ++i)
	{
		Rect& rect = rects[i];
		rect.halfSize += rect.sizeDirection * 0.12f * deltaTime;
		if (rect.halfSize >= 0.22f)
		{
			rect.halfSize = 0.22f;
			rect.sizeDirection = -1.0f;
		}
		else if (rect.halfSize <= 0.06f)
		{
			rect.halfSize = 0.06f;
			rect.sizeDirection = 1.0f;
		}

		float limit = 1.0f - rect.halfSize;
		rect.x = std::clamp(rect.x, -limit, limit);
		rect.y = std::clamp(rect.y, -limit, limit);
	}
}

void UpdateColor(float deltaTime)
{
	if (!colorChanging)
		return;

	colorElapsed += deltaTime;
	if (colorElapsed < 0.3f)
		return;

	colorElapsed = 0.0f;
	for (int i = 0; i < rectCount; ++i)
		RandomizeColor(i);
}

void UpdateReturnHome(float deltaTime)
{
	if (moveMode != MoveMode::ReturnHome)
		return;

	const float speed = 0.7f;
	float step = speed * deltaTime;
	bool allArrived = true;

	for (int i = 0; i < rectCount; ++i)
	{
		Rect& rect = rects[i];
		float dx = rect.originalX - rect.x;
		float dy = rect.originalY - rect.y;
		float distance = std::sqrt(dx * dx + dy * dy);
		if (distance <= step)
		{
			rect.x = rect.originalX;
			rect.y = rect.originalY;
		}
		else
		{
			rect.x += dx / distance * step;
			rect.y += dy / distance * step;
			allArrived = false;
		}
	}

	if (allArrived)
		moveMode = MoveMode::Stop;
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_1)
	{
		moveMode = moveMode == MoveMode::Diagonal ? MoveMode::Stop : MoveMode::Diagonal;
	}
	else if (key == GLFW_KEY_2)
	{
		if (moveMode == MoveMode::Zigzag)
		{
			moveMode = MoveMode::Stop;
		}
		else
		{
			moveMode = MoveMode::Zigzag;

			for (int i = 0; i < rectCount; ++i)
				rects[i].zigzagStopped = false;
		}
	}
	else if (key == GLFW_KEY_3)
	{
		if (moveMode == MoveMode::Perimeter)
			moveMode = MoveMode::Stop;
		else
		{
			StartPerimeter();
			moveMode = MoveMode::Perimeter;
		}
	}
	else if (key == GLFW_KEY_4)
	{
		sizeChanging = !sizeChanging;
	}
	else if (key == GLFW_KEY_5)
	{
		colorChanging = !colorChanging;
		colorElapsed = 0.0f;
	}
	else if (key == GLFW_KEY_S)
	{
		//모든 애니메이션을 현재 위치·크기·색에서 멈춤.
		moveMode = MoveMode::Stop;
		sizeChanging = false;
		colorChanging = false;
	}
	else if (key == GLFW_KEY_M)
	{
		//처음 클릭한 위치까지 부드럽게 이동.
		sizeChanging = false;
		colorChanging = false;
		moveMode = MoveMode::ReturnHome;
	}
	else if (key == GLFW_KEY_R)
	{
		//사각형을 지우고 다시 마우스 클릭을 받을 수 있게 초기화.
		rectCount = 0;
		moveMode = MoveMode::Stop;
		sizeChanging = false;
		colorChanging = false;
		colorElapsed = 0.0f;
	}
	else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
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

	//여기서 마우스의 위치를 opengl의 좌표로 변환.
	float openGLX = static_cast<float>(mouseX / windowWidth * 2.0 - 1.0);
	float openGLY = static_cast<float>(1.0 - mouseY / windowHeight * 2.0);

	if(rectCount >= 5)	//사각형이 5개 이상이 되면 리턴.
		return;

	//클릭할떄 마우스의 위치를 사각형의 x, y좌표에 삽입.
	rects[rectCount] = Rect{};
	rects[rectCount].x = openGLX;	//rectCount번쨰 사각형의 x좌표에 마우스의 x좌표 삽입.
	rects[rectCount].y = openGLY;	//rectCount번쨰 사각형의 x좌표에 마우스의 y좌표 삽입
	rects[rectCount].originalX = openGLX;
	rects[rectCount].originalY = openGLY;
	//클릭할떄 사각형의 이동방향 정하기.
	rects[rectCount].directionX = distribution(generator) < 0.5f ? -1.0f : 1.0f;
	rects[rectCount].directionY = distribution(generator) < 0.5f ? -1.0f : 1.0f;

	RandomizeColor(rectCount);
	if (moveMode == MoveMode::Perimeter)
		SetPerimeterDestination(rects[rectCount]);
	++rectCount;
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
