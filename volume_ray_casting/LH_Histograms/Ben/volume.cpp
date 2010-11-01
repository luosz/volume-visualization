#include "volume.h"
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <cmath>

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
	
//	volume::readData(rawFilename);

	return true;
}

bool volume::readData(char * s)
{
	FILE * fp = fopen(s, "rb");

	printf("Loading data file: %s ......\n", s);
	cout<<fp;
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
	cout<<"max = "<<max_data<<endl;
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

	max_data = 0;
	min_data = 10000;

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
			if(c > max_data)
				max_data = c;
			if(c < min_data)
				min_data = c;
		}	
	}
	else if(strcmp(format, "USHORT") == 0)
	{
		for(i = 0;i < count;++i)
		{
			unsigned short * temp = (unsigned short *)data;
			int c = int(temp[i]);
			histogram[c]++;
			if(c > max_data)
				max_data =c;
			if(c < min_data)
				min_data = c;
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

	max_grad = df_dx = df_dy = df_dz = df = 0;
	for(x = 0;x < length;++x)
		for(y = 0;y < width;++y)
			for(z = 0;z < height;++z)
			{
				index = getIndex(x, y, z);
				df = 0;
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height -1)
				{
					gradient[index] = 0;
				//	df_dx = 
				}	
				else
				{
					df_dx = abs(float(getData(x - 1, y, z)) - float(getData(x, y, z)))
						  + abs(float(getData(x + 1, y, z)) - getData(x, y, z));
					df_dy = abs(float(getData(x, y - 1, z)) - getData(x, y, z))
						  + abs(float(getData(x, y + 1, z)) - getData(x, y, z));
					df_dz = abs(float(getData(x, y, z - 1)) - getData(x, y, z))
						  + abs(float(getData(x, y, z + 1)) - getData(x, y, z));
					df = df_dx + df_dy + df_dz;
					/*for(i = x - 1;i <= x + 1;++i)
						for(j = y - 1;j <= y + 1;++j)
							for(k = z - 1;k <= z + 1;++k)
								df += abs((long)(getData(i, j, k) - getData(x, y, z)));*/
 					gradient[index] = df;
					if(df > max_grad)
						max_grad= df;
				}				
			}
}

void volume::calGrad(void)
{
	int x, y, z, index;
	unsigned int i, j, k;
	double df_dx, df_dy, df_dz, df;
	
	df_dx = df_dy = df_dz = 0.0;
	gradient = (unsigned int *)malloc(count * sizeof(unsigned int));
	if(gradient == NULL)
		fprintf(stderr, "not enough memory for grdient");
	memset(gradient, 0, count * sizeof(unsigned int));

	max_grad = 0;
	min_grad = 10000;
	for(x = 0;x < length;++x)
		for(y = 0;y < width;++y)
			for(z = 0;z < height;++z)
			{
				index = getIndex(x, y, z);
				df = 0;

				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height -1)
				{
					if(x == 0)
					{
						df_dx = float(getData(x + 1, y, z)) - float(getData(x, y, z));
						df_dx *= 0.5;
					}	
					else if(x == length - 1)
					{
						df_dx = float(getData(x, y, z)) - float(getData(x - 1, y, z));
						df_dx *= 0.5;
					}					
					else if(y == 0)
					{
						df_dy = float(getData(x, y + 1, z)) - float(getData(x, y, z));
						df_dy *= 0.5;
					}					
					else if(y == width - 1)
					{
						df_dy = float(getData(x, y, z)) - float(getData(x, y - 1, z));
						df_dy *= 0.5;
					}	
					else if(z == 0)
					{
						df_dz = float(getData(x, y, z + 1)) - float(getData(x, y, z));
						df_dz *= 0.5;
					}					
					else 
					{
						df_dz = float(getData(x, y, z)) - float(getData(x, y, z - 1));
						df_dz *= 0.5;
					}
				}
				else
				{
					df_dx = float(getData(x + 1, y, z)) - float(getData(x - 1, y, z));
					df_dy = float(getData(x, y + 1, z)) - float(getData(x, y - 1, z));
					df_dz = float(getData(x, y, z + 1)) - float(getData(x, y, z - 1));
					df_dx *= 0.5;
					df_dy *= 0.5;
					df_dz *= 0.5;				
				}				
				df = sqrt(df_dx * df_dx + df_dy * df_dy + df_dz * df_dz);
				
 					gradient[index] = int(df);
					if(df > max_grad)
						max_grad = int(df);
					if(df < min_grad)
						min_grad = int (df);
		//				cout<<"df_dx = "<<df_dx<<"   df_dy = "<<df_dy<<"    df_dz = "<<df_dz<<"    df ="<<gradient[index]<<endl;
			}
}

void volume::calGrad_ex()
{
	int x, y, z, index;
	unsigned int i, j, k;
	double df_dx, df_dy, df_dz, df;
	double f1, f2;
	
	gradient = (unsigned int *)malloc(count * sizeof(unsigned int));
	if(gradient == NULL)
		fprintf(stderr, "not enough memory for grdient");
	memset(gradient, 0, count * sizeof(unsigned int));

	max_grad = df_dx = df_dy = df_dz = df = 0;
	for(x = 0;x < length;++x)
		for(y = 0;y < width;++y)
			for(z = 0;z < height;++z)
			{
				index = getIndex(x, y, z);
				df = 0;
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height -1)
				{
					gradient[index] = 0;
				}	
				else
				{
					f1 = getData(x - 1, y, z);
					f2 = getData(x + 1, y, z);
					if(int(f1) == 0)
						f1 = 1e-10;
					if(int(f2) == 0)
						f2 = 1e-10;
					df_dx = 0.5 * sqrt(f1 * f2) * log(f2 / f1);
		//			df_dx = pow(2.718281828459045, df_dx);
					
					f1 = getData(x, y - 1, z);
					f2 = getData(x, y + 1, z);
					if(int(f1) == 0)
						f1 = 1e-10;
					if(int(f2) == 0)
						f2 = 1e-10;
					df_dy = 0.5 * sqrt(f1 * f2) * log(f2 / f1);
		//			df_dy = pow(2.718281828459045, df_dy);

					f1 = getData(x, y, z - 1);
					f2 = getData(x, y, z + 1);
					if(int(f1) == 0)
						f1 = 1e-10;
					if(int(f2) == 0)
						f2 = 1e-10;
					df_dz = 0.5 * sqrt(f1 * f2) * log(f2 / f1);
		//			df_dz = pow(2.718281828459045, df_dz);

					df = sqrt(df_dx * df_dx + df_dy * df_dy + df_dz * df_dz);
			//		df = pow(2.7182, double(df)) - 1;
 					gradient[index] = int(df);
					if(df > max_grad)
						max_grad = int(df);
					if(df < min_grad)
						min_grad = int (df);
				}				
			}
}

void volume::calDf2(void)
{
	int x, y, z, index, i, j, k;
	double df2_dx, df2_dy, df2_dz, Df2;

	df2 = (unsigned int *)malloc(count * sizeof(unsigned int));
	if(df2 == NULL)
		fprintf(stderr, "not enough memory for df2");
	memset(df2, 0, count * sizeof(unsigned int));

	max_df2 = 0;
	min_df2 = 10000;
	df2_dx = df2_dy = df2_dz = Df2 = 0;
	for(x = 0; x < length; ++x)
		for(y = 0; y < width; ++y)
			for(z = 0; z < height; ++z)
			{
				index = getIndex(x, y, z);
				Df2 = 0;
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height - 1)
				{
					if(x == 0)
					{
						df2_dx = float(getGrad(x + 1, y, z)) - float(getGrad(x, y, z));
						df2_dx *= 0.5;
					}
					else if(x == length - 1)
					{
						df2_dx = float(getGrad(x, y, z)) - float(getGrad(x - 1, y, z));
						df2_dx *= 0.5;
					}
					else if(y == 0)
					{
						df2_dy = float(getGrad(x, y + 1, z)) - float(getGrad(x, y, z));
						df2_dy *= 0.5;
					}
					else if(y == width - 1)
					{
						df2_dy = float(getGrad(x, y, z)) - float(getGrad(x, y - 1, z));
						df2_dy *= 0.5;
					}
					else if(z == 0)
					{
						df2_dz = float(getGrad(x, y, z + 1)) - float(getGrad(x, y, z));
						df2_dz *= 0.5;
					}
					else
					{
						df2_dz = float(getGrad(x, y, z)) - float(getGrad(x, y, z - 1));
						df2_dz *= 0.5;
					}
				}
				else
				{
					df2_dx = float(getGrad(x + 1, y, z)) - float(getGrad(x - 1, y, z));
					df2_dy = float(getGrad(x, y + 1, z)) - float(getGrad(x, y - 1, z));
					df2_dz = float(getGrad(x, y, z + 1)) - float(getGrad(x, y, z - 1));
					df2_dx *= 0.5;
					df2_dy *= 0.5;
					df2_dz *= 0.5;
					
				}
				Df2 = sqrt(df2_dx * df2_dx + df2_dy * df2_dy + df2_dz * df2_dz);
						
					df2[index] = int(Df2);
					if(Df2 > max_df2)
						max_df2 = int(Df2);
					if(Df2 < min_df2)
						min_df2 = int(Df2);
	//			cout<<"df2_dx = "<<df2_dx<<"    df2_dy = "<<df2_dy<<"    df2_dz = "<<df2_dz<<" Df2 = "<<df2[index]<<endl;
			}
}

void volume::calDf3(void)
{
	int x, y, z, index, i, j, k; 
	double	df3_dx, df3_dy, df3_dz;
    int Df3;

	df3 = (unsigned int *)malloc(count * sizeof(unsigned int));
	if(df3 == NULL)
		fprintf(stderr, "not enough memory for df3");
	memset(df3, 0, count * sizeof(unsigned int));

	max_df3 = 0;
	min_df3 = 10000;	
	df3_dx = df3_dy = df3_dz = Df3 = 0;
	for(x = 0; x < length; ++x)
		for(y = 0; y < width; ++y)
			for(z = 0; z < height; ++z)
			{
				index = getIndex(x, y, z);
				Df3 = 0;
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height - 1)
				{
					if(x == 0)
					{
						df3_dx = float(getDf2(x + 1, y, z)) - float(getDf2(x, y, z));
						df3_dx *= 0.5;
					}
					else if(x == length - 1)
					{
						df3_dx = float(getDf2(x, y, z)) - float(getDf2(x - 1, y, z));
						df3_dx *= 0.5;
					}
					else if(y == 0)
					{
						df3_dy = float(getDf2(x, y + 1, z)) - float(getDf2(x, y, z));
						df3_dy *= 0.5;
					}
					else if(y == width - 1)
					{
						df3_dy = float(getDf2(x, y, z)) - float(getDf2(x, y - 1, z));
						df3_dy *= 0.5;
					}
					else if(z == 0)
					{
						df3_dz = float(getDf2(x, y, z + 1)) - float(getDf2(x, y, z));
						df3_dz *= 0.5;
					}
					else
					{
						df3_dz = float(getDf2(x, y, z)) - float(getDf2(x, y, z - 1));
						df3_dz *= 0.5;
					}
				}
				else
				{
					df3_dx = float(getDf2(x + 1, y, z)) - float(getDf2(x - 1, y, z));
					df3_dy = float(getDf2(x, y + 1, z)) - float(getDf2(x, y - 1, z));
					df3_dz = float(getDf2(x, y, z + 1)) - float(getDf2(x, y, z - 1));
					df3_dx *= 0.5;
					df3_dy *= 0.5;
					df3_dz *= 0.5;					
				}
				Df3 = sqrt(df3_dx * df3_dx + df3_dy * df3_dy + df3_dz * df3_dz);

					df3[index] = Df3;
					if(Df3 > max_df3)
						max_df3 = Df3;
					if(Df3 < min_df3)
						min_df3 = Df3;
					//			cout<<"df3_dx = "<<df3_dx<<"    df3_dy = "<<df3_dy<<"    df3_dz = "<<df3_dz<<" Df3 = "<<df3[index]<<endl;
			}
}


unsigned int volume::getGrad(unsigned int x, unsigned int y, unsigned int z)
{
	int index = z * width * length + y * length + x;
	
	return gradient[index]; 
}

unsigned int volume::getMaxData(void)
{
	return max_data;
}

unsigned int volume::getMinData(void)
{
	return min_data;
}

unsigned int volume::getMaxGrad(void)
{
	return max_grad; 
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

unsigned int volume::getMinGrad(void)
{
	return min_grad;
}

unsigned int volume::getMinDf2(void)
{
	return min_df2;
}

unsigned int volume::getMinDf3(void)
{
	return min_df3;
}

void volume::getInfo(void)
{
	int x, y, z;
	cout<<"Dimension X = "<<length<<"\t Dimension Y = "<<width<<"\t Dimension Z = "<<height<<endl;
	cout<<"Data max = "<<max_data<<"\t Data min = "<<min_data<<endl;
	cout<<"Grad max = "<<max_grad<<"\t Grad min = "<<min_grad<<endl;
	cout<<"Df2 max = "<<max_df2<<"\t Df2 min = "<<min_df2<<endl;
	cout<<"Df3 max = "<<max_df3<<"\t Df3 min = "<<min_df3<<endl;

	/*for(x = 0; x < length; ++x)
		for(y = 0;y < width; ++y)
			for(z = 0; z < height; ++z)
				cout<<getGroup(x, y, z)<<"\t"<<endl;*/
}

bool volume::allTraverse(bool * tag)
{
	int i, count;

	count = length * width * height;
	for(i = 0; i < count; ++i)
		if(tag[i] == false)
			return false;
	
	return true; 
}

void volume::setTag(unsigned int x, unsigned int y, unsigned int z)
{
	int index;
	
	index = getIndex(x, y, z);
	tag[index] = true;
}

bool volume::getTag(unsigned int x, unsigned int y, unsigned int z)
{
	return tag[getIndex(x, y, z)];
}

void volume::setGroup(unsigned int x, unsigned int y, unsigned int z, unsigned int id)
{
	group[getIndex(x, y, z)] = id;	
}

unsigned int volume::getGroup(unsigned int x, unsigned int y, unsigned int z)
{
	return group[getIndex(x, y, z)];
}

bool volume::visit(grid a, grid seed)
{
	grid next;
	if(getTag(a.x, a.y, a.z) || a.x < 0 || a.x >= length || a.y < 0 || a.y >= width || a.z < 0 || a.z >= height)
		return false;
	else
	{
		if(abs(float(getGrad(a.x, a.y, a.z)) - float(getGrad(seed.x , seed.y, seed.z))) < threshold)
		{
			setTag(a.x, a.y, a.z);
			setGroup(a.x, a.y, a.z, getGroup(seed.x, seed.y, seed.z));
		}
		else
		{
			setTag(a.x, a.y, a.z);
			setGroup(a.x, a.y, a.z, getGroup(seed.x, seed.y, seed.z) + 1);
		}
		visitNeighbor(a);
		return true;
	}	
}

void volume::visitNeighbor(grid seed)
{
	grid next;
	
	next = seed;
	next.x = seed.x - 1;
	visit(next, seed);

	next = seed;
	next.x = seed.x + 1;
	visit(next, seed);

	next = seed;
	next.y = seed.y - 1;
	visit(next, seed);

	next = seed;
	next.y = seed.y + 1;
	visit(next, seed);

	next = seed;
	next.z = seed.z - 1;
	visit(next, seed);

	next = seed;
	next.z = seed.z + 1;
	visit(next, seed);
}

void volume::cluster()
{
	int i;
	int x, y, z, index;
	grid seed, temp;
	int seed_x, seed_y, seed_z;
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

	i = 1;
	seed.x = length / 2;
	seed.y = width / 2;
	seed.z = height / 2;
	setGroup(seed.x, seed.y, seed.z, i);
	while(!allTraverse(tag))
	{
	   visitNeighbor(seed);
	}
}

void volume::statistics(void)
{
	int x, y, z;
	int dim_x, dim_y, dim_z;
	double total, count;

	ex = total = var = 0.0;
	count = length * width * height;
	for(x = 0; x < range;++x)
	{
		total += int(histogram[x]) * x;
	
	}	
	ex = total / count;
	for(x = 0; x < range;++x)	
		var += int(histogram[x]) * (x - ex) * (x - ex);
	var /= count;
	var = sqrt(var);
	cv = var / ex;
	cout<<"E(f) = "<<ex<<endl;
	cout<<"Var(f) = "<<var<<endl;
	cout<<"Cv(f) = "<<cv<<endl;
}

unsigned int volume::getHistogram(unsigned int intensity)
{
	return histogram[intensity];
}

unsigned int volume::getMaxFrequency(void)
{
	int i, result = 0;
	for(i = 0;i < range;++i)
		if(histogram[i] > result)
			result = histogram[i];

	return result;
}