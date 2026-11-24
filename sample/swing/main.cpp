// 이 파일은 DogAgilityPractice/main.cpp에 복사할 그네 부품이다.
// 이 파일 아래의 실제 함수는 DrawSwingScene과 UpdateSwing 두 개이다.
// include와 실행용 main을 추가할 필요가 없다.
// 설명에 적힌 코드를 실제 main.cpp로 옮길 때는 앞의 주석 표시를 제거한다.
//
// 붙여 넣을 위치
// DogAgilityPractice/main.cpp에서 void Update(float deltaTime)의 함수 정의를 찾는다.
// 그 바로 위에 아래 실제 함수 두 개를 붙인다.
// 이 위치에서는 DrawPartScene과 필요한 OpenGL 초기화가 이미 준비되어 있다.
//
// 전역변수 추가
// main 함수 위에서 Camera camera; 같은 전역변수가 있는 곳에 아래를 추가한다.
// glm::vec3 swingPosition(0.0f, 0.0f, 3.0f);
// float swingPhase = 0.0f;
// float swingAngle = 0.0f;
// bool swingMoving = false;
// swingPosition은 그네 전체 배치이며 X와 Z로 바닥 위 위치를 바꾼다.
//
// 기존 강아지 모델 함수 삭제
// void DrawDogScene()의 함수 정의를 여는 중괄호부터 짝이 맞는 닫는 중괄호까지 삭제한다.
// 기존 DrawPartScene과 DrawCubeScene은 그네 부품을 그릴 때 사용하는 함수이다.
//
// 그리기 호출 교체
// void DrawScene() 안의 다음 한 줄을 찾는다.
// DrawDogScene();
// 그 줄을 아래로 교체한다.
// DrawSwingScene(swingPosition, swingAngle);
// 카메라 행렬을 보내고 바닥과 장애물을 그리는 나머지 호출은 함께 사용할 수 있다.
//
// 갱신 함수 교체
// void Update(float deltaTime)의 기존 함수 정의 전체를 아래로 교체한다.
// 이 교체로 강아지 이동과 강아지에 묶인 시소 및 통과 판정을 제거한다.
// 터널의 반복 애니메이션과 카메라는 계속 갱신한다.
// void Update(float deltaTime)
// {
//     tunnel.phase = std::fmod(tunnel.phase + tunnelScaleSpeed * deltaTime, glm::radians(360.0f));
//     tunnel.scale = 1.0f + tunnelScaleAmount * std::sin(tunnel.phase);
//     UpdateSwing(deltaTime, swingMoving, swingPhase, swingAngle);
//     UpdateCamera(deltaTime);
// }
//
// 키 콜백 교체
// void keyCallBack(GLFWwindow* currentWindow, int key, int, int action, int mods)의
// 기존 함수 정의 전체를 아래로 교체한다.
// P는 그네 시작과 정지이며 C와 R은 카메라 회전이다.
// X와 Y 및 Z 이동은 기존 UpdateCamera에서 처리한다.
// 시작 키를 바꾸려면 아래 GLFW_KEY_P를 바꾼다.
// void keyCallBack(GLFWwindow* currentWindow, int key, int, int action, int mods)
// {
//     if (action != GLFW_PRESS) return;
//     if (key == GLFW_KEY_P) swingMoving = !swingMoving;
//     const float rotationSpeed = (mods & GLFW_MOD_SHIFT) ? -cameraRotateSpeed : cameraRotateSpeed;
//     if (key == GLFW_KEY_C)
//     {
//         camera.selfRotateSpeed = camera.selfRotateSpeed == rotationSpeed ? 0.0f : rotationSpeed;
//     }
//     if (key == GLFW_KEY_R)
//     {
//         camera.orbitSpeed = camera.orbitSpeed == rotationSpeed ? 0.0f : rotationSpeed;
//     }
//     if (key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) ResetScene();
//     if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
//         glfwSetWindowShouldClose(currentWindow, GLFW_TRUE);
// }
//
// 리셋 연결
// void ResetScene()의 닫는 중괄호 바로 위에 아래 세 줄을 추가한다.
// swingPhase = 0.0f;
// swingAngle = 0.0f;
// swingMoving = false;
// 기존 숫자 0 입력이 ResetScene을 호출하므로 그네도 함께 초기화된다.
//
// 값 수정 위치
// 배치는 위에 추가한 swingPosition을 바꾼다.
// 기둥 높이와 줄 길이 및 좌석 크기와 RGB는 DrawSwingScene에서 바꾼다.
// 최대 왕복 각도와 속도는 UpdateSwing에서 바꾼다.
//

// 두 기둥과 가로대 및 줄 두 개와 좌석을 큐브로 조립한다.
void DrawSwingScene(const glm::vec3& position, float angle)
{
    const float frameHeight = 3.0f; // 기둥 높이와 줄이 매달리는 높이
    const float frameHalfWidth = 1.4f; // 두 기둥 중심 간격의 절반
    const float ropeLength = 2.0f; // 줄 길이이며 frameHeight보다 작게 설정
    const float ropeHalfWidth = 0.5f; // 줄 두 개 사이 간격의 절반
    const glm::vec3 frameColor(0.2f, 0.4f, 0.85f); // 기둥과 가로대 RGB
    const glm::vec3 ropeColor(0.3f, 0.3f, 0.3f); // 줄 RGB
    const glm::vec3 seatColor(0.9f, 0.55f, 0.15f); // 좌석 RGB
    const glm::vec3 seatSize(1.3f, 0.15f, 0.6f); // 좌석 가로와 두께 및 깊이

    const glm::mat4 frame = glm::translate(glm::mat4(1.0f), position);
    for (float side : {-1.0f, 1.0f})
    {
        DrawPartScene(frame, {side * frameHalfWidth, frameHeight * 0.5f, 0.0f},
            {0.2f, frameHeight, 0.25f}, frameColor);
    }
    DrawPartScene(frame, {0.0f, frameHeight, 0.0f},
        {frameHalfWidth * 2.0f + 0.2f, 0.2f, 0.25f}, frameColor);

    // 크기 변환 전의 매다는 지점을 부모로 삼아 줄과 좌석만 회전시킨다.
    glm::mat4 swing = glm::translate(frame, {0.0f, frameHeight, 0.0f});
    swing = glm::rotate(swing, glm::radians(angle), {1.0f, 0.0f, 0.0f});
    for (float side : {-1.0f, 1.0f})
    {
        DrawPartScene(swing, {side * ropeHalfWidth, -ropeLength * 0.5f, 0.0f},
            {0.05f, ropeLength, 0.05f}, ropeColor);
    }
    DrawPartScene(swing, {0.0f, -ropeLength, 0.0f}, seatSize, seatColor);
}

// 시작 상태에서만 각도를 갱신하며 멈추면 현재 자세를 유지한다.
void UpdateSwing(float deltaTime, bool moving, float& phase, float& angle)
{
    const float maxAngle = 25.0f; // 그네가 앞뒤로 기울어지는 최대 각도
    const float phaseSpeed = 90.0f; // 커질수록 빠르게 왕복
    if (!moving) return;

    phase = std::fmod(phase + phaseSpeed * deltaTime, 360.0f);
    angle = maxAngle * std::sin(glm::radians(phase));
}
