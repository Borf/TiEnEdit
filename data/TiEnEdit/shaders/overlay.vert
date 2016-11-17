attribute vec2 a_position;
attribute vec2 a_texture;
varying vec2 texCoord;
uniform mat4 matrix;
uniform mat4 projectionMatrix;

void main()
{
	texCoord = a_texture;
	gl_Position = projectionMatrix * matrix * vec4(a_position,0.0,1.0);
}