
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
in vec4 fLightSpacePosition;

uniform int uDirectionalLightsCount;
uniform SDirectionalLight uDirectionalLights[LIGHT_MAX];
uniform SMaterial uMaterial;

uniform sampler2D uShadowMap;

out vec4 outColor;


float ShadowCalculation(vec4 fragPosLightSpace)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(uShadowMap, projCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;

	float bias = 0.005;
	// Check whether current frag pos is in shadow
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

	return shadow;
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

	float Shadow = ShadowCalculation(fLightSpacePosition);
	outColor = vec4((1.0  - Shadow) * Diffuse + uMaterial.AmbientColor, 1);

	if (Shadow > 0.0)
	{
		// outColor.rgb = vec3(0.0, 1.0, 1.0);
	}
}
