#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform Scene
{
	vec3 cameraPos;
	mat4 view;
	mat4 projection;
};

uniform mat4 model;

out vec3 normal;
out vec4 wordPos;
out vec2 texCoord;
out float depth;


void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	normal = (transpose(inverse(model)) * vec4(aNormal, 0.0f)).xyz;
	texCoord = aTexCoord;
	wordPos = model * vec4(aPos, 1.0f);
	depth = gl_Position.w;
}