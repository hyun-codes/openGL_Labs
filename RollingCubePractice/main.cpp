#include "PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

// 카메라의 위치와 바라보는 위치를 관리한다.
struct Camera
{
    glm::vec3 position{7.0f, 5.5f, 9.0f};
    glm::vec3 target{0.0f, 1.8f, 0.0f};
    glm::vec3 up{0,1,0};
    float rotateVelocityY = 0;
    float orbitVelocityY = 0;
    float moveSpeed = 3;
    float zoomSpeed = 4;
    float fovDegrees = 45;
    float nearPlane = .1f;
    float farPlane = 100;
    bool isPerspective = true;
};

// OpenGL 윈도우와 셰이더 및 공통 육면체 버퍼를 관리한다.
GLFWwindow* window = nullptr;
GLuint shaderProgram = 0;
Mesh cubeMesh;
Mesh gridMesh;
// 방의 왼쪽과 오른쪽 및 바닥과 천장과 앞뒤 벽을 각각 저장한다.
std::array<Mesh,6> roomMeshes;
const float roomHalfWidth = 6.0f;
const float roomHalfDepth = 6.0f;
const float roomHeight = 6.0f;
Camera camera;
int framebufferWidth = 1000;
int framebufferHeight = 800;

bool InitOpenGL();
void InitBuffer();
void InitCubeBuffer();
void InitGridBuffer();
void InitRoomBuffer();
void ResetScene();
void ProcessInput(float deltaTime);
void ResetCamera();
void ProcessCameraInput(float deltaTime);
void UpdateCamera(float deltaTime);
void ProcessCameraKey(int key, int mods);
void Update(float deltaTime);
void DrawScene();
void DrawRoomScene();
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color);
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center,
    const glm::vec3& size, const glm::vec3& color);
void Cleanup();
void keyCallBack(GLFWwindow* currentWindow, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* currentWindow, int width, int height);

// 정육면체 중심과 누적 회전 및 현재 모서리 회전 각도를 관리한다.
struct RollingCube
{
    glm::vec3 position{-2,.5f,2};
    glm::mat4 orientation{1};
    float rollAngle = 0;
    float rollSpeed = 100;
    int directionIndex = 0;
    // 누른 키의 요청 방향은 현재 진행 중인 한 칸 회전이 끝난 뒤 적용한다.
    int requestedDirection = -1;
    bool isStepActive = false;
    bool isRolling = true;
};
RollingCube cube;
const float cubeSize = 1.0f;
// 방향 번호는 오른쪽과 앞쪽 및 왼쪽과 뒤쪽의 월드 축 방향이다.
const glm::vec3 directions[4] = {{1,0,0},{0,0,-1},{-1,0,0},{0,0,1}};

glm::mat4 GetRollingTransform();
void CompleteRoll();
bool StartRoll();
void DrawRollingCubeScene();

// 윈도우와 버퍼를 준비한 뒤 시간에 따라 갱신하고 화면을 그린다.
int main()
{
    if (!InitOpenGL()) return -1;
    InitBuffer();
    ResetScene();
    std::cout << "W/A/S/D: roll direction | Space: pause/resume | C/I: reset | ESC/Q: exit\n";
    std::cout << "x/X,z/Z: translate | y/Y: self rotate | r/R: orbit | +/-: zoom | p/P: perspective/ortho\n";
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        Update(deltaTime);
        DrawScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    Cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// OpenGL 3.3과 깊이 테스트 및 셰이더를 초기화한다.
bool InitOpenGL()
{
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1000, 800, "RollingCubePractice", nullptr, nullptr);
    if (!window) { glfwTerminate(); return false; }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallBack);
    glfwSetFramebufferSizeCallback(window, framebufferCallBack);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    framebufferCallBack(window, framebufferWidth, framebufferHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (!shaderProgram)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }
    return true;
}

// 객체용 육면체와 방의 여섯 면 및 바닥 격자를 한 번만 저장한다.
void InitBuffer()
{
    InitCubeBuffer();
    InitRoomBuffer();
    InitGridBuffer();
}

// 원점 중심이며 한 변이 1인 육면체의 정점과 색상을 저장한다.
void InitCubeBuffer()
{
    const glm::vec3 points[8] = {
        {-.5f,-.5f,-.5f}, {.5f,-.5f,-.5f}, {.5f,.5f,-.5f}, {-.5f,.5f,-.5f},
        {-.5f,-.5f,.5f}, {.5f,-.5f,.5f}, {.5f,.5f,.5f}, {-.5f,.5f,.5f}
    };
    // 면마다 삼각형 두 개를 바깥에서 보았을 때 반시계 방향으로 배치한다.
    const int indices[36] = {
        0,2,1, 0,3,2, 5,7,4, 5,6,7, 4,3,0, 4,7,3,
        1,6,5, 1,2,6, 3,6,2, 3,7,6, 4,1,5, 4,0,1
    };
    const glm::vec3 faceColors[6] = { {1,.25f,.2f}, {.2f,.7f,1}, {.8f,.2f,1}, {1,.8f,.1f}, {.2f,1,.6f}, {1,.5f,.15f} };
    std::vector<Vertex> vertices;
    for (int i = 0; i < 36; ++i)
    {
        glm::vec3 p = points[indices[i]];
        glm::vec3 c = faceColors[i / 6];
        vertices.push_back({p.x, p.y, p.z, c.r, c.g, c.b});
    }
    InitMeshBuffer(cubeMesh, vertices);
}

// 방의 여섯 면을 안쪽에서 보았을 때 반시계 방향이 되도록 저장한다.
void InitRoomBuffer()
{
    float w = roomHalfWidth, d = roomHalfDepth, h = roomHeight;
    const glm::vec3 corners[6][4] = {
        {{-w,0,d}, {-w,0,-d}, {-w,h,-d}, {-w,h,d}},
        {{w,0,-d}, {w,0,d}, {w,h,d}, {w,h,-d}},
        {{-w,0,d}, {w,0,d}, {w,0,-d}, {-w,0,-d}},
        {{-w,h,-d}, {w,h,-d}, {w,h,d}, {-w,h,d}},
        {{-w,0,-d}, {w,0,-d}, {w,h,-d}, {-w,h,-d}},
        {{w,0,d}, {-w,0,d}, {-w,h,d}, {w,h,d}}
    };
    const int indices[6] = {0,1,2,0,2,3};
    for (int face = 0; face < 6; ++face)
    {
        std::vector<Vertex> vertices;
        for (int index : indices)
        {
            glm::vec3 p = corners[face][index];
            vertices.push_back({p.x,p.y,p.z,1,1,1});
        }
        InitMeshBuffer(roomMeshes[face], vertices);
    }
}

// 이동 거리와 바닥 접촉을 확인할 수 있도록 방 크기에 맞는 격자를 만든다.
void InitGridBuffer()
{
    std::vector<Vertex> vertices;
    int gridLimit = static_cast<int>(std::min(roomHalfWidth, roomHalfDepth));
    for (int i = -gridLimit; i <= gridLimit; ++i)
    {
        float value = static_cast<float>(i);
        vertices.push_back({value,.002f,-roomHalfDepth,1,1,1});
        vertices.push_back({value,.002f,roomHalfDepth,1,1,1});
        vertices.push_back({-roomHalfWidth,.002f,value,1,1,1});
        vertices.push_back({roomHalfWidth,.002f,value,1,1,1});
    }
    InitMeshBuffer(gridMesh, vertices, GL_LINES);
}

// 중심 위치와 누적 회전을 처음 상태로 되돌린다.
void ResetScene()
{
    cube = RollingCube{};
    ResetCamera();
}

// 누르고 있는 WASD 키를 다음 한 칸의 진행 방향으로 저장한다.
void ProcessInput(float deltaTime)
{
    (void)deltaTime;
    cube.requestedDirection = -1;
    // 두 키 이상을 누르면 앞뒤 키를 먼저 선택하고 대각선으로 구르지는 않는다.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cube.requestedDirection = 1;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cube.requestedDirection = 3;
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cube.requestedDirection = 2;
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cube.requestedDirection = 0;
}

// 큐브 전체가 방 안에 들어가는 다음 칸일 때만 새 모서리 회전을 시작한다.
bool StartRoll()
{
    if (cube.requestedDirection < 0) return false;
    glm::vec3 nextPosition = cube.position + directions[cube.requestedDirection] * cubeSize;
    float halfSize = cubeSize * .5f;
    if (std::abs(nextPosition.x) + halfSize > roomHalfWidth
        || std::abs(nextPosition.z) + halfSize > roomHalfDepth) return false;
    cube.directionIndex = cube.requestedDirection;
    cube.isStepActive = true;
    return true;
}

// 회전이 끝난 자세를 저장하고 중심을 정확히 한 칸 이동한다.
void CompleteRoll()
{
    glm::vec3 direction = directions[cube.directionIndex];
    glm::vec3 axis = glm::cross(glm::vec3(0,1,0), direction);
    cube.position += direction * cubeSize;
    cube.orientation = glm::rotate(glm::mat4(1), glm::radians(90.0f), axis) * cube.orientation;
    cube.rollAngle = 0;
    cube.isStepActive = false;
}

// 시작한 한 칸은 끝까지 굴리고 다음 칸에서 현재 누른 키 방향을 적용한다.
void Update(float deltaTime)
{
    deltaTime = std::clamp(deltaTime, 0.0f, .05f);
    ProcessInput(deltaTime);
    UpdateCamera(deltaTime);
    if (!cube.isRolling) return;
    float remainingAngle = cube.rollSpeed * deltaTime;
    while (remainingAngle > 0)
    {
        if (!cube.isStepActive && !StartRoll()) break;
        float stepAngle = std::min(90.0f - cube.rollAngle, remainingAngle);
        cube.rollAngle += stepAngle;
        remainingAngle -= stepAngle;
        // 중간에 키를 놓거나 방향을 바꿔도 현재 접촉 모서리는 끝까지 유지한다.
        if (cube.rollAngle >= 90.0f - .0001f) CompleteRoll();
    }
}

// 바닥에 닿은 진행 방향 모서리를 회전 중심으로 사용한다.
glm::mat4 GetRollingTransform()
{
    glm::vec3 direction = directions[cube.directionIndex];
    glm::vec3 pivot = cube.position + direction * (cubeSize * .5f);
    pivot.y -= cubeSize * .5f;
    glm::vec3 axis = glm::cross(glm::vec3(0,1,0), direction);
    // 모서리를 원점으로 옮겨 회전한 뒤 되돌리고 기존 자세를 결합한다.
    glm::mat4 transform = glm::translate(glm::mat4(1), pivot);
    transform = glm::rotate(transform, glm::radians(cube.rollAngle), axis);
    transform = glm::translate(transform, -pivot);
    transform = transform * glm::translate(glm::mat4(1), cube.position) * cube.orientation;
    return glm::scale(transform, glm::vec3(cubeSize));
}

// 정점 버퍼를 바꾸지 않고 모서리 회전 행렬로 육면체를 그린다.
void DrawRollingCubeScene()
{
    DrawCubeScene(GetRollingTransform(), glm::vec3(1));
}

// 일시정지와 초기화 및 종료 입력을 처리한다.
void keyCallBack(GLFWwindow* currentWindow, int key, int scancode, int action, int mods)
{
    (void)scancode;
    if (action != GLFW_PRESS) return;
    ProcessCameraKey(key, mods);
    if (key == GLFW_KEY_SPACE) cube.isRolling = !cube.isRolling;
    if (key == GLFW_KEY_C || key == GLFW_KEY_I) ResetScene();
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) glfwSetWindowShouldClose(currentWindow, GLFW_TRUE);
}

// 예제에 맞는 초기 시점과 직교하는 카메라 위쪽 방향을 설정한다.
void ResetCamera()
{
    camera = Camera{};

    glm::vec3 forward = glm::normalize(camera.target - camera.position);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
    camera.up = glm::normalize(glm::cross(right, forward));
}

// 소문자는 양의 축 방향으로 이동하고 Shift를 함께 누르면 반대로 이동한다.
void ProcessCameraInput(float deltaTime)
{
    bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    float sign = shift ? -1.0f : 1.0f;
    glm::vec3 movement(0);
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) movement.x += sign;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) movement.z += sign;
    // 시선 목표도 함께 옮겨 카메라 이동만으로 바라보는 방향이 바뀌지 않게 한다.
    camera.position += movement * camera.moveSpeed * deltaTime;
    camera.target += movement * camera.moveSpeed * deltaTime;

    bool zoomIn = glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS;
    bool zoomOut = glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
    float zoom = static_cast<float>(zoomIn) - static_cast<float>(zoomOut);
    glm::vec3 offset = camera.position - camera.target;
    float distance = glm::length(offset);
    float nextDistance = std::clamp(distance - zoom * camera.zoomSpeed * deltaTime, 1.5f, 45.0f);
    camera.position = camera.target + glm::normalize(offset) * nextDistance;
}

// 제자리 회전과 장면 중심 공전을 각각 갱신하고 두 기능을 함께 사용할 수 있게 한다.
void UpdateCamera(float deltaTime)
{
    ProcessCameraInput(deltaTime);
    // 제자리 회전은 카메라 위치를 유지하고 자신의 위쪽 축을 기준으로 시선만 돌린다.
    glm::mat4 selfRotation = glm::rotate(glm::mat4(1),
        glm::radians(camera.rotateVelocityY * deltaTime), camera.up);
    camera.target = camera.position + glm::vec3(
        selfRotation * glm::vec4(camera.target - camera.position, 0));

    // 공전은 월드 원점의 y축을 기준으로 위치와 목표 및 위쪽 방향을 함께 돌린다.
    glm::mat4 orbit = glm::rotate(glm::mat4(1),
        glm::radians(camera.orbitVelocityY * deltaTime), glm::vec3(0,1,0));
    camera.position = glm::vec3(orbit * glm::vec4(camera.position, 1));
    camera.target = glm::vec3(orbit * glm::vec4(camera.target, 1));
    camera.up = glm::normalize(glm::vec3(orbit * glm::vec4(camera.up, 0)));
}

// Shift로 반대 회전을 선택하고 같은 회전 키를 다시 누르면 해당 회전만 멈춘다.
void ProcessCameraKey(int key, int mods)
{
    if (key == GLFW_KEY_Y || key == GLFW_KEY_R)
    {
        float velocity = (mods & GLFW_MOD_SHIFT) ? 35.0f : -35.0f;
        float& currentVelocity = key == GLFW_KEY_Y ? camera.rotateVelocityY : camera.orbitVelocityY;
        currentVelocity = currentVelocity == velocity ? 0.0f : velocity;
    }
    // 소문자 p는 원근 투영이고 Shift와 p는 직각 투영이다.
    if (key == GLFW_KEY_P) camera.isPerspective = (mods & GLFW_MOD_SHIFT) == 0;
}

// 카메라와 선택한 투영을 전달한 뒤 방과 실습 객체를 그린다.
void DrawScene()
{
    glClearColor(.09f,.12f,.18f,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glm::mat4 view = glm::lookAt(camera.position, camera.target, camera.up);
    float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
    glm::mat4 projection;
    if (camera.isPerspective)
        projection = glm::perspective(glm::radians(camera.fovDegrees), aspect, camera.nearPlane, camera.farPlane);
    else
    {
        // 카메라와 목표 사이 거리로 직각 투영 범위를 정해 줌을 동일하게 적용한다.
        float halfHeight = glm::length(camera.position - camera.target) * std::tan(glm::radians(camera.fovDegrees) * .5f);
        projection = glm::ortho(-halfHeight * aspect, halfHeight * aspect,
            -halfHeight, halfHeight, camera.nearPlane, camera.farPlane);
    }
    SetTransformUniforms(shaderProgram, glm::mat4(1), view, projection);
    DrawRoomScene();
    DrawRollingCubeScene();
}

// 가까운 벽의 바깥쪽은 컬링하고 반대편 벽의 안쪽은 보이도록 방을 그린다.
void DrawRoomScene()
{
    const glm::vec3 colors[6] = {
        {.25f,.6f,.35f}, {.25f,.35f,.65f}, {.5f,.65f,.45f},
        {.65f,.6f,.4f}, {.7f,.25f,.25f}, {.3f,.65f,.7f}
    };
    glm::mat4 roomTransform(1);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(roomTransform));
    // 여섯 면을 모두 그려도 카메라 위치와 면의 앞뒤에 따라 보이는 벽이 달라진다.
    for (int face = 0; face < 6; ++face)
    {
        glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(colors[face]));
        DrawMesh(roomMeshes[face]);
    }
    // 방 바닥 바로 위에 격자를 덧그려 이동과 바닥 접촉을 확인한다.
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), .58f,.65f,.62f);
    DrawMesh(gridMesh);
}

// 같은 육면체 버퍼를 다른 모델 행렬과 색상으로 재사용한다.
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
    DrawMesh(cubeMesh);
}

// 부모 행렬에 부품의 위치와 크기를 곱해서 그린다.
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center,
    const glm::vec3& size, const glm::vec3& color)
{
    DrawCubeScene(glm::scale(glm::translate(parent, center), size), color);
}

// 창 크기에 맞춰 뷰포트와 원근 투영에 사용할 비율을 갱신한다.
void framebufferCallBack(GLFWwindow* currentWindow, int width, int height)
{
    (void)currentWindow;
    framebufferWidth = std::max(width, 1);
    framebufferHeight = std::max(height, 1);
    glViewport(0, 0, width, height);
}

// 윈도우를 닫기 전에 GPU 버퍼와 셰이더 프로그램을 해제한다.
void Cleanup()
{
    DeleteMesh(cubeMesh);
    DeleteMesh(gridMesh);
    // 객체뿐 아니라 방의 여섯 면에 사용한 GPU 버퍼도 해제한다.
    for (Mesh& mesh : roomMeshes) DeleteMesh(mesh);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    shaderProgram = 0;
}

