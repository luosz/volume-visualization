// Data

// back and front faces of the cube
uniform sampler2D back, front;

// volume data and transfer functions
uniform sampler3D volume_texture, transfer_texture, volume2;

// stepsize and luminance of the raycasting process
uniform float stepsize, luminance;

// the current position of the ray
varying vec4 position;

// the direct volume rendering process
vec4 directRendering(vec3 frontPos, vec3 backPos)
{
	// get the direction vector from the back and front positions
	vec3 dir = backPos - frontPos;

	// the length from front to back is calculated and used to terminate the ray
	float len = length(dir);

	// normalize the direction vector
	vec3 norm_dir = normalize(dir);

	// delta is the increment of the ray position
	vec3 delta_dir = norm_dir * stepsize;

	// the length of delta
	float delta_dir_len = length(delta_dir);

	// the front position
	vec3 ray = frontPos;

	// accumulated alpha value
	float alpha_acc = 0.;
//	float alpha_acc = 1.0;

	// accumulated length
	float length_acc = 0.;

	// color sample 
	vec4 color_sample;

	// alpha sample
	float alpha_sample;

	// initial accumulated color
	vec4 col_acc = vec4(0,0,0,0);
//	vec4 col_acc = vec4(1,1,1,1);

	// number of iterations
	int count = int(len / stepsize);

	// the loop for ray casting
	for(int i = 0; i < count; i++)
	{
		// get the current color sample from the transfer function 
		color_sample = texture3D(transfer_texture, ray);

		// calculate the alpha sample by color sample and stepsize
		alpha_sample = color_sample.a * stepsize;

		// calculate the accumulated color by stepsize
		col_acc += (1.0 - alpha_acc) * color_sample * stepsize;

		// calculate the accumulated color by alpha sample
		col_acc   += (1.0 - alpha_acc) * color_sample * alpha_sample * 10.0;
	//	col_acc   -= (1.0 - alpha_acc) * color_sample * alpha_sample ;

		// calculate the accumulated alpha value
		alpha_acc += alpha_sample;
	//	alpha_acc -= alpha_sample;

		// the ray position vector
		ray += delta_dir;

		// increase the length
		length_acc += delta_dir_len;

		// terminate if opacity > 1 or the ray is outside the volume
		if(length_acc >= len || alpha_acc > 1.0)
			break;
	}
	return col_acc;
}

void main(void)
{
	// find the right place to lookup in the backside buffer
	vec2 tex_coord = ((position.xy / position.w) + 1.) / 2.;

	// the start position of the ray is stored in the texturecoordinate
	vec4 start = gl_TexCoord[1];

	// get the back position from the texture coordinate
	vec4 back_position  = texture2D(back, tex_coord);

	// get the back vector from back position
	vec3 backPos = back_position.xyz;

	// get the front vector from start position
	vec3 frontPos = start.xyz;

	// calculate the direction vector
	vec4 dir = back_position - start;

	//determine whether the ray has to be casted
	if (frontPos == backPos) {
		//background need no raycasting
		discard;
	} else {
		//fragCoords are lying inside the boundingbox
		gl_FragColor = directRendering(frontPos, backPos);

		//vec3 dir = backPos - frontPos;
		//vec3 inc = normalize(dir) * stepsize;

		//gl_FragColor = gl_TexCoord[1];
		//gl_FragColor.r += stepsize * 10;
		//gl_FragColor.g += stepsize * 10;
		//gl_FragColor.b += stepsize * 10;
		//if(length(FragColor) < 1e-6)
		//	FragColor = vec4(1,1,1,1);
	}
}
