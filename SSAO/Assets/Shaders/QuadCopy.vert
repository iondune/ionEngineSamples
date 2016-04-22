
#version 330

in vec3 vPosition;
out vec2 fTexCoords;


void main()
{
	gl_Position = vec4(vPosition, 1.0);
	fTexCoords = vPosition.xy / 2.0 + vec2(0.5);
}
