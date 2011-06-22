//  *************************************************************************
//	xClusterFilter       version: 1.0 	
//  -------------------------------------------------------------------------
//  Purpose:    聚类算法库
//	Author:		JH Wu [ jh.wu@siat.ac.cn ; jianhuang.wu@gmail.com ]
//  Date:       2010/08/05  15:49
//	-------------------------------------------------------------------------
//	Copyright (c) 2010 - All Rights Reserved
//
//  Center for Human-Computer Interaction,
//  Shenzhen Institutes of Advanced Technology, Chinese Academy of Sciences
//  *************************************************************************
//
//  *************************************************************************
#pragma once



// The following ifdef block is the standard way of creating macros 
// which make exporting from a DLL simpler. All files within this DLL 
// are compiled with the FILTER_EXPORTS symbol defined on the command line. 
// this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include 
// this file see FILTER_API functions as being imported from a DLL, 
// whereas this DLL sees symbols defined with this macro as being exported.

#ifdef CLUSTER_FILTER_EXPORTS
#define WUJH_CLUSTER_FILTER_API __declspec(dllexport)
#else
#define WUJH_CLUSTER_FILTER_API __declspec(dllimport)
#endif


#include <vector>
using std::vector;
#pragma warning(push)
#pragma warning(disable:4251)


struct Sample  // 样本;
{
	vector<double> pattern;
};

struct Group  // 簇类;
{
	vector<double> center; // 簇类中心;
	vector<int>    member; // Index of data belonging to this cluster
};

const double cERROR = 0.000001;

enum {KMEANS = 0,      // K均值;
      FUZZY_CMEANS,    // 模糊C均值;
      PSO_CLUSTER,     // 粒子群聚类
      PSO_KMEANS,      // 基于粒子群优化的K均值;
      PSO_FUZZY_CMEANS // 基于粒子群优化的模糊C均值; 
     };

class WUJH_CLUSTER_FILTER_API CxClusterFilter
{
public:
	CxClusterFilter(void);
	~CxClusterFilter(void);

	int m_nCluster;   // 分成几类;

	int m_nParticle;  // 如果是选择后3种粒子群聚类算法,
	                  // 则需要设置粒子的个数;

	vector<Sample> m_ArraySample;  // 存储待分类的样本;
	vector<Group>  m_ArrayGroup;   // 存储分类结果;

	double Run(int nMethod = KMEANS, int nIter = 200);       // 返回聚类准则代价;
	double Run(int nMethod = KMEANS, double error = cERROR); // 返回聚类准则代价;

};

#pragma warning(pop)
