#version 330 core

in vec3 TexCoord;

uniform samplerCube skyBox;
uniform float uRoughness;
uniform int uTextureLob;

out vec4 FragColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

vec2 Hammersley(uint i, uint N)
{
	uint bits = (i << 16) | (i >> 16);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) / float(N), rdi);
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

void main()
{
	vec3 N = normalize(TexCoord);
	
	vec3 R = N;
	vec3 V = R;
	
	const uint SAMPLE_COUNT = 1024u;

	vec3 prefilteredColor = vec3(0.0);
	float totalWight = 0.0;

	for (uint i = 0u; i < SAMPLE_COUNT; i++)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, uRoughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0)
		{
			float D = DistributionGGX(N, H, uRoughness);
			float NdotH = max(dot(N, H), 0.0);
			float HdotV = max(dot(H, V), 0.0);
			float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;//Jacobian

			float resolution = 512.0;
			float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
			float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

			float mipLevel = uRoughness == 0 ? 0.0 : 0.5 * log2(saSample / saTexel);

			prefilteredColor += textureLod(skyBox, L, mipLevel).rgb * NdotL;
			totalWight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWight;
	FragColor = vec4(prefilteredColor, 1.0);
}
