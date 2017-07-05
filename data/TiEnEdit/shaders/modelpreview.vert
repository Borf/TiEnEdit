#version 150

in vec3 a_position;
in vec2 a_texcoord;

out vec2 texcoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec4 color;

void main()
{
	texcoord = a_texcoord;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position,1.0);
}