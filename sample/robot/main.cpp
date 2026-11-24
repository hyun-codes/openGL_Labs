// 이 파일은 DogAgilityPractice/main.cpp에 복사할 로봇 부품이다.
// 이 파일 아래의 실제 함수는 DrawRobotScene과 UpdateRobot 두 개이다.
// include와 실행용 main을 추가할 필요가 없다.
// 설명에 적힌 코드를 실제 main.cpp로 옮길 때는 앞의 주석 표시를 제거한다.
//
// 붙여 넣을 위치
// DogAgilityPractice/main.cpp에서 void Update(float deltaTime)의 함수 정의를 찾는다.
// 그 바로 위에 아래 실제 함수 두 개를 붙인다.
// 기존 DrawPartScene과 UpdateDog가 정의된 뒤이므로 별도 함수 선언이 필요 없다.
//
// 전역변수 추가
// main 함수 위에서 Camera camera; 같은 전역변수가 있는 곳에 아래를 추가한다.
// float robotLimbAngle = 0.0f;
// dog 구조체는 로봇의 위치와 방향 및 점프 상태를 저장하는 용도로 재사용한다.
// 시작 배치는 initialDogPosition이며 처음 방향은 initialDogDirection이다.
//
// 충돌 크기 교체
// 상단에서 const float dogHalfWidth = 로 시작하는 줄을 찾는다.
// 그 줄부터 const float dogHeight = 의 계산이 끝나는 세미콜론까지 아래로 교체한다.
// const float dogHalfWidth = 0.4f;
// const float dogHalfDepth = 0.32f;
// const float dogRadius = dogHalfWidth;
// const float dogHeight = 1.25f;
// 크기만 바꿔도 예전 강아지의 긴 충돌 범위가 남지 않도록 맞추는 작업이다.
// 아래 DrawRobotScene의 모델 크기를 크게 바꾸면 이 폭과 깊이 및 높이도 맞춘다.
//
// 기존 강아지 모델 함수 삭제
// void DrawDogScene()의 함수 정의를 여는 중괄호부터 짝이 맞는 닫는 중괄호까지 삭제한다.
// UpdateRobot은 기존 UpdateDog를 호출하므로 UpdateDog 함수가 필요하다.
// StartJump와 IsBlocked 및 GetGroundHeight도 기존 점프와 충돌에 사용하는 함수이다.
//
// 그리기 호출 교체
// void DrawScene() 안의 다음 한 줄을 찾는다.
// DrawDogScene();
// 그 줄을 아래로 교체한다.
// DrawRobotScene(dog.position, dog.heading, robotLimbAngle);
//
// 갱신 호출 교체
// void Update(float deltaTime) 안의 다음 한 줄을 찾는다.
// UpdateDog(deltaTime, dog.direction);
// 그 줄을 아래로 교체한다.
// UpdateRobot(deltaTime, robotLimbAngle);
// 이 줄 앞의 const glm::vec3 previous = dog.position;는 통과 판정에 필요하다.
// 이 줄 뒤의 시소 각도 계산과 UpdateObstacles(previous) 및 UpdateCamera는 계속 사용한다.
// 이 연결로 로봇도 허들을 넘고 터널을 통과하며 시소 위에서 이동한다.
//
// 리셋 연결
// void ResetScene()의 닫는 중괄호 바로 위에 아래 한 줄을 추가한다.
// robotLimbAngle = 0.0f;
// 기존 dog = Dog{};가 로봇의 위치와 이동 및 점프 상태를 초기화한다.
//
// 키 입력
// 기존 keyCallBack에서 P는 시작과 정지이며 WASD는 진행 방향이고 Space는 점프이다.
// P를 바꾸려면 GLFW_KEY_P를 바꾸고 점프 키는 GLFW_KEY_SPACE를 바꾼다.
// 방향 키는 GLFW_KEY_W와 GLFW_KEY_S 및 GLFW_KEY_A와 GLFW_KEY_D를 바꾼다.
// C와 R 회전 및 X와 Y와 Z 이동은 기존 카메라 코드에 연결되어 있다.
//
// 값 수정 위치
// 걷기 속도는 dogMoveSpeed이며 점프는 jumpSpeed와 gravity로 조절한다.
// 몸통과 머리 크기 및 RGB는 DrawRobotScene에서 바꾼다.
// 다리와 팔 길이는 legLength와 armLength이며 최대 흔들림은 UpdateRobot의 maxAngle이다.
//

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
