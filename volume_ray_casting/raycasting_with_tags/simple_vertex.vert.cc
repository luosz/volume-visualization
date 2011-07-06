/**
 * Simply pass through the provided vertex position.
 */
// for raycasting
varying vec4 position;

// for lighting
uniform vec3 fvLightPosition;
uniform vec3 fvEyePosition;

varying vec3 ViewDirection;
varying vec3 LightDirection;
varying vec3 Normal;

void main() {
    gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = ftransform();
	position = gl_Position;

	// for lighting
	vec4 fvObjectPosition = gl_ModelViewMatrix * gl_Vertex;

	ViewDirection  = fvEyePosition - fvObjectPosition.xyz;
	LightDirection = fvLightPosition - fvObjectPosition.xyz;
	Normal         = gl_NormalMatrix * gl_Normal;
}
