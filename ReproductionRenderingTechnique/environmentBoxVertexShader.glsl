#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoord;

layout (std140) uniform Scene
{
	vec3 cameraPosition;
	mat4 view;
	mat4 projection;
};

void main()
{
	TexCoord = aPos;
	vec4 position = projection * mat4(mat3(view)) * vec4(aPos, 1.0f);
	gl_Position = position.xyww;
}


