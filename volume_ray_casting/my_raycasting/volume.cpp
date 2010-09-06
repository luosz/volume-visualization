#include "volume.h"
#include <iostream>
#include <cstring>
#include <fstream>

using namespace std;

bool volume::readVolFile(char * s)
{
	char * cp, line[100], rawFilename[100];
	FILE * fp = fopen(s, "r");
	
	printf("Reading data description file %s ......\n", s);
	if(fp == NULL)
	{
		fprintf(stderr, "ERROR: Could not open file %s\n", s);
		return false;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if(strstr(line, "ObjectFileName"))
		{
			cp = strchr(line, ':');
			sscanf(cp + 1, "%s", rawFilename);
			printf("Get data file name: %s ......\n", rawFilename);
		}
		else if(strstr(line, "Resolution"))
		{
			cp = strchr(line, ':');
			sscanf(cp + 1, "%i %i %i", &length, &width, &height);
			count = length * width * height;
			printf("Get data's resolution\nlength = %i\nwidth = %i\nheight = %i\n", length, width, height);
		}
		else if(strstr(line, "Format"))
		{
			cp = strchr(line, ':');
			if(strstr(line, "UCHAR"))
			{
				strcpy(format, "UCHAR");
				printf("Get data's format: Unsigned Char\n");
				dataTypeSize = sizeof(char);
				range = 256;				
			}
			else if(strstr(line, "USHORT"))
			{
				strcpy(format, "USHORT");
				printf("Get data's format: Unsigned Short\n");
				dataTypeSize = sizeof(short);
				range = 65536;
			}
			else 
				fprintf(stderr, "invalid data format\n");
		}	
		else 
		;	
		//fprintf(stderr, "skipping line %s\n", line);
	}
	fclose(fp);
	
	//volume::readData(rawFilename);

	return true;
}

bool volume::readData(char * s)
{
	FILE * fp = fopen(s, "rb");

	printf("Loading data file: %s ......\n", s);
	//cout<<fp;
	if(fp == NULL)
	{
		fprintf(stderr, "ERROR: Could not open file %s\n", s);
		return false;
	}
	data = malloc(count * dataTypeSize);
	if(data == NULL)
	{
		fprintf(stderr, "not enough memory for volume data\n");
		return false;
	}
	memset(data, 0, count * dataTypeSize);

	if(fread(data, dataTypeSize, length * width * height, fp) != count)
	{
		fprintf(stderr, "reading volume data filed\n");
		return false;
	}
	printf("Loading data done.\n");
	fclose(fp);
	return true;
}

void volume::traverse()
{
	int i, j, k;	
	
	unsigned char c; 
	unsigned short a;
	ofstream write;
	write.open("a.txt");
	
	if(strcmp(format, "UCHAR") == 0)
	{
		for(i = 0; i < count; ++i)
		{
			unsigned char * temp = (unsigned char *)data;
			c = int(temp[i]);
		
			cout<<(int)c<<"\t";
			write<<hex<<(int)c<<"\t";
		}
		cout<<endl;	
			
	}
	else if(strcmp(format, "USHORT") == 0)
	{
		for(i = 0; i < count; ++i)
		{
			unsigned short * temp = (unsigned short *)data;
			a	= int(temp[i]);
		//	cout<<(int)a<<"\t";
				write<<hex<<(int)a<<"\t";
		}
	}	
	else			
	{
		printf("Invalid data.\n");
	}

/*	for(i = 0; i < length;++i)
			for(j = 0;j < width;++j)
				for(k = 0;k < height;++k)
				{
					write<<hex<<volume::getData(i, j, k)<<"\t";
				}
*/
	cout<<"length = "<<length<<endl;
	cout<<"width = "<<width<<endl;
	cout<<"hight = "<<height<<endl;
	cout<<"count = "<<count<<endl;
	cout<<"format = "<<format<<endl;
	cout<<"range = "<<range<<endl;
	cout<<"max = "<<max<<endl;
	write<<"Histogram: "<<endl;
	for(i = 0;i < range;++i)
		write<<"Histogram["<<i<<"] = "<<(int)histogram[i]<<endl;

	write.close();
}

unsigned int volume::getX(void)
{
	return length;
}

unsigned int volume::getY(void)
{
	return width;
}

unsigned int volume::getZ(void)
{
	return height;
}

void volume::calHistogram()
{
	int i, c;
	histogram = (unsigned int * )malloc(range * sizeof(unsigned int));
	if(histogram == NULL)
	{
		fprintf(stderr, "not enough memory for histogram\n");
	}
	for(i = 0;i < range;++i)
		histogram[i] = 0;
	if(strcmp(format, "UCHAR") == 0)
	{
		for(i = 0;i < count;++i)
		{
			unsigned char * temp = (unsigned char *)data;
			c = int(temp[i]);
			histogram[c]++;
			if(c > max)
				max = c;
		}	
	}
	else if(strcmp(format, "USHORT") == 0)
	{
		for(i = 0;i < count;++i)
		{
			unsigned short * temp = (unsigned short *)data;
			int c = int(temp[i]);
			histogram[c]++;
			if(c > max)
				max =c;
		}	
	}
	else
		printf("Invalid data.\n");
}

unsigned int volume::getData(unsigned int x, unsigned int y, unsigned int z)
{
	int index;
	index = getIndex(x, y, z);
	if(strcmp(format, "UCHAR") == 0)
	{
		unsigned char * temp = (unsigned char *)data;
		return (unsigned int)temp[index];
	}
	else if(strcmp(format, "USHORT") == 0)
	{
		unsigned short * temp = (unsigned short *)data;
		return (unsigned int)temp[index];
	}
	else 
	{
		printf("Invalid data.\n");
		return -1;
	}
}

unsigned int volume::getIndex(unsigned int x, unsigned int y, unsigned int z)
{
	return (z * width * length + y * length + x);
}

void volume::calGradient()
{
	int x, y, z, index;
	unsigned int df, i, j, k, df_dx, df_dy, df_dz;
	
	gradient = (unsigned int *)malloc(count * sizeof(unsigned int));
	if(gradient == NULL)
		fprintf(stderr, "not enough memory for grdient");
	memset(gradient, 0, count * sizeof(unsigned int));

	max_gradient = df_dx = df_dy = df_dz = df = 0;
	for(x = 0;x < length;++x)
		for(y = 0;y < width;++y)
			for(z = 0;z < height;++z)
			{
				index = getIndex(x, y, z);
				df = 0;
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height -1)
					gradient[index] = 0;
				else
				{
					/*df_dx = abs(getData(x - 1, y, z) - getData(x, y, z))
						  + abs(getData(x + 1, y, z) - getData(x, y, z));
					df_dy = abs(getData(x, y - 1, z) - getData(x, y, z))
						  + abs(getData(x, y + 1, z) - getData(x, y, z));
					df_dz = abs(getData(x, y, z - 1) - getData(x, y, z))
						  + abs(getData(x, y, z + 1) - getData(x, y, z));
					df = df_dx + df_dy + df_dz;*/
					for(i = x - 1;i <= x + 1;++i)
						for(j = y - 1;j <= y + 1;++j)
							for(k = z - 1;k <= z + 1;++k)
								df += abs((long)(getData(i, j, k) - getData(x, y, z)));
 					gradient[index] = df;
					if(df > max_gradient)
						max_gradient = df;
				}				
			}
}

void volume::calDf2(void)
{
	int x, y, z, index, i, j, k, Df2, df2_dx, df2_dy, df2_dz;

	df2 = (unsigned int *)malloc(count * sizeof(unsigned int));
	if(df2 == NULL)
		fprintf(stderr, "not enough memory for df2");
	memset(df2, 0, count * sizeof(unsigned int));

	max_df2 = df2_dx = df2_dy = df2_dz = Df2 = 0;
	for(x = 0; x < length; ++x)
		for(y = 0; y < width; ++y)
			for(z = 0; z < height; ++z)
			{
				index = getIndex(x, y, z);
				Df2 = 0;
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height - 1)
					df2[index] = 0;
				else
				{
			/*		df2_dx = abs(getGradient(x - 1, y, z) - getGradient(x, y, z))
						   + abs(getGradient(x + 1, y, z) - getGradient(x, y, z));
					df2_dy = abs(getGradient(x, y - 1, z) - getGradient(x, y, z))
						   + abs(getGradient(x, y + 1, z) - getGradient(x, y, z));
					df2_dz = abs(getGradient(x, y, z - 1) - getGradient(x, y, z))
						   + abs(getGradient(x, y, z + 1) - getGradient(x, y, z));
					Df2 = df2_dx + df2_dy + df2_dz;
			*/
					for(i = x - 1;i <= x + 1;++i)
						for(j = y - 1;j <= y + 1;++j)
							for(k = z - 1;k <= z + 1;++k)
								Df2 += abs((long)(getGradient(i, j, k) - getGradient(x, y, z)));
					df2[index] = Df2;
					if(Df2 > max_df2)
						max_df2 = Df2;
				}
			}
}

void volume::calDf3(void)
{
	int x, y, z, index, i, j, k, df3_dx, df3_dy, df3_dz;
    int Df3;

	df3 = (unsigned int *)malloc(count * sizeof(unsigned int));
	if(df3 == NULL)
		fprintf(stderr, "not enough memory for df3");
	memset(df3, 0, count * sizeof(unsigned int));

	max_df3 = df3_dx = df3_dy = df3_dz = Df3 = 0;
	for(x = 0; x < length; ++x)
		for(y = 0; y < width; ++y)
			for(z = 0; z < height; ++z)
			{
				index = getIndex(x, y, z);
				Df3 = 0;
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height - 1)
					df3[index] = 0;
				else
				{
			/*		df3_dx = abs(getDf2(x - 1, y, z) - getDf2(x, y, z))
						   + abs(getDf2(x + 1, y, z) - getDf2(x, y, z));
					df3_dy = abs(getDf2(x, y - 1, z) - getDf2(x, y, z))
						   + abs(getDf2(x, y + 1, z) - getDf2(x, y, z));
					df3_dz = abs(getDf2(x, y, z - 1) - getDf2(x, y, z))
						   + abs(getDf2(x, y, z + 1) - getDf2(x, y, z));	
					Df3 = df3_dx + df3_dy + df3_dz;
			*/
					for(i = x - 1;i <= x + 1;++i)
						for(j = y - 1;j <= y + 1;++j)
							for(k = z - 1;k <= z + 1;++k)
								Df3 += abs((long)(getDf2(i, j, k) - getDf2(x, y, z)));
					df3[index] = Df3;
					if(Df3 > max_df3)
						max_df3 = Df3;
				}
			}
}

unsigned int volume::getGradient(unsigned int x, unsigned int y, unsigned int z)
{
	int index = z * width * length + y * length + x;
	
	return gradient[index]; 
}

bool allTraverse(bool * tag, int n)
{
	int i;
	for(i = 0; i < n; ++i)
		if(tag[i] == false)
			return false;
	
	return true; 
}

void volume::clustering()
{
	int i;
	int x, y, z, index;
	bool * tag;
	
	i = 0;
	threshold = 15;
	group = (unsigned char *)malloc(count * sizeof(unsigned char));
	tag = (bool *)malloc(count * sizeof(bool));
	if(group == NULL)
		fprintf(stderr, "not enough memory for group");
	if(tag == NULL)
		fprintf(stderr, "not enough memory for tag");
	memset(group, '0', count);
	memset(tag, false, count);
	for(x = 0; x < length; ++x)
		for(y = 0; y < width; ++y)
			for(z = 0; z < height; ++z)
			{
				if(x == 0 || x == length - 1 
				|| y == 0 || y == width  - 1 
				|| z == 0 || z == height - 1)
				{
					index = z * width * length + y * length + x;
					group[index] = i;
					tag[index] = true;
				}
			}
	++i;
	x = length / 2;
	y = width / 2;
	z = height / 2;
	index = z * width * length + y * length + x;

/*	do
	{
		group[index] = i;
		tag[index] = true;
		if( abs(getGradient(x, y, z) - getGradient(x - 1, y, z)) < threshold)
			;
	}while(allTraverse(tag, count) == false);
*/
	
}

unsigned int volume::getMax(void)
{
	return max;
}

unsigned int volume::getMaxGrad(void)
{
	return max_gradient; 
}

unsigned int volume::getRange(void)
{
	return range;
}

unsigned int volume::getMaxDf2(void)
{
	return max_df2;
}

unsigned int volume::getMaxDf3(void)
{
	return max_df3;
}

unsigned int volume::getDf2(unsigned int x, unsigned int y, unsigned int z)
{
	return df2[getIndex(x, y, z)];
}

unsigned int volume::getDf3(unsigned int x, unsigned int y, unsigned int z)
{
	return df3[getIndex(x, y, z)];
}

char * volume::getFormat(void)
{
	return format;
}

void * volume::getDataAddr(void)
{
	return data;
}