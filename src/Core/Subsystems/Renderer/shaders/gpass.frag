#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMetallicRoughness;

in  vec2 texCoords;
in  vec3 normal;
in  vec3 fragPos;

uniform sampler2D   albedoMap;
uniform sampler2D   metallicRoughnessMap;

void main()
{
    gPosition = fragPos;
    gNormal = normalize(normal);
    gAlbedo.rgb = texture(albedoMap, texCoords).rgb;
    gMetallicRoughness.rgb = texture(metallicRoughnessMap, texCoords).rgb;
}
