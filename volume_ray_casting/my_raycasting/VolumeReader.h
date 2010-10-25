#ifndef VolumeReader_h
#define VolumeReader_h

#include <cstdio>
#include <cctype>
#include <cstring>

#include "../BenBenRaycasting/volume.h"
#include "reader.h"
#include "filename_utility.h"

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
	virtual bool readVolFile(char * s)
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
		std::cout<<"VolumeReader::readVolFile - get the raw file path and filename"<<std::endl;
		char str[MAX_STR_SIZE];
		filename_utility::get_raw_filename_from_dat_filename(s, rawFilename, str);
		volume::readData(str);
		//////////////////////////////////////////////////////////////////////////

		return true;
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