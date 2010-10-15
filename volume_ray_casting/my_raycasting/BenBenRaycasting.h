#ifndef BenBenRaycasting_h
#define BenBenRaycasting_h

//////////////////////////////////////////////////////////////////////////
// imported from Ben's codes
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// An adapter for importing Ben\volume.h
#include "VolumeReader.h"
//////////////////////////////////////////////////////////////////////////

#ifndef _WINDEF_
typedef unsigned char       BYTE;
#endif
#include "../BenBenRaycasting/color.h"

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

void setTransferfunc_Ben(color_opacity *& tf, volume & Volume)
{
	int x, y, z, index;
	float temp1, temp2,temp3, temp4;
	double d, g, df2, a, alpha, elasity, a1, a2, opacity, k = 0.1, f1, f2;
	float center_x, center_y, center_z;
	double df_dx, df_dy, df_dz, gradient, df, Ra;
	float range, q;
	float H, S, L;

	//dim_x = Volume.getX();
	//dim_y = Volume.getY();
	//dim_z = Volume.getZ();
	//tf = (color_opacity *)malloc(sizeof(color_opacity) * dim_x * dim_y * dim_z);
	unsigned int dim_x = Volume.getX();
	unsigned int dim_y = Volume.getY();
	unsigned int dim_z = Volume.getZ();

	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}
	center_x = float(Volume.getX()) / 2.0; 
	center_y = float(Volume.getY()) / 2.0;
	center_z = float(Volume.getZ()) / 2.0;
	for(z = 0; z < dim_z; ++z)
		for(y = 0;y < dim_y; ++y)
			for(x = 0; x < dim_x; ++x)
			{
				index = Volume.getIndex(x, y, z);
				d = 1 / 3.0 * (dim_x + dim_y + dim_z);				
				range = Volume.getRange();
				H = double(Volume.getData(x ,y ,z)) / double(range) * 360.0; 

				//	S = 1 - pow(e , -1.0 * d *double(Volume.getData(x, y, z)));
				//	S = exp()
				S = norm(Volume.getMinData(), Volume.getMaxData(), Volume.getData(x, y, z));
				L = norm(Volume.getMinGrad(), Volume.getMaxGrad(), Volume.getGrad(x, y, z));
				//			L = double(x) + double(y) + double(z) / (3 * d);


				HSL2RGB(H, S, L, &temp1, &temp2, &temp3);
				temp1 = sqrt(temp1);
				temp2 = sqrt(temp2);
				temp3 = sqrt(temp3);
				tf[index].r  = (unsigned char)(temp1 * 255);

				tf[index].g = (unsigned char)(temp2 * 255);
				tf[index].b = (unsigned char)(temp3 * 255);



				elasity = Volume.getEp(x, y, z);
				gradient = Volume.getGrad(x, y, z);
				q = log(d);
				if(gradient < 20 ||  Volume.getDf3(x ,y , z) < 10 || Volume.getDf2(x, y, z) < 10)
					opacity = 0;

				else 
				{
					Ra = - double(Volume.getDf2(x, y, z)) / double(Volume.getGrad(x, y, z));

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

#endif // BenBenRaycasting_h