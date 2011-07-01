/**	@file
* a header file for segmentation tags manipulation
*/

#ifndef tag_h
#define tag_h

#include <set>
#include <map>

namespace volume_utility
{
	/// normalize labels in the label volume
	template <class T, int TYPE_SIZE>
	void normalize_volume(T *data, const unsigned int count, const unsigned int components)
	{
		for (unsigned int j=0; j<components; j++)
		{
			std::set<T> data_set;
			for (unsigned int i=0; i<count; i++)
			{
				unsigned int index = i * components + j;
				if (data_set.find(data[index]) == data_set.end())
				{
					data_set.insert(data[index]);
				}
			}

			double increment = TYPE_SIZE / (double)data_set.size();
			double sum = 0;

			std::map<T, T> data_map;
			for (std::set<T>::iterator i=data_set.begin(); i!=data_set.end(); i++)
			{
				data_map[*i] = (T)sum;
				sum += increment;
			}

			for (unsigned int i=0; i<count; i++)
			{
				unsigned int index = i * components + j;
				data[index] = data_map[data[index]];
			}
		}
	}
}

#endif // tag_h
