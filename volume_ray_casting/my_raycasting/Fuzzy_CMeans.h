#ifndef CLUSTER_ADAPTOR_H
#define CLUSTER_ADAPTOR_H

namespace clustering
{
	/**	@brief	An adapter class for xFuzzyCMeans
	*	
	*/
	template <class T>
	void k_means(const std::vector<T> & data, const int k, unsigned char *& label_ptr, real get_distance(const T & v1, const T & v2), T get_centroid(const std::vector<T> & list))
	{
		const unsigned int count = data.size();

		std::vector<std::vector<T>> clusters(k);
		std::vector<T> centroids0(k);
		std::vector<T> centroids1(k);
		std::vector<T> * centroids = &centroids0;
		std::vector<T> * centroids_new = &centroids1;
		std::vector<T> * temp_ptr;


	}

}

#endif