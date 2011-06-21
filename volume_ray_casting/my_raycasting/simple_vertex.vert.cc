/**
 * Simply pass through the provided vertex position.
 */
varying vec4 position;

void main() {
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	position = gl_Position;
}
