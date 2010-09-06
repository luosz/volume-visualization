#pragma once

#include <cstdio>
#include <cctype>
#include <cstring>
using namespace std;

#include "volume.h"
#include "reader.h"

#define STR_BUFFER_SIZE 320

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
	void readVolume_original(char* filename = NULL)
	{
		readVolFile(filename);
		char str[STR_BUFFER_SIZE];
		strcpy(str, filename);
		int length = strlen(str);
		if (length > 3 && tolower(str[length - 3])=='d' && tolower(str[length - 2])=='a' && tolower(str[length - 1])=='t')
		{
			str[length - 3] = 'r';
			str[length - 2] = 'a';
			str[length - 1] = 'w';
			readData(str);
		}else
		{
			throw std::exception(filename);
		}
	}

	// read volume data from file using readData in reader.h
	void readVolume_new(char* filename)
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
			char str[STR_BUFFER_SIZE];
			sprintf(str, "Unsupported data type in %s!", filename);
			throw std::exception(str);
		}

		data = *data_ptr;
		delete data_ptr;
		data_ptr = NULL;
	}
};
