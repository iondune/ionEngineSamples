
#version 150

#define LIGHT_MAX 4

struct SMaterial
{
	vec3 AmbientColor;
	vec3 DiffuseColor;
};

struct SDirectionalLight
{
	vec3 Direction;
	vec3 Color;
};

in vec3 fNormal;
in vec3 fWorldPosition;

uniform int uDirectionalLightsCount;
uniform SDirectionalLight uDirectionalLights[LIGHT_MAX];
uniform SMaterial uMaterial;

out vec4 outColor;


float sq(float v)
{
	return v * v;
}

void main()
{
	vec3 Diffuse = vec3(0);

	for (int i = 0; i < LIGHT_MAX && i < uDirectionalLightsCount; ++ i)
	{
		vec3 nLight = normalize(-uDirectionalLights[i].Direction);
		vec3 nNormal = normalize(fNormal);

		float Shading = clamp(dot(nNormal, nLight), 0.0, 1.0);
		Diffuse += uMaterial.DiffuseColor * Shading * uDirectionalLights[i].Color;
	}

	outColor = vec4(Diffuse + uMaterial.AmbientColor, 1);
}
