#version 330 core

in vec3 passColor;
out vec4 FragColor;

void main()
{
    // 앞으로 그릴 점, 선, 삼각형의 기본 색상
    FragColor = vec4(passColor, 1.0);
}
