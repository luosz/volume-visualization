#ifndef Ben_imported_h
#define Ben_imported_h

//////////////////////////////////////////////////////////////////////////
// An adapter for importing Ben\volume.h
#include "VolumeReader.h"
//////////////////////////////////////////////////////////////////////////

#ifndef _WINDEF_
typedef unsigned char       BYTE;
#endif
#include "Ben/color.h"

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

void setTransferfunc(color_opacity *& tf, volume & Volume)
{
	//int x, y, z, index;
	unsigned int index;
	float temp1, temp2,temp3, temp4;
	double d, g, df2, a, alpha;
	float range;
	float H, S, L;
	unsigned int dim_x = Volume.getX();
	unsigned int dim_y = Volume.getY();
	unsigned int dim_z = Volume.getZ();
	//tf = (color_opacity *)malloc(sizeof(color_opacity) * dim_x * dim_y * dim_z);
	if(tf == NULL)
	{
		fprintf(stderr, "Not enough space for tf");
	}
	for(unsigned int z = 0; z < dim_z; ++z)
		for(unsigned int y = 0;y < dim_y; ++y)
			for(unsigned int x = 0; x < dim_x; ++x)
			{
				index = Volume.getIndex(x, y, z);
			/*	H = (float(Volume.getData(x, y, z)) / float(Volume.getMaxData())) * 360.0; 
				S = (float(Volume.getDf2(x, y, z)) / float(Volume.getMaxDf2()));
				L = (float(Volume.getDf3(x, y, z)) / float(Volume.getMaxDf3()));*/
				range = Volume.getRange();
				if(Volume.getData(x, y, z) <= range / 6.0)
					H = 30;
				else if(Volume.getData(x, y, z) <= range * (1.0 / 3.0))
					H = 90;
				else if(Volume.getData(x, y, z) <= range * (1.0 / 2.0))
					H = 150;
				else if(Volume.getData(x, y, z) <= range * (2.0 / 3.0))
					H = 210;
				else if(Volume.getData(x, y, z) <= range * (5.0 / 6.0))
					H = 270;
				else
					H = 330;
		
			
			//	H = Volume.getHistogram(Volume.getData(x, y, z)) / (Volume.getX() * Volume.getY() * Volume.getZ());

				S = norm(float(Volume.getMinData()), float(Volume.getMaxData()), float(Volume.getData(x, y, z))) * 360.0;				
			//	 H = norm(float(Volume.getMinDf2()), float(Volume.getMaxDf2()), float(Volume.getDf2(x, y, z)));
				L = norm(float(Volume.getMinDf3()), float(Volume.getMaxDf3()), float(Volume.getDf3(x, y, z)));
		//		L = norm(float(Volume.getMinData()), float(Volume.getMaxData()), float(Volume.getData(x, y, z)));
				
			//	S = pow(double(1 / e), double(1.0) / double(Volume.getGrad(x, y, z)));*/
			
				HSL2RGB(H, S, L, &temp1, &temp2, &temp3);
				/*temp1 = (sqrt(sqrt(temp1)));
				temp2 = (sqrt(sqrt(temp2)));
				temp3 = (sqrt(sqrt(temp3)));*/
			//	cout<<temp1<<"\t"<<temp2<<"\t"<<temp3<<endl;
			/*	temp1 = pow(double(temp1), double(0.3333333));
				temp2 = pow(double(temp2), double(0.3333333));
				temp3 = pow(double(temp3), double(0.3333333));*/
				temp1 *= 1.5;
				temp2 *= 1.5;
				temp3 *= 1.5;
				if(temp1 > 1.0)
					temp1 = 1.0;
				if(temp2 > 1.0)
					temp2 = 1.0;
				if(temp3 > 1.0)
					temp3 = 1.0;
				tf[index].r  = (unsigned char)(temp1 * 255);
			
				tf[index].g = (unsigned char)(temp2 * 255);
				tf[index].b = (unsigned char)(temp3 * 255);
			//	temp4 = norm(float(Volume.getMinGrad()), float(Volume.getMaxGrad()), float(Volume.getGrad(x, y, z)));
		/*		temp4 = 1.5 * temp4;
				if(temp4 > 1.0)
					temp4 = 1.0;*/
				/*	tf[index].r = 255 - tf[index].r;
					tf[index].g = 255 - tf[index].g;
					tf[index].b = 255 - tf[index].b;*/
			//	tf[index].a = (unsigned char)(temp4 * 255);
		//		if(Volume.getGrad(x, y, z) < 2000 || Volume.getDf2(x, y, z) < 00)
		//			tf[index].a = tf[index].r = tf[index].g = tf[index].b = 0;
		//	else
				{
					d = double(Volume.getData(x, y, z));
					g = double(Volume.getGrad(x, y, z));
					a = log(double(Volume.getX() + Volume.getY() + Volume.getZ()) / 3.0) / log(e);		
					temp4 = pow(double(1/ e),  d / g);
					alpha = temp4;
					alpha = 1.0 + 1 / a * log((1.0 - pow(e, -a)) * temp4 + pow(e, -a)) / log(e);
				//	temp4 = pow(double(1/ e), double(1.0 / float(Volume.getGrad(x, y, z))));
			//		temp4 = pow(e, - );
					tf[index].a =  unsigned char(alpha * 255);
				}
				
			}
}

#endif // Ben_imported_h