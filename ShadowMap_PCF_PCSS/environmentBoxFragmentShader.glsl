#version 330 core

in vec3 TexCoord;

uniform samplerCube skyBox;
uniform int uTextureLob;

out vec4 FragCoord;

void main()
{
	//FragCoord = texture(skyBox, TexCoord);
	FragCoord = textureLod(skyBox, TexCoord, uTextureLob);
}
