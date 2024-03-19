#version 330 core

in vec2 texCoord;

uniform sampler2D texture_diffuse;

out vec4 FragColor;

float unPack(vec4 rgbaDepth)
{
	const vec4 bitShift = vec4(1.0f, 1.0f / 256.0f, 1.0f / (256.0f * 256.0f), 1.0f / (256.0f * 256.0f * 256.0f));
	return dot(rgbaDepth, bitShift);
}

void main()
{
	FragColor = vec4(vec3(unPack(texture(texture_diffuse, texCoord))), 1.0f);
}

