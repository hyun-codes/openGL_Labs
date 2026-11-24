// 강아지 실습의 걷기와 점프를 재사용할 간단한 로봇 모델과 동작 함수이다.
// 이 파일의 함수 두 개를 DogAgilityPractice/main.cpp에 복사한다.
// 기존 DrawPartScene과 큐브 VAO 및 셰이더를 그대로 사용한다.
// 기존 dog 구조체는 로봇의 위치와 방향 및 점프 상태를 저장하는 용도로 사용한다.
//
// main 위의 전역변수 부분에 아래 한 줄을 추가한다.
// float robotLimbAngle = 0.0f;
//
// 상단의 initialDogPosition 값으로 로봇 시작 위치를 바꾼다.
// initialDogDirection으로 처음 진행 방향을 바꾼다.
// dogMoveSpeed와 jumpSpeed 및 gravity로 걷기와 점프를 조절한다.
//
// 상단의 dogHalfWidth 정의부터 dogHeight 정의 끝까지 아래 네 줄로 교체한다.
// const float dogHalfWidth = 0.4f;
// const float dogHalfDepth = 0.32f;
// const float dogRadius = dogHalfWidth;
// const float dogHeight = 1.25f;
// 이 값은 아래 로봇의 크기에 맞춘 충돌 범위이며 모델을 크게 바꾸면 함께 조절한다.
//
// 이 파일의 함수 두 개는 기존 Update 함수 바로 위에 붙여 넣는다.
// DrawPartScene과 UpdateDog 정의보다 아래에 두면 별도 함수 선언이 필요 없다.
//
// 기존 DrawDogScene 함수 정의를 지우고 아래 DrawRobotScene을 사용한다.
// DrawScene에서 DrawDogScene(); 줄을 아래 줄로 교체한다.
// DrawRobotScene(dog.position, dog.heading, robotLimbAngle);
//
// Update에서 UpdateDog(deltaTime, dog.direction); 한 줄을 아래 줄로 교체한다.
// UpdateRobot(deltaTime, robotLimbAngle);
// previous 위치 저장과 UpdateObstacles 및 시소 판정과 UpdateCamera는 그대로 사용한다.
// 로봇이 장애물을 넘을 때도 기존 충돌과 통과 판정이 함께 적용된다.
//
// ResetScene 안에 robotLimbAngle = 0.0f; 한 줄을 추가한다.
// 기존 P와 WASD 및 Space 입력은 그대로 로봇 걷기와 방향 변경 및 점프에 사용한다.
// 입력 키는 기존 keyCallBack의 GLFW_KEY_P와 WASD 및 GLFW_KEY_SPACE를 바꾼다.
// 카메라 입력도 기존 UpdateCamera와 keyCallBack을 그대로 사용한다.
//
// 모델의 색과 부품 크기는 DrawRobotScene 안의 값을 바꾼다.
// 팔과 다리의 최대 각도는 UpdateRobot 안의 maxAngle 값을 바꾼다.

// 몸통과 머리 및 팔과 다리와 눈을 큐브로 조립한다.
void DrawRobotScene(const glm::vec3& position, float heading, float limbAngle)
{
    const glm::vec3 bodyColor(0.2f, 0.45f, 0.9f); // 몸통 RGB
    const glm::vec3 headColor(0.85f, 0.7f, 0.4f); // 머리 RGB
    const glm::vec3 limbColor(0.4f, 0.45f, 0.5f); // 팔과 다리 RGB
    const glm::vec3 eyeColor(0.05f, 0.05f, 0.05f); // 눈 RGB
    const glm::vec3 bodySize(0.45f, 0.5f, 0.3f); // 몸통 가로와 높이 및 깊이
    const glm::vec3 headSize(0.35f, 0.35f, 0.35f); // 머리 가로와 높이 및 깊이
    const float legLength = 0.5f; // 바닥에서 골반까지 높이
    const float armLength = 0.4f; // 어깨에서 손까지 길이
    const float bodyCenterY = legLength + bodySize.y * 0.5f;
    const float headCenterY = legLength + bodySize.y + headSize.y * 0.5f - 0.1f;

    glm::mat4 parent = glm::translate(glm::mat4(1.0f), position);
    parent = glm::rotate(parent, glm::radians(heading), {0.0f, 1.0f, 0.0f});
    DrawPartScene(parent, {0.0f, bodyCenterY, 0.0f}, bodySize, bodyColor);
    DrawPartScene(parent, {0.0f, headCenterY, 0.0f}, headSize, headColor);

    for (float side : {-1.0f, 1.0f})
    {
        DrawPartScene(parent, {side * 0.09f, headCenterY + 0.04f, headSize.z * 0.5f + 0.01f},
            {0.05f, 0.05f, 0.03f}, eyeColor);

        // 다리는 골반 관절을 기준으로 좌우가 반대로 움직인다.
        glm::mat4 leg = glm::translate(parent, {side * 0.14f, legLength, 0.0f});
        leg = glm::rotate(leg, glm::radians(limbAngle * side), {1.0f, 0.0f, 0.0f});
        DrawPartScene(leg, {0.0f, -legLength * 0.5f, 0.0f}, {0.16f, legLength, 0.18f}, limbColor);

        // 팔은 어깨 관절을 기준으로 같은 쪽 다리와 반대로 움직인다.
        glm::mat4 arm = glm::translate(parent, {side * 0.32f, legLength + bodySize.y - 0.06f, 0.0f});
        arm = glm::rotate(arm, glm::radians(-limbAngle * side), {1.0f, 0.0f, 0.0f});
        DrawPartScene(arm, {0.0f, -armLength * 0.5f, 0.0f}, {0.14f, armLength, 0.16f}, limbColor);
    }
}

// 기존 이동과 점프를 실행한 뒤 로봇 팔과 다리의 각도를 계산한다.
void UpdateRobot(float deltaTime, float& limbAngle)
{
    const float maxAngle = 25.0f; // 걷는 동안 팔과 다리가 흔들리는 최대 각도
    UpdateDog(deltaTime, dog.direction);
    limbAngle = dog.walking && dog.grounded ? maxAngle * std::sin(dog.walkPhase) : 0.0f;
}
