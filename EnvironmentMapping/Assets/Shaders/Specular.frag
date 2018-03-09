#version 330

#define LIGHT_MAX 4

struct SMaterial
{
	vec3 AmbientColor;
	vec3 DiffuseColor;
	vec3 SpecularColor;
	float Shininess;
};

struct SLight
{
	vec3 Position;
	vec3 Color;
	float Radius;
};

in vec3 fNormal;
in vec3 fWorldPosition;

uniform int uPointLightsCount;
uniform SLight uPointLights[LIGHT_MAX];
uniform SMaterial uMaterial;

uniform vec3 uCameraPosition;
uniform sampler2D uEnvironmentMap;

out vec4 outColor;


float sq(float v)
{
	return v * v;
}

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
	vec3 nEye = normalize(normalize(uCameraPosition - fWorldPosition));
	vec3 nNormal = normalize(fNormal);

	vec3 Diffuse = vec3(0);
	vec3 Specular = vec3(0);

	for (int i = 0; i < LIGHT_MAX && i < uPointLightsCount; ++ i)
	{
		vec3 Light = uPointLights[i].Position - fWorldPosition;

		vec3 nLight = normalize(Light);
		vec3 nHalf = normalize(nLight + nEye);

		float Shading = clamp(dot(nNormal, nLight), 0.0, 1.0);
		float Distance = length(Light);
		float Attenuation = 1.0 / sq(Distance / uPointLights[i].Radius + 1);

		Diffuse += uMaterial.DiffuseColor * Shading * Attenuation * uPointLights[i].Color;

		float Highlight = pow(clamp(dot(nHalf, nNormal), 0.0, 1.0), uMaterial.Shininess);
		Specular += uMaterial.SpecularColor * Highlight * Attenuation * uPointLights[i].Color;
	}

	vec3 EnvironmentColor = texture(uEnvironmentMap, SampleSphericalMap(normalize(reflect(-nEye, nNormal)))).rgb;

	outColor = vec4(Specular + Diffuse + uMaterial.AmbientColor, 1);
	outColor.rgb = EnvironmentColor;
}
