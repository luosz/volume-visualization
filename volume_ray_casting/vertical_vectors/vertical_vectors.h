#ifndef vertical_vectors_h
#define vertical_vectors_h

#include <nvMath.h>

nv::vec3f get_vetical_vector(nv::vec3f v1)
{
	nv::vec3f v2(0,0,0);
	int count = 0, index1 = -1, index2 = -1;
	for (int i=0; i<v1.size(); i++)
	{
		if (v1[i] != 0)
		{
			count++;
			if (index1 == -1)
			{
				index1 = i;
			}else
			{
				index2 = i;
			}
		}
	}

	// get a vector v2 that is vertical to v1
	switch(count)
	{
	case 0: break;
	case 1:
		v2[(index1+1) % v1.size()] = v1[index1];
		break;
	case 2:
		v2[index1] = v1[index2];
		v2[index2] = -v1[index1];
		break;
	default:
		v2.x = v2.y = v1.z;
		v2.z = - v1.x - v1.y;
	}

	return v2;
}

#endif //vertical_vectors_h