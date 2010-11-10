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
using namespace reader;

char filename[MAX_STR_SIZE] = "D:\\_data\\data\\nucleon.dat";
int sizes[3];

template <class T, int TYPE_SIZE>
void calculate_derivatives(const T *data, const unsigned int count, const unsigned int components, int width, int height, int depth)
{
	unsigned int histogram[TYPE_SIZE] = {0};
	vector<float> scalar_value(count); // the scalar data in const T *data
	vector<nv::vec3f> gradient(count);
	vector<float> gradient_magnitude(count);
	vector<nv::vec3f> second_derivative(count);
	vector<float> second_derivative_magnitude(count);
	float max_gradient_magnitude, max_second_derivative_magnitude;

	std::cout<<"Scalar histogram..."<<std::endl;
	volume_utility::generate_scalar_histogram<T, TYPE_SIZE>(data, count, components, histogram, scalar_value);

	std::cout<<"Gradients and second derivatives..."<<std::endl;
	volume_utility::generate_gradient(sizes, count, components, scalar_value, gradient, gradient_magnitude, max_gradient_magnitude, second_derivative, second_derivative_magnitude, max_second_derivative_magnitude);

	char derivative_filename[MAX_STR_SIZE];
	sprintf(derivative_filename, "%s.derivatives.txt", filename);
	ofstream out(derivative_filename);
	for (unsigned int i=0; i<count; i++)
	{
		out<<scalar_value[i]<<","<<gradient_magnitude[i]<<","<<second_derivative_magnitude[i]<<endl;
	}
	out.close();
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
		calculate_derivatives<unsigned short, 65536>((unsigned short*)*data_ptr, count, color_omponent_number, sizes[0], sizes[1], sizes[2]);
	}else
	{
		//cluster<unsigned char, 256>((unsigned char*)*data_ptr, count);
		calculate_derivatives<unsigned char, 256>((unsigned char*)*data_ptr, count, color_omponent_number, sizes[0], sizes[1], sizes[2]);
	}

	cout<<"Done."<<endl;

	if (data_ptr)
	{
		free(*data_ptr);
		delete data_ptr;
		data_ptr = NULL;
	}
}
