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


void main()
{
	ivec2 TexCoords = (vPosition + uDataOffset);
	ivec2 TextureSize = textureSize(uHeightMap, 0);
	fTexCoords = vec2(TexCoords) / vec2(TextureSize);

	float Height = texelFetch(uHeightMap, TexCoords % TextureSize, 0).r;
	vec4 WorldPosition = uModelMatrix * vec4(vec3(vPosition.x, Height, vPosition.y) * uScale + uTranslation, 1.0);

	gl_Position = uProjectionMatrix * uViewMatrix * WorldPosition;
}
