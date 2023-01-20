
#include "global.h"



/********************** Internal macros declaration ************************/
#define TIMER_TEMPORARY		4
#define TIMERCNT_MAX		48000

//#define TIMER_TEMPORARY		1
//#define TIMERCNT_MAX		1000

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static int   main_sub(const ST_EVENT_MSG *pstEventMsg);
static int   CheckTermSatus(const ST_EVENT_MSG *pstEventMsg);
static void  FirstRunProc(const ST_EVENT_MSG *pstEventMsg);
static void  SetIdleTimer(void);
static uchar ChkIdleTimer(int iSeconds);
static void  ShowIdleMainUI(uchar *pbForceUpdate, uchar bShowGallery, int iGallery_Image_Num);
static void  ShowGallery(int num);
static int   CheckInitTerminal(void);
static void  MainIdleProc(void);
int txnType = 0;
int emvcore = 0;
int magstp = 0;
int pincheck = 0;
/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/
extern int event_main(ST_EVENT_MSG *pstEventMsg);

const APPINFO AppInfo =
{
	APP_NAME,
	EDCAPP_AID,
	EDC_VER_INTERN _TERMTYPE_,
	"ARCAPAY",
	"ARCAPAY",
	"",
	0xFF,
	0xFF,
	0x01,
	""
};

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/


int event_main(ST_EVENT_MSG *pstEventMsg)
{
	ShowLogs(1, "Inside Event");
	glEdcMsgPtr = pstEventMsg;	// save the event pointer 
	return main_sub(pstEventMsg);
}

int main(void)
{
	uchar			bUpdateUI, bGallery = FALSE;
	int				iRet, i, j = 0;
	ST_EVENT_MSG	stEventMsg; //For event handling
	int iGallery_Image_Num = 0;
	uchar szGallery_Image_Num[5]= {0};
	char store_env[120] = {0};
	uchar outputData[10240] = {0};
	uchar	szEngTime[16+1];
	uchar key;
	int chk = 0;
#ifdef APP_DEBUG
 	ST_KEY_INFO     stTmp_Key;
 	ST_KCV_INFO		stTmp_Kcv;
#endif

 	//Beep();
 	//ScrTextOut(0, 0, (unsigned char *)"Love is golden");
 	//DelayMs(20000);
 	//Gui_ClearScr();
	//CLcdTextOut(0, 0, (unsigned char *)"Love is golden");
	//DelayMs(20000);
	//Gui_ClearScr();

 	/*Gui_ClearScr();
	Beep();
	ScrGotoxy(0, 0);
    Lcdprintf("Error:%s error\nPls download ParamFile", "Checking");
    DelayMs(5000);

    Gui_ClearScr();
	Beep();
    CLcdTextOut(0, 0, (char *)"Christ is Permanent");
    DelayMs(5000);
    Beep();

    ScrCls(); 
    ScrFontSet(ASCII); 
    Lcdprintf("Pax Technology Ltd."); 
    ScrGotoxy(0, 2); 
    ScrFontSet(CFONT); 
    Lcdprintf("PAX Computer Technology company \nAugust 2001\n");*/


 	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

 	//Logging starts after here
 	memset(store_env, '\0', strlen(store_env));

 	if(UtilGetEnv("logs", store_env))
 	{
 		iRet = atoi(store_env);
 		if(iRet == 1)
 			DisplayLogsCheck = 1;
 		else
 			DisplayLogsCheck = 0;
 		ShowLogs(1, "Logs Gotten: %s", store_env);
 	}else
 	{
 		DisplayLogsCheck = 1;
 		ShowLogs(1, "Could not get value. Logging Set.");
 		UtilPutEnv("logs", "1");
 	}

 	DisplayLogsCheck = 0;//For logs - 0 for Production. 1 for debug
 	
 	//Optimization
 	if(DisplayLogsCheck)
		//DisplayLogsCheck = 0;//Release
		DisplayLogsCheck = 1;//Debug

 	ShowLogs(1, "App Begins");

 	//ReadAllData("logreads.txt", outputData);
 	//ShowLogs(1, "%s", outputData);
 	//memset(outputData, '\0', strlen(outputData));
 	//ReadAllData("logreads1.txt", outputData);
 	//ShowLogs(1, "%s", outputData);

    // Application initialization
	memset(&stEventMsg, 0, sizeof(stEventMsg));
	stEventMsg.MsgType = USER_MSG;
	iRet = event_main(&stEventMsg);
	if (iRet == ERR_EXIT_APP)
	{
		//	CommOnHook(TRUE);
		return 0;
	}
	ShowLogs(1, "Done with event_main for USER_MSG");

	GetEngTime(szEngTime);
	Gui_ClearScr();
	Gui_ShowMsgBox(szEngTime, gl_stTitleAttr, _T("ARCA v1.0"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	memset(store_env, '\0', strlen(store_env));
 	iRet = UtilGetEnv("fresh", store_env);
	ShowLogs(1, "Check for Application download.........");
	checkForNewApp();
	//GetCelPlatformID();
 	//Set struct here
 	memset(&tidIso, 0, sizeof(ISOTid));
 	EtopLogo();
 	DelayMs(5000);
 	//wifiSetup();//Test

 	//iRet = atoi(store_env);
 	if(iRet == 1)
 	{
 		//Commented out by Olubayo
 		//CreateWrite("receipt.txt", "0");//clear receipt counter
 		//CreateWrite("reprint.txt", "0");//clear reprint file
 		ShowLogs(1, "It is an old Setup");
//remove on start 		
//OldProfile();
//checkForNewApp();
 	}
 	else
 	{
 		ShowLogs(1, "It is a fresh Setup");
 		CreateWrite("tlog.txt", "1");//Creating txn log
 		FreshProfile();
 	}

 	//CreateWrite("eod.txt", "0");//Delete immediately
 	//CreateWrite("count.txt", "0");//Delete immediately
 	//CreateWrite("tlog.txt", "1");//Delete immediately

 	/*DisplayInfoNone("", "APP UNZIPPED 1", 2);
 	Beep();
 	DisplayInfoNone("", "APP UNZIPPED 2", 2);
 	Beep();
 	DisplayInfoNone("", "APP UNZIPPED 3", 2);
 	Beep();*/

#ifdef APP_DEBUG
	ShowLogs(1, "Debug defined.......");
#endif
	//////////////////////////////////////////////////////////////////////////
    // Main loop
	bUpdateUI = TRUE;
	if(0 == GetEnv("PIC_NO", szGallery_Image_Num))
	{
	    iGallery_Image_Num = atoi(szGallery_Image_Num);
	}
	else
	{
	    iGallery_Image_Num = 0;
	}

	
	//ShowLogs(1, "Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	//RunApp(1); test

	while( 1 )
	{
		// Setup idle timer
		if (bUpdateUI)
		{
			SetIdleTimer();
		}

		//Check if it ECR here

		//IF not ecr
		// If any key is pressed
		if(1)
		{
			ShowLogs(1, "Inside Start");
			iRet = ProcKeyMsg();
			bUpdateUI = TRUE;
			CommOnHook(FALSE);
			continue;
		}

		//ShowLogs(1, "About Displaying ShowIdleMainUI");
		ShowIdleMainUI(&bUpdateUI, bGallery, iGallery_Image_Num);	// bUpdateUI value may change
		//ShowLogs(1, "Done Displaying ShowIdleMainUI");

        // When chip card inserted
		if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
		{
			ShowLogs(1, "Inside ChkIdleTimer and IccDetect");
			UtilPutEnv("actype", "00");
			txnType = 1;
			EMVSetDebug(1);//Delete Use to capture logs
			iRet = ProcICCMsg();
			ShowLogs(1, "Switch ICCARD_MSG Response: %d", iRet);
			PromptRemoveICC();
			//EMVSetDebug(0);//Delete Use to capture logs
			
			//ShowLogs(1, "Done with PromptRemoveICC");
			//ReadAllData("logreads.txt", outputData);
			//ShowLogs(1, "Done with PromptRemoveICC");


			bUpdateUI = TRUE;
			bGallery = FALSE;
			CommOnHook(FALSE);
			continue;
		}

		// TODO:add your event-handlers here...
        memset(store_env, '\0', strlen(store_env));
        UtilGetEnv("chinterval", store_env);
        if (ChkIdleTimer(atoi(store_env)))
        {
        	ShowLogs(1, "Inside Check Idle Time");
        	//bGallery = TRUE;
        	bUpdateUI = TRUE;
			bGallery = FALSE;
            DisplayInfoNone("", "PERFORMING HOMECALL", 2);
        	PackCallHomeData();
        	Gui_ClearScr();
            SetIdleTimer();
            continue;
        }
	} // end of while( 1
	return 0;
}

// event processing
int main_sub(const ST_EVENT_MSG *pstEventMsg)
{
	int		iRet;

	ShowLogs(1, "System Init Start");
	SystemInit();
	ShowLogs(1, "System Init Done");

#ifdef ENABLE_EMV
	if(emvcore == 0)
	{
		ShowLogs(1, "About Initiating Core");
		iRet = EMVCoreInit();
		if( iRet==EMV_KEY_EXP )
		{
			ShowLogs(1, "Inside EraseExpireCAPK");
			EraseExpireCAPK();
		}
		ShowLogs(1, "Initiating Core Returns: %d", iRet);
		emvcore++;
	}
#endif

	ShowLogs(1, "About Doing SetOffBase");
	SetOffBase(NULL);
	ShowLogs(1, "Done SetOffBase. About CheckTermSatus");
	CheckTermSatus(pstEventMsg);
	ShowLogs(1, "Done Terminal Status");

#ifndef APP_MANAGER_VER
	// Process manager attached administrative message.
	// Not implemented
#endif

	iRet = ERR_NO_DISP;
	ShowLogs(1, "After CheckTermStatus: %d", iRet);
	ShowLogs(1, "Checking...: %d", pstEventMsg->MsgType);

	switch( pstEventMsg->MsgType )
	{
		case USER_MSG:
			ShowLogs(1, "Switch USER_MSG");
			ProcUserMsg();
			break;
		case ICCARD_MSG:
			ShowLogs(1, "Inside When Chip Card Inserted 4a");
			ShowLogs(1, "Switch ICCARD_MSG");
			iRet = ProcICCMsg();
			ShowLogs(1, "Switch ICCARD_MSG Response: %d", iRet);
			PromptRemoveICC();
			ShowLogs(1, "Done with PromptRemoveICC");
		    break;
		case KEYBOARD_MSG:
			ShowLogs(1, "Switch KEYBOARD_MSG 2");
			iRet = ProcKeyMsg();
			ShowLogs(1, "Done with ProcKeyMsg. %d", iRet);
			break;
		default:
		    break;
	}

	if( iRet!=0 )
	{
		ShowLogs(1, "1d. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
		ShowLogs(1, "1d. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
		ShowLogs(1, "1d. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
		ShowLogs(1, "Return not equal 0");
		DispResult(iRet);
		Gui_ClearScr(); // Added by Kim_LinHB 2014-08-13 v1.01.0003 bug512
	}
	ShowLogs(1, "Outside Switch Step 1");

	SetOffBase(NULL);
	ShowLogs(1, "2a. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "2a. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "2a. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	ShowLogs(1, "Outside Switch Step 2");
    kbflush();
    ShowLogs(1, "Outside Switch Step 3");
	CheckInitTerminal();
	ShowLogs(1, "3a. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "3a. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "3a. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	ShowLogs(1, "Outside Switch Step 4");
	UnLockTerminal();
	ShowLogs(1, "4a. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "4a. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "4a. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	ShowLogs(1, "Outside Switch Step 5");

	if (iRet==ERR_EXIT_APP)
	{
		ShowLogs(1, "Outside Switch Step 6");
		return ERR_EXIT_APP;
	}

#ifndef APP_MANAGER_VER
	// Response to manager admin msg
#endif
	ShowLogs(1, "Outside Switch Step 7");
	return iRet; // Modified by Kim_LinHB 2014-08-13 v1.01.0003 bug 512
}

// read config parameters, check terminal status(e.g. if need to download parameters, if it's locked, etc.)
// and reset reversal flag etc
int CheckTermSatus(const ST_EVENT_MSG *pstEventMsg)
{
	ShowLogs(1, "Inside Checking Terminal Status");
	FirstRunProc(pstEventMsg);
	ShowLogs(1, "Done with FirstRunProc");
	LoadEdcLang();
	ShowLogs(1, "6. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "6. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "6. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	memcpy(&glSysParamBak, &glSysParam, sizeof(glSysParam));
	LoadSysDownEdcFile();	// load the files downloaded from Protims
	ShowLogs(1, "7. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "7. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "7. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	CheckInitTerminal();
	ShowLogs(1, "8. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "8. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "8. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	UnLockTerminal();
	ShowLogs(1, "9. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "9. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "9. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	RecoverTranLog();	// must called after system initialization
	ShowLogs(1, "10. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "10. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "10. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	//Manual setting by wisdom
	
	InitTransInfo();//The problem is here. Investigate.
	//When commented out, it worked  but with some issues
	ShowLogs(1, "11. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "11. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "11. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

#ifndef APP_MANAGER_VER
	// load the synchronous parameters file received from main application if main application is existed
	// Not implemented
#endif

	return 0;
}

// µÚÒ»´ÎÔËÐÐÊ±ºò´¦Àí(ÊÂÊµÉÏÃ¿´Îmain_sub¶¼»áÔËÐÐ)
// process for the first run
static char bFirstRun = 1; //added by Kim_LinHB 2014-6-7
void FirstRunProc(const ST_EVENT_MSG *pstEventMsg)
{
	uchar	szEngTime[16+1];
	uchar	ucNeedUpdateParam;
	
	ShowLogs(1, "Processing First Run");
	// Added by Kim_LinHB 2014-7-2
	if(bFirstRun)
	{
		ShowLogs(1, "FirstRunProc: Inside bFirstRun");
		ST_FONT stFont[3];
		unsigned char sTermInfo[30];

		kbmute(0);
		GetTermInfo(sTermInfo);
		
		Gui_Init(_RGB_INT_(255,255,255), _RGB_INT_(126,11,128), NULL);
		
		memcpy(stFont, gl_Font_Def, sizeof(gl_Font_Def));
		// Added by Kim_LinHB 2014-8-12 v1.01.0003 bug510
		//modified by Kevin Liu 20160613 bug847 bug MEDC-13
		if(sTermInfo[19] & 0x02){
			//added by Kevin Liu 20160803 bugMEDC-13
			stFont[0].Width = 8;
			stFont[0].Height = 16;

			stFont[1].Width = 12;
			stFont[1].Height = 24;
		}
		else{
			stFont[0].Width = 6;
			stFont[0].Height = 8;

			stFont[1].Width = 8;
			stFont[1].Height = 16;
		}

		Gui_LoadFont(GUI_FONT_SMALL, &stFont[0],  NULL);
		Gui_LoadFont(GUI_FONT_NORMAL, &stFont[1], NULL);
		Gui_LoadFont(GUI_FONT_LARGE, &stFont[2], NULL );

		// Modified by Kim_LinHB 2014-7-2
		if( ExistSysFiles() )
		{
			ShowLogs(1, "FirstRunProc: Inside ExistSysFiles");
			if (ValidSysFiles())
			{
				ShowLogs(1, "FirstRunProc: Inside ValidSysFiles");
				LoadSysParam();
				LoadSysCtrlAll();

				ucNeedUpdateParam = FALSE;
				if (pstEventMsg->MsgType==USER_MSG)
				{
					if (UpdateTermInfo() || InitMultiAppInfo())
					{
						ucNeedUpdateParam = TRUE;
					}
				}
				if( glSysParam.stTxnCommCfg.pfUpdWaitUI!=DispWaitRspStatus )
				{
					glSysParam.stTxnCommCfg.pfUpdWaitUI = DispWaitRspStatus;
					glSysParam.stTMSCommCfg.pfUpdWaitUI = DispWaitRspStatus;
					ucNeedUpdateParam = TRUE;
				}

				if (ucNeedUpdateParam)
				{
					SaveSysParam();
				}
				bFirstRun = 0;
				//return;
			}
			else
			{
				//Commented out by Wisdom
				/*
				ShowLogs(1, "FirstRunProc: Outside ValidSysFiles");
				int iRet;
				Gui_ClearScr();
				iRet = Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("APP AND DATA\nINCONSIST.\nRESET CONFIG?"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);
				if (iRet != GUI_OK)
				{
					SysHaltInfo("PLS REPLACE APP");
				}

				Gui_ClearScr();
				iRet = Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("WARNING\nDATA WILL BE CLEAR\nCONTINUE ?"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);
				if (iRet != GUI_OK)
				{
					SysHaltInfo("PLS REPLACE APP");
				}*/
				RemoveSysFiles();
			}
		}
		ShowLogs(1, "FirstRunProc: Outside ValidSysFiles");
		
		GetEngTime(szEngTime);
		Gui_ClearScr();
		Gui_ShowMsgBox(szEngTime, gl_stTitleAttr, _T("Please Wait"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

		ShowLogs(1, "FirstRunProc: LoadEdcDefault");
		ShowLogs(1, "1. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
		ShowLogs(1, "1. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
		ShowLogs(1, "1. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
		LoadEdcDefault();	// set EDC default values //glSysCtrl - Wisdom //Set glSysParam// Please revisit
		ShowLogs(1, "FirstRunProc: InitTransInfo");
		ShowLogs(1, "2. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
		ShowLogs(1, "2. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
		ShowLogs(1, "2. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
		//InitTranLogFile();	// Init transaction log file //Writing files - Wisdom
		ShowLogs(1, "FirstRunProc: NoDownloadInit");
		ShowLogs(1, "3. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
		ShowLogs(1, "3. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
		ShowLogs(1, "3. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
		NoDownloadInit();//Displays the load Default. Opens up the Admin Panel //By Wisdom
		ShowLogs(1, "FirstRunProc: Done with NoDownloadInit");
		ShowLogs(1, "4. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
		ShowLogs(1, "4. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
		ShowLogs(1, "4. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
		bFirstRun = 0;
	}

#ifdef ENABLE_EMV
	LoadEmvDefault();	// Init EMV kernel //EMV Keys
	ShowLogs(1, "5. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	ShowLogs(1, "5. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	ShowLogs(1, "5. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
#endif
}

// ÉèÖÃ¿ÕÏÐ¼ÆÊ±¡£ÉèÖÃÒ»¸ö±È½Ï³¤µÄµ¹¼ÆÊ±£¬ÒÔÓÃÓÚ²»Ö¹Ò»ÖÖµÄ¿ÕÏÐÊÂ¼þ´¦Àí
// set a idle timer with a long period of time, for processing several idle events
void SetIdleTimer(void)
{
	TimerSet(TIMER_TEMPORARY, TIMERCNT_MAX);
}

// ¼ì²é¿ÕÏÐ¼ÆÊ±£¬¿´ÊÇ·ñÒÑ¾­Á÷¹ýÁËÖ¸¶¨µÄ·ÖÖÓÊý
// check if the timer counted the specific time(uint:minute)
uchar ChkIdleTimer(int iSeconds)
{
	int	iCnt = TIMERCNT_MAX-TimerCheck(TIMER_TEMPORARY);
	//ShowLogs(1, "ChkIdleTimer: %d", iSeconds);	
	//PubASSERT(TIMERCNT_MAX > iSeconds*10);	//	ScrPrint(0,7,ASCII,"%d  ", iCnt/10);
	
	return (iCnt >= iSeconds*10);
}

// ÏÔÊ¾¿ÕÏÐÊ±ÓÃ»§½çÃæ
// show an idle UI
void ShowIdleMainUI(uchar *pbForceUpdate, uchar bShowGallery, int iGallery_Image_Num)
{
	static	uchar	szLastTime[5+1] = {"00000"};
	uchar	szCurTime[16+1];

	GetEngTime(szCurTime);
	if( *pbForceUpdate || memcmp(szLastTime, &szCurTime[11], 4)!=0 )	// Reset magstripe reader every 10 minutes
	{
		MagClose();
		MagOpen();
		MagReset();
	}

	//DO ECR here

	if(bShowGallery){
	    ShowGallery(iGallery_Image_Num);
	}

	if( *pbForceUpdate || memcmp(szLastTime, &szCurTime[11], 5)!=0)
	{
		// refresh UI
		sprintf((char *)szLastTime, "%.5s", &szCurTime[11]); 

		//Gui_ClearScr(); // removed by Kim_LinHB 2014-08-13 v1.01.0003 bug512
		// Modified by Kim_LinHB 2014-8-11 v1.01.0003
        //Gui_ShowMsgBox(szCurTime, gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0,NULL);
		//Gui_UpdateTitle(szCurTime, gl_stTitleAttr); //By Wisdom
		Gui_DrawText(szCurTime, gl_stTitleAttr, 2, 4);
		//Gui_DrawText(szCurTime, gl_stTitleAttr, 0, 5);
        if(*pbForceUpdate){
            if(!bShowGallery)
            {
            	//Menu is here
                Gui_UpdateKey(KEYFN, "FUNC");
                Gui_UpdateKey(KEYMENU, "MENU");
                DispSwipeCard(TRUE);
			}
        }
		*pbForceUpdate = FALSE;
	}
#ifdef _WIN32
	DelayMs(100);
#endif
}

static void ShowGallery(int num)
{
    static int iCurrImages = 0;
    static unsigned long ulTimer_Late = 0;
    static unsigned long ulTimer_Curr = 0;

    if(num <= 0)
        return;

    ulTimer_Curr = GetTimerCount();
    if(ulTimer_Curr - ulTimer_Late >= 10000)
    {
        uchar szImageKey[8];
        uchar szImageValue[255] = {0};
        sprintf(szImageKey, "PIC_%d", iCurrImages);
        if(0 == GetEnv(szImageKey, szImageValue))
        {
            ulTimer_Late = ulTimer_Curr;
            Gui_ClearScr();
			Gui_DrawImage(szImageValue, 0,0);
        }

        ++iCurrImages;
        if(iCurrImages >= num)
            iCurrImages = 0;
    }

}

// Modified by Kim_LinHB 2014-7-8
int CheckInitTerminal(void)
{
	uchar	szCurTime[16+1], szLastTime[16+1];
	uchar	ucKey;
	uchar	szBuff[50];
	
	ShowLogs(1, "Inside Check Init Terminal.");

	if( !(glSysParam.ucTermStatus & INIT_MODE) )
	{
		return 0;
	}
	
	TimerSet(0, 0);
	memset(szCurTime,  0, sizeof(szCurTime));
	memset(szLastTime, 0, sizeof(szLastTime));
	while( glSysParam.ucTermStatus & INIT_MODE )
	{
		if( TimerCheck(0)==0 )
		{
			TimerSet(0, 10);
			GetEngTime(szCurTime);
			if (strcmp(szCurTime, szLastTime)!=0)
			{
				Gui_ClearScr();
				sprintf(szBuff, "%s\n[%.14s]", _T("PLEASE INIT"), AppInfo.AppName);
				Gui_UpdateTitle(szCurTime, gl_stTitleAttr);
				Gui_DrawText(szBuff, gl_stCenterAttr, 0, 50);
				memcpy(szLastTime, szCurTime, sizeof(szLastTime));
			}
		}

		ucKey = PubWaitKey(10);
		if( ucKey==KEYF1 || ucKey==KEYFN )
		{
			InitTransInfo();
			FunctionInit();
			TimerSet(0, 0);
			memset(szLastTime, 0, sizeof(szLastTime));
		}
	}

	return 0;
}

void MainIdleProc(void)
{
//#if !defined(_WIN32)
#if 1
	// should not let POS go into sleep mode when running simulator
	int		iRet;

	if (ChkTerm(_TERMINAL_S90_))
	{
		if (glSysParam.stEdcInfo.ucIdleShutdown)
		{
			PowerOff();
		}
		else
		{
			// Modified by Kim_LinHB 2014-7-8
			Gui_ClearScr();
			Gui_ShowMsgBox(_T("POWER SAVING"), gl_stTitleAttr, _T("PRESS ANY KEY\nTO RECOVER"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

			do 
			{
				iRet = SysSleep("00");
			} while((iRet==-3) && (kbhit()!=0));
			// ºÜÆæ¹ÖµÄÏÖÏó£º³ý·ÇÔÚÉÏ´ÎSysSleep·µ»ØÖ®ºóµ÷ÓÃDelayMs(3000)£¬·ñÔò¼´Ê¹¼ä¸ô1·ÖÖÓ£¬µ÷ÓÃSysSleepÈÔ»áÖ±½Ó·µ»Ø-3¡£
			// Òò´ËÎÒÔÚÕâÀï¼ÓÁËÅÐ¶Ï£¬Èç¹û·µ»Ø-3¶øÇÒÃ»ÓÐ°´¼üÔò¼ÌÐøÖØ¸´SysSleep
			// ÔÚÍâ²¿ÒÑ¾­±£Ö¤ÁË½øÈëMainIdleProcµÄ¼ä¸ô>=1·ÖÖÓ
			// it needs to delay 3 seconds after return from SysSleep, otherwise SysSleep will return -3 even the period of calling SysSleep is over 1 min,
			// so here is a special processing, if return -3 from SysSleep and no key was pressed then continue calling SysSleep.

			DelayMs(100);
			kbflush();
			Gui_ClearScr(); // Added by Kim_LinHB 2014-08-13 v1.01.0003
		}
	}
#endif
}

// end of file

