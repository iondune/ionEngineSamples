
#version 150

in vec3 vPosition;
in vec3 vNormal;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uLightMatrix;

uniform vec3 uCameraPosition;

out vec3 fNormal;
out vec3 fWorldPosition;
out vec4 fLightSpacePosition;


void main()
{
	vec4 Position = uModelMatrix * vec4(vPosition, 1.0);

	fWorldPosition = Position.xyz;
	fLightSpacePosition = uLightMatrix * Position;
	fNormal = vNormal;

	gl_Position = uProjectionMatrix * uViewMatrix * Position;
}
