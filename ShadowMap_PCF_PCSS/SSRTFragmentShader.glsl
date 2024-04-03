#version 330 core
#define PI 3.141592653589793
#define PI2 6.283185307179586
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309
#define EPS 1e-4
#define SAMPLE_SIZE 1

uniform sampler2D MipMap;
uniform sampler2D gNormalBuffer;
uniform sampler2D gKdBuffer;
uniform sampler2D gPosBuffer;
uniform sampler2D gColorBuffer;
uniform vec2 SCREEN_SIZE;
uniform int maxLevel;

in mat4 PV;
in vec3 viewPos;

out vec4 FragColor;

layout (std140) uniform LightScene
{
	vec3 LightIntansity;
	vec3 LightPosition;
	mat4 LightView;
	mat4 LightProjection;
};

//0 - 1
float Rand1(float x)
{
	return fract(sin(x) * 1000.0f) * 0.5 + 0.5;
}

float unPack(vec4 rgbaDepth)
{
	const vec4 bitShift = vec4(1.0f, 1.0f / 256.0f, 1.0f / (256.0f * 256.0f), 1.0f / (256.0f * 256.0f * 256.0f));
	return dot(rgbaDepth, bitShift);
}


// normalize sampler2D dir
vec3 get3DRandDir(vec2 uv, inout float pdf)
{
	float phi = Rand1(uv.x) * PI;
	float beta = Rand1(uv.y) * PI2;
	pdf = INV_TWO_PI;
	return vec3(cos(beta) * cos(phi), sin(beta) * cos(phi), sin(phi));
}

float getMipMapDepth(vec2 uv)
{
	return texture(MipMap, uv).r;
}

bool haveKd(vec2 uv)
{
	if (uv == vec2(0.0))
		return false;
	else 
		return texture(gKdBuffer, uv).w > 0.01;
}

vec3 getKd(vec2 uv)
{
	if (uv == vec2(0.0))
		return vec3(0.0);
	else 
		return vec3(texture(gKdBuffer, uv));
}

vec3 getNormal(vec2 uv)
{
	if (uv == vec2(0.0))
		return vec3(0.0);
	else 
		return normalize(vec3(texture(gNormalBuffer, uv)));
}

vec3 getPos(vec2 uv)
{
	return vec3(texture(gPosBuffer, uv));
}

vec3 getColor(vec2 uv)
{
	return vec3(texture(gColorBuffer, uv));
}

vec4 normalizeW(vec4 v)
{
	return v / v.w;
}

// to ndc x : 0 - 1 y : 0 - 1
vec2 GetScreenCoordinate(vec3 posWorld)
{
	return normalizeW(PV * vec4(posWorld, 1.0f)).xy * 0.5f + vec2(0.5f);
}

float GetWordPosScreenDepth(vec3 posWorld)
{
	return (PV * vec4(posWorld, 1.0f)).w;
}

vec3 getDir(vec3 normal, vec3 dir)
{
	vec3 left, up;
	if (normal.z == 1.0f)
		up = vec3(1.0f, 0.0f, 0.0f);
	else 
		up = vec3(0.0f, 0.0f, 1.0f);
	left = cross(normal, up);
	up = cross(left, normal);
	mat3 m = mat3(up, left, normal);
	return m * dir;
}

vec3 rayMatchMipMap(vec3 currentCoord, vec3 normal, inout float pdf)
{
	int nowLevel = 0;
	vec3 dir = getDir(normal, get3DRandDir(gl_FragCoord.xy, pdf));
	float stepX = dir.x > 0 ? (1.0f - currentCoord.x) / dir.x : - currentCoord.x / dir.x;
	float stepY = dir.y > 0 ? (1.0f - currentCoord.y) / dir.y : - currentCoord.y / dir.y;
	float stepZ = dir.z > 0 ? (1.0f - currentCoord.z) / dir.z : - currentCoord.z / dir.z;
	float stepSize = 0.02;
	float maxStep = min(min(stepX, stepY), stepZ) * 50;
	float nowStep = 1;
	vec3 hitPos = vec3(0.0f);
	while (nowLevel > 0 && nowStep < maxStep)
	{
		//RayMarch 
		float currentDepth = currentCoord.z + dir.z * stepSize;
		vec2 currentXY = currentCoord.xy + dir.xy * stepSize;
		float marchDepth = unPack(texture(MipMap, currentXY, nowLevel));
		if (currentDepth < marchDepth + EPS)
			nowLevel = min(nowLevel + 1, maxLevel);
		else if (nowLevel == 0)
		{
			hitPos = vec3(currentXY, marchDepth);
			break;
		}
		else	
			nowLevel--;
		nowStep++;
	}
	return hitPos;
}

bool rayMatch(vec3 oir, vec3 dir, vec3 oirNormal, out vec3 hitPos)
{
	float maxStep = 1000;
	float stepSize = 0.01;
	vec3 stepDir = dir * stepSize;
	float PerPixelThickness = 0.1180;
	float PerPixelCompareBias = 0.1025;
	vec3 ray = oir;
	for (int i = 0; i < maxStep; i++)
	{
		ray += stepDir;
		vec2 screenSpaceRay = GetScreenCoordinate(ray);
		float rayDepth = GetWordPosScreenDepth(ray);

		float ssRayDepth = getMipMapDepth(screenSpaceRay.xy);

		if (rayDepth > ssRayDepth + PerPixelCompareBias && rayDepth < ssRayDepth + PerPixelThickness)
		{
			float ssOirDepth = GetWordPosScreenDepth(oir);
			vec2 ssHitPos = GetScreenCoordinate(ray); 
			vec3 hitPosNormal = getNormal(ssHitPos);
			if (ssOirDepth > ssRayDepth || dot(hitPosNormal, oirNormal) > 0.99)
				return false;
			hitPos = ray;
			return true;
		}
	}
	return false;
}

vec3 EvalDiffuse(vec3 wi, vec3 wo, vec2 uv)
{
	vec3 diffuse = getKd(uv);
	vec3 normal = getNormal(uv);
	float COS = max(0.0f, dot(wi, normal));
	return diffuse * INV_PI * COS;
}

vec3 reflectRayMatch(vec3 wordPos, vec3 viewPos, vec3 fragNormal, out vec3 hitPos)
{
	vec3 color = vec3(0.0);
	if (fragNormal == vec3(0.0))
		return color;
	vec3 fragWi = normalize(wordPos - viewPos);
	vec3 dir = normalize(reflect(fragWi, fragNormal));
	bool hit = rayMatch(wordPos, dir, fragNormal, hitPos);
	vec2 ssHitPos = GetScreenCoordinate(hitPos);
	vec3 hitPosColor = vec3(texture(gColorBuffer, ssHitPos));
	if (hit)
		color = hitPosColor;
	return color;
}

void main()
{
    vec3 color = vec3(0.0f);
	vec2 screenCoord = vec2((gl_FragCoord.xy - 0.5) / SCREEN_SIZE);
	vec3 wordPos = getPos(screenCoord);
	vec3 FragKd = getKd(screenCoord);
	vec3 FragNormal = getNormal(screenCoord);
	float FragDepth = getMipMapDepth(screenCoord);
	if (!haveKd(screenCoord))
	{
		vec3 hitPos = vec3(0.0f);
		vec3 Li = reflectRayMatch(wordPos, viewPos, FragNormal, hitPos);
		vec3 wi = normalize(hitPos - wordPos);
		vec3 wo = normalize(viewPos - wordPos);
		color = Li * EvalDiffuse(wi, wo, screenCoord);
	}
	color += getColor(screenCoord);
	FragColor = vec4(color, 1.0f);
}