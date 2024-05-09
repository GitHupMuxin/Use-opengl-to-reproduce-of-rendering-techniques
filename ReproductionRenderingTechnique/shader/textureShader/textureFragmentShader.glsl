#version 450 core

in vec2 texCoord;

uniform sampler2D Texture;
uniform int level;

out vec4 FragColor;

float unPack(vec4 rgbaDepth)
{
	const vec4 bitShift = vec4(1.0f, 1.0f / 256.0f, 1.0f / (256.0f * 256.0f), 1.0f / (256.0f * 256.0f * 256.0f));
	return dot(rgbaDepth, bitShift);
}

void main()
{
	ivec2 currentCoord = ivec2(gl_FragCoord.xy);
	currentCoord = currentCoord >> level;
	//FragColor = vec4(pow(unPack(texelFetch(Texture, currentCoord, level)), 64));
	//FragColor = vec4(pow(texelFetch(Texture, currentCoord, level).r, 64));
	FragColor = vec4(textureLod(Texture, texCoord, level)); 
	FragColor = vec4(pow(textureLod(Texture, texCoord, level).r, 64)); 
	//FragColor =	vec4(unPack(texelFetch(Texture, currentCoord, level)));;
}
