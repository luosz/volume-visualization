#ifndef K_Means_PP_DIY_h
#define K_Means_PP_DIY_h

namespace clustering
{
	/**	@brief	A class for k-means++ clustering
	*	This class is obsolete. Please use K_Means_PP_Generic for k-means++ clustering.
	* 
	* A Tutorial on Clustering Algorithms
	* http://home.dei.polimi.it/matteucc/Clustering/tutorial_html/kmeans.html
	* 
	* K-means++
	* http://en.wikipedia.org/wiki/K-means++
	*/
	class K_Means_PP
	{
	public:
		static void k_means(const unsigned int count, const vector<float> & scalar_value, const vector<float> & gradient_magnitude, const vector<float> & second_derivative_magnitude, const int k, unsigned char *& label_ptr)
		{
			const int D = 3;

			float *temp_ptr;
			float *centroids = new float[k * D];
			float *centroids_new = new float[k * D];
			int *centroids_quantity = new int[k];
			float distance, distance_new, x, y, z;

			// Make initial guesses for the means m1, m2, ..., mk
			// choose the first centroid at random

			srand((unsigned)time(NULL));
			int random = rand() % count;
			centroids[0] = scalar_value[random];
			centroids[1] = gradient_magnitude[random];
			centroids[2] = second_derivative_magnitude[random];

			// estimate the total cost
			float total_cost = 0;
			float *distance_accumulation = new float[count];
			for (unsigned int i=0; i<count; i++)
			{
				x = scalar_value[i] - centroids[0];
				y = gradient_magnitude[i] - centroids[1];
				z = second_derivative_magnitude[i] - centroids[2];
				distance_new = x * x + y * y + z * z;

				total_cost += distance_new;
				if (i > 0)
				{
					distance_accumulation[i] = distance_accumulation[i-1] + distance_new;
				}else
				{
					distance_accumulation[i] = distance_new;
				}
			}

			bool loop;
			float cutoff;

			// Repeatedly choose more centers
			for (int cluster_index = 1; cluster_index < k; cluster_index++)
			{
				loop = true;
				while (loop)
				{
					cutoff = (rand() / float(RAND_MAX)) * total_cost;

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
				centroids[cluster_index*D] = scalar_value[random];
				centroids[cluster_index*D+1] = gradient_magnitude[random];
				centroids[cluster_index*D+2] = second_derivative_magnitude[random];
			}

			delete [] distance_accumulation;
			distance_accumulation = NULL;

#ifdef _DEBUG_OUTPUT
			ofstream fc("D:\\K_Means_PP_DIY_centroids.txt", ios::out);
			for (int i=0; i<k; i++)
			{
				fc<<centroids[i*D]<<","<<centroids[i*D+1]<<","<<centroids[i*D+2]<<"\t";
			}
			fc<<endl;
			int loop_count = 0;
#endif

			const float epsilon = 1e-4;
			unsigned char centroids_index;
			bool changed = true;

			// Until there are no changes in any mean
			while (changed)
			{
				// Empty the clusters before classification
				memset(centroids_quantity, 0, k * sizeof(int));
				memset(centroids_new, 0, k * D * sizeof(float));

				//#ifdef OUTPUT
				//			cout<<"memset\t"<<centroids_quantity[0]<<"\t"<<centroids_new[0]<<endl;
				//#endif // OUTPUT

				// Use the estimated means to classify the samples into K clusters
				for (unsigned int i=0; i<count; i++)
				{
					// estimate the distance between points[i] and centroids[0]
					centroids_index = 0;
					x = scalar_value[i] - centroids[0];
					y = gradient_magnitude[i] - centroids[1];
					z = second_derivative_magnitude[i] - centroids[2];
					distance = x * x + y * y + z * z;

					// look for a smaller distance in the rest of centroids
					for (unsigned char j=1; j<k; j++)
					{
						x = scalar_value[i] - centroids[j*D];
						y = gradient_magnitude[i] - centroids[j*D+1];
						z = second_derivative_magnitude[i] - centroids[j*D+2];
						distance_new = x * x + y * y + z * z;

						if (distance_new < distance)
						{
							centroids_index = j;
							distance = distance_new;
						}
					}
					label_ptr[i] = centroids_index;
					centroids_quantity[centroids_index]++;
					centroids_new[centroids_index*D] += scalar_value[i];
					centroids_new[centroids_index*D+1] += gradient_magnitude[i];
					centroids_new[centroids_index*D+2] += second_derivative_magnitude[i];
				}

#ifdef _DEBUG_OUTPUT
				loop_count++;
				fc<<loop_count<<")\t";
#endif

				// estimate the values of the new centers
				for (int i=0; i<k; i++)
				{
					if (centroids_quantity[i] > 0)
					{
						centroids_new[i*D] /= centroids_quantity[i];
						centroids_new[i*D+1] /= centroids_quantity[i];
						centroids_new[i*D+2] /= centroids_quantity[i];
					}else
					{
#ifdef _DEBUG_OUTPUT
						fc<<"loop:"<<loop_count<<"\tno item in this cluster.\t"<<centroids_quantity[i]<<endl;
#endif
					}
#ifdef _DEBUG_OUTPUT
					fc<<centroids_new[i*D]<<","<<centroids_new[i*D+1]<<","<<centroids_new[i*D+2]<<"\t";
#endif
				}

#ifdef _DEBUG_OUTPUT
				fc<<endl;
#endif

				// the loop will continue if some centroids have changed
				changed = false;

				for (int i=0; i<k; i++)
				{
					distance_new
						= abs(centroids[i*D] - centroids_new[i*D])
						+ abs(centroids[i*D+1] - centroids_new[i*D+1])
						+ abs(centroids[i*D+2] - centroids_new[i*D+2]);

					if (distance_new > epsilon)
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

			delete [] centroids;
			delete [] centroids_new;
			delete [] centroids_quantity;
			centroids = centroids_new = NULL;
			centroids_quantity = NULL;

#ifdef _DEBUG_OUTPUT
			cout<<"loop count:"<<loop_count<<endl;
#endif

		}
	};

}

#endif // K_Means_PP_DIY_h
