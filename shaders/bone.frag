#version 460 core

in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 startBonePos;
uniform vec3 endBonePos;

void main(void)
{
	float dist = distance(fs_in.FragPos - startBonePos, endBonePos - startBonePos);
	float r = 1/pow(dist, 4);
	float g = 1/pow(dist, 4);
	float b = 1/pow(dist, 4);
	FragColor = vec4(0, g, b, 1.0);
}
