#version 330 core
in vec3 vertexColor;
uniform vec3 objectColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(vertexColor * objectColor, 1.0);
}
