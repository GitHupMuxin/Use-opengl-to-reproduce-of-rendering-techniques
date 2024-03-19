#version 330 core

in vec3 TexCoord;

uniform samplerCube skyBox;

out vec4 FragCoord;

void main()
{
	FragCoord = texture(skyBox, TexCoord);
}
