// textfile.h: interface for reading and writing text files
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//////////////////////////////////////////////////////////////////////

/**	@brief read text files named fn
*	
*/
char *textFileRead(char *fn);

/**	@brief write text files named fn
*	
*/
int textFileWrite(char *fn, char *s);
