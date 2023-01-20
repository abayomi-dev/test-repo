#include "global.h"

void schemeKeysDownload()
{
	char tempp[128] = {0};
	memset(tempp, '\0', strlen(tempp));
	UtilGetEnv("profilecardschemekeytypes", tempp);
	if(strstr(tempp, "live") != NULL)
	{
		DisplayInfoNone("", "LIVE KEYS", 5);
		//Do live scheme keys download here
	}else
	{
		DisplayInfoNone("", "TEST KEYS", 5);
	}
}
