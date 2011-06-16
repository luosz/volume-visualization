/**	@file
* All transfer functions are in this file	
*/

#ifndef TRANSFER_FUNCTION_H
#define TRANSFER_FUNCTION_H

#include <fstream>

#include "color.h"
#include "Volume.h"
#include "Vector3.h"

const double e = 2.7182818284590452353602874713526624977572470936999595749669676277240766303535;
const double pi = 3.1415926535;

/**	@brief to store color and opacity, color is denoted by rgb triple, opacity is denoted 
*   by a, data are in unsigned char format to reduce memory to be used	
*/
typedef struct  
{	
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}color_opacity;

/**	@brief convert data value originally in range[min, max] to [0, 1]
*	
*/
float norm(float min, float max, float x)  
{
	float result;
	if(fabs(max - min) < 1e-4)
		result = max;
	else
		result = (x - min) / (max - min);
	return result;
}

/**	@brief check if the pointer is NULL, if not ,free the pointer, if so, do nothing.
*	
*/
void free_transfer_function_pointer(color_opacity *& p)
{
	if (p != NULL)
	{
		free(p);
		p = NULL;
	}
}

/**	@brief  free the pointer (if not NULL) and then allocate memory for it
*	
*/
void alloc_transfer_function_pointer(color_opacity *& p, unsigned int dim_x, unsigned int dim_y, unsigned int dim_z)
{
	free_transfer_function_pointer(p);
	p = (color_opacity *)malloc(sizeof(color_opacity) * dim_x * dim_y * dim_z);
}

/**	@brief set transfer function in HSL color space and using boundary emphasize
*	
*/
void setTransferfunc(color_opacity *& tf, Volume & volume)
{
	int x, y, z, index;
	// temp value,not final result
	float temp1, temp2,temp3, temp4; 
	double d, g, df2, a, alpha, elasity, a1, a2, opacity, k = 0.1, f1, f2;
	float center_x, center_y, center_z;
	double df_dx, df_dy, df_dz, gradient, df, Ra;
	float range, q;
	// Hue, saturation and Lightness
	float H, S, L;                                       
	// get X dimension
	unsigned int dim_x = volume.getX();    
	// get Y dimension
	unsigned int dim_y = volume.getY();    
	// get Z dimension
	unsigned int dim_z = volume.getZ();    

	// alloc_transfer_function_pointer, tf is the pointer points to the memory stores transfer 
	// function, dim_x, dim_y and dim_z are dimensions of the volume
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);

	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}

	// compute volume's central position
	center_x = float(volume.getX()) / 2.0; 
	center_y = float(volume.getY()) / 2.0;
	center_z = float(volume.getZ()) / 2.0;

	// iteration to every voxel to compute opacity and color 
	for(z = 0; z < dim_z; ++z)
	{
		for(y = 0;y < dim_y; ++y)
		{
			for(x = 0; x < dim_x; ++x)
			{
				// compute data's index in the volume data
				index = volume.getIndex(x, y, z);
				d = 1 / 3.0 * (dim_x + dim_y + dim_z);				
				range = volume.getRange();
				H = double(volume.getData(x ,y ,z)) / double(range) * 360.0; 

				// S = 1 - pow(e , -1.0 * d *double(Volume.getData(x, y, z)));
				// S = exp()
				S = norm(volume.getMinData(), volume.getMaxData(), volume.getData(x, y, z));
				L = norm(volume.getMinGrad(), volume.getMaxGrad(), volume.getGrad(x, y, z));
				// L = double(x) + double(y) + double(z) / (3 * d);


				HSL2RGB(H, S, L, &temp1, &temp2, &temp3);
				temp1 = sqrt(temp1);
				temp2 = sqrt(temp2);
				temp3 = sqrt(temp3);
				tf[index].r  = (unsigned char)(temp1 * 255);
				tf[index].g = (unsigned char)(temp2 * 255);
				tf[index].b = (unsigned char)(temp3 * 255);



				elasity = volume.getEp(x, y, z);
				gradient = volume.getGrad(x, y, z);
				q = log(d);
				if(gradient < 20 ||  volume.getDf3(x ,y , z) < 10 || volume.getDf2(x, y, z) < 10)
					opacity = 0;
				else 
				{
					Ra = - double(volume.getDf2(x, y, z)) / double(volume.getGrad(x, y, z));

					//		opacity = 1 - pow(e , -1.0 *  log(d)  * double(Volume.getMaxGrad() ) / gradient);
					opacity = 1 - pow(e, -1.0  * Ra);
					opacity = (exp(-1.0 * k * (1 - opacity)) - exp(-1.0 * q)) / (1 - exp(-1.0 * q));
					//		opacity = sqrt(opacity);
					opacity = sqrt(opacity);
					//	opdacity = sqrt(pow(x - center_x, 2.0) + pow())
				}	

				tf[index].a = unsigned char(opacity * 255);
			}
		}
	}
}

/**	@brief set transfer function in RGB color space and using boundary emphasize
*	get dim_x, dim_y, dim_z using volume.getX(),  volume.getY() and volume.getZ()
*   respectively, d to get data value using volume.getData(x, y, z)) and  g to get gradient
*    magnitude using volume.getGrad(x, y, z), using alpha = 
*    1.0 + 1 / a * log((1.0 - pow(e, -a)) * temp4 + pow(e, -a)) / log(e) to get opacity.
*/
void setTransferfunc2(color_opacity *& tf, Volume & volume)
{
	int x, y, z, index, i, j;
	float temp1, temp2,temp3, temp4;
	double d, g, df2, a, alpha;
	float range;
	float max_alpha = 0.5;
	float p, q, r, value, dF1;
	int t1, t2;
	float df1, df1_max, f, f_max, df2_max;
	float sigma = 4;

	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();

	// added by ark @ 2011.04.26
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);

	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}
	for(z = 0; z < dim_z; ++z)
	{
		for(y = 0;y < dim_y; ++y)
		{
			for(x = 0; x < dim_x; ++x)
			{
				index = volume.getIndex(x, y, z);
				d = double(volume.getData(x, y, z));
				g = double(volume.getGrad(x, y, z));
				a = log(double(volume.getX() + volume.getY() + volume.getZ()) / 3.0); 
				temp4 = exp(-d / g);

				df1 = (float)volume.getGrad(x, y, z);
				df1_max = (float)volume.getMaxGrad();
				f = (float)volume.getData(x, y, z);
				f_max = volume.getMaxData();
				df2 = double(volume.getDf2(x,y ,z));
				df2_max = volume.getMaxDf2();

				temp4 =	exp(- d / g);
				alpha = temp4;
				alpha = 1.0 + 1 / a * log((1.0 - pow(e, -a)) * temp4 + pow(e, -a)) / log(e);
				//alpha = (exp(-a * (1.0 - temp4)) - exp(-a)) / (1 - exp(-a));

				float ddd = sqrt(x / float(volume.getX()) * x / (float)volume.getX()
					+ y / float(volume.getY() * y / float(volume.getY()))
					+ z / float(volume.getZ() * z / float(volume.getZ())));

				tf[index].a =  unsigned char(alpha * 255);

			}
		}
	}
}

/**	@brief set transfer function in HSL color space and using boundary emphasize
*	
*/
void setTransferfunc3(color_opacity *& tf, Volume & volume)
{
	int x, y, z, index, i,j ;   
	// temp value to store intermediate value
	float temp1, temp2,temp3, temp4;      
	double d, g, df2, a, alpha;
	float range;
	float max_alpha = 0.5;
	float p, q, r, value, df1;
	int t1, t2;
	float H, S, L;
	bool b1;
	bool b2;
	float dF1, df1_max, f, f_max, df2_max;
	float sigma = 4;


	// get number of voxels of dimension X
	unsigned int dim_x = volume.getX();          
	// get number of voxels of dimension Y
	unsigned int dim_y = volume.getY();          
	// get number of voxels of dimension Z
	unsigned int dim_z = volume.getZ();          

	// added by ark @ 2011.04.26
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);

	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}

	// iteration to every voxel to compute opacity and color 
	for(z = 0; z < dim_z; ++z)
	{
		for(y = 0;y < dim_y; ++y)
		{
			for(x = 0; x < dim_x; ++x)
			{
				// compute data's index in the volume data
				index = volume.getIndex(x, y, z);

				// compute volume's data value range [0,range]
				range = volume.getRange();

				// compute Hue value according to data value
				if(volume.getData(x, y, z) <= range / 6.0)
					H = 30;
				else if(volume.getData(x, y, z) <= range * (1.0 / 3.0))
					H = 90;
				else if(volume.getData(x, y, z) <= range * (1.0 / 2.0))
					H = 150;
				else if(volume.getData(x, y, z) <= range * (2.0 / 3.0))
					H = 210;
				else if(volume.getData(x, y, z) <= range * (5.0 / 6.0))
					H = 270;
				else
					H = 330;

				// compute saturation according to gradient magnitude
				S = norm(float(volume.getMinGrad()), float(volume.getMaxGrad()), float(volume.getGrad(x, y, z))) * 360.0; 

				// compute lightness according to second derivative
				L = norm(float(volume.getMinDf2()), float(volume.getMaxDf2()), float(volume.getDf2(x, y, z)));

				// convert H, S, and L to rgb color space, r, g and b componet stored in temp1, temp2 and temp3
				HSL2RGB(H, S, L, &temp1, &temp2, &temp3);
				temp1 *= 1.5;
				temp2 *= 1.5;
				temp3 *= 1.5;
				if(temp1 > 1.0)
					temp1 = 1.0;
				if(temp2 > 1.0)
					temp2 = 1.0;
				if(temp3 > 1.0)
					temp3 = 1.0;

				// compute transfer function's color
				tf[index].r  =  (unsigned char)(temp1 * 255);
				tf[index].g = (unsigned char)(temp2 * 255);
				tf[index].b =  (unsigned char)(temp3 * 255);

				// compute transfer function's opacity
				// get data value
				d = double(volume.getData(x, y, z));
				// get gradient magnitude
				g = double(volume.getGrad(x, y, z));
				a = log(double(volume.getX() + volume.getY() + volume.getZ()) / 3.0); 
				// compute temp opacity
				temp4 = exp(-d / g);

				df1 = (float)volume.getGrad(x, y, z);
				df1_max = (float)volume.getMaxGrad();
				f = (float)volume.getData(x, y, z);
				f_max = volume.getMaxData();
				//	df2 = double(volume.getDf2(x,y ,z));
				//	df2_max = volume.getMaxDf2();
				//temp4 = 1.6 *(df1) / df1_max *  f / f_max;
				//temp4 = exp(df2 / df2_max * df1 / df1_max);
				temp4 =	exp(- d / g);
				alpha = temp4;

				// correct temp opacity to get final opacity
				alpha = (exp(-a * (1.0 - temp4)) - exp(-a)) / (1 - exp(-a));
				alpha *= 1.5;

				if(d < 0.8 * volume.getMaxGrad())
					alpha = 0;
				//if(volume.getLocalEntropy(x, y, z) > 0.7 * volume.getLocalEntropyMax())
				//	alpha = 0;
				//	alpha = f / f_max * df1 / df1_max;

				// compute transfer function's opacity and stores it in tf
				tf[index].a =  unsigned char(alpha * 255);
			}
		}
	}
}

/**	@brief set transfer function in statistical space and using gradient vector
*	to set color 
*/
void setTransferfunc5(color_opacity *& tf, Volume & volume)
{
	int x, y, z, i, j, k, p, q, r, index, intensity, num = 0;
	// statistical property -- 
	float a;  
	float d;
	float d_max = 0;
	float gx, gy, gz, g, g_magnitude;
	float alpha1, alpha2, alpha3, alpha4, beta;

	// get number of voxels at dimension X, Y and Z respectively
	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();
	beta = log((dim_x + dim_y + dim_z) / 3.0);

	// allocate transfer function space
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);

	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}

	// iteration to every voxel to compute opacity and color 
	for(i = 0;i < dim_x; ++i)
	{
		for(j = 0; j < dim_y; ++j)
		{
			for(k = 0; k < dim_z; ++k)
			{
				// compute data's index in the volume data
				index = volume.getIndex(i, j, k);	

				// for voxel at the boundary of the volume, all values set to 0
				if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
				{						
					a = d = 0; 
					tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
				}
				else
				{
					// initialize average and deviation to 0
					a = d = 0;

					// compute average value around the central voxel at (i, j, k)
					for(p = i - 1;p <= i + 1;++p)
						for(q = j - 1; q <= j + 1; ++q)
							for(r = k - 1; r <= k + 1; ++r)
								a += float(volume.getData(p, q, r));
					a /= 27.0;

					// compute deviation around cenral voxel at (i, j, k)
					for(p = i - 1;p <= i + 1;++p)
						for(q = j - 1; q <= j + 1; ++q)
							for(r = k - 1; r <= k + 1; ++r)
								d += pow(double(volume.getData(p, q, r)) - a, 2.0);
					d /= 27;
					if(d == 0)
						d = 1e-4;

					// compute maximum deviation
					if(d > d_max)
						d_max = d;
				}

			}
			//		cout<<"d_max = "<<d_max<<endl; 
			for(i = 0;i < dim_x; ++i)
				for(j = 0;j < dim_y; ++j)
					for(k = 0;k < dim_z; ++k)
					{
						index = volume.getIndex(i, j, k);	
						if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
						{						
							a = d = 0; 
							tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
						}
						else
						{
							a = d = 0;

							// compute average value around the central voxel at (i, j, k)
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										a += float(volume.getData(p, q, r));
							a /= 27;

							// compute deviation around cenral voxel at (i, j, k)
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										d += pow(double(volume.getData(p, q, r)) - a, 2.0);
							d /= 27;
							if(d == 0)
								d = 1e-4;

							//		intensity = Volume.getData(i, j, k);
							//		g_magnitude = Volume.getGrad(i, j, k);

							// compute orginal opacity using average and deviation
							alpha1 = exp(-1.0 * a / d);

							// correct original opacity to get final result
							alpha2 = ( exp(-beta * (1 - alpha1)) - exp(-beta) ) / (1 - exp(-beta));

							//	if(unsigned int(d) < unsigned int(0.95 * d_max))
							//		alpha2 = 0;


							/*if(volume.getLocalEntropy(i, j, k) > 0.7 * volume.getLocalEntropyMax())
							alpha2 = 0;*/
							//if(alpha2 > 0.6)
							//{
							//	num++;
							//	//	cout<<alpha2<<endl;
							//}
							if(alpha2 < 0.8)
								alpha2 = 0;
							/*		else
							alpha2 *= 1.5;*/
							//		alpha2 = 0;

							// comput transfer function's opacity
							tf[index].a = unsigned char(alpha2 * 255);

							/*if(alpha4 < 0.2)
							alpha4 = 0;
							tf[index].a  = unsigned char(alpha4 * 255);*/

							// compute gradient vector of direction x, y and z
							gx = fabs(float(volume.getData(i + 1, j, k)) - float(volume.getData(i - 1, j, k)));
							gy = fabs(float(volume.getData(i , j + 1, k)) - float(volume.getData(i , j - 1, k)));
							gz = fabs(float(volume.getData(i , j, k + 1)) - float(volume.getData(i , j, k - 1)));
							// compute gradient magnitude
							g = sqrt(gx * gx + gy * gy + gz * gz);

							// map normalized gradient vector to rgb component to get transfer function's final color
							tf[index].r = unsigned char(gx / g * 255.0);
							tf[index].g = unsigned char(gy / g * 255.0);
							tf[index].b = unsigned char(gz / g * 255.0); 
						}
					}
		}
	}
}

/**	@brief set transfer function in statistical space and using gradient vector
*	to set color 
*/
void setTransferfunc6(color_opacity *& tf, Volume & volume)
{
	int x, y, z, i, j, k, p, q, r, index, intensity, num = 0;
	float a, d, d_max = 0, gx, gy, gz, g, g_magnitude, t1, t2, t3;
	float alpha1, alpha2, alpha3, alpha4, beta;
	float theta1, theta2, bounding_angle, center_x, center_y, center_z;
	float v_x, v_y, v_z, dis;
	float g1, g2;

	Vector3 v;

	ofstream file("E:\\grad_direction.csv", std::ios::out);

	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();
	beta = log((dim_x + dim_y + dim_z) / 3.0);

	// added by ark @ 2011.04.26
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);
	for(x = 0; x < dim_x; ++x)
		for(y = 0; y < dim_y; ++y)
			for(z = 0; z < dim_z; ++z)
			{
				index = volume.getIndex(x, y, z);
				tf[index].r = tf[index].g = tf[index].b = tf[index].a = 0;
			}

			if(tf == NULL)
			{
				fprintf(stderr, "Not enough space for tf");
			}
			for(i = 0;i < dim_x; ++i)
			{
				for(j = 0; j < dim_y; ++j)
				{
					for(k = 0; k < dim_z; ++k)
					{
						index = volume.getIndex(i, j, k);	
						if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
						{						
							a = d = 0; 
							tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
						}
						else
						{
							a = d = 0;
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										a += float(volume.getData(p, q, r));
							a /= 27;
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										d += pow(double(volume.getData(p, q, r)) - a, 2.0);
							d /= 27;
							if(d == 0)
								d = 1e-4;
							if(d > d_max)
								d_max = d;
						}

					}
				}
			}
			//	d_max = sqrt(d_max);

			for(i = 0;i < dim_x; ++i)
			{
				for(j = 0;j < dim_y; ++j)
				{
					for(k = 0;k < dim_z; ++k)
					{
						index = volume.getIndex(i, j, k);	
						if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
						{						
							a = d = 0; 
							tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
						}
						else
						{
							a = d = 0;
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										a += float(volume.getData(p, q, r));
							a /= 27;
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										d += pow(double(volume.getData(p, q, r)) - a, 2.0);
							d /= 27;
							//			d = sqrt(d);
							if(d == 0)
								d = 1e-4;

							/*	intensity = volume.getData(i, j, k);
							g_magnitude = volume.getGrad(i, j, k);*/
							//	cout<<"d =" <<d<<endl;
							alpha1 = exp(-1.0 * a / d);
							alpha2 = ( exp(-beta * (1 - alpha1)) - exp(-beta) ) / (1 - exp(-beta));

							//			alpha3 = exp(-1.0 * float(intensity) / g_magnitude);
							//			alpha4 = ( exp(-beta * (1 - alpha3)) - exp(-beta) ) / (1 - exp(-beta));
							//			cout<<d<<endl;
							if(d < (0.9 * d_max))
								alpha2 = 0;		
							else
								alpha2 *= 1.5;
							/*	if(volume.getLocalEntropy(i, j, k) >= 0.7 * volume.getLocalEntropyMax())
							alpha2 = 0;
							else
							alpha2 *= 1.5;*/




							//		tf[index].a = unsigned char(alpha2 * 255);

							//if(alpha4 < 0.2)
							//alpha4 = 0;

							tf[index].a  = unsigned char(alpha2 * 255);

							x = i;
							y = j;
							z = k;
							if(x == 0)
								gx = float(volume.getData(x + 1, y, z) - volume.getData(x, y, z));
							else if(x == dim_x - 1)
								gx = float(volume.getData(x, y, z) - volume.getData(x - 1, y, z));
							else
								gx = float(volume.getData(x + 1, y, z)) - float(volume.getData(x - 1, y, z));

							if(y == 0)
								gy = float(volume.getData(x , y + 1, z) - volume.getData(x, y, z));
							else if(y == dim_y - 1)
								gy = float(volume.getData(x, y, z) - volume.getData(x, y - 1, z));
							else
								gy = float(volume.getData(x , y + 1, z)) - float(volume.getData(x , y - 1, z));

							if(z == 0)
								gz = float(volume.getData(x, y, z + 1) - volume.getData(x, y, z));
							else if(z == dim_z - 1)
								gz =float(volume.getData(x, y, z) - volume.getData(x, y, z - 1));
							else
								gz = float(volume.getData(x , y, z + 1)) - float(volume.getData(x , y, z - 1));

							/*	if(!(gx >=0 && gy >=0 && gz >= 0))
							{
							tf[index].a = 0;
							}
							else
							tf[index].a = 255;*/
							g = sqrt(gx * gx + gy * gy + gz * gz);
							file<<gx<<", "<<gy<<", "<<gz<<endl;
							gx =fabs(gx);
							gy =fabs(gy);
							gz = fabs(gz);



							tf[index].r = unsigned char(gx / g * 255.0);
							tf[index].g = unsigned char(gy / g * 255.0);
							tf[index].b = unsigned char(gz / g * 255.0); 
							/*				
							if(i * i + j * j + k * k > 150 * 150)
							tf[index].a = 0;*/

						}
					}
				}
			}
			cout<<"d_max = "  <<d_max<<endl;
			/*cout<<"theta 1 = :";
			cin>>theta1;
			cout<<endl<<"theta 2 = :";
			cin>>theta2;
			cout<<endl<<"bounding angle = :";
			cin>>bounding_angle;
			cout<<endl<<"g1 = :";
			cin>>g1;
			cout<<endl<<"g2 = :";
			cin>>g2;

			theta1 = theta1 / 180 * pi;
			theta2 = theta2 / 180 * pi;

			v_x = cos(theta1);
			v_y = sin(theta1) * cos(theta2);
			v_z = sin(theta1) * sin(theta2);*/

			v = Vector3(v_x, v_y, v_z);
			//		select_user_interested_area(v, bounding_angle, g1, g2);

			/*center_x =float(dim_x) / 2.0;
			center_y = float(dim_y) / 2.0;
			center_z = float(dim_z) / 2.0;*/

			//for(i = 0;i < dim_x; ++i)
			//	for(j = 0;j < dim_y; ++j)
			//		for(k = 0;k < dim_z; ++k)
			//		{
			//			index = volume.getIndex(i, j, k);
			//			dis = sqrt( pow(double(i - center_x), 2.0)
			//				+ pow(double(j - center_y), 2.0)
			//				+ pow(double(k - center_z), 2.0));
			//			if(dis >= float(dim_x + dim_y + dim_z) / 6.0)
			//				tf[index].a = 0;
			//		}
			//		cout<<float(num) / float(dim_x * dim_y * dim_z)<<endl;
}

/**	@brief set transfer function in statistical space and using gradient vector
*	to set color 
*/
void setTransferfunc7(color_opacity *& tf, Volume & volume)
{
	int x, y, z, i, j, k, p, q, r, index, intensity, num = 0;
	float a, d, d_max = 0, gx, gy, gz, g, g_magnitude, t1, t2, t3;
	float alpha1, alpha2, alpha3, alpha4, beta;
	float theta1, theta2, bounding_angle, center_x, center_y, center_z;
	float v_x, v_y, v_z, dis;
	float g1, g2;

	Vector3 v;

	ofstream file("E:\\grad_direction.csv", std::ios::out);

	// get number of voxels at dimension x, y and z respectively
	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();
	beta = log((dim_x + dim_y + dim_z) / 3.0);

	// allocate memory for transfer function space
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);

	// initialize transfer function
	for(x = 0; x < dim_x; ++x)
		for(y = 0; y < dim_y; ++y)
			for(z = 0; z < dim_z; ++z)
			{
				index = volume.getIndex(x, y, z);
				tf[index].r = tf[index].g = tf[index].b = tf[index].a = 0;
			}

			if(tf == NULL)
			{
				fprintf(stderr, "Not enough space for tf");
			}

			// traverse all the voxels to compute opacity and color
			for(i = 0;i < dim_x; ++i)
			{
				for(j = 0;j < dim_y; ++j)
				{
					for(k = 0;k < dim_z; ++k)
					{
						// get data value's index in the volume 
						index = volume.getIndex(i, j, k);	

						// for voxels lie on the boundary of the volume data, color and opacity set to 0
						if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
						{						
							tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
						}
						else
						{
							// compute average value
							a = volume.getAverage(i, j, k);

							// compute variation value
							d = volume.getVariation(i, j, k);

							// compute original opacity
							alpha1 = exp(-1.0 * a / d);

							// compute final opacity 
							alpha2 = ( exp(-beta * (1 - alpha1)) - exp(-beta) ) / (1 - exp(-beta));

							d_max = volume.getMaxVariation();
							if(d < (0.9 * d_max))
								alpha2 = 0;		
							else
							{
								if(volume.getLocalEntropy(i, j, k) >= (0.7 * volume.getLocalEntropyMax()))
									alpha2 = 0;
								else
									alpha2 *= 1.5;
							}

							// compute final opacity stored in tf
							tf[index].a  = unsigned char(alpha2 * 255);

							x = i;
							y = j;
							z = k;

							// compute gradient vector in x direction
							if(x == 0)
								gx = float(volume.getData(x + 1, y, z) - volume.getData(x, y, z));
							else if(x == dim_x - 1)
								gx = float(volume.getData(x, y, z) - volume.getData(x - 1, y, z));
							else
								gx = float(volume.getData(x + 1, y, z)) - float(volume.getData(x - 1, y, z));

							// compute gradient vector in y direction
							if(y == 0)
								gy = float(volume.getData(x , y + 1, z) - volume.getData(x, y, z));
							else if(y == dim_y - 1)
								gy = float(volume.getData(x, y, z) - volume.getData(x, y - 1, z));
							else
								gy = float(volume.getData(x , y + 1, z)) - float(volume.getData(x , y - 1, z));

							// compute gradient vector in z direction
							if(z == 0)
								gz = float(volume.getData(x, y, z + 1) - volume.getData(x, y, z));
							else if(z == dim_z - 1)
								gz =float(volume.getData(x, y, z) - volume.getData(x, y, z - 1));
							else
								gz = float(volume.getData(x , y, z + 1)) - float(volume.getData(x , y, z - 1));

							// compute gradient magnitude
							g = sqrt(gx * gx + gy * gy + gz * gz);
							file<<gx<<", "<<gy<<", "<<gz<<endl;
							gx =fabs(gx);
							gy =fabs(gy);
							gz = fabs(gz);

							// set color r, g, b using normalized gradient vector of x, y and z direction 
							tf[index].r = unsigned char(gx / g * 255.0);
							tf[index].g = unsigned char(gy / g * 255.0);
							tf[index].b = unsigned char(gz / g * 255.0); 
						}
					}
				}
			}

}

/// transfer function in statistical space
void setTransferfunc8(color_opacity *& tf, Volume & volume)
{
	int x, y, z, i, j, k, p, q, r, index, intensity, num = 0;
	double a, d, d_max = 0, gx, gy, gz, g, g_magnitude, t1, t2, t3;
	double alpha1, alpha2, alpha3, alpha4, beta;

	float g1, g2;
	
	// get number of voxels at dimension x, y and z respectively
	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();
	beta = log(double(dim_x + dim_y + dim_z) / 3.0);

	// allocate memory for transfer function space
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);
	for(x = 0; x < dim_x; ++x)
		for(y = 0; y < dim_y; ++y)
			for(z = 0; z < dim_z; ++z)
			{
				index = volume.getIndex(x, y, z);
				tf[index].r = tf[index].g = tf[index].b = tf[index].a = 0;
			}

			if(tf == NULL)
			{
				fprintf(stderr, "Not enough space for tf");
			}

			// traverse all the voxels to compute maximum deviation
			for(i = 0;i < dim_x; ++i)
			{
				for(j = 0;j < dim_y; ++j)
				{
					for(k = 0;k < dim_z; ++k)
					{
						index = volume.getIndex(i, j, k);	
						
						// for voxels lie on the boundary of the volume data, color and opacity set to 0
						if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
						{						
							tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
						}
						else
						{
							index = volume.getIndex(i, j, k);	
							
							// get average value
							a = double(volume.getAverage(i, j, k));
							// get variation value
							d = double(volume.getVariation(i, j, k));
							d = sqrt(d);
							//		cout<<"a = "<<a<<"   d  ="<<d<<endl;
							
							// compute original opacity using average value and deviation value
							alpha1 = exp(-a / d);

							// compute final opacity 
							alpha2 = ( exp(-beta * (1.0 - alpha1)) - exp(-beta) ) / (1.0 - exp(-beta));

							alpha2 *= 1.5;
							d_max = double(volume.getMaxVariation());
							d_max = sqrt(d_max);
							//	cout<<"d_max ="<<d_max<<endl;
							if(d < (0.9 * d_max))
								alpha2 = 0;		
							/*else
							{
							if(volume.getLocalEntropy(i, j, k) >= (0.7 * volume.getLocalEntropyMax()))
							alpha2 = 0;
							else
							alpha2 *= 1.5;
							}*/

							// compute final opacity stored in tf
							tf[index].a  = unsigned char(alpha2 * 255);

							x = i;
							y = j;
							z = k;

							// compute gradient vector in x direction
							if(x == 0)
								gx = float(volume.getData(x + 1, y, z) - volume.getData(x, y, z));
							else if(x == dim_x - 1)
								gx = float(volume.getData(x, y, z) - volume.getData(x - 1, y, z));
							else
								gx = float(volume.getData(x + 1, y, z)) - float(volume.getData(x - 1, y, z));

							// compute gradient vector in y direction
							if(y == 0)
								gy = float(volume.getData(x , y + 1, z) - volume.getData(x, y, z));
							else if(y == dim_y - 1)
								gy = float(volume.getData(x, y, z) - volume.getData(x, y - 1, z));
							else
								gy = float(volume.getData(x , y + 1, z)) - float(volume.getData(x , y - 1, z));

							// compute gradient vector in z direction
							if(z == 0)
								gz = float(volume.getData(x, y, z + 1) - volume.getData(x, y, z));
							else if(z == dim_z - 1)
								gz =float(volume.getData(x, y, z) - volume.getData(x, y, z - 1));
							else
								gz = float(volume.getData(x , y, z + 1)) - float(volume.getData(x , y, z - 1));

							// compute gradient magnitude
							g = sqrt(gx * gx + gy * gy + gz * gz);
							gx =fabs(gx);
							gy =fabs(gy);
							gz = fabs(gz);

							// set color r, g, b using normalized gradient vector of x, y and z direction 
							tf[index].r = unsigned char(gx / g * 255.0);
							tf[index].g = unsigned char(gy / g * 255.0);
							tf[index].b = unsigned char(gz / g * 255.0); 
						}
					}
				}
			}

}

/// transfer function in statistical space using average and deviation
void setTransferfunc9(color_opacity *& tf, Volume & volume)
{
	int x, y, z, i, j, k, p, q, r, index, intensity, num = 0;
	double a, d, d_max = 0, gx, gy, gz, g, g_magnitude;
	float alpha1, alpha2, alpha3, alpha4, beta;

	// get number of voxels at dimension x, y and z respectively
	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();
	// compute beta
	beta = log((dim_x + dim_y + dim_z) / 3.0);

	// allocate memory for transfer function space
	alloc_transfer_function_pointer(tf, dim_x, dim_y, dim_z);

	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}

	// traverse all the voxels to compute maximum deviation
	for(i = 0;i < dim_x; ++i)
		for(j = 0; j < dim_y; ++j)
			for(k = 0; k < dim_z; ++k)
			{
				// get data value's index in the volume 
				index = volume.getIndex(i, j, k);	
				
				// for voxels lie on the boundary of the volume data, color and opacity set to 0
				if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
				{						
					a = d = 0; 
					tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
				}
				else
				{
					a = d = 0;
					
					// compute average value around central voxel at (i, j, k)
					for(p = i - 1;p <= i + 1;++p)
						for(q = j - 1; q <= j + 1; ++q)
							for(r = k - 1; r <= k + 1; ++r)
								a += float(volume.getData(p, q, r));
					a /= 27.0;
					
					// compute variation value around central voxel at (i, j, k)
					for(p = i - 1;p <= i + 1;++p)
						for(q = j - 1; q <= j + 1; ++q)
							for(r = k - 1; r <= k + 1; ++r)
								d += pow(double(volume.getData(p, q, r)) - a, 2.0);
					d /= 27.0;
					//		cout<<d<<endl;

					if(d == 0)
						d = 1e-4;

					// compute maximum deviation
					if(d > d_max)
						d_max = d;
				}
			}
			//cout<<d_max<<endl;

			// traverse all the voxels to set opacity and color
			for(i = 0;i < dim_x; ++i)
				for(j = 0; j < dim_y; ++j)
					for(k = 0;k < dim_z; ++k)
					{
						// get data value's index in the volume
						index = volume.getIndex(i, j, k);	
						//	cout<<index<<endl;

						// for voxels lie on the boundary of the volume data, color and opacity set to 0
						if(i == 0 || i == dim_x - 1|| j == 0 || j == dim_y - 1 || k == 0 || k == dim_z - 1)
						{						
							a = d = 0; 
							tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
						}
						else
						{
							a = d = 0;
							
							// compute average value around central voxel at (i, j, k)
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										a += float(volume.getData(p, q, r));
							a /= 27.0;
							//	cout<<"a = "<<a<<endl;

							// compute deviation value around central voxel at (i, j, k)
							for(p = i - 1;p <= i + 1;++p)
								for(q = j - 1; q <= j + 1; ++q)
									for(r = k - 1; r <= k + 1; ++r)
										d += pow(double(volume.getData(p, q, r)) - a, 2.0);
							d /= 27.0;
							if(d == 0)
								d = 1e-4;
							
							// compute original opacity using average value and deviation value
							alpha1 = exp(-a / d);

							// compute final opacity 
							alpha2 = ( exp(-beta * (1.0 - alpha1)) - exp(-beta) ) / (1.0 - exp(-beta));
							//		cout<<d<<endl<<d_max<<endl;
							if(d < 0.6 * d_max)
							{
								//	cout<<"ok"<<endl;
								alpha2 = 0;
							}
							if(alpha2 < 0.9)
								alpha2 = 0;
							else alpha2 *= 1.5;

							// compute final opacity stored in tf
							tf[index].a  = unsigned char(alpha2 * 255);

							x = i;
							y = j;
							z = k;
							
							// compute gradient vector in x direction
							if(x == 0)
								gx = float(volume.getData(x + 1, y, z) - volume.getData(x, y, z));
							else if(x == dim_x - 1)
								gx = float(volume.getData(x, y, z) - volume.getData(x - 1, y, z));
							else
								gx = float(volume.getData(x + 1, y, z)) - float(volume.getData(x - 1, y, z));

							// compute gradient vector in y direction
							if(y == 0)
								gy = float(volume.getData(x , y + 1, z) - volume.getData(x, y, z));
							else if(y == dim_y - 1)
								gy = float(volume.getData(x, y, z) - volume.getData(x, y - 1, z));
							else
								gy = float(volume.getData(x , y + 1, z)) - float(volume.getData(x , y - 1, z));
							
							// compute gradient vector in z direction
							if(z == 0)
								gz = float(volume.getData(x, y, z + 1) - volume.getData(x, y, z));
							else if(z == dim_z - 1)
								gz =float(volume.getData(x, y, z) - volume.getData(x, y, z - 1));
							else
								gz = float(volume.getData(x , y, z + 1)) - float(volume.getData(x , y, z - 1));
							
							// compute gradient magnitude
							g = sqrt(gx * gx + gy * gy + gz * gz);
							gx =fabs(gx);
							gy =fabs(gy);
							gz = fabs(gz);

							// set color r, g, b using normalized gradient vector of x, y and z direction 
							tf[index].r = unsigned char(gx / g * 255.0);
							tf[index].g = unsigned char(gy / g * 255.0);
							tf[index].b = unsigned char(gz / g * 255.0); 
						}
					}	
}





#endif // TRANSFER_FUNCTION_H