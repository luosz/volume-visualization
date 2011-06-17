/**	@file
* a header file for segmentation tags manipulation
*/

#include <set>
#include <map>

namespace volume_utility
{
	/// normalize labels in the label volume
	template <class T, int TYPE_SIZE>
	void normalize_label(T * data_new, const T *data, const unsigned int count, const unsigned int components)
	{
		for (unsigned int j=0; j<components; j++)
		{
			std::set<T> set1;
			for (unsigned int i=0; i<count; i++)
			{
				unsigned int index = i * components + j;
				set1.insert(data[index]);
			}

			double increment = TYPE_SIZE / (double)set1.size();
			double sum = 0;

			std::map<T, T> map1;
			for (std::set<T>::iterator i=set1.begin(); i!=set1.end(); i++)
			{
				map1[*i] = (T)sum;
				sum += increment;
			}

			for (unsigned int i=0; i<count; i++)
			{
				unsigned int index = i * components + j;
				data_new[index] = map1[data[index]];
			}
		}
	}
}