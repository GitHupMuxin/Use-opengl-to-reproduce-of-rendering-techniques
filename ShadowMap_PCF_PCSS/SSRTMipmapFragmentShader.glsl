#version 330 core
/*
layout (location = 0) out vec4 FragNormal;
layout (location = 1) out vec4 FragKd;
layout (location = 2) out vec4 FragPos;
layout (location = 3) out float FragDepth;
*/
in vec3 normal;
in vec4 wordPos;
in vec2 texCoord;
in float depth;

uniform bool haveDiffuseTexture;
uniform vec3 kd;
uniform sampler2D texture_diffuse1;

vec4 pack(float depth)
{
	const vec4 bitShift = vec4(1.0, 256.0, 256.0 * 256.0, 256.0 * 256.0 * 256.0);
	const vec4 bitMask = vec4(1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0, 0.0);
	vec4 rgbaDepth = fract(depth * bitShift);
	rgbaDepth -= rgbaDepth.gbaa * bitMask;
	return rgbaDepth;
}

float unPack(vec4 rgbaDepth)
{
	const vec4 bitShift = vec4(1.0f, 1.0f / 256.0f, 1.0f / (256.0f * 256.0f), 1.0f / (256.0f * 256.0f * 256.0f));
	return dot(rgbaDepth, bitShift);
}

void main()
{
	//FragNormal = vec4(normalize(normal), 1.0f);
	//FragPos = wordPos;
	gl_FragData[0] = vec4(normalize(normal), 1.0f);
	if (haveDiffuseTexture)
	{
		gl_FragData[1] = texture(texture_diffuse1, texCoord);
		gl_FragData[1].w = 0.99f;
	}
	else
		gl_FragData[1] = vec4(kd, 0.0f);
	gl_FragData[2] = wordPos;
	gl_FragData[3].x = depth;
} 




