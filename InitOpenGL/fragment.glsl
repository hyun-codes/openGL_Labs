#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

uniform vec3 objectColor;

void main()
{
    FragColor = vec4(vertexColor * objectColor, 1.0);
}
