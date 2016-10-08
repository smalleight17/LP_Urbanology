#version 150

uniform mat4 ciModelViewProjection;

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec2 TexCoord;

void main()
{
	gl_Position = ciModelViewProjection * ciPosition;
	TexCoord = ciTexCoord0;
}
