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