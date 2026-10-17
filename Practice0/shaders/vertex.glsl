#version 330 core

// 앞으로 C++의 VAO/VBO에서 전달받을 정점 위치
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;

out vec3 passColor;

void main()
{
    gl_Position = vec4(vPosition, 1.0);
    passColor = vColor;
}
