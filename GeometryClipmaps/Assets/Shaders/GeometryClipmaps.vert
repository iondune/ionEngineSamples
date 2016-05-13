
#version 150

in ivec2 vPosition;

uniform vec3 uTranslation;
uniform vec3 uScale;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

uniform sampler2D uHeightMap;

uniform ivec2 uDataOffset;

out vec2 fTexCoords;
out vec3 fWorldPosition;

const float Pi = 3.14159;

vec3 LatLongToCart(vec3 LongLatElev)
{
	float lat_rad = LongLatElev.y / 1000.0 * (Pi / 180.0f);
	float lng_rad = LongLatElev.x / 1000.0 * (Pi / 180.0f);

	float h = 0.0f;
	float a = 6378137;
	float b = 6356752.31425;

	float sin_lat = sin(lat_rad);
	float cos_lat = cos(lat_rad);
	float sin_lng = sin(lng_rad);
	float cos_lng = cos(lng_rad);

	if (LongLatElev.z > -1)
	{
		h = LongLatElev.z;
	}

	float eSq = (a * a - b * b) / (a * a);
	float v = a / sqrt(1 - eSq * sin_lat * sin_lat);

	float x = (v + h) * cos_lat * cos_lng * 1.06932e-06;
	float y = (v + h) * cos_lat * sin_lng * 1.06932e-06;
	float z = ((1 - eSq) * v + h) * sin_lat * 1.06932e-06;

	return vec3(x, y, z);
}

void main()
{
	ivec2 TexCoords = (vPosition + uDataOffset);
	ivec2 TextureSize = textureSize(uHeightMap, 0);
	fTexCoords = vec2(TexCoords) / vec2(TextureSize);

	float Height = texelFetch(uHeightMap, TexCoords % TextureSize, 0).r;
	vec4 WorldPosition = uModelMatrix * vec4(LatLongToCart((vec3(vPosition.x, Height, vPosition.y) * uScale + uTranslation).xzy), 1.0);
	fWorldPosition = WorldPosition.xyz;

	gl_Position = uProjectionMatrix * uViewMatrix * WorldPosition;
}
