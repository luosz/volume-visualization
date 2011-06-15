#include <iostream>
#include "../my_raycasting/Fuzzy_CMeans.h"
#include "../my_raycasting/K_Means_PP_Generic.h"
using namespace std;

void main()
{
	const int count = 150;
	int k = 3;

	std::vector<nv::vec4f> v = clustering::Fuzzy_CMeans::get_data_from_file();

	unsigned char * label_ptr = new unsigned char[count];
	clustering::Fuzzy_CMeans::k_means(v, k, label_ptr);

	if (label_ptr != NULL)
	{
		cout<<endl;
		cout<<"fuzzy c-means"<<endl;
		for (int i=0; i<v.size(); i++)
		{
			cout<<(int)label_ptr[i]<<"\t";
		}
		cout<<endl;
	}

	delete [] label_ptr;

	label_ptr = new unsigned char[count];
	clustering::K_Means_PP_Generic::k_means(v, k, label_ptr, clustering::K_Means_PP_Generic::get_distance<nv::vec4f>, clustering::K_Means_PP_Generic::get_centroid<nv::vec4f>);

	if (label_ptr != NULL)
	{
		cout<<endl;
		cout<<"k-means++"<<endl;
		for (int i=0; i<v.size(); i++)
		{
			cout<<(int)label_ptr[i]<<"\t";
		}
		cout<<endl;
	}

	delete [] label_ptr;
}
