#version 460 core

#define PI 3.14159265359

struct PointLight
{
    vec3    color;
    float   radius;
    vec4    position;
    vec4    positionVS;
};

in  vec2 texCoords;
in  vec3 normal;
in  vec3 fragPos;
in  mat3 TBN;

out vec4 FragColor;

layout (std430, binding = 0) buffer LightIndexList
{
    uint gLightIndexList[];
};
layout (std430, binding = 1) buffer LightsBuffer
{
    PointLight gPointLights[];
};

uniform usampler2DRect  lightGrid;
uniform sampler2D       albedoMap;
uniform sampler2D       metallicRoughnessMap;
uniform sampler2D       emissiveMap;
uniform sampler2D       normalMap;
uniform sampler2D       occlusionMap;

uniform vec3            viewPos;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{             
    uvec2 tileIndex = uvec2(floor(gl_FragCoord / 16.0) * 16);
    uint startOffset = texture(lightGrid, tileIndex).x;
    uint lightCount = texture(lightGrid, tileIndex).y;

    vec3 albedo = pow(texture(albedoMap, texCoords).rgb, vec3(2.2, 2.2, 2.2));
    float metallic = texture(metallicRoughnessMap, texCoords).b;
    float roughness = texture(metallicRoughnessMap, texCoords).g;
    float occlusion = texture(occlusionMap, texCoords).r;

    //FragColor = vec4(albedo, 1);

    vec3 N = normalize(normal);
    //vec3 N = normalize(TBN * (texture(normalMap, texCoords).rgb * 2.0f - 1.0f));
    vec3 V = normalize(viewPos - fragPos);

    vec3 Lo = vec3(0.0);
    
    for(uint i = 0; i < lightCount; ++i)
    {
        uint lightIndex = gLightIndexList[startOffset + i];
        PointLight light = gPointLights[lightIndex];

        vec3 L = normalize(light.position.xyz - fragPos);
        vec3 H = normalize(V + L);

        float dist = length(light.position.xyz - fragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = light.color * attenuation;

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);
        vec3 F = fresnelSchlick(max(dot(H,V), 0.0), F0);

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    //vec3 ambient = vec3(0.03) * albedo;
    vec3 color = Lo * occlusion + pow(texture(emissiveMap, texCoords).rgb, vec3(2.2, 2.2, 2.2));

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
