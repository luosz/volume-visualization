#ifndef reader_tag_h
#define reader_tag_h

#include "../my_raycasting/reader.h"

namespace file_utility
{
	/// read volume data with tags
	void readData_with_tag(char *filename, int *sizes, float *dists, void **data, DataType *type, int *numComponents, char *tag_filename = NULL)
	{
		char *cp, line[100], rawFilename[100];
		int parseError, dataTypeSize = 0;
		size_t size;
		FILE *fp;

		if (! (fp = fopen(filename, "rb")))
		{
			perror("opening .dat file failed");
			exit(1);
		}

		parseError = 0;
		while (fgets(line, sizeof(line), fp))
		{
			if (strstr(line, "ObjectFileName"))
			{
				if (! (cp = strchr(line, ':')))
				{
					parseError = 1;
					break;
				}
				if (sscanf(cp + 1, "%s", rawFilename) != 1)
				{
					parseError = 1;
					break;
				}
			}
			else if (strstr(line, "Resolution"))
			{
				if (! (cp = strchr(line, ':')))
				{
					parseError = 1;
					break;
				}
				if (sscanf(cp + 1, "%i %i %i",
					&sizes[0], &sizes[1], &sizes[2]) != 3)
				{
					parseError = 1;
					break;
				}
			}
			else if (strstr(line, "SliceThickness"))
			{
				if (! (cp = strchr(line, ':')))
				{
					parseError = 1;
					break;
				}
				if (sscanf(cp + 1, "%f %f %f",
					&dists[0], &dists[1], &dists[2]) != 3)
				{
					parseError = 1;
					break;
				}
			}
			else if (strstr(line, "Format"))
			{
				if (strstr(line, "UCHAR"))
				{
					*type = DATRAW_UCHAR;
					*numComponents = 1;
					dataTypeSize = sizeof(char);
				}
				else if (strstr(line, "USHORT"))
				{
					*type = DATRAW_USHORT;
					*numComponents = 1;
					dataTypeSize = sizeof(short);
				}
				else if (strstr(line, "FLOAT"))
				{
					char *cp, temp[100];

					*type = DATRAW_FLOAT;
					dataTypeSize = sizeof(float);

					if (! (cp = strchr(line, ':')))
					{
						parseError = 1;
						break;
					}
					if (sscanf(cp + 1, "%s", temp) != 1)
					{
						parseError = 1;
						break;
					}

					cp = temp;
					while (*cp && ! isdigit(*cp))
					{
						cp++;
					}
					if (*cp)
					{
						if (sscanf(cp, "%i", numComponents) != 1)
						{
							parseError = 1;
							break;
						}
					}
					else
					{
						*numComponents = 1;
					}
				}
				else
				{
					fprintf(stderr, "cannot process data other than of "
						"UCHAR and FLOAT* format\n");
					exit(1);
				}
			}
			else
			{
				if (tag_filename != NULL && strstr(line, "TaggedFileName"))
				{
					if (! (cp = strchr(line, ':')))
					{
						parseError = 1;
						break;
					}
					if (sscanf(cp + 1, "%s", tag_filename) != 1)
					{
						parseError = 1;
						break;
					}
				}else
				{
					fprintf(stderr, "skipping line %s", line);
				}
			}
		}

		if (parseError)
		{
			fprintf(stderr, "parse error: %s\n", line);
			exit(1);
		}

		fclose(fp);

		if (! (fp = fopen(rawFilename, "rb")))
		{
			char temp[100];
			strcpy(temp, filename);
			if (! (cp = strrchr(temp, DIR_SEP)))
			{
				perror("opening .raw file failed");
				exit(1);
			}
			strcpy(cp + 1, rawFilename);
			if (! (fp = fopen(temp, "rb")))
			{
				perror("opening .raw file failed");
				exit(1);
			}
		}

		size = sizes[0] * sizes[1] * sizes[2];

		if (! (*data = malloc(size * *numComponents * dataTypeSize)))
		{
			fprintf(stderr, "not enough memory for volume data\n");
			exit(1);
		}

		if (fread(*data, dataTypeSize, size * *numComponents, fp) !=
			size * *numComponents)
		{
			fprintf(stderr, "reading volume data failed");
			exit(1);
		}

		fclose(fp);
	}

}

#endif // reader_tag_h
