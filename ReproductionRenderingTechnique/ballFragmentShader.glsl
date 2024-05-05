#version 330 core


uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;

in VS_OUT
{
	vec4 FragPosLightSpace;
	vec3 FragPos;
	vec3 Normal;
	vec3 cameraPos;
	vec3 LightPos;
	vec3 LightIntansity;
	vec2 texCoord;
} fsIn;

out vec4 FragColor;

vec3 bilinPhong(vec3 lightDirection, vec3 normal, vec3 cameraDir, float r2)
{
	vec3 Kd = kd;
	vec3 Ks = vec3(0.0f);
	vec3 halfVec = normalize(lightDirection + cameraDir);
	vec3 ambient = Kd * 0.05f;
	vec3 diffuse = fsIn.LightIntansity * Kd * max(0.0f, dot(lightDirection, normal)) / r2;
	vec3 specular = fsIn.LightIntansity * Ks * pow(max(0.0f, dot(halfVec, normal)), 32.0f) / r2;
	return ambient + diffuse + specular;
}

vec3 toSRGB(vec3 color)
{
	for (int i = 0; i < 3; i++)
		color[i] = pow(color[i], 1.0f / 2.2f);
	return color;
}
 
void main()
{
	vec3 LightDir = fsIn.LightPos - fsIn.FragPos;
	float r2 = dot(LightDir, LightDir);
	LightDir = normalize(LightDir);
	vec3 normal = normalize(fsIn.Normal);
	vec3 cameraDir = normalize(fsIn.cameraPos - fsIn.FragPos);
	vec3 color = bilinPhong(LightDir, normal, cameraDir, r2);
	FragColor = vec4(toSRGB(color), 1.0f);
}
