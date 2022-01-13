#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec2 texCoords;
out vec3 normal;
out vec3 fragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos   = model * vec4(aPos, 1.0);
    vec3 T          = normalize((model * vec4(aTangent, 0.0f)).xyz);
    vec3 N          = normalize((model * vec4(aNormal, 0.0f)).xyz);
    vec3 B          = cross(N, T);

    fragPos         = worldPos.xyz;
    texCoords       = aTexCoords;
    normal          = transpose(inverse(mat3(model))) * aNormal;
    TBN             = mat3(T, B, N);

    gl_Position     = projection * view * worldPos;
}
