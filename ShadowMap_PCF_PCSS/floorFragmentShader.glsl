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
} fsIn;

uniform vec3 lightDir;
uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

uniform vec3 LightInstansity;

out vec4 FragColor;



float getBias(vec3 LightDir, vec3 normal, float Cdepth, float Cnormal)
{
	float A = 20.0f / 1024.0f * 0.5;
	float B = 1.0f - dot(LightDir, normal);
	float DepthBias = Cdepth * A * B;
	float NormalBias = Cnormal * A * B;
	return DepthBias;
}

float getVisibility(float Bias)
{
	vec3 coord = fsIn.FragPosLightSpace.xyz / fsIn.FragPosLightSpace.w;
	coord = coord * 0.5 + 0.5;
	float shadowMapDepth = texture(shadowMap, coord.xy).r;
	float currentDepth = coord.z;
	shadowMapDepth = shadowMapDepth + Bias;
	float visibility = shadowMapDepth > currentDepth ? 1.0f : 0.0f;
	return visibility;
}

float PCF(int filterSize, float Bias)
{
	float result = 0.0f;
	vec3 coord = fsIn.FragPosLightSpace.xyz / fsIn.FragPosLightSpace.w;
	coord = coord * 0.5 + 0.5;
	vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
	float FragDepth = coord.z;
	float count = 0.0f;
	if (coord.z > 1.0f)
		return 1.0f;
	for (int x = -filterSize; x <= filterSize; x++)
	{
		for (int y = -filterSize; y <= filterSize; y++)
		{
			float R = length(vec2(x, y));
			float RBias = (1.0f + ceil(R)) * Bias + EPS;
			float depth = texture(shadowMap, vec2(x, y) * texelSize + coord.xy).r;
			if (depth + RBias > FragDepth)
				result++;
			count++;
		}
	}
	result = result / count;
	return result;
}

vec3 bilinPhong(vec3 lightDirection, vec3 normal, vec3 cameraDir, float r2)
{
	vec3 Kd = vec3(0.8f);
	vec3 Ks = vec3(1.0f);
	vec3 halfVec = normalize(lightDirection + cameraDir);
	vec3 ambient = Kd * 0.1f;
	vec3 diffuse = fsIn.LightIntansity * Kd * max(0.0f, dot(lightDirection, normal)) / r2;
	vec3 specular = fsIn.LightIntansity * Ks * pow(max(0.0f, dot(halfVec, normal)), 32.0f) / r2;
	float Bais = getBias(lightDirection, normal, 2.5f, 1.0f);
	return ambient + (diffuse + specular);
}

void main()
{
	vec3 LightDir = fsIn.LightPos - fsIn.FragPos;
	float r2 = dot(LightDir, LightDir);
	LightDir = normalize(LightDir);
	vec3 normal = normalize(fsIn.Normal);
	vec3 cameraDir = normalize(fsIn.cameraPos - fsIn.FragPos);
	vec3 color = bilinPhong(LightDir, normal, cameraDir, r2);
	FragColor = vec4(color, 1.0f);
}






