// Data
uniform sampler2D back, front;
uniform sampler3D volume_texture, transfer_texture, volume2;
uniform float stepsize, luminance;
varying vec4 pos;

vec4 directRendering(vec3 frontPos, vec3 backPos)
{
	vec3 dir = backPos - frontPos;
	float len = length(dir);  // the length from front to back is calculated and used to terminate the ray
	vec3 norm_dir = normalize(dir);
	vec3 delta_dir = norm_dir * stepsize;
	float delta_dir_len = length(delta_dir);
	vec3 vec = frontPos;
	float alpha_acc = 0.;
	//float alpha_acc = 1.;
	float length_acc = 0.;
	vec4 color_sample;
	float alpha_sample;
////	vec4 col_acc = vec4(0,0,0,0);
	vec4 col_acc = vec4(1,1,1,1);
	int count = int(len / stepsize);
	for(int i = 0; i < count; i++)
	{
		color_sample = texture3D(transfer_texture, vec);
		alpha_sample = color_sample.a * stepsize;
		//col_acc += (1.0 - alpha_acc) * color_sample * stepsize;
		//col_acc   += (1.0 - alpha_acc) * color_sample * alpha_sample  * 15;
		col_acc   -= (1.0 - alpha_acc) * color_sample * alpha_sample  * 15;
		alpha_acc += alpha_sample;
		//alpha_acc -= alpha_sample;
		vec += delta_dir;
		length_acc += delta_dir_len;
		if(length_acc >= len || alpha_acc > 1.0) break; // terminate if opacity > 1 or the ray is outside the volume
	}
	return col_acc;
}

void main(void)
{
	vec2 tex_coord = ((pos.xy / pos.w) + 1.) / 2.; // find the right place to lookup in the backside buffer
	vec4 start = gl_TexCoord[1]; // the start position of the ray is stored in the texturecoordinate
	vec4 back_position  = texture2D(back, tex_coord);
	vec3 backPos = back_position.xyz;
	vec3 frontPos = start.xyz;

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
