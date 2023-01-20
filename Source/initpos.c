
#include "global.h"

/********************** Internal macros declaration ************************/
#define DOWNPARA_FILE	"SYS_DOWN_EDC"

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void RemoveEmvAppCapk(void);
static int  SelectDownloadMode(uchar *pucCommType);
static int  OldTmsDownLoad(void);
static int  UnpackPara(const uchar *psParaData, long lDataLen);
static void UnpackParaEdc(const uchar *psPara);
static void TransformIP(const uchar * ip_in, uchar * ip_out);
static void TransformPort(const uchar * port_in, uchar * port_out);
static int  UnpackParaAcq(uchar ucIndex, const uchar *psPara, ulong ulSubFieldLen);
static int  UnpackParaIssuer(uchar ucIndex, const uchar *psPara);
static int  UnpackParaCard(uchar ucIndex, const uchar *psPara);

static int  UnpackInstPara(const uchar *psPara);
static int  UnpackDescPara(const uchar *psPara);
static uchar SearchIssuerKeyArray(void);
static void AfterLoadParaProc(void);
static void  SearchIssuerKeyArraySub(uchar *sIssuerKey, uchar ucAcqKey);
static int  GetDownLoadTelNo(void);
static int  GetDownLoadGprsPara(void);
static int  GetDownLoadLanPara(void);
static int  GetDownLoadWIFIPara(void);
static int  GetDownLoadComm(uchar ucCommType);
static int  GetDownLoadTID(uchar *pszID);
static int  SaveEmvMisc(const uchar *psPara);
static int  SaveEmvApp(const uchar *psPara);
static int  SaveEmvCapk(const uchar *psPara);
static void GetNextAutoDayTime(uchar *pszDateTimeInOut, ushort uiInterval);
static int  SaveDetail(const uchar *psData);
static int  SaveCardBin(uchar *psCardBinInOut);

void InitEdcParam(void);

/********************** Internal variables declaration *********************/
static uchar	sgSyncDial, sgNewTMS;
static uchar	sgTempBuf[1024*20];
static uchar	sgEMVDownloaded;

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
static void GetDefCurrency(CURRENCY_CONFIG *pstConfig)
{
#ifdef AREA_HK
	*pstConfig = glCurrency[0];
#elif defined(AREA_Arabia)
	*pstConfig = glCurrency[33];
#else
	*pstConfig = glCurrency[15];
#endif
}

// ÉèÖÃEDCÈ±Ê¡²ÎÊý
// Set to default EDC parameter
void LoadEdcDefault(void)
{
	int			iCnt;

	ShowLogs(1, "Entering ResetAllPara to set glSysParam");
	ResetAllPara(TRUE); //glSysParam - Wisdom
	ShowLogs(1, "Done ResetAllPara to set glSysParam");

	DelFilesbyPrefix(GetCurSignPrefix(ACQ_ALL));

	memset(&glSysCtrl, 0, sizeof(SYS_CONTROL));
	glSysCtrl.ulInvoiceNo = 1L;
	glSysCtrl.ulSTAN      = 1L;
	glSysCtrl.uiLastRecNo = 0xFFFF;
	glSysCtrl.uiErrLogNo  = 0;
	ShowLogs(1, "LoadEdcDefault: MAX_ACQ: %d", MAX_ACQ);
	for(iCnt=0; iCnt<MAX_ACQ; iCnt++)
	{
		glSysCtrl.sAcqStatus[iCnt]		   = S_RESET;
		glSysCtrl.stField56[iCnt].uiLength = 0;
		glSysCtrl.uiLastRecNoList[iCnt]    = 0xFFFF;
	}
	ShowLogs(1, "LoadEdcDefault: MAX_TRANLOG: %d", MAX_TRANLOG);
	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		glSysCtrl.sAcqKeyList[iCnt]    = INV_ACQ_KEY;		// set to invalid acquirer key
		glSysCtrl.sIssuerKeyList[iCnt] = INV_ISSUER_KEY;	// set to invalid issuer key
	}
	glSysCtrl.stWriteInfo.bNeedSave = SAVE_NONEED;
	ShowLogs(1, "LoadEdcDefault: About Saving SaveSysCtrlAll");
	SaveSysCtrlAll();
	ShowLogs(1, "LoadEdcDefault: Done Saving SaveSysCtrlAll");

#ifdef ENABLE_EMV
	ShowLogs(1, "LoadEdcDefault: About Saving SaveEmvStatus");
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
	ShowLogs(1, "LoadEdcDefault: Done Saving SaveEmvStatus");
#endif
}

#ifdef ENABLE_EMV
void LoadEmvDefault(void)
{
	char asc[5] = {0};
	char bcd[3] = {0};
	char dts[256] = {0};
	char temp[128] = {0};
	CURRENCY_CONFIG		stLocalCurrency;
	ShowLogs(1, "LoadEmvDefault: Step 1");
	GetDefCurrency(&stLocalCurrency);
	RemoveEmvAppCapk();
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
	
	EMVGetParameter(&glEmvParam);
	//memcpy(glEmvParam.Capability, "\xE0\xB0\xC8", 3);//\xE0\xE1\xC8
	//EMVSetTLVData(0x9F33, (uchar *)"\xE0\xB0\xC8", 3);//\xE0\xE1\xC8
	memcpy(glEmvParam.Capability, "\xE0\xF8\xC8", 3);
	EMVSetTLVData(0x9F33, (uchar *)"\xE0\xF8\xC8", 3);
	EMVSetTLVData(0x9F35, (uchar *)"\x22", 1);
	EMVSetTLVData(0x9F09, (uchar *)"\x00\x02", 2);
	EMVSetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6);
	EMVSetTLVData(0x5F2A, (uchar *)"\x05\x66", 2);
	EMVSetTLVData(0x9F1A, (uchar *)"\x05\x66", 2);
	EMVSetTLVData(0x9C, (uchar *)"\x00", 1);
	memcpy(glEmvParam.ExCapability,  "\xE0\x00\xF0\xA0\x01", 5);
	memcpy(glEmvParam.ReferCurrCode,  "\x05\x66", 2);
	memcpy(glEmvParam.CountryCode,  "\x05\x66", 2);
	memcpy(glEmvParam.TransCurrCode,  "\x05\x66", 2);
	glEmvParam.TransCurrExp = 0x02;
	glEmvParam.ReferCurrExp = 0x02;
	glEmvParam.TransType = EMV_GOODS;
	glEmvParam.ForceOnline   = 1;
	glEmvParam.GetDataPIN    = 1;//Try 0x01
	glEmvParam.SurportPSESel = 1;
	/*memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMNL", temp);
	memcpy(glEmvParam.MerchName,  temp, 40);
	memset(asc, '\0', strlen(asc));
	memset(bcd, '\0', strlen(bcd));
	UtilGetEnvEx("txnMCC", asc);
	PubAsc2Bcd(asc, 2, bcd);
	memcpy(glEmvParam.MerchCateCode,  (uchar *)bcd, 2);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	memcpy(glEmvParam.MerchId,  temp, 15);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	memcpy(glEmvParam.TermId,  temp, 8);
	memcpy(glEmvParam.ReferCurrCode,  "\x05\x66", 2);
	memcpy(glEmvParam.CountryCode,  "\x05\x66", 2);
	memcpy(glEmvParam.TransCurrCode,  "\x05\x66", 2);
	memset(asc, '\0', strlen(asc));
	memset(bcd, '\0', strlen(bcd));
	PubAsc2Bcd(asc, 2, bcd);
	memset(asc, '\0', strlen(asc));
	memset(bcd, '\0', strlen(bcd));
	PubAsc2Bcd(asc, 2, bcd);
	memset(asc, '\0', strlen(asc));
	memset(bcd, '\0', strlen(bcd));
	PubAsc2Bcd(asc, 2, bcd);
	memcpy(glEmvParam.MerchCateCode,  bcd, 2);
	glEmvParam.TransCurrExp = 0x02;
	glEmvParam.ReferCurrExp = 0x02;
	glEmvParam.TransType = EMV_GOODS;
	glEmvParam.ForceOnline   = 1;
	glEmvParam.GetDataPIN    = 1;//Try 0x01
	glEmvParam.SurportPSESel = 1;*/

	EMVSetParameter(&glEmvParam);

	AddAllApps();
	AddAllKeys();
}
#endif

void LoadDefCommPara(void)
{
	uchar	sBuff[HWCFG_END+1];

	// ================================ ½»Ò×²ÎÊý ================================
	// ========================= Transaction parameters =========================
	// ÉèÖÃ»Øµ÷º¯Êý
	// Setup callback function used in waiting response display
	glSysParam.stTxnCommCfg.pfUpdWaitUI   = DispWaitRspStatus;

	// TMS comm type
	glSysParam.stTMSCommCfg.ucCommType    = CT_NONE;	// will not be set until fun0
	glSysParam.stTMSCommCfg.ucCommTypeBak = CT_NONE;

	// TXN default comm type
	GetTermInfo(sBuff);
	if ((sBuff[HWCFG_MODEL]==_TERMINAL_S90_))
	{
		if (sBuff[HWCFG_GPRS]!=0)
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_GPRS;
		}
		else if (sBuff[HWCFG_CDMA]!=0)
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_CDMA;
		}
		else if (sBuff[HWCFG_WCDMA]!=0)     // added by  Gillian 2015/11/23
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_WCDMA;
		}
	}
	else if (sBuff[HWCFG_MODEM]!=0)
	{
		glSysParam.stTxnCommCfg.ucCommType = CT_MODEM;
	}
	else
	{
		glSysParam.stTxnCommCfg.ucCommType = CT_RS232;
	}

	glSysParam.stTxnCommCfg.ucCommTypeBak  = CT_NONE;

	// TCP length header format.
	glSysParam.stTMSCommCfg.ucTCPClass_BCDHeader = TRUE;
	glSysParam.stTxnCommCfg.ucTCPClass_BCDHeader = TRUE;

	// ½»Ò×²¦ºÅÈ±Ê¡²ÎÊý
	// Default dial parameter in transaction
	// please see details in API manual
	glSysParam._TxnPSTNPara.ucSendMode = CM_SYNC;
    glSysParam._TxnPSTNPara.ucSignalLevel = 0;
	glSysParam._TxnModemPara.DP      = 0;
	glSysParam._TxnModemPara.CHDT    = 0;
	glSysParam._TxnModemPara.DT1     = 20;		// µÈºò²¦ºÅÒôµÄ×î³¤Ê±¼ä(20~~255), µ¥Î»: 100ms
	glSysParam._TxnModemPara.DT2     = 10;		// ²¦ÍâÏßÊ±","µÈ´ýÊ±¼ä(0~~255), µ¥Î»: 100ms
	glSysParam._TxnModemPara.HT      = 70;		// Ë«Òô²¦ºÅµ¥Ò»ºÅÂë±£³ÖÊ±¼ä(µ¥Î»:1ms,ÓÐÐ§·¶Î§50~255)
	glSysParam._TxnModemPara.WT      = 5;		// Ë«Òô²¦ºÅÁ½¸öºÅÂëÖ®¼äµÄ¼ä¸ôÊ±¼ä(µ¥Î»:10ms,ÓÐÐ§·¶Î§5~25)
	glSysParam._TxnModemPara.SSETUP  = 0x05;	// 0x45: 9600bps, 0x05:1200 bps	// Í¨Ñ¶×Ö½Ú
	glSysParam._TxnModemPara.DTIMES  = 1;		// Ñ­»·²¦ºÅ×Ü´ÎÊý,²¦Íê²¦ºÅ´®µÄËùÓÐºÅÂëÎªÒ»´Î[ÓÐÐ§·¶Î§1~255]
	glSysParam._TxnModemPara.TimeOut = 6;		// Ã»ÓÐÊý¾Ý½»»»MODEM¹Ò¶ÏµÈ´ýÊ±¼ä;ÒÔ10ÃëÎªµ¥Î»,Îª0Ê±ÎÞ³¬Ê±,×î´óÖµ65
	glSysParam._TxnModemPara.AsMode  = 0;		// Òì²½Í¨Ñ¶µÄËÙÂÊ

	// RS232È±Ê¡²ÎÊý
	// Default parameter in transaction for RS232
#ifdef _WIN32
#if defined(_Sxx_)
	glSysParam._TxnRS232Para.ucPortNo   = PINPAD;
#else
	glSysParam._TxnRS232Para.ucPortNo   = COM2;
#endif
#else
	glSysParam._TxnRS232Para.ucPortNo   = COM1;
#endif
	glSysParam._TxnRS232Para.ucSendMode = CM_SYNC;
	sprintf((char *)glSysParam._TxnRS232Para.szAttr, "9600,8,n,1");
#ifdef _Dxxx_
	glSysParam._TxnBlueToothPara.stCommParam.ucPortNo =  COM_BT;
	glSysParam._TxnBlueToothPara.stCommParam.ucSendMode = CM_RAW;//hdadd // Modified by Kim_LinHB 2014-08-18 v1.01.0004 from Sync to Raw
	sprintf((char *)glSysParam._TxnBlueToothPara.stCommParam.szAttr, "115200,8,n,1");//hdadd // Modified by Kim_LinHB 2014-08-15 v1.01.0004

	// Added by Kim_LinHB 2014-08-15 v1.01.0004
#ifdef _MIPS_
	glSysParam._TxnBlueToothPara.stConfig.role = BT_ROLE_SLAVE;
	glSysParam._TxnBlueToothPara.stConfig.baud = 115200;
#endif
	strcpy(glSysParam._TxnBlueToothPara.stConfig.name, "Dxxx ");
	ReadSN(glSysParam._TxnBlueToothPara.stConfig.name + strlen(glSysParam._TxnBlueToothPara.stConfig.name));
	strcpy(glSysParam._TxnBlueToothPara.stConfig.pin, "0000");
#endif

	// TCP/IPÈ±Ê¡²ÎÊý
	// Default parameter in transaction for TCPIP
	glSysParam._TxnTcpIpPara.ucDhcp = 1;
	glSysParam._TxnTcpIpPara.szNetMask[0] = 0;
	glSysParam._TxnTcpIpPara.szGatewayIP[0] = 0;
	glSysParam._TxnTcpIpPara.szLocalIP[0] = 0;
    memset(&glSysParam._TxnTcpIpPara.stHost1, 0, sizeof(IP_ADDR));
    memset(&glSysParam._TxnTcpIpPara.stHost2, 0, sizeof(IP_ADDR));
	glSysParam._TxnTcpIpPara.szDNSIP[0] = 0;
	
	// Default parameter in transaction for WIFI
	memset(&glSysParam._TxnWifiPara,0x00,sizeof(WIFI_PARA));
	memcpy(&glSysParam._TxnWifiPara,&glSysParam._TxnTcpIpPara,sizeof(WIFI_PARA));
	//glSysParam._TxnWifiPara.ucPortNo = 5;
	glSysParam._TxnWifiPara.stParam.DhcpEnable = TRUE;//Dhcp

	// GPRS/CDMAÈ±Ê¡²ÎÊý
	// Default parameter in TMS for GPRS/CDMA
	glSysParam._TxnWirlessPara.szAPN[0] = 0;
	glSysParam._TxnWirlessPara.szUID[0] = 0;
	glSysParam._TxnWirlessPara.szPwd[0] = 0;
	glSysParam._TxnWirlessPara.szSimPin[0] = 0;
	glSysParam._TxnWirlessPara.szDNS[0] = 0;
    memset(&glSysParam._TxnWirlessPara.stHost1, 0, sizeof(IP_ADDR));
    memset(&glSysParam._TxnWirlessPara.stHost2, 0, sizeof(IP_ADDR));

	// ================================ ÏÂÔØ²ÎÊý ================================
	// ====================== TMS communication parameters ======================
	// ÉèÖÃ»Øµ÷º¯Êý
	// Setup callback function in TMS download
	glSysParam.stTMSCommCfg.pfUpdWaitUI = DispWaitRspStatus;
	glSysParam.stTMSCommCfg.ucCommType  = CT_MODEM;

	// ²ÎÊýÏÂÔØÈ±Ê¡²ÎÊý
	// Default dial parameter in TMS download
	glSysParam._TmsModemPara.DP      = 0;
	glSysParam._TmsModemPara.CHDT    = 0x40;
	glSysParam._TmsModemPara.DT1     = 5;
	glSysParam._TmsModemPara.DT2     = 7;
	glSysParam._TmsModemPara.HT      = 70;
	glSysParam._TmsModemPara.WT      = 5;
	glSysParam._TmsModemPara.SSETUP  = 0x87;	/* asynchronise link */
	glSysParam._TmsModemPara.DTIMES  = 1;
	glSysParam._TmsModemPara.TimeOut = 6;
	glSysParam._TmsModemPara.AsMode  = 0xF0;

	// RS232È±Ê¡²ÎÊý(TMS)
	// RS232 para in TMS download
	memcpy(&glSysParam._TmsRS232Para, &glSysParam._TxnRS232Para, sizeof(glSysParam._TmsRS232Para));

	// TCP/IPÈ±Ê¡²ÎÊý
	// TCP/IP para in TMS download
	memcpy(&glSysParam._TmsTcpIpPara, &glSysParam._TxnTcpIpPara, sizeof(glSysParam._TmsTcpIpPara));

	// GPRS/CDMAÈ±Ê¡²ÎÊý
	// GPRS/CDMA para in TMS download
	memcpy(&glSysParam._TmsWirlessPara, &glSysParam._TxnWirlessPara, sizeof(glSysParam._TmsWirlessPara));
}

void ResetAllPara(uchar bFirstTime)
{
	int				iCnt;
	uchar			ucNewTmsBak, ucTMSSyncDial;
	uchar			szDownTelNo[25+1], szDownLoadTID[8+1], szPabx[10+1];
	IP_ADDR			stTmsIP;
	uchar			sEdcExtOptions[sizeof(glSysParam.stEdcInfo.sExtOption)];
	uchar			ucCommType, ucCommTypeBak, ucIdleMin, ucIdleOpt;
	TCPIP_PARA		stBakTmsTcpip, stBakTxnTcpip;
	WIRELESS_PARAM	stBakTmsWireless, stBakTxnWireless;
	// and WIFI, ...
	LANG_CONFIG		stLangBak;

	// Backup
	if( !bFirstTime )
	{
		ucNewTmsBak   = glSysParam.ucNewTMS;
		ucTMSSyncDial = glSysParam.ucTMSSyncDial;
		sprintf((char *)szDownTelNo,   "%.25s", glSysParam.stEdcInfo.szDownTelNo);
		sprintf((char *)szDownLoadTID, "%.8s",  glSysParam.stEdcInfo.szDownLoadTID);
		memcpy(&stTmsIP, &glSysParam.stEdcInfo.stDownIpAddr, sizeof(IP_ADDR));
		memcpy(sEdcExtOptions, glSysParam.stEdcInfo.sExtOption, sizeof(sEdcExtOptions));

		ucCommType    = glSysParam.stTxnCommCfg.ucCommType;
		ucCommTypeBak = glSysParam.stTxnCommCfg.ucCommTypeBak;
		memcpy(&stBakTmsTcpip, &glSysParam._TmsTcpIpPara, sizeof(TCPIP_PARA));
		memcpy(&stBakTxnTcpip, &glSysParam._TxnTcpIpPara, sizeof(TCPIP_PARA));
		memcpy(&stBakTmsWireless, &glSysParam._TmsWirlessPara, sizeof(WIRELESS_PARAM));
		memcpy(&stBakTxnWireless, &glSysParam._TxnWirlessPara, sizeof(WIRELESS_PARAM));
		memcpy(szPabx, glSysParam.stEdcInfo.szPabx, sizeof(szPabx));

		stLangBak = glSysParam.stEdcInfo.stLangCfg;
		ucIdleMin = glSysParam.stEdcInfo.ucIdleMinute;
		ucIdleOpt = glSysParam.stEdcInfo.ucIdleShutdown;
	}

	memset(&glSysParam, 0, sizeof(SYS_PARAM));

	LoadDefCommPara();
	if (bFirstTime)
	{
		LoadDefaultLang();
	}
	
	glSysParam.ucTermStatus              = INIT_MODE;
	glSysParam.stEdcInfo.bPreDial        = TRUE;
	glSysParam.stEdcInfo.ucScrGray       = 4;
	glSysParam.stEdcInfo.ucAcceptTimeout = 3;
	glSysParam.stEdcInfo.ucTMSTimeOut    = 60;
	glSysParam.stEdcInfo.ucIdleMinute    = 1;
	glSysParam.stEdcInfo.ucIdleShutdown  = 0;
	sprintf((char *)glSysParam.stEdcInfo.szTMSNii, "000");

	ResetPwdAll();

	// Recover
	if( !bFirstTime )
	{
		glSysParam.ucNewTMS      = ucNewTmsBak;
		glSysParam.ucTMSSyncDial = ucTMSSyncDial;
		sprintf((char *)glSysParam.stEdcInfo.szDownTelNo,   "%.25s", szDownTelNo);
		sprintf((char *)glSysParam.stEdcInfo.szDownLoadTID, "%.8s",  szDownLoadTID);
		memcpy(&glSysParam.stEdcInfo.stDownIpAddr, &stTmsIP, sizeof(IP_ADDR));
		memcpy(glSysParam.stEdcInfo.sExtOption, sEdcExtOptions, sizeof(glSysParam.stEdcInfo.sExtOption));
		
		glSysParam.stTxnCommCfg.ucCommType    = ucCommType;
		glSysParam.stTxnCommCfg.ucCommTypeBak = ucCommTypeBak;

		memcpy(&glSysParam._TmsTcpIpPara, &stBakTmsTcpip, sizeof(TCPIP_PARA));
		memcpy(&glSysParam._TxnTcpIpPara, &stBakTxnTcpip, sizeof(TCPIP_PARA));

		memcpy(&glSysParam._TmsWirlessPara, &stBakTmsWireless, sizeof(WIRELESS_PARAM));
		memcpy(&glSysParam._TxnWirlessPara, &stBakTxnWireless, sizeof(WIRELESS_PARAM));

		memcpy(glSysParam.stEdcInfo.szPabx, szPabx, sizeof(glSysParam.stEdcInfo.szPabx));

		glSysParam.stEdcInfo.stLangCfg = stLangBak;
		glSysParam.stEdcInfo.ucIdleMinute = ucIdleMin;
		glSysParam.stEdcInfo.ucIdleShutdown = ucIdleOpt;
	}

	glSysParam.stEdcInfo.ucAutoMode     = 0;	// Don't auto update parameter
	glSysParam.stEdcInfo.uiAutoInterval = 90;
	GetNextAutoDayTime(glSysParam.stEdcInfo.szAutoDayTime, glSysParam.stEdcInfo.uiAutoInterval);

	for(iCnt=0; iCnt<MAX_ACQ; iCnt++)
	{
		glSysParam.stAcqList[iCnt].ucKey = INV_ACQ_KEY;
	}
	for(iCnt=0; iCnt<MAX_ISSUER; iCnt++)
	{
		glSysParam.stIssuerList[iCnt].ucKey = INV_ISSUER_KEY;
	}

	UpdateTermInfo();
	glSysParam.stEdcInfo.ucPedMode = PED_INT_PCI;

	InitMultiAppInfo();

	SaveSysParam();
}

// Modified by Kim_LinHB 2014-6-8
void NoDownloadInit(void)
{
#ifdef ALLOW_NO_TMS
	ShowLogs(1, "NoDownloadInit: Step 1");
	// Modified by Kim_LinHB 2014-4-4
	TransInit(LOAD_PARA);

	ShowLogs(1, "NoDownloadInit: Step 2");

	/*
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("LOAD DEFAULT ? "), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL)){
		ShowLogs(1, "NoDownloadInit: Step 2a");
		return;
	}
	*/

	ShowLogs(1, "NoDownloadInit: Step 3");

	//DispProcess();

	ShowLogs(1, "NoDownloadInit: Step 4");

	InitEdcParam();

	ShowLogs(1, "NoDownloadInit: Step 5");

	glSysParam.ucTermStatus = TRANS_MODE;

	ShowLogs(1, "NoDownloadInit: Step 6");

	SaveSysParam();

	ShowLogs(1, "NoDownloadInit: Step 7");

	SaveSysCtrlAll();

	ShowLogs(1, "NoDownloadInit: Step 8");

#ifdef ENABLE_EMV
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	ShowLogs(1, "NoDownloadInit: Step 9");
	SaveEmvStatus();
	ShowLogs(1, "NoDownloadInit: Step 10");
#endif

	//Gui_ClearScr();
	//Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("COMPLETED!"), gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
	ShowLogs(1, "NoDownloadInit: Step 11");

	//SetSystemParamAll();//Commented out by Wisdom
	ShowLogs(1, "NoDownloadInit: Step 12");

	SaveSysParam();
	ShowLogs(1, "NoDownloadInit: Step 13");
#endif
}

int DownLoadTMSPara_Manual(void)
{
	DownLoadTMSPara(0);
	return 0;
}

int DownLoadTMSPara_Auto(void)
{
	DownLoadTMSPara(1);
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
void DownLoadTMSPara(uchar ucMode)
{
	uchar	ucCommType;
	int		iRet;
	
	InitTransInfo();
	TransInit(LOAD_PARA);

	if( PasswordBank()!=0 )
	{
		return;
	}

	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
	
	if (!ChkIfBatchEmpty())
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS SETTLE BATCH"), gl_stCenterAttr, GUI_BUTTON_OK, USER_OPER_TIMEOUT, NULL);
		return;
	}

	// If never setup, switch to manual mode
	if ((ucMode!=0) && (glSysParam.stTMSCommCfg.ucCommType==CT_NONE))
	{
		ucMode = 0;
	}

	// select downloading mode & enter config
	if (ucMode==0)
	{
		iRet = SelectDownloadMode(&ucCommType);
		if( iRet!=0 )
		{
			return;
		}
	}
	else
	{
		sgNewTMS   = glSysParam.ucNewTMS;
		sgSyncDial = glSysParam.ucTMSSyncDial;
		ucCommType = glSysParam.stTMSCommCfg.ucCommType;
	}

	// ÌáÈ¡ÒÑÊäÈëµÄÍ¨ÐÅÊý¾Ý
	// update communication parameters
	memcpy(&glCommCfg, &glSysParam.stTMSCommCfg, sizeof(COMM_CONFIG));
	if (glSysParam.ucTMSSyncDial)
	{
		memcpy(&glCommCfg.stPSTNPara.stPara, &glSysParam._TxnModemPara, sizeof(COMM_PARA));
	}

	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	SetOffBase(OffBaseDisplay);

	// begin to download
	if( sgNewTMS )
	{
		iRet = NewTmsDownLoad(ucCommType);
	}
	else
	{
		iRet = OldTmsDownLoad();
	}

	SetOffBase(NULL);
	
	if( iRet!=0 )
	{
		unsigned char szResult[100];
		CommOnHook(FALSE);
		sprintf(szResult, "%s\n   %d", _T("INITIAL FAIL"), iRet);
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szResult, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return;
	}

	Gui_ClearScr();
	PubBeepOk();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INIT FINISHED."), gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
}

#ifdef ENABLE_EMV
void RemoveEmvAppCapk(void)
{
	int				iCnt;
	int				iRet;
	EMV_CAPK		stEmvCapk;
	EMV_APPLIST		stEmvApp;

	for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stEmvCapk, 0, sizeof(EMV_CAPK));
		iRet = EMVGetCAPK(iCnt, &stEmvCapk);
		if( iRet==EMV_OK )
		{
			EMVDelCAPK(stEmvCapk.KeyID, stEmvCapk.RID);
		}
	}
	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
		iRet = EMVGetApp(iCnt, &stEmvApp);
		if( iRet==EMV_OK )
		{
			EMVDelApp(stEmvApp.AID, (int)stEmvApp.AidLen);
		}
	}
}
#endif

// Modified by Kim_LinHB 2014-6-8
int SelectDownloadMode(uchar *pucCommType)
{
	int		iRet;
	uchar	ucComm;
	GUI_MENU	stSmDownMode;
	int iSelected;
	GUI_MENUITEM stDefCommMenuItem[] =
	{
		{ "MODEM", 1, TRUE, NULL},
		{ "TCPIP", 2,TRUE,  NULL},
		{ "GPRS", 3,TRUE,  NULL},
		{ "CDMA", 4,TRUE,  NULL},
		{ "OLD ASYNC", 5,TRUE,  NULL},
		{ "OLD SYNC", 6,TRUE,  NULL},
		{ "RS232", 7,TRUE,  NULL},
		{ "LOAD DEFAULT", 8,TRUE,  NULL},
		{ "WIFI", 9, TRUE,  NULL}, 
		{ "BLUETOOTH", 10, FALSE,  NULL},
		{ "USB", 11, TRUE,  NULL},
		{ "WCDMA", 12,TRUE,  NULL},
		{ "", -1,FALSE,  NULL},
	};// This menu does not provide translation
	GUI_MENUITEM stCommMenuItem[20];
	int iMenuItemNum = 0;

	//--------------------------------------------------

	if (!ChkHardware(HWCFG_MODEM, HW_NONE) && stDefCommMenuItem[0].bVisible)
	{
	    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[0], sizeof(GUI_MENUITEM));
		sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[0].szText);
	    ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_LAN, HW_NONE) && stDefCommMenuItem[1].bVisible)
	{
        memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[1], sizeof(GUI_MENUITEM));
        sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[1].szText);
        ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_GPRS, HW_NONE) && stDefCommMenuItem[2].bVisible)
	{
        memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[2], sizeof(GUI_MENUITEM));
        sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[2].szText);
        ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_CDMA, HW_NONE) && stDefCommMenuItem[3].bVisible)
	{
	    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[3], sizeof(GUI_MENUITEM));
        sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[3].szText);
        ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_WCDMA, HW_NONE) && stDefCommMenuItem[11].bVisible)  // added by  Gillian 2015/11/23
	{
	    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[11], sizeof(GUI_MENUITEM));
	    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[11].szText);
	    ++iMenuItemNum;
	}

    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[6], sizeof(GUI_MENUITEM));
    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[6].szText);
    ++iMenuItemNum;

   	if (!ChkHardware(HWCFG_WIFI, HW_NONE) && stDefCommMenuItem[8].bVisible)	//
	{
         memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[8], sizeof(GUI_MENUITEM));
         sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[8].szText);
         ++iMenuItemNum;
	}

    if (!ChkHardware(HWCFG_MODEM, HW_NONE))
    {
        if(stDefCommMenuItem[4].bVisible)
        {
            memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[4], sizeof(GUI_MENUITEM));
            sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[4].szText);
            ++iMenuItemNum;
        }

        if(stDefCommMenuItem[5].bVisible)
        {
            memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[5], sizeof(GUI_MENUITEM));
            sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[5].szText);
            ++iMenuItemNum;
        }
    }

    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[10], sizeof(GUI_MENUITEM));
    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[10].szText);
    ++iMenuItemNum;

#ifdef ALLOW_NO_TMS
   	memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[7], sizeof(GUI_MENUITEM));
    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[7].szText);
    ++iMenuItemNum;
#endif

    stCommMenuItem[iMenuItemNum].szText[0] = 0;

	memset(&stSmDownMode, 0, sizeof(stSmDownMode));
	Gui_BindMenu("SELECT MODE", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stCommMenuItem, &stSmDownMode);

	Gui_ClearScr();
	iSelected = 0;
	iRet = Gui_ShowMenuList(&stSmDownMode, 0, USER_OPER_TIMEOUT, &iSelected);

	if (iRet != GUI_OK)
	{
		return ERR_USERCANCEL;
	}

	sgSyncDial  = FALSE;
	sgNewTMS    = TRUE;
	ucComm      = CT_NONE;
	switch(iSelected)
	{
	case 1:
		ucComm = CT_MODEM;
		break;
	case 2:
		ucComm = CT_TCPIP;
		break;
	case 3:
		ucComm = CT_GPRS;
	    break;
	case 4:
		ucComm = CT_CDMA;
	    break;
	case 5:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
	    break;
	case 6:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
		sgSyncDial = TRUE;
		break;
	case 7:
		ucComm = CT_RS232;
		sgSyncDial = TRUE;
		break;
	case 8:
		NoDownloadInit();
		return ERR_NO_DISP;
	case 9:
		ucComm = CT_WIFI;
		break;
	case 10:
		ucComm = CT_BLTH;
		//Hidden by Kim_LinHB 2014-8-16 v1.01.0004
		//sgNewTMS = FALSE;
		//sgSyncDial = TRUE;
		break;
    case 11:
        ucComm = CT_USB;
        break;
    case 12:
    	ucComm = CT_WCDMA;  // added by  Gillian 2015/11/23
    	break;
	default:
	    return ERR_NO_DISP;
	}

	if( GetDownLoadComm(ucComm)!=0 )
	{
		return ERR_NO_DISP;
	}

	iRet = GetDownLoadTID(glSysParam.stEdcInfo.szDownLoadTID);
	if (iRet!=0)
	{
		return ERR_NO_DISP;
	}

	// save TMS download settings.
	glSysParam.stTMSCommCfg.ucCommType = ucComm;
	glSysParam.stTMSCommCfg.stRS232Para.ucSendMode = (sgSyncDial ? CM_SYNC : CM_ASYNC);
	glSysParam.stTMSCommCfg.stPSTNPara.ucSendMode  = (sgSyncDial ? CM_SYNC : CM_ASYNC);
	glSysParam.ucNewTMS      = sgNewTMS;
	glSysParam.ucTMSSyncDial = sgSyncDial;
	SaveSysParam();

	*pucCommType = ucComm;
	return 0;
}

// MonitorÏÂÔØÎÄ¼þµÄ²ÎÊýÏÂÔØ
// New Protims download protocol, done by monitor level.
int NewTmsDownLoad(uchar ucCommType)
{
#ifndef _WIN32
	int				iRet;
	T_INCOMMPARA	stCommPara;
	//TMS_LOADSTATUS	stLoadStatus;
	T_LOADSTATUS	stLoadStatus;
	COMM_PARA		stModemPara;
	uchar			szTelNo[25+1+1], szTermID[8+1];
	uchar szBuff[100];

	TCPIP_PARA stLocalInfo;

	memset(&stCommPara, 0, sizeof(T_INCOMMPARA));

    stCommPara.psProtocol = 0;
    stCommPara.ucCallMode = 0;
    stCommPara.bLoadType = 0xFF;

	stCommPara.psAppName  = (uchar *)"";		//(uchar *)AppInfo.AppName;
	sprintf((char *)szTermID, "%.8s", glSysParam.stEdcInfo.szDownLoadTID);
	stCommPara.psTermID   = szTermID;


	switch(ucCommType)
	{
	case CT_MODEM:
		memset(&stModemPara, 0, sizeof(COMM_PARA));
		stModemPara.CHDT    = 0x00;
		stModemPara.DT1     = 0x0A;//5;
		stModemPara.DT2     = 0x0A;//5;
		stModemPara.HT      = 0x64;
		stModemPara.WT      = 0x0A;
		stModemPara.SSETUP  = 0xE7; //0x87;	/* asynchronise link */
		stModemPara.DTIMES  = 0;
		stModemPara.AsMode  = 0x70;
		stModemPara.TimeOut = 6;	// 60 seconds
		stCommPara.bCommMode  = 1;		// modem
		if( glProcInfo.bAutoDownFlag )
		{
			stCommPara.ucCallMode = 0x11;
		}
		sprintf((char *)szTelNo, "%.25s.", glSysParam.stEdcInfo.szDownTelNo);
		stCommPara.tUnion.tModem.psTelNo   = szTelNo;
		stCommPara.tUnion.tModem.bTimeout  = 1;
		stCommPara.tUnion.tModem.ptModPara = &stModemPara;
		break;
	case CT_RS232:
		stCommPara.bCommMode = 0;
		stCommPara.tUnion.tSerial.psPara = (uchar *)glSysParam._TmsRS232Para.szAttr;
		break;
	case CT_TCPIP:
		stCommPara.bCommMode = 2;
		stCommPara.tUnion.tLAN.psLocal_IP_Addr = glSysParam._TmsTcpIpPara.szLocalIP;
		stCommPara.tUnion.tLAN.wLocalPortNo = 1010;
		//stCommPara.tUnion.tLAN.wLocalPortNo = 2;
		stCommPara.tUnion.tLAN.psSubnetMask	= glSysParam._TmsTcpIpPara.szNetMask; 
		stCommPara.tUnion.tLAN.psGatewayAddr = glSysParam._TmsTcpIpPara.szGatewayIP;
		stCommPara.tUnion.tLAN.psRemote_IP_Addr = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tLAN.wRemotePortNo = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
	case CT_GPRS:
	case CT_CDMA:
		stCommPara.bCommMode = ((ucCommType==CT_GPRS) ? 3 : 4);
		stCommPara.tUnion.tGPRS.psAPN      = stCommPara.tUnion.tCDMA.psTelNo    = glSysParam._TmsWirlessPara.szAPN;
		stCommPara.tUnion.tGPRS.psUserName = stCommPara.tUnion.tCDMA.psUserName = glSysParam._TmsWirlessPara.szUID;
		stCommPara.tUnion.tGPRS.psPasswd   = stCommPara.tUnion.tCDMA.psPasswd   = glSysParam._TmsWirlessPara.szPwd;
		stCommPara.tUnion.tGPRS.psPIN_CODE = stCommPara.tUnion.tCDMA.psPIN_CODE = glSysParam._TmsWirlessPara.szSimPin;
		stCommPara.tUnion.tGPRS.psIP_Addr  = stCommPara.tUnion.tCDMA.psIP_Addr  = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tGPRS.nPortNo    = stCommPara.tUnion.tCDMA.nPortNo    = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
		// added by  Gillian 2015/11/23
	case CT_WCDMA:
		stCommPara.bCommMode = 9;
		stCommPara.tUnion.tGPRS.psAPN      = stCommPara.tUnion.tCDMA.psTelNo    = glSysParam._TmsWirlessPara.szAPN;
		stCommPara.tUnion.tGPRS.psUserName = stCommPara.tUnion.tCDMA.psUserName = glSysParam._TmsWirlessPara.szUID;
		stCommPara.tUnion.tGPRS.psPasswd   = stCommPara.tUnion.tCDMA.psPasswd   = glSysParam._TmsWirlessPara.szPwd;
		stCommPara.tUnion.tGPRS.psPIN_CODE = stCommPara.tUnion.tCDMA.psPIN_CODE = glSysParam._TmsWirlessPara.szSimPin;
		stCommPara.tUnion.tGPRS.psIP_Addr  = stCommPara.tUnion.tCDMA.psIP_Addr  = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tGPRS.nPortNo    = stCommPara.tUnion.tCDMA.nPortNo    = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
	case CT_WIFI:
		sprintf(stLocalInfo.szLocalIP, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Ip[0],
			glSysParam._TmsWifiPara.stParam.Ip[1],
			glSysParam._TmsWifiPara.stParam.Ip[2],
			glSysParam._TmsWifiPara.stParam.Ip[3]);
		sprintf(stLocalInfo.szNetMask, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Mask[0],
			glSysParam._TmsWifiPara.stParam.Mask[1],
			glSysParam._TmsWifiPara.stParam.Mask[2],
			glSysParam._TmsWifiPara.stParam.Mask[3]);
		sprintf(stLocalInfo.szGatewayIP, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Gate[0],
			glSysParam._TmsWifiPara.stParam.Gate[1],
			glSysParam._TmsWifiPara.stParam.Gate[2],
			glSysParam._TmsWifiPara.stParam.Gate[3]);
		sprintf(stLocalInfo.szDNSIP, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Dns[0],
			glSysParam._TmsWifiPara.stParam.Dns[1],
			glSysParam._TmsWifiPara.stParam.Dns[2],
			glSysParam._TmsWifiPara.stParam.Dns[3]);

		stCommPara.bCommMode = 6;//

		stCommPara.tUnion.tWIFI.Wifi_SSID =  glSysParam._TmsWifiPara.stLastAP.Ssid;
		if(WLAN_SEC_WEP == glSysParam._TmsWifiPara.stLastAP.SecMode)
		{
#ifdef _MIPS_
			stCommPara.tUnion.tWIFI.psPasswd =  glSysParam._TmsWifiPara.stParam.Wep;
#else
			stCommPara.tUnion.tWIFI.psPasswd =  glSysParam.stTMSCommCfg.stWifiPara.stParam.Wep.Key[0];
#endif
			stCommPara.tUnion.tWIFI.Encryption_Mode = 2;
			stCommPara.tUnion.tWIFI.Encryption_Index = 1;
		}
		else
		{	
			stCommPara.tUnion.tWIFI.psPasswd = glSysParam._TmsWifiPara.stParam.Wpa;
			if(WLAN_SEC_WPA_WPA2 == glSysParam._TmsWifiPara.stLastAP.SecMode)
			{
				stCommPara.tUnion.tWIFI.Encryption_Mode = 3;
			}
			else
			{
				stCommPara.tUnion.tWIFI.Encryption_Mode = 4;
			}
		}
		stCommPara.tUnion.tWIFI.Local_IP = stLocalInfo.szLocalIP;
		stCommPara.tUnion.tWIFI.Local_PortNo = 1010;
		stCommPara.tUnion.tWIFI.SubnetMask	=stLocalInfo.szNetMask;
		stCommPara.tUnion.tWIFI.GatewayAddr = stLocalInfo.szGatewayIP;
		stCommPara.tUnion.tWIFI.Dns1 = stLocalInfo.szDNSIP;
		stCommPara.tUnion.tWIFI.DHCP_Flag = glSysParam._TmsWifiPara.stParam.DhcpEnable;

		stCommPara.tUnion.tWIFI.Remote_IP_Addr = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tWIFI.RemotePortNo = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
		
	case CT_BLTH:
		stCommPara.bCommMode = 0;
		stCommPara.tUnion.tSerial.psPara = (uchar *)"115200,8,n,1";
		break;
	case CT_USB:
	    stCommPara.bCommMode = 7;
	    break;
	default:
		return ERR_NO_DISP;
	}
	
	iRet = RemoteLoadApp(&stCommPara);
	// Modified by Kim_LinHB 2014-6-8
	SaveEdcParam();
	CommOnHook(FALSE);
	Gui_ClearScr();
	sprintf(szBuff, "%d", iRet);
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 3, NULL);
	if( iRet!=0 )
	{
		return iRet;
	}

	//memset(&stLoadStatus, 0, sizeof(TMS_LOADSTATUS));
	// Removed by Kim_LinHB 2014-08-14 1.01.0003 cuz it is just for old version
// 	memset(&stLoadStatus, 0, sizeof(T_LOADSTATUS));
// 	iRet = GetLoadedAppStatus((uchar *)"", &stLoadStatus);
// 	if( iRet!=0 )
// 	{
// 		return iRet;
// 	}
//
//	if( stLoadStatus.bAppTotal!=0 )
	{
		strcpy(szBuff, _T("SYSTEM UPDATED."));
		strcat(szBuff,"\n");
		if (!ChkTerm(_TERMINAL_S90_))	// S90 do not support soft-reboot
		{
			strcat(szBuff, _T("REBOOT..."));
			Gui_ClearScr();
			Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 3, NULL);
			Reboot();
		}
		strcat(szBuff, _T("PLS REBOOT POS."));
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		while(1);
	}

	return 0;
#else
	Gui_ClearScr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("NOT IMPLEMENT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	return ERR_TRAN_FAIL;
#endif
}

// ´«Í³µÄ8583·½Ê½µÄ²ÎÊýÏÂÔØ
// Traditional Protims download protocol, by application level, through ISO8583 message packet
int OldTmsDownLoad(void)
{
	int		iRet;
	long	lDataLen;

	sgEMVDownloaded = 0;

	sprintf((char *)glCurAcq.szNii,    "%.3s", glSysParam.stEdcInfo.szTMSNii);
	sprintf((char *)glCurAcq.szTermID, "%.8s", glSysParam.stEdcInfo.szDownLoadTID);

	glCurAcq.ucPhoneTimeOut = glSysParam.stEdcInfo.ucTMSTimeOut;
	glCurAcq.ucTcpTimeOut   = glSysParam.stEdcInfo.ucTMSTimeOut;
	glCurAcq.ucPppTimeOut   = glSysParam.stEdcInfo.ucTMSTimeOut;
	glCurAcq.ucGprsTimeOut  = glSysParam.stEdcInfo.ucTMSTimeOut;

	SetCommReqField();
	ReadSN(glTMSSend.szField61);
	iRet = SendRecvPacket();
	if( iRet!=0 )
	{
		return iRet;
	}
	iRet = AfterTranProc();
	if( iRet!=0 )
	{
		return iRet;
	}

	ResetAllPara(FALSE);

	while( 1 )
	{
		memset(sgTempBuf, 0, sizeof(sgTempBuf));
		lDataLen = (long)PubChar2Long(glTMSRecv.sField60, 2);
		memcpy(sgTempBuf, &glTMSRecv.sField60[2], (int)lDataLen);

		if( glTMSRecv.szProcCode[LEN_PROC_CODE-1]!='0' )
		{
			SetCommReqField();
			glTMSSend.szProcCode[LEN_PROC_CODE-1] = '1';
			ReadSN(glTMSSend.szField61);
			iRet = SendPacket();
			if( iRet!=0 )
			{
				PubTRACE1("txd tms:%d", iRet);
				return iRet;
			}
		}

		DispProcess();
		if( UnpackPara(sgTempBuf, lDataLen)!=0 )
		{
			PubTRACE0("unpack tms");
			return ERR_NO_DISP;
		}
		if( glTMSRecv.szProcCode[LEN_PROC_CODE-1]=='0' )
		{
			break;
		}

		iRet = RecvPacket();
		if( iRet!=0 )
		{
			PubTRACE1("rxd tms:%d", iRet);
			return iRet;
		}
		iRet = AfterTranProc();
		if( iRet!=0 )
		{
			PubTRACE1("proc tms:%d", iRet);
			return iRet;
		}
#ifdef _WIN32
		DelayMs(300);
#endif
	}

	CommOnHook(FALSE);
	DispProcess();

	if( !SearchIssuerKeyArray() )
	{
		return ERR_NO_DISP;
	}

	AfterLoadParaProc();
	return 0;
}

// save EDC parameter
void UnpackParaEdc(const uchar *psPara)
{
	TMS_EDC_INFO	*pstEdc;
	CURRENCY_CONFIG	stCurrency;

	pstEdc = (TMS_EDC_INFO*)psPara;
	glSysParam.stEdcInfo.ucDllTracking = pstEdc->ucDllTracking;
	glSysParam.stEdcInfo.bClearBatch   = pstEdc->bClearBatch;
	glSysParam.stEdcInfo.ucPrinterType = pstEdc->ucPrinterType;
	glSysParam.stEdcInfo.ucEcrSpeed    = pstEdc->ucEcrSpeed;
	//glSysParam.stEdcInfo.ucLanguage    = pstEdc->ucLanguage;
	glSysParam.stEdcInfo.ucCurrencySymbol = pstEdc->ucCurrencySymbol;
	glSysParam.stEdcInfo.ucTranAmtLen  = (uchar)PubBcd2Long(&pstEdc->ucTranAmtLen, 1);
	glSysParam.stEdcInfo.ucStlAmtLen   = (uchar)PubBcd2Long(&pstEdc->ucStlAmtLen,  1);
	glSysParam.stEdcInfo.stLocalCurrency.ucDecimal = (uchar)PubBcd2Long(&pstEdc->ucDecimalPos, 1);

	SetTime(pstEdc->sInitTime);
	memcpy(glSysParam.stEdcInfo.szInitTime, "20", 2);
	PubBcd2Asc(pstEdc->sInitTime, 6, &glSysParam.stEdcInfo.szInitTime[2]);

	PubBcd2Asc(pstEdc->sHelpTelNo, 12, glSysParam.stEdcInfo.szHelpTelNo);
	PubTrimTailChars(glSysParam.stEdcInfo.szHelpTelNo, 'F');
	if(0 == memcmp(glSysParam.stEdcInfo.szHelpTelNo, "000000000000", 12))
		glSysParam.stEdcInfo.szHelpTelNo[0] = 0;

	glSysParam.stEdcInfo.sOption[0] = pstEdc->ucOption1;
	glSysParam.stEdcInfo.sOption[1] = pstEdc->ucOption2;
	glSysParam.stEdcInfo.sOption[2] = pstEdc->ucOption3;
	glSysParam.stEdcInfo.sOption[3] = pstEdc->ucPwdMask;
	glSysParam.stEdcInfo.sOption[4] = pstEdc->ucDialOption;

	glSysParam.stEdcInfo.sReserved[0] = pstEdc->ucUnused1;
	memcpy(&glSysParam.stEdcInfo.sReserved[1], pstEdc->sUnused2, 3);
	
	//modified by Kim_LinHB 2014-6-7
	sprintf(glSysParam.stEdcInfo.szMerchantAddr, "%.46s", pstEdc->sMerchantAddr);
	sprintf(glSysParam.stEdcInfo.szMerchantName, "%.23s", pstEdc->sMerchantName);
	memcpy(glSysParam.stEdcInfo.szAddlPrompt,   pstEdc->sAddlPrompt,   20);
	sprintf(glSysParam.stEdcInfo.stLocalCurrency.szName, "%.3s", pstEdc->sCurrencyName);
	if (glSysParam.stEdcInfo.stLocalCurrency.szName[2]==' ')
	{
		glSysParam.stEdcInfo.stLocalCurrency.szName[2] = 0;
	}
	glSysParam.stEdcInfo.ulOfflineLimit = PubBcd2Long(pstEdc->sOfflineLimit, 5);

	// Protims ²»Ìá¹© ignore digit£¬±ØÐëÓÉEDC×Ô¼ºÅÐ¶Ï
	// unsupported to get ignore digit in Protims, do it in application 
	if (FindCurrency(glSysParam.stEdcInfo.stLocalCurrency.szName, &stCurrency)==0)
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit = stCurrency.ucIgnoreDigit;
		memcpy(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, stCurrency.sCurrencyCode, 2);
        memcpy(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, stCurrency.sCountryCode, 2);
		// Èç¹û²é±íµÃµ½¸Ã»õ±ÒÖ§³Ö½Ç·Ö£¬µ«ProtimsÉèÖÃdecimal  positionÎª0Î»£¨¼´ÉèÖÃÎª²»ÊäÈë½Ç·Ö£©
		// ÔòÓ¦°Ñ½Ç·Ö¼ÆÈëºöÂÔÎ»
		// if terminal does not ignore any digit, but decimal position was set with 0 in Protims,
		// then terminal should ignore the discrepancy
		// e.g. terminal supports 123.45, Protims supports 123
		// then terminal should ignore "45"
		if (glSysParam.stEdcInfo.stLocalCurrency.ucDecimal<stCurrency.ucDecimal)
		{
			glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit += (stCurrency.ucDecimal-glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
		}
	}
	else
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit = 0;
	}

	memcpy(glSysParam.stEdcInfo.sInitSTAN, pstEdc->sInitialSTAN, 3);

	// save password
	PubBcd2Asc0(pstEdc->sVoidPwd,   2, glSysParam.sPassword[PWD_VOID]);
	PubBcd2Asc0(pstEdc->sRefundPwd, 2, glSysParam.sPassword[PWD_REFUND]);
	PubBcd2Asc0(pstEdc->sSettlePwd, 2, glSysParam.sPassword[PWD_SETTLE]);
	PubBcd2Asc0(pstEdc->sAdjustPwd, 2, glSysParam.sPassword[PWD_ADJUST]);
	PubBcd2Asc0(pstEdc->sTermPwd,   2, glSysParam.sPassword[PWD_TERM]);

    // Printer type
	glSysParam.stEdcInfo.ucPrinterType = pstEdc->ucPrinterType;
	if (!ChkTerm(_TERMINAL_S60_))
	{
		if (ChkHardware(HWCFG_PRINTER, 'T')==TRUE)
		{
			glSysParam.stEdcInfo.ucPrinterType = 1;
		}
		else
		{
			glSysParam.stEdcInfo.ucPrinterType = 0;
		}
	}
}

void TransformIP(const uchar * ip_in, uchar * ip_out)
{
	sprintf(ip_out, "%u.%u.%u.%u", ip_in[0],ip_in[1],ip_in[2],ip_in[3]);
}

void TransformPort(const uchar * port_in, uchar * port_out)
{
	int iPortNum;
	iPortNum = port_in[0]*256+port_in[1];
	sprintf(port_out, "%d", iPortNum );
}

// save acquirer parameters
int UnpackParaAcq(uchar ucIndex, const uchar *psPara, ulong ulSubFieldLen)
{
	uchar			ucNum, szBuff[10];
	TMS_ACQUIRER	*pstAcq;

	if( glSysParam.ucAcqNum >= MAX_ACQ )
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF ACQ"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	ucNum  = glSysParam.ucAcqNum;
	pstAcq = (TMS_ACQUIRER *)psPara;
//	PubTRACEHEX("acq", psPara, sizeof(TMS_ACQUIRER));

	glSysParam.stAcqList[ucNum].ucKey = pstAcq->ucKey;
	memcpy(glSysParam.stAcqList[ucNum].szName,    pstAcq->sName,    10);
	memcpy(glSysParam.stAcqList[ucNum].szPrgName, pstAcq->sPrgName, 10);

	memcpy(glSysParam.stAcqList[ucNum].sOption, pstAcq->sOption, 4);

	PubBcd2Asc0(pstAcq->sNii, 2, szBuff);
// 	PubTRACE1("NII[%s]", szBuff);
	memcpy(glSysParam.stAcqList[ucNum].szNii,        &szBuff[1],          3);

	memcpy(glSysParam.stAcqList[ucNum].szTermID,     pstAcq->sTermID,     8);
	memcpy(glSysParam.stAcqList[ucNum].szMerchantID, pstAcq->sMerchantID, 15);

	glSysParam.stAcqList[ucNum].ucPhoneTimeOut = (uchar)PubBcd2Long(&pstAcq->ucTimeOut, 1);
	if( memcmp(pstAcq->sCurBatchNo, "\x00\x00\x00", 3)!=0 )
	{
		glSysParam.stAcqList[ucNum].ulCurBatchNo = PubBcd2Long(pstAcq->sCurBatchNo, 3);
	}
	if( glSysParam.stAcqList[ucNum].ulCurBatchNo==0L )
	{
		glSysParam.stAcqList[ucNum].ulCurBatchNo++;
	}
	glSysParam.stAcqList[ucNum].ulNextBatchNo = GetNewBatchNo(glSysParam.stAcqList[ucNum].ulCurBatchNo);

	memcpy(glSysParam.stAcqList[ucNum].szVisa1TermID, pstAcq->sVisa1TermID, 23); //Alex
	memcpy(glSysParam.stAcqList[ucNum].sReserved,     pstAcq->sReserved,    4);

	//------------------------ process modem parameter ------------------------
	if(pstAcq->sTxnTelNo1[0]!=0xff)
	{
		PubBcd2Asc0(pstAcq->sTxnTelNo1 , 12, glSysParam.stAcqList[ucNum].TxnTelNo1);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].TxnTelNo1, 'F');
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[0].ucDialWait     = pstAcq->ucTxnDialWait1;
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[0].ucDialAttempts = pstAcq->ucTxnDialAttempts1;

		PubBcd2Asc0(pstAcq->sTxnTelNo2, 12, glSysParam.stAcqList[ucNum].TxnTelNo2);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].TxnTelNo2, 'F');
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[1].ucDialWait     = pstAcq->ucTxnDialWait2;
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[1].ucDialAttempts = pstAcq->ucTxnDialAttempts2;

		PubBcd2Asc0(pstAcq->sStlTelNo1, 12, glSysParam.stAcqList[ucNum].StlTelNo1);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].StlTelNo1, 'F');
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialWait     = pstAcq->ucStlDialWait1;
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialAttempts = pstAcq->ucStlDialAttempts1;

		PubBcd2Asc0(pstAcq->sStlTelNo2, 12, glSysParam.stAcqList[ucNum].StlTelNo2);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].StlTelNo2, 'F');
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialWait     = pstAcq->ucStlDialWait2;
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialAttempts = pstAcq->ucStlDialAttempts2;

		glSysParam.stAcqList[ucNum].ucTxnModemMode = pstAcq->ucTxnModemMode;
		glSysParam.stAcqList[ucNum].ucStlModemMode = pstAcq->ucStlModemMode;
	}


	// TMS_ACQUIRER is defined to same as in new protims
	// so if using old protims/tims, the input data length will far less than sizeof(TMS_ACQUIRER)
	// to avoid the error caused by memory align, use "sizeof(TMS_ACQUIRER)-8" for comparison
	if (ulSubFieldLen > sizeof(TMS_ACQUIRER)-8)
	{
		memcpy(glSysParam.stAcqList[ucNum].sCommTypes, pstAcq->COMM_Mode, 8);

		//------------------------ process GPRS parameter ------------------------
		if ( memcmp(pstAcq->WIR_txn_IPADD1,"\x00\x00\x00\x00",4)!=0 )
		{
			// For APN,UID,PWD,SIMPIN, EDC only adopt ACQ's first available set
			// All ACQ share them. So only need to setup in one (or none) of those acquirer
			if ((pstAcq->WIR_APN[0]!=0) && (pstAcq->WIR_APN[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szAPN, pstAcq->WIR_APN, 64);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szAPN, ' ');
			}
			if ((pstAcq->WIR_USER[0]!=0) && (pstAcq->WIR_USER[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szUID, pstAcq->WIR_USER, 64);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szUID, ' ');
			}
			if ((pstAcq->WIR_PWD[0]!=0) && (pstAcq->WIR_PWD[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szPwd, pstAcq->WIR_PWD, 64);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szPwd, ' ');
			}
			if ((pstAcq->WIR_SIMPIN[0]!=0) && (pstAcq->WIR_SIMPIN[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szSimPin, pstAcq->WIR_SIMPIN, 16);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szSimPin, ' ');
			}

			TransformIP(pstAcq->WIR_txn_IPADD1,glSysParam.stAcqList[ucNum].stTxnGPRSInfo[0].szIP);
			TransformPort(pstAcq->WIR_txn_IPPORT1, glSysParam.stAcqList[ucNum].stTxnGPRSInfo[0].szPort);

			TransformIP(pstAcq->WIR_txn_IPADD2,glSysParam.stAcqList[ucNum].stTxnGPRSInfo[1].szIP);
			TransformPort(pstAcq->WIR_txn_IPPORT2, glSysParam.stAcqList[ucNum].stTxnGPRSInfo[1].szPort);

			TransformIP(pstAcq->WIR_stl_IPADD1,glSysParam.stAcqList[ucNum].stStlGPRSInfo[0].szIP);
			TransformPort(pstAcq->WIR_stl_IPPORT1, glSysParam.stAcqList[ucNum].stStlGPRSInfo[0].szPort);

			TransformIP(pstAcq->WIR_stl_IPADD2,glSysParam.stAcqList[ucNum].stStlGPRSInfo[1].szIP);
			TransformPort(pstAcq->WIR_stl_IPPORT2, glSysParam.stAcqList[ucNum].stStlGPRSInfo[1].szPort);

			glSysParam.stAcqList[ucNum].ucGprsTimeOut = pstAcq->WIR_timeout_val;
		}

		if ( memcmp(pstAcq->TCP_txn_IPADD1,"\x00\x00\x00\x00",4)!=0 )
		{
			TransformIP(pstAcq->TCP_txn_IPADD1, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[0].szIP);
			TransformPort(pstAcq->TCP_txn_IPPORT1, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[0].szPort);

			TransformIP(pstAcq->TCP_txn_IPADD2, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[1].szIP);
			TransformPort(pstAcq->TCP_txn_IPPORT2, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[1].szPort);

			TransformIP(pstAcq->TCP_stl_IPADD1, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[0].szIP);
			TransformPort(pstAcq->TCP_stl_IPPORT1, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[0].szPort);

			TransformIP(pstAcq->TCP_stl_IPADD2, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[1].szIP);
			TransformPort(pstAcq->TCP_stl_IPPORT2, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[1].szPort);

			glSysParam.stAcqList[ucNum].ucTcpTimeOut = pstAcq->TCP_timeout_val;
		}

		if ( memcmp(pstAcq->PPP_txn_IPADD1,"\x00\x00\x00\x00",4)!=0 )
		{
			PubBcd2Asc0(pstAcq->PPP_txn_PHONE, 12, glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szTelNo, 'F');
			TransformIP(pstAcq->PPP_txn_IPADD1, glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szIPAddr);
			TransformPort(pstAcq->PPP_txn_IPPORT1, glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szUserName, pstAcq->PPP_txn_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szUserPWD, pstAcq->PPP_txn_USERPWD, 16);

			PubBcd2Asc0(pstAcq->PPP_txn_PHONE, 12, glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szTelNo, 'F');
			TransformIP(pstAcq->PPP_txn_IPADD2, glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szIPAddr);
			TransformPort(pstAcq->PPP_txn_IPPORT2, glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szUserName, pstAcq->PPP_txn_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szUserPWD, pstAcq->PPP_txn_USERPWD, 16);

			PubBcd2Asc0(pstAcq->PPP_stl_PHONE, 12, glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szTelNo, 'F');
			TransformIP(pstAcq->PPP_stl_IPADD1, glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szIPAddr);
			TransformPort(pstAcq->PPP_stl_IPPORT1, glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szUserName, pstAcq->PPP_stl_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szUserPWD, pstAcq->PPP_stl_USERPWD, 16);

			PubBcd2Asc0(pstAcq->PPP_stl_PHONE, 12, glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szTelNo, 'F');
			TransformIP(pstAcq->PPP_stl_IPADD2, glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szIPAddr);
			TransformPort(pstAcq->PPP_stl_IPPORT2, glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szUserName, pstAcq->PPP_stl_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szUserPWD, pstAcq->PPP_stl_USERPWD, 16);

			glSysParam.stAcqList[ucNum].ucPppTimeOut = pstAcq->PPP_timeout_val;
		}
	}

	glSysCtrl.sAcqStatus[ucNum] = S_USE;
	glSysParam.ucAcqNum++;

	return 0;
}

// save issuer parameters
// Modified by Kim_LinHB 2014-6-8
int UnpackParaIssuer(uchar ucIndex, const uchar *psPara)
{
	uchar		ucNum;
	TMS_ISSUER	*pstIssuer;

	if( glSysParam.ucIssuerNum >= MAX_ISSUER )
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF ISSUER"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	pstIssuer = (TMS_ISSUER *)psPara;
	ucNum = glSysParam.ucIssuerNum;

	glSysParam.stIssuerList[ucNum].ucKey = pstIssuer->ucKey;
	memcpy(glSysParam.stIssuerList[ucNum].szName, pstIssuer->sName, 10);
	PubBcd2Asc(pstIssuer->sRefTelNo, 12, glSysParam.stIssuerList[ucNum].szRefTelNo);
	PubTrimTailChars(glSysParam.stIssuerList[ucNum].szRefTelNo, 'F');
	memcpy(glSysParam.stIssuerList[ucNum].sOption, pstIssuer->sOption, 4);

	glSysParam.stIssuerList[ucNum].ucDefAccount = pstIssuer->ucDefAccount;

	glSysParam.stIssuerList[ucNum].sPanMask[0] = pstIssuer->sReserved[0];
	glSysParam.stIssuerList[ucNum].sPanMask[1] = pstIssuer->sPanMask[1];
	glSysParam.stIssuerList[ucNum].sPanMask[2] = pstIssuer->sPanMask[0];

	glSysParam.stIssuerList[ucNum].ulFloorLimit = PubBcd2Long(pstIssuer->sFloorLimit, 2);
	glSysParam.stIssuerList[ucNum].ucAdjustPercent = (uchar)PubBcd2Long(&pstIssuer->ucAdjustPercent, 1);
	glSysParam.stIssuerList[ucNum].ucReserved = pstIssuer->sReserved[1];

	glSysParam.ucIssuerNum++;

	return 0;
}

// save card range parameters
// Modified by Kim_LinHB 2014-6-8
int UnpackParaCard(uchar ucIndex, const uchar *psPara)
{
	CARD_TABLE	*pstCardTbl;

	if( glSysParam.ucCardNum >= MAX_CARD )
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	pstCardTbl = &glSysParam.stCardTable[glSysParam.ucCardNum];
	memcpy(pstCardTbl, psPara, sizeof(CARD_TABLE));
	pstCardTbl->ucPanLength = (uchar)PubBcd2Long(&pstCardTbl->ucPanLength, 1);
	glSysParam.ucCardNum++;

	return 0;
}

// save instalment parameters
int UnpackInstPara(const uchar *psPara)
{
	uchar					ucNum;
	TMS_INSTALMENT_PLAN		*pstPlan;

	if( glSysParam.ucPlanNum >= MAX_PLAN )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF PLAN"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	pstPlan = (TMS_INSTALMENT_PLAN *)psPara;
	ucNum   = glSysParam.ucPlanNum;
	glSysParam.stPlanList[ucNum].ucIndex    = pstPlan->ucIndex;
	glSysParam.stPlanList[ucNum].ucAcqIndex = pstPlan->ucAcqIndex;
	sprintf((char *)glSysParam.stPlanList[ucNum].szName, "%.7s", pstPlan->sName);
	glSysParam.stPlanList[ucNum].ucMonths    = (uchar)PubBcd2Long(&pstPlan->ucMonths, 1);
	glSysParam.stPlanList[ucNum].ulBottomAmt = PubBcd2Long(pstPlan->sBottomAmt, 6);
	glSysParam.ucPlanNum++;

	return 0;
}

// save product descriptors
int UnpackDescPara(const uchar *psPara)
{
	uchar				ucNum;
	TMS_DESCRIPTOR		*pstDesc;

	if( glSysParam.ucDescNum >= MAX_DESCRIPTOR )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF DESC"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	pstDesc = (TMS_DESCRIPTOR *)(psPara+1);
	ucNum   = glSysParam.ucDescNum;
	glSysParam.stDescList[ucNum].ucKey = pstDesc->ucKey;
	sprintf((char *)glSysParam.stDescList[ucNum].szCode, "%.2s", pstDesc->sCode);
	sprintf((char *)glSysParam.stDescList[ucNum].szText, "%.20s", pstDesc->sText);
	glSysParam.ucDescNum++;

	return 0;
}

#ifdef ENABLE_EMV
// save EMV parameters
int SaveEmvMisc(const uchar *psPara)
{
	TMS_EMV_MISC	*pstEmvMisc;

	pstEmvMisc = (TMS_EMV_MISC *)psPara;
	EMVGetParameter(&glEmvParam);

	memcpy(glEmvParam.CountryCode,   pstEmvMisc->sCourtryCode,  2);
	memcpy(glEmvParam.TransCurrCode, pstEmvMisc->sCurcyCode,    2);
	memcpy(glEmvParam.ReferCurrCode, pstEmvMisc->sRefCurcyCode, 2);
	glEmvParam.TransCurrExp = pstEmvMisc->ucCurcyExp;
	glEmvParam.ReferCurrExp = pstEmvMisc->ucRefCurcyExp;
//	pstEmvMisc->Language;	// Unused

	EMVSetParameter(&glEmvParam);
	return 0;
}
#endif

#ifdef ENABLE_EMV
// save EMV application
int SaveEmvApp(const uchar *psPara)
{
	int				iRet;
	TMS_EMV_APP		*pstApp;
	EMV_APPLIST		stEmvApp;

	pstApp = (TMS_EMV_APP *)(psPara+1);
	memset(&stEmvApp, 0, sizeof(EMV_APPLIST));

	if( pstApp->bLocalName )
	{
		memcpy(stEmvApp.AppName, pstApp->sLocalName, (int)MIN(16, pstApp->ucLocalNameLen));
	}

	memcpy(stEmvApp.AID, pstApp->sAID, (int)MIN(16, pstApp->ucAIDLen));
	stEmvApp.AidLen   = pstApp->ucAIDLen;
	stEmvApp.SelFlag  = (pstApp->ucASI==0) ? PART_MATCH : FULL_MATCH;
//	stEmvApp.Priority = returned by card, not used here;

	stEmvApp.TargetPer       = pstApp->ucTargetPer;
	stEmvApp.MaxTargetPer    = pstApp->ucMaxTargetPer;
	if( sgNewTMS )
	{
		stEmvApp.FloorLimitCheck = ((pstApp->ucTermRisk & TRM_FLOOR_CHECK)!=0);
		stEmvApp.RandTransSel    = ((pstApp->ucTermRisk & TRM_RANDOM_TRAN_SEL)!=0);
		stEmvApp.VelocityCheck   = ((pstApp->ucTermRisk & TRM_VELOCITY_CHECK)!=0);
	}
	else
	{	// ¾É°æÐ­Òé²»ÏÂÔØÕâÐ©Êý¾Ý
		// those data are undefined in the old version of protocol
		stEmvApp.FloorLimitCheck = 1;
		stEmvApp.RandTransSel    = 1;
		stEmvApp.VelocityCheck   = 1;
	}

	stEmvApp.FloorLimit = PubChar2Long(pstApp->sFloorLimit, 4);
	stEmvApp.FloorLimit *= 100;		// floor, unit is YUAN
	stEmvApp.Threshold = PubChar2Long(pstApp->sThreshold,  4);

	memcpy(stEmvApp.TACDenial,  pstApp->sTACDenial,  5);
	memcpy(stEmvApp.TACOnline,  pstApp->sTACOnline,  5);
	memcpy(stEmvApp.TACDefault, pstApp->sTACDefault, 5);
//	memcpy(stEmvApp.AcquierId, // not set here

	stEmvApp.dDOL[0] = pstApp->ucTermDDOLLen;
	memcpy(&stEmvApp.dDOL[1], pstApp->sTermDDOL, stEmvApp.dDOL[0]);
	if( sgNewTMS )
	{
		stEmvApp.tDOL[0] = strlen((char *)pstApp->sTDOL)/2;
		PubAsc2Bcd(pstApp->sTDOL, (uint)stEmvApp.tDOL[0], &stEmvApp.tDOL[1]);
	}
	memcpy(stEmvApp.Version, pstApp->sAppVer, 2);
//	stEmvApp.RiskManData

	iRet = EMVAddApp(&stEmvApp);
	if( iRet!=EMV_OK )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("ERR SAVE EMV APP"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return iRet;
	}

	return 0;
}
#endif

#ifdef ENABLE_EMV
// save CAPK
int SaveEmvCapk(const uchar *psPara)
{
	int				iRet;
	TMS_EMV_CAPK	*pstCapk;
	EMV_CAPK		stEmvCapk;

	pstCapk = (TMS_EMV_CAPK *)(psPara+1);
	memset(&stEmvCapk, 0, sizeof(EMV_CAPK));

	memcpy(stEmvCapk.RID, pstCapk->sRID, 5);
	stEmvCapk.KeyID    = pstCapk->ucKeyID;
	stEmvCapk.HashInd  = pstCapk->ucHashInd;
	stEmvCapk.ArithInd = pstCapk->ucArithInd;
	stEmvCapk.ModulLen = pstCapk->ucModulLen;
	memcpy(stEmvCapk.Modul, pstCapk->sModul, stEmvCapk.ModulLen);
	stEmvCapk.ExponentLen = pstCapk->ucExpLen;
	memcpy(stEmvCapk.Exponent, pstCapk->sExponent, stEmvCapk.ExponentLen);
// PubTRACE3("%02X %02X %02X", pstCapk->sExpiry[0], pstCapk->sExpiry[1], pstCapk->sExpiry[2]);
	if( memcmp(pstCapk->sExpiry, "\x00\x00\x00", 3)!=0 )
	{
		memcpy(stEmvCapk.ExpDate, pstCapk->sExpiry, 3);
	}
	else
	{
		memcpy(stEmvCapk.ExpDate, "\x20\x12\x31", 3); // unknown expiry
	}
	memcpy(stEmvCapk.CheckSum, pstCapk->sCheckSum, 20);

	iRet = EMVAddCAPK(&stEmvCapk);
	if( iRet!=EMV_OK )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("ERR SAVE CAPK"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
//		return iRet;
	}

	return 0;
}
#endif

// parameter analyse control
int UnpackPara(const uchar *psParaData, long lDataLen)
{
	uchar	ucSubType, ucTemp, *psCurPtr, *psBack;
	ulong	ulSubFieldLen;

	psCurPtr = (uchar *)psParaData;
	while( psCurPtr<(psParaData+lDataLen) )
	{
		ucSubType = *psCurPtr;
		psCurPtr++;

		// ³¤¶È³¬¹ý9999×Ö½ÚÊ±,bcdÂëµÚÒ»¸ö×Ö½ÚÊÇ0xFn,F±íÊ¾³¤¶È³¬¹ý9999,n±íÊ¾ºóÃæÓÐ¼¸¸ö×Ö½ÚµÄbcdÂë
		// when length > 9999, first byte is 0xFn, "F" means >9999, "n" indicates number of following bytes
		if( (*psCurPtr & 0xF0)==0xF0 )
		{
			ulSubFieldLen = PubBcd2Long(psCurPtr+1, (uint)(*psCurPtr & 0x0F));
			psCurPtr += (*psCurPtr & 0x0F) + 1;
		}
		else
		{
			ulSubFieldLen = PubBcd2Long(psCurPtr, 2);
			psCurPtr += 2;
		}
		psBack = psCurPtr;
		ucTemp = (uchar)PubBcd2Long(psCurPtr, 1);
		if( ucTemp>0 )
		{
			ucTemp--;
		}

#ifdef ENABLE_EMV
		// If any EMV is downloaded, clear the old data
		switch(ucSubType)
		{
		case 10:
		case 11:
		case 12:
			if (!sgEMVDownloaded)
			{
				sgEMVDownloaded = 1;
				RemoveEmvAppCapk();
			}
			break;
		default:
			break;	
		}
#endif

		switch(ucSubType)
		{
		case 1:
			UnpackParaEdc(psCurPtr);
			break;

		case 2:
			if( UnpackParaCard(ucTemp, psCurPtr+1)!=0 )
			{
				return 1;
			}
			break;

		case 3:
			if( UnpackParaIssuer(ucTemp, psCurPtr+1)!=0 )
			{
				return 2;
			}
			break;

		case 4:
			if( UnpackParaAcq(ucTemp, psCurPtr+1, ulSubFieldLen)!=0 )
			{
				return 3;
			}
			break;

		case 5:
			if( UnpackDescPara(psCurPtr)!=0 )
			{
				return 4;
			}
			break;

		case 9:
			if( UnpackInstPara(psCurPtr)!=0 )
			{
				return 6;
			}
			break;

		case 10:
#ifdef ENABLE_EMV
			if( SaveEmvMisc(psCurPtr)!=0 )
			{
				return 7;
			}
#endif
			break;

		case 11:
#ifdef ENABLE_EMV
			if( SaveEmvApp(psCurPtr)!=0 )
			{
				return 8;
			}
#endif
			break;

		case 12:
#ifdef ENABLE_EMV
			if( SaveEmvCapk(psCurPtr)!=0 )
			{
				return 9;
			}
#endif
			break;

		case 20:
			if( SaveCardBin(psCurPtr)!=0 )
			{
				 return 20;
			}
			break;

		case 21:
		case 22:
			if( ulSubFieldLen>sizeof(glSysParam.sAdData) )
			{
				return 21;
			}
			glSysParam.bTextAdData = (ucSubType==21) ? TRUE : FALSE;
			memcpy(glSysParam.sAdData, psCurPtr, (int)ulSubFieldLen);
			break;

		case 23:
			memcpy(glSysParam.stEdcInfo.szCallInTime, psCurPtr, 8);
			break;

		default:
			break;
		}
		psCurPtr = psBack+ulSubFieldLen;
	}

	return 0;
}

// Éú³ÉÃ¿Ò»¸ö acq µÄsIssuerKey, TREU: ³É¹¦, FALSE: Ê§°Ü
// Generates sIssuerKey for every acquirer.
uchar SearchIssuerKeyArray(void)
{
	uchar	ucCnt, ucFlag;

	ucFlag = FALSE;
	for(ucCnt=0; ucCnt<glSysParam.ucAcqNum; ucCnt++)
	{
		if( (glSysParam.stAcqList[ucCnt].ucKey!=INV_ACQ_KEY) &&
			(glSysCtrl.sAcqStatus[ucCnt]==S_USE) )
		{
			ucFlag = TRUE;
			memset(glSysParam.stAcqList[ucCnt].sIssuerKey, (uchar)INV_ISSUER_KEY, MAX_ISSUER);
			SearchIssuerKeyArraySub(glSysParam.stAcqList[ucCnt].sIssuerKey, glSysParam.stAcqList[ucCnt].ucKey);
		}
	}

	return ucFlag;
}

void AfterLoadParaProc(void)
{
	glSysParam.stEdcInfo.uiInitFlag = EDC_INIT_OK;
	glSysParam.ucTermStatus         = TRANS_MODE;
	if( glSysParam.stEdcInfo.bClearBatch )
	{
		ClearRecord(ACQ_ALL);
	}
	if( memcmp(glSysParam.stEdcInfo.sInitSTAN, "\x00\x00\x00", 3)!=0 )
	{
		glSysCtrl.ulSTAN      = PubBcd2Long(glSysParam.stEdcInfo.sInitSTAN, 3);
		glSysCtrl.ulInvoiceNo = PubBcd2Long(glSysParam.stEdcInfo.sInitSTAN, 3);
	}

	SaveSysCtrlAll();
	SaveSysParam();
	SyncPassword();

#ifdef ENABLE_EMV
	SyncEmvCurrency(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
#endif
    
    if (memcmp(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, "\x00\x00", 2)==0)
    {
		// Modified by Kim_LinHB 2014-6-8
		unsigned char szBuff[200];
		sprintf(szBuff, "%s\n%s", _T("COUNTRY CODE"), _T("MISSING"));
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
    }
}

void SearchIssuerKeyArraySub(uchar *sIssuerKey, uchar ucAcqKey)
{
	uchar	ucCnt, sTempKey[256];
	int		iTemp, iKeyNum;

	memset(sTempKey, (uchar)INV_ISSUER_KEY, sizeof(sTempKey));
	for(ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
	{
		if( glSysParam.stCardTable[ucCnt].ucAcqKey==ucAcqKey &&
			glSysParam.stCardTable[ucCnt].ucAcqKey!=INV_ACQ_KEY )
		{	// Ïû³ýÖØ¸´,ÒòÎª¿ÉÄÜÒ»¸öissuer¶Ô¶à¸öcard
			// for ignoring repeats, because several cards may map to the same issuer 
			sTempKey[glSysParam.stCardTable[ucCnt].ucIssuerKey] = glSysParam.stCardTable[ucCnt].ucIssuerKey;
		}
	}

	for(iKeyNum=iTemp=0; iTemp<256; iTemp++)
	{
		if( sTempKey[iTemp]!=INV_ISSUER_KEY )
		{
			sIssuerKey[iKeyNum] = sTempKey[iTemp];
			iKeyNum++;
		}
	}
}

// get TMS telepnone number
// Modified by Kim_LinHB 2014-6-8
int GetDownLoadTelNo(void)
{
	int iRet;
	uchar	szBuff[30];
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 25;

	sprintf((char *)szBuff, "%.25s", glSysParam.stEdcInfo.szDownTelNo);
	
	Gui_ClearScr();
	iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("PHONE NO"), gl_stLeftAttr, 
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		return ERR_USERCANCEL;
	}

	sprintf((char *)glSysParam.stEdcInfo.szDownTelNo, "%.25s", szBuff);
	SaveEdcParam();

	return 0;
}

int GetDownLoadGprsPara(void)
{
	uchar	ucRet = SetWirelessParam(&glSysParam._TmsWirlessPara);
	SyncWirelessParam(&glSysParam._TxnWirlessPara, &glSysParam._TmsWirlessPara);

	return ucRet;
}

// Modified by Kim_LinHB 2014-6-8
int GetDownLoadLanPara(void)
{
	uchar	ucRet;

	SetCurrTitle("SETUP TCPIP");
	Gui_ClearScr();
	
	ucRet = SetTcpIpParam(&glSysParam._TmsTcpIpPara);
	if (ucRet==0)
	{
		SyncTcpIpParam(&glSysParam._TxnTcpIpPara, &glSysParam._TmsTcpIpPara);
		return 0;
	}

	return ucRet;
}

int GetDownLoadWIFIPara(void)
{
	int ret;

	SetCurrTitle("SETUP WIFI");

	ret = SetWiFiApp(&glSysParam._TmsWifiPara);
	if(ret != 0)
	{
		Beep();	
		DispWifiErrorMsg(ret);
		return 0;
	}
	SyncWifiParam(&glSysParam._TxnWifiPara, &glSysParam._TmsWifiPara);
	return 0;
}

int GetDownLoadComm(uchar ucCommType)
{
	int		iRet;

	switch(ucCommType)
	{
	case CT_RS232:
        iRet = SetRs232Param(&glSysParam._TmsRS232Para);
		break;
	case CT_MODEM:
		iRet = GetDownLoadTelNo();
		break;
	case CT_TCPIP:
		iRet = GetDownLoadLanPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
		break;
	case CT_WCDMA:
	case CT_CDMA:
	case CT_GPRS:
		iRet = GetDownLoadGprsPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
		break;
	case CT_WIFI:
		iRet=GetDownLoadWIFIPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
 		break;
	case CT_BLTH:
	case CT_USB:
		iRet = 0;
		break;
	default:
		iRet = ERR_NO_DISP;
		break;
	}

	return iRet;
}

// get TMS download terminal ID
// Modified by Kim_LinHB 2014-6-8
int GetDownLoadTID(uchar *pszID)
{
	uchar	szBuff[8+1];
	int		iRet;
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 8;
	stInputAttr.nMaxLen = 8;

	while (1)
	{
		sprintf((char *)szBuff, "%.8s", pszID);
		Gui_ClearScr();
		iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("DOWNLOAD ID"), gl_stLeftAttr, 
			szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if( iRet!=GUI_OK )
		{
			return ERR_USERCANCEL;
		}
		if (atol((char *)szBuff)!=0)
		{
			break;
		}
	}

	sprintf((char *)pszID, "%.8s", szBuff);
	return 0;
}

//void UpdateCommType(void)

// ¼ÓÔØ²ÎÊýÎÄ¼þ
// Load downloaded EDC parameter file (downloaded by new Protims protocol)
// Modified by Kim_LinHB 2014-6-8
int LoadSysDownEdcFile(void)
{
	int		fd, iRet, iLength, iLeftLenBytes;
	long	lOffset, lMaxOffset, lSubFieldLen;
	uchar	szCurTime[16+1], ucHeadLen;
	
	ShowLogs(1, "Loading EDC parameter file.");
	
	fd = open(DOWNPARA_FILE, O_RDWR);
	if( fd<0 )
	{
		return 1;
	}

	sgEMVDownloaded = 0;

	sgNewTMS = TRUE;
	GetEngTime(szCurTime);
	Gui_ClearScr();
	Gui_ShowMsgBox(szCurTime, gl_stTitleAttr, _T("Loading Param..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	ResetAllPara(FALSE);

	lMaxOffset = filesize(DOWNPARA_FILE);
	lOffset    = 0L;
	while( lOffset<lMaxOffset )
	{
		seek(fd, lOffset, SEEK_SET);
		memset(sgTempBuf, 0, sizeof(sgTempBuf));
		iLength = read(fd, sgTempBuf, 3);
		if( iLength!=3 )
		{
			break;
		}

		// ³¤¶È³¬¹ý9999×Ö½ÚÊ±,bcdÂëµÚÒ»¸ö×Ö½ÚÊÇ0xFn,F±íÊ¾³¤¶È³¬¹ý9999,n±íÊ¾ºóÃæÓÐ¼¸¸ö×Ö½ÚµÄbcdÂë
		// when length > 9999, first byte is 0xFn, "F" means >9999, "n" indicates number of following bytes
		if( (sgTempBuf[1] & 0xF0)==0xF0 )
		{
			iLeftLenBytes = (sgTempBuf[1] & 0x0F) - 1;
			if( iLeftLenBytes<=0 )
			{
				break;
			}
			iLength = read(fd, &sgTempBuf[3], iLeftLenBytes);
			if( iLength!=iLeftLenBytes )
			{
				break;
			}
			lSubFieldLen = (long)PubBcd2Long(&sgTempBuf[2], (uint)(iLeftLenBytes+1));
			ucHeadLen = (uchar)(2+(sgTempBuf[1] & 0x0F));
		}
		else
		{
			lSubFieldLen = (long)PubBcd2Long(&sgTempBuf[1], 2);
			ucHeadLen = 3;
		}
		if( lSubFieldLen+ucHeadLen+1>sizeof(sgTempBuf) )	// reserved a byte to keep '\0'
		{
			PubTRACE1("SubFieldLen:%ld", lSubFieldLen);
			break;
		}

		iLength = read(fd, &sgTempBuf[ucHeadLen], (int)lSubFieldLen);
		if( iLength!=(int)lSubFieldLen )
		{
			PubTRACE1("iLength:%d", iLength);
			break;
		}

		iRet = UnpackPara(sgTempBuf, lSubFieldLen+ucHeadLen);
		if( iRet!=0 )
		{
			PubTRACE1("unpack:%d", iRet);
			break;
		}
		lOffset += (lSubFieldLen + ucHeadLen);
	}
	close(fd);

	if( !(lOffset>=lMaxOffset && SearchIssuerKeyArray()) )
	{
		remove(DOWNPARA_FILE);
		GetEngTime(szCurTime);
		Gui_ClearScr();
		Gui_ShowMsgBox(szCurTime, gl_stTitleAttr, _T("Loading Failed!"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_TRAN_FAIL;
	}

	AfterLoadParaProc();
	remove(DOWNPARA_FILE);

	GetEngTime(szCurTime);
	Gui_ClearScr();
	Gui_ShowMsgBox(szCurTime, gl_stTitleAttr, _T("Loading Success!"), gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);

	return 0;
}

// ²úÉúÏÂÒ»¸ö×Ô¶¯ÏÂÔØÊ±¼ä
// To figure when will be the next time to download auto
void GetNextAutoDayTime(uchar *pszDateTimeInOut, ushort uiInterval)
{
	uchar	szBuff[50];
	int		iYear, iMonth, iDate, iHour, iMinute, iMaxDay;

	GetDateTime(szBuff);
	iYear  = (int)PubAsc2Long(&szBuff[2], 2);
	iMonth = (int)PubAsc2Long(&szBuff[4], 2);
	iDate  = (int)PubAsc2Long(&szBuff[6], 2);
	srand((uint)PubAsc2Long(&szBuff[10], 4));
	iHour   = rand()%6;		// hour generated must be in range 0 - 5
	iMinute = rand()%60;	// minute generated must be in range 0 - 59
	if( uiInterval>999 )
	{
		uiInterval = 999;
	}

	iDate += uiInterval;
	while( 1 )
	{
		switch( iMonth )
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			iMaxDay = 31;
			break;

		case 2:
			iMaxDay = 29;
			break;

		default:
			iMaxDay = 30;
			break;
		}

		if( iDate<=iMaxDay )
		{
			break;
		}
		iDate -= iMaxDay;
		iMonth++;

		if( iMonth>12 )
		{
			iMonth -= 12;
			iYear++;
		}
	}	// while( 1

	sprintf((char *)pszDateTimeInOut, "%02d%02d%02d%02d%02d", iYear%100, iMonth,
			iDate, iHour, iMinute);
}

void GetOneLine(uchar **psCurPtr, uchar *psData, int Maxlen)
{
#define ISSPACE(ch) ( ((ch)==' ')  || ((ch)=='\t') || \
					  ((ch)=='\n') || ((ch)=='\r') )
#define ISLINEEND(ch) ( ((ch)=='\n') || ((ch)=='\r') )
	uchar	*p, *q;

	for(p=*psCurPtr; *p && ISSPACE(*p); p++);

	*psData = 0;
	for(q=psData; *p && (q-psData<Maxlen) && !ISLINEEND(*p); )	*q++ = *p++;
	*q = 0;
	PubTrimStr(psData);

	for(; *p && !ISLINEEND(*p); p++);  // erase characters of the left lines
	*psCurPtr = p;
#undef ISSPACE
#undef ISLINEEND
}

int SaveDetail(const uchar *psData)
{
	uchar	*psCurPtr, *psBack, ucLen, ucCmpLen;
	uchar	szBuf[80], szStartNo[20+1], szEndNo[20+1];
	ushort	uiIndex;

	// exceed the size of card table
	if( glSysParam.uiCardBinNum >= MAX_CARDBIN_NUM )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("CARDBIN OVERFLOW"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return 1;
	}

	psBack   = (uchar *)psData;
	psCurPtr = (uchar *)strchr((char *)psBack, ',');
	if( psCurPtr==NULL )
	{
		return 1;
	}
	sprintf((char *)szBuf, "%.*s", (int)MIN(psCurPtr-psBack, 9), psBack);
	ucLen = (uchar)atol((char *)szBuf);

	psBack   = psCurPtr+1;
	psCurPtr = (uchar *)strchr((char *)psBack, ',');
	if( psCurPtr==NULL )
	{
		return 1;
	}
	sprintf((char *)szStartNo, "%.*s", (int)MIN(psCurPtr-psBack, 19), psBack);
	sprintf((char *)szEndNo,   "%.*s", (int)MIN(psCurPtr-psBack, 19), psCurPtr+1);
	if( strlen((char *)szStartNo)!=strlen((char *)szEndNo) )
	{
		return 1;
	}

	// save card bin record
	uiIndex = glSysParam.uiCardBinNum;
	glSysParam.stCardBinTable[uiIndex].ucPanLen      = ucLen;
	ucCmpLen = (uchar)strlen((char *)szStartNo);
	glSysParam.stCardBinTable[uiIndex].ucMatchLen    = ucCmpLen;
	glSysParam.stCardBinTable[uiIndex].ucIssuerIndex = (uchar)glSysParam.uiIssuerNameNum;

	memset(glSysParam.stCardBinTable[uiIndex].sStartNo, 0, sizeof(glSysParam.stCardBinTable[uiIndex].sStartNo));
	memset(glSysParam.stCardBinTable[uiIndex].sEndNo, (uchar)0xFF, sizeof(glSysParam.stCardBinTable[uiIndex].sEndNo));
	if( ucCmpLen % 2 )	// card length is odd, pad with a specific character
	{
		szStartNo[ucCmpLen] = '0';
		PubAsc2Bcd(szStartNo, (uint)(ucCmpLen+1), glSysParam.stCardBinTable[uiIndex].sStartNo);
		szEndNo[ucCmpLen]   = 'F';
		PubAsc2Bcd(szEndNo,   (uint)(ucCmpLen+1), glSysParam.stCardBinTable[uiIndex].sEndNo);
	}
	else
	{
		PubAsc2Bcd(szStartNo, (uint)ucCmpLen, glSysParam.stCardBinTable[uiIndex].sStartNo);
		PubAsc2Bcd(szEndNo,   (uint)ucCmpLen, glSysParam.stCardBinTable[uiIndex].sEndNo);
	}

	glSysParam.uiCardBinNum++;

	return 0;
}

// save card bin table for HK
int SaveCardBin(uchar *psCardBinInOut)
{
	uchar	*psCurPtr, *psBack, ucFlag;
	uchar	szBuf[80], szChineseName[16+1], szEnglishName[MAX_CARBIN_NAME_LEN+1];
	ushort	uiIndex;

	psCurPtr = psCardBinInOut;
	while( *psCurPtr )
	{
		if( glSysParam.uiIssuerNameNum >= MAX_CARDBIN_ISSUER )
		{
			return 1;
		}

		// search Tag [Issuer]
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if( PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")!=0 )
		{
			continue;
		}

		// get Chinese name
		psBack = psCurPtr;
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if(PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0 )
		{
			psCurPtr = psBack;
			continue;
		}
		sprintf((char *)szChineseName, "%.16s", szBuf);

		// get English name
		psBack = psCurPtr;
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if( PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0 )
		{
			psCurPtr = psBack;
			continue;
		}
		sprintf((char *)szEnglishName, "%.*s", MAX_CARBIN_NAME_LEN, szBuf);

		// get details
		ucFlag = 0;
		while( *psCurPtr )
		{
			psBack = psCurPtr;
			GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
			if( (szBuf[0]==0) || (PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0) )
			{
				psCurPtr = psBack;
				break;
			}
			if( SaveDetail(szBuf)!=0 )
			{
				return 1;
			}
			ucFlag = 1;
		}
		if( !ucFlag )	// ignore null detail lines!
		{
			continue;
		}

		// save Chinese/English name of issuer
		uiIndex = glSysParam.uiIssuerNameNum;
		sprintf((char *)glSysParam.stIssuerNameList[uiIndex].szChineseName, "%.16s", szChineseName);
		sprintf((char *)glSysParam.stIssuerNameList[uiIndex].szEnglishName, "%.*s", MAX_CARBIN_NAME_LEN, szEnglishName);
		glSysParam.uiIssuerNameNum++;
	}

	return 0;
}

void LoadEdcLang(void)
{
	SetSysLang(1);
	ShowLogs(1, "Setting Language to 1 == English");
#ifdef AREA_Arabia
    CustomizeAppLibForArabiaLang( strcmp(LANGCONFIG, "Arabia")==0 );
#endif
}

void LoadDefaultLang(void)
{
	SetCurrTitle(_T("SELECT LANG")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	SetSysLang(0);
	SetCurrTitle("");
#ifdef AREA_Arabia
    CustomizeAppLibForArabiaLang( strcmp(LANGCONFIG, "Arabia")==0 );
#endif
}


#ifdef _WIN32
int GetLoadedAppStatus(uchar *psAppName, TMS_LOADSTATUS *ptStat)
{
	// Modified by Kim_LinHB 2014-6-8
	Gui_ClearScr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("NOT IMPLEMENT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	return -1;
}
#endif

#ifdef ALLOW_NO_TMS
// processing of loading default parameters without downloading
void InitEdcParam(void)
{
	CURRENCY_CONFIG		stLocalCurrency;
	int iIAcqCnt = 0;
	int iIssuerCnt = 0;
	int iCardTableCnt = 0;
	char temp[128] = {0};

	GetDefCurrency(&stLocalCurrency);
	
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("merName", temp);
	//sprintf((char *)glSysParam.stEdcInfo.szMerchantName, "MERCHANT NAME");
	sprintf((char *)glSysParam.stEdcInfo.szMerchantName, temp);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("merAddr", temp);
	//sprintf((char *)glSysParam.stEdcInfo.szMerchantAddr, "MERCHANT ADDR");
	sprintf((char *)glSysParam.stEdcInfo.szMerchantAddr, temp);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	//sprintf((char *)&glSysParam.stEdcInfo.szDownLoadTID, "00000000");
	sprintf((char *)&glSysParam.stEdcInfo.szDownLoadTID, temp);

	memcpy(glSysParam.stEdcInfo.sOption, "\xE6\x28\x00\x09\x00", 5);
	glSysParam.stEdcInfo.ucTranAmtLen     = 10;
	glSysParam.stEdcInfo.ucStlAmtLen      = 12;

	// ÕâÀïÏÈÉèÄ¬ÈÏ¡£Ö®ºó»áÓÐÒ»¸öÊÖ¶¯ÐÞ¸ÄµÄ»ú»á
	// set with default parameters, enable to modify later
	glSysParam.stEdcInfo.stLocalCurrency  = stLocalCurrency;
	glSysParam.stEdcInfo.ucCurrencySymbol = ' ';

	if (ChkHardware(HWCFG_PRINTER, 'S')==TRUE)
	{
		glSysParam.stEdcInfo.ucPrinterType = 0;
	}
	else
	{
		glSysParam.stEdcInfo.ucPrinterType = 1;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set acquirer parameters
	glSysCtrl.sAcqStatus[iIAcqCnt] 		= S_USE;
	glSysParam.stAcqList[iIAcqCnt].ucKey   = 0x01;
	glSysParam.stAcqList[iIAcqCnt].ucIndex = 0;
	sprintf((char *)glSysParam.stAcqList[iIAcqCnt].szNii,     "019");
	sprintf((char *)glSysParam.stAcqList[iIAcqCnt].szName, "VM_ACQ");
	sprintf((char *)glSysParam.stAcqList[iIAcqCnt].szPrgName, "VISA");
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	sprintf((char *)glSysParam.stAcqList[iIAcqCnt].szTermID,  temp);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	//sprintf((char *)glSysParam.stAcqList[iIAcqCnt].szTermID,  "00000001");
	//sprintf((char *)glSysParam.stAcqList[iIAcqCnt].szMerchantID, "123456789012345");
	sprintf((char *)glSysParam.stAcqList[iIAcqCnt].szMerchantID, temp);

	memcpy(glSysParam.stAcqList[iIAcqCnt].sOption, "\x06\x20\x0E\x00", 4);
	glSysParam.stAcqList[iIAcqCnt].TxnTelNo1[0] = 0;
	glSysParam.stAcqList[iIAcqCnt].TxnTelNo2[0] = 0;
	glSysParam.stAcqList[iIAcqCnt].StlTelNo1[0] = 0;
	glSysParam.stAcqList[iIAcqCnt].StlTelNo2[0] = 0;
	memset(glSysParam.stAcqList[iIAcqCnt].stTxnTCPIPInfo, 0, 2 *sizeof(IP_ADDR));
	memset(glSysParam.stAcqList[iIAcqCnt].stStlTCPIPInfo, 0, 2 *sizeof(IP_ADDR));
	glSysParam.stAcqList[iIAcqCnt].ulCurBatchNo  = 1L;
	glSysParam.stAcqList[iIAcqCnt].ulNextBatchNo = 2L;
	glSysParam.stAcqList[iIAcqCnt].ucPhoneTimeOut = 30;
	glSysParam.stAcqList[iIAcqCnt].ucTcpTimeOut   = 30;
	glSysParam.stAcqList[iIAcqCnt].ucPppTimeOut   = 30;
	glSysParam.stAcqList[iIAcqCnt].ucGprsTimeOut  = 30;
	memcpy(glSysParam.stAcqList[iIAcqCnt].sIssuerKey, "\x01\x02\x03\x04\x05\x06\x07", 7);// visa master other // modified by Kim 20150116 bug 611 820
	++iIAcqCnt;
	glSysParam.ucAcqNum = iIAcqCnt;

	ShowLogs(1, "Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set issuer parameters

	glSysParam.stIssuerList[iIssuerCnt].ucKey = 0x01;
	sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szRefTelNo, "86");
	sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szName, "VISA");
//	memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFC\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFE\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[iIssuerCnt].sPanMask, "\x00\xFF\xF0", 3);
	glSysParam.stIssuerList[iIssuerCnt].ulFloorLimit    = 0;
	glSysParam.stIssuerList[iIssuerCnt].ucAdjustPercent = 10;
	++iIssuerCnt;

	glSysParam.stIssuerList[iIssuerCnt].ucKey = 0x02;
	sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szRefTelNo, "86");
	sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szName, "MASTER");
//	memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFC\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFE\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[iIssuerCnt].sPanMask, "\x00\xFF\xF0", 3);
	glSysParam.stIssuerList[iIssuerCnt].ulFloorLimit    = 0;
	glSysParam.stIssuerList[iIssuerCnt].ucAdjustPercent = 10;
	++iIssuerCnt;

	glSysParam.stIssuerList[iIssuerCnt].ucKey = 0x03;
    sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szRefTelNo, "86");
    sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szName, "AMEX");
//    memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFC\x1E\x10\x00", 4);
    memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFE\x1E\x10\x00", 4);
    memcpy(glSysParam.stIssuerList[iIssuerCnt].sPanMask, "\x00\xFF\xF0", 3);
    glSysParam.stIssuerList[iIssuerCnt].ulFloorLimit    = 0;
    glSysParam.stIssuerList[iIssuerCnt].ucAdjustPercent = 10;
    ++iIssuerCnt;

	glSysParam.stIssuerList[iIssuerCnt].ucKey = 0x04;
    sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szRefTelNo, "86");
    sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szName, "DINERS");
//    memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFC\x1E\x10\x00", 4);
    memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFE\x1E\x10\x00", 4);
    memcpy(glSysParam.stIssuerList[iIssuerCnt].sPanMask, "\x00\xFF\xF0", 3);
    glSysParam.stIssuerList[iIssuerCnt].ulFloorLimit    = 0;
    glSysParam.stIssuerList[iIssuerCnt].ucAdjustPercent = 10;
    ++iIssuerCnt;

    glSysParam.stIssuerList[iIssuerCnt].ucKey = 0x05;
    sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szRefTelNo, "86");
    sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szName, "JCB");
//    memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFC\x1E\x10\x00", 4);
    memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFE\x1E\x10\x00", 4);
    memcpy(glSysParam.stIssuerList[iIssuerCnt].sPanMask, "\x00\xFF\xF0", 3);
    glSysParam.stIssuerList[iIssuerCnt].ulFloorLimit    = 0;
    glSysParam.stIssuerList[iIssuerCnt].ucAdjustPercent = 10;
    ++iIssuerCnt;

	glSysParam.stIssuerList[iIssuerCnt].ucKey = 0x06;
	sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szRefTelNo, "86");
	sprintf((char *)glSysParam.stIssuerList[iIssuerCnt].szName,     "OTHER");
//	memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption,  "\xFC\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[iIssuerCnt].sOption, "\xFE\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[iIssuerCnt].sPanMask, "\x00\xFF\xF0",     3);
	glSysParam.stIssuerList[iIssuerCnt].ulFloorLimit    = 0;
	glSysParam.stIssuerList[iIssuerCnt].ucAdjustPercent = 10;
	++iIssuerCnt;
	
	glSysParam.ucIssuerNum = iIssuerCnt;
	ShowLogs(1, "Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set card range
	// VISA
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x40\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x49\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
	glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
	++iCardTableCnt;

	//MASTER
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x50\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x59\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x02;
	glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
	glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
	++iCardTableCnt;

	//AE1 added by Kevinliu 20160505
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x34\x00\x00\x00\x00", 5);
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x34\x99\x99\x99\x99", 5);
	//AE2 added by Kevinliu 20160505
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[1].sPanRangeLow,  "\x37\x00\x00\x00\x00", 5);
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[1].sPanRangeHigh, "\x37\x99\x99\x99\x99", 5);
    glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x03;
    glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
    glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
    glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
    ++iCardTableCnt;

	//Diners added by Kevinliu 20160505
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x36\x00\x00\x00\x00", 5);
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x36\x99\x99\x99\x99", 5);
    glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x04;
    glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
    glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
    glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
    ++iCardTableCnt;

	//JCB
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x35\x00\x00\x00\x00", 5);
//    memcpy(glSysParam.stCardTable[iCardTableCnt].sPanRangeHigh, "\x39\x99\x99\x99\x99", 5);	//modified by Kevinliu 20160505
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x35\x99\x99\x99\x99", 5);
    glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x05;
    glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
    glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
    glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
    ++iCardTableCnt;

	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x60\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x99\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x06;
	glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
	glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
	++iCardTableCnt;

	glSysParam.ucCardNum = iCardTableCnt;

	ShowLogs(1, "Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
}
#endif

// end of file

