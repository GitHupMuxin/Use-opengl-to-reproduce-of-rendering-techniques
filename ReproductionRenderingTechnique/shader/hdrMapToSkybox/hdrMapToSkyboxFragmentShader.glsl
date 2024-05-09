#version 330 core

in vec3 TexCoord;

uniform sampler2D skyBox;

out vec4 FragColor;

const float PI = 3.14159265359;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(TexCoord));
	vec3 color = texture(skyBox, uv).rgb;
	FragColor = vec4(color, 1.0);
}