#version 150

in vec3 vPosition;
in vec3 vNormal;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec3 uCameraPosition;

out vec3 fEye;
out vec3 fNormal;
out vec3 fWorldPosition;


void main()
{
	vec4 Position = uModelMatrix * vec4(vPosition, 1.0);

	gl_Position = uProjectionMatrix * uViewMatrix * Position;
	fEye = normalize(uCameraPosition - Position.xyz);
	fNormal = vNormal;
	fWorldPosition = Position.xyz;
}
