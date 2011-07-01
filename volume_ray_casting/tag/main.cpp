#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <set>
#include <ctime>
#include <cstdio>
using namespace std;

#include "../my_raycasting/filename_utility.h"
#include "../raycasting_with_tags/reader_tag.h"
#include "../raycasting_with_tags/tag.h"

void main(int argc, char **argv)
{
	char filename[MAX_STR_SIZE];

	// print about information
	filename_utility::print_about(argc, argv);

	// get volume filename from arguments or console input
	filename_utility::get_filename(argc, argv, filename);

	cout<<filename<<endl;

	/// call finailize() to free the memory before exit
	void ** data_ptr = NULL;
	GLenum gl_type;
	int sizes[3];
	int color_component_number;

	float dists[3];
	file_utility::DataType type;

	if (!data_ptr)
	{
		data_ptr = new void *;
	}
	char tag_filename[MAX_STR_SIZE] = "";
	file_utility::readData_with_tag(filename, sizes, dists, data_ptr, &type, &color_component_number, tag_filename);

	switch (type)
	{
	case file_utility::DATRAW_UCHAR:
		gl_type = GL_UNSIGNED_BYTE;
		break;
	case file_utility::DATRAW_USHORT:
		gl_type = GL_UNSIGNED_SHORT;
		break;
	default:
		std::cerr<<"Unsupported data type in "<<filename<<endl;
	}

	cout<<"size:"<<sizes[0]<<"\t"<<sizes[1]<<"\t"<<sizes[2]<<endl;

	// load tags volume as texture
	if (strlen(tag_filename) > 0)
	{
		char tag_filename_with_path[MAX_STR_SIZE];
		filename_utility::get_path_from_other_filename(filename, tag_filename, tag_filename_with_path);
		std::cout<<"tag\t"<<tag_filename<<endl;
		std::cout<<"tag\t"<<tag_filename_with_path<<endl;
		int tag_sizes[3];
		float tag_dists[3];
		void ** tag_data_ptr = new void *;
		file_utility::DataType tag_type;
		int tag_color_component_number;
		file_utility::readData(tag_filename_with_path, tag_sizes, tag_dists, tag_data_ptr, &tag_type, &tag_color_component_number);
		std::cout<<"tag_sizes:"<<tag_sizes[0]<<"\t"<<tag_sizes[1]<<"\t"<<tag_sizes[2]<<"\n"<<tag_dists[0]<<"\t"<<tag_dists[1]<<"\t"<<tag_dists[2]<<"\n"<<tag_color_component_number<<endl;

		unsigned int count = tag_sizes[0] * tag_sizes[1] * tag_sizes[2];
		GLenum tag_gl_type;
		switch (tag_type)
		{
		case file_utility::DATRAW_UCHAR:
			tag_gl_type = GL_UNSIGNED_BYTE;
			volume_utility::normalize_volume<unsigned char, 256>((unsigned char*)*tag_data_ptr, count, tag_color_component_number);
			break;
		case file_utility::DATRAW_USHORT:
			tag_gl_type = GL_UNSIGNED_SHORT;
			volume_utility::normalize_volume<unsigned short, 65536>((unsigned short*)*tag_data_ptr, count, tag_color_component_number);
			break;
		default:
			std::cerr<<"Unsupported data type in "<<filename<<endl;
		}
	}

	//srand(time(0));
	//set<unsigned char> s;
	//for (unsigned int i=0; i<6553600; i++)
	//{
	//	unsigned char c = (unsigned char)rand() % 256;
	//	if (s.find(c) == s.end())
	//	{
	//		cout<<(int)c<<"\t";
	//		s.insert(c);
	//	}
	//}
	//cout<<endl;
	//cout<<s.size()<<endl;
}
