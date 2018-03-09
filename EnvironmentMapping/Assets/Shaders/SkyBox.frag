#version 330

in vec3 fTexCoord;

uniform sampler2D uTexture;

out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec3 sample = normalize(fTexCoord);
	vec2 uv = SampleSphericalMap(sample);
	outColor = texture(uTexture, uv);

	// outColor.rgb = sample * 0.5 + vec3(0.5);
	// outColor.rgb = vec3(uv, 0.0);
	// outColor = texture(uTexture, fTexCoord);
}
