// 강아지 실습에 이식할 간단한 그네 모델과 동작 함수이다.
// 이 파일의 함수 두 개를 DogAgilityPractice/main.cpp에 복사한다.
// 기존 DrawPartScene과 큐브 VAO 및 셰이더를 그대로 사용한다.
//
// main 위의 전역변수 부분에 아래 네 줄을 추가한다.
// glm::vec3 swingPosition(0.0f, 0.0f, 0.0f);
// float swingPhase = 0.0f;
// float swingAngle = 0.0f;
// bool swingMoving = false;
// swingPosition의 X와 Z로 그네 배치를 바꾸며 Y는 바닥 높이이다.
//
// 이 파일의 함수 두 개는 기존 Update 함수 바로 위에 붙여 넣는다.
// 기존 DrawPartScene 정의보다 아래에 두면 별도 함수 선언이 필요 없다.
//
// 기존 DrawDogScene 함수 정의를 지우고 아래 DrawSwingScene을 사용한다.
// DrawScene에서 DrawDogScene(); 줄을 아래 줄로 교체한다.
// DrawSwingScene(swingPosition, swingAngle);
// 바닥만 필요하면 DrawScene의 DrawHurdleScene과 DrawTunnelScene 및 DrawSeesawScene 호출도 뺀다.
//
// Update 함수를 아래 내용으로 교체한다.
// void Update(float deltaTime)
// {
//     UpdateSwing(deltaTime, swingMoving, swingPhase, swingAngle);
//     UpdateCamera(deltaTime);
// }
// 카메라 입력과 바닥 그리기는 기존 코드를 사용한다.
//
// keyCallBack에서 P를 검사하는 기존 if 블록을 아래 한 줄로 교체한다.
// if (key == GLFW_KEY_P) swingMoving = !swingMoving;
// P 대신 다른 키를 쓰려면 이 줄의 GLFW_KEY_P를 바꾼다.
// 같은 함수의 Space 검사부터 WASD 검사와 dog.heading 대입까지는 삭제한다.
// C와 R의 카메라 회전 및 0과 Q의 처리는 그대로 사용한다.
//
// ResetScene 안에 아래 세 줄을 추가한다.
// swingPhase = 0.0f;
// swingAngle = 0.0f;
// swingMoving = false;
//
// 모델의 색과 길이는 DrawSwingScene 안의 값을 바꾼다.
// 흔들리는 최대 각도와 속도는 UpdateSwing 안의 값을 바꾼다.

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
