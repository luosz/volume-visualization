//  *****************************************************************
//	xKMeans       version: 1.0 	
//  ----------------------------------------------------------------
//  Purpose:    
//	Author:		JH Wu [ jh.wu@siat.ac.cn ]
//  Date:       2010/07/30  10:11
//	----------------------------------------------------------------
//	Copyright (c) 2010 - All Rights Reserved
//
//  Center for Human-Computer Interaction,
//  Shenzhen Institute of Advanced Integration Technology, 
//  Chinese Academy of Sciences/The Chinese University of Hong Kong
//  *****************************************************************
//
//  *****************************************************************
#include "StdAfx.h"
#include "xKMeans.h"

#include <cassert>
#include <cmath>
#include <limits>	//使用double 的最大值
using std::numeric_limits;	

#include "xRand.h"

Ran gRand2(5);


void CxKMeans::InitClusters()
{
	// 从样本中随机选择K个点作为K个聚类的中心点;
	int sizes = NumPatterns();
	assert ( sizes >= NumClusters() );

	for ( int i=0; i<NumClusters(); ++i )
	{
		int rand_id = gRand2.int32(0, NumPatterns()-1);
		const Pattern& p = m_ArrayPattern.at(rand_id);
		Cluster c; 
		c.center = p.pattern;
		c.member.push_back(rand_id);
		m_ArrayCluster[i] = c;
	}


	return ;
}

bool CxKMeans::CalNewClusterCenters()
{	
	int dim = (int)m_ArrayCluster.at(0).center.size();
	bool flag = true; // 是否收敛;

	for(int k=0; k<NumClusters(); ++k)
	{
		int size = (int)m_ArrayCluster.at(k).member.size();
		if ( size == 0 )
			continue;

		vector<double> center(dim, 0);
		for (int n=0; n<size; ++n)
		{
			for(int i=0; i<dim; ++i)
			{
				int id = m_ArrayCluster.at(k).member.at(n);
				const Pattern& pat = m_ArrayPattern.at(id); 
				center[i] += pat.pattern.at(i);
			}
		}

		for (int i=0; i<dim; ++i)
		{
			center.at(i) = center.at(i)/(double)size;

			// if ( center.at(i) != m_ArrayCluster.at(k).center.at(i))
			if ( fabs(center.at(i) - m_ArrayCluster.at(k).center.at(i)) > GetError() )
				flag = false;

			m_ArrayCluster.at(k).center.at(i) = center.at(i);
		}

	} // end for k;

	return flag;
}
double CxKMeans::CalFitCost()
{
	double fit = 0;
	int nClusters = NumClusters();
	for (int c=0; c<nClusters; ++c )
	{
		int nMembers = (int)m_ArrayCluster.at(c).member.size();
		double sum = 0.0;
		for( int i=0; i<nMembers; ++i )
		{
			sum += CalEuclideanNorm(c, m_ArrayCluster.at(c).member.at(i));
		}

		fit += sum;
	}

	return fit;
}

// int CxKMeans::Run(double epislon /*= EPISLON*/)
// {
// 	bool converged = false;
// 	int nIter = 0;
// 	while ( converged == false )
// 	{
// 		DistributeSamples();
// 		nIter++;
// 		converged = CalNewClusterCenters();
// 	}
// 
// 	return nIter;
// }

// double CxKMeans::Run(int nIter)
// {
// 	double fit = 0;
// 	for(int i=0; i<nIter; ++i)
// 	{
// 		DistributeSamples();
// 		CalNewClusterCenters();
// 	}
// 
// 	return fit;
// }