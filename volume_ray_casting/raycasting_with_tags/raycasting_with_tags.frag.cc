// Data

// back and front faces of the cube
uniform sampler2D back, front;

// 2D transfer function
uniform sampler2D transfer_function_2D;

// volume data and transfer functions
uniform sampler3D volume_texture, transfer_texture, gradient_texture;

// volume data for segmentation tags
uniform sampler3D tag_texture;

// stepsize and luminance of the raycasting process
uniform float stepsize, luminance;

// for choosing a transfer function
uniform int transfer_function_option;

// enable or disable lighting
uniform int lighting_option;

// size of the volume data
uniform vec3 sizes;

// the current position of the ray
varying vec4 position;

// for lighting
uniform vec4 fvAmbient;
uniform vec4 fvSpecular;
uniform vec4 fvDiffuse;
uniform float fSpecularPower;

//varying vec3 ViewDirection;
//varying vec3 LightDirection;
//varying vec3 Normal;

uniform vec3 fvLightPosition;
uniform vec3 fvEyePosition;

float sum3(vec4 c)
{
	return c.x + c.y + c.z;
}

float sum4(vec4 c)
{
	return c.x + c.y + c.z + c.w;
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

	// alpha sample
	float alpha_sample;

	// initial accumulated color
	vec4 col_acc = vec4(0,0,0,0);

	// number of iterations
	int count = int(len / stepsize);

	// a mask for vector operations
	const vec4 mask = vec4(1, 0, 0, 0);

	// offset for sobel operator
	vec4 e = vec4(vec3(-1, 1, 0)/sizes, 0);

	// for Sobel operator
	vec4 g_x, g_y, g_z;

	// the loop for ray casting
	for(int i = 0; i < count; i++)
	{
		// get the current color sample from the transfer function 
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

		case 3:
			// Segmentation tags with simple 2D transfer function
			vec4 tag = texture3D(tag_texture, ray);
			if (tag.x > 1e-4)
			{
				color_sample = mask.xxxw * texture2D(transfer_function_2D, tag.xy) + mask.wwwx * sum3(texture3D(volume_texture, ray));
			}
			else
			{
				color_sample = texture3D(volume_texture, ray);
			}
			break;

		case 4:
			// Sobel 3D operator
			g_x = abs(
				4.0 * (texture3D(volume_texture, ray+e.yww)-texture3D(volume_texture, ray+e.xww))
				+ 2.0 * (texture3D(volume_texture, ray+e.yxw)-texture3D(volume_texture, ray+e.xxw))
				+ 2.0 * (texture3D(volume_texture, ray+e.yyw)-texture3D(volume_texture, ray+e.xyw)) 
				+ 2.0 * (texture3D(volume_texture, ray+e.ywx)-texture3D(volume_texture, ray+e.xwx))
				+ 2.0 * (texture3D(volume_texture, ray+e.ywy)-texture3D(volume_texture, ray+e.xwy))
				+ (texture3D(volume_texture, ray+e.yxx)-texture3D(volume_texture, ray+e.xxx))
				+ (texture3D(volume_texture, ray+e.yyy)-texture3D(volume_texture, ray+e.xyy)) 
				+ (texture3D(volume_texture, ray+e.yyx)-texture3D(volume_texture, ray+e.xyx))
				+ (texture3D(volume_texture, ray+e.yxy)-texture3D(volume_texture, ray+e.xxy))
				);

			g_y = abs(
				4.0 * (texture3D(volume_texture, ray+e.wyw)-texture3D(volume_texture, ray+e.wxw))
				+ 2.0 * (texture3D(volume_texture, ray+e.xyw)-texture3D(volume_texture, ray+e.xxw)) 
				+ 2.0 * (texture3D(volume_texture, ray+e.yyw)-texture3D(volume_texture, ray+e.yxw))
				+ 2.0 * (texture3D(volume_texture, ray+e.wyx)-texture3D(volume_texture, ray+e.wxx))
				+ 2.0 * (texture3D(volume_texture, ray+e.wyy)-texture3D(volume_texture, ray+e.wxy))
				+ (texture3D(volume_texture, ray+e.xyx)-texture3D(volume_texture, ray+e.xxx)) 
				+ (texture3D(volume_texture, ray+e.yyy)-texture3D(volume_texture, ray+e.yxy))
				+ (texture3D(volume_texture, ray+e.yyx)-texture3D(volume_texture, ray+e.yxx))
				+ (texture3D(volume_texture, ray+e.xyy)-texture3D(volume_texture, ray+e.xxy))
				);

			g_z = abs(
				4.0 * (texture3D(volume_texture, ray+e.wwy)-texture3D(volume_texture, ray+e.wwx))
				+ 2.0 * (texture3D(volume_texture, ray+e.xwy)-texture3D(volume_texture, ray+e.xwx)) 
				+ 2.0 * (texture3D(volume_texture, ray+e.ywy)-texture3D(volume_texture, ray+e.ywx))
				+ 2.0 * (texture3D(volume_texture, ray+e.wxy)-texture3D(volume_texture, ray+e.wxx))
				+ 2.0 * (texture3D(volume_texture, ray+e.wyy)-texture3D(volume_texture, ray+e.wyx))
				+ (texture3D(volume_texture, ray+e.xxy)-texture3D(volume_texture, ray+e.xxx)) 
				+ (texture3D(volume_texture, ray+e.yyy)-texture3D(volume_texture, ray+e.yyx))
				+ (texture3D(volume_texture, ray+e.yxy)-texture3D(volume_texture, ray+e.yxx))
				+ (texture3D(volume_texture, ray+e.xyy)-texture3D(volume_texture, ray+e.xyx))
				);

			color_sample
				= mask.xwww * g_x
				+ mask.wxww * g_y
				+ mask.wwxw * g_z
				+ mask.wwwx * (g_x.x + g_y.y + g_z.z);
			break;

		case 5:
			// Sobel 3D operator with gradients from gradient_texture

			//g_x = abs(
			//	4.0 * (texture3D(volume_texture, ray+e.yww)-texture3D(volume_texture, ray+e.xww))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.yxw)-texture3D(volume_texture, ray+e.xxw))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.yyw)-texture3D(volume_texture, ray+e.xyw)) 
			//	+ 2.0 * (texture3D(volume_texture, ray+e.ywx)-texture3D(volume_texture, ray+e.xwx))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.ywy)-texture3D(volume_texture, ray+e.xwy))
			//	+ (texture3D(volume_texture, ray+e.yxx)-texture3D(volume_texture, ray+e.xxx))
			//	+ (texture3D(volume_texture, ray+e.yyy)-texture3D(volume_texture, ray+e.xyy)) 
			//	+ (texture3D(volume_texture, ray+e.yyx)-texture3D(volume_texture, ray+e.xyx))
			//	+ (texture3D(volume_texture, ray+e.yxy)-texture3D(volume_texture, ray+e.xxy))
			//	);

			//g_y = abs(
			//	4.0 * (texture3D(volume_texture, ray+e.wyw)-texture3D(volume_texture, ray+e.wxw))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.xyw)-texture3D(volume_texture, ray+e.xxw)) 
			//	+ 2.0 * (texture3D(volume_texture, ray+e.yyw)-texture3D(volume_texture, ray+e.yxw))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.wyx)-texture3D(volume_texture, ray+e.wxx))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.wyy)-texture3D(volume_texture, ray+e.wxy))
			//	+ (texture3D(volume_texture, ray+e.xyx)-texture3D(volume_texture, ray+e.xxx)) 
			//	+ (texture3D(volume_texture, ray+e.yyy)-texture3D(volume_texture, ray+e.yxy))
			//	+ (texture3D(volume_texture, ray+e.yyx)-texture3D(volume_texture, ray+e.yxx))
			//	+ (texture3D(volume_texture, ray+e.xyy)-texture3D(volume_texture, ray+e.xxy))
			//	);

			//g_z = abs(
			//	4.0 * (texture3D(volume_texture, ray+e.wwy)-texture3D(volume_texture, ray+e.wwx))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.xwy)-texture3D(volume_texture, ray+e.xwx)) 
			//	+ 2.0 * (texture3D(volume_texture, ray+e.ywy)-texture3D(volume_texture, ray+e.ywx))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.wxy)-texture3D(volume_texture, ray+e.wxx))
			//	+ 2.0 * (texture3D(volume_texture, ray+e.wyy)-texture3D(volume_texture, ray+e.wyx))
			//	+ (texture3D(volume_texture, ray+e.xxy)-texture3D(volume_texture, ray+e.xxx)) 
			//	+ (texture3D(volume_texture, ray+e.yyy)-texture3D(volume_texture, ray+e.yyx))
			//	+ (texture3D(volume_texture, ray+e.yxy)-texture3D(volume_texture, ray+e.yxx))
			//	+ (texture3D(volume_texture, ray+e.xyy)-texture3D(volume_texture, ray+e.xyx))
			//	);

			color_sample = mask.xxxw * texture3D(gradient_texture, ray) + mask.wwwx * sum3(texture3D(volume_texture, ray));
			break;

		default:
			// Raw scalar values without a transfer function
			color_sample = texture3D(volume_texture, ray);
		}

		/************************************************************************/
		/* lighting                                                             */
		/************************************************************************/
		if (lighting_option == 1)
		{
			vec3 ViewDirection  = fvEyePosition - ray;
			vec3 LightDirection = fvLightPosition - ray;

			vec3  fvLightDirection = normalize( LightDirection );
			vec3  fvNormal         = normalize( texture3D(gradient_texture, ray) ).xyz;
			float fNDotL           = dot( fvNormal, fvLightDirection ); 

			vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection ); 
			vec3  fvViewDirection  = normalize( ViewDirection );
			float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );

			vec4  fvBaseColor      = color_sample;

			vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 
			vec4  fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
			vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );

			color_sample = ( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular );
		}
		/************************************************************************/

		// get the alpha sample value
		alpha_sample = color_sample.a * stepsize;

		// calculate the accumulated color by stepsize
		col_acc += (1.0 - alpha_acc) * color_sample * alpha_sample;

		// calculate the accumulated alpha value
		alpha_acc += alpha_sample;

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
	}
}
