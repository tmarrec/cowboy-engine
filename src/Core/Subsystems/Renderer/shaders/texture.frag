#version 460 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D texture0;

void main()
{             
    FragColor = vec4(texture(texture0, texCoords).rgba);
}
