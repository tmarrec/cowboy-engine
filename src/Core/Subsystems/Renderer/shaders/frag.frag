#version 460 core

out vec4 FragColor;

in  vec2 texCoord;
in  vec3 normal;
in  vec3 worldPos;

uniform vec3        camPos;

uniform sampler2D   tex;
uniform vec3        albedo;
uniform float       metallic;
uniform float       roughness;
uniform float       ao;

void main()
{
    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - worldPos);

    FragColor = texture(tex, texCoord);
}
