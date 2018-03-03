
#version 330

#define LIGHT_MAX 4

struct SDirectionalLight
{
	vec3 Color;
	vec3 Direction;
};

in vec3 fNormal;

uniform int uDirectionalLightsCount;
uniform SDirectionalLight uDirectionalLights[LIGHT_MAX];
uniform vec3 uColor;

out vec4 outColor;


void main()
{
	vec3 Diffuse = vec3(0);

	vec3 nNormal = normalize(fNormal);
	for (int i = 0; i < LIGHT_MAX && i < uDirectionalLightsCount; ++ i)
	{
		vec3 nLight = -normalize(uDirectionalLights[i].Direction);
		float Shading = clamp(dot(nNormal, nLight), 0.0, 1.0);

		Diffuse += Shading * uDirectionalLights[i].Color;
	}

	vec3 AmbientColor = vec3(0.2) * uColor;
	vec3 DiffuseColor = vec3(0.8) * uColor;
	outColor = vec4(Diffuse * DiffuseColor + AmbientColor, 1.0);
	// outColor = vec4(nNormal * 0.5 + vec3(0.5), 1.0);
}
