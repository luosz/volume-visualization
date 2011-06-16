/**	@file
* a header file for k-means clustering
*/

#ifndef K_MEANS
#define K_MEANS
#include <iostream>
#include <cmath>
#include "Volume.h"

using namespace std;

const int NUM = 6;  
const double epsilon_k_means = 1e-3;

typedef struct k_means_grid
{
	double center_x, center_y, center_z;
	int intensity;
	int gradient;
	int df2;
	int df3;
	double average_data;
	double average_grad;
	double average_df2;
	double average_df3;
}k_means_grid;

void k_means(Volume   * v,  char * lable)
{
	int i, index, x, y, z;
	
	int dim_x, dim_y, dim_z;
	int temp_x, temp_y, temp_z;
	int temp_data, temp_grad, temp_df2, temp_df3;
	int distance;
	double min;
	int j;
	k_means_grid k_grid[NUM];
	
	dim_x = v->getX();
	dim_y = v->getY();
	dim_z = v->getZ();

		for(x = 0; x < dim_x; ++x)
			for(y = 0; y < dim_y; ++y)
				for(z = 0; z < dim_z; ++z)
				{
					index = v->getIndex(x, y, z);
					if(v->getData(x, y, z) == v->getMaxData())
					{
						k_grid[0].center_x = x;
						k_grid[0].center_y = y;
						k_grid[0].center_z = z;
						k_grid[0].intensity = k_grid[0].average_data = v->getData(x, y, z) ;
						k_grid[0].gradient = k_grid[0].average_grad =  v->getGrad(x, y, z);
						k_grid[0].df2 = k_grid[0].average_df2 = v->getDf2(x, y, z);
						k_grid[0].df3 = k_grid[0].average_df3 = v->getDf3(x, y, z);
						lable[index] = 0;
					}
					else if(v->getData(x, y, z ) == v->getMinData())
					{
						k_grid[1].center_x = x;
						k_grid[1].center_y = y;
						k_grid[1].center_z = z;
						k_grid[1].intensity = k_grid[1].average_data = v->getData(x, y, z);
						k_grid[1].gradient = k_grid[1].average_grad = v->getGrad(x, y, z);
						k_grid[1].df2 = k_grid[1].average_df2 = v->getDf2(x, y, z);
						k_grid[1].df3 = k_grid[1].average_df3 = v->getDf3(x, y, z);
						lable[index] = 1;
					}
					else if(v->getGrad(x, y, z) == v->getMaxGrad())
					{
						k_grid[2].center_x = x;
						k_grid[2].center_y = y;
						k_grid[2].center_z = z;
						k_grid[2].intensity = k_grid[2].average_data = v->getData(x, y, z);
						k_grid[2].gradient = k_grid[2].average_grad = v->getGrad(x, y, z);
						k_grid[2].df2 = k_grid[2].average_df2 = v->getDf2(x, y, z);
						k_grid[2].df3 = k_grid[2].average_df3 = v->getDf3(x, y, z);
						lable[index] = 2;
					}
					else if(v->getGrad(x, y, z) == v->getMinGrad())
					{
						k_grid[3].center_x = x;
						k_grid[3].center_y = y;
						k_grid[3].center_z = z;
						k_grid[3].intensity = k_grid[3].average_data = v->getData(x, y, z);
						k_grid[3].gradient = k_grid[3].average_grad = v->getGrad(x, y, z);
						k_grid[3].df2 = k_grid[3].average_df2 = v->getDf2(x, y, z);
						k_grid[3].df3 = k_grid[3].average_df3 = v->getDf3(x, y, z);
						lable[index] = 3;
					}
					else if(v->getDf2(x, y, z) == v->getMaxDf2())
					{
			//			cout<<"ok"<<endl;
		//				cout<<v->getDf2(x, y, z)<<endl;
						k_grid[4].center_x = x;
						k_grid[4].center_y = y;
						k_grid[4].center_z = z;
						k_grid[4].intensity = k_grid[4].average_data = v->getData(x, y, z);
						k_grid[4].gradient = k_grid[4].average_grad = v->getGrad(x, y, z);
						k_grid[4].df2 = k_grid[4].average_df2 = v->getDf2(x, y, z);
						k_grid[4].df3 = k_grid[4].average_df3 = v->getDf3(x, y, z);
						lable[index] = 4;
					}
					else if(v->getDf3(x, y, z) == v->getMaxDf3())
					{
		//				cout<<"ok"<<endl;
		//				cout<<v->getDf3(x,y, z)<<endl;
						k_grid[5].center_x = x;
						k_grid[5].center_y = y;
						k_grid[5].center_z = z;
						k_grid[5].intensity = k_grid[5].average_data = v->getData(x, y, z);
						k_grid[5].gradient = k_grid[5].average_grad = v->getGrad(x, y, z);
						k_grid[5].df2 = k_grid[5].average_df2 = v->getDf2(x, y, z);
						k_grid[5].df3 = k_grid[5].average_df3 = v->getDf3(x, y, z);
						lable[index] = 5;
					}
					else 
						;
				}

			for(i = 0;i < 6; ++ i)
				std::cout<<k_grid[i].center_x << ",  "<<k_grid[i].center_y <<",   "<<k_grid[i].center_z<<endl;
				for(x = 0; x < dim_x; ++x)
					for(y = 0; y < dim_y; ++y)
						for(z = 0; z < dim_z; ++z)
						{
							min = 1e10;
							for(i = 0; i < NUM;++i)
							{
								temp_data = v->getData(x, y, z);
								temp_grad = v->getGrad(x, y, z);
								temp_df2 = v->getDf2(x, y, z);
								temp_df3 = v->getDf3(x, y, z);
								distance = pow(double(temp_data - k_grid[i].average_data), 2.0)
												+ pow(double(temp_grad - k_grid[i].average_grad), 2.0)
												+ pow(double(temp_df2 - k_grid[i].average_df2), 2.0)
												+ pow(double(temp_df3 - k_grid[i].average_df3), 2.0);
								distance = pow(distance, 0.5);
								if(distance < min)
								{
									min = distance;
									j = i;
								}
							}
							lable[v->getIndex(x, y , z)] = j;
						}
			for(x = 0; x < dim_x; ++x)
				for(y = 0; y < dim_y; ++y)
					for(z = 0; z  < dim_z; ++z)
					{
		//				cout<<"x = "<<x<<", y = "<<y<<", z = "<<z<<", label = "<<(int)lable[v->getIndex(x,y , z)]<<endl;
					}
}

#endif