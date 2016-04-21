
#version 150

in vec2 fTexCoords;
in vec3 fWorldPosition;

uniform sampler2D uNormalMap;
uniform sampler2D uColorMap;
uniform sampler2D uHeightMap;

uniform int uSamplingMode;
uniform int uDebugDisplay;


#define LIGHT_MAX 3

struct SLight
{
	vec3 Direction;
	vec3 Color;
};

uniform SLight uDirectionalLights[LIGHT_MAX];
uniform int uDirectionalLightsCount;

out vec4 outColor;


// Cubic texture interpolation
// Looks a lot better than the bilinear interpolation that we get by default
// Utilizes the built in filter to get away with only four samples
// (normally cubic would take 16)
// http://www.java-gaming.org/index.php?topic=35123.0

vec4 cubic(float v)
{
	vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
	vec4 s = n * n * n;
	float x = s.x;
	float y = s.y - 4.0 * s.x;
	float z = s.z - 4.0 * s.y + 6.0 * s.x;
	float w = 6.0 - x - y - z;
	return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D tex, vec2 texCoords)
{
	vec2 texSize = textureSize(tex, 0);
	vec2 invTexSize = 1.0 / texSize;

	texCoords = texCoords * texSize - 0.5;

	vec2 fxy = fract(texCoords);
	texCoords -= fxy;

	vec4 xcubic = cubic(fxy.x);
	vec4 ycubic = cubic(fxy.y);

	vec4 c = texCoords.xxyy + vec2(-0.5, +1.5).xyxy;

	vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
	vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;

	offset *= invTexSize.xxyy;

	vec4 sample0 = texture(tex, offset.xz);
	vec4 sample1 = texture(tex, offset.yz);
	vec4 sample2 = texture(tex, offset.xw);
	vec4 sample3 = texture(tex, offset.yw);

	float sx = s.x / (s.x + s.y);
	float sy = s.z / (s.z + s.w);

	return mix(
		mix(sample3, sample2, sx), mix(sample1, sample0, sx),
		sy);
}


void main()
{
	if (uDebugDisplay == 4) // Texture Coordinates
	{
		outColor = vec4(fTexCoords, vec2(1.0));
		return;
	}

	vec3 Normal = textureBicubic(uNormalMap, fTexCoords).rgb;
	vec3 Color = textureBicubic(uColorMap, fTexCoords).rgb;

	if (uDebugDisplay == 1) // Normals
	{
		outColor = vec4(Normal, 1.0);
		return;
	}

	if (uDebugDisplay == 2) // Color
	{
		outColor = vec4(Color, 1.0);
		return;
	}

	// The normal is [0,1] in the normal map
	// Rescale to [-1, 1] here
	Normal = normalize((Normal - vec3(0.5)) * vec3(2.0));

	if (uDebugDisplay == 3) // Simple Diffuse
	{
		outColor = vec4(vec3(clamp(dot(Normal, normalize(vec3(1.0, 4.0, 2.0))), 0.0, 1.0)), 1.0);
		return;
	}

	// Simple diffuse directional lights
	vec3 Lighting = vec3(0.0);
	for (int i = 0; i < LIGHT_MAX && i < uDirectionalLightsCount; ++ i)
	{
		vec3 LightDirection = normalize(-uDirectionalLights[i].Direction);
		vec3 LightColor = uDirectionalLights[i].Color;

		Lighting += LightColor * clamp(dot(Normal, LightDirection), 0.0, 1.0);
	}

	// Material params
	const float Ambient = 0.4;
	const float Diffuse = 0.6;
	outColor = vec4(vec3(Ambient + Diffuse * Lighting) * Color, 1.0);
}
