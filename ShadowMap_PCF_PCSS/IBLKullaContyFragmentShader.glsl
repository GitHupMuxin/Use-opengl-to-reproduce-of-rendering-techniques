#version 330 core
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float uRoughness;
uniform sampler2D EmuIs;
uniform sampler2D Eavg;
uniform sampler2D EmuMc;
uniform sampler2D LUTTexture;
uniform samplerCube skyBox;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;

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
const float SKYBOX_MAX_LEVLE = 4;

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

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//https://blog.selfshadow.com/publications/s2017-shading-course/imageworks/s2017_pbs_imageworks_slides_v2.pdf
vec3 AverageFresnel(vec3 r, vec3 g)
{
	return vec3(0.087237) + 0.0230685*g - 0.0864902*g*g + 0.0774594*g*g*g
           + 0.782654*r - 0.136432*r*r + 0.278708*r*r*r
           + 0.19744*g*r + 0.0360605*g*g*r - 0.2586*g*r*r;
}

vec3 MultiScatterBRDF(float NdotL, float NdotV, float roughness, vec3 albedo)
{
	vec3 Eo = texture(EmuIs, vec2(NdotV, roughness)).xyz;
	vec3 Ei = texture(EmuIs, vec2(NdotL, roughness)).xyz;

	vec3 Ea = texture(Eavg, vec2(0, roughness)).xyz;

	vec3 edgetint = vec3(0.827, 0.792, 0.678);
    vec3 Favg = AverageFresnel(albedo, edgetint);
  
	vec3 Fms = (1.0f - Eo) * (1.0f - Ei) / (PI * (1.0 - Ea));
	vec3 Fadd = Favg * Ea / (1.0f - Favg * (1.0 - Ea));

	return Fms * Fadd;
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
	vec3 Lo = vec3(0.0f);
	vec3 LightDir = fsIn.LightPos - fsIn.FragPos;
	vec3 albedo = kd;

	vec3 N = normalize(fsIn.Normal);
	vec3 V = normalize(fsIn.cameraPos - fsIn.FragPos);
	vec3 R = reflect(-V, N);
	vec3 L = normalize(LightDir);
	vec3 H = normalize(V + L);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, uMetallic);

	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0f);

	float D = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 dirF = fresnelSchlick(F0, V, H);

	vec3 Li = vec3(1.0f);

	////DirLight
	vec3 numerator = D * G * dirF;
	float denominator = max((4.0f * NdotL * NdotV), 0.001);
	vec3 Fmicro = numerator / max(denominator, 0.0001);//BRDF Eu

	vec3 Fms = MultiScatterBRDF(NdotL, NdotV, roughness, albedo);
	vec3 KsBRDF = Fmicro + Fms;

	Lo += Li * KsBRDF * NdotL;
	
	////IBL
	vec3 F = fresnelSchlickRoughness(F0, max(dot(N, V), 0.0), roughness);
	//fss
	vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * SKYBOX_MAX_LEVLE).rgb;
	vec2 LUT = texture(LUTTexture, vec2(max(dot(N, V), 0.0), roughness)).rg;
	KsBRDF = F * LUT.x + LUT.y;

	Lo += prefilteredColor * KsBRDF;
	//fsm
	vec3 Eo = texture(EmuIs, vec2(NdotV, roughness)).xyz;
	vec3 Ei = texture(EmuIs, vec2(NdotL, roughness)).xyz;
	vec3 Ea = texture(Eavg, vec2(0, roughness)).xyz;
	vec3 edgetint = vec3(0.827, 0.792, 0.678);
    vec3 Favg = AverageFresnel(albedo, edgetint);

	KsBRDF = (1.0 - Eo) * Favg * Ea * PI * (1.0 - Ea);
	KsBRDF /= PI * (1.0 - Ea) * (1.0 - Favg * (1.0 - Ea));

	Lo += prefilteredColor * KsBRDF;
	
	vec3 color = Lo;
	color = color / (color + vec3(1.0f));
	FragColor = vec4(toSRGB(color), 1.0);
}
