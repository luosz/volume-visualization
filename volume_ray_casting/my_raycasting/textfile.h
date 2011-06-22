/**	@file
*	A text file reader from
*	Hello World in GLSL
*	http://www.lighthouse3d.com/tutorials/glsl-tutorial/hello-world-in-glsl/
*/

// textfile.h: interface for reading and writing text files
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given

#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace file_utility
{
	char *textFileRead(char *fn);
	int textFileWrite(char *fn, char *s);

	// textfile.cpp
	//
	// simple reading and writing for text files
	//
	// www.lighthouse3d.com
	//
	// You may use these functions freely.
	// they are provided as is, and no warranties, either implicit,
	// or explicit are given

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

#endif // TEXTFILE_H
