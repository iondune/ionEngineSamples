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
uniform float uMetalness;
uniform float uIOR;

uniform int uDChoice;
uniform int uGChoice;

uniform int uDebugExclusive;

out vec4 outColor;

#include "Util.glsl"



float NDF_BlinnPhong(vec3 H, vec3 N, float alpha)
{
	float NormFactor = 1.0 / (Pi * sq(alpha));

	float power = 2.0 / sq(alpha) - 2.0;
	float HdotN = saturate(dot(H, N));

	return NormFactor * pow(HdotN, power);
}

float NDF_Beckmann(vec3 H, vec3 N, float alpha)
{
	float NormFactor = 1.0 / (Pi * sq(alpha));

	float HdotN = saturate(dot(H, N));

	if (HdotN == 0)
	{
		return 0.0;
	}

	return NormFactor * exp((sq(HdotN) - 1.0) / (sq(alpha * HdotN))) / (pow4(HdotN));
}

float NDF_GGX_Trowbridge_Reitz(vec3 H, vec3 N, float alpha)
{
	float HdotN = saturate(dot(H, N));
	return sq(alpha) / (Pi * sq(sq(HdotN) * (sq(alpha) - 1) + 1));
}

float Partial_GGX(vec3 V, vec3 N, float alpha)
{
	return 2.0 / (1.0 + sqrt(sq(alpha) + (1.0 - sq(alpha))));
}

float G_GGX(vec3 L, vec3 V, vec3 N, float alpha)
{
	return Partial_GGX(L, N, alpha) * Partial_GGX(V, N, alpha);
}


float chi_positive(float v)
{
	return v > 0 ? 1 : 0;
}

float GGX_Distribution(vec3 H, vec3 N, float alpha)
{
	float NdotH = dot(N, H);
	float alpha2 = sq(alpha);
	float NdotH2 = sq(NdotH);

	float denominator = NdotH2 * alpha2 + (1 - NdotH2);

	return (alpha2 * chi_positive(NdotH)) / (Pi * sq(denominator));
}

float GGX_Partial(vec3 v, vec3 g, vec3 m, float alpha)
{
	float chi = chi_positive(dot(v, m) / dot(v, g));
	float tan2theta = (1.0 - sq(dot(v, g))) / sq(dot(v, g));
	// float tan2theta = sq(tan_theta);

	return chi * 2.0 / (1.0 + sqrt(1.0 + sq(alpha) * tan2theta));
}

// float GGX_PartialGeometryTerm(float VdotH, float VdotN, float alpha)
// {
// 	float chi = chi_positive(saturate(VdotH) / saturate(VdotN));
// 	float VdotH2 = sq(saturate(VdotH));
// 	float tan2 = ( 1 - VdotH2 ) / VdotH2;
// 	return (chi * 2) / ( 1 + sqrt( 1 + sq(alpha) * tan2 ) );
// }

// float GGX_PartialGeometryTerm(vec3 v, vec3 n, vec3 h, float alpha)
// {
// 	float VoH2 = saturate(dot(v,h));
// 	float chi = chiGGX( VoH2 / saturate(dot(v,n)) );
// 	VoH2 = VoH2 * VoH2;
// 	float tan2 = ( 1 - VoH2 ) / VoH2;
// 	return (chi * 2) / ( 1 + sqrt( 1 + alpha * alpha * tan2 ) );
// }


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

	float s = uMetalness;
	float d = (1.0 - s);


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
		Diffuse += d * Kd * Shading * Attenuation * Lc;


		float roughness = uRoughness;
		float alpha = sq(roughness);
		float power = 2.0 / sq(alpha) - 2.0;

		float HdotN = saturate(dot(H, N));
		float NdotV = saturate(dot(N, V));
		float NdotL = saturate(dot(N, L));
		float LdotH = saturate(dot(L, H));
		float VdotH = saturate(dot(V, H));
		float VdotN = saturate(dot(V, N));

		float D = 1.0;
		if (uDChoice == 0)
		{
			D = NDF_BlinnPhong(H, N, alpha);
		}
		else if (uDChoice == 1)
		{
			D = NDF_Beckmann(H, N, alpha);
		}
		else if (uDChoice == 2)
		{
			D = NDF_GGX_Trowbridge_Reitz(H, N, alpha);
		}

		float G = 1.0;
		if (uGChoice == 0)
		{
			// Cook-Torrance
			G = min3(1.0, (2.0*HdotN*NdotV/VdotH), (2.0*HdotN*NdotL/VdotH));
		}
		else if (uGChoice == 1)
		{
			// G = G_GGX(L, V, N, alpha);
			// G = GGX_PartialGeometryTerm(VdotH, VdotN, alpha) * GGX_PartialGeometryTerm(LdotH, NdotL, alpha);
			G = GGX_Partial(V, N, H, alpha) * GGX_Partial(L, N, H, alpha);
		}

		// float D = beckman_actual(HdotN, alpha);
		// float G = NdotL * NdotV / sq(VdotH);
		// float G = NdotL * NdotV / max(NdotL, NdotV);
		// float G =

		float F0 = sq((uIOR - 1) / (uIOR + 1));
		float F = F0 + (1.0 - F0) * pow(1.0 - dot(V, H), 5.0);

		float Denom = 4.0 * max(1.0 / 16.0, abs(dot(N, V)));
		float Highlight = D * G * F / Denom;
		// float Highlight = D / Denom;
		// float Highlight = 1.0 / Denom;
		// float Highlight = D;
		// float Highlight = G;
		// float Highlight = F;
		// float Highlight = D * G / Denom;

		// float a2 = alpha;
		// float G_V = NdotV + sqrt( (NdotV - NdotV * a2) * NdotV + a2 );
		// float G_L = NdotL + sqrt( (NdotL - NdotL * a2) * NdotL + a2 );
		// float Highlight = rcp( G_V * G_L ) * D * F;


		Specular += s * Ks * Highlight * Attenuation * Lc;
		// Specular += vec3(1.0) * Highlight;

		if (uDebugExclusive == 1)
		{
			if (D > 0)
			{
				outColor = vec4(vec3(D), 1.0);
			}
			else
			{
				outColor = vec4(vec2(saturate(abs(D)), 0.0).xyx, 1.0);
			}
			return;
		}
		else if (uDebugExclusive == 2)
		{
			outColor = vec4(vec3(G), 1.0);
			return;
		}
		else if (uDebugExclusive == 3)
		{
			outColor = vec4(vec3(F), 1.0);
			return;
		}
		else if (uDebugExclusive == 4)
		{
			outColor = vec4(vec3(1.0 / Denom), 1.0);
			return;
		}
	}

	// Ambient = vec3(0.0);
	// Diffuse = vec3(0.0);

	outColor = vec4(Specular + Diffuse + Ambient, 1.0);
}
