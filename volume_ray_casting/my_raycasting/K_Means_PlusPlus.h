#ifndef K_Means_PlusPlus_h
#define K_Means_PlusPlus_h

#include "kmpp/KMeansPP.h"

class K_Means_PlusPlus
{
public:
	static void k_means(const unsigned int count, const vector<float> & scalar_value, const vector<float> & gradient_magnitude, const vector<float> & second_derivative_magnitude, const int k, unsigned char *& label_ptr)
	{
		int n = (int)count, d = 3;
		Scalar *points = new Scalar[count*d];
		for (unsigned int i=0; i<count; i++)
		{
			points[i*d] = scalar_value[i];
			points[i*d + 1] = gradient_magnitude[i];
			points[i*d + 2] = second_derivative_magnitude[i];
		}
		int *assignments = new int[n];
		Scalar cost = RunKMeansPlusPlus(n, k, d, points, 1, NULL, assignments);
		delete [] points;

		for (unsigned int i=0; i<count; i++)
		{
			label_ptr[i] = static_cast<unsigned char>(assignments[i]);
		}
	}
};

#endif // K_Means_PlusPlus_h