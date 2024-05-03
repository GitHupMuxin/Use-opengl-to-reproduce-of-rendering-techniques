#version 330 core

in vec3 normal;
in vec4 wordPos;
in vec4 viewSpacePos;
in vec2 texCoord;
in float depth;

uniform bool haveDiffuseTexture;
uniform vec3 kd;
uniform sampler2D texture_diffuse1;
uniform float zNear;
uniform float zFar;
uniform bool SSRTON;


layout (std140) uniform Scene
{
	vec3 cameraPos;
	mat4 view;
	mat4 projection;
	float nearPlane;
	float farPlane;
};

float LinearizeDepth(float z)
{
	z = z * 2.0f - 1.0f;
	return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
	gl_FragData[0] = vec4(normalize(normal), 1.0f);
	if (!SSRTON)
	{
		gl_FragData[1] = texture(texture_diffuse1, texCoord);
		gl_FragData[1].w = 0.99f;
	}
	else
		gl_FragData[1] = vec4(kd, 0.0f);
	gl_FragData[2] = wordPos;
	gl_FragData[2].w = depth;
	gl_FragData[3].r = gl_FragCoord.z;
	gl_FragData[4] = viewSpacePos;
	gl_FragData[5].r = LinearizeDepth(gl_FragCoord.z);
} 




