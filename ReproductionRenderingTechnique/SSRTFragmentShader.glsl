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
uniform sampler2D gViewPosBuffer;
uniform sampler2D gViewDepthBuffer;
uniform vec2 SCREEN_SIZE;
uniform int maxLevel;

in mat4 P;
in mat4 V;
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

// normalize sampler2D dir
vec3 get3DRandDir(vec2 uv, inout float pdf)
{
	float phi = Rand1(uv.x) * PI;
	float beta = Rand1(uv.y) * PI2;
	pdf = INV_TWO_PI;
	return vec3(cos(beta) * cos(phi), sin(beta) * cos(phi), sin(phi));
}

vec4 normalizeW(vec4 v)
{
	return v / v.w;
}

float getSSDepth(vec2 uv)
{
	return texture(gPosBuffer, uv).w;
}

float getViewDepthBuffer(vec2 uv)
{
	ivec2 texSize = textureSize(MipMap, 0);
	ivec2 UV = ivec2(texSize * uv);
	return texelFetch(gViewDepthBuffer, UV, 0).r;
}

float getViewDepthBuffer(ivec2 uv)
{
	return texelFetch(gViewDepthBuffer, uv, 0).r;
}

float getMipMapDepth(vec2 uv, int level)
{
	ivec2 texSize = textureSize(MipMap, level);
	ivec2 UV = ivec2(texSize * uv);
	return texelFetch(MipMap, UV, level).r;
}

float getMipMapDepth(ivec2 uv, int level)
{
	return texelFetch(MipMap, uv, level).r;
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

vec3 getPos(ivec2 uv)
{
	vec2 p = vec2(uv) / SCREEN_SIZE;
	return vec3(texture(gPosBuffer, p));
}

vec3 getViewSpacePos(vec2 uv)
{
	return texture(gViewPosBuffer, uv).xyz;
}

vec3 getColor(vec2 uv)
{
	return vec3(texture(gColorBuffer, uv));
}

// to ndc x : 0 - 1 y : 0 - 1
vec2 GetScreenCoordinate(vec3 posWorld)
{
	return normalizeW(PV * vec4(posWorld, 1.0f)).xy * 0.5f + vec2(0.5f);
}

vec2 ViewSpaceToScreenSpace(vec3 posView)
{
	return normalizeW(P * vec4(posView, 1.0f)).xy * 0.5f + 0.5f;
}

float GetWordPosScreenDepth(vec3 posWorld)
{
	return (PV * vec4(posWorld, 1.0f)).w;
}

vec3 GetScreenSpaceRay(vec3 posWorld)
{
	vec4 temp = PV * vec4(posWorld, 1.0f);
	temp.xyz /= temp.w;
	return vec3(temp.xy * 0.5 + 0.5, temp.z);
}

bool between(vec2 bt, float x)
{
	return bt.x < x && x < bt.y;
}

bool between(vec2 bt, vec2 x)
{
	return bt.x < x.x && bt.y > x.y;
}

float distanceSquared(vec2 p0, vec2 p1)
{
	p1 -= p0;
	return dot(p1, p1);
}

bool rayMatch2DMy(vec3 oir, vec3 dir, vec3 oirNormal, out vec3 hitPos)
{
	float scale = 10.0f;
	vec3 starPos = oir;
	vec3 endPos = oir + dir * scale;

	vec4 V0 = V * vec4(starPos, 1.0f), V1 = V * vec4(endPos, 1.0);

	vec4 H0 = P * V0, H1 = P * V1;

	float K0 = 1.0 / H0.w, K1 = 1.0 / H1.w;

	float Q0 = V0.z * K0, Q1 = V1.z * K1;

	vec2 P0 = ((H0.xy * K0) * 0.5 + 0.5) * SCREEN_SIZE, P1 = ((H1.xy * K1) * 0.5 + 0.5) * SCREEN_SIZE;

	P1 += vec2(distanceSquared(P0, P1) < 0.0001 ? 0.01 : 0.0);

	float Step = 0.0f;
	vec2 delta = (P1 - P0);
	float k = delta.y / delta.x;
	float invK = 1.0 / k;
	float dx, dy, dQ, dK; 
	if (abs(k) < 1.0)
	{
		Step = abs(delta.x);
		dx = delta.x / Step;
		dy = delta.y / Step;
		dQ = (Q1 - Q0) / Step;
		dK = (K1 - K0) / Step;
		Step = min(Step, (SCREEN_SIZE.x - P0.x));
	}
	else
	{
		Step = abs(delta.y);
		dx = delta.x / Step;
		dy = delta.y / Step;
		dQ = (Q1 - Q0) / Step; 
		dK = (K1 - K0) / Step; 
		Step = min(Step, (SCREEN_SIZE.y - P0.y));
	}

	float stride = 1.0;
	float jitter = 2.0;
	float zThickness = 0.001;

	vec2 P = P0;
	float Q = Q0;
	float K = K0;

	vec2 dP = vec2(dx, dy);
	dP *= stride;
	dQ *= stride;
	dK *= stride;

	Step /= stride;

	P += dP * jitter;
	Q += dQ * jitter;
	K += dK * jitter;
	float lastDepth = Q / K;

	for (int i = 0; i < Step; i++)
	{
		P += dP;
		Q += dQ;
		K += dK;

		float nowDepth = (Q + dQ * 0.5) / (K + dK * 0.5);
		vec2 passDepth = vec2(lastDepth, nowDepth);
		lastDepth = nowDepth;
		if (passDepth.x > passDepth.y)
			passDepth = passDepth.yx;

		float ssRayDepth = -getViewDepthBuffer(ivec2(P));
		vec2 ssSize = vec2(ssRayDepth - zThickness, ssRayDepth);
		if (between(passDepth, ssSize))
		{
			vec3 hitPosNormal = getNormal(P);
			hitPos = getPos(ivec2(P));
			return true;
		}
	}
	return false;
}

bool rayMatch2D(vec3 oir, vec3 dir, vec3 oirNormal, out vec3 hitPos)
{
	float scale = 10.0f;
	vec3 starPos = oir;
	vec3 endPos = oir + dir * scale;

	vec4 V0 = V * vec4(starPos, 1.0f), V1 = V * vec4(endPos, 1.0);

	vec4 H0 = P * V0, H1 = P * V1;

	float K0 = 1.0 / H0.w, K1 = 1.0 / H1.w;

	float Q0 = V0.z * K0, Q1 = V1.z * K1;

	vec2 P0 = ((H0.xy * K0) * 0.5 + 0.5) * SCREEN_SIZE, P1 = ((H1.xy * K1) * 0.5 + 0.5) * SCREEN_SIZE;

	P1 += vec2(distanceSquared(P0, P1) < 0.0001 ? 0.01 : 0.0);

	float Step = 0.0f;
	vec2 delta = P1 - P0;

	bool tranceX = true;

	if (abs(delta.x) < abs(delta.y))
	{
		tranceX = false;
		P0 = P0.yx;
		P1 = P1.yx;
		delta = delta.yx;
	}

	float stepDir = sign(delta.x);
	float invStepSize = stepDir / delta.x;

	vec2 dP = vec2(stepDir, delta.y * invStepSize);
	float dQ = (Q1 - Q0) * invStepSize;
	float dK = (K1 - K0) * invStepSize;

	float stride = 2.0;
	float jitter = 2.0;
	float zThickness = 0.0;

	dP *= stride;
	dQ *= stride;
	dK *= stride;

	P0 += dP * jitter;
	Q0 += dQ * jitter;
	K0 += dK * jitter;
	float lastDepth = V0.z;

	vec2 P = P0;
	float Q = Q0;
	float K = K0;
	float end = P1.x * stepDir, stepCount = 0.0;
	float maxStep = 1000;
	while (P.x * stepDir < end && stepCount < maxStep)
	{
		vec2 ssHitPos = tranceX ? P : P.yx;

		float nowDepth = (Q + dQ * 0.5) / (K + dK * 0.5);
		vec2 passDepth = vec2(lastDepth, nowDepth);
		lastDepth = nowDepth;
		if (passDepth.x > passDepth.y)
			passDepth = passDepth.yx;

		float ssRayDepth = -getViewDepthBuffer(ivec2(ssHitPos));
		vec2 ssSize = vec2(ssRayDepth - zThickness, ssRayDepth);
		//if (between(passDepth, ssSize))
		if (between(passDepth, ssRayDepth))
		{
			vec3 hitPosNormal = getNormal(ssHitPos);
			hitPos = getPos(ivec2(ssHitPos));
			return true;
		}
		P += dP; Q += dQ; K += dK; stepCount++;
	}
	return false;
}

bool rayMatch3DWithDDA(vec3 oir, vec3 dir, vec3 oirNormal, out vec3 hitPos)
{
	float scale = 10.0f;
	vec3 starPos = oir;
	vec3 endPos = oir + dir * scale;
	float deltaX = (endPos.x - starPos.x);
	float deltaY = (endPos.y - starPos.y);
	float deltaZ = (endPos.z - starPos.z);

	vec3 ssStar = GetScreenSpaceRay(starPos);
	vec3 ssEnd = GetScreenSpaceRay(endPos);
	float Step = 0.0f;

	float ssDeltaX = (ssEnd.x - ssStar.x);
	float ssDeltaY = (ssEnd.y - ssStar.y);
	float k = ssDeltaY / ssDeltaX;
	float invK = 1.0 / k;
	float dx, dy, dz;
	vec3 stepVec = vec3(0.0f);
	if (abs(k) < 1.0)
	{
		dx = 1.0 / SCREEN_SIZE.x;
		dy = k * dx;
		dz = (ssEnd.z - ssStar.z) / ssDeltaX * dx;
		Step = abs(ssDeltaX) * SCREEN_SIZE.x;
		stepVec.x = deltaX / Step;
		stepVec.y = deltaY / Step;
		stepVec.z = deltaZ / Step;
		Step = min(Step, (1.0 - ssStar.x) * SCREEN_SIZE.x);
	}
	else
	{
		dy = 1.0 / SCREEN_SIZE.y;	
		dx = invK * dy;
		dz = (ssEnd.z - ssStar.z) / ssDeltaY * dy; 
		Step = abs(ssDeltaY) * SCREEN_SIZE.y;
		stepVec.x = deltaX / Step;
		stepVec.y = deltaY / Step;
		stepVec.z = deltaZ / Step;
		Step = min(Step, (1.0 - ssStar.y) * SCREEN_SIZE.y);
	}
	
	float PerPixelThickness = 0.20;
	float PerPixelCompareBias = 0.1025;
	vec3 ray = oir;
	for (int i = 0; i < Step; i++)
	{
		ray += stepVec;
		float rayDepth = GetWordPosScreenDepth(ray);
		vec2 ssRay = GetScreenCoordinate(ray);

		float ssRayDepth = getSSDepth(ssRay);
		if (rayDepth > ssRayDepth + PerPixelCompareBias && rayDepth < ssRayDepth + PerPixelThickness)
		{
			float ssOirDepth = GetWordPosScreenDepth(oir);
			vec3 hitPosNormal = getNormal(ssRay);
			if (ssOirDepth > ssRayDepth || dot(hitPosNormal, oirNormal) > 0.99)
				return false;
			hitPos = getPos(ssRay);
			return true;
		}
	}
	return false;
}

bool rayMatch3DTextureSpace(vec3 oir, vec3 dir, vec3 oirNormal, out vec3 hitPos)
{
	vec3 starPos = oir;
	vec3 endPos = oir + dir * 100;

	vec4 V0 = V * vec4(starPos, 1.0f), V1 = V * vec4(endPos, 1.0f);
	vec4 H0 = P * V0, H1 = P * V1;
	vec3 P0 = (H0.xyz / H0.w) * 0.5 + 0.5, P1 = (H1.xyz / H1.w) * 0.5 + 0.5;
	vec3 delta = P1 - P0;

	int Step = 1000;

	vec3 dP = delta / Step;

	vec3 P = P0;

	float lastDepth = P0.z;

	for (int i = 0; i < Step; i++)
	{
		P += dP;
		if (P.x < 0 || P.x > 1 || P.y < 0 || P.y > 1 || P.z < 0 || P.z > 1)
			return false;
		float nowDepth = P.z;
		vec2 passDepth = vec2(lastDepth, nowDepth);
		passDepth = passDepth.x < passDepth.y ? passDepth : passDepth.yx;
		lastDepth = nowDepth;

		float ssDepth = texture(MipMap, P.xy).r;

		float difDepth = P.z - ssDepth;
		//vec2 Thickness = vec2(ssDepth - zThickness, ssDepth);
		//if (between(passDepth, ssDepth))
		if (difDepth > 0 && difDepth < 0.00004)
		{
			hitPos = vec3(getPos(P.xy));
			return true;
		}
	}
	return false;
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

		float ssRayDepth = getSSDepth(screenSpaceRay.xy);

		if (rayDepth > ssRayDepth + PerPixelCompareBias && rayDepth < ssRayDepth + PerPixelThickness)
		{
			float ssOirDepth = GetWordPosScreenDepth(oir);
			vec2 ssHitPos = screenSpaceRay; 
			vec3 hitPosNormal = getNormal(ssHitPos);
			if (ssOirDepth > ssRayDepth || dot(hitPosNormal, oirNormal) > 0.99)
				return false;
			hitPos = ray;
			return true;
		}
	}
	return false;
}

vec2 getCellId(vec2 uv, vec2 cellCount)
{
	return floor(uv * cellCount);
}

vec2 getCellCount(float level)
{
	return SCREEN_SIZE / exp2(level);
}

bool isSameCell(vec2 cellIdOne, vec2 cellIdTwo)
{
	ivec2 cell0 = ivec2(cellIdOne);
	ivec2 cell1 = ivec2(cellIdTwo);
	return cell0.x == cell1.x && cell0.y == cell1.y;
}

vec3 getIntersection(vec3 oir, vec3 dir, vec2 cellId, vec2 cellCount, vec2 dirStep, vec2 dirOffset)
{
	vec2 cellSize = 1.0f / cellCount;
	vec2 cellxy = cellId / cellCount + cellSize * dirStep;
	vec2 result = (cellxy - oir.xy) / dir.xy;
	vec3 position = oir + dir * min(result.x, result.y);
	position.xy += (result.x < result.y) ? vec2(dirOffset.x, 0.0) : vec2(0.0, dirOffset.y);
	return position;
}

vec3 tranceRay(vec3 oir, vec3 dir, float t)
{
	return oir + dir * t;
}

bool rayMatchWitMipMap(vec3 oir, vec3 dir, vec3 oirNormal, out vec3 hitPos)
{
	float scale = 10.0f;
	vec3 starPos = oir;
	vec3 endPos = oir + dir * scale;
	vec4 V0 = V * vec4(starPos, 1.0f), V1 = V * vec4(endPos, 1.0f);
	vec4 H0 = P * V0, H1 = P * V1;
	vec3 P0 = (H0 / H0.w).xyz * 0.5 + 0.5, P1 = (H1 / H1.w).xyz * 0.5 + 0.5;

	int level = 0;
	vec3 ray = P0;
	vec3 rayDir = normalize(P1 - P0);
	vec3 vZ = rayDir / rayDir.z;

	vec2 hiZSize = getCellCount(level);
	//vec2 dirStep = vec2(sign(d.x), sign(d.y));
	vec2 dirStep = vec2(rayDir.x >= 0 ? 1.0 : -1.0, rayDir.y >= 0 ? 1.0 : -1.0);
	vec2 dirOffset = dirStep * 0.00001;
	dirStep = clamp(dirStep, vec2(0.0), vec2(1.0));

	vec2 nowCellId = getCellId(ray.xy, hiZSize);
	ray = getIntersection(ray, rayDir, nowCellId, hiZSize, dirStep, dirOffset);//先trace到当前格子

	while (level >= 0)
	{
		 vec2 currentCellCount = getCellCount(level);
		 vec2 oldCellId = getCellId(ray.xy, currentCellCount);

		 float minZ = getMipMapDepth(ray.xy, level);
		 vec3 tempRay = ray;
		 if (rayDir.z > 0)
		 {
			float stepRay = minZ - ray.z;
			tempRay = stepRay > 0 ? ray + vZ * stepRay : tempRay;//以当前格子的深度为步进进行trace
			vec2 newCellId = getCellId(tempRay.xy, currentCellCount);
			if (!isSameCell(oldCellId, newCellId))//如果来到下一个格子
			{
				tempRay = getIntersection(ray, rayDir, oldCellId, currentCellCount, dirStep, dirOffset);
				level = min(maxLevel, level + 2);
				//走一个step并level + 1(循环末尾有level--)
			}
			else 
			{
				/*
				if (level == 1 && abs(stepRay) > 0.00001) //还是同一个格子
				{
					tempP = getIntersection(P, Dir, oldCellId, currentCellCount, dirStep, dirOffset);
					level = 2;
					//走一个step level不变
				}
				*/
			}
		 }
		 else if (ray.z < minZ)//如果没有hit
		 {
			tempRay = getIntersection(ray, rayDir, oldCellId, currentCellCount, dirStep, dirOffset);
			level = min(maxLevel, level + 2);
			//走一个step并level + 1(循环末尾有level--)
		 }
		 ray = tempRay;
		 level--;
	}
	hitPos = getPos(ray.xy);
	return true;
}

bool rayMatchWitMipMapMy(vec3 oir, vec3 dir, vec3 oirNormal, out vec3 hitPos)
{
	vec3 starPos = oir;
	vec3 endPos = oir + dir * 100;

	vec4 V0 = V * vec4(starPos, 1.0f), V1 = V * vec4(endPos, 1.0f);
	vec4 H0 = P * V0, H1 = P * V1;
	vec3 P0 = (H0.xyz / H0.w) * 0.5 + 0.5, P1 = (H1.xyz / H1.w) * 0.5 + 0.5;

	int level = 0;
	vec3 star = P0;
	vec3 rayDir = normalize(P1 - P0);

	vec2 dirStep = vec2(rayDir.x >= 0 ? 1.0 : -1.0, rayDir.y >= 0 ? 1.0 : -1.0);
	vec2 dirOffset = dirStep * 0.00001;
	dirStep = clamp(dirStep, 0.0, 1.0);

	float maxTranceX = rayDir.x >= 0.0 ? (1.0 - star.x) / rayDir.x : -star.x / rayDir.x;
	float maxTranceY = rayDir.y >= 0.0 ? (1.0 - star.y) / rayDir.y : -star.y / rayDir.y;
	float maxTranceZ = rayDir.z >= 0.0 ? (1.0 - star.z) / rayDir.z : -star.z / rayDir.z;
	float maxTranceDistance = min(maxTranceX, min(maxTranceY, maxTranceZ));

	vec3 o = star;
	vec3 d = rayDir * maxTranceDistance;
	float deltaZ = rayDir.z * maxTranceDistance;
	vec2 starCellCount = getCellCount(level);
	vec2 starCellId = getCellId(o.xy, starCellCount);
	
	vec3 ray = P0;
	int Step = 100;

	float lastDepth = ray.z;


	for (int i = 0; i < Step; i++)
	{
		vec2 currentCellCount = getCellCount(level);
		vec2 currentCellId = getCellId(ray.xy, currentCellCount);
		vec3 tempRay = getIntersection(ray, d, currentCellId, currentCellCount, dirStep, dirOffset);

		float nowDepth = tempRay.z;
		vec2 passDepth = vec2(lastDepth, nowDepth);
		passDepth = passDepth.x < passDepth.y ? passDepth : passDepth.yx;
		lastDepth = nowDepth;

		float ssDepth = getMipMapDepth(tempRay.xy, level);

		float difDepth = tempRay.z - ssDepth;
		//vec2 Thickness = vec2(ssDepth - zThickness, ssDepth);
		if (difDepth > 0 && difDepth < 0.0001)
		{
			hitPos = vec3(getPos(tempRay.xy));
			return true;
		}
		ray = tempRay;
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
	//bool hit = rayMatch(wordPos, dir, fragNormal, hitPos);
	//bool hit = rayMatch2D(wordPos, dir, fragNormal, hitPos);
	bool hit = rayMatch2DMy(wordPos, dir, fragNormal, hitPos);
	//bool hit = rayMatch3DWithDDA(wordPos, dir, fragNormal, hitPos);
	//bool hit = rayMatch3DTextureSpace(wordPos, dir, fragNormal, hitPos);

	//还没写好
	//bool hit = rayMatchWitMipMap(wordPos, dir, fragNormal, hitPos);
	//bool hit = rayMatchWitMipMapMy(wordPos, dir, fragNormal, hitPos);

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
	float wordPosDepth = GetWordPosScreenDepth(wordPos);

	vec3 viewSpacePos = getViewSpacePos(screenCoord);

	vec3 FragKd = getKd(screenCoord);
	vec3 FragNormal = getNormal(screenCoord);
	float FragDepth = getSSDepth(screenCoord);
	if (!haveKd(screenCoord))
	{
		vec3 hitPos = vec3(0.0f);
		vec3 Li = reflectRayMatch(wordPos, viewPos, FragNormal, hitPos);
		vec3 wi = normalize(hitPos - wordPos);
		vec3 wo = normalize(viewPos - wordPos);
		color = Li * EvalDiffuse(wi, wo, screenCoord);
	}
	color += getColor(screenCoord);
	FragColor = vec4(color, 1.0);
}
