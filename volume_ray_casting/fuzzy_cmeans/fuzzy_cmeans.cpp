#include <iostream>
#include "../my_raycasting/Fuzzy_CMeans.h"
using namespace std;

void main()
{
	unsigned char * label_ptr = new unsigned char[150];
	std::vector<nv::vec4f> v = clustering::Fuzzy_CMeans::get_data_from_file();
	clustering::Fuzzy_CMeans::k_means(v, 3, label_ptr);
	delete [] label_ptr;
}