/*------------------------------------------------------------
* FileName: debug.c
* Author: linhb
* Date: 2014-12-02
------------------------------------------------------------*/

#include "debug.h"


#ifdef APP_DEBUG

#define LEN_DBGDATA         1024    // Maximum debug data length
#define MAX_CHARS           5       // Max characters per line in debug usage

static void GetBaseName(const uchar *pszFullPath, uchar *pszBaseName);

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
void DispHexMsg(const uchar *pszTitle, const uchar *psMsg, uint uiMsgLen, short nTimeOut)
{
	uint	i, iLineNum;
	GUI_PAGELINE	stBuff[100];
	GUI_PAGE		stHexMsgPage;

	memset(stBuff, 0, sizeof(stBuff));

	iLineNum = 0;
	// Format message
	uiMsgLen = MIN(uiMsgLen, LEN_DBGDATA);
	for (i=0; i<uiMsgLen; i+=MAX_CHARS)
	{
		if (uiMsgLen-i<MAX_CHARS)
		{
			DispHexLine(stBuff[iLineNum].szLine, i, psMsg+i, uiMsgLen-i);
		}
		else
		{
			DispHexLine(stBuff[iLineNum].szLine, i, psMsg+i, MAX_CHARS);
		}
		stBuff[iLineNum++].stLineAttr = gl_stLeftAttr;
	}   // end of for (pszBuff=

	Gui_CreateInfoPage(pszTitle, gl_stTitleAttr, stBuff, iLineNum, &stHexMsgPage);
	// Display message
	Gui_ClearScr();
	Gui_ShowInfoPage(&stHexMsgPage, FALSE, USER_OPER_TIMEOUT);
}

// print a line as hexadecimal format
int DispHexLine(uchar *pszBuffInOput, uint uiOffset, const uchar *psMsg, uint uiMsgLen)
{
	uint	i;
	uchar	*p = pszBuffInOput;

	// Print line information
	pszBuffInOput += sprintf((char *)pszBuffInOput, "%04Xh:", uiOffset);

	for (i=0; i<uiMsgLen; i++)
	{
		pszBuffInOput += sprintf((char *)pszBuffInOput, " %02X", psMsg[i]);
	}
	for (; i<MAX_CHARS; i++)
	{   // append blank spaces, if needed
		pszBuffInOput += sprintf((char *)pszBuffInOput, "   ");
	}

	return (pszBuffInOput-p);
}

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
// For Debug use, display file name and line
void DispAssert(const uchar *pszFileName, ulong ulLineNo)
{
	uchar	szFName[30];
	uchar	szBuff[200];

	GetBaseName(pszFileName, szFName);
	sprintf(szBuff, "FILE:%.11s\nLINE:%ld", szFName, ulLineNo);

	Gui_ClearScr();
	PubLongBeep();
	Gui_ShowMsgBox("Assert Failure", gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_OK, -1, NULL);
}

// get basename of a full path name
void GetBaseName(const uchar *pszFullPath, uchar *pszBaseName)
{
	uchar	*pszTmp;

	*pszBaseName = 0;
	if (!pszFullPath || !*pszFullPath)
	{
		return;
	}

	pszTmp = (uchar *)&pszFullPath[strlen((char *)pszFullPath)-1];
	while( pszTmp>=pszFullPath && *pszTmp!='\\' && *pszTmp!='/' )
	{
		pszTmp--;
	}
	sprintf((char *)pszBaseName, "%s", (char *)(pszTmp+1));
}

#endif
