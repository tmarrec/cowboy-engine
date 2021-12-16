#version 460 core

out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;

void main()
{
    FragColor = vec4(normal, 1.0f);
}
