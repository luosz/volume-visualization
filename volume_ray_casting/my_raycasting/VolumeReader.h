#pragma once

#ifndef VolumeReader_h
#define VolumeReader_h

#include <cstdio>
#include <cctype>
#include <cstring>

#include "Ben\volume.h"
#include "reader.h"

class VolumeReader : public volume
{
public:

	VolumeReader(void)
	{
	}

	virtual ~VolumeReader(void)
	{
	}

	// get the .raw file name from the .dat file automatically
	bool readVolume(char * s)
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
				fprintf(stderr, "skipping line %s\n", line);
		}
		fclose(fp);

		//////////////////////////////////////////////////////////////////////////
		// get the raw file path and filename
		const char *p = get_file_path_separator_position(s);
		if (NULL == p)
		{
			volume::readData(rawFilename);
		}else
		{
			strcpy((char *)(p) + 1, rawFilename);
			volume::readData(s);
		}
		//////////////////////////////////////////////////////////////////////////

		return true;
	}

	// utilize readVolFile and readData
	void readVolume_Ben(char* filename = NULL)
	{
		readVolFile(filename);
		char str[STR_BUFFER_SIZE];
		strcpy(str, filename);
		int length = strlen(str);
		if (length > 4 && tolower(str[length - 4])=='.' && tolower(str[length - 3])=='d' && tolower(str[length - 2])=='a' && tolower(str[length - 1])=='t')
		{
			str[length - 3] = 'r';
			str[length - 2] = 'a';
			str[length - 1] = 'w';
			readData(str);
		}else
		{
			std::cerr<<"Errors in reading .dat file: "<<filename<<" and .raw file: "<<str<<std::endl;
		}
	}

	// read volume data from file using readData in reader.h
	void readVolume_reader(char* filename)
	{
		using namespace reader;

		void ** data_ptr = new void *;
		int sizes[3];
		int color_omponent_number;
		float dists[3];
		DataType type;
		reader::readData(filename, sizes, dists, data_ptr, &type, &color_omponent_number);

		// Please set the properties of class volume to protected, in order to set their values.
		length = sizes[0];
		width = sizes[1];
		height = sizes[2];
		count = length * width * height;

		switch (type)
		{
		case DATRAW_UCHAR:
			strcpy(format, "UCHAR");
			dataTypeSize = sizeof(char);
			range = 256;
			break;
		case DATRAW_USHORT:
			strcpy(format, "USHORT");
			dataTypeSize = sizeof(short);
			range = 65536;
			break;
		default:
			std::cerr<<"Unsupported data type in "<<filename<<std::endl;
		}

		data = *data_ptr;
		delete data_ptr;
		data_ptr = NULL;
	}
};

#endif // VolumeReader_h