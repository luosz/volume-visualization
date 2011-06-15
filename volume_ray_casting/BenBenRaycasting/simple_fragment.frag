/* pass-through fragment shader */
varying vec4 pos;

void main()
{
	//gl_FragColor = gl_Color;
	gl_FragColor = pos;
}
