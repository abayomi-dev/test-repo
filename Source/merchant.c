#include "global.h"

void setupPin()
{
	int iRet;
	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	char pin1[25] = {0};
	char pin2[25] = {0};
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


	if(fexist("merchantpin.txt") >= 0)
	{
		Beep();
		DisplayInfoNone("", "PIN ALREADY SET", 2);
		return;
	}


	memset(pin1, '\0', strlen(pin1));
	Gui_ClearScr();
	iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("INPUT PIN"), gl_stLeftAttr,
		pin1, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		Beep();
		DisplayInfoNone("", "PIN NOT SET", 2);
		return;
	}

	memset(pin2, '\0', strlen(pin2));
	Gui_ClearScr();
	iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("RETYPE PIN"), gl_stLeftAttr,
		pin2, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		Beep();
		DisplayInfoNone("", "PIN NOT SET", 2);
		return;
	}

	if((strncmp(pin1, pin2, strlen(pin2)) == 0) && (strlen(pin1) == strlen(pin2)))
	{
		CreateWrite("merchantpin.txt", pin2);
		DisplayInfoNone("", "MERCHANT PIN SET", 2);
		return;
	}else
	{
		Beep();
		DisplayInfoNone("", "MERCHANT PIN MISMATCH", 2);
		return;
	}
}

void setupPin2()
{
	int iRet;
	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	char pin1[25] = {0};
	char pin2[25] = {0};
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


	memset(pin1, '\0', strlen(pin1));
	Gui_ClearScr();
	iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("INPUT PIN"), gl_stLeftAttr,
		pin1, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		Beep();
		DisplayInfoNone("", "PIN NOT SET", 2);
		return;
	}

	memset(pin2, '\0', strlen(pin2));
	Gui_ClearScr();
	iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("RETYPE PIN"), gl_stLeftAttr,
		pin2, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		Beep();
		DisplayInfoNone("", "PIN NOT SET", 2);
		return;
	}

	if((strncmp(pin1, pin2, strlen(pin2)) == 0) && (strlen(pin1) == strlen(pin2)))
	{
		CreateWrite("merchantpin.txt", pin2);
		DisplayInfoNone("", "MERCHANT PIN SET", 2);
		return;
	}else
	{
		Beep();
		DisplayInfoNone("", "MERCHANT PIN MISMATCH", 2);
		return;
	}
}

void setMerchantPin()
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
	int l = 0;
	uchar k;

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	//numLines = 6;
	
	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "SELECT PIN");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "SELECT PIN", strlen("SELECT PIN"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "CHANGE PIN");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "CHANGE PIN", strlen("CHANGE PIN"));

    if(fexist("merchantpin.txt") >= 0)
		l = 2;
	else
		l = 1;

    for(i = 0; i < l; ++i)
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
		if(strncmp(txnName1[iMenuNo], "SELECT PIN", 10) == 0)
		{
			setupPin();
			setMerchantPin();
		}else if(strncmp(txnName1[iMenuNo], "CHANGE PIN", 10) == 0)
        {
            if(merchantPin() == 1)
            	setupPin2();
            setMerchantPin();
        }
		Gui_ClearScr();
		return iRet;
	}


}

int merchantPin()
{
	int iRet;
	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	char pin[25] = {0};
	char mpin[25] = {0};
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

	if(fexist("merchantpin.txt") >= 0)
	{
		memset(pin, '\0', strlen(pin));
		Gui_ClearScr();
		iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("Security Pin"), gl_stLeftAttr,
			pin, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if( iRet!=GUI_OK )
		{
			return 0;
		}

		memset(mpin, '\0', strlen(mpin));
		iRet = ReadAllData("merchantpin.txt", mpin);
		ShowLogs(1, "Merchant Pin - %s: Length: %d..... Entered Pin - %s: Length: %d", mpin, strlen(mpin), pin, strlen(pin));
		if(strncmp(pin, mpin, strlen(pin)) != 0)
		{
			Beep();
			DisplayInfoNone("", "Wrong Pin", 2);
			return 0;
		}else
		{
			return 1;
		}
	}else
	{
		Beep();
		DisplayInfoNone("", "Transaction Key Empty", 2);
		return 0;
	}
}