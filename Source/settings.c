#include "global.h"

int adminTid = 0;

int checkTidPin()
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

	memset(pin, '\0', strlen(pin));
	Gui_ClearScr();
	iRet = Gui_ShowInputBox("", gl_stTitleAttr, _T("TID PIN"), gl_stLeftAttr,
		pin, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		return 0;
	}

	memset(mpin, '\0', strlen(mpin));
	ShowLogs(1, "Tid Pin: %s", tidIso.tidpin);
	strcpy(mpin, tidIso.tidpin);
	ShowLogs(1, "Tid Pin - %s: Length: %d..... Entered Pin - %s: Length: %d", mpin, strlen(mpin), pin, strlen(pin));
	if(strncmp(pin, mpin, strlen(pin)) != 0)
	{
		Beep();
		DisplayInfoNone("", "Tid Pin Failed", 2);
		return 0;
	}else
	{
		return 1;
	}
}

void changeProfileID()
{
	char serverIp[20] = {0};
	char serverPort[20] = {0};
	char serverIpStore[20] = {0};
	char serverPortStore[20] = {0};
	char tid[20] = {0};
	char tidst[20] = {0};
	char term[100] = {0};
	int iRet;

	memset(tid, '\0', strlen(tid));
	memset(term, '\0', strlen(term));
	
	UtilGetEnvEx("tid", tid);

	memset(serverIpStore, '\0', strlen(serverIpStore));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("tcmIP", term);
	iRet = DisplayMsg("", "TMS IP", term, serverIpStore, 7, 29);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}

	memset(serverPortStore, '\0', strlen(serverPortStore));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("tcmPort", term);
	iRet = DisplayMsg("", "TMS PORT", term, serverPortStore, 4, 6);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	memset(serverIp, '\0', strlen(serverIp));
	parseParameter(serverIpStore, serverIp);
	ShowLogs(1, "Parsed Ip: %s", serverIp);
	memset(serverPort, '\0', strlen(serverPort));
	parseParameter(serverPortStore, serverPort);
	ShowLogs(1, "Parsed Port: %s", serverPort);
	memset(tidst, '\0', strlen(tidst));
	parseParameter(tid, tidst);
	ShowLogs(1, "Parsed Tid: %s", tidst);
	
	UtilPutEnv("tIPaddr", serverIp);
	UtilPutEnv("tPort", serverPort);
	UtilPutEnv("tTermi", tidst);
	//Ok
	adminTid = 1;
	OldProfile();
}

void downProfile()
{
	ShowLogs(1, "Inside Download Profile 2");
	OldProfile();
}

void downloadKeys()
{
	uchar outputData[10240] = {0};
	int iRet;
	char lasthost[128] = {0};
	char temp[128] = {0};

	if(GetMasterKey() == 1)
	{
		DisplayInfoNone("", "TMK OK", 2);
		if(GetSessionKey() == 1)
		{
			DisplayInfoNone("", "TSK OK", 2);
			if(GetPinKey() == 1)
			{
				DisplayInfoNone("", "TPK OK", 2);
				if(GetParaMeters() == 1)
				{
					DisplayInfoNone("", "PARAMS OK", 2);
				}else
				{
					DisplayInfoNone("", "PARAMS FAILED", 2);
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
			DisplayInfoNone("", "TSK FAILED", 2);
	}else
		DisplayInfoNone("", "TMK FAILED", 2);

	CommOnHookCustom(TRUE);

}

void callHomeSettings()
{
	char callhome[20] = {0};
	char term[100] = {0};
	int iRet;
	memset(callhome, '\0', strlen(callhome));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("chinterval", term);
	iRet = DisplayMsg("", "IDLE INTERVALS", term, callhome, 2, 6);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	UtilPutEnv("chinterval", callhome);

	memset(callhome, '\0', strlen(callhome));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("chremarks", term);
	iRet = DisplayMsg("", "COUNTS", term, callhome, 1, 6);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	UtilPutEnv("chremarks", callhome);
	DisplayInfoNone("", "SETTINGS SAVED", 2);
}

void networkSettings()
{
	char apn[40] = {0};
	char username[40] = {0};
	char password[40] = {0};
	char apnStore[40] = {0};
	char usernameStore[40] = {0};
	char passwordStore[40] = {0};
	char term[100] = {0};
	int iRet;

	memset(apn, '\0', strlen(apn));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("coapn", term);
	iRet = DisplayMsg("", "APN", term, apn, 0, 39);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}

	memset(username, '\0', strlen(username));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("cosubnet", term);
	iRet = DisplayMsg("", "USERNAME", term, username, 0, 39);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}

	memset(password, '\0', strlen(password));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("copwd", term);
	iRet = DisplayMsg("", "PASSWORD", term, password, 0, 39);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	memset(apnStore, '\0', strlen(apnStore));
	parseParameter(apn, apnStore);
	memset(usernameStore, '\0', strlen(usernameStore));
	parseParameter(username, usernameStore);
	memset(passwordStore, '\0', strlen(passwordStore));
	parseParameter(password, passwordStore);
	UtilPutEnv("coapn", apnStore);
	UtilPutEnv("cosubnet", usernameStore);
	UtilPutEnv("copwd", passwordStore);
	DisplayInfoNone("", "SETTINGS SAVED", 2);
}

void hostSettings()
{
	char ip[40] = {0};
	char hostname[100] = {0};
	char port[40] = {0};
	char ipStore[40] = {0};
	char portStore[40] = {0};
	char hostnameStore[100] = {0};
	char term[100] = {0};
	char sStore[100] = {0};
	int iRet;
	uchar k;

	memset(hostname, '\0', strlen(hostname));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("hostfname", term);
	iRet = DisplayMsg("", "NAME", term, hostname, 6, 90);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	UtilPutEnv("hostfname", hostname);

	memset(ip, '\0', strlen(ip));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("hostip", term);
	iRet = DisplayMsg("", "IP", term, ip, 6, 39);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	UtilPutEnv("hostip", ip);
	UtilPutEnv("uhostip", ip);

	memset(port, '\0', strlen(port));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("hostport", term);
	iRet = DisplayMsg("", "Port", term, port, 1, 6);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	UtilPutEnv("hostport", port);
	UtilPutEnv("uhostport", port);

	memset(sStore, '\0', strlen(sStore));
	UtilGetEnv("hostssl", sStore);
	if(strstr(sStore, "true") != NULL)
	{
		DisplayInfoNone("", "TURN OFF SSL?", 1);
	}else
	{
		DisplayInfoNone("", "TURN ON SSL?", 1);
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
				UtilPutEnv("uhostssl", sStore);
				//DisplayInfoNone("", "Settings Not Saved", 2);
				//DisplayInfoNone("", "Settings Saved", 2);
				//break;
			}else if(KEYENTER == k)
			{
				if(strstr(sStore, "true") != NULL)
				{
					UtilPutEnv("hostssl", "false");
					UtilPutEnv("uhostssl", "false");
				}
				else
				{
					UtilPutEnv("hostssl", "true");
					UtilPutEnv("uhostssl", "true");
				}
			}
			DisplayInfoNone("", "Settings Saved", 2);
			break;
		}
		DelayMs(200);
	}


	


}

void reprintAnyReceipt()
{
	char number[4] = {0};
	int iRet;
	memset(number, '\0', strlen(number));
	iRet = DisplayMsg("", "PRINTER NUMBER?", "1", number, 1, 10);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	ShowLogs(1, "Number: %s", number);
	reprintAny(number);
}

void reprintByRrn()
{
	char number[13] = {0};
	char send[20] = {0};
	int iRet;
	memset(number, '\0', strlen(number));
	iRet = DisplayMsg("", "RECEIPT RRN?", "0", number, 12, 12);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	ShowLogs(1, "RRN: %s", number);
	strcpy(send, "|");
	strcat(send, number);
	strcat(send, "|");
	ShowLogs(1, "GOING RRN: %s", send);
	reprintRrn(send);
}

void tcmSettings()
{
	char ip[100] = {0};
	char port[40] = {0};
	char term[100] = {0};
	int iRet;

	memset(ip, '\0', strlen(ip));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("tcmIP", term);
	iRet = DisplayMsg("", "TMS IP", term, ip, 6, 90);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	UtilPutEnv("tcmIP", ip);

	memset(port, '\0', strlen(port));
	memset(term, '\0', strlen(term));
	UtilGetEnvEx("tcmPort", port);
	iRet = DisplayMsg("", "TMS PORT", term, port, 4, 6);
	if(iRet == GUI_ERR_USERCANCELLED)
	{
		Beep();
		DisplayInfoNone("", "CANCELED", 2);
		return;
	}
	UtilPutEnv("tcmPort", port);
	DisplayInfoNone("", "SETTINGS SAVED", 2);
}

void commsType()
{
	int iRet = 0, iMenuNo, iRev = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key = 0;
	GUI_MENUITEM stDefTranMenuItem1[20] = {{0}};
	char txnName1[20][128];
	int iTemp = 0;
	char temp[5] = {0};
	char sStore[20] = {0};

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;

	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "GPRS");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "GPRS", strlen("GPRS"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "WIFI");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "WIFI", strlen("WIFI"));

	for(i = 0; i < 2; ++i)
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
		if(strncmp(txnName1[iMenuNo], "GPRS", 4) == 0)
		{
			UtilPutEnv("cotype", "GPRS\n");
			networkSettings();
			commsType();
		}else if(strncmp(txnName1[iMenuNo], "WIFI", 4) == 0)
		{
			UtilPutEnv("cotype", "WIFI\n");
			while(1)
			{
				if(wifiSetup())
					break;
				if(cancelWifi == 1)
				{
					UtilPutEnv("cotype", "GPRS\n");
					UtilPutEnv("cosubnet", "web\n");
					UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
					UtilPutEnv("copwd", "web\n");
					DisplayInfoNone("", "OK", 1);
					break;
				}
			}
			commsType();
		}
		Gui_ClearScr();
		return;
	}
}

void commsSettings()
{
	int iRet = 0, iMenuNo, iRev = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key = 0;
	uchar k;
	GUI_MENUITEM stDefTranMenuItem1[20] = {{0}};
	char txnName1[20][128];
	int iTemp = 0;
	char temp[5] = {0};
	char sStore[20] = {0};

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;

	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "HOST");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "HOST", strlen("HOST"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "SSL");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "SSL", strlen("SSL"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "COMMS TYPE");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "COMMS TYPE", strlen("COMMS TYPE"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "CALL HOME HOST");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "CALL HOME HOST", strlen("CALL HOME HOST"));


	for(i = 0; i < 4; ++i)
    {
        if(stDefTranMenuItem1[i].bVisible)
        {
        	memcpy(&stTranMenuItem[iMenuItemNum], &stDefTranMenuItem1[i], sizeof(GUI_MENUITEM));
            sprintf(stTranMenuItem[iMenuItemNum].szText, "%s", stDefTranMenuItem1[i].szText);
            ++iMenuItemNum;
        }
    }

    stTranMenuItem[iMenuItemNum].szText[0] = 0;
	Gui_BindMenu("COMMS SETTINGS", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stTranMenuItem, &stTranMenu);
	Gui_ClearScr();
	iMenuNo = 0;
	iRet = Gui_ShowMenuList(&stTranMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iMenuNo);
	if(GUI_OK == iRet)
	{
		checkBoard = 0;
		if(strncmp(txnName1[iMenuNo], "HOST", 4) == 0)
		{
			hostSettings();
			commsSettings();
		}else if(strncmp(txnName1[iMenuNo], "SSL", 3) == 0)
		{
			memset(sStore, '\0', strlen(sStore));
			UtilGetEnv("hostssl", sStore);
			if(strstr(sStore, "true") != NULL)
			{
				DisplayInfoNone("", "TURN OFF SSL?", 1);
			}else
			{
				DisplayInfoNone("", "TURN ON SSL?", 1);
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
						if(strstr(sStore, "true") != NULL)
							UtilPutEnv("hostssl", "false");
						else
							UtilPutEnv("hostssl", "true");
					}
					break;
				}
				DelayMs(200);
			}
			commsSettings();
		}else if(strncmp(txnName1[iMenuNo], "COMMS TYPE", 10) == 0)
		{
			commsType();
			commsSettings();
		}else if(strncmp(txnName1[iMenuNo], "CALL HOME HOST", 14) == 0)
		{
			tcmSettings();
			commsSettings();
		}
		Gui_ClearScr();
		return;
	}
}