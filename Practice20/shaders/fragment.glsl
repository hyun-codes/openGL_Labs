#version 330 core

in vec3 vertexColor;
// 정점 색상에 곱할 객체별 색상을 C++에서 전달받는다.
uniform vec3 objectColor;
out vec4 FragColor;

// 보간된 정점 색상과 객체 색상을 곱해 최종 픽셀 색상을 정한다.
void main()
{
    FragColor = vec4(vertexColor * objectColor, 1.0);
}
