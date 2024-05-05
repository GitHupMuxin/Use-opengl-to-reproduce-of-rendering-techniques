#version 330 core

in vec3 TexCoord;

uniform samplerCube skyBox;
uniform int uTextureLob;

out vec4 FragCoord;

const float PI = 3.14159265359;

void main()
{
	vec3 N = normalize(TexCoord);
	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = normalize(cross(N, right));

	float delta = 0.0125;
	float count = 0.0f;
	for (float phi = 0.0; phi < 2.0 * PI; phi += delta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += delta)
		{
			vec3 tangentSample = vec3(sin(theta)* cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 SampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(skyBox, SampleVec).rgb * cos(theta) * sin(theta);
			count++;
		} 
	}
	irradiance = PI * irradiance / count;

	FragCoord = vec4(irradiance, 1.0);
}
