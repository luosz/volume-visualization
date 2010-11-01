#include "LH_Histograms_Constructor.h"
#include <iostream>
#include <algorithm>


void generate_gradient(const int datalength,const int datawidth,const int dataheight, unsigned int *scalar_value, vector<nv::vec3f> &gradient, vector<float> &gradient_magnitude, float &max_gradient_magnitude, vector<nv::vec3f> &second_derivative, vector<float> &second_derivative_magnitude, float &max_second_derivative_magnitude)
{
	unsigned int index;

	max_second_derivative_magnitude = max_gradient_magnitude = -1;
	for (int i=0; i< datalength; i++)
	{
		for (int j=0; j< datawidth; j++)
		{
			for (int k=0; k< dataheight; k++)
			{
				index = i * j * k;
				if (i == 0 || j == 0 || k == 0 || i == (datalength -1) || j == (datawidth-1) || k == (dataheight-1))
				{
					gradient_magnitude[index] = gradient[index].x = gradient[index].y = gradient[index].z = 0;
				}else
				{
					gradient[index].x = scalar_value[(i+1) * j * k] - scalar_value[(i-1) * j * k];
					gradient[index].y = scalar_value[i * (j+1) * k] - scalar_value[i * (j-1) * k];
					gradient[index].z = scalar_value[i * j * (k+1)] - scalar_value[i * j * (k-1)];
					gradient_magnitude[index] = length(gradient[index]);
					max_gradient_magnitude = std::max(gradient_magnitude[index], max_gradient_magnitude);
				}
			}
		}
	}

	for (int i=0; i< datalength; i++)
	{
		for (int j=0; j< datawidth; j++)
		{
			for (int k=0; k< dataheight; k++)
			{
				index = i * j * k;
				if  (i == 0 || j == 0 || k == 0 || i == (datalength -1) || j == (datawidth-1) || k == (dataheight-1))
				{
					second_derivative_magnitude[index] = 0;
				}else
				{
					second_derivative[index].x = gradient[(i+1) * j * k].x - gradient[(i-1) * j * k].x;
					second_derivative[index].y = gradient[i * (j+1) * k].y - gradient[i * (j-1) * k].y;
					second_derivative[index].z = gradient[i * j * (k+1)].z - gradient[i * j * (k-1)].z;
					second_derivative_magnitude[index] = length(second_derivative[index]);
					max_second_derivative_magnitude = std::max(second_derivative_magnitude[index], max_second_derivative_magnitude);
				}
			}
		}
	}
}

int main(){
	VolumeReader* volume = new VolumeReader();
	vector<nv::vec3f> gradient;
	vector<float> grandient_magnitude;
	vector<nv::vec3f> second_derivative;
	vector<float> second_derivative_magnitude;
	float maxGradientMagnitude;
	float maxSecondDerivativeMagnitude;
	volume->readVolume(volume_filename);
	generate_gradient(volume->length,volume->width,volume->height,(unsigned int *)volume->data,gradient,grandient_magnitude,maxGradientMagnitude,second_derivative,second_derivative_magnitude,maxSecondDerivativeMagnitude);
	LH_Histograms LHHistograms = LH_Histograms(volume);
	LHHistograms.constructor(gradient,grandient_magnitude,second_derivative,second_derivative_magnitude);
	return 0;
}