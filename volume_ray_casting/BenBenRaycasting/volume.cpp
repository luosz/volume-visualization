/**	@file
* A source code file for the Volume class
*/

/// header file to be included
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <cmath>
#include "Volume.h"

using namespace std;

/// read volume's description file
bool Volume::readVolFile(char * s)
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

/// read data file to store in data
bool Volume::readData(char * s)
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

/// traverse all the voxels in the volume
void Volume::traverse()
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

/// get number of volume's voxels at dimension x
unsigned int Volume::getX(void)
{
	return length;
}

/// get number of volume's voxels at dimension y
unsigned int Volume::getY(void)
{
	return width;
}

/// get number of volume's voxels at dimension z
unsigned int Volume::getZ(void)
{
	return height;
}

/// calculate histogram of a dataset
void Volume::calHistogram()
{
	int i, c, total;

	max_data = 0;
	min_data = 10000;
	ofstream file("s.csv", std::ios::out);

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

	acc_distribution = (float * )malloc(sizeof(float) * count);
	if(acc_distribution == NULL)
		cout<<"Not enough space for acc_distribution"<<endl;
	
	total = 0;
	for(i = 0;i < range; ++i)
	{
		total += int(histogram[i]);
		acc_distribution[i] = float(total) / float(count);
//		cout<<"acc_distribution [ "<<i<<" ]  = "<< acc_distribution[i]<<endl;
	}

	for(i = 0; i < range-1; ++i)
	{
		file<<histogram[i]<<","<<histogram[i]<<endl;		
	}
	file<<histogram[i];
}

/// get volume data at position(x, y, z)
unsigned int Volume::getData(unsigned int x, unsigned int y, unsigned int z)
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

///	return array's index of the dataset at position (x, y, z)
	
unsigned int Volume::getIndex(unsigned int x, unsigned int y, unsigned int z)
{
	return (z * width * length + y * length + x);
}

/// calculate gradient magnitude
void Volume::calGradient()
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

/// compute gradient magnitude
void Volume::calGrad(void)
{
	int x, y, z, index;
	unsigned int i, j, k;
	double df_dx, df_dy, df_dz, df;
	ofstream file("E:\\bucky.csv", std::ios::out);

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
					//	df_dx *= 0.5;
					}	
					else if(x == length - 1)
					{
						df_dx = float(getData(x, y, z)) - float(getData(x - 1, y, z));
				//		df_dx *= 0.5;
					}					
					else if(y == 0)
					{
						df_dy = float(getData(x, y + 1, z)) - float(getData(x, y, z));
				//		df_dy *= 0.5;
					}					
					else if(y == width - 1)
					{
						df_dy = float(getData(x, y, z)) - float(getData(x, y - 1, z));
				//		df_dy *= 0.5;
					}	
					else if(z == 0)
					{
						df_dz = float(getData(x, y, z + 1)) - float(getData(x, y, z));
				//		df_dz *= 0.5;
					}					
					else 
					{
						df_dz = float(getData(x, y, z)) - float(getData(x, y, z - 1));
				//		df_dz *= 0.5;
					}
				}
				else
				{
					df_dx = float(getData(x + 1, y, z)) - float(getData(x - 1, y, z));
					df_dy = float(getData(x, y + 1, z)) - float(getData(x, y - 1, z));
					df_dz = float(getData(x, y, z + 1)) - float(getData(x, y, z - 1));
				//	df_dx *= 0.5;
				//	df_dy *= 0.5;
				//	df_dz *= 0.5;				
				}				
				df = sqrt(df_dx * df_dx + df_dy * df_dy + df_dz * df_dz);
				if(df != 0 && getData(x, y, z) != 0)
					file<<getData(x, y, z)<<", "<<df<<endl;
 					gradient[index] = int(df);
					if(df > max_grad)
						max_grad = int(df);
					if(df < min_grad)
						min_grad = int (df);
					
		//				cout<<"df_dx = "<<df_dx<<"   df_dy = "<<df_dy<<"    df_dz = "<<df_dz<<"    df ="<<gradient[index]<<endl;
			}
}

/// compute gradient magnitude using expoent function
void Volume::calGrad_ex()
{
	int x, y, z, index;
	unsigned int i, j, k;
	double df_dx, df_dy, df_dz, df;
	double f1, f2;
	ofstream file("E:\\d4_ex.csv", std::ios::out);

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
					if(x == 0)
					{
						f1 =getData(x, y, z);
						f2 = getData(x + 1,y ,z);
					}	
					if(x == length - 1)
					{
						f1 = getData(x - 1, y, z);
						f2 = getData(x, y, z);
					}
					if(int(f1) == 0)
						f1 = 1e-10;
					if(int(f2) == 0)
						f2 = 1e-10;
					df_dx = 0.5 * sqrt(f1 * f2) * log(f2 / f1);

					if(y == 0)
					{
						f1 =getData(x, y, z);
						f2 = getData(x, y +1,z);
					}	
					if(y == width - 1)
					{
						f1 = getData(x, y - 1, z);
						f2 = getData(x, y, z);
					}
					if(int(f1) == 0)
						f1 = 1e-10;
					if(int(f2) == 0)
						f2 = 1e-10;
					df_dy = 0.5 * sqrt(f1 * f2) * log(f2 / f1);					

					if(z == 0)
					{
						f1 =getData(x, y, z);
						f2 = getData(x, y, z + 1);
					}	
					if(z == height - 1)
					{
						f1 = getData(x, y, z - 1);
						f2 = getData(x, y, z);
					}
					if(int(f1) == 0)
						f1 = 1e-10;
					if(int(f2) == 0)
						f2 = 1e-10;
					df_dz = 0.5 * sqrt(f1 * f2) * log(f2 / f1);	

					df = sqrt(df_dx * df_dx + df_dy * df_dy + df_dz * df_dz);
					//		df = pow(2.7182, double(df)) - 1;
					if(df != 0 && getData(x, y, z) != 0)
						file<<getData(x, y, z)<<", "<<df<<endl;
					gradient[index] = int(df);
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
					
					if(df != 0 && getData(x, y, z) != 0)
						file<<getData(x, y, z)<<", "<<df<<endl;
 					gradient[index] = int(df);
					if(df > max_grad)
						max_grad = int(df);
					if(df < min_grad)
						min_grad = int (df);
				}				
			}			
}

/// calculate second derivative
void Volume::calDf2(void)
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

/// calculate third derivative
void Volume::calDf3(void)
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

					df3[index] = int(Df3);
					if(Df3 > max_df3)
						max_df3 = int(Df3);
					if(Df3 < min_df3)
						min_df3 = int(Df3);
					//			cout<<"df3_dx = "<<df3_dx<<"    df3_dy = "<<df3_dy<<"    df3_dz = "<<df3_dz<<" Df3 = "<<df3[index]<<endl;
			}
}

/// return gradient magnitude at position (x, y, z)
unsigned int Volume::getGrad(unsigned int x, unsigned int y, unsigned int z)
{
	int index = z * width * length + y * length + x;
	
	return gradient[index]; 
}

/// return maximum data
unsigned int Volume::getMaxData(void)
{
	return max_data;
}

/// return minimum data
unsigned int Volume::getMinData(void)
{
	return min_data;
}

/// return maximum gradient magnitude
unsigned int Volume::getMaxGrad(void)
{
	return max_grad; 
}

/// get data's range
unsigned int Volume::getRange(void)
{
	return range;
}

/// get maximum second derivative
unsigned int Volume::getMaxDf2(void)
{
	return max_df2;
}

/// get maximum third derivative
unsigned int Volume::getMaxDf3(void)
{
	return max_df3;
}

/// get second derivative at position (x, y, z)
unsigned int Volume::getDf2(unsigned int x, unsigned int y, unsigned int z)
{
	return df2[getIndex(x, y, z)];
}

/// get third derivative at position (x, y, z)
unsigned int Volume::getDf3(unsigned int x, unsigned int y, unsigned int z)
{
	return df3[getIndex(x, y, z)];
}

/// return data's format
char * Volume::getFormat(void)
{
	return format;
}

/// return data's address
void * Volume::getDataAddr(void)
{
	return data;
}

/// return minimum gradient magnitude
unsigned int Volume::getMinGrad(void)
{
	return min_grad;
}

/// return minimum second derivative
unsigned int Volume::getMinDf2(void)
{
	return min_df2;
}

/// return miminum third derivative
unsigned int Volume::getMinDf3(void)
{
	return min_df3;
}

/// print information about the data
void Volume::getInfo(void)
{
	int x, y, z;
	cout<<"Dimension X = "<<length<<"\t Dimension Y = "<<width<<"\t Dimension Z = "<<height<<endl;
	cout<<"Data max = "<<max_data<<"\t Data min = "<<min_data<<endl;
	cout<<"Grad max = "<<max_grad<<"\t Grad min = "<<min_grad<<endl;
//	cout<<"Df2 max = "<<max_df2<<"\t Df2 min = "<<min_df2<<endl;
//	cout<<"Df3 max = "<<max_df3<<"\t Df3 min = "<<min_df3<<endl;

	/*for(x = 0; x < length; ++x)
		for(y = 0;y < width; ++y)
			for(z = 0; z < height; ++z)
				cout<<getGroup(x, y, z)<<"\t"<<endl;*/
}

/// test if all the voxels in the volume are traversed
bool Volume::allTraverse(bool * tag)
{
	int i, count;

	count = length * width * height;
	for(i = 0; i < count; ++i)
		if(tag[i] == false)
			return false;
	
	return true; 
}

/// set tag to voxel at position(x, y, z)
void Volume::setTag(unsigned int x, unsigned int y, unsigned int z)
{
	int index;
	
	index = getIndex(x, y, z);
	tag[index] = true;
}

/// test if voxel at position (x, y, z) has been taged
bool Volume::getTag(unsigned int x, unsigned int y, unsigned int z)
{
	return tag[getIndex(x, y, z)];
}

/// set group identifier at position (x, y, z)
void Volume::setGroup(unsigned int x, unsigned int y, unsigned int z, unsigned int id)
{
	group[getIndex(x, y, z)] = id;	
}

/// return group identifier at position (x, y, z)
unsigned int Volume::getGroup(unsigned int x, unsigned int y, unsigned int z)
{
	return group[getIndex(x, y, z)];
}

/// visist grid a
bool Volume::visit(grid a, grid seed)
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

/// visit neighboring voxels arond seed
void Volume::visitNeighbor(grid seed)
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

/// cluster all the voxels
void Volume::cluster()
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

/// calculate some statistical information
void Volume::statistics(void)
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
	cout<<"max_grad = "<<max_grad<<endl;

	for(x = 1; x <= 11; ++x)
		for(y = 1; y <= 11;++y)
		{
			cout<<x<<",  "<<y<<",   "<<intensity_gradient_histogram[x][y]<<endl;
			cout<<"Spatial distribution ="<<spatial_distribution[x][y]<<endl;
		}		
	
}

/// return number of voxels with data intensity
unsigned int Volume::getHistogram(unsigned int intensity)
{
	return histogram[intensity];
}

/// return intensity that occurs most frequently
unsigned int Volume::getMaxFrequency(void)
{
	int i, result = 0;
	for(i = 0;i < range;++i)
		if(histogram[i] > result)
			result = histogram[i];

	return result;
}

/// calculate elasitiy
void Volume::calEp(void)
{
	int x, y, z, index;
	double f, f1, f2, df_dx, df_dy, df_dz, ep_x, ep_y, ep_z;

	ep = (float *)malloc(sizeof(float) * count);

	if(ep == NULL)
	{
		cout<<"Not enough space for EP"<<endl;
		return;
	}
	max_ep = 0;
	min_ep = 100;
	for(x = 0; x < length; ++x)
		for(y = 0;y < width; ++y)
			for(z = 0; z < height; ++z)
			{
				index = getIndex(x, y, z);
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height - 1)
					ep[index] = 0;
				else
				{				
					if(getData(x, y, z) == 0)
						f = 0.01;
					else
						f = getData(x, y, z);
					f1 = getData(x - 1, y, z);
					f2 = getData(x + 1, y, z);
					df_dx = (f2 - f1) / 2.0;
					
					ep_x = df_dx * double(x) / f;

					f1 = getData(x, y - 1, z);
					f2 = getData(x, y + 1, z);
					df_dy = (f2 - f1) / 2.0;
					ep_y = df_dy * double(y) / f;

					f1 = getData(x, y, z - 1);
					f2 = getData(x, y, z + 1);
					df_dz = (f2 - f1) / 2.0;
					ep_z = df_dz * double(y) / f;
					ep[index] = sqrt(ep_x * ep_x 
																+ ep_y * ep_y
																+ ep_z * ep_z);
					if(ep[index] > max_ep)
						max_ep = ep[index];
					if(ep[index] < min_ep)
						min_ep = ep[index];
				}
			}
}

/// return elasticity at position (x, y, z)
float Volume::getEp(unsigned int x, unsigned int y, unsigned int z)
{
	return ep[getIndex(x, y, z)];
}

/// return maximum elasticity 
float Volume::getMaxEp()
{
	return max_ep;
}

/// return minimum elasticity
float Volume::getMinEp()
{
	return min_ep;
}

/// return number of voxels of the dataset
unsigned int Volume::getCount()
{
	return length * width * height;
}

/// calculate LH histogram
void Volume::calLH()
{
	int x, y, z, index;
	int i, j, k, H, L;
	int current_x, current_y, current_z;
	int current2_x, current2_y, current2_z;
	int H_x, H_y, H_z;
	int L_x, L_y, L_z;

	LH * LH_Histogram = (LH *)malloc(sizeof(LH) * length * width * height);
	if(LH_Histogram == NULL)
		cout<<"Not enough space for LH Histogram"<<endl;


	for(x = 0; x < length; ++x)
		for(y = 0; y < width; ++y)
			for(z = 0; z < height; ++z)
			{
				index = getIndex(x, y, z);
				if(x == 0 || x == length - 1 || y == 0 || y == width - 1 || z == 0 || z == height - 1)
					LH_Histogram[index].FH = LH_Histogram[index].FL = getData(x, y, z);
				else if(getGrad(x, y, z) < little_epsilon)
					LH_Histogram[index].FH = LH_Histogram[index].FL = getData(x, y, z);
				else
				{
				//	cout<<"OK"<<endl;
					current_x = x;
					current_y = y;
					current_z = z;
					do
					{
						H = getData(current_x, current_y, current_z);
				//		cout<<"H = "<<H<<endl;
						for(i = current_x - 1;i <= current_x + 1; ++i)
						{
							for(j = current_y - 1; j <= current_y +1; ++j)
							{
								for(k = current_z - 1; k <= current_z + 1; ++k)
								{
									if(i < 0 || i > length - 1 || j < 0 || j > width - 1 || z < 0 || z > height - 1)
									{
						//				cout<<"Not ok"<<endl;;
										goto tag1;
									}
									if(getData(i, j, k) >= H)
									{
						//				cout<<"Higher"<<endl;
										H = getData(i, j, k);
										H_x = i;
										H_y = j;
										H_z = k;
					//					cout<<i<<", "<<j<<", "<<k<<endl;
									}
								}
							}
					 }
								current_x = H_x;
								current_y = H_y;
								current_z = H_z;
								if(current_x < 0 || current_y > length -1 || current_y < 0 || current_y > width - 1 || current_z < 0 || current_z > height -1)
									goto tag1;
					}while(getGrad(current_x, current_y, current_z) >= little_epsilon 
						     );
					tag1 : ; 
					cout<<"break out"<<endl;

					current_x = x;
					current_y = y;
					current_z = z;
					do
					{
						L = getData(current_x, current_y, current_z);
						for(i = current_x - 1;i <= current_x + 1; ++i)
							for(j = current_y - 1; j <= current_y +1; ++j)
								for(k = current_z - 1; k <= current_z + 1; ++k)
								{
									if(i < 0 || i > length - 1 || j < 0 || j > width - 1 || z < 0 || z > height - 1)
									{
										cout<<"Not ok";
										goto tag2;
									}	
									if(i == current_x && j == current_y && k == current_z)
										break;
									if(getData(i, j, k) <= L)
									{
										L = getData(i, j, k);
										L_x = i;
										L_y = j;
										L_z = k;
									}
								}
								current_x = L_x;
								current_y = L_y;
								current_z = L_z;
					}while(
						      getGrad(current_x, current_y, current_z) >= little_epsilon);
					cout<<"break out 2"<<endl;
					tag2: ;
					LH_Histogram[index].FH = H;
					LH_Histogram[index].FL = L;

					cout<<"H = "<<H<<", L = "<<L<<endl;
				}
			}
			for(x = 0; x < length; ++x)
				for(y = 0; y  < width; ++y)
					for(z = 0; z < height; ++z)
					{
						index = getIndex(x, y, z);
						if(LH_Histogram[index].FH != LH_Histogram[index].FL)
						cout<<"X = "<<x<<",  Y = "<<y<<" ,  Z = "<<z<<" , FH = "
							<<LH_Histogram[index].FH<<endl;
					//		" , FL = "<<LH_Histogram[index].FL<<endl;
					}
}

/// calculate intensity-gradient magnitude scatter plot
void Volume::Intensity_gradient_histogram()
{
	int i, j;
	int x, y, z, temp1, temp2;
	float pre_x, pre_y, pre_z, pos_x, pos_y, pos_z;
	float value, df1, value_max;

	for(i = 1;i <= 11;++i)
		for(j = 1;j <= 11;++j)
		{
			intensity_gradient_histogram[i][j] = 0;
			spatial_distribution[i][j] = 0;
		}	

		for(i = 1; i <= 11;++i)
			for(j = 1; j <= 11; ++j)
			{
				pre_x = pre_y = pre_z = 0;
				for(x = 0; x < length;++x)
					for(y = 0; y < width; ++y)
						for(z = 0; z < height; ++z)
						{
							value = float(getData(x, y, z));
							df1 = float(getGrad(x, y, z));
							value /= float(getMaxData());
							df1 /= float(getMaxGrad());
							temp1 = int(value * 10);
							temp2 = int(df1 * 10);
								if((temp1 >= i - 1) &&(temp1 < i) && (temp2 >= j - 1) && (temp2 < j))
								{
									intensity_gradient_histogram[i][j]++;
									pos_x = float(x) / float(length);
									pos_y = float(y) / float(width);
									pos_z = float(z) / float(height);
					//				cout<<pos_x<<pos_y<<pos_z<<endl;
									spatial_distribution[i][j] += sqrt(pow(double(pos_x - pre_x), 2.0) 
																					+  pow(double(pos_y - pre_y), 2.0)
																					+  pow(double(pos_z - pre_z), 2.0));
									pre_x = pos_x;
									pre_y = pos_y;
									pre_z = pos_z;
								}
						}
			}
		
}

/// return data's spatial distribution at (i, j)
float Volume::getSpatialDistribution(int i, int j)
{
	return spatial_distribution[i][j];
}

/// return getIntensity_gradient_histogram at (i, j)
unsigned int Volume::getIntensity_gradient_histogram(int i, int j)
{
	return intensity_gradient_histogram[i][j];
}

/// test if voxels comply to normal distribution
void Volume::NormalDistributionTest()
{
	int x, y, z, i, j, k;
	int index, number = 0;
	int value[27], temp;
	float a[13] = { 0.4366, 0.3018, 0.2522, 0.2152, 0.1848, 0.1584, 0.1346,
		0.1128, 0.0923, 0.0728, 0.0540, 0.0358, 0.0178
	};
	float average, d1, d2, w;

	for(x = 1; x <= getX() - 2; ++x)
		for(y = 1;y <= getY() - 2; ++y)
			for(z = 1; z  <= getZ() - 2; ++z)
			{
				index = 1;
				for(i = x - 1; i <= x + 1; ++i)
					for(j = y - 1; j <= y +1; ++j)
						for(k = z - 1; k <= z + 1; ++k)
						{
							value[index] = getData(i, j, k);
							index++;
						}
						for(i = 0; i < 27; ++i)
							for(j = i + 1;j < 27; ++j)
							{
								if(value[j] < value[i])
								{
									temp = value[j];
									value[j] = value[i];
									value[i] = temp;
								}
							}
							average = 0;
							for(i = 0; i < 27;++i)
							{
								average += float(value[i]);
								//		cout<<value[i]<<"\t";
							}
							average /= 27;
							d1 = d2 = 0;
							for(i = 0;i < 13; ++i)
								d1 += a[i] * double(value[26 - i] - value[i]);
							d1 = d1 * d1;
							for(i = 0;i < 27;++i)
								d2 += float(value[i] - average) * float(value[i] - average);
							if(d2 == 0)
								w = 0;
							else
								w = d1 / d2;
					//			cout<<"w = "<<w<<endl;
							if(w - 0.935 < 0)     
							;	// 0.923 0.935
				//						cout<<"不符合正态分布"<<endl;
							else
							{
								number++;
				//				cout<<"符合正态分布"<<endl;
							}
			}
			cout<<float(number) /  float(getCount());
			while(1);
}

/// filter voxels to shape edges 
void Volume::filter()
{
	unsigned char * data_char;
	unsigned short * data_short;
	int i, j, k, index, p, q, r;
	float tt;

	if(strcmp(getFormat(), "UCHAR"))
	{
		data_char = (unsigned char *)malloc(sizeof(unsigned char) * getCount());
		for(i = 0;i < getX(); ++i)
			for(j = 0;j < getY(); ++j)
				for(k = 0;k < getZ();++k)
				{
					index = getIndex(i ,j ,k);
					if(i == 0 || i == getX() - 1|| j == 0 || j == getY() - 1 || k == 0 || k == getZ() - 1)
					{						
						data_char[index] = getData(i, j, k);
					}
					else
					{
						tt = 0;
						for(p = i - 1;p <= i + 1;++p)
							for(q = j - 1; q <= j + 1; ++q)
								for(r = k - 1; r <= k + 1; ++r)
									tt += int(getData(p, q, r));
						tt /= 27;
						data_char[index] = int(tt);
					}
				}
		memcpy(data, data_char, sizeof(unsigned char) * getCount());
	}
	else if(strcmp(getFormat(), "USHORT"))
	{
		data_short = (unsigned short *)malloc(sizeof(unsigned short) * getCount());
		for(i = 0;i < getX(); ++i)
			for(j = 0;j < getY(); ++j)
				for(k = 0;k < getZ();++k)
				{
					index = getIndex(i ,j ,k);
					if(i == 0 || i == getX() - 1|| j == 0 || j == getY() - 1 || k == 0 || k == getZ() - 1)
					{						
						data_short[index] = getData(i, j, k);
					}
					else
					{
						tt = 0;
						for(p = i - 1;p <= i + 1;++p)
							for(q = j - 1; q <= j + 1; ++q)
								for(r = k - 1; r <= k + 1; ++r)
									tt += int(getData(p, q, r));
						tt /= 27;
						data_short[index] = int(tt);
					}
				}
				memcpy(data, data_short, sizeof(unsigned short) * getCount());
	}
	else
		;
}

/// calculate average value and deviation value
void Volume::average_deviation()
{
	int i, j, k, p, q, r, index;
	float a, d, d_max = 0;
	
	max_variation = 0;
	average = (float *)malloc(sizeof(float) * getCount());
	variation = (float *)malloc(sizeof(float) * getCount());
//	ofstream file("E:\\d4_ad.csv", std::ios::out);
	for(i = 0;i < getX(); ++i)
		for(j = 0; j < getY(); ++j)
			for(k = 0; k < getZ(); ++k)
			{
				index =getIndex(i ,j, k);
				if(i == 0 || i == getX() - 1|| j == 0 || j == getY() - 1 || k == 0 || k == getZ() - 1)
				{						
					a = d = 0; 
					average[index] = variation[index] = 0;
				//	file<<a<<", "<<d<<endl;
				}
				else
				{
					a = d = 0;
					for(p = i - 1;p <= i + 1;++p)
						for(q = j - 1; q <= j + 1; ++q)
							for(r = k - 1; r <= k + 1; ++r)
								a += float(getData(p, q, r));
					a /= 27;
					average[index] = a;
					for(p = i - 1;p <= i + 1;++p)
						for(q = j - 1; q <= j + 1; ++q)
							for(r = k - 1; r <= k + 1; ++r)
								d += pow(double(getData(p, q, r)) - a, 2.0);
					d /= 27.0;
					variation[index] = d;
					if(d > max_variation)
						max_variation = d;
		//			cout<<a<<d<<endl;
		//			if(d > 0 && a != 0)
		//				file<<a<<", "<<d<<endl;
				
			//		cout<<i<<", "<<j<<", "<<k<<",  a = "<<a<<", d = "<<d<<endl;
					
				}
			}
		//	cout<<d_max<<endl;
}

/// calculate local entropy of all the voxels
void Volume::calLocalEntropy()
{
	int x, y, z, index, i, j, k, p;
	float sum, prob;

	local_entropy = (float *)malloc(sizeof(float) * getCount());

	if(local_entropy == NULL)
		cout<<"Not enough space for local entropy"<<endl;

	typedef struct 
	{
		int value;
		int freq;
	}vf;

	vf a[27];

	unsigned long num[65536];

	local_entropy_max = 0;


	for(x = 0; x < getX(); ++x)
		for(y = 0; y < getY(); ++y)
			for(z = 0; z < getZ(); ++z)
			{
				for(i = 0; i < 65536; ++i)
					num[i] = 0;
				index = getIndex(x, y, z);
				if(x == 0 || x == getX() - 1 || y == 0 || y == getY() - 1 || z == 0 || z == getZ() - 1)
					local_entropy[index] = 0;
				else
				{
					for(i = x - 1; i <= x + 1; ++i)
						for(j = y - 1;j <= y + 1; ++j)
							for(k = z - 1; k <= z + 1; ++k)
							{
								p = getData(i, j, k);
								num[p]++;
							}
				sum = 0;
				for(i = 0;i < 65536; ++i)
				{
					if(num[i] != 0)
					{
						prob = float(num[i]) / 27.0;
						sum += (-prob) * log(prob);
					}
				}
				local_entropy[index] = sum;
				if(sum > local_entropy_max)
					local_entropy_max = sum;
			}
		}
}

/// return local entropy at (x, y, z)
float Volume::getLocalEntropy(unsigned int x, unsigned int y, unsigned int z)
{
	int index = getIndex(x, y, z);

	return local_entropy[index];
}

/// return maximum local entropy
float Volume::getLocalEntropyMax()
{
	return local_entropy_max;
}

/// calculate local statistical property - average value 
void Volume::calAverage()
{
	int x, y, z, i, j, k, index;
	float sum;

	average = (float *)malloc(sizeof(float) * getCount());

	for(x = 0; x < getX(); ++x)
		for(y = 0; y < getY(); ++y)
			for(z = 0; z < getZ(); ++z)
			{
				index = getIndex(x, y, z);
				average[index] = 0;
			}
	for(x = 0; x < getX(); ++x)
		for(y = 0; y < getY(); ++y)
			for(z = 0; z < getZ(); ++z)
			{
				index = getIndex(x, y, z);
				if(x == 0 || x == getX() - 1 
				  || y ==0 || y == getY() - 1 
				  || z == 0 || z == getZ() - 1)
					average[index] = 0;
				else
				{
					sum = 0;	
					for(i = x - 1; i <= x + 1; ++i)
							for(j = y - 1; j <= y + 1; ++j)
								for(k = z - 1; k <= z + 1; ++k)
									sum += getData(x, y, z);
					sum /= 27.0;
					average[index] = sum;
				}
			}
}

/// return average value at position (x, y, z)
float Volume::getAverage(unsigned int x, unsigned int y, unsigned int z)
{
	int index = getIndex(x, y, z);

	return average[index];
}

void Volume::calVariation()
{
	int x, y, z, i, j, k, index;
	float sum, a;

	max_variation = 0;
	variation = (float *)malloc(sizeof(float) * getCount());

	for(x = 0; x < getX(); ++x)
		for(y = 0; y < getY(); ++y)
			for(z = 0; z < getZ(); ++z)
			{
				index = getIndex(x, y, z);
				variation[index] = 0;
			}

	for(x = 0; x < getX(); ++x)
		for(y = 0; y < getY(); ++y)
			for(z = 0; z < getZ(); ++z)
			{
				index = getIndex(x, y, z);
				if(x == 0 || x == getX() - 1 
					|| y ==0 || y == getY() - 1 
					|| z == 0 || z == getZ() - 1)
					variation[index] = 0;
				else
				{
					sum = 0;	
					a =getAverage(x, y, z);
					for(i = x - 1; i <= x + 1; ++i)
						for(j = y - 1; j <= y + 1; ++j)
							for(k = z - 1; k <= z + 1; ++k)
								sum += pow(double(a - getData(i, j, k)), 2.0);
					sum /= 27.0;
					variation[index] = sum;
					if(sum == 0)
						variation[index] = 1e-4;
					if(sum > max_variation)
						max_variation = sum;
				}
			}
}

/// return variation at position (x, y, z)
float Volume::getVariation(unsigned int x, unsigned int y, unsigned int z)
{
	int index = getIndex(x, y, z);

	return variation[index];
}

/// return maximum variation of the dataset
float Volume::getMaxVariation()
{
	return max_variation;
}