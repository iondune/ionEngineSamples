
#version 330

in vec3 vPosition;
in vec3 vNormal;

uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 fNormal;
out vec3 fViewPosition;


void main()
{
	vec4 Position = uViewMatrix * uModelMatrix * vec4(vPosition, 1.0);
	fViewPosition = Position.xyz;

	fNormal = normalize(transpose(inverse(mat3(uViewMatrix * uModelMatrix))) * vNormal);

	gl_Position = uProjectionMatrix * Position;
}
