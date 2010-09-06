// volume data and transfer functions
uniform sampler2D back, front, transfer_function_2D;
uniform sampler3D volume, transfer_texture, cluster_texture;

// for raycasting
uniform float stepsize, luminance, clip;
varying vec4 pos; // vertex position, pos = gl_Position;

// for threshold peeling
uniform int peeling_option, transfer_function_option;
uniform float threshold_low, threshold_high;

uniform vec3 sizes; // size of the volume data

// for cluster peeling
uniform int cluster_limit;
uniform float cluster_interval;

int to_cluster_number(float position)
{
	return (int)(position / cluster_interval + 0.5);
}

float multiply(vec4 c)
{
	return c.x * c.y * c.z;
}

float average(vec4 c)
{
	return (c.x + c.y + c.z) * 0.33333333333333333333333333333333;
}

bool isReacheThreshold(vec4 col_acc, vec4 color_sample)
{
	return average(col_acc) > threshold_high && average(color_sample) > threshold_low;
}

vec4 directRendering(vec3 frontPos, vec3 backPos)
{
	vec3 dir = backPos - frontPos;
	float len = length(dir);  // the length from front to back is calculated and used to terminate the ray
	vec3 norm_dir = normalize(dir);
	vec3 delta_dir = norm_dir * stepsize;
	float delta_dir_len = length(delta_dir);
	vec3 ray = frontPos;
	float alpha_acc = 0.;
	float length_acc = 0.;
	vec4 color_sample;
	float alpha_sample;
	vec4 col_acc = vec4(0,0,0,0);
	int count = int(len / stepsize);
	bool threshold_reached = false;

	// calculate difference to sharpen the image
	vec4 d = vec4(vec3(1,1,1)/sizes, 0);
	const vec2 mask = vec2(1.0, 0.0);
	vec4 c, c2, second_derivative;
	float second_derivative_magnitude;

	// for culstering peeling
	int cluster_number = -1, cluster_number_new, cluster_count = 0;
	bool cluster_limit_reached = false;

	for(int i = 0; i < count; i++)
	{
		length_acc += delta_dir_len;
		if(length_acc > clip)
		{
			// transfer function
			if(transfer_function_option == 1)
			{
				c = texture3D(volume, ray);
				color_sample
					= mask.xxxy * texture2D(transfer_function_2D, c.xw)
					+ mask.yyyx * multiply(c);
			}else
			{
				if(transfer_function_option == 2)
				{
					color_sample = texture3D(transfer_texture, ray);
				}
				else
				{
					if(transfer_function_option == 3)
					{
						color_sample
							= mask.xyyy * abs(texture3D(volume, ray+d.xww)-texture3D(volume, ray-d.xww))
							+ mask.yxyy * abs(texture3D(volume, ray+d.wyw)-texture3D(volume, ray-d.wyw))
							+ mask.yyxy * abs(texture3D(volume, ray+d.wwz)-texture3D(volume, ray-d.wwz))
							+ mask.yyyx * multiply(texture3D(volume, ray));
					}else
					{
						if(transfer_function_option == 4)
						{
							c = texture3D(volume, ray);
							c2 = c * 2;
							second_derivative
								= mask.xyyy * abs(texture3D(volume, ray+2*d.xww) - c2 + texture3D(volume, ray-2*d.xww))
								+ mask.yxyy * abs(texture3D(volume, ray+2*d.wyw) - c2 + texture3D(volume, ray-2*d.wyw))
								+ mask.yyxy * abs(texture3D(volume, ray+2*d.wwz) - c2 + texture3D(volume, ray-2*d.wwz));
							color_sample
								= mask.xyyy * abs(texture3D(volume, ray+d.xww)-texture3D(volume, ray-d.xww))
								+ mask.yxyy * abs(texture3D(volume, ray+d.wyw)-texture3D(volume, ray-d.wyw))
								+ mask.yyxy * abs(texture3D(volume, ray+d.wwz)-texture3D(volume, ray-d.wwz))
								+ mask.yyyx * multiply(c) * length(second_derivative);
						}else
						{
							if(transfer_function_option == 5)
							{
								c = texture3D(volume, ray);
								c2 = c * 2;
								second_derivative
									= mask.xyyy * abs(texture3D(volume, ray+2*d.xww) - c2 + texture3D(volume, ray-2*d.xww))
									+ mask.yxyy * abs(texture3D(volume, ray+2*d.wyw) - c2 + texture3D(volume, ray-2*d.wyw))
									+ mask.yyxy * abs(texture3D(volume, ray+2*d.wwz) - c2 + texture3D(volume, ray-2*d.wwz));
								second_derivative_magnitude = max(length(second_derivative), 1e-9);
								color_sample
									= mask.xyyy * abs(texture3D(volume, ray+d.xww)-texture3D(volume, ray-d.xww))
									+ mask.yxyy * abs(texture3D(volume, ray+d.wyw)-texture3D(volume, ray-d.wyw))
									+ mask.yyxy * abs(texture3D(volume, ray+d.wwz)-texture3D(volume, ray-d.wwz))
									+ mask.yyyx * multiply(c) / second_derivative_magnitude;
							}else
							{
								if(transfer_function_option == 6)
								{
									//color_sample = texture3D(cluster_texture, ray);

									c = texture3D(cluster_texture, ray);
									color_sample = texture2D(transfer_function_2D, c.xw);

									//color_sample
									//	= mask.xxxy * texture2D(transfer_function_2D, vec2(average(c), c.a))
									//	+ mask.yyyx * multiply(texture3D(volume, ray));
								}else
								{
									color_sample = texture3D(volume, ray);
								}
							}
						}
					}
				}
			}

			alpha_sample = color_sample.a * stepsize;
			alpha_acc += alpha_sample;
			//col_acc += (1.0 - alpha_acc) * color_sample * stepsize;
			col_acc += (1.0 - alpha_acc) * color_sample * alpha_sample * luminance;
		}

		ray += delta_dir;

		if(peeling_option == 1)
		{
			// threshold
			if(!threshold_reached && isReacheThreshold(col_acc, color_sample))
			{
				col_acc = vec4(0,0,0,0);
				alpha_acc = 0;
				threshold_reached = true;
			}
		}else
		{
			if(peeling_option == 2)
			{
				c = texture3D(cluster_texture, ray);
				cluster_number_new = to_cluster_number(c.x);
				if(cluster_number == -1)
				{
					cluster_number = cluster_number_new;
				}else
				{
					if(cluster_number != cluster_number_new)
					{
						cluster_number = cluster_number_new;
						cluster_count++;
						if(!cluster_limit_reached && cluster_count >= cluster_limit)
						{
							col_acc = vec4(0,0,0,0);
							alpha_acc = 0;
							cluster_limit_reached = true;
						}
					}
				}
			}else
			{
				if(peeling_option == 3)
				{
					c = texture3D(cluster_texture, ray);
					cluster_number_new = to_cluster_number(c.x);
					if(!cluster_limit_reached && cluster_number_new == cluster_limit)
					{
						col_acc = vec4(0,0,0,0);
						alpha_acc = 0;
						cluster_limit_reached = true;
					}
				}
			}
		}

		if(length_acc >= len || alpha_acc > 1.0) break; // terminate if opacity > 1 or the ray is outside the volume
	}

	col_acc.a = alpha_acc;
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
	}
}
