#version 330 core

in vec2 texCoord;

void main()
{	
	gl_FragData[1] = vec4(0.0);
	gl_FragData[3].x = 1.0f;
}