#version 150

in vec2 vPosition;
in vec2 vTexCoords;

in vec3 vInstanceLocation;
in float vInstanceSize;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform float uGlobalScale;

out vec2 fTexCoords;


void main()
{
	vec3 CameraRight = vec3(uViewMatrix[0][0], uViewMatrix[1][0], uViewMatrix[2][0]);
	vec3 CameraUp = vec3(uViewMatrix[0][1], uViewMatrix[1][1], uViewMatrix[2][1]);

	vec3 Position = vInstanceLocation;
	vec2 Size = vec2(vInstanceSize * uGlobalScale);

	vec2 VertexOffset = vPosition;
	VertexOffset *= Size;

	vec3 Vertex =
		CameraRight * VertexOffset.x +
		CameraUp * VertexOffset.y;

	Vertex += Position;

	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(Vertex, 1.0);
	fTexCoords = vTexCoords;
}
