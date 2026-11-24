#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

// 이 구역의 위치와 크기 및 색을 바꿔 장면을 조정한다.
constexpr float floorHalfSize = 6.0f; // 바닥 반쪽 길이이며 전체 가로와 세로는 이 값의 두 배
constexpr float gridSpacing = 2.0f; // 격자 한 칸의 간격이며 0보다 큰 값 사용
constexpr float gridThickness = 0.05f; // 격자선 두께
const glm::vec3 floorColor{0.65f, 0.78f, 0.86f}; // 바닥 RGB이며 각 성분은 0부터 1
const glm::vec3 gridColor{0.95f, 0.97f, 1.0f}; // 격자선 RGB
const glm::vec3 obstacleColor{0.15f, 0.4f, 0.9f}; // 허들과 터널 및 시소의 공통 기본 RGB
const glm::vec3 clearedColor{0.2f, 0.85f, 0.35f}; // 허들과 터널을 통과한 뒤의 RGB
const glm::vec3 dogColor{0.7f, 0.44f, 0.22f}; // 강아지 몸통과 머리 및 다리의 RGB
const glm::vec3 eyeColor{0.04f, 0.04f, 0.04f}; // 강아지 눈의 RGB
const glm::vec3 backgroundColor{0.64f, 0.79f, 0.9f}; // 화면 배경 RGB

const glm::vec3 initialCameraPosition{0.8f, 8.2f, 13.5f}; // 처음 카메라 위치이며 X와 Y 및 Z 순서
const glm::vec3 initialCameraTarget{0.0f, 0.4f, 0.0f}; // 처음 바라보는 지점이며 Y를 올리면 위쪽을 봄
const glm::vec3 initialDogPosition{-3.0f, 0.0f, 2.5f}; // 강아지 시작 위치이며 X와 Z로 바닥 위 배치
const glm::vec3 initialDogDirection{0.0f, 0.0f, -1.0f}; // 강아지 시작 방향이며 현재 음의 Z축 방향
const glm::vec3 initialTunnelPosition{1.0f, 0.0f, 0.0f}; // 터널 위치이며 바닥에 두려면 Y는 0
const glm::vec3 initialSeesawPosition{4.5f, 0.0f, 0.0f}; // 시소 위치이며 바닥에 두려면 Y는 0
// 허들 세 개의 배치이며 각 줄의 X와 Z를 바꿔 이동한다.
const std::array<glm::vec3, 3> hurdlePositions{{
    {-4.4f, 0.0f, 0.0f}, // 첫 번째 허들
    {-2.9f, 0.0f, 0.0f}, // 두 번째 허들
    {-1.4f, 0.0f, 0.0f}  // 세 번째 허들
}};
constexpr std::array<float, 3> hurdleHeights{0.7f, 0.85f, 1.0f}; // 첫 번째부터 세 번째 허들의 막대 높이

constexpr float dogMoveSpeed = 2.8f; // 강아지가 초당 이동하는 거리
constexpr float cameraMoveSpeed = 4.0f; // 카메라 X와 Y 및 Z축 이동 속도
constexpr float cameraRotateSpeed = 35.0f; // 카메라 제자리 회전과 공전의 초당 각도
constexpr float gravity = 12.0f; // 커질수록 빨리 떨어짐
constexpr float jumpSpeed = 7.0f; // 점프 초기 속도이며 커질수록 높이 뜀
constexpr float hurdleHalfWidth = 0.9f; // 허들 기둥 사이 거리의 절반
constexpr float hurdleThickness = 0.12f; // 허들 막대와 기둥의 두께
constexpr float hurdlePostExtension = 0.3f; // 허들 막대 위로 더 올라오는 기둥 길이
constexpr float tunnelHalfWidth = 1.0f; // 터널 양옆 벽 중심 간격의 절반
constexpr float tunnelHalfDepth = 1.05f; // 터널 앞뒤 길이의 절반
constexpr float tunnelHeight = 1.65f; // 바닥에서 터널 지붕 중심까지 높이
constexpr float tunnelThickness = 0.16f; // 터널 벽과 지붕 두께
constexpr float tunnelScaleAmount = 0.12f; // 터널 크기 변화 폭이며 현재 원래 크기의 12퍼센트
constexpr float tunnelScaleSpeed = 1.6f; // 터널이 커졌다 작아지는 반복 속도
constexpr float seesawHalfLength = 1.6f; // 시소 판 앞뒤 길이의 절반
constexpr float seesawHalfWidth = 0.55f; // 시소 판 좌우 폭의 절반
constexpr float seesawThickness = 0.12f; // 시소 판 두께
constexpr float initialSeesawAngle = 15.0f; // 시소 초기 기울기이며 도 단위

const glm::vec3 dogBodyCenter{0.0f, 0.6f, 0.0f}; // 몸통 중심이며 강아지 발 위치 기준
const glm::vec3 dogBodySize{0.5f, 0.4f, 0.9f}; // 몸통의 가로와 높이 및 깊이
const glm::vec3 dogHeadCenter{0.0f, 0.9f, 0.55f}; // 머리 중심이며 Z를 늘리면 앞쪽으로 이동
const glm::vec3 dogHeadSize{0.4f, 0.4f, 0.4f}; // 머리의 가로와 높이 및 깊이
const glm::vec3 dogTailCenter{0.0f, 0.65f, -0.6f}; // 꼬리 중심이며 음의 Z가 강아지 뒤쪽
const glm::vec3 dogTailSize{0.1f, 0.1f, 0.4f}; // 꼬리의 가로와 높이 및 길이
const glm::vec3 dogEarCenter{0.16f, 1.15f, 0.55f}; // 한쪽 귀 중심이며 반대쪽 귀는 X를 반전
const glm::vec3 dogEarSize{0.12f, 0.2f, 0.16f}; // 귀의 가로와 높이 및 깊이
const glm::vec3 dogEyeCenter{0.11f, 0.94f, 0.76f}; // 한쪽 눈 중심이며 반대쪽 눈은 X를 반전
const glm::vec3 dogEyeSize{0.06f, 0.06f, 0.04f}; // 눈의 가로와 높이 및 깊이
const glm::vec3 dogLegJoint{0.19f, 0.4f, 0.3f}; // 한쪽 다리 관절이며 X와 Z를 반전해 네 다리 배치
const glm::vec3 dogLegSize{0.13f, 0.4f, 0.14f}; // 다리의 가로와 길이 및 깊이
// 부품 크기에서 충돌 범위를 계산하므로 모델 크기를 바꾸면 함께 갱신된다.
const float dogHalfWidth = std::max({dogBodySize.x * 0.5f, dogHeadSize.x * 0.5f,
    dogEarCenter.x + dogEarSize.x * 0.5f, dogLegJoint.x + dogLegSize.x * 0.5f});
const float dogHalfDepth = std::max({dogBodySize.z * 0.5f,
    std::abs(dogTailCenter.z) + dogTailSize.z * 0.5f,
    dogHeadCenter.z + dogHeadSize.z * 0.5f, dogEyeCenter.z + dogEyeSize.z * 0.5f});
const float dogRadius = dogHalfWidth;
const float dogHeight = std::max({dogBodyCenter.y + dogBodySize.y * 0.5f,
    dogHeadCenter.y + dogHeadSize.y * 0.5f, dogEarCenter.y + dogEarSize.y * 0.5f});

struct Camera
{
    glm::vec3 position = initialCameraPosition;
    glm::vec3 target = initialCameraTarget;
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    float fovDegrees = 45.0f; // 시야각이며 작게 하면 화면이 확대되어 보임
    float nearPlane = 0.1f; // 이 거리보다 가까운 물체는 잘림
    float farPlane = 100.0f; // 이 거리보다 먼 물체는 잘림
    float orbitSpeed = 0.0f; // 원점 주위 공전 속도이며 0이면 정지
    float selfRotateSpeed = 0.0f; // 제자리 회전 속도이며 0이면 정지
};

struct Dog
{
    glm::vec3 position = initialDogPosition;
    glm::vec3 direction = initialDogDirection;
    float heading = glm::degrees(std::atan2(initialDogDirection.x, initialDogDirection.z));
    float verticalSpeed = 0.0f;
    float walkPhase = 0.0f;
    bool running = false; // true로 바꾸면 실행 직후부터 걸음
    bool grounded = true;
    bool walking = false;
};

struct Hurdle
{
    glm::vec3 position;
    float height;
    bool cleared = false;
};

struct Tunnel
{
    glm::vec3 position = initialTunnelPosition;
    float scale = 1.0f;
    float phase = 0.0f;
    int entrySide = 0; // 터널의 어느 입구로 들어왔는지 기록
    bool cleared = false;
};

struct Seesaw
{
    glm::vec3 position = initialSeesawPosition;
    float angle = initialSeesawAngle;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

GLFWwindow* window = nullptr;
GLuint shaderProgram = 0;
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
const int windowWidth = 1100; // 처음 창의 가로 크기
const int windowHeight = 800; // 처음 창의 세로 크기
int framebufferWidth = windowWidth;
int framebufferHeight = windowHeight;

Camera camera;
Dog dog;
Tunnel tunnel;
Seesaw seesaw;
std::array<Hurdle, 3> hurdles{{
    {hurdlePositions[0], hurdleHeights[0]},
    {hurdlePositions[1], hurdleHeights[1]},
    {hurdlePositions[2], hurdleHeights[2]}
}};

// 원점 중심이며 한 변 길이 1인 큐브를 모든 모델에 재사용한다.
constexpr GLfloat cubeVertices[][3] = {
	// 앞면
	{ -0.5f,  0.5f, -0.5f },
	{  0.5f,  0.5f, -0.5f },
	{  0.5f, -0.5f, -0.5f },

	{ -0.5f,  0.5f, -0.5f },
	{  0.5f, -0.5f, -0.5f },
	{ -0.5f, -0.5f, -0.5f },

	// 뒷면
	{ -0.5f,  0.5f,  0.5f },
	{ -0.5f, -0.5f,  0.5f },
	{  0.5f, -0.5f,  0.5f },

	{ -0.5f,  0.5f,  0.5f },
	{  0.5f, -0.5f,  0.5f },
	{  0.5f,  0.5f,  0.5f },

	// 왼쪽 면
	{ -0.5f,  0.5f, -0.5f },
	{ -0.5f, -0.5f, -0.5f },
	{ -0.5f, -0.5f,  0.5f },

	{ -0.5f,  0.5f, -0.5f },
	{ -0.5f, -0.5f,  0.5f },
	{ -0.5f,  0.5f,  0.5f },

	// 오른쪽 면
	{  0.5f,  0.5f, -0.5f },
	{  0.5f,  0.5f,  0.5f },
	{  0.5f, -0.5f,  0.5f },

	{  0.5f,  0.5f, -0.5f },
	{  0.5f, -0.5f,  0.5f },
	{  0.5f, -0.5f, -0.5f },

	// 윗면
	{ -0.5f,  0.5f, -0.5f },
	{ -0.5f,  0.5f,  0.5f },
	{  0.5f,  0.5f,  0.5f },

	{ -0.5f,  0.5f, -0.5f },
	{  0.5f,  0.5f,  0.5f },
	{  0.5f,  0.5f, -0.5f },

	// 아랫면
	{ -0.5f, -0.5f, -0.5f },
	{  0.5f, -0.5f, -0.5f },
	{  0.5f, -0.5f,  0.5f },

	{ -0.5f, -0.5f, -0.5f },
	{  0.5f, -0.5f,  0.5f },
	{ -0.5f, -0.5f,  0.5f }
};

bool InitOpenGL();
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void InitBuffer();
void InitCubeBuffer();
void ResetScene();
void Update(float deltaTime);
void DrawScene();
void Cleanup();
void keyCallBack(GLFWwindow* currentWindow, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* currentWindow, int width, int height);
bool IntersectsDog(const glm::vec3& position, const glm::vec3& center, const glm::vec3& size);

// 창을 준비하고 입력과 애니메이션 및 그리기를 반복한다.
int main()
{
    if (!InitOpenGL()) return -1;
    InitBuffer();
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        const double currentTime = glfwGetTime();
        const float deltaTime = std::clamp(static_cast<float>(currentTime - lastTime), 0.0f, 0.05f);
        lastTime = currentTime;
        Update(deltaTime);
        DrawScene();
        glfwSwapBuffers(window);
    }

    Cleanup();
    return 0;
}

bool InitOpenGL()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(windowWidth, windowHeight, "Dog Agility Practice", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "OpenGL 3.3 윈도우 생성 실패!\n";
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW 초기화 실패!\n";
		Cleanup();
		return false;
	}

	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);
    glfwSetKeyCallback(window, keyCallBack);
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	framebufferCallBack(window, framebufferWidth, framebufferHeight);
	glEnable(GL_DEPTH_TEST);
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (!shaderProgram)
	{
		Cleanup();
		return false;
	}
	std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
	std::cout << "P: start/pause | WASD: direction | SPACE: jump | 0: reset\n";
	std::cout << "C/Shift+C: self rotation | R/Shift+R: orbit | X/Y/Z with Shift: camera movement\n";
	return true;
}

// 모든 물체가 공유할 육면체 버퍼를 초기화한다.
void InitBuffer()
{
    InitCubeBuffer();
}

// 단위 육면체의 위치와 면별 법선을 GPU에 저장한다.
void InitCubeBuffer()
{
    const glm::vec3 normals[] = {
        {0, 0, -1}, {0, 0, 1}, {-1, 0, 0},
        {1, 0, 0}, {0, 1, 0}, {0, -1, 0}
    };
    Vertex vertices[36];
    for (int i = 0; i < 36; ++i)
        vertices[i] = {glm::vec3(cubeVertices[i][0], cubeVertices[i][1], cubeVertices[i][2]), normals[i / 6]};

    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// 강아지와 장애물 및 카메라를 초기 상태로 되돌린다.
void ResetScene()
{
    // 상단의 초기 위치와 방향 설정으로 돌아간다.
    camera = Camera{};
    dog = Dog{};
    tunnel = Tunnel{};
    seesaw = Seesaw{};
    for (Hurdle& hurdle : hurdles)
        hurdle.cleared = false;
}

// 육면체의 모델 행렬과 색을 보내고 그린다.
void DrawCubeScene(const glm::mat4& model, const glm::vec3& color)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(model));
    // 각 Draw 함수에서 고른 색을 프래그먼트 셰이더로 보낸다.
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// 부모 행렬에 부품의 중심 위치와 크기를 결합한다.
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center,
    const glm::vec3& size, const glm::vec3& color)
{
    // center는 부모 기준 위치이고 size는 실제 가로와 높이 및 깊이다.
    const glm::mat4 model = glm::scale(glm::translate(parent, center), size);
    DrawCubeScene(model, color);
}

// 밝은 바닥 위에 얇은 육면체로 격자선을 그린다.
void DrawFloorScene()
{
    const glm::mat4 identity(1.0f);
    // 바닥 윗면을 Y가 0인 위치에 맞추며 색은 floorColor로 바꾼다.
    DrawPartScene(identity, {0.0f, -0.04f, 0.0f},
        {floorHalfSize * 2.0f, 0.08f, floorHalfSize * 2.0f}, floorColor);

    // gridSpacing으로 칸 간격을 바꾸고 gridThickness로 선 두께를 바꾼다.
    const int count = static_cast<int>(floorHalfSize * 2.0f / gridSpacing);
    for (int i = 0; i <= count; ++i)
    {
        const float offset = -floorHalfSize + i * gridSpacing;
        DrawPartScene(identity, {offset, 0.003f, 0.0f},
            {gridThickness, 0.006f, floorHalfSize * 2.0f}, gridColor);
        DrawPartScene(identity, {0.0f, 0.003f, offset},
            {floorHalfSize * 2.0f, 0.006f, gridThickness}, gridColor);
    }
}

// 세 허들의 기둥과 막대 및 받침을 그린다.
void DrawHurdleScene()
{
    for (const Hurdle& hurdle : hurdles)
    {
        // hurdlePositions에서 정한 위치로 허들 전체와 판정을 함께 이동한다.
        glm::mat4 parent = glm::translate(glm::mat4(1.0f), hurdle.position);
        // 허들은 X축 방향으로 넘도록 배치하며 각도를 바꾸면 판정 축도 맞춰야 한다.
        parent = glm::rotate(parent, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // 기본색은 obstacleColor이고 넘은 뒤 색은 clearedColor이다.
        const glm::vec3 color = hurdle.cleared ? clearedColor : obstacleColor;
        for (float side : {-1.0f, 1.0f})
        {
            DrawPartScene(parent, {side * hurdleHalfWidth, (hurdle.height + hurdlePostExtension) * 0.5f, 0.0f},
                {hurdleThickness, hurdle.height + hurdlePostExtension, hurdleThickness}, color);
            // 허들 받침의 크기는 아래 size 값으로 조절한다.
            DrawPartScene(parent, {side * hurdleHalfWidth, 0.04f, 0.0f},
                {0.3f, 0.08f, 0.6f}, color);
        }
        DrawPartScene(parent, {0.0f, hurdle.height, 0.0f}, {hurdleHalfWidth * 2.0f, hurdleThickness, hurdleThickness}, color);
    }
}

// 양옆 벽과 지붕으로 앞뒤가 뚫린 터널을 그린다.
void DrawTunnelScene()
{
    // initialTunnelPosition으로 터널 전체의 배치를 바꾼다.
    glm::mat4 parent = glm::translate(glm::mat4(1.0f), tunnel.position);
    // tunnelScaleAmount와 tunnelScaleSpeed가 이 배율의 변화 폭과 속도를 정한다.
    parent = glm::scale(parent, glm::vec3(tunnel.scale));
    // 터널을 완주하면 허들과 같은 clearedColor로 바뀐다.
    const glm::vec3 color = tunnel.cleared ? clearedColor : obstacleColor;
    for (float side : {-1.0f, 1.0f})
        DrawPartScene(parent, {side * tunnelHalfWidth, tunnelHeight * 0.5f, 0.0f},
            {tunnelThickness, tunnelHeight, tunnelHalfDepth * 2.0f}, color);
    DrawPartScene(parent, {0.0f, tunnelHeight, 0.0f},
        {tunnelHalfWidth * 2.0f + tunnelThickness, tunnelThickness, tunnelHalfDepth * 2.0f}, color);
}

// 아래쪽 끝이 바닥에 닿도록 시소의 회전 중심 높이를 계산한다.
float GetSeesawCenterHeight()
{
    const float radians = glm::radians(seesaw.angle);
    return std::sin(std::abs(radians)) * seesawHalfLength + seesawThickness * 0.5f * std::cos(radians);
}

// 기울어진 시소 판과 중앙 받침대를 그린다.
void DrawSeesawScene()
{
    const float height = GetSeesawCenterHeight();
    // initialSeesawPosition으로 시소 전체의 배치를 바꾼다.
    const glm::mat4 parent = glm::translate(glm::mat4(1.0f), seesaw.position);
    // 받침대 폭과 깊이는 아래 0.7과 0.45이며 색은 obstacleColor를 공유한다.
    DrawPartScene(parent, {0.0f, height * 0.5f, 0.0f}, {0.7f, height, 0.45f}, obstacleColor);
    glm::mat4 board = glm::translate(parent, {0.0f, height, 0.0f});
    // initialSeesawAngle로 처음 기울기를 바꾸고 HalfLength와 HalfWidth로 판 크기를 바꾼다.
    board = glm::rotate(board, glm::radians(seesaw.angle), {1.0f, 0.0f, 0.0f});
    DrawPartScene(board, {0.0f, 0.0f, 0.0f}, {seesawHalfWidth * 2.0f, seesawThickness, seesawHalfLength * 2.0f},
        obstacleColor);
}

// 큐브로 몸통과 머리와 꼬리 및 눈과 귀와 네 다리를 그린다.
void DrawDogScene()
{
    // 강아지 전체 배치는 initialDogPosition을 바꾼다.
    glm::mat4 parent = glm::translate(glm::mat4(1.0f), dog.position);
    parent = glm::rotate(parent, glm::radians(dog.heading), {0.0f, 1.0f, 0.0f});
    // 각 부품의 Center로 상대 위치를 바꾸고 Size로 크기를 바꾼다.
    DrawPartScene(parent, dogBodyCenter, dogBodySize, dogColor);
    DrawPartScene(parent, dogHeadCenter, dogHeadSize, dogColor);
    DrawPartScene(parent, dogTailCenter, dogTailSize, dogColor);

    for (float side : {-1.0f, 1.0f})
    {
        glm::vec3 ear = dogEarCenter;
        glm::vec3 eye = dogEyeCenter;
        ear.x *= side;
        eye.x *= side;
        DrawPartScene(parent, ear, dogEarSize, dogColor);
        DrawPartScene(parent, eye, dogEyeSize, eyeColor);

        for (float front : {-1.0f, 1.0f})
        {
            // 27은 다리를 흔드는 최대 각도이며 양옆과 앞뒤가 반대로 움직인다.
            const float swing = dog.walking && dog.grounded ? 27.0f * std::sin(dog.walkPhase) * side * front : 0.0f;
            glm::mat4 leg = glm::translate(parent,
                {side * dogLegJoint.x, dogLegJoint.y, front * dogLegJoint.z});
            leg = glm::rotate(leg, glm::radians(swing), {1.0f, 0.0f, 0.0f});
            DrawPartScene(leg, {0.0f, -dogLegSize.y * 0.5f, 0.0f}, dogLegSize, dogColor);
        }
    }
}

// 강아지 발 아래에 있는 상자 윗면의 높이를 구한다.
float GetBoxSupportHeight(const glm::vec3& position, const glm::vec3& center, const glm::vec3& size)
{
    const float top = center.y + size.y * 0.5f;
    glm::vec3 probe = position;
    probe.y = top - 0.01f;
    if (position.y >= top - 0.001f && IntersectsDog(probe, center, size))
        return top;
    return 0.0f;
}

// 바닥과 시소 및 장애물 윗면 중 발 아래의 가장 높은 지면을 구한다.
float GetGroundHeight(const glm::vec3& position)
{
    float height = 0.0f;
    // 현재 시소 위치를 빼서 계산하므로 X와 Z 배치를 바꿔도 지면 판정이 따라간다.
    const glm::vec3 local = position - seesaw.position;
    const float radians = glm::radians(seesaw.angle);
    const float boardZ = (local.z - seesawThickness * 0.5f * std::sin(radians)) / std::cos(radians);
    if (std::abs(local.x) <= seesawHalfWidth && std::abs(boardZ) <= seesawHalfLength)
        height = GetSeesawCenterHeight() - std::tan(radians) * local.z + seesawThickness * 0.5f / std::cos(radians);

    for (const Hurdle& hurdle : hurdles)
    {
        height = std::max(height, GetBoxSupportHeight(position,
            hurdle.position + glm::vec3(0.0f, hurdle.height, 0.0f), {hurdleThickness, hurdleThickness, hurdleHalfWidth * 2.0f}));
        for (float side : {-1.0f, 1.0f})
            height = std::max(height, GetBoxSupportHeight(position,
                hurdle.position + glm::vec3(0.0f, (hurdle.height + hurdlePostExtension) * 0.5f, side * hurdleHalfWidth),
                {hurdleThickness, hurdle.height + hurdlePostExtension, hurdleThickness}));
    }

    height = std::max(height, GetBoxSupportHeight(position,
        tunnel.position + glm::vec3(0.0f, tunnelHeight, 0.0f) * tunnel.scale,
        glm::vec3(tunnelHalfWidth * 2.0f + tunnelThickness, tunnelThickness, tunnelHalfDepth * 2.0f) * tunnel.scale));
    return height;
}

// 강아지의 방향을 반영한 외접 상자와 장애물 부품의 겹침을 검사한다.
bool IntersectsDog(const glm::vec3& position, const glm::vec3& center, const glm::vec3& size)
{
    const float radians = glm::radians(dog.heading);
    const float halfX = dogHalfWidth * std::abs(std::cos(radians)) + dogHalfDepth * std::abs(std::sin(radians));
    const float halfZ = dogHalfWidth * std::abs(std::sin(radians)) + dogHalfDepth * std::abs(std::cos(radians));
    return std::abs(position.x - center.x) < halfX + size.x * 0.5f
        && std::abs(position.z - center.z) < halfZ + size.z * 0.5f
        && position.y < center.y + size.y * 0.5f - 0.001f
        && position.y + dogHeight > center.y - size.y * 0.5f + 0.001f;
}

// 허들의 막대와 기둥 및 터널의 벽과 지붕에 막히는지 확인한다.
bool IsBlocked(const glm::vec3& position)
{
    for (const Hurdle& hurdle : hurdles)
    {
        if (IntersectsDog(position, hurdle.position + glm::vec3(0.0f, hurdle.height, 0.0f),
            {hurdleThickness, hurdleThickness, hurdleHalfWidth * 2.0f})) return true;
        for (float side : {-1.0f, 1.0f})
        {
            if (IntersectsDog(position,
                hurdle.position + glm::vec3(0.0f, (hurdle.height + hurdlePostExtension) * 0.5f, side * hurdleHalfWidth),
                {hurdleThickness, hurdle.height + hurdlePostExtension, hurdleThickness})) return true;
        }
    }

    const float scale = tunnel.scale;
    for (float side : {-1.0f, 1.0f})
    {
        if (IntersectsDog(position,
            tunnel.position + glm::vec3(side * tunnelHalfWidth, tunnelHeight * 0.5f, 0.0f) * scale,
            glm::vec3(tunnelThickness, tunnelHeight, tunnelHalfDepth * 2.0f) * scale)) return true;
    }
    return IntersectsDog(position, tunnel.position + glm::vec3(0.0f, tunnelHeight, 0.0f) * scale,
        glm::vec3(tunnelHalfWidth * 2.0f + tunnelThickness, tunnelThickness, tunnelHalfDepth * 2.0f) * scale);
}

// 이동 방향과 점프 높이 및 지면 접촉을 갱신한다.
void UpdateDog(float deltaTime, glm::vec3 movement)
{
    dog.walking = false;
    if (!dog.running) return;

    const glm::vec3 previous = dog.position;
    if (glm::length(movement) > 0.0f)
    {
        movement = glm::normalize(movement);
        dog.heading = glm::degrees(std::atan2(movement.x, movement.z));
    }

    if (!dog.grounded)
    {
        dog.verticalSpeed -= gravity * deltaTime;
        const float nextY = dog.position.y + dog.verticalSpeed * deltaTime;
        glm::vec3 next = dog.position;
        next.y = nextY;
        const float support = GetGroundHeight(dog.position);
        if (dog.verticalSpeed <= 0.0f && nextY <= support && dog.position.y >= support - 0.001f)
        {
            dog.position.y = support;
            dog.verticalSpeed = 0.0f;
            dog.grounded = true;
        }
        else if (IsBlocked(next) && !IsBlocked(dog.position))
            dog.verticalSpeed = 0.0f;
        else
            dog.position.y = nextY;
    }

    // dogMoveSpeed로 걷는 속도를 바꾸며 프레임 시간만큼 이동한다.
    const glm::vec3 step = movement * dogMoveSpeed * deltaTime;
    for (int axis : {0, 2})
    {
        glm::vec3 next = dog.position;
        next[axis] = std::clamp(next[axis] + step[axis],
            -floorHalfSize + 0.9f, floorHalfSize - 0.9f); // 0.9는 바닥 경계에서 남기는 여유
        const float height = GetGroundHeight(next);
        const bool stepAllowed = !dog.grounded || height <= dog.position.y + 0.16f;
        if (stepAllowed && !IsBlocked(next))
            dog.position[axis] = next[axis];
    }

    const float surface = GetGroundHeight(dog.position);
    if (dog.grounded && std::abs(dog.position.y - surface) <= 0.16f)
    {
        dog.position.y = surface;
    }
    else if (dog.verticalSpeed <= 0.0f && dog.position.y <= surface
        && previous.y >= surface - 0.16f)
    {
        dog.position.y = surface;
        dog.verticalSpeed = 0.0f;
        dog.grounded = true;
    }
    else
    {
        dog.grounded = false;
    }

    dog.walking = glm::length(glm::vec2(dog.position.x - previous.x, dog.position.z - previous.z)) > 0.0001f;
    // 장애물이나 바닥 경계에 막히면 반대 방향으로 돌아 걷는다.
    if (!dog.walking && glm::length(movement) > 0.0f)
    {
        dog.direction = -movement;
        dog.heading = glm::degrees(std::atan2(dog.direction.x, dog.direction.z));
    }
    if (dog.walking && dog.grounded)
        dog.walkPhase = std::fmod(dog.walkPhase + 12.0f * deltaTime, glm::radians(360.0f)); // 12는 다리 왕복 속도
}

// 공간 키를 누르면 지면에서 점프를 시작한다.
void StartJump()
{
    if (dog.running && dog.grounded)
    {
        dog.verticalSpeed = jumpSpeed;
        dog.grounded = false;
    }
}

// 허들 위를 넘었는지와 터널의 반대편으로 나왔는지 검사한다.
void UpdateObstacles(const glm::vec3& previous)
{
    if (!dog.running) return;
    // 허들의 X 위치를 기준으로 막대 위를 지나갔는지 검사한다.
    for (Hurdle& hurdle : hurdles)
    {
        const float before = previous.x - hurdle.position.x;
        const float after = dog.position.x - hurdle.position.x;
        const bool crossed = (before < 0.0f && after >= 0.0f) || (before > 0.0f && after <= 0.0f);
        if (crossed && std::abs(dog.position.z - hurdle.position.z) < hurdleHalfWidth - dogRadius
            && std::min(previous.y, dog.position.y) > hurdle.height + hurdleThickness * 0.5f)
            hurdle.cleared = true; // 다음 그리기부터 clearedColor를 선택
    }

    // 터널 위치를 빼서 통과를 판정하므로 위치를 옮겨도 함께 이동한다.
    const glm::vec3 local = dog.position - tunnel.position;
    const glm::vec3 before = previous - tunnel.position;
    const float halfDepth = tunnelHalfDepth * tunnel.scale;
    const float corridor = (tunnelHalfWidth - tunnelThickness * 0.5f) * tunnel.scale - dogRadius;
    const bool fits = std::abs(local.x) < corridor
        && dog.position.y + dogHeight < (tunnelHeight - tunnelThickness * 0.5f) * tunnel.scale;
    if (!fits)
    {
        tunnel.entrySide = 0;
        return;
    }

    if (before.z >= halfDepth && local.z < halfDepth && local.z > -halfDepth)
        tunnel.entrySide = 1;
    if (before.z <= -halfDepth && local.z > -halfDepth && local.z < halfDepth)
        tunnel.entrySide = -1;

    if (tunnel.entrySide == 1 && local.z < -halfDepth - dogHalfDepth)
    {
        tunnel.cleared = true; // 반대편으로 완전히 나온 뒤 색을 변경
        tunnel.entrySide = 0;
    }
    if (tunnel.entrySide == -1 && local.z > halfDepth + dogHalfDepth)
    {
        tunnel.cleared = true; // 반대편으로 완전히 나온 뒤 색을 변경
        tunnel.entrySide = 0;
    }
}

// 카메라의 제자리 회전과 공전 및 월드 축 이동을 갱신한다.
void UpdateCamera(float deltaTime)
{
    // 위치는 유지하고 바라보는 지점만 돌려 제자리 회전을 만든다.
    if (camera.selfRotateSpeed != 0.0f)
    {
        const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(camera.selfRotateSpeed * deltaTime), camera.up);
        camera.target = camera.position + glm::vec3(rotation * glm::vec4(camera.target - camera.position, 0.0f));
    }

    // 월드 원점의 Y축을 중심으로 카메라 위치와 시선 목표를 함께 돌린다.
    if (camera.orbitSpeed != 0.0f)
    {
        const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(camera.orbitSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
        camera.position = glm::vec3(rotation * glm::vec4(camera.position, 1.0f));
        camera.target = glm::vec3(rotation * glm::vec4(camera.target, 1.0f));
    }

    const bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    // cameraMoveSpeed로 이동 속도를 바꾸며 Shift는 방향을 반전한다.
    const float step = (shift ? -1.0f : 1.0f) * cameraMoveSpeed * deltaTime;
    glm::vec3 movement(0.0f);
    // X축 이동 키를 바꾸려면 아래 GLFW_KEY_X를 수정하며 Shift는 반대 방향이다.
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) movement.x = step;
    // Y축 이동 키를 바꾸려면 아래 GLFW_KEY_Y를 수정하며 Shift는 반대 방향이다.
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) movement.y = step;
    // X와 Y는 위치와 목표를 함께 옮겨 바라보는 방향을 유지한다.
    camera.position += movement;
    camera.target += movement;

    // Z축 이동 키를 바꾸려면 아래 GLFW_KEY_Z를 수정하며 기본은 음의 Z 방향이다.
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        glm::vec3 next = camera.position;
        next.z -= step;
        const float distance = glm::length(next - camera.target);
        if (distance > 2.0f && distance < 28.0f) camera.position = next; // 2와 28은 허용 거리
    }
}

// 터널은 계속 움직이고 강아지는 시작 상태에서만 갱신한다.
void Update(float deltaTime)
{
    tunnel.phase = std::fmod(tunnel.phase + tunnelScaleSpeed * deltaTime, glm::radians(360.0f));
    tunnel.scale = 1.0f + tunnelScaleAmount * std::sin(tunnel.phase);

    const glm::vec3 previous = dog.position;
    UpdateDog(deltaTime, dog.direction);
    UpdateObstacles(previous);
    UpdateCamera(deltaTime);
}

// 강아지 조작과 카메라 회전 및 리셋 입력을 처리한다.
void keyCallBack(GLFWwindow* currentWindow, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS) return; // 한 번 누를 때만 토글하며 누른 채 반복 입력은 무시
    // 시작과 정지 키를 바꾸려면 아래 GLFW_KEY_P를 수정한다.
    if (key == GLFW_KEY_P)
    {
        dog.running = !dog.running;
        dog.walking = false;
    }
    // 점프 키를 바꾸려면 아래 GLFW_KEY_SPACE를 수정한다.
    if (key == GLFW_KEY_SPACE) StartJump();
    // 각 방향 키를 바꾸려면 해당 줄의 GLFW_KEY_W와 S 및 A와 D를 수정한다.
    if (key == GLFW_KEY_W) dog.direction = {0.0f, 0.0f, -1.0f};
    if (key == GLFW_KEY_S) dog.direction = {0.0f, 0.0f, 1.0f};
    if (key == GLFW_KEY_A) dog.direction = {-1.0f, 0.0f, 0.0f};
    if (key == GLFW_KEY_D) dog.direction = {1.0f, 0.0f, 0.0f};
    dog.heading = glm::degrees(std::atan2(dog.direction.x, dog.direction.z));
    // cameraRotateSpeed로 회전 속도를 바꾸며 Shift는 역회전을 선택한다.
    const float rotationSpeed = (mods & GLFW_MOD_SHIFT) ? -cameraRotateSpeed : cameraRotateSpeed;
    // 제자리 회전 키는 아래 GLFW_KEY_C를 바꾸며 같은 키를 다시 누르면 정지한다.
    if (key == GLFW_KEY_C)
    {
        camera.selfRotateSpeed = camera.selfRotateSpeed == rotationSpeed ? 0.0f : rotationSpeed;
    }
    // 공전 키는 아래 GLFW_KEY_R을 바꾸며 같은 키를 다시 누르면 정지한다.
    if (key == GLFW_KEY_R)
    {
        camera.orbitSpeed = camera.orbitSpeed == rotationSpeed ? 0.0f : rotationSpeed;
    }
    // 리셋 키는 아래 GLFW_KEY_0과 숫자패드 GLFW_KEY_KP_0을 수정한다.
    if (key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) ResetScene();
    // 종료 키는 아래 GLFW_KEY_Q와 GLFW_KEY_ESCAPE를 수정한다.
    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(currentWindow, GLFW_TRUE);
}

// 카메라 행렬을 보내고 바닥과 장애물 및 강아지를 그린다.
void DrawScene()
{
    // 배경 색은 상단 backgroundColor의 RGB를 바꾼다.
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    // 초기 시점은 initialCameraPosition과 initialCameraTarget을 바꾼다.
    const glm::mat4 view = glm::lookAt(camera.position, camera.target, camera.up);
    const float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
    const glm::mat4 projection = glm::perspective(glm::radians(camera.fovDegrees), aspect,
        camera.nearPlane, camera.farPlane);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewTransform"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectionTransform"), 1, GL_FALSE, glm::value_ptr(projection));

    DrawFloorScene();
    DrawHurdleScene();
    DrawTunnelScene();
    DrawSeesawScene();
    DrawDogScene();
}

// 화면 크기에 맞춰 뷰포트와 투영 비율을 갱신한다.
void framebufferCallBack(GLFWwindow*, int width, int height)
{
    framebufferWidth = std::max(width, 1);
    framebufferHeight = std::max(height, 1);
    glViewport(0, 0, width, height);
}

// GPU 자원과 윈도우를 해제한다.
void Cleanup()
{
    if (cubeVBO) glDeleteBuffers(1, &cubeVBO);
    if (cubeVAO) glDeleteVertexArrays(1, &cubeVAO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}
std::string ReadShaderFile(const char* filePath)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "셰이더 파일을 열 수 없습니다: " << filePath << '\n';
		return {};
	}
	std::ostringstream source;
	source << file.rdbuf();
	return source.str();
}

GLuint CompileShader(GLenum shaderType, const char* filePath)
{
	std::string source = ReadShaderFile(filePath);
	if (source.empty()) return 0;
	GLuint shader = glCreateShader(shaderType);
	const char* sourceText = source.c_str();
	glShaderSource(shader, 1, &sourceText, nullptr);
	glCompileShader(shader);
	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar errorLog[2048]{};
		glGetShaderInfoLog(shader, sizeof(errorLog), nullptr, errorLog);
		std::cerr << filePath << " 컴파일 실패:\n" << errorLog << '\n';
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
{
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexPath);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentPath);
	if (!vertexShader || !fragmentShader)
	{
		if (vertexShader) glDeleteShader(vertexShader);
		if (fragmentShader) glDeleteShader(fragmentShader);
		return 0;
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	GLint success = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar errorLog[2048]{};
		glGetProgramInfoLog(program, sizeof(errorLog), nullptr, errorLog);
		std::cerr << "셰이더 프로그램 연결 실패:\n" << errorLog << '\n';
		glDeleteProgram(program);
		return 0;
	}
	return program;
}
