//  *************************************************************************
//	xCluster       version: 1.0 	
//  -------------------------------------------------------------------------
//  Purpose:    
//	Author:		JH Wu [ jh.wu@siat.ac.cn ]
//  Date:       2010/08/03  11:22
//	-------------------------------------------------------------------------
//	Copyright (c) 2010 - All Rights Reserved
//
//  Center for Human-Computer Interaction,
//  Shenzhen Institutes of Advanced Technology, Chinese Academy of Sciences
//  *************************************************************************
//
//  *************************************************************************
#include "StdAfx.h"
#include "xCluster.h"


#include <cassert>
#include <cmath>
#include <limits>	//使用double 的最大值
using std::numeric_limits;	

#ifdef max
#undef max
#endif


CxCluster::CxCluster(int nPattern, int nDim, int nCluster)
{
	m_nPattern = nPattern;
	m_nDim = nDim;
	m_nCluster = nCluster;

	m_ArrayPattern.reserve(nPattern);
	m_ArrayCluster.resize(nCluster);
	m_ArrayLabel.resize(nPattern);

	m_Error = EPISLON;

}

double CxCluster::CalEuclideanNorm(const vector<double> &p1, const vector<double> &p2)
{
	assert( p1.size() == p2.size() );

	int dim = (int)p1.size();
	double sum = 0.0;

	for(int d=0; d<dim; ++d)
		sum += (p1.at(d) - p2.at(d))*(p1.at(d) - p2.at(d));

	return sqrt(sum);
}

double CxCluster::CalEuclideanNorm(int c, // 聚类中心索引
								   int p) // 样本索引;
{
	const vector<double>& center = m_ArrayCluster.at(c).center;
	const vector<double>& sample = m_ArrayPattern.at(p).pattern;

	return CalEuclideanNorm(center, sample);
}

// 返回离样本最近的聚类索引; 最近邻原则;
int CxCluster::FindClosestCluster(int p)
{
	double minDist = numeric_limits<double>::max();
	int clusterID = -1;
	double dist;

	for ( int c=0; c<NumClusters(); ++c )
	{
		dist = CalEuclideanNorm(c, p);
		if ( dist < minDist )
		{
			minDist = dist;
			clusterID = c;
		}
	}

	assert ( clusterID >= 0 );

	return clusterID;
}

void CxCluster::InitClusters()
{
	// 随机选择K个点作为K个聚类的中心点;
	// 这里选择[0, k-1]前K个数据作为中心点;
	int sizes = NumPatterns();
	assert ( sizes >= NumClusters() );

	for ( int i=0; i<NumClusters(); ++i )
	{
		const Pattern& p = m_ArrayPattern.at(i);
		Cluster c; 
		c.center = p.pattern;
		c.member.push_back(i);
		m_ArrayCluster[i] = c;
	}

	return ;
}

void CxCluster::DistributeSamples()
{

	//Clear membership list for all current clusters
	for (int k=0; k<NumClusters(); ++k)
	{
		m_ArrayCluster.at(k).member.clear();
	}

	int sizes = (int)m_ArrayPattern.size();
	for (int i=0; i<sizes; ++i )
	{
		//Find cluster center to which the pattern is closest
		int id = FindClosestCluster(i);

		//add this pattern to the cluster
		m_ArrayCluster.at(id).member.push_back(i);

	}
}


double CxCluster::Run(int nIter)
{
	for(int i=0; i<nIter; ++i)
	{
		DistributeSamples();
		CalNewClusterCenters();
	}

	return CalFitCost();
}

double CxCluster::Run(double epsilon)
{
	SetError(epsilon);

	bool converged = false;
	int nIter = 0;


	while ( converged == false )
	{
		DistributeSamples();
		nIter++;
		converged = CalNewClusterCenters();
	}

	//return nIter;
	return CalFitCost();
}

double CxCluster::CalFitCostMinMax(double w1 /* = 0.5 */, double w2 /* = 0.5 */)
{
	double fit = 0.0;
	double maxdist = -1;
	for (int c=0; c<NumClusters(); ++c)
	{
		const vector<int>& member = m_ArrayCluster.at(c).member;
		int n = (int)member.size();

		if ( n == 0 ) {continue;}

		double sum = 0.0;
		for(int i=0; i<n; ++i)
		{
			int p = member.at(i);
			double dist = CalEuclideanNorm(c, p);
			sum += dist;
		}

		sum /= n;
		
		if ( maxdist < sum ) 
			maxdist = sum;

	}

	double mindist = numeric_limits<double>::max();
	for (int c=0; c<NumClusters(); ++c)
	{
		for(int k=0; k<NumClusters(); ++k)
		{
			if ( c != k )
			{
				const vector<double>& center1 = m_ArrayCluster.at(c).center;
				const vector<double>& center2 = m_ArrayCluster.at(k).center;
				double dist = CalEuclideanNorm(center1, center2);
				if ( mindist > dist )
					mindist = dist;
			}
		}
	}

	// 最小化聚类内部距离,最大化类间距离;
	fit = w1*maxdist - w2*mindist;

	return fit;
}