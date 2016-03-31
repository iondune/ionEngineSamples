#version 150

in vec3 fNormal;
in vec3 fEye;
in vec3 fWorldPosition;

struct SDirectionalLight
{
	vec3 Direction;
	vec3 Color;
};

struct SPointLight
{
	vec3 Position;
	vec3 Color;
	float Radius;
};

#define LIGHT_MAX 1

uniform int uDirectionalLightsCount;
uniform SDirectionalLight uDirectionalLights[LIGHT_MAX];
uniform int uPointLightsCount;
uniform SPointLight uPointLights[LIGHT_MAX];

out vec4 outColor;


float sq(float v)
{
	return v * v;
}

void main()
{
	const float Shininess = 10.0;
	const float AmbientStrength = 0.15;
	const float DiffuseStrength = 0.4;
	const float SpecularStength = 0.4;

	vec3 nEye = normalize(fEye);
	vec3 nNormal = normalize(fNormal);


	vec3 Ambient = vec3(AmbientStrength);
	vec3 Diffuse = vec3(0);
	vec3 Specular = vec3(0);

	for (int i = 0; i < LIGHT_MAX && i < uDirectionalLightsCount; ++ i)
	{
		vec3 nLight = normalize(uDirectionalLights[i].Direction);
		vec3 Reflection = reflect(-nLight, nNormal);

		float Shading = clamp(dot(nNormal, nLight), 0.0, 1.0);
		Diffuse += DiffuseStrength * Shading * uDirectionalLights[i].Color;

		float Highlight = pow(clamp(dot(nEye, Reflection), 0.0, 1.0), Shininess);
		Specular += SpecularStength * Highlight * uDirectionalLights[i].Color;
	}
	for (int i = 0; i < LIGHT_MAX && i < uPointLightsCount; ++ i)
	{
		vec3 LightVector = uPointLights[i].Position - fWorldPosition;
		vec3 nLight = normalize(LightVector);
		vec3 Reflection = reflect(-nLight, nNormal);

		float Shading = clamp(dot(nNormal, nLight), 0.0, 1.0);
		float Distance = length(LightVector);
		float Attenuation = 1.0 / sq(Distance / uPointLights[i].Radius + 1);
		Diffuse += DiffuseStrength * Shading * Attenuation * uPointLights[i].Color;

		float Highlight = pow(clamp(dot(nEye, Reflection), 0.0, 1.0), Shininess);
		Specular += SpecularStength * Highlight * Attenuation * uPointLights[i].Color;
	}

	outColor = vec4(Specular + Diffuse + Ambient, 1);
}
