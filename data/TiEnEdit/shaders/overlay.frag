uniform sampler2D s_texture;
uniform vec4 colorMult;
varying vec2 texCoord;


void main()
{
	gl_FragColor = colorMult * texture2D(s_texture, texCoord);
}
