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

class volume
{
// 2010-09-26 16:30:20 ark: Changed to protected in order to use the properties in subclasses
//private:
protected:
	unsigned int length;               //体数据长度
	unsigned int width;                //体数据宽度
	unsigned int height;               //体数据高度
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
public:
	volume()
	{
		length = width = height = count = range = dataTypeSize = max_data = min_data = threshold = 0;
		max_frequency = 0;
		max_grad =  min_grad = 0;
		min_df2 = min_df3 = 0;
		data = NULL;
		histogram = NULL;
		color = NULL;
		opacity = NULL;
		group = NULL;
		tag = NULL;
		df2 = NULL;
		df3 = NULL;
	}
	~volume()
	{
		free(data);
		free(histogram);
		free(gradient);
		free(opacity);
		free(group);
		free(tag);
		free(df2);
		free(df3);
	}
	bool readVolFile(char * s);
	bool readData(char * s);
	void traverse();
	void calHistogram();
	void calDf2();
	void calDf3();
	void calGradient();
	void calGrad();
	void calGrad_ex();                   //use f(x) = a * exp(bx)
	unsigned int getData(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getGrad(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getDf2(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getDf3(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getIndex(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getX(void);
	unsigned int getY(void);
	unsigned int getZ(void);
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
};

#endif