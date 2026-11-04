#version 330 core

// VAO의 위치 속성에서 정점 좌표를 읽는다.
layout(location = 0) in vec3 vPosition;
// VAO의 색상 속성에서 정점 색상을 읽는다.
layout(location = 1) in vec3 vColor;
// 도형 변환과 카메라 변환 및 투영 변환을 C++에서 전달받는다.
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
out vec3 vertexColor;

// 모델과 뷰 및 투영 순서로 좌표를 변환하고 색상을 다음 단계로 전달한다.
void main()
{
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0);
    vertexColor = vColor;
}
