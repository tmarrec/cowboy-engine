#version 460 core

in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 bonePos;

void main(void)
{
	float dist = distance(fs_in.FragPos, bonePos);
	FragColor = vec4(1, 1, 0, 1.0);
	if (dist < 1)
	{
		FragColor = vec4(1, 0, 0, 1.0);
	}
}
