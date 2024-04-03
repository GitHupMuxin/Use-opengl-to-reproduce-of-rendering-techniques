#version 330 core
out vec4 FragColor;

uniform sampler2D MipMap;
uniform ivec2 viewPort;
uniform int lastLevel;

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
	ivec2 currentCoord = ivec2(gl_FragCoord.xy);
	ivec2 lastCurrenCoord = currentCoord * 2;
	float x1 = texelFetch(MipMap, lastCurrenCoord + ivec2(0, 0), lastLevel).r;
	float x2 = texelFetch(MipMap, lastCurrenCoord + ivec2(0, 1), lastLevel).r;
	float x3 = texelFetch(MipMap, lastCurrenCoord + ivec2(1, 0), lastLevel).r;
	float x4 = texelFetch(MipMap, lastCurrenCoord + ivec2(1, 1), lastLevel).r;
	float minDepth = min(min(min(x1, x2), x3), x4);
	if ((viewPort.x & 1) == 1)
	{
		float x1 = texelFetch(MipMap, lastCurrenCoord + ivec2(2, 0), lastLevel).r;
		float x2 = texelFetch(MipMap, lastCurrenCoord + ivec2(2, 1), lastLevel).r;
		minDepth = min(minDepth, min(x1, x2));
	}
	if ((viewPort.y & 1) == 1)
	{
		float x1 = texelFetch(MipMap, lastCurrenCoord + ivec2(0, 2), lastLevel).r;
		float x2 = texelFetch(MipMap, lastCurrenCoord + ivec2(1, 2), lastLevel).r;
		minDepth = min(minDepth, min(x1, x2));
	}
	FragColor = pack(minDepth);
}