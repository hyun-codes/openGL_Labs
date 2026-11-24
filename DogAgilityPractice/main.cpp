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
constexpr float floorHalfSize = 6.0f;
constexpr float gridSpacing = 2.0f;
constexpr float gridThickness = 0.05f;
const glm::vec3 floorColor{0.65f, 0.78f, 0.86f};
const glm::vec3 gridColor{0.95f, 0.97f, 1.0f};
const glm::vec3 obstacleColor{0.15f, 0.4f, 0.9f};
const glm::vec3 clearedColor{0.2f, 0.85f, 0.35f};
const glm::vec3 dogColor{0.7f, 0.44f, 0.22f};
const glm::vec3 eyeColor{0.04f, 0.04f, 0.04f};
const glm::vec3 backgroundColor{0.64f, 0.79f, 0.9f};

const glm::vec3 initialCameraPosition{0.8f, 8.2f, 13.5f};
const glm::vec3 initialCameraTarget{0.0f, 0.4f, 0.0f};
const glm::vec3 initialDogPosition{-3.0f, 0.0f, 2.5f};
const glm::vec3 initialDogDirection{0.0f, 0.0f, -1.0f};
const glm::vec3 initialTunnelPosition{1.0f, 0.0f, 0.0f};
const glm::vec3 initialSeesawPosition{4.5f, 0.0f, 0.0f};
const std::array<glm::vec3, 3> hurdlePositions{{
    {-4.4f, 0.0f, 0.0f}, {-2.9f, 0.0f, 0.0f}, {-1.4f, 0.0f, 0.0f}
}};
constexpr std::array<float, 3> hurdleHeights{0.7f, 0.85f, 1.0f};

constexpr float dogMoveSpeed = 2.8f;
constexpr float gravity = 12.0f;
constexpr float jumpSpeed = 7.0f;
constexpr float hurdleHalfWidth = 0.9f;
constexpr float hurdleThickness = 0.12f;
constexpr float hurdlePostExtension = 0.3f;
constexpr float tunnelHalfWidth = 1.0f;
constexpr float tunnelHalfDepth = 1.05f;
constexpr float tunnelHeight = 1.65f;
constexpr float tunnelThickness = 0.16f;
constexpr float tunnelScaleAmount = 0.12f;
constexpr float tunnelScaleSpeed = 1.6f;
constexpr float seesawHalfLength = 1.6f;
constexpr float seesawHalfWidth = 0.55f;
constexpr float seesawThickness = 0.12f;
constexpr float initialSeesawAngle = 15.0f;

const glm::vec3 dogBodyCenter{0.0f, 0.6f, 0.0f};
const glm::vec3 dogBodySize{0.5f, 0.4f, 0.9f};
const glm::vec3 dogHeadCenter{0.0f, 0.9f, 0.55f};
const glm::vec3 dogHeadSize{0.4f, 0.4f, 0.4f};
const glm::vec3 dogTailCenter{0.0f, 0.65f, -0.6f};
const glm::vec3 dogTailSize{0.1f, 0.1f, 0.4f};
const glm::vec3 dogEarCenter{0.16f, 1.15f, 0.55f};
const glm::vec3 dogEarSize{0.12f, 0.2f, 0.16f};
const glm::vec3 dogEyeCenter{0.11f, 0.94f, 0.76f};
const glm::vec3 dogEyeSize{0.06f, 0.06f, 0.04f};
const glm::vec3 dogLegJoint{0.19f, 0.4f, 0.3f};
const glm::vec3 dogLegSize{0.13f, 0.4f, 0.14f};
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
    float fovDegrees = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float orbitSpeed = 0.0f;
};

struct Dog
{
    glm::vec3 position = initialDogPosition;
    glm::vec3 direction = initialDogDirection;
    float heading = glm::degrees(std::atan2(initialDogDirection.x, initialDogDirection.z));
    float verticalSpeed = 0.0f;
    float walkPhase = 0.0f;
    bool running = false;
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
    int entrySide = 0;
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
const int windowWidth = 1100;
const int windowHeight = 800;
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
	std::cout << "P: start/pause | WASD: move | SPACE: jump | R: orbit | Z/Shift+Z: camera Z | 0: reset\n";
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
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// 부모 행렬에 부품의 중심 위치와 크기를 결합한다.
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center,
    const glm::vec3& size, const glm::vec3& color)
{
    const glm::mat4 model = glm::scale(glm::translate(parent, center), size);
    DrawCubeScene(model, color);
}

// 밝은 바닥 위에 얇은 육면체로 격자선을 그린다.
void DrawFloorScene()
{
    const glm::mat4 identity(1.0f);
    DrawPartScene(identity, {0.0f, -0.04f, 0.0f},
        {floorHalfSize * 2.0f, 0.08f, floorHalfSize * 2.0f}, floorColor);

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
        glm::mat4 parent = glm::translate(glm::mat4(1.0f), hurdle.position);
        parent = glm::rotate(parent, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec3 color = hurdle.cleared ? clearedColor : obstacleColor;
        for (float side : {-1.0f, 1.0f})
        {
            DrawPartScene(parent, {side * hurdleHalfWidth, (hurdle.height + hurdlePostExtension) * 0.5f, 0.0f},
                {hurdleThickness, hurdle.height + hurdlePostExtension, hurdleThickness}, color);
            DrawPartScene(parent, {side * hurdleHalfWidth, 0.04f, 0.0f},
                {0.3f, 0.08f, 0.6f}, color);
        }
        DrawPartScene(parent, {0.0f, hurdle.height, 0.0f}, {hurdleHalfWidth * 2.0f, hurdleThickness, hurdleThickness}, color);
    }
}

// 양옆 벽과 지붕으로 앞뒤가 뚫린 터널을 그린다.
void DrawTunnelScene()
{
    glm::mat4 parent = glm::translate(glm::mat4(1.0f), tunnel.position);
    parent = glm::scale(parent, glm::vec3(tunnel.scale));
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
    const glm::mat4 parent = glm::translate(glm::mat4(1.0f), seesaw.position);
    DrawPartScene(parent, {0.0f, height * 0.5f, 0.0f}, {0.7f, height, 0.45f}, obstacleColor);
    glm::mat4 board = glm::translate(parent, {0.0f, height, 0.0f});
    board = glm::rotate(board, glm::radians(seesaw.angle), {1.0f, 0.0f, 0.0f});
    DrawPartScene(board, {0.0f, 0.0f, 0.0f}, {seesawHalfWidth * 2.0f, seesawThickness, seesawHalfLength * 2.0f},
        obstacleColor);
}

// 큐브로 몸통과 머리와 꼬리 및 눈과 귀와 네 다리를 그린다.
void DrawDogScene()
{
    glm::mat4 parent = glm::translate(glm::mat4(1.0f), dog.position);
    parent = glm::rotate(parent, glm::radians(dog.heading), {0.0f, 1.0f, 0.0f});
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

    const glm::vec3 step = movement * dogMoveSpeed * deltaTime;
    for (int axis : {0, 2})
    {
        glm::vec3 next = dog.position;
        next[axis] = std::clamp(next[axis] + step[axis],
            -floorHalfSize + 0.9f, floorHalfSize - 0.9f);
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
    if (!dog.walking && glm::length(movement) > 0.0f)
    {
        dog.direction = -movement;
        dog.heading = glm::degrees(std::atan2(dog.direction.x, dog.direction.z));
    }
    if (dog.walking && dog.grounded)
        dog.walkPhase = std::fmod(dog.walkPhase + 12.0f * deltaTime, glm::radians(360.0f));
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
    for (Hurdle& hurdle : hurdles)
    {
        const float before = previous.x - hurdle.position.x;
        const float after = dog.position.x - hurdle.position.x;
        const bool crossed = (before < 0.0f && after >= 0.0f) || (before > 0.0f && after <= 0.0f);
        if (crossed && std::abs(dog.position.z - hurdle.position.z) < hurdleHalfWidth - dogRadius
            && std::min(previous.y, dog.position.y) > hurdle.height + hurdleThickness * 0.5f)
            hurdle.cleared = true;
    }

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
        tunnel.cleared = true;
        tunnel.entrySide = 0;
    }
    if (tunnel.entrySide == -1 && local.z > halfDepth + dogHalfDepth)
    {
        tunnel.cleared = true;
        tunnel.entrySide = 0;
    }
}

// 카메라 공전과 월드 Z축 이동을 갱신한다.
void UpdateCamera(float deltaTime)
{
    if (camera.orbitSpeed != 0.0f)
    {
        const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(camera.orbitSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
        camera.position = glm::vec3(rotation * glm::vec4(camera.position, 1.0f));
        camera.target = glm::vec3(rotation * glm::vec4(camera.target, 1.0f));
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        const bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
            || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        glm::vec3 next = camera.position;
        next.z += (shift ? 1.0f : -1.0f) * 4.0f * deltaTime;
        const float distance = glm::length(next - camera.target);
        if (distance > 2.0f && distance < 28.0f) camera.position = next;
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

// 시작과 정지 및 점프와 카메라 공전과 리셋을 처리한다.
void keyCallBack(GLFWwindow* currentWindow, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_P)
    {
        dog.running = !dog.running;
        dog.walking = false;
    }
    if (key == GLFW_KEY_SPACE) StartJump();
    if (key == GLFW_KEY_W) dog.direction = {0.0f, 0.0f, -1.0f};
    if (key == GLFW_KEY_S) dog.direction = {0.0f, 0.0f, 1.0f};
    if (key == GLFW_KEY_A) dog.direction = {-1.0f, 0.0f, 0.0f};
    if (key == GLFW_KEY_D) dog.direction = {1.0f, 0.0f, 0.0f};
    dog.heading = glm::degrees(std::atan2(dog.direction.x, dog.direction.z));
    if (key == GLFW_KEY_R)
    {
        const float speed = (mods & GLFW_MOD_SHIFT) ? -35.0f : 35.0f;
        camera.orbitSpeed = camera.orbitSpeed == speed ? 0.0f : speed;
    }
    if (key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) ResetScene();
    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(currentWindow, GLFW_TRUE);
}

// 카메라 행렬을 보내고 바닥과 장애물 및 강아지를 그린다.
void DrawScene()
{
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
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
