//  *************************************************************************
//	xFuzzyCMeans       version: 1.0 	
//  -------------------------------------------------------------------------
//  Purpose:    
//	Author:		JH Wu [ jh.wu@siat.ac.cn ]
//  Date:       2010/08/02  11:21
//	-------------------------------------------------------------------------
//	Copyright (c) 2010 - All Rights Reserved
//
//  Center for Human-Computer Interaction,
//  Shenzhen Institutes of Advanced Technology, Chinese Academy of Sciences
//  *************************************************************************
//
//  *************************************************************************
#include "StdAfx.h"
#include "xFuzzyCMeans.h"

#include <cassert>
#include <cmath>
#include <limits>	//使用double 的最大值
using std::numeric_limits;

#include <utility>
using std::pair;

#include "xRand.h"

Ran gRand(10);


void CxFuzzyCMeans::InitClusters()
{
	int sizes = NumPatterns();
	assert ( sizes >= NumClusters() );

	for ( int i=0; i<NumClusters(); ++i )
	{		
		int rad = gRand.int32(0, sizes-1); // 从样本中随机选择一个作为聚类中心;
		const Pattern& p = m_ArrayPattern.at(rad);

		Cluster c; 
		c.center = p.pattern;
		c.member.push_back(i);
		m_ArrayCluster[i] = c;
	}

	m_FuzzyMat.resize(sizes);

	return;
}

void CxFuzzyCMeans::InitClusters2()
{
	int size = (int)m_ArrayPattern.size();
	if ( size == 0) return;
	m_FuzzyMat.resize(size);
	
	for(int i=0; i<size; ++i)
	{
		vector<double> degree(NumClusters(), 0);
		for(int c=0; c<NumClusters(); ++c)
		{
			degree.at(c) = gRand.doub(0, 1); // 随机生成隶属度;
		}

		m_FuzzyMat.at(i) = degree;
	}

	// normalization;
	for(int i=0; i<size; ++i)
	{
		vector<double> degree = m_FuzzyMat.at(i);
		double sum = 0;

		for(int c=0; c<NumClusters(); ++c)
			sum += degree.at(c);
		
		for(int c=0; c<NumClusters(); ++c)
			degree.at(c) = degree.at(c)/sum;

		m_FuzzyMat.at(i) = degree;
	}

	// compute center;
	int dim = (int)m_ArrayPattern.at(0).pattern.size();
	m_ArrayCluster.resize(NumClusters());

	for(int c=0; c<NumClusters(); ++c)
	{
		double sum = 0;	
		vector<double> center(dim, 0);

		for(int i=0; i<size; ++i)
		{
			double u = m_FuzzyMat[i][c];
			u = pow(u, m_M);
			const Pattern& pat = m_ArrayPattern.at(i);
			for(int d=0; d<dim; ++d)
				center[d] += u * pat.pattern.at(d);
			sum += u;
		}

		for (int d=0; d<dim; ++d)
		{
			double t = center[d]/sum;
			m_ArrayCluster.at(c).center.push_back(t);
		}

	} // end for c;
	
}

void CxFuzzyCMeans::CalFuzzyMatrix()
{
	int size = (int)m_ArrayPattern.size();
	vector<double> degree(NumClusters(), 0.0);
    
	vector<pair<int, int> > tmp; // 记录那些距离为零;

	for(int i=0; i<size; ++i)
	{
		for(int c=0; c<NumClusters(); ++c)
		{
			double dist = CalEuclideanNorm(c, i);
			double sum = 0;
			for(int k=0; k<NumClusters(); ++k)
			{
				double dist2 = CalEuclideanNorm(k, i);
				double u = 0;
				if ( dist2 != 0 )
				{
					u = dist / dist2;
					u = pow(u, 2.0/(m_M-1));
				}
				else   // 这里是否有问题?? 
				{
					if ( c == k ) 
					{
						u = 1;
						tmp.push_back(std::make_pair(i, c));
					}
					else u = 0;
				}

				sum += u;
			}

			degree.at(c) = 1.0/sum;
		}

		m_FuzzyMat.at(i) = degree;

	} // end for i

	// 修正模糊矩阵,
	// 如果某点与中心的距离为零,则他完全属于这个聚类
	// 即属于这个类的隶属度为1, 其他为0;
	int size2 = (int)tmp.size();
	for(int i=0; i<size2; ++i)
	{
		pair<int,int>& p = tmp.at(i);
		for(int c=0; c<NumClusters(); ++c)
		{
			if ( p.second == c )
				m_FuzzyMat[p.first][c] = 1;
			else
				m_FuzzyMat[p.first][c] = 0;
		}
	}
}

bool CxFuzzyCMeans::CalNewClusterCenters()
{
	CalFuzzyMatrix();

	int size = (int)m_ArrayPattern.size();
	int dim = (int)m_ArrayCluster.at(0).center.size();
    bool convergence = true;
	for(int c=0; c<NumClusters(); ++c)
	{
		double sum = 0;	
		vector<double> center(dim, 0);

		for(int i=0; i<size; ++i)
		{
			double u = m_FuzzyMat[i][c];
			u = pow(u, m_M);
            const Pattern& pat = m_ArrayPattern.at(i);
            for(int d=0; d<dim; ++d)
				center[d] += u * pat.pattern.at(d);
			sum += u;
		}

		for (int d=0; d<dim; ++d)
		{
			double t = center[d]/sum;
			//if ( m_ArrayCluster.at(c).center[d] != t )
			if ( fabs(m_ArrayCluster.at(c).center[d] - t) > GetError())
				convergence = false;

			m_ArrayCluster.at(c).center[d] = t;
		}

	} // end for c;

	return convergence;
}

int CxFuzzyCMeans::FindMaxDegreeCluster(int p)
{
	double maxDegree = -1;
	int clusterID = -1;
	double degree;
	for (int c=0; c<NumClusters(); ++c)
	{
		degree = m_FuzzyMat.at(p).at(c);
		if ( degree > maxDegree )
		{
			maxDegree = degree;
			clusterID = c;
		}
	}

	assert ( clusterID >= 0 );
	return clusterID;
}


void CxFuzzyCMeans::DistributeSamples()
{
	//Clear membership list for all current clusters
	for (int c=0; c<NumClusters(); ++c)
		m_ArrayCluster.at(c).member.clear();

	int sizes = (int)m_ArrayPattern.size();
	for (int i=0; i<sizes; ++i )
	{
		// 最近邻原则
		// Find cluster center to which the pattern is closest
		// int id = FindClosestCluster(i);

		// 最大隶属度原则
		// Find cluster center to which the degree is maximum;
		int id = FindMaxDegreeCluster(i);

		//add this pattern to the cluster
		m_ArrayCluster.at(id).member.push_back(i);

		// set a cluster index in the label array
		if (m_label_ptr == NULL)
		{
			m_label_ptr[i] = (unsigned char)id;
		}
	}

}


double CxFuzzyCMeans::CalFitCost()
{
	double sum = 0;
	int size = (int)m_ArrayPattern.size();
	for(int i=0; i<size; ++i)
	{
		double total = 0;
		const Pattern& pat = m_ArrayPattern.at(i);
		for(int c=0; c<NumClusters(); ++c)
		{
			const vector<double>& center = m_ArrayCluster.at(c).center;
			double u = m_FuzzyMat.at(i).at(c);
			u = pow(u, m_M);
			double dist = CalEuclideanNorm(c, i);
			total += u*dist;
		}

		sum += total;
	}

	return sum;
}

double CxFuzzyCMeans::Run(double epsilon /* = EPISLON */)
{
	SetError(epsilon);

	bool converged = false;
	int nIter = 0;
	while ( converged == false )
	{
		nIter++;
		converged = CalNewClusterCenters();
		DistributeSamples();
	}

	return CalFitCost();
}

double CxFuzzyCMeans::Run(int nIter)
{
	double fit = 0;
	for(int i=0; i<nIter; ++i)
	{
		CalNewClusterCenters();
		DistributeSamples();
	}

	return CalFitCost();
}

