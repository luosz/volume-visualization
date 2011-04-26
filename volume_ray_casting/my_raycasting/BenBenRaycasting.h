#ifndef BenBenRaycasting_h
#define BenBenRaycasting_h

#include "VolumeReader.h"

// This definition is for ../BenBenRaycasting/color.h
#ifndef _WINDEF_
typedef unsigned char       BYTE;
#endif
#include "../BenBenRaycasting/color.h"
#include "../BenBenRaycasting/Vector3.h"

const double e = 2.7182818284590452353602874713526624977572470936999595749669676277240766303535;

typedef struct  
{	
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}color_opacity;

inline float norm(float min, float max, float x)  //convert data in range[min, max] to [0, 1]
{
	float result;
	if(fabs(max - min) < 1e-4)
		result = max;
	else
		result = (x - min) / (max - min);
	return result;
}

void setTransferfunc(color_opacity *& tf, Volume & volume)
{
	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();

	int x, y, z, index;
	float temp1, temp2,temp3, temp4;
	double d, g, df2, a, alpha, elasity, a1, a2, opacity, k = 0.1, f1, f2;
	float center_x, center_y, center_z;
	double df_dx, df_dy, df_dz, gradient, df, Ra;
	float range, q;
	float H, S, L;
	//dim_x = volume.getX();
	//dim_y = volume.getY();
	//dim_z = volume.getZ();
	tf = (color_opacity *)malloc(sizeof(color_opacity) * dim_x * dim_y * dim_z);
	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}
	center_x = float(volume.getX()) / 2.0; 
	center_y = float(volume.getY()) / 2.0;
	center_z = float(volume.getZ()) / 2.0;
	for(z = 0; z < dim_z; ++z)
		for(y = 0;y < dim_y; ++y)
			for(x = 0; x < dim_x; ++x)
			{
				index = volume.getIndex(x, y, z);
				d = 1 / 3.0 * (dim_x + dim_y + dim_z);				
				range = volume.getRange();
				H = double(volume.getData(x ,y ,z)) / double(range) * 360.0; 

				//	S = 1 - pow(e , -1.0 * d *double(Volume.getData(x, y, z)));
				//	S = exp()
				S = norm(volume.getMinData(), volume.getMaxData(), volume.getData(x, y, z));
				L = norm(volume.getMinGrad(), volume.getMaxGrad(), volume.getGrad(x, y, z));
				//			L = double(x) + double(y) + double(z) / (3 * d);


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

void setTransferFunc3(color_opacity *& tf, Volume & volume)
{
	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();

	int x, y, z, index, i,j ;
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



	//dim_x = volume.getX();
	//dim_y = volume.getY();
	//dim_z = volume.getZ();
	tf = (color_opacity *)malloc(sizeof(color_opacity) * dim_x * dim_y * dim_z);
	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}
	for(z = 0; z < dim_z; ++z)
		for(y = 0;y < dim_y; ++y)
			for(x = 0; x < dim_x; ++x)
			{
				index = volume.getIndex(x, y, z);

				range = volume.getRange();
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

				S = norm(float(volume.getMinGrad()), float(volume.getMaxGrad()), float(volume.getGrad(x, y, z))) * 360.0; 

				L = norm(float(volume.getMinDf2()), float(volume.getMaxDf2()), float(volume.getDf2(x, y, z)));


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

				tf[index].r  =  (unsigned char)(temp1 * 255);

				tf[index].g = (unsigned char)(temp2 * 255);
				tf[index].b =  (unsigned char)(temp3 * 255);

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
				//temp4 = 1.6 *(df1) / df1_max *  f / f_max;
				//temp4 = exp(df2 / df2_max * df1 / df1_max);
				temp4 =	exp(- d / g);
				alpha = temp4;
				//alpha = 1.0 + 1 / a * log((1.0 - pow(e, -a)) * temp4 + pow(e, -a)) / log(e);
				alpha = (exp(-a * (1.0 - temp4)) - exp(-a)) / (1 - exp(-a));
				//	alpha = f / f_max * df1 / df1_max;

				tf[index].a =  unsigned char(alpha * 255);



			}
}

void setTransferfunc6(color_opacity *& tf, Volume & volume)
{
	unsigned int dim_x = volume.getX();
	unsigned int dim_y = volume.getY();
	unsigned int dim_z = volume.getZ();

	int x, y, z, i, j, k, p, q, r, index, intensity, num = 0;
	float a, d, d_max = 0, gx, gy, gz, g, g_magnitude, t1, t2, t3;
	float alpha1, alpha2, alpha3, alpha4, beta;
	float theta1, theta2, bounding_angle, center_x, center_y, center_z;
	float v_x, v_y, v_z, dis;
	float g1, g2;

	Vector3 v;

	ofstream file("E:\\grad_direction.csv", std::ios::out);

	//dim_x = volume.getX();
	//dim_y = volume.getY();
	//dim_z = volume.getZ();
	beta = log((dim_x + dim_y + dim_z) / 3.0);

	tf = (color_opacity *)malloc(sizeof(color_opacity) * dim_x * dim_y * dim_z);
	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}
	for(i = 0;i < dim_x; ++i)
		for(j = 0; j < dim_y; ++j)
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
		//	d_max = sqrt(d_max);
		//	cout<<"d_max = "<<d_max<<endl;
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
							d = sqrt(d);
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
							/*if(d < (0.9 * d_max))
								alpha2 = 0;
							else
								alpha2 *= 1.5;*/
							/*	if(volume.getLocalEntropy(i, j, k) >= 0.7 * volume.getLocalEntropyMax())
									alpha2 = 0;
							*/

								

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

#endif // BenBenRaycasting_h