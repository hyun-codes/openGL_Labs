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

// 로봇의 발바닥 중심 위치와 이동 및 관절 애니메이션 상태를 관리한다.
struct Robot
{
    glm::vec3 position{-3.5f,0,2.5f};
    float rotateAngleY = 0;
    float moveSpeed = 2.5f;
    float walkPhase = 0;
    float limbSwingAngle = 0;
    float jumpVelocity = 0;
    bool isGrounded = true;
    bool isMoving = false;
};
Robot robot;
const float jumpSpeed = 6.0f;
const float gravity = 12.0f;

void UpdateRobotJump(float deltaTime);
void UpdateRobotWalking(float deltaTime);
void DrawRobotScene();
void DrawRobotLimb(const glm::mat4& parent, const glm::vec3& joint, float angle,
    float length, float width, const glm::vec3& color);

// 그네 윗부분의 고정축과 흔들림 각도 및 시작 상태를 관리한다.
struct Swing
{
    glm::vec3 pivot{0,4.5f,0};
    float phase = 0;
    float angle = 0;
    float maxAngle = 35;
    float phaseSpeed = 100;
    bool isSwinging = true;
};
Swing swing;

void DrawSwingFrameScene();
void DrawSwingScene();

// 윈도우와 버퍼를 준비한 뒤 시간에 따라 갱신하고 화면을 그린다.
int main()
{
    if (!InitOpenGL()) return -1;
    InitBuffer();
    ResetScene();
    std::cout << "Arrows: move robot | J: robot jump | Space: pause/resume | C/I: reset | ESC/Q: exit\n";
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
    window = glfwCreateWindow(1000, 800, "SwingPractice", nullptr, nullptr);
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
    const glm::vec3 faceColors[6] = { {.85f,.85f,.85f}, {.7f,.7f,.7f}, {.75f,.75f,.75f}, {.95f,.95f,.95f}, {1,1,1}, {.6f,.6f,.6f} };
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

// 기구와 로봇의 위치 및 점프와 카메라를 처음 상태로 되돌린다.
void ResetScene()
{
    swing = Swing{};
    robot = Robot{};
    ResetCamera();
}

// 방향키 입력으로 같은 방 안의 로봇을 이동시키고 진행 방향으로 회전시킨다.
void ProcessInput(float deltaTime)
{
    glm::vec3 movement(0);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) movement.z -= 1;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) movement.z += 1;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) movement.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) movement.x += 1;
    glm::vec3 previous = robot.position;
    if (glm::length(movement) > 0)
    {
        movement = glm::normalize(movement);
        robot.rotateAngleY = glm::degrees(std::atan2(-movement.x, -movement.z));
        robot.position += movement * robot.moveSpeed * deltaTime;
        // 팔다리가 벽을 넘어가지 않도록 로봇의 여유 폭을 남긴다.
        robot.position.x = std::clamp(robot.position.x, -roomHalfWidth + .6f, roomHalfWidth - .6f);
        robot.position.z = std::clamp(robot.position.z, -roomHalfDepth + .6f, roomHalfDepth - .6f);
    }
    robot.isMoving = glm::length(robot.position - previous) > .000001f;
}

// 사인 함수로 양쪽 끝에서 느려지고 가운데에서 빨라지는 흔들림을 만든다.
void Update(float deltaTime)
{
    deltaTime = std::clamp(deltaTime, 0.0f, .05f);
    ProcessInput(deltaTime);
    UpdateCamera(deltaTime);
    // 그네나 시소가 일시정지되어 있어도 로봇의 이동과 점프는 계속 갱신한다.
    UpdateRobotJump(deltaTime);
    UpdateRobotWalking(deltaTime);
    if (!swing.isSwinging) return;
    swing.phase = std::fmod(swing.phase + swing.phaseSpeed * deltaTime, 360.0f);
    swing.angle = swing.maxAngle * std::sin(glm::radians(swing.phase));
}

// 그네의 고정 기둥과 받침대 및 가로대를 그린다.
void DrawSwingFrameScene()
{
    glm::mat4 identity(1);
    for (float side : {-1.0f,1.0f})
    {
        DrawPartScene(identity, {side * 2,2.35f,0}, {.28f,4.7f,.28f}, {.2f,.5f,.8f});
        DrawPartScene(identity, {side * 2,.1f,0}, {.6f,.2f,3.2f}, {.15f,.3f,.5f});
    }
    DrawPartScene(identity, {0,4.7f,0}, {4.6f,.3f,.35f}, {.2f,.5f,.8f});
    DrawPartScene(identity, swing.pivot, {1.9f,.14f,.14f}, {1,.75f,.2f});
}

// 줄과 의자에 같은 부모 행렬을 적용하여 윗축을 중심으로 함께 움직인다.
void DrawSwingScene()
{
    DrawSwingFrameScene();
    glm::mat4 swingTransform = glm::translate(glm::mat4(1), swing.pivot);
    swingTransform = glm::rotate(swingTransform, glm::radians(swing.angle), glm::vec3(1,0,0));
    // 줄의 위쪽 끝을 회전축에 맞추고 아래쪽 끝에 의자를 배치한다.
    DrawPartScene(swingTransform, {-.75f,-1.5f,0}, {.055f,3,.055f}, {.9f,.9f,.9f});
    DrawPartScene(swingTransform, {.75f,-1.5f,0}, {.055f,3,.055f}, {.9f,.9f,.9f});
    DrawPartScene(swingTransform, {0,-3,0}, {1.9f,.18f,.8f}, {1,.35f,.15f});
}

// 흔들림을 현재 각도에서 멈추거나 이어서 시작한다.
void keyCallBack(GLFWwindow* currentWindow, int key, int scancode, int action, int mods)
{
    (void)scancode;
    if (action != GLFW_PRESS) return;
    ProcessCameraKey(key, mods);
    // Space는 기구 정지에 사용하고 J로 로봇의 단일 점프를 시작한다.
    if (key == GLFW_KEY_J && robot.isGrounded)
    {
        robot.jumpVelocity = jumpSpeed;
        robot.isGrounded = false;
    }
    if (key == GLFW_KEY_SPACE) swing.isSwinging = !swing.isSwinging;
    if (key == GLFW_KEY_C || key == GLFW_KEY_I) ResetScene();
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) glfwSetWindowShouldClose(currentWindow, GLFW_TRUE);
}

// 위쪽 속도와 중력으로 로봇을 점프시키고 방 바닥에 착지시킨다.
void UpdateRobotJump(float deltaTime)
{
    if (robot.isGrounded) return;
    robot.jumpVelocity -= gravity * deltaTime;
    robot.position.y += robot.jumpVelocity * deltaTime;
    if (robot.position.y <= 0)
    {
        robot.position.y = 0;
        robot.jumpVelocity = 0;
        robot.isGrounded = true;
    }
}

// 로봇이 이동하는 동안 팔과 다리가 서로 반대로 흔들리게 한다.
void UpdateRobotWalking(float deltaTime)
{
    if (robot.isMoving)
    {
        robot.walkPhase = std::fmod(robot.walkPhase + 400.0f * deltaTime, 360.0f);
        robot.limbSwingAngle = 30.0f * std::sin(glm::radians(robot.walkPhase));
    }
    else robot.limbSwingAngle = 0;
}

// 로봇의 어깨와 골반을 기준으로 팔다리를 회전시킨다.
void DrawRobotLimb(const glm::mat4& parent, const glm::vec3& joint, float angle,
    float length, float width, const glm::vec3& color)
{
    glm::mat4 limbTransform = glm::translate(parent, joint);
    limbTransform = glm::rotate(limbTransform, glm::radians(angle), glm::vec3(1,0,0));
    DrawPartScene(limbTransform, {0,-length * .5f,0}, {width,length,width}, color);
}

// 그네나 시소와 별개로 로봇 전체와 각 부품의 변환을 적용하여 그린다.
void DrawRobotScene()
{
    glm::mat4 robotTransform = glm::translate(glm::mat4(1), robot.position);
    robotTransform = glm::rotate(robotTransform, glm::radians(robot.rotateAngleY), glm::vec3(0,1,0));
    // 몸통과 머리 및 앞뒤를 구분하는 코를 배치한다.
    DrawPartScene(robotTransform, {0,1.15f,0}, {.65f,.8f,.4f}, {.2f,.6f,1});
    DrawPartScene(robotTransform, {0,1.82f,0}, {.5f,.5f,.5f}, {1,.8f,.5f});
    DrawPartScene(robotTransform, {0,1.8f,-.34f}, {.16f,.15f,.22f}, {1,.3f,.1f});
    // 같은 쪽 팔과 다리의 회전 부호를 반대로 설정한다.
    float angle = robot.limbSwingAngle;
    DrawRobotLimb(robotTransform, {-.46f,1.5f,0}, -angle, .75f,.2f, {1,.2f,.2f});
    DrawRobotLimb(robotTransform, {.46f,1.5f,0}, angle, .75f,.2f, {.2f,.9f,.3f});
    DrawRobotLimb(robotTransform, {-.2f,.75f,0}, angle, .75f,.24f, {.8f,.2f,.9f});
    DrawRobotLimb(robotTransform, {.2f,.75f,0}, -angle, .75f,.24f, {1,.8f,.15f});
}

// 예제에 맞는 초기 시점과 직교하는 카메라 위쪽 방향을 설정한다.
void ResetCamera()
{
    camera = Camera{};
    camera.target.y = 2.3f;
    camera.position = {7,5.5f,10};
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
    DrawSwingScene();
    DrawRobotScene();
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

