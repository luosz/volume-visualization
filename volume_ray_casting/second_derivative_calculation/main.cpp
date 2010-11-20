#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
using namespace std;

// NVIDIA OpenGL SDK
#include <nvGlutManipulators.h>
#include <nvGlutWidgets.h>

#include "../my_raycasting/filename_utility.h"
#include "../my_raycasting/volume_utility.h"
#include "../my_raycasting/reader.h"
#include "../my_raycasting/K_Means_PP_Generic.h"
using namespace reader;

char filename[MAX_STR_SIZE] = "D:\\_data\\data\\nucleon.dat";
int sizes[3];

template <class T, int TYPE_SIZE>
void k_means_on_derivatives(const T *data, const unsigned int count, const unsigned int components, int width, int height, int depth)
{
	unsigned int histogram[TYPE_SIZE] = {0};
	vector<float> scalar_value(count); // the scalar data in const T *data
	vector<nv::vec3f> gradient(count);
	vector<float> gradient_magnitude(count);
	vector<nv::vec3f> second_derivative(count);
	vector<float> second_derivative_magnitude(count);
	float max_gradient_magnitude, max_second_derivative_magnitude;

	// calculate scalar histogram, gradients and second derivatives
	std::cout<<"Scalar histogram..."<<std::endl;
	volume_utility::generate_scalar_histogram<T, TYPE_SIZE>(data, count, components, histogram, scalar_value);

	std::cout<<"Gradients and second derivatives..."<<std::endl;
	volume_utility::generate_gradient(sizes, count, components, scalar_value, gradient, gradient_magnitude, max_gradient_magnitude, second_derivative, second_derivative_magnitude, max_second_derivative_magnitude);

	char derivative_filename[MAX_STR_SIZE];
	sprintf(derivative_filename, "%s.derivatives.txt", filename);
	ofstream derivative_file(derivative_filename);
	for (unsigned int i=0; i<count; i++)
	{
		derivative_file<<scalar_value[i]<<","<<gradient_magnitude[i]<<","<<second_derivative_magnitude[i]<<endl;
	}
	derivative_file.close();

	// test the generic k-means
	nv::vec3f scalar_min, scalar_max;
	scalar_min.x = scalar_max.x = scalar_value[0];
	scalar_min.y = scalar_max.y = gradient_magnitude[0];
	scalar_min.z = scalar_max.z = second_derivative_magnitude[0];
	for (unsigned int i=1; i<count; i++)
	{
		if (scalar_value[i] < scalar_min.x)
		{
			scalar_min.x = scalar_value[i];
		}
		if (gradient_magnitude[i] < scalar_min.y)
		{
			scalar_min.y = gradient_magnitude[i];
		}
		if (second_derivative_magnitude[i] < scalar_min.z)
		{
			scalar_min.z = second_derivative_magnitude[i];
		}
		if (scalar_value[i] > scalar_max.x)
		{
			scalar_max.x = scalar_value[i];
		}
		if (gradient_magnitude[i] > scalar_max.y)
		{
			scalar_max.y = gradient_magnitude[i];
		}
		if (second_derivative_magnitude[i] > scalar_max.z)
		{
			scalar_max.z = second_derivative_magnitude[i];
		}
	}
	K_Means_PP_Generic::range_for_normalization = scalar_max - scalar_min;
	if (K_Means_PP_Generic::range_for_normalization.x < 1e-6 || K_Means_PP_Generic::range_for_normalization.y < 1e-6  || K_Means_PP_Generic::range_for_normalization.z < 1e-6)
	{
		std::cout<<"Range error!\t"<<K_Means_PP_Generic::range_for_normalization.x<<"\t"<<K_Means_PP_Generic::range_for_normalization.y<<"\t"<<K_Means_PP_Generic::range_for_normalization.z<<std::endl;
		std::cout<<"Range error!\t"<<scalar_max.x<<"\t"<<scalar_max.y<<"\t"<<scalar_max.z<<std::endl;
		std::cout<<"Range error!\t"<<scalar_min.x<<"\t"<<scalar_min.y<<"\t"<<scalar_min.z<<std::endl;
	}
	vector<nv::vec3f> scalar(count);
	for (unsigned int i=0; i<count; i++)
	{
		scalar[i].x = scalar_value[i];
		scalar[i].y = gradient_magnitude[i];
		scalar[i].z = second_derivative_magnitude[i];
	}

	const int k = 8;
	unsigned char * label_ptr = new unsigned char[count];
	K_Means_PP_Generic::k_means<nv::vec3f>(scalar, k, label_ptr, K_Means_PP_Generic::get_distance_normailzed, K_Means_PP_Generic::get_centroid<nv::vec3f>);

	// write labels to file
	char label_filename[MAX_STR_SIZE];
	sprintf(label_filename, "%s.%d.generic.txt", filename, k);
	std::cout<<"Write labels to file "<<label_filename<<std::endl;
	std::ofstream label_file(label_filename);
	for (unsigned int i=0; i<count; i++)
	{
		label_file<<std::hex<<(int)label_ptr[i];
	}
	label_file.close();

	// write data to file
	char data_filename[MAX_STR_SIZE];
	sprintf(data_filename, "%s.%d.clusters.txt", filename, k);
	std::cout<<"Write data to file "<<data_filename<<std::endl;
	std::ofstream data_file(data_filename);
	for (unsigned int i=0; i<count; i++)
	{
		data_file<<std::hex<<(int)label_ptr[i]<<"\t"<<scalar.at(i).x<<","<<scalar.at(i).y<<","<<scalar.at(i).z<<endl;
	}
	data_file.close();

	delete label_ptr;

	cout<<"k_means_on_derivatives is done."<<endl;
}

void main(int argc, char* argv[])
{
	// read filename from arguments if available
	if (argc > 1)
	{
		strcpy(filename, argv[1]);
	}
	cout<<"File "<<filename<<endl;

	float dists[3];
	int color_omponent_number;
	reader::DataType type;
	void ** data_ptr = new void *;
	if (!data_ptr)
	{
		data_ptr = new void *;
	}
	reader::readData(filename, sizes, dists, data_ptr, &type, &color_omponent_number);

	switch (type)
	{
	case reader::DATRAW_UCHAR:
		//gl_type = GL_UNSIGNED_BYTE;
		break;
	case reader::DATRAW_USHORT:
		//gl_type = GL_UNSIGNED_SHORT;
		break;
	default:
		std::cerr<<"Unsupported data type in "<<filename<<endl;
	}

	//GLenum gl_type;
	unsigned int count = sizes[0]*sizes[1]*sizes[2];
	if (type == reader::DATRAW_USHORT)
	{
		//cluster<unsigned short, 65536>((unsigned short*)*data_ptr, count);
		k_means_on_derivatives<unsigned short, 65536>((unsigned short*)*data_ptr, count, color_omponent_number, sizes[0], sizes[1], sizes[2]);
	}else
	{
		//cluster<unsigned char, 256>((unsigned char*)*data_ptr, count);
		k_means_on_derivatives<unsigned char, 256>((unsigned char*)*data_ptr, count, color_omponent_number, sizes[0], sizes[1], sizes[2]);
	}

	if (data_ptr)
	{
		free(*data_ptr);
		delete data_ptr;
		data_ptr = NULL;
	}
}
