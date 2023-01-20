#include "global.h"

int adPinflag = 0;

int blockedTerminal()
{
	int iRet = 0, iMenuNo, iRev = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key = 0;
	int chk = 0;
	char pin[25] = {0};
	GUI_MENUITEM stDefTranMenuItem1[20] = {{0}};
	char txnName1[20][128];
	int iTemp = 0;
	char temp[5] = {0};
	char tempp[128] = {0};
	char sStore[100] = {0};
	char block[100] = {0};
	uchar k;

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 0;
	stInputAttr.nMinLen = 4;
	stInputAttr.nMaxLen = 20;
	stInputAttr.bSensitive = 1;

	memset(block, '\0',strlen(block));
	UtilGetEnv("block", block);
	if(strcmp(block, "true") == 0)
	{
		ShowLogs(1, "This device is locked");
	}else
		return 1;

	memset(pin, '\0', strlen(pin));
	Gui_ClearScr();
	iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("UNBLOCK PIN"), gl_stLeftAttr,
		pin, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK ){
		return 0;
	}
	memset(sStore, '\0', strlen(sStore));
	UtilGetEnv("blockedpin", sStore);

	ShowLogs(1, "pin: %s", pin);
	ShowLogs(1, "store blockedpin: %s", sStore);

	if(strncmp(pin, sStore, strlen(sStore)) != 0)
	{
		DisplayInfoNone("", "CONTACT ARCA", 2);
		return 0;
	}else
	{
		return 1;
	}
}

int endofdayMenu()
{
    int iRet = 0, iMenuNo, iRev = 0;
    ST_EVENT_MSG stEventMsg;
    uchar key = 0;
    int chk = 0;
    char pin[25] = {0};
    GUI_MENUITEM stDefTranMenuItem1[20] = {{0}};
    char txnName1[20][128];
    int iTemp = 0;
    char temp[5] = {0};
    char sStore[100] = {0};
    uchar k;

    GUI_MENU stTranMenu;
    GUI_MENUITEM stTranMenuItem[20];
    int iMenuItemNum = 0;
    int i;

    GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
    stTextAttr.eFontSize = GUI_FONT_SMALL;
    
    numLines = 6;
    
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "THIS DAY");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "THIS DAY", strlen("THIS DAY"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "ALL STORED");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "ALL STORED", strlen("ALL STORED"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "DATE RANGE");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "DATE RANGE", strlen("DATE RANGE"));

    for(i = 0; i < 3; ++i)
    {
        if(stDefTranMenuItem1[i].bVisible)
        {
            memcpy(&stTranMenuItem[iMenuItemNum], &stDefTranMenuItem1[i], sizeof(GUI_MENUITEM));
            sprintf(stTranMenuItem[iMenuItemNum].szText, "%s", stDefTranMenuItem1[i].szText);
            ++iMenuItemNum;
        }
    }

    stTranMenuItem[iMenuItemNum].szText[0] = 0;

    Gui_BindMenu("", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stTranMenuItem, &stTranMenu);
    
    Gui_ClearScr();
    iMenuNo = 0;
    iRet = Gui_ShowMenuList(&stTranMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iMenuNo);
    if(GUI_OK == iRet)
    {
        checkBoard = 0;
        if(strncmp(txnName1[iMenuNo], "THIS DAY", 8) == 0)
        {
            endofday();
        }else if(strncmp(txnName1[iMenuNo], "ALL STORED", 10) == 0)
        {
            eodCollection();
        }else if(strncmp(txnName1[iMenuNo], "DATE RANGE", 10) == 0)
        {
            daterangeeodCollection();
        }
        Gui_ClearScr();
        return iRet;
    }
    return 0;
}

int Reports()
{
    int iRet = 0, iMenuNo, iRev = 0;
    ST_EVENT_MSG stEventMsg;
    uchar key = 0;
    int chk = 0;
    char pin[25] = {0};
    GUI_MENUITEM stDefTranMenuItem1[20] = {{0}};
    char txnName1[20][128];
    int iTemp = 0;
    char temp[5] = {0};
    char tempp[128] = {0};
    char sStore[100] = {0};
    uchar k;

    GUI_MENU stTranMenu;
    GUI_MENUITEM stTranMenuItem[20];
    int iMenuItemNum = 0;
    int i;

    GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
    stTextAttr.eFontSize = GUI_FONT_SMALL;
    
    GUI_INPUTBOX_ATTR stInputAttr;

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.eType = GUI_INPUT_NUM;
    stInputAttr.bEchoMode = 0;
    stInputAttr.nMinLen = 4;
    stInputAttr.nMaxLen = 20;
    stInputAttr.bSensitive = 1;

    if(1)
    {
        memset(pin, '\0', strlen(pin));
        Gui_ClearScr();
        iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("MERCHANT PIN"), gl_stLeftAttr,
            pin, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
        if( iRet!=GUI_OK ){
            return 0;
        }
        memset(sStore, '\0', strlen(sStore));
        UtilGetEnv("adminpin", sStore);

        ShowLogs(1, "pin: %s", pin);
        ShowLogs(1, "store pin: %s", sStore);

        if(strncmp(pin, sStore, strlen(sStore)) != 0)
        {
            DisplayInfoNone("", "WRONG PIN", 2);
            return 0;
        }
    }

    adPinflag = 1;
    numLines = 6;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "RECENT RECEIPT");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "RECENT RECEIPT", strlen("RECENT RECEIPT"));
    key++;
    
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "ANY RECEIPT");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "ANY RECEIPT", strlen("ANY RECEIPT"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "END OF DAY");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "END OF DAY", strlen("END OF DAY"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "BY RRN");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "BY RRN", strlen("BY RRN"));
    key++;
    
    
    for(i = 0; i <= key; ++i)
    {
        if(stDefTranMenuItem1[i].bVisible)
        {
            memcpy(&stTranMenuItem[iMenuItemNum], &stDefTranMenuItem1[i], sizeof(GUI_MENUITEM));
            sprintf(stTranMenuItem[iMenuItemNum].szText, "%s", stDefTranMenuItem1[i].szText);
            ++iMenuItemNum;
        }
    }

    stTranMenuItem[iMenuItemNum].szText[0] = 0;

    Gui_BindMenu("", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stTranMenuItem, &stTranMenu);
    
    Gui_ClearScr();
    iMenuNo = 0;
    iRet = Gui_ShowMenuList(&stTranMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iMenuNo);
    if(GUI_OK == iRet)
    {
        checkBoard = 0;
        if(strncmp(txnName1[iMenuNo], "RECENT RECEIPT", 14) == 0)
        {   
            if(merchantPin() == 1)
            {
                reprintLast();
            }
        }else if(strncmp(txnName1[iMenuNo], "ANY RECEIPT", 11) == 0)
        {   
            if(merchantPin() == 1)
            {
                reprintAnyReceipt();
            }
        }else if(strncmp(txnName1[iMenuNo], "BY RRN", 6) == 0)
        {   
            if(merchantPin() == 1)
            {
                reprintByRrn();
            }
        }else if(strncmp(txnName1[iMenuNo], "END OF DAY", 10) == 0)
        {   
            if(merchantPin() == 1)
            {
                endofdayMenu();
            }
        }
        Gui_ClearScr();
        return iRet;
    }
    adPinflag = 0;
    return 0;
}

int adminFunc()
{
	int iRet = 0, iMenuNo, iRev = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key = 0;
	int chk = 0;
	char pin[25] = {0};
	GUI_MENUITEM stDefTranMenuItem1[20] = {{0}};
	char txnName1[20][128];
	int iTemp = 0;
	char temp[5] = {0};
	char tempp[128] = {0};
	char sStore[100] = {0};
	uchar k;
    uchar outputData[10240] = {0};
	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 0;
	stInputAttr.nMinLen = 4;
	stInputAttr.nMaxLen = 20;
	stInputAttr.bSensitive = 1;

	if(1)
	{
		memset(pin, '\0', strlen(pin));
		Gui_ClearScr();
		iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("MERCHANT PIN"), gl_stLeftAttr,
			pin, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if( iRet!=GUI_OK ){
			return 0;
		}
		memset(sStore, '\0', strlen(sStore));
		UtilGetEnv("adminpin", sStore);

		ShowLogs(1, "pin: %s", pin);
		ShowLogs(1, "store pin: %s", sStore);

		if(strncmp(pin, sStore, strlen(sStore)) != 0)
		{
			DisplayInfoNone("", "WRONG PIN", 2);
			return 0;
		}
	}

	adPinflag = 1;
	numLines = 6;
	
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "SYNC WITH TMS");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "SYNC WITH TMS", strlen("SYNC WITH TMS"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "KEY DOWNLOAD");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "KEY DOWNLOAD", strlen("KEY DOWNLOAD"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "IDLE INTERVAL");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "IDLE INTERVAL", strlen("IDLE INTERVAL"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "PRINT DETAILS");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "PRINT DETAILS", strlen("PRINT DETAILS"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "RECEIPT LOGO");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "RECEIPT LOGO", strlen("RECEIPT LOGO"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "BACKGROUND LOGO");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "BACKGROUND LOGO", strlen("BACKGROUND LOGO"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "REMOTE UPDATE");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "REMOTE UPDATE", strlen("REMOTE UPDATE"));
    key++;
    
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "HOST SETTINGS");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "HOST SETTINGS", strlen("HOST SETTINGS"));
    key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "TERMINAL RESET");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "TERMINAL RESET", strlen("TERMINAL RESET"));    
	key++;

	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "TERMINAL LOGS");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "TERMINAL LOGS", strlen("TERMINAL LOGS"));    
	key++;
    
	for(i = 0; i <= key; ++i)
    {
        if(stDefTranMenuItem1[i].bVisible)
        {
        	memcpy(&stTranMenuItem[iMenuItemNum], &stDefTranMenuItem1[i], sizeof(GUI_MENUITEM));
            sprintf(stTranMenuItem[iMenuItemNum].szText, "%s", stDefTranMenuItem1[i].szText);
            ++iMenuItemNum;
        }
    }

    stTranMenuItem[iMenuItemNum].szText[0] = 0;

	Gui_BindMenu("", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stTranMenuItem, &stTranMenu);
	
	Gui_ClearScr();
	iMenuNo = 0;
	iRet = Gui_ShowMenuList(&stTranMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iMenuNo);
	if(GUI_OK == iRet)
	{
		checkBoard = 0;
        if(strncmp(txnName1[iMenuNo], "SYNC WITH TMS", 13) == 0)
		{
			downProfile();
		}else if(strncmp(txnName1[iMenuNo], "KEY DOWNLOAD", 12) == 0)
        {
            //downloadKeys();
            KeyProfile();
            
            if(GetMasterKey() == 1)
            {
            DisplayInfoNone("", "TMK OK", 3);
            if(GetSessionKey() == 1)
            {
            DisplayInfoNone("", "TSK OK", 3);
            if(GetPinKey() == 1)
            {
            DisplayInfoNone("", "TPK OK", 3);
            if(GetParaMeters() == 1)
            {
            DisplayInfoNone("", "PARAMS OK", 3);
            }else
            {
            DisplayInfoNone("", "PARAMS FAILED", 3);
            memset(outputData, '\0', strlen(outputData));
            iRet = ReadAllData("param.txt", outputData);
            if(iRet == 0)
            {
            if(parseParametersOld(outputData))
            {
                ShowLogs(1, "Parameters Parse successful");
            }
            }
            }
            }else
            DisplayInfoNone("", "TPK FAILED", 2);
            }else
            DisplayInfoNone("", "TPK FAILED", 2);
            }else
            DisplayInfoNone("", "TMK FAILED", 2);

            /////PRIVATE SIMS
        }else if(strncmp(txnName1[iMenuNo], "IDLE INTERVAL", 13) == 0)
        {
            callHomeSettings();
        }else if(strncmp(txnName1[iMenuNo], "TERMINAL LOGS", 13) == 0)
		{
			if(DisplayLogsCheck)
			{
				DisplayInfoNone("", "TURN LOGS OFF?", 1);
			}else
			{
				DisplayInfoNone("", "TURN LOGS ON?", 1);
			}
			kbflush();
			while(1)
			{
				if( 0==kbhit() )
				{
					Beep();
					k = getkey();
					if(KEYCANCEL == k)
					{
						break;
					}else if(KEYENTER == k)
					{
						if(DisplayLogsCheck)
							DisplayLogsCheck = 0;
						else
							DisplayLogsCheck = 1;
					}
					break;
				}
				DelayMs(200);
			}
		}else if(strncmp(txnName1[iMenuNo], "RECEIPT LOGO", 12) == 0)
		{
			logoDownload(1);
		}else if(strncmp(txnName1[iMenuNo], "BACKGROUND LOGO", 15) == 0)
		{
			logoDownload(0);
		}else if(strncmp(txnName1[iMenuNo], "REMOTE UPDATE", 13) == 0)
		{
			checkForNewApp();
		}else if(strncmp(txnName1[iMenuNo], "HOST SETTINGS", 13) == 0)
        {
            hostSettings();
        }else if(strncmp(txnName1[iMenuNo], "TERMINAL RESET", 14) == 0)
		{
			uchar k;
            if(merchantPin() == 1)
            {
                DisplayInfoNone("", "RESET DEVICE?", 0);
                kbflush();
                while(1)
                {
                    if( 0==kbhit() )
                    {
                        Beep();
                        k = getkey();
                        if(KEYCANCEL == k)
                        {
                            break;
                        }else if(KEYENTER == k)
                        {
                            DisplayInfoNone("", "TMS SYNC", 2);
                            PackCallHomeData();
                            CreateWrite("receipt.txt", "0");
                            CreateWrite("reprint.txt", "0");
                            CreateWrite("tlog.txt", "0");
                            CreateWrite("eod.txt", "0");
                            CreateWrite("count.txt", "0");
                            CreateWrite("TRANLOG.DAT", "");
                            DisplayInfoNone("", "TERMINAL RESET \nOK...", 2);
                        }
                        break;
                    }
                    DelayMs(200);
                }
            }
		}
		else if(strncmp(txnName1[iMenuNo], "PRINT DETAILS", 13) == 0)
		{
			printDetails();
		}
		Gui_ClearScr();
		return iRet;
	}
	adPinflag = 0;
	return 0;
}