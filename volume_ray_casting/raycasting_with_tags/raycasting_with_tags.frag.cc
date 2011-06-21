// Data

// back and front faces of the cube
uniform sampler2D back, front;

// 2D transfer function
uniform sampler2D transfer_function_2D;

// volume data and transfer functions
uniform sampler3D volume_texture, transfer_texture;

// stepsize and luminance of the raycasting process
uniform float stepsize, luminance;

// for choosing a transfer function
uniform int transfer_function_option;

// the current position of the ray
varying vec4 pos;

float sum3(vec4 c)
{
	return c.x + c.y + c.z;
}

// the direct volume rendering process
vec4 directRendering(vec3 frontPos, vec3 backPos)
{
	// get the direction vector from the back and front positions
	vec3 dir = backPos - frontPos;

	// the length from front to back is calculated and used to terminate the ray
	float len = length(dir);

	// delta_dir is the increment of the ray position
	vec3 delta_dir = normalize(dir) * stepsize;

	// the length of delta
	float delta_dir_len = length(delta_dir);

	// the front position
	vec3 ray = frontPos;

	// accumulated alpha value
	float alpha_acc = 0.;

	// accumulated length
	float length_acc = 0.;

	// color sample 
	vec4 color_sample;

	// initial accumulated color
	vec4 col_acc = vec4(0,0,0,0);

	// number of iterations
	int count = int(len / stepsize);

	// a mask for vector operations
	const vec4 mask = vec4(1, 0, 0, 0);

	// the loop for ray casting
	for(int i = 0; i < count; i++)
	{
		//// get the current color sample from the transfer function 
		//color_sample = texture3D(volume_texture, vec);

		// transfer functions
		switch(transfer_function_option)
		{
		case 0:
			// Raw scalar values without a transfer function
			color_sample = texture3D(volume_texture, ray);
			break;

		case 1:
			// Simple 2D transfer function
			color_sample = texture3D(volume_texture, ray);
			color_sample = mask.xxxw * texture2D(transfer_function_2D, color_sample.xy) + mask.wwwx * sum3(color_sample);
			break;

		case 2:
			// Ben transfer function
			color_sample = texture3D(transfer_texture, ray);
			break;

		default:
			// Raw scalar values without a transfer function
			color_sample = texture3D(volume_texture, ray);
		}

		// calculate the alpha sample by color sample and stepsize
		color_sample.a = color_sample.a * stepsize;

		// calculate the accumulated color
		col_acc.rgb = mix(col_acc.rgb, color_sample.rgb, color_sample.a);

		// calculate the accumulated alpha value
		col_acc.a = mix(color_sample.a, 1.0, col_acc.a);

		// the ray position vector
		ray += delta_dir;

		// increase the length
		length_acc += delta_dir_len;

		// terminate if opacity > 1 or the ray is outside the volume
		if(length_acc >= len || alpha_acc > 1.0)
			break;
	}

	// set the luminance of the ray
	col_acc.rgb *= luminance;

	return col_acc;
}

void main(void)
{
	// find the right place to lookup in the backside buffer
	vec2 tex_coord = ((pos.xy / pos.w) + 1.) / 2.;

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
	}
}
