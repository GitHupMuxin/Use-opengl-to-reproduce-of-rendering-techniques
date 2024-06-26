#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform Scene
{
	vec3 cameraPos;
	mat4 view;
	mat4 projection;
	float nearPlane;
	float farPlane;
};


out mat4 P;
out mat4 V;
out mat4 PV;
out vec3 viewPos;

void main()
{
	gl_Position = vec4(aPos, 1.0f);
	P = projection;
	V = view;
	PV = projection * view;
	viewPos = cameraPos;
}