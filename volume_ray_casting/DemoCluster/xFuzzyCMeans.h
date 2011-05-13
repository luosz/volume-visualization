//  *************************************************************************
//	xFuzzyCMeans       version: 1.0 	
//  -------------------------------------------------------------------------
//  Purpose:    Fuzzy C-Means Cluster
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
#pragma once

#include "xCluster.h"

class CxFuzzyCMeans:public CxCluster
{
private:

	int                     m_M;        // 模糊指数;
	vector<vector<double> > m_FuzzyMat; // 模糊矩阵;

protected:

	void   DistributeSamples();     
	bool   CalNewClusterCenters(); 
    int    FindMaxDegreeCluster(int p); // 返回最大隶属度所对应的聚类索引;

	double CalFitCost();     // 计算目标函数
	void   CalFuzzyMatrix(); // 计算模糊矩阵;

public:

	CxFuzzyCMeans(int nPattern, int nDim, int nCluster):
	             CxCluster(nPattern, nDim, nCluster){m_M = 2;}

	CxFuzzyCMeans(int nCluster);
	~CxFuzzyCMeans(void) {}
	
	void SetM(int m)  { m_M = m;  }
	int  GetM()       {return m_M;}

	// 初始化有两种,
	void InitClusters(); // 第一种是从样本随机选择K个样本作为初始中心;FCM-V
	void InitClusters2();// 第二种是随机生成每个样本的隶属度,然后计算其中心; FCM-U

 	virtual double Run(double epsilon = EPISLON);
	virtual double Run(int nIter);


};

// NOTE:
// 模糊C均值的主要缺点是对初始值敏感,算法容易收敛到局部解.
// 对噪声数据敏感,模糊系数难确定.
