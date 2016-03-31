#version 150

in vec3 vPos;
in vec3 vNor;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec3 uCameraPosition;

out vec3 fEye;
out vec3 fNormal;
out vec3 fWorldPosition;


void main()
{
	vec4 Position = uModelMatrix * vec4(vPos, 1.0);

	gl_Position = uProjectionMatrix * uViewMatrix * Position;
	fEye = normalize(uCameraPosition - Position.xyz);
	fNormal = vNor;
	fWorldPosition = Position.xyz;
}
