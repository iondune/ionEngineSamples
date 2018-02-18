
#version 330

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

uniform bool uDebugShadows;
uniform float uShadowBias;

uniform sampler2D uShadowMap;

out vec4 outColor;


void main()
{

	vec3 Color = vec3(0);


	//////////////////////////////
	// Standard Diffuse Shading //
	//////////////////////////////

	vec3 Diffuse = vec3(0);
	vec3 Ambient = uMaterial.AmbientColor * 0.75;

	for (int i = 0; i < LIGHT_MAX && i < uDirectionalLightsCount; ++ i)
	{
		vec3 nLight = normalize(-uDirectionalLights[i].Direction);
		vec3 nNormal = normalize(fNormal);

		float Shading = clamp(dot(nNormal, nLight), 0.0, 1.0);
		Diffuse += uMaterial.DiffuseColor * Shading * uDirectionalLights[i].Color;
	}
	Color = Diffuse + Ambient;



	////////////////////////
	// Shadow Calculation //
	////////////////////////

	// Transform to NDC
	vec3 ndc = fLightSpacePosition.xyz / fLightSpacePosition.w;

	// Read depth from shadow map
	float closestDepth = texture(uShadowMap, ndc.xy * 0.5 + vec2(0.5)).r;

	// Calculate depth in light space
	float currentDepth = ndc.z * 0.5 + 0.5;

	if (ndc.x > 1.0 || ndc.x < -1.0)
	{
		// Point is outside of shadow view - "not in shadow"
		if (uDebugShadows)
		{
			Color.rgb = vec3(1.0, 0.0, 0.0);
		}
	}
	else if (ndc.y > 1.0 || ndc.y < -1.0)
	{
		// Point is outside of shadow view - "not in shadow"
		if (uDebugShadows)
		{
			Color.rgb = vec3(1.0, 0.5, 0.0);
		}
	}
	else if (ndc.z > 1.0 || ndc.z < -1.0)
	{
		// Point is outside of shadow view - "not in shadow"
		if (uDebugShadows)
		{
			Color.rgb = vec3(1.0, 0.0, 1.0);
		}
	}
	else if (currentDepth - uShadowBias < closestDepth)
	{
		// Point is in front of shadow map value - "not in shadow"
		if (uDebugShadows)
		{
			Color.rgb = vec3(0.0, 1.0, 0.0) * currentDepth;
		}
	}
	else
	{
		Color = Ambient;

		// In shadow
		if (uDebugShadows)
		{
			Color.rgb = vec3(0.0, 0.0, 1.0) * currentDepth;
		}
	}



	outColor = vec4(Color, 1.0);
}
