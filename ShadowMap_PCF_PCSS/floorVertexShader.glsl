#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform Scene
{
	vec3 cameraPosition;
	mat4 view;
	mat4 projection;
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
	vec3 cameraPos;
	vec3 LightPos;
	vec3 LightIntansity;
	vec2 texCoord;
} vsOut;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	vsOut.FragPosLightSpace = LightProjection * LightView * model * vec4(aPos, 1.0f);
	vsOut.FragPos = vec3(model * vec4(aPos, 1.0f));
	vsOut.Normal = (transpose(inverse(model)) * vec4(aNormal, 0.0f)).xyz;
	vsOut.cameraPos = cameraPosition;
	vsOut.LightPos = LightPosition;
	vsOut.LightIntansity = LightIntansity;
	vsOut.texCoord = aTexCoord;
}



