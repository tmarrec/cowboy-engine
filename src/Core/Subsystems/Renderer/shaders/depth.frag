#version 460 core

layout (location = 0) out vec4 gDepth;

void main()
{
    gDepth = vec4(vec3(gl_FragCoord.z), 1.0);
}
