#version 330

in vec3 vPosition;
in vec3 vNormal;

uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uProjectionViewMatrix;

out vec3 fWorldPosition;
out vec3 fNormal;


void main()
{
	vec4 Position = uModelMatrix * vec4(vPosition, 1.0);

	fWorldPosition = Position.xyz;
	fNormal = vec3(uNormalMatrix * vec4(vNormal, 0.0));

	gl_Position = uProjectionViewMatrix * Position;
}
