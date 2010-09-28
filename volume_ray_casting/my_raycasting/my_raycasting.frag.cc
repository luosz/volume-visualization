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
uniform int peeling_layer;

int to_cluster_number(float position)
{
	return int(position / cluster_interval + 0.5);
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

//bool multi_sample_boundary_detection(vec3 v1, vec3 position, vec3 delta_dir)
//{
//	vec3 v2 = vec3(0,0,0);
//	const int size = 3;
//	int count = 0;
//	// how many non-zero components there are
//	for (int i=0; i<size; i++)
//	{
//		if (v1[i] != 0)
//		{
//			count++;
//		}
//	}
//
//	int index = -1;
//	// get a vector v2 that is vertical to v1
//	switch(count)
//	{
//	case 0: return false;
//	case 1:
//		for (int i=0; i<size; i++)
//		{
//			if (v1[i] != 0)
//			{
//				v2[(i+1) % size] = v1[i];
//				break;
//			}
//		}
//		break;
//	case 2:
//		for (int i=0; i<size; i++)
//		{
//			if (v1[i] != 0)
//			{
//				if (index == -1)
//				{
//					index = i;
//				}else
//				{
//					v2[i] = -v1[index];
//					v2[index] = v1[i];
//					break;
//				}
//			}
//		}
//		break;
//	default:
//		v2.x = v2.y = v1.z;
//		v2.z = - v1.x - v1.y;
//	}
//
//	v2 = normalize(v2);
//	vec3 v3 = normalize(cross(v1, v2));
//	vec3 delta2 = v2 * stepsize;
//	vec3 delta3 = v3 * stepsize;
//	vec3 p1 = position + v2 * stepsize;
//	vec3 p2 = position - v2 * stepsize;
//	vec3 p3 = position + v3 * stepsize;
//	vec3 p4 = position - v3 * stepsize;
//
//	count = 0;
//	if(1 <= abs(to_cluster_number(texture3D(cluster_texture, position + delta_dir).x)
//		- to_cluster_number(texture3D(cluster_texture, position - delta_dir).x)))
//		count++;
//	if(1 <= abs(to_cluster_number(texture3D(cluster_texture, p1 + delta_dir).x)
//		- to_cluster_number(texture3D(cluster_texture, p1 - delta_dir).x)))
//		count++;
//	if(1 <= abs(to_cluster_number(texture3D(cluster_texture, p2 + delta_dir).x)
//		- to_cluster_number(texture3D(cluster_texture, p2 - delta_dir).x)))
//		count++;
//	if(1 <= abs(to_cluster_number(texture3D(cluster_texture, p3 + delta_dir).x)
//		- to_cluster_number(texture3D(cluster_texture, p3 - delta_dir).x)))
//		count++;
//	if(1 <= abs(to_cluster_number(texture3D(cluster_texture, p4 + delta_dir).x)
//		- to_cluster_number(texture3D(cluster_texture, p4 - delta_dir).x)))
//		count++;
//	return count > 2;
//}

bool detect_boundary_multisample_5(vec3 v1, vec3 position, vec3 delta_v1)
{
	const float epsilon = 1e-5;
	vec3 v2 = vec3(0,0,0);
	const int size = 3;
	int count = 0, index1 = -1, index2 = -1;

	// how many non-zero components there are
	for (int i=0; i<size; i++)
	{
		if (abs(v1[i]) > epsilon)
		{
			count++;
			if (index1 == -1)
			{
				index1 = i;
			}else
			{
				index2 = i;
			}
		}
	}

	// get a vector v2 that is vertical to v1
	switch(count)
	{
	case 0: return false;
	case 1:
		index2 = index1 + 1;
		index2 = (index2 >= size) ? (index2 - size) : index2;
		v2[index2] = v1[index1];
		break;
	case 2:
		v2[index2] = -v1[index1];
		v2[index1] = v1[index2];
		break;
	default:
		v2.x = v2.y = v1.z;
		v2.z = - v1.x - v1.y;
	}

	// get a vector v3 that is vertical to both v1 and v2, and then normalize v2 and v3
	// get four positions that are adjacent to the original position
	const float one = 1 - epsilon;
	v2 = normalize(v2);
	vec3 v3 = normalize(cross(v1, v2));
	vec3 delta_v2 = v2 * stepsize;
	vec3 delta_v3 = v3 * stepsize;
	vec3 p1 = position + delta_v2;
	vec3 p2 = position - delta_v2;
	vec3 p3 = position + delta_v3;
	vec3 p4 = position - delta_v3;

	// how many pairs belong to different clusters
	count = 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, position + delta_v1).x) 
		- to_cluster_number(texture3D(cluster_texture, position - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p1 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p1- delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p2 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p2 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p3 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p3 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p4 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p4 - delta_v1).x)))) ? 1 : 0;

	// It is a boundary if more than a half pairs belong to different clusters
	return count >= 3;
}

bool detect_boundary_multisample_9(vec3 v1, vec3 position, vec3 delta_v1)
{
	const float epsilon = 1e-5;
	vec3 v2 = vec3(0,0,0);
	const int size = 3;
	int count = 0, index1 = -1, index2 = -1;

	// how many non-zero components there are
	for (int i=0; i<size; i++)
	{
		if (abs(v1[i]) > epsilon)
		{
			count++;
			if (index1 == -1)
			{
				index1 = i;
			}else
			{
				index2 = i;
			}
		}
	}

	// get a vector v2 that is vertical to v1
	switch(count)
	{
	case 0: return false;
	case 1:
		index2 = index1 + 1;
		index2 = (index2 >= size) ? (index2 - size) : index2;
		v2[index2] = v1[index1];
		break;
	case 2:
		v2[index2] = -v1[index1];
		v2[index1] = v1[index2];
		break;
	default:
		v2.x = v2.y = v1.z;
		v2.z = - v1.x - v1.y;
	}

	// get a vector v3 that is vertical to both v1 and v2, and then normalize v2 and v3
	// get four positions that are adjacent to the original position
	const float one = 1 - epsilon;
	v2 = normalize(v2);
	vec3 v3 = normalize(cross(v1, v2));
	vec3 delta_v2 = v2 * stepsize;
	vec3 delta_v3 = v3 * stepsize;
	vec3 p1 = position + delta_v2;
	vec3 p2 = position - delta_v2;
	vec3 p3 = position + delta_v3;
	vec3 p4 = position - delta_v3;
	vec3 p5 = position + delta_v2 + delta_v3;
	vec3 p6 = position - delta_v2 - delta_v3;
	vec3 p7 = position + delta_v2 - delta_v3;
	vec3 p8 = position - delta_v2 + delta_v3;

	// how many pairs belong to different clusters
	count = 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, position + delta_v1).x) 
		- to_cluster_number(texture3D(cluster_texture, position - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p1 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p1- delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p2 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p2 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p3 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p3 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p4 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p4 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p5 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p5 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p6 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p6 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p7 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p7 - delta_v1).x)))) ? 1 : 0;
	count += (one < abs(float(to_cluster_number(texture3D(cluster_texture, p8 + delta_v1).x)
		- to_cluster_number(texture3D(cluster_texture, p8 - delta_v1).x)))) ? 1 : 0;

	// It is a boundary if more than a half pairs belong to different clusters
	return count >= 5;
}

vec4 directRendering(vec3 frontPos, vec3 backPos)
{
	vec3 dir = backPos - frontPos;
	float len = length(dir);  // the length from front to back is calculated and used to terminate the ray
	vec3 norm_dir = normalize(dir);
	vec3 delta_dir = norm_dir * stepsize;
	float delta_dir_len = length(delta_dir);
	vec3 ray = frontPos;
	//float alpha_acc = 0.;
	float length_acc = 0.;
	vec4 color_sample;
	//float alpha_sample;

	// black or white background
	vec4 col_acc = vec4(0,0,0,0);
	//vec4 col_acc = vec4(1,1,1,1);

	int sample_number = int(len / stepsize);
	//bool threshold_reached = false;

	// calculate difference to sharpen the image
	vec4 d = vec4(vec3(1,1,1)/sizes, 0);
	const vec2 mask = vec2(1.0, 0.0);
	vec4 c, c2, second_derivative;
	float second_derivative_magnitude;

	// for culstering peeling
	//int cluster_number = -1, cluster_number_new, cluster_count = 0;
	//bool cluster_limit_reached = false;
	int peeling_counter = 0;

	for(int i = 0; i < sample_number; i++)
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
							c2 = c * 2.0;
							second_derivative
								= mask.xyyy * abs(texture3D(volume, ray+2.0*d.xww) - c2 + texture3D(volume, ray-2.0*d.xww))
								+ mask.yxyy * abs(texture3D(volume, ray+2.0*d.wyw) - c2 + texture3D(volume, ray-2.0*d.wyw))
								+ mask.yyxy * abs(texture3D(volume, ray+2.0*d.wwz) - c2 + texture3D(volume, ray-2.0*d.wwz));
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
								c2 = c * 2.0;
								second_derivative
									= mask.xyyy * abs(texture3D(volume, ray+2.0*d.xww) - c2 + texture3D(volume, ray-2.0*d.xww))
									+ mask.yxyy * abs(texture3D(volume, ray+2.0*d.wyw) - c2 + texture3D(volume, ray-2.0*d.wyw))
									+ mask.yyxy * abs(texture3D(volume, ray+2.0*d.wwz) - c2 + texture3D(volume, ray-2.0*d.wwz));
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

			//////////////////////////////////////////////////////////////////////////
			// version 1
			//alpha_sample = color_sample.a * stepsize;
			//alpha_acc += alpha_sample;
			////col_acc += (1.0 - alpha_acc) * color_sample * stepsize;

			// black or white background
			//col_acc += (1.0 - alpha_acc) * color_sample * alpha_sample * luminance;
			////col_acc -= (1.0 - alpha_acc) * color_sample * alpha_sample * luminance;
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			// version 2, behave the same as version 1, but uses fewer variables
			//color_sample.a = color_sample.a * stepsize;
			//col_acc.a += color_sample.a;
			//col_acc.rgb += (1.0 - col_acc.a) * color_sample.rgb * color_sample.a * luminance;
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			// version 3, uses mix() in GLSL to interpolate the colors
			// Accumulate RGB : acc.rgb = voxelColor.rgb*voxelColor.a + (1.0 - voxelColor.a)*acc.rgb;
			//acc.rgb = mix(acc.rgb, voxelColor.rgb, voxelColor.a)*LightIntensity;
			// Accumulate Opacity: acc.a = acc.a + (1.0 - acc.a)*voxelColor.a;
			//acc.a = mix(voxelColor.a, 1.0, acc.a);

			color_sample.a = color_sample.a * stepsize;
			col_acc.rgb = mix(col_acc.rgb, color_sample.rgb, color_sample.a);
			col_acc.a = mix(color_sample.a, 1.0, col_acc.a);
			//////////////////////////////////////////////////////////////////////////
		}

		ray += delta_dir;

		if(peeling_option == 1)
		{
			// opacity peeling
			//if(!threshold_reached && isReacheThreshold(col_acc, color_sample))
				//threshold_reached = true;
			if(average(col_acc) > threshold_high && average(color_sample) < threshold_low)
			{
				if (peeling_counter == peeling_layer)
				{
					break;
				}else
				{
					col_acc = vec4(0,0,0,0);
					//alpha_acc = 0;
					peeling_counter++;
				}
			}
		}else
		{
			if(peeling_option == 2)
			{
				// feature peeling
				//c = texture3D(cluster_texture, ray);
				//cluster_number_new = to_cluster_number(c.x);
				//if(cluster_number == -1)
				//{
				//	cluster_number = cluster_number_new;
				//}else
				//{
				//	if(cluster_number != cluster_number_new)
				//	{
				//		cluster_number = cluster_number_new;
				//		cluster_count++;
				//		if(!cluster_limit_reached && cluster_count >= cluster_limit)
				//		{
				//			col_acc = vec4(0,0,0,0);
				//			alpha_acc = 0;
				//			cluster_limit_reached = true;
				//		}
				//	}
				//}
			}else
			{
				if(peeling_option == 3)
				{
					// peel the back
					//if(0.5 < abs(to_cluster_number(texture3D(cluster_texture, ray + delta_dir).x)
					//	- to_cluster_number(texture3D(cluster_texture, ray - delta_dir).x)))
					if(detect_boundary_multisample_9(norm_dir, ray, delta_dir))
					{
						if (peeling_counter < peeling_layer)
						{
							peeling_counter++;
						}else
						{
							break;
						}
					}
				}else
				{
					if(peeling_option == 4)
					{
						// peel the front
						// classification peeling, peel cluster layers
						//if(0.5 < abs(to_cluster_number(texture3D(cluster_texture, ray + delta_dir).x)
						//	- to_cluster_number(texture3D(cluster_texture, ray - delta_dir).x)))
						if(detect_boundary_multisample_9(norm_dir, ray, delta_dir))
						{
							if (peeling_counter < peeling_layer)
							{
								col_acc = vec4(0,0,0,0);
								//alpha_acc = 0;
								peeling_counter++;
							}
						}
					}
				}
			}
		}

		if(length_acc >= len || col_acc.a >= 1.0) break; // terminate if opacity > 1 or the ray is outside the volume
	}

	//col_acc.a = alpha_acc;
	col_acc.rgb *= luminance;
	//for (int i=0; i<3; i++)
	//{
	//	col_acc[i] *= luminance;
	//}
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
