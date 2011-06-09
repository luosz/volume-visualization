//  *****************************************************************
//	xKMeans       version: 1.0 	
//  ----------------------------------------------------------------
//  Purpose:    A simple implementation of K-Means Cluster Algorithm
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
#pragma once

#include "xCluster.h"

class CxKMeans:public CxCluster
{	
protected:

//	virtual void  DistributeSamples();    // Step 2 数据划分
	virtual bool  CalNewClusterCenters(); // Step 3 重新计算新的中心向量;
    virtual double CalFitCost();

public:

	CxKMeans(int nPattern, int nDim, int nCluster):
	         CxCluster(nPattern, nDim, nCluster){}

	~CxKMeans(void) {}
	
  	void   InitClusters();          // Step 1 随机初始化K个中心;

//	int    Run(double epislon = EPISLON);
//	double Run(int nIter);
	
};
// Note:
// K-Means聚类算法的结果受到聚类中心个数以及初始聚类中心的选择影响,
// 也收到样品几何性质及排列次序影响.如果样品的几何特性表明能形成几个
// 较远的小块孤立区域则算法多能收敛.
// end.