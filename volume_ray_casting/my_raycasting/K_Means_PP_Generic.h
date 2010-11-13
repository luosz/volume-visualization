#pragma once

#ifndef K_Means_PP_Generic_h
#define K_Means_PP_Generic_h

#include <vector>
#include <nvMath.h>

namespace K_Means_PP_Generic
{
//#ifdef _DEBUG
//#define _DEBUG_OUTPUT
//#endif

	typedef float real; // This type definition make it flexible to switch between float and double

	real get_distance_real(const real & v1, const real & v2)
	{
		return abs(v1 - v2);
	}

	real get_distance_vec3f(const nv::vec3f & v1, const nv::vec3f & v2)
	{
		return nv::length(v1 - v2);
	}

	template <class T>
	real get_distance(const T & v1, const T & v2)
	{
		return nv::length(v1 - v2);
	}

	real get_centroid_real(const std::vector<real> & list)
	{
		real sum = 0;
		for (std::vector<real>::const_iterator i=list.begin(); i!=list.end(); i++)
		{
			sum += *i;
		}
		return (sum / list.size());
	}

	nv::vec3f get_centroid_vec3f(const std::vector<nv::vec3f> & list)
	{
		nv::vec3f sum(0, 0, 0);
		for (std::vector<nv::vec3f>::const_iterator i=list.begin(); i!=list.end(); i++)
		{
			sum += *i;
		}
		return (sum / list.size());
	}

	template <class T>
	T get_centroid(const std::vector<T> & list)
	{
		T sum;
		for (std::vector<T>::const_iterator i=list.begin(); i!=list.end(); i++)
		{
			if (i != list.begin())
			{
				sum += *i;
			}else
			{
				sum = *i;
			}
		}
		return (sum / list.size());
	}

	/************************************************************************
	A Tutorial on Clustering Algorithms
	http://home.dei.polimi.it/matteucc/Clustering/tutorial_html/kmeans.html

	K-means++
	http://en.wikipedia.org/wiki/K-means++
	************************************************************************/
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

		real distance;

		{
			// Make initial guesses for the means m1, m2, ..., mk
			// choose the first centroid at random

			srand((unsigned)time(NULL));
			int random = rand() % count;
			centroids->at(0) = data[random];

			// estimate the total cost
			real total_cost = 0;
			//float *distance_accumulation = new float[count];
			std::vector<real> distance_accumulation(count);
			for (unsigned int i=0; i<count; i++)
			{
				distance = get_distance(data[i], centroids->at(0));

				total_cost += distance;
				if (i > 0)
				{
					distance_accumulation[i] = distance_accumulation[i-1] + distance;
				}else
				{
					distance_accumulation[i] = distance;
				}
			}

			bool loop;
			real cutoff;

			// Repeatedly choose more centers
			for (int cluster_index = 1; cluster_index < k; cluster_index++)
			{
				loop = true;
				while (loop)
				{
					cutoff = (rand() / real(RAND_MAX)) * total_cost;

					for (unsigned int j = 0; j < count; j++)
					{
						if (distance_accumulation[j] >= cutoff)
						{
							random = j;
							loop = false;
							break;
						}
					}

				}
				centroids->at(cluster_index) = data[random];
			}

		} // // Make initial guesses for the means m1, m2, ..., mk


#ifdef _DEBUG_OUTPUT
		ofstream fc("D:\\K_Means_PP_Generic_centroids.txt", ios::out);
		int loop_count = 0;
		for (int i=0; i<k; i++)
		{
			fc<<loop_count<<"\t";
			fc<<i<<"\t";
			fc<<centroids->at(i).x<<","<<centroids->at(i).y<<","<<centroids->at(i).z<<"\t";
			fc<<endl;
		}
#endif

		const real epsilon = 1e-4;
		unsigned char centroids_index;
		real distance_temp;
		bool changed = true;

		// Until there are no changes in any mean
		while (changed)
		{
			// Empty the clusters before classification
			for (int i=0; i<k; i++)
			{
				if (clusters[i].size() > 0)
				{
					clusters[i].clear();
				}
			}

			// Use the estimated means to classify the samples into K clusters
			for (unsigned int i=0; i<count; i++)
			{
				// estimate the distance between points[i] and centroids[0]
				centroids_index = 0;
				distance = get_distance(data[i], centroids->at(0));

				// look for a smaller distance in the rest of centroids
				for (unsigned char j=1; j<k; j++)
				{
					distance_temp = get_distance(data[i], centroids->at(j));

					if (distance_temp < distance)
					{
						centroids_index = j;
						distance = distance_temp;
					}
				}
				label_ptr[i] = centroids_index;
				clusters[centroids_index].push_back(data[i]);
			}

#ifdef _DEBUG_OUTPUT
			loop_count++;
#endif

			// estimate the values of the new centers
			for (int i=0; i<k; i++)
			{
				if (clusters[i].size() > 0)
				{
					centroids_new->at(i) = get_centroid(clusters[i]);
				}else
				{
#ifdef _DEBUG_OUTPUT
					//fc<<"loop:"<<loop_count<<"\tno item in this cluster.\t"<<clusters[i].size()<<endl;
#endif
				}
#ifdef _DEBUG_OUTPUT
				fc<<loop_count<<"\t";
				fc<<i<<"\t";
				fc<<centroids_new->at(i).x<<","<<centroids_new->at(i).y<<","<<centroids_new->at(i).z<<"\t";
				fc<<endl;
#endif
			}

#ifdef _DEBUG_OUTPUT
#endif

			// the loop will continue if some centroids have changed
			changed = false;

			for (int i=0; i<k; i++)
			{
				distance_temp = get_distance(centroids->at(i), centroids_new->at(i));

				if (distance_temp > epsilon)
				{
					changed = true;
					break;
				}
			}

			// swap the new centroids with the old ones
			temp_ptr = centroids;
			centroids = centroids_new;
			centroids_new = temp_ptr;
		}

#ifdef _DEBUG_OUTPUT
		cout<<"loop count:"<<loop_count<<endl;
#endif

	}
}

#endif // K_Means_PP_Generic_h
