#ifndef vertical_vectors_h
#define vertical_vectors_h

#include <nvMath.h>

nv::vec3f get_vetical_vector(nv::vec3f v1)
{
	nv::vec3f v2;
	int count = 0;
	for (int i=0; i<v1.size(); i++)
	{
		if (v1[i] != 0)
		{
			count++;
		}
	}

	int index = -1;
	switch(count)
	{
	case 0: break;
	case 1:
		for (int i=0; i<v1.size(); i++)
		{
			if (v1[i] != 0)
			{
				v2[(i+1) % v1.size()] = v1[i];
				break;
			}
		}
		break;
	case 2:
		for (int i=0; i<v1.size(); i++)
		{
			if (v1[i] != 0)
			{
				if (index == -1)
				{
					index = i;
				}else
				{
					v2[i] = v1[index];
					v2[index] = v1[i];
					break;
				}
			}
		}
		break;
	default:
		v2.x = v2.y = v1.z;
		v2.z = - v1.x - v1.y;
	}

	return normalize(v2);
}

#endif //vertical_vectors_h