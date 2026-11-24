#version 330 core
// 위치는 C++의 속성 0에서 받고 법선은 속성 2에서 받는다.
layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec3 vNormal;

// 각 부품의 배치와 크기 및 회전은 C++의 모델 행렬에서 정한다.
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
out vec3 worldNormal;

void main()
{
    // 축마다 다른 크기를 적용해도 면의 법선 방향을 유지한다.
    worldNormal = transpose(inverse(mat3(modelTransform))) * vNormal;
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0);
}
