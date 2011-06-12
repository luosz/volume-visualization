// DemoCluster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//#include "xClusterFilter.h"
// #pragma comment(lib, "LibCluster.lib")

#include <iostream>
#include <iomanip>
#include <fstream>
#include <ostream>
using namespace std;

#include "xFuzzyCMeans.h"
#include "xKMeans.h"

void output(int nCluster, CxFuzzyCMeans& filter)
{
//	cout<<"******************** Clustering results ***********************\n";

	for (int c=0; c<nCluster; ++c )
	{
		vector<double> center = filter.m_ArrayCluster.at(c).center;
		cout<<"Cluster " << c; 
		cout<<"\ncenter: ["<<center[0]<<", "<<center[1]<<"]\n";

		int nMember = (int)filter.m_ArrayCluster.at(c).member.size();
		cout<< nMember << " members: ";

		for (int j=0; j<nMember; ++j)
		{
			int id = filter.m_ArrayCluster.at(c).member.at(j);
			cout<< id <<" ";
		}
		cout<<"\n\n";
	}
	//cout<<"******************** End ***********************\n";
	cout<<"--------------------------------";
}

bool LoadData(CxFuzzyCMeans& filter)
{
	ifstream infile("iris.txt", ios::in);

	if ( !infile.is_open())        
	{ cout << "Error opening file"; return 0;}

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

int main(int argc, _TCHAR* argv[])
{
	const double EPISLON = 0.000001;
	const int nIter = 200;
	int nCluster = 3;

	int size = 18;

	CxFuzzyCMeans fcm(150, 4, 3);

	unsigned char * label_ptr = new unsigned char[150];
	fcm.m_label_ptr = label_ptr;

	if ( LoadData(fcm) == 0 ) return 0;

	fcm.InitClusters();
	fcm.Run(EPISLON);
	output(nCluster, fcm);

	if (fcm.m_label_ptr != NULL)
	{
		cout<<endl;
		for (int i=0; i<fcm.NumPatterns(); i++)
		{
			cout<<(int)fcm.m_label_ptr[i]<<"\t";
		}
		cout<<endl;
	}

	delete [] label_ptr;

//	CxKMeans kmeans(100, 3, 3);
//	kmeans.Run(EPISLON);



#if 0

	// STEP 1: Load data;

	CxClusterFilter filter;

	filter.m_nCluster = nCluster;
	if ( LoadData(filter) == 0 )
		return 0;

	// STEP 2: Run;
	cout<<"\nK Means:\n";
	double fit = 0;
//	fit = filter.Run(KMEANS, nIter);
//	output(nCluster, filter);
	fit = filter.Run(KMEANS, EPISLON);
   
	// STEP 3: Output;
	output(nCluster, filter);

#if 1
	// Fuzzy C-Means;
	cout<<"\nFuzzy C-Means:\n";
//	fit = filter.Run(FUZZY_CMEANS, nIter);
	fit = filter.Run(FUZZY_CMEANS, EPISLON);
	output(nCluster, filter);
#endif


	// 设置粒子的个数,
	filter.m_nParticle = 80;


#if 1
	// PSO Cluster;
	cout<<"\nPSO Cluster:\n";
//	fit = filter.Run(PSO_CLUSTER, nIter);
	fit = filter.Run(PSO_CLUSTER, EPISLON);
	output(nCluster, filter);
#endif

#if 1
	// PSO_KMeans;
	cout<<"\nPSO K Means:\n";
//	fit = filter.Run(PSO_KMEANS, nIter);
	fit = filter.Run(PSO_KMEANS, EPISLON);
	output(nCluster, filter);
#endif

#if 1
	// PSO_Fuzzy C-Means
	cout<<"\nPSO Fuzzy C-Means:\n";
//	fit = filter.Run(PSO_FUZZY_CMEANS, nIter);
//	output(nCluster, filter);

	fit = filter.Run(PSO_FUZZY_CMEANS, EPISLON);
	output(nCluster, filter);
#endif

#endif

	return 0;
}
