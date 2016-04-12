#version 150

#define LIGHT_MAX 3

struct SLight
{
	vec3 Direction;
	vec3 Color;
};

in vec2 fTexCoords;

uniform sampler2D uNormalMap;
uniform sampler2D uColorMap;
uniform sampler2D uHeightMap;

uniform float uHeightmapResolution;
uniform float uTexelSize;
uniform int uScaleFactor;

uniform int uSamplingMode;
uniform int uDebugDisplay;
uniform int uDiffuseOnly;

uniform SLight uDirectionalLights[LIGHT_MAX];
uniform int uDirectionalLightsCount;

out vec4 outColor;


vec4 filterIQ(sampler2D iChannel0, vec2 uv)
{
	uv = uv*uHeightmapResolution + 0.5;
	vec2 iuv = floor( uv );
	vec2 fuv = fract( uv );
	uv = iuv + fuv*fuv*(3.0-2.0*fuv); // fuv*fuv*fuv*(fuv*(fuv*6.0-15.0)+10.0);;
	uv = (uv - 0.5)/uHeightmapResolution;
	return texture( iChannel0, uv );
}

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

float getHeightAt(vec2 Offset)
{
	vec2 texSize = textureSize(uHeightMap, 0);
	vec2 invTexSize = 1.0 / texSize;

	return textureBicubic(uHeightMap, fTexCoords + Offset * invTexSize).r;
}


float getOcclusion(float Offset)
{
	float occlusion = 0;
	float here = getHeightAt(vec2(0, 0));
	float step;

	// Offset /= float(uScaleFactor);

	step = getHeightAt(vec2(Offset, 0));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(-Offset, 0));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(0, Offset));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(0, -Offset));
	if (step > here)
		occlusion += step - here;

	Offset *= 2;
	step = getHeightAt(vec2(Offset, 0));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(-Offset, 0));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(0, Offset));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(0, -Offset));
	if (step > here)
		occlusion += step - here;

	Offset *= 2;
	step = getHeightAt(vec2(Offset, 0));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(-Offset, 0));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(0, Offset));
	if (step > here)
		occlusion += step - here;
	step = getHeightAt(vec2(0, -Offset));
	if (step > here)
		occlusion += step - here;
	return occlusion / float(uScaleFactor);
}

vec4 texelWrap(sampler2D i, ivec2 x)
{
	ivec2 s = textureSize(i, 0);
	return texelFetch(i, x % s, 0);
}

void main()
{
	if (uDebugDisplay == 4) // Texture Coordinates
	{
		outColor = vec4(fTexCoords, vec2(1.0));
		return;
	}

	vec3 Normal = vec3(0.0);
	vec3 Color = vec3(0.0);

	if (uSamplingMode == 0) // Bilinear (hardware)
	{
		Normal = texture(uNormalMap, fTexCoords).rgb;
		Color = texture(uColorMap, fTexCoords).rgb;
	}
	else if (uSamplingMode == 1) // IQ's filter technique
	{
		Normal = filterIQ(uNormalMap, fTexCoords).rgb;
		Color = filterIQ(uColorMap, fTexCoords).rgb;
	}
	else if (uSamplingMode == 2) // Bicubic (software)
	{
		Normal = textureBicubic(uNormalMap, fTexCoords).rgb;
		Color = textureBicubic(uColorMap, fTexCoords).rgb;
	}
	else if (uSamplingMode == 3) // Nearest Neighbor
	{
		Normal = texelWrap(uNormalMap, ivec2(floor(fTexCoords * uHeightmapResolution + vec2(0.5)))).rgb;
		Color = texelWrap(uColorMap, ivec2(floor(fTexCoords * uHeightmapResolution + vec2(0.5)))).rgb;
	}

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

	Normal = normalize((Normal - vec3(0.5)) * vec3(2.0));

	if (uDebugDisplay == 3) // Simple Diffuse
	{
		outColor = vec4(vec3(clamp(dot(Normal, normalize(vec3(1.0, 4.0, 2.0))), 0.0, 1.0)), 1.0);
		return;
	}

	vec3 Lighting = vec3(0.0);
	for (int i = 0; i < LIGHT_MAX && i < uDirectionalLightsCount; ++ i)
	{
		vec3 LightDirection = normalize(-uDirectionalLights[i].Direction);
		vec3 LightColor = uDirectionalLights[i].Color;

		Lighting += LightColor * clamp(dot(Normal, LightDirection), 0.0, 1.0);
	}

	const float Ambient = 0.4;
	const float Diffuse = 0.6;
	outColor = vec4(vec3(Ambient + Diffuse * Lighting) * Color, 1.0);
	// outColor = vec4(vec3(Ambient + Diffuse * Lighting) * Color * vec3(1.0 - getOcclusion(0.2) * 0.1), 1.0);
	// outColor = vec4(vec3(1.0 - getOcclusion(0.2) * 0.1), 1.0);

	float Distance = gl_FragCoord.z / gl_FragCoord.w;
	const float Far = 15000.0;
	float FogFactor = clamp(Distance / Far, 0.0, 0.9);

	vec3 FogColor = vec3(188.0/255.0, 251/255.0, 1.0);
	outColor = vec4(mix(outColor.rgb, FogColor, FogFactor), 1.0);
}
