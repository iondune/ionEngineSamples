
#version 330

in vec2 fTexCoords;

uniform sampler2D uTexture;
uniform sampler2D tSceneNormals;
uniform bool uDoBlur;
uniform bool uUnconstrained;
uniform float uNormalThreshold;

out vec4 outColor;


const int numSamples = 5;
const float offset[numSamples] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
const float weight[numSamples] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );


void main()
{
	vec3 normal = texture(tSceneNormals, fTexCoords).rgb * 2.0 - vec3(1.0);
	outColor = vec4(texture(uTexture, fTexCoords).rgb, 1.0) * weight[0];
	float weights = weight[0];

	vec2 textureSize = vec2(textureSize(uTexture, 0));

	if (uDoBlur)
	{
		for (int i = 1; i < numSamples; i ++)
		{
			vec2 off = vec2(0.0, offset[i]) / textureSize;

			if (uUnconstrained || dot(texture(tSceneNormals, fTexCoords + off).rgb * 2.0 - vec3(1.0), normal) > uNormalThreshold)
			{
				outColor.rgb += texture(uTexture, fTexCoords + off).rgb * weight[i];
				weights += weight[i];
			}
			if (uUnconstrained || dot(texture(tSceneNormals, fTexCoords - off).rgb * 2.0 - vec3(1.0), normal) > uNormalThreshold)
			{
				outColor.rgb += texture(uTexture, fTexCoords - off).rgb * weight[i];
				weights += weight[i];
			}
		}
	}

	outColor /= weights;
}
