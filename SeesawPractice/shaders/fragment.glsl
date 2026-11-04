#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

uniform vec3 objectColor;

// 전달받은 정점 색상에 객체 색상을 곱한다.
void main()
{
    FragColor = vec4(vertexColor * objectColor, 1.0);
}
