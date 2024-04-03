#version 330 core

#define EPS 1e-3;
#define PI 3.141592653589793
#define PI2 6.283185307179586
#define NUM_SAMPLES 36
#define NUM_RINGS 10
#define SHADOW_MAP_SIZE 2048
#define LIGHT_SIZE 40
#define SHADOW_NEAR 0.01


layout (std140) uniform Scene
{
	vec3 cameraPos;
	mat4 view;
	mat4 projection;
};

layout (std140) uniform LightScene
{
	vec3 LightIntansity;
	vec3 LightPosition;
	mat4 LightView;
	mat4 LightProjection;
};

in VS_OUT
{
	vec4 FragPosLightSpace;
	vec3 FragPos;
	vec3 Normal;
	vec3 color;
	vec2 texCoord;
}fsIn;


vec2 poissonDisk[NUM_SAMPLES];

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

out vec4 FragColor;

highp float rand_1to1(float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}

highp float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const float a = 12.9898, b = 78.233, c = 43758.5453;
	float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

void poissonDiskSamples( const in vec2 randomSeed ) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = rand_2to1( randomSeed ) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

float unPack(vec4 rgbaDepth)
{
	const vec4 bitShift = vec4(1.0f, 1.0f / 256.0f, 1.0f / (256.0f * 256.0f), 1.0f / (256.0f * 256.0f * 256.0f));
	return dot(rgbaDepth, bitShift);
}

float getBias(vec3 LightDir, vec3 normal, float Cdepth, float Cnormal)
{
	float A = 20.0f / float(SHADOW_MAP_SIZE) * 0.5;
	float B = 1.0f - dot(LightDir, normal);
	float DepthBias = Cdepth * A * B;
	float NormalBias = Cnormal * A * B;
	return DepthBias;
}

float getVisibility(float Bias)
{
	vec3 coord = fsIn.FragPosLightSpace.xyz / fsIn.FragPosLightSpace.w;
	coord = coord * 0.5 + 0.5;
	float shadowMapDepth = unPack(texture(shadowMap, coord.xy));
	float currentDepth = coord.z;
	shadowMapDepth = shadowMapDepth + Bias;
	float visibility = shadowMapDepth > currentDepth ? 1.0f : 0.0f;
	return visibility;
}

float PCF(float filterSize, float Bias)
{
	float result = 0.0f;
	vec3 coord = fsIn.FragPosLightSpace.xyz / fsIn.FragPosLightSpace.w;
	coord = coord * 0.5 + 0.5;
	if (coord.z > 1.0f)
		return 1.0f;
	poissonDiskSamples(coord.xy);
	for (int i = 0; i < NUM_SAMPLES; i++)
	{

		vec2 shadowCoord = coord.xy + poissonDisk[i] * filterSize;
		float R = length(vec2(poissonDisk[i].x, poissonDisk[i].y));
		float RBias = (1.0f + ceil(R)) * Bias + EPS;
		float depth = unPack(texture(shadowMap, shadowCoord));
		if (coord.z < depth + RBias)
			result++;
	}
	result = result / float(NUM_SAMPLES);
	return result;
}

float findBlock(vec2 coord, float zReceiver)
{
	vec3 LightToFragPoint = fsIn.FragPos - LightPosition;
	float LightSize = float(LIGHT_SIZE);
	float LightToFragPointDistance = sqrt(dot(LightToFragPoint, LightToFragPoint));
	float shadowMapNear = float(SHADOW_NEAR);
	float r = LightSize / LightToFragPointDistance * (LightToFragPointDistance - shadowMapNear);
	r = r / float(SHADOW_MAP_SIZE);
	poissonDiskSamples(coord);
	float sum = 0.0f;
	float count = 0.0f;
	float depth = zReceiver + EPS;
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		vec2 shadowCoord = coord + poissonDisk[i] * r;
		float currentDepth = unPack(texture(shadowMap, shadowCoord));
		if (currentDepth < depth)
		{
			sum = sum + currentDepth;
			count++;
		}
	}
	return sum / count;
}

float PCSS(vec3 lightDirection, vec3 normal)
{
	vec3 coord = fsIn.FragPosLightSpace.xyz / fsIn.FragPosLightSpace.w;
	coord = coord * 0.5 + 0.5;
	float zReceiver = coord.z;
	float shadowMapDepth = unPack(texture(shadowMap, coord.xy)) + EPS;
	float block = findBlock(coord.xy, zReceiver);
	vec3 LightToFragPoint = fsIn.FragPos - LightPosition;
	float LightSize = float(LIGHT_SIZE);
	float wPenumbra = LightSize / block * (zReceiver - block);
	wPenumbra = wPenumbra / float(SHADOW_MAP_SIZE) * 0.5;
	float Bias = getBias(lightDirection, normal, 0.05f, 1.0f);
	return PCF(wPenumbra, 0.0f);
}

vec3 bilinPhongWithPCF(vec3 lightDirection, vec3 normal, vec3 cameraDir, float r2)
{
	vec3 Kd = vec3(0.8f);
	vec3 Ks = vec3(1.0f);
	vec3 halfVec = normalize(lightDirection + cameraDir);
	vec3 diffuse = LightIntansity * Kd * max(0.0f, dot(lightDirection, normal)) / r2;
	vec3 specular = LightIntansity * Ks * pow(max(0.0f, dot(halfVec, normal)), 32.0f) / r2;
	float Bias = getBias(lightDirection, normal, 0.05f, 1.0f);
	float filterSize = 2.0f;
	filterSize = filterSize / float(SHADOW_MAP_SIZE);
	return (diffuse + specular) * PCF(filterSize, 0.0f);
}

vec3 bilinPhongWithPCSS(vec3 lightDirection, vec3 normal, vec3 cameraDir, float r2)
{
	vec3 Kd = vec3(0.8f);
	vec3 Ks = vec3(1.0f);
	vec3 halfVec = normalize(lightDirection + cameraDir);
	vec3 diffuse = LightIntansity * Kd * max(0.0f, dot(lightDirection, normal)) / r2;
	vec3 specular = LightIntansity * Ks * pow(max(0.0f, dot(halfVec, normal)), 32.0f) / r2;
	return (diffuse + specular) * PCSS(lightDirection, normal);
}

vec3 toSRGB(vec3 color)
{
	for (int i = 0; i < 3; i++)
		color[i] = pow(color[i], 1.0f / 2.2f);
	return color;
}

void main()
{
	vec3 LightDir = LightPosition - fsIn.FragPos;
	float r2 = dot(LightDir, LightDir);
	LightDir = normalize(LightDir);
	vec3 normal = normalize(fsIn.Normal);
	vec3 cameraDir = normalize(cameraPos - fsIn.FragPos);
	vec3 diffuse = vec3(0.8f);
	vec3 color = bilinPhongWithPCSS(LightDir, normal, cameraDir, r2) + fsIn.color * diffuse;
	FragColor = vec4(toSRGB(color), 1.0f);
}
