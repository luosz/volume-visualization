#ifndef _VOLUME_
#define _VOLUME_

#include <cstdio>
#include <cstdlib>
#include <cmath>



typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
}Color;

typedef struct
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
}grid;

typedef struct
{
	unsigned short FL;
	unsigned short FH;
}LH;

class Volume
{
protected:
	unsigned int length;               //体数据长度
	unsigned int width;                //体数据宽度
	unsigned int height;               //体数??高度
	unsigned int count;                  //体素数
	char format[50];                     //数据格式
	void * data;   
	unsigned int range;                  //灰度级
	unsigned int * histogram;            //灰度直方图
	unsigned short dataTypeSize;         //数据类型大小
	unsigned int * gradient;             //梯度幅值
	unsigned int * df2;
	unsigned int * df3;
	Color * color;
	float * opacity;
	float ex, var, cv;       //cv 变异系数
	unsigned char * group;             
	bool * tag;
	unsigned int threshold;
	unsigned int max_data, min_data;
	unsigned int max_grad, max_df2, max_df3, min_grad, min_df2, min_df3, max_frequency;
	float max_ep, min_ep;
	float * ep, little_epsilon;
	float * acc_distribution;
	int intensity_gradient_histogram[12][12];
	float spatial_distribution[12][12];
	LH * LH_Histogram;
public:
	Volume()
	{
		length = width = height = count = range = dataTypeSize = max_data = min_data = threshold = 0;
		max_frequency = 0;
		max_grad =  min_grad = 0;
		min_df2 = min_df3 = max_ep = min_ep = 0;
		data = NULL;
		histogram = NULL;
		color = NULL;
		opacity = NULL;
		group = NULL;
		tag = NULL;
		gradient = NULL;
		df2 = NULL;
		df3 = NULL;
		little_epsilon = 10;
	}
	virtual ~Volume()
	{
		if(data)
			free(data);
		if(histogram)
			free(histogram);
		if(gradient)
			free(gradient);
		if(opacity)
			free(opacity);
		if(group)
			free(group);
		if(tag)	
			free(tag);
		if(df2)
			free(df2);
		if(df3)
			free(df3);
	}
	virtual bool readVolFile(char * s);
	bool readData(char * s);
	void traverse();
	void calHistogram();
	void calDf2();
	void calDf3();
	void calGradient();
	void calGrad();
	void calGrad_ex();                   //use f(x) = a * exp(bx)
	void calEp();
	void calLH();
	void Intensity_gradient_histogram();
	virtual unsigned int getData(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getGrad(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getDf2(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getDf3(unsigned int x, unsigned int y, unsigned int z);
	float   getEp(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getIndex(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getX(void);
	unsigned int getY(void);
	unsigned int getZ(void);
	unsigned int getCount(void);
	unsigned int getMaxData(void);
	unsigned int getMinData(void);
	unsigned int getMaxGrad(void);
	unsigned int getMinGrad(void);
	unsigned int getMaxDf2(void);
	unsigned int getMinDf2(void);
	unsigned int getMaxDf3(void);
	unsigned int getMinDf3(void);
	unsigned int getRange(void);
	unsigned int getHistogram(unsigned int intensity);
	float getSpatialDistribution(int i, int j);
	unsigned int getIntensity_gradient_histogram(int i, int j);
	unsigned int getMaxFrequency();
	char * getFormat(void);
	void * getDataAddr(void);
	bool allTraverse(bool * tag);
	void setTag(unsigned int x, unsigned int y, unsigned int z);
	bool getTag(unsigned int x, unsigned int y, unsigned int z);
	void setGroup(unsigned int x, unsigned int y, unsigned int z, unsigned int id);
	unsigned int getGroup(unsigned int x, unsigned int y, unsigned int z);
	bool visit(grid a, grid seed);
	void visitNeighbor(grid seed);
	void cluster();
	void getInfo();
	void statistics();
	float getMaxEp();
	float getMinEp();
	void NormalDistributionTest();
	void filter();
	void average_deviation();
};

#endif