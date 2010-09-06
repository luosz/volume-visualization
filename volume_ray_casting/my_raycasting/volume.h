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

class volume
{
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
	unsigned char * group;             
	bool * tag;
	unsigned int threshold;
	unsigned int max, min;
	unsigned int max_gradient, max_df2, max_df3;
public:
	volume()
	{
		length = width = height = count = range = dataTypeSize = max = min = threshold = 0;
		max_gradient = 0;
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
	unsigned int getData(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getGradient(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getDf2(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getDf3(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getIndex(unsigned int x, unsigned int y, unsigned int z);
	unsigned int getX(void);
	unsigned int getY(void);
	unsigned int getZ(void);
	unsigned int getMax(void);
	unsigned int getMaxGrad(void);
	unsigned int getMaxDf2(void);
	unsigned int getMaxDf3(void);
	unsigned int getRange(void);
	char * getFormat(void);
	void * getDataAddr(void);
	bool allTraverse(bool * tag, int n);
	void clustering();
};

#endif