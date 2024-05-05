#version 330 core
out vec4 FragColor;

uniform sampler2D MipMap;
uniform ivec2 viewPort;
uniform int lastLevel;

void main()
{
	ivec2 currentCoord = ivec2(gl_FragCoord.xy);
	ivec2 lastCurrenCoord = currentCoord * 2;
	float x1 = texelFetch(MipMap, lastCurrenCoord + ivec2(0, 0), lastLevel).r;
	float x2 = texelFetch(MipMap, lastCurrenCoord + ivec2(0, 1), lastLevel).r;
	float x3 = texelFetch(MipMap, lastCurrenCoord + ivec2(1, 0), lastLevel).r;
	float x4 = texelFetch(MipMap, lastCurrenCoord + ivec2(1, 1), lastLevel).r;
	float minDepth = min(min(min(x1, x2), x3), x4);
	bool x = (viewPort.x & 1) == 1, y = (viewPort.y & 1) == 1;
	if (x)
	{
		float x1 = texelFetch(MipMap, lastCurrenCoord + ivec2(2, 0), lastLevel).r;
		float x2 = texelFetch(MipMap, lastCurrenCoord + ivec2(2, 1), lastLevel).r;
		minDepth = min(minDepth, min(x1, x2));
	}
	if (y)
	{
		float x1 = texelFetch(MipMap, lastCurrenCoord + ivec2(0, 2), lastLevel).r;
		float x2 = texelFetch(MipMap, lastCurrenCoord + ivec2(1, 2), lastLevel).r;
		minDepth = min(minDepth, min(x1, x2));
	}
	if (x && y)
	{
		float temp = texelFetch(MipMap, lastCurrenCoord + ivec2(2, 2), lastLevel).r;
		minDepth = min(minDepth, temp);
	}
	FragColor.r = minDepth;
}