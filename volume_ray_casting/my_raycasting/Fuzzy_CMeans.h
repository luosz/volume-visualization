#ifndef FUZZY_CMEANS_H
#define FUZZY_CMEANS_H

#include <vector>
#include <nvMath.h>
#include <iostream>
#include <fstream>

#include "../DemoCluster/xFuzzyCMeans.h"
#include "../DemoCluster/xKMeans.h"

namespace clustering
{
	/**	@brief	An adapter class for xFuzzyCMeans
	*	
	*/
	class Fuzzy_CMeans
	{
		static int get_dimension(const nv::vec2f & v)
		{
			return 2;
		}

		static int get_dimension(const nv::vec3f & v)
		{
			return 3;
		}

		static int get_dimension(const nv::vec4f & v)
		{
			return 4;
		}

		static Pattern to_Pattern(const nv::vec2f & v)
		{
			Pattern p;
			p.pattern.push_back(v.x);
			p.pattern.push_back(v.y);
			return p;
		}

		static Pattern to_Pattern(const nv::vec3f & v)
		{
			Pattern p;
			p.pattern.push_back(v.x);
			p.pattern.push_back(v.y);
			p.pattern.push_back(v.z);
			return p;
		}

		static Pattern to_Pattern(const nv::vec4f & v)
		{
			Pattern p;
			p.pattern.push_back(v.x);
			p.pattern.push_back(v.y);
			p.pattern.push_back(v.z);
			p.pattern.push_back(v.w);
			return p;
		}

		static void output(int nCluster, CxFuzzyCMeans& filter)
		{
			//	cout<<"******************** Clustering results ***********************\n";

			for (int c=0; c<nCluster; ++c )
			{
				vector<double> center = filter.m_ArrayCluster.at(c).center;
				std::cout<<"Cluster " << c; 
				std::cout<<"\ncenter: ["<<center[0]<<", "<<center[1]<<"]\n";

				int nMember = (int)filter.m_ArrayCluster.at(c).member.size();
				std::cout<< nMember << " members: ";

				for (int j=0; j<nMember; ++j)
				{
					int id = filter.m_ArrayCluster.at(c).member.at(j);
					std::cout<< id <<" ";
				}
				std::cout<<"\n\n";
			}
			//cout<<"******************** End ***********************\n";
			std::cout<<"--------------------------------";
		}

		template <class T>
		static void LoadData(CxFuzzyCMeans& filter, const std::vector<T> & data)
		{
			for (const std::vector<T>::iterator i=data.begin(); i!=data.end(); i++)
			{
				filter.m_ArrayPattern.push_back(to_Pattern(*i));
			}
		}

		bool LoadData(CxFuzzyCMeans& filter)
		{
			std::ifstream infile("iris.txt", std::ios::in);

			if ( !infile.is_open())        
			{
				std::cout << "Error opening file";
				return 0;
			}

			int nDim = 4;
			while ( !infile.eof())
			{
				if( infile.fail() )  
					break;

				char buf[1024];
				infile.getline(buf, 1024);

				float a, b, c, d;
				int g;

				if(strlen(buf)<=0)
					continue;

				sscanf(buf, "%f %f %f %f %d", &a, &b, &c, &d, &g);
				Pattern p;
				p.pattern.push_back(a);
				p.pattern.push_back(b);
				p.pattern.push_back(c);
				p.pattern.push_back(d);
				filter.m_ArrayPattern.push_back(p);

			}
			infile.close();

			return 1;
		}

		template <class T>
		static void k_means(const std::vector<T> & data, const int k, unsigned char *& label_ptr)
		{
			const unsigned int count = data.size();

			std::vector<std::vector<T>> clusters(k);
			std::vector<T> centroids0(k);
			std::vector<T> centroids1(k);
			std::vector<T> * centroids = &centroids0;
			std::vector<T> * centroids_new = &centroids1;
			std::vector<T> * temp_ptr;

			const double EPISLON = 1e-4;

			int nCluster = k;

			int nDim = get_dimension(*data.begin());

			CxFuzzyCMeans fcm(count, nDim, nCluster);
			fcm.m_label_ptr = label_ptr;

			LoadData(fcm, data);

			fcm.InitClusters();
			fcm.Run(EPISLON);
			output(nCluster, fcm);

			if (fcm.m_label_ptr != NULL)
			{
				cout<<endl;
				for (int i=0; i<fcm.NumPatterns(); i++)
				{
					cout<<fcm.m_label_ptr[i]<<"\t";
				}
				cout<<endl;
			}
		}

	};

}

#endif