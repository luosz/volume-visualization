#include "VolumeReader.h"
#include "LH_Histograms_Constructor.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <nvMath.h>
using namespace std;

//void generate_gradient(const unsigned int datalength,const unsigned int datawidth,const unsigned int dataheight, unsigned int *scalar_value, std::vector<nv::vec3f> &gradient, std::vector<float> &gradient_magnitude, float &max_gradient_magnitude, std::vector<nv::vec3f> &second_derivative, std::vector<float> &second_derivative_magnitude, float &max_second_derivative_magnitude)
//{
//
//	//std::cout<<"gradient vector address = "<<&gradient<<std::endl;
//	unsigned int index;
//	int boundary[3] = {datalength-1, datawidth-1, dataheight-1};
//	unsigned int width = datalength, height = datawidth, depth = dataheight;
//
//	max_second_derivative_magnitude = max_gradient_magnitude = -1;
//	for (int i=0; i</*datalength*/depth; i++)
//	{
//		for (int j=0; j</*datawidth*/height; j++)
//		{
//			for (int k=0; k</*dataheight*/width; k++)
//			{
//				index = ((i) * height + j) * width + k;
//				//index = k*datalength*datawidth + j*datalength +i;
//				//std::cout<<"index = "<<index<<std::endl;
//				if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
//				{
//					gradient_magnitude[index] = gradient[index].x = gradient[index].y = gradient[index].z = 0;
//				}else
//				{
//					gradient[index].x = scalar_value[((i + 1) * height + j) * width + k] - scalar_value[((i - 1) * height + j) * width + k];
//					gradient[index].y = scalar_value[((i) * height + j + 1) * width + k] - scalar_value[((i) * height + j - 1) * width + k];
//					gradient[index].z = scalar_value[((i) * height + j) * width + k + 1] - scalar_value[((i) * height + j) * width + k - 1];
//					gradient_magnitude[index] = nv::length(gradient[index]);
//					max_gradient_magnitude = std::max(gradient_magnitude[index], max_gradient_magnitude);
//				}
//			}
//		}
//	}
//
//	for (int i=0; i<depth; i++)
//	{
//		for (int j=0; j<height; j++)
//		{
//			for (int k=0; k<width; k++)
//			{
//				index = ((i) * height + j) * width + k;
//				if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
//				{
//					second_derivative_magnitude[index] = 0;
//				}else
//				{
//					second_derivative[index].x = gradient[((i + 1) * height + j) * width + k].x - gradient[((i - 1) * height + j) * width + k].x;
//					second_derivative[index].y = gradient[((i) * height + j + 1) * width + k].y - gradient[((i) * height + j - 1) * width + k].y;
//					second_derivative[index].z = gradient[((i) * height + j) * width + k + 1].z - gradient[((i) * height + j) * width + k - 1].z;
//					second_derivative_magnitude[index] = nv::length(second_derivative[index]);
//					max_second_derivative_magnitude = std::max(second_derivative_magnitude[index], max_second_derivative_magnitude);
//				}
//			}
//		}
//	}
//}

void generate_gradient(volume &vv,const unsigned int datalength,const unsigned int datawidth,const unsigned int dataheight, std::vector<nv::vec3f> &gradient, std::vector<float> &gradient_magnitude, float &max_gradient_magnitude, std::vector<nv::vec3f> &second_derivative, std::vector<float> &second_derivative_magnitude, float &max_second_derivative_magnitude)
{
	//std::cout<<"gradient vector address = "<<&gradient<<std::endl;
	unsigned int index;
	int boundary[3] = {datalength-1, datawidth-1, dataheight-1};
	//unsigned int width = datalength, height = datawidth, depth = dataheight;

	max_second_derivative_magnitude = max_gradient_magnitude = -1;
	for (int i=0; i<datalength/*depth*/; i++)
	{
		for (int j=0; j<datawidth/*height*/; j++)
		{
			for (int k=0; k<dataheight/*width*/; k++)
			{
				//index = ((i) * height + j) * width + k;
				index = vv.getIndex(i,j,k);
				//std::cout<<"index = "<<index<<std::endl;
				if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
				{
					gradient_magnitude[index] = gradient[index].x = gradient[index].y = gradient[index].z = 0;
				}else
				{
					
						gradient[index].x = (float)vv.getData(i+1,j,k) - vv.getData(i-1,j,k);
						gradient[index].y = (float)vv.getData(i,j+1,k) - vv.getData(i,j-1,k);
						gradient[index].z = (float)vv.getData(i,j,k+1) - vv.getData(i,j,k-1);
						gradient_magnitude[index] = nv::length(gradient[index]);
						max_gradient_magnitude = std::max(gradient_magnitude[index], max_gradient_magnitude);
				
				}
			}
		}
	}

	for (int i=0; i<datalength/*depth*/; i++)
	{
		for (int j=0; j<datawidth/*height*/; j++)
		{
			for (int k=0; k<dataheight/*width*/; k++)
			{
				//index = ((i) * height + j) * width + k;
				index = vv.getIndex(i,j,k);
				if (k==0 || j==0 || i==0 || k==boundary[0] || j==boundary[1] || i==boundary[2])
				{
					second_derivative_magnitude[index] = 0;
				}else
				{
					second_derivative[index].x = gradient[(k*datawidth + j)*datalength +i+1].x - gradient[(k*datawidth + j)*datalength +i-1].x;
					second_derivative[index].y = gradient[(k*datawidth + (j+1))*datalength +i].y - gradient[(k*datawidth + (j-1))*datalength +i].y;
					second_derivative[index].z = gradient[((k+1)*datawidth + j)*datalength +i].z - gradient[((k-1)*datawidth + j)*datalength +i].z;
					second_derivative_magnitude[index] = nv::length(second_derivative[index]);
					max_second_derivative_magnitude = std::max(second_derivative_magnitude[index], max_second_derivative_magnitude);
				}
			}
		}
	}
}

void main()
{

	VolumeReader volume;
	char *volume_filename = "../my_raycasting/data/nucleon.dat";
	volume.readVolFile(volume_filename);
	unsigned int count = volume.getCount();
	std::vector<nv::vec3f> gradient(count);
	std::vector<float> grandient_magnitude(count);
	std::vector<nv::vec3f> second_derivative(count);
	std::vector<float> second_derivative_magnitude(count);
	float maxGradientMagnitude;
	float maxSecondDerivativeMagnitude;
	generate_gradient(volume,volume.getX(),volume.getY(),volume.getZ(),gradient,grandient_magnitude,maxGradientMagnitude,second_derivative,second_derivative_magnitude,maxSecondDerivativeMagnitude);
	
	std::ofstream out("D:/result.txt");
	for(int i = 0; i <= volume.getX()-1; i++){
		for(int j = 0; j <= volume.getY()-1; j++ ){
			for(int k = 0; k <= volume.getZ()-1; k++){
				unsigned int index = volume.getIndex(i,j,k);
				out<<"voxel coordinate = ("<< i <<","<<j<<","<<k<<");\t";
				out<<"voxel Index = "<<index<<";\t";
				out<<"voxel data = "<<volume.getData(i,j,k)<<";\t";
				out<<"voxel gradient = ("<<gradient[index].x<<","<<gradient[index].y<<","<<gradient[index].z<<");\t";
				out<<"voxel gradient magnitude = "<<grandient_magnitude[index]<<";\t";
				out<<"voxel second derivative = ("<<second_derivative[index].x<<","<<second_derivative[index].y<<","<<second_derivative[index].z<<");\t";
				out<<"voxel second derivative magnitude = "<<second_derivative_magnitude[index]<<";\t"<<std::endl;
			}
		}
	}
	LH_Histograms LHHistograms = LH_Histograms();
	LHHistograms.constructor(volume,gradient,grandient_magnitude,second_derivative,second_derivative_magnitude);
	out.close();
	out.open("d:/FLandFH.txt");
	for(std::vector<std::pair<nv::vec3f,std::pair<float,float>>>::iterator it = LHHistograms.m_LHInformation.begin();it !=LHHistograms.m_LHInformation.end(); it++){
		out<<"coordinate = ("<<it->first.x<<","<<it->first.y<<","<<it->first.z<<")\t";
		out<<"FL and FH values = ("<<it->second.first<<","<<it->second.second<<")"<<std::endl;
	}
	//std::cout<<"Hello"<<std::endl;
}