#version 330 core


uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;

uniform float uRoughness;
uniform samplerCube skyBox;
uniform float uTextureLevel;

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

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return nom / max(denom, 0.0001f);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float a = roughness + 1;
	float k = (a * a) / 8.0f;
	
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(0.0f, dot(N, V));
	float NdotL = max(0.0f, dot(N, L));
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(vec3 F0, vec3 V, vec3 H)
{
	float costheta = max(0.0f, dot(V, H));
	vec3 F = F0 + (1.0f - F0) * pow(1.0 - costheta, 5.0);
	return F;
}

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
	float uMetallic = 1.0;
	float roughness = uRoughness;

	vec3 LightDir = fsIn.LightPos - fsIn.FragPos;
	vec3 albedo = kd;

	vec3 N = normalize(fsIn.Normal);
	vec3 V = normalize(fsIn.cameraPos - fsIn.FragPos);
	vec3 L = normalize(LightDir);
	vec3 H = normalize(V + L);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, uMetallic);

	vec3 Lo = vec3(0.0f);
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0f);

	float D = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(F0, V, H);

	vec3 Li = vec3(1.0f);

	/////DirLigth
	vec3 numerator = D * G * F;
	float denominator = max((4.0f * NdotL * NdotV), 0.001);
	vec3 Fmicro = numerator / max(denominator, 0.0001);//BRDF Eu

	vec3 KsBRDF = Fmicro;

	Lo += Li * KsBRDF * NdotL;

	vec3 color = Lo;
	color = color / (color + vec3(1.0f));
	FragColor = vec4(toSRGB(color), 1.0);
}
