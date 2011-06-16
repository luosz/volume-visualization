// textfile.h: interface for reading and writing text files
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//////////////////////////////////////////////////////////////////////

namespace file_utility
{
	/// read text from text file
	char *textFileRead(char *fn);

	/// write text to text file
	int textFileWrite(char *fn, char *s);

	//////////////////////////////////////////////////////////////////////////
	// The codes below are from textfile.cpp
	//////////////////////////////////////////////////////////////////////////

	// textfile.cpp
	//
	// simple reading and writing for text files
	//
	// www.lighthouse3d.com
	//
	// You may use these functions freely.
	// they are provided as is, and no warranties, either implicit,
	// or explicit are given
	//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

	/// read text files
	char *textFileRead(char *fn)
	{


		FILE *fp;
		char *content = NULL;

		int count=0;

		if (fn != NULL)
		{
			fp = fopen(fn,"rt");

			if (fp != NULL)
			{

				fseek(fp, 0, SEEK_END);
				count = ftell(fp);
				rewind(fp);

				if (count > 0)
				{
					content = (char *)malloc(sizeof(char) * (count+1));
					count = fread(content,sizeof(char),count,fp);
					content[count] = '\0';
				}
				fclose(fp);
			}
		}
		return content;
	}

	/// write text files
	int textFileWrite(char *fn, char *s)
	{

		FILE *fp;
		int status = 0;

		if (fn != NULL)
		{
			fp = fopen(fn,"w");

			if (fp != NULL)
			{

				if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
					status = 1;
				fclose(fp);
			}
		}
		return(status);
	}

}
