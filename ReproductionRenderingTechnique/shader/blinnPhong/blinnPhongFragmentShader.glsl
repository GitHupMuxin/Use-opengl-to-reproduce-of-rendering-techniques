#version 330 core

#define EPS 1e-3;

in VS_OUT
{
	vec4 FragPosLightSpace;
	vec3 FragPos;
	vec3 Normal;
	vec3 cameraPos;
	vec3 LightPos;
	vec3 LightIntansity;
	vec2 texCoord;
}fsIn;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

vec3 processPointLight(vec3 LightDir, vec3 normal, vec3 cameraDir, float r2)
{
	vec3 kd = vec3(texture(texture_diffuse1, fsIn.texCoord)) * vec3(0.8f);
	vec3 ks = vec3(0.0f);
	vec3 halfVec = normalize(LightDir + cameraDir);
	vec3 ambient = kd * 0.1f;
	vec3 diffuse = fsIn.LightIntansity * kd * max(0.0f, dot(LightDir, normal)) / r2;
	vec3 specular = fsIn.LightIntansity * ks * pow(max(0.0f, dot(halfVec, normal)), 32.0f) / r2;
	return ambient + diffuse + specular;
}

void main()
{
	vec3 LightDir = fsIn.LightPos - fsIn.FragPos;
	float r2 = dot(LightDir, LightDir);
	LightDir = normalize(LightDir);
	vec3 normal = normalize(fsIn.Normal);
	vec3 cameraDir = normalize(fsIn.cameraPos - fsIn.FragPos);
	vec3 color = processPointLight(LightDir, normal, cameraDir, r2);
	FragColor = vec4(color, 1.0f);
}
