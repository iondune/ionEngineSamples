#version 330

#define LIGHT_MAX 4

struct SLight
{
	vec3 Position;
	vec3 Color;
	float Radius;
};

struct SMaterial
{
	vec3 AmbientColor;
	vec3 DiffuseColor;
	vec3 SpecularColor;
	float Shininess;
};

in vec3 fNormal;
in vec3 fWorldPosition;

uniform vec3 uCameraPosition;
uniform int uPointLightsCount;
uniform SLight uPointLights[LIGHT_MAX];
uniform bool uLightsVisible0;
uniform bool uLightsVisible1;
uniform bool uLightsVisible2;
uniform bool uLightsVisible3;

uniform SMaterial uMaterial;
uniform float uRoughness;
uniform int uShadingModel;

out vec4 outColor;

#include "Util.glsl"


void main()
{
	bool LightsVisible[4];
	LightsVisible[0] = uLightsVisible0;
	LightsVisible[1] = uLightsVisible1;
	LightsVisible[2] = uLightsVisible2;
	LightsVisible[3] = uLightsVisible3;

	vec3 V = normalize(uCameraPosition - fWorldPosition);
	vec3 N = normalize(fNormal);

	vec3 Ambient = uMaterial.AmbientColor;
	vec3 Diffuse = vec3(0);
	vec3 Specular = vec3(0);

	for (int i = 0; i < LIGHT_MAX && i < uPointLightsCount; ++ i)
	{
		if (! LightsVisible[i])
			continue;

		vec3 LightVector = uPointLights[i].Position - fWorldPosition;

		vec3 L = normalize(LightVector);
		vec3 R = reflect(-L, N);
		vec3 H = normalize(V + L);

		float Distance = length(LightVector);
		float Attenuation = 1.0 / sq(Distance / uPointLights[i].Radius + 1);

		vec3 Kd = uMaterial.DiffuseColor;
		vec3 Ks = uMaterial.SpecularColor;
		vec3 Lc = uPointLights[i].Color;

		float Shading = clamp(dot(N, L), 0.0, 1.0);
		Diffuse += Kd * Shading * Attenuation * Lc;

		float roughness = uRoughness;
		float alpha = sq(roughness);
		float power = 2.0 / sq(alpha) - 2.0;

		if (uShadingModel == 0) // Phong
		{
			float Highlight = pow(clamp(dot(V, R), 0.0, 1.0), power) ;
			Specular += Ks * Highlight * Attenuation * Lc;
		}
		else if (uShadingModel == 1) // Blinn-Phong
		{
			float Highlight = pow(clamp(dot(H, N), 0.0, 1.0), power) ;
			Specular += Ks * Highlight * Attenuation * Lc;
		}
		else if (uShadingModel == 2) // Beckmann
		{
			float HdotN = dot(H, N);
			float Highlight = exp((sq(HdotN) - 1.0) / (sq(alpha * HdotN))) / (pow4(HdotN));

			Specular += Ks * Highlight * Attenuation * Lc;
		}
	}

	// Ambient = vec3(0.0);
	// Diffuse = vec3(0.0);

	outColor = vec4(Specular + Diffuse + Ambient, 1);
}
