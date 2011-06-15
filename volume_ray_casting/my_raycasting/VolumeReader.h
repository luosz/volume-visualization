#ifndef VolumeReader_h
#define VolumeReader_h

#include <cstdio>
#include <cctype>
#include <cstring>
#include <iostream>
#include <fstream>

#include "../BenBenRaycasting/Volume.h"
#include "../my_raycasting/reader.h"
#include "../my_raycasting/filename_utility.h"

namespace volume_utility
{
	/**	@brief	An abstract class for getting data from the volume
	*	
	*/
	struct AccessBase
	{
		virtual unsigned int getData(unsigned int index) = 0;
	};

	/**	@brief	A generic implementation of AccessBase 
	*	
	*/
	template <class T>
	struct AccessGeneric : AccessBase
	{
		T  datatransfered;

		AccessGeneric(void * d)
		{
			datatransfered = (T)d;
		}

		virtual unsigned int getData(unsigned int index)
		{
			return (unsigned int)datatransfered[index];
		}
	};

	/**	@brief	An adapter class for Volume
	*	
	*/
	class VolumeReader : public Volume
	{
	protected:
		AccessBase * accessor;

	public:

		VolumeReader(void)
		{
			accessor = NULL;
		}

		virtual ~VolumeReader(void)
		{
			if (accessor)
			{
				delete accessor;
			}
		}

		/// get the .raw file name from the .dat file automatically
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
						dataTypeSize = sizeof(unsigned char);
						range = 256;
					}
					else if(strstr(line, "USHORT"))
					{
						strcpy(format, "USHORT");
						printf("Get data's format: Unsigned Short\n");
						dataTypeSize = sizeof(unsigned short);
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
			//std::cout<<"VolumeReader::readVolFile - get the raw file path and filename"<<std::endl;
			char str[MAX_STR_SIZE];
			filename_utility::get_raw_filename_from_dat_filename(s, rawFilename, str);
			Volume::readData(str);

			if(strcmp(format, "UCHAR") == 0)
			{
				accessor = new AccessGeneric<unsigned char *>(data);
			}else
			{
				if(strcmp(format, "USHORT") == 0)
				{
					accessor = new AccessGeneric<unsigned short *>(data);	
				}else 
				{
					std::cerr<<"Unsupported data type in "<<s<<std::endl;
				}
			}
			//////////////////////////////////////////////////////////////////////////

			return true;
		}

		/// get data by the accessor
		virtual unsigned int getData(unsigned int x, unsigned int y, unsigned int z)
		{
			return accessor->getData(getIndex(x, y, z));
		}

		/// read volume data from file using readData in reader.h
		void readVolume_reader(char* filename)
		{
			using namespace volume_utility;

			void ** data_ptr = new void *;
			int sizes[3];
			int color_omponent_number;
			float dists[3];
			DataType type;
			volume_utility::readData(filename, sizes, dists, data_ptr, &type, &color_omponent_number);

			// Please set the properties of class volume to protected, in order to set their values.
			length = sizes[0];
			width = sizes[1];
			height = sizes[2];
			count = length * width * height;

			switch (type)
			{
			case DATRAW_UCHAR:
				strcpy(format, "UCHAR");
				dataTypeSize = sizeof(unsigned char);
				range = 256;
				accessor = new AccessGeneric<unsigned char *>(data);
				break;
			case DATRAW_USHORT:
				strcpy(format, "USHORT");
				dataTypeSize = sizeof(unsigned short);
				range = 65536;
				accessor = new AccessGeneric<unsigned short *>(data);
				break;
				//default:
				//std::cerr<<"Unsupported data type in "<<filename<<std::endl;
			}

			data = *data_ptr;
			delete data_ptr;
			data_ptr = NULL;
		}
	};

}

#endif // VolumeReader_h