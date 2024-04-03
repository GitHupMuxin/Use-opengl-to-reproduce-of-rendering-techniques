#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 LightTransport0;
layout (location = 4) in vec3 LightTransport1;
layout (location = 5) in vec3 LightTransport2;

layout (std140)	uniform Scene
{
	vec3 cameraPos;
	mat4 view;
	mat4 projection;
};

layout (std140) uniform LightSH
{
	mat4 rLightSH;
	mat4 gLightSH;
	mat4 bLightSH;
};

layout (std140) uniform LightScene
{
	vec3 LightIntansity;
	vec3 LightPosition;
	mat4 LightView;
	mat4 LightProjection;
};

uniform mat4 model;

out VS_OUT
{
	vec4 FragPosLightSpace;
	vec3 FragPos;
	vec3 Normal;
	vec3 color;
	vec2 texCoord;
}vsOut;

float prt(mat4 LightSH, mat3 LightTransport)
{
	float result = 0.0f;
	for (int i = 0; i < 3; i++)
		result += dot(LightSH[i].xyz, LightTransport[i]);
	return result;
}

void main()
{      
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	mat3 LightTransport = mat3(LightTransport0, LightTransport1, LightTransport2);
	vsOut.color[0] = prt(rLightSH, LightTransport);
	vsOut.color[1] = prt(gLightSH, LightTransport);
	vsOut.color[2] = prt(bLightSH, LightTransport);
	vsOut.FragPosLightSpace = LightProjection * LightView * model * vec4(aPos, 1.0f);
	vsOut.FragPos = vec3(model * vec4(aPos, 1.0f));
	vsOut.Normal = (transpose(inverse(model)) * vec4(aNormal, 0.0f)).xyz;;
	vsOut.texCoord = aTexCoord;
}


