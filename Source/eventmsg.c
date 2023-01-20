
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// When the first time called by manager, the msg type must be USER_MSG
// Modified by Kim_LinHB 2014-6-8
int ProcUserMsg(void)
{
	int		iRet;
	uchar	szEngTime[17+1];
	uchar	ucAcqIndex;

	ShowLogs(1, "Comms type: %d", glSysParam.stTxnCommCfg.ucCommType);
	// no need to init module if running as minor sub-app
	if( glSysParam.stTxnCommCfg.ucCommType==CT_GPRS  ||
		glSysParam.stTxnCommCfg.ucCommType==CT_CDMA  ||
		glSysParam.stTxnCommCfg.ucCommType==CT_WCDMA  || // added by  Gillian 2015/11/23
		glSysParam.stTxnCommCfg.ucCommType==CT_BLTH  || // Added by Kim_LinHB 2014-08-18 v1.01.0004
		glSysParam.stTxnCommCfg.ucCommType==CT_TCPIP ||
		glSysParam.stTxnCommCfg.ucCommType==CT_WIFI)
	{
		GetEngTime(szEngTime);

		ShowLogs(1, "GetEngTime: %s", szEngTime);

		Gui_ClearScr();

		Gui_ShowMsgBox(szEngTime, gl_stTitleAttr, _T("Please Wait"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);

		if (iRet!=0)
		{
			unsigned char szBuff[200];
			ShowLogs(1, "Comms Initialization failed.");
			CommOnHook(TRUE);
			strcpy(szBuff, _T("INIT FAIL"));
			strcat(szBuff, "\n");
			if (glSysParam.stTxnCommCfg.ucCommType==CT_GPRS  ||
				glSysParam.stTxnCommCfg.ucCommType==CT_CDMA  ||
				glSysParam.stTxnCommCfg.ucCommType==CT_WCDMA)
			{
				strcat(szBuff, _T("PLS CHECK SIM OR\nHARDWARE/SIGNAL."));
			}
			else if (glSysParam.stTxnCommCfg.ucCommType==CT_TCPIP)
			{
				strcat(szBuff, _T("PLS CHECK CABLE\nOR CONFIG."));
			}
			else if (glSysParam.stTxnCommCfg.ucCommType==CT_WIFI)
			{
				strcat(szBuff, _T("PLS CHECK SIGNAL\nOR CONFIG."));
			}
			Gui_ClearScr();
			Gui_ShowMsgBox(szEngTime, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, -1, NULL);
		}
	}
	// Use a new method to communicates with app-manager

	// erase transaction log of last settlement
	for(ucAcqIndex=0; ucAcqIndex<glSysParam.ucAcqNum; ucAcqIndex++)
	{
		ShowLogs(1, "Clearing Record");
		if( glSysCtrl.sAcqStatus[ucAcqIndex]==S_CLR_LOG )
		{
			ClearRecord(glSysParam.stAcqList[ucAcqIndex].ucKey);
		}
	}

	return 0;
}

// Process magcard swipe event
int ProcMagMsg(void)
{
	int		iRet;

	// Modified by Kim_LinHB 2014-8-8 v1.01.0002 bug506
	//glProcInfo.stTranLog.ucTranType = SALE_OR_AUTH;
	TransInit(SALE_OR_AUTH);

	iRet = SwipeCardProc(TRUE);
	if( iRet!=0 )
	{
		return ERR_NO_DISP;
	}

	iRet = ValidCard();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = TransCapture();
	if( iRet!=0 )
	{
		CommOnHook(FALSE);
		return iRet;
	}

	return 0;
}

// Process IC card insertion event
int ProcICCMsg(void)
{
	int		iRet;
	ShowLogs(1, "Inside ProcIccMsg");
	// Modified by Kim_LinHB 2014-8-8 v1.01.0002 bug506
	// glProcInfo.stTranLog.ucTranType = SALE_OR_AUTH;
	//TransInit(SALE_OR_AUTH); //By wisdom
	TransInitPurchase(SALE);

	//EMVSetTLVData(0x9F35, (uchar *)"\x22", 1);//Instead of 34

	amtCount = 0;
	iRet = GetCard(SKIP_DETECT_ICC|CARD_INSERTED); //Amount is here
	ShowLogs(1, "Inside ProcICCMsg");
	if( iRet!=0 )
	{
		return iRet;
	}

	ShowLogs(1, "ProcICCMsg. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "ProcICCMsg. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "ProcICCMsg. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	//Fine until here

	ShowLogs(1, "1. Card Details Gotten");
	iRet = TransCapture();//Problem is here. Open it up Please
	ShowLogs(1, "2. Card Details Gotten: %d", iRet);

	if( iRet!=0 )
	{
		ShowLogs(1, "3. Card Details Gotten");
		CommOnHook(FALSE);
		return iRet;
	}

	return 0;
}



void getName2(char *data, int *len, int key, char **txnAtm)
{
    int iLen = 0;
	int i, j = 0;
	char name[128] = {0};
	iLen = strlen(data);
	for(i = 0; i < iLen; i++)
    {
        if(data[i] == '#')
        {
            if(data[i + 1] == '#')
            {
                if(data[i + 2] == '#')
                {
                    break;
                }
            }
        }else
        {
            name[j] = data[i];
            j++;
        }
    }
    ShowLogs(1, "Transaction Name: %s", name);
    strncpy(txnAtm[key], name, strlen(name));
}

int parseTxn2(char *data, char **txnAtm)
{
	int iLen = 0;
	int i, j = 0, key = 0;
	char name[128] = {0};
	iLen = strlen(data);
	char *ret;
	for(i = 0; i < iLen; i++)
    {
    	ret = strstr(data + i, "name - ");
    	if(ret)
        {
            j = iLen - strlen(ret);
            getName2(ret + 7, &i, key, txnAtm);
            i = j;
            key++;
        }
    }
    return key--;
}



void parseAtm(int key)
{
	int iRet = 0;
	int i;
	char txnAtm[20][128];

	for(i = 0; i < 20; i++)
	{
		memset(txnAtm[i], '\0', sizeof(txnAtm[i]));
	}
	i = key;
	if(strncmp(txnAtm[i], "Purchase", 8) == 0)
	{
		TransPurchase();
	}
}

// Process common key pressed event
int ProcKeyMsg(void)
{
	int		iRet, iFuncNo;
	int		iRet2;
	
	while(1)
	{
		iRet = TransOther();
		PromptRemoveICC();
		Gui_ClearScr();
	}
}

// end of file

