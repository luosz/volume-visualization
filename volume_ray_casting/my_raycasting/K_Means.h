/**	@file
* a header file for the K_Means class
*/

#ifndef K_Means_h
#define K_Means_h

namespace clustering
{
	/**	@brief	A class for k-means clustering
	*	This class is obsolete. Please use K_Means_PP_Generic for k-means++ clustering.
	* 
	* The K-means algorithm is composed of the following steps:
	* 
	* Place K points into the space represented by the objects that are being clustered. These points represent initial group centroids.
	* Assign each object to the group that has the closest centroid.
	* When all objects have been assigned, recalculate the positions of the K centroids.
	* Repeat Steps 2 and 3 until the centroids no longer move. This produces a separation of the objects into groups from which the metric to be minimized can be calculated.
	*/
	class K_Means
	{
	public:
		static void k_means(const unsigned int count, const vector<float> & scalar_value, const vector<float> & gradient_magnitude, const vector<float> & second_derivative_magnitude, const int k, unsigned char *& label_ptr)
		{
			// Put the scalar values, gradient magnitudes and second derivative magnitudes into the vector points 
			vector<nv::vec3f> points(count);
			for (unsigned int i=0; i<count; i++)
			{
				points[i].x = scalar_value[i];
				points[i].y = gradient_magnitude[i];
				points[i].z = second_derivative_magnitude[i];
			}

			vector<int> labels(count);
			vector<nv::vec3f> centroids(k);

#ifdef _DEBUG_OUTPUT
			ofstream fc("D:\\centroids.txt", ios::out);
#endif

			// Make initial guesses for the means m1, m2, ..., mk
			// Seed the random-number generator with the current time so that
			// the numbers will be different every time we run.
			srand((unsigned)time(NULL));
			for (int i=0; i<k; i++)
			{
				centroids[i] = points[rand() % count];

#ifdef _DEBUG_OUTPUT
				fc<<centroids[i].x<<","<<centroids[i].y<<","<<centroids[i].z<<"    ";
#endif

			}

#ifdef _DEBUG_OUTPUT
			fc<<endl;
#endif

			const float epsilon = numeric_limits<float>::epsilon() * 1e4; // const float epsilon = 0.001;
			cout<<epsilon<<endl;

			//int loop_count = 0;
			float distance, distance_temp;
			int label;
			nv::vec3f centroid_new;
			int size;
			vector<vector<int>> clusters(k);
			bool changed = true;

			// Until there are no changes in any mean
			while (changed)
			{
				//loop_count++;

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
					label = 0;
					//distance = abs(points[i].x - centroids[0].x)
					//	+ abs(points[i].y - centroids[0].y)
					//	+ abs(points[i].z - centroids[0].z);
					distance = length(points[i] - centroids[0]);

					// look for a smaller distance in the rest centroids
					for (int j=1; j<k; j++)
					{
						//distance_temp
						//	= abs(points[i].x - centroids[j].x)
						//	+ abs(points[i].y - centroids[j].y)
						//	+ abs(points[i].z - centroids[j].z);
						distance_temp = length(points[i] - centroids[j]);
						if (distance_temp < distance)
						{
							label = j;
						}
					}
					labels[i] = label;
					clusters[label].push_back(i);
				}

				changed = false;

				// Replace mi with the mean of all of the samples for cluster i
				for (int i=0; i<k; i++)
				{
					size = (int)clusters[i].size();
					if (size > 0)
					{
						centroid_new.x = centroid_new.y = centroid_new.z = 0;
						for (int j=0; j<size; j++)
						{
							centroid_new += points[clusters[i][j]];
						}
						centroid_new = centroid_new / size;
						//distance = abs(centroid_new.x - centroids[i].x) + abs(centroid_new.y - centroids[i].y) + abs(centroid_new.z - centroids[i].z);
						distance = length(centroid_new - centroids[i]);
						if (distance > epsilon)
						{
							changed = true;
							centroids[i] = centroid_new;
						}
					}

#ifdef _DEBUG_OUTPUT
					fc<<centroids[i].x<<","<<centroids[i].y<<","<<centroids[i].z<<"    ";
#endif

				}

#ifdef _DEBUG_OUTPUT
				fc<<endl;
#endif

			}
		}
	};

}

#endif // K_Means_h
