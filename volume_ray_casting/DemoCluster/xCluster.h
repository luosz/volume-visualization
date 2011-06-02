//  *************************************************************************
//	xCluster       version: 1.0 	
//  -------------------------------------------------------------------------
//  Purpose:    An abstract class of Cluster;
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
#pragma once

#include <vector>
using std::vector;
const double EPISLON = 0.000001;


struct Pattern  // 样本;
{
	vector<double> pattern;
};
 
struct Cluster  // 簇类;
{
	vector<double> center; // 簇类中心;
	vector<int>    member; // Index of data belonging to this cluster
};

class CxCluster
{
private:

	int m_nPattern;  // 样本总数；
	int m_nDim;      // 样本维数;
	int m_nCluster;  // 聚类个数;

	double m_Error;  // 中心向量误差;

protected:

	virtual void   DistributeSamples(); // 根据最近邻原则分配样本;
	virtual bool   CalNewClusterCenters() = 0; 
	virtual double CalFitCost() = 0; // 计算聚类准则函数代价;

	int    FindClosestCluster(int p); // return index of cluster closest to pattern p

	double CalEuclideanNorm(const vector<double>& p1, const vector<double>& p2); // 两点之间欧式距离;
	double CalEuclideanNorm(int c, int p);   // 样本到聚类中心距离,

	//最小化聚内距离,最大化聚间距离
	double CalFitCostMinMax(double w1 = 0.5, double w2 = 0.5);

public:

	CxCluster(int nPattern, // 样本总数;
		      int nDim,     // 样本维数;
			  int nCluster);// 聚类个数;

	virtual~CxCluster(void) {}

	vector<Pattern> m_ArrayPattern; // 存储待聚类样本;
	vector<Cluster> m_ArrayCluster; // 聚类结果;
	vector<int> m_ArrayLabel; // 所属分类的索引

	int GetDim()      { return m_nDim;     }
	int NumClusters() { return m_nCluster; }
	int NumPatterns() { return m_nPattern; }

	void SetError(double e) { m_Error = e; }
	double GetError() {return m_Error;     }

	virtual void  InitClusters(); // STEP 1;

	                                              // 聚类过程的收敛条件：
	virtual double Run(int nIter);                // 1.迭代次数, 返回聚类准则代价;
	virtual double Run(double epsilon = EPISLON); // 2.中心变化误差,返回聚类准则代价;

};
