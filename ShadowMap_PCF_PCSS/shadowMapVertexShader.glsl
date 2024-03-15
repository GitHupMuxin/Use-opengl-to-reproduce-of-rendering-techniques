#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform LightScene
{
	vec3 LightIntansity;
	vec3 LightPos;
	mat4 LightView;
	mat4 LightProjection;
};

uniform mat4 model;

void main()
{
	gl_Position = LightProjection * LightView * model * vec4(aPos, 1.0f);
}




