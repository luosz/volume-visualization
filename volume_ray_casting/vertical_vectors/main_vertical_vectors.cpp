#include <iostream>
using namespace std;
#include "vertical_vectors.h"

void main()
{
	nv::vec3f v1, v2, v3;
	do 
	{
		cout<<"Input a 3D vector (separate by space)"<<endl;
		cin>>v1.x>>v1.y>>v1.z;
		v2 = get_vetical_vector(v1);
		v3 = normalize(cross(v1, v2));
		v2 = normalize(v2);
		cout<<v2.x<<"\t"<<v2.y<<"\t"<<v2.z<<"\n"<<v3.x<<"\t"<<v3.y<<"\t"<<v3.z<<endl;
	} while (length(v2) != 0);
}