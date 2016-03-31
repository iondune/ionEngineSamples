#version 150

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;

uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform int uPointLightsCount;
uniform vec3 uCameraPosition;

out vec3 fNormal;
out vec2 fTexCoords;


void main()
{
	fNormal = (uNormalMatrix * vec4(vNormal, 1.0)).xyz;
	fTexCoords = vTexCoords;

	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(vPosition, 1.0);
}
