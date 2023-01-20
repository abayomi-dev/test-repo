
#include "global.h"




#ifdef ENABLE_EMV




/********************** Internal macros declaration ************************/
// macros for analyze EMV TLV string
#define TAGMASK_CLASS			0xC0	// tag mask of tag class
#define TAGMASK_CONSTRUCTED		0x20	// tag mask of constructed/primitive data
#define TAGMASK_FIRSTBYTE		0x1F	// tag mask of first byte
#define TAGMASK_NEXTBYTE		0x80	// tag mask of next byte

#define LENMASK_NEXTBYTE		0x80	// length mask
#define LENMASK_LENBYTES		0x7F	// mask of bytes of lenght

#define TAG_NULL_1				0x00	// null tag
#define TAG_NULL_2				0xFF	// null tag

#define DE55_LEN_FIXED		0x01	// for amex
#define DE55_LEN_VAR1		0x02
#define DE55_LEN_VAR2		0x03

#define DE55_MUST_SET		0x10	// ±ØÐë´æÔÚ
#define DE55_OPT_SET		0x20	// ¿ÉÑ¡Ôñ´æÔÚ
#define DE55_COND_SET		0x30	// ¸ù¾ÝÌõ¼þ´æÔÚ
/********************** Internal structure declaration *********************/
// callback function for GetTLVItem() to save TLV value
typedef void (*SaveTLVData)(uint uiTag, const uchar *psData, int iDataLen);

typedef struct _tagDE55Tag
{
	ushort	uiEmvTag;
	uchar	ucOption;
	uchar	ucLen;		// for process AMEX bit 55, no used for ADVT/TIP
}DE55Tag;

typedef struct _tagScriptInfo
{
	ushort	uiTag;
	int		iIDLen;
	uchar	sScriptID[4];
	int		iCmdLen[20];
	uchar	sScriptCmd[20][300];
}ScriptInfo;

/********************** Internal functions declaration *********************/
static int  SetAmexDE55(const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  AppendStdTagList(DE55Tag *pstList, ushort uiTag, uchar ucOption, uchar ucMaxLen);
static int  RemoveFromTagList(DE55Tag *pstList, ushort uiTag);
static int  SetStdDE55(uchar bForUpLoad, const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  SetStdDE56(const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll);
static int  GetSpecTLVItem(const uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen);
static int  GetDE55Amex(const uchar *psSendHeader, const uchar *psRecvDE55, int iLen);
static int  IsConstructedTag(uint uiTag);
static void SaveRspICCData(uint uiTag, const uchar *psData, int iDataLen);
static void BuildTLVString(ushort uiEmvTag, const uchar *psData, int iLength, uchar **ppsOutData);
static void SaveEmvData(void);
static void AdjustIssuerScript(void);
static void SaveScriptData(uint uiTag, const uchar *psData, int iDataLen);
static void PackTLVData(uint uiTag, const uchar *psData, uint uiDataLen, uchar *psOutData, int *piOutLen);
static void PackTLVHead(uint uiTag, uint uiDataLen, uchar *psOutData, int *piOutLen);
static int  CalcTLVTotalLen(uint uiTag, uint uiDataLen);
static void PackScriptData(void);
static void SaveTVRTSI(uchar bBeforeOnline);
static void UpdateEntryModeForOfflinePIN(void);

/********************** Internal variables declaration *********************/
static uchar sAuthData[16];			// authentication data from issuer
static uchar sIssuerScript[300];	// issuer script
// { // for test only
// 	0x71, 0x12+0x0F, 0x9F, 0x18, 0x00, 0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// 	0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// 	0x72, 0x12+4, 0x9F, 0x18, 0x04, 0,1,2,3, 0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// };
//static int iScriptLen=40+15+4;

static int			sgAuthDataLen, sgScriptLen;
static ScriptInfo	sgScriptInfo;
static uchar		sgScriptBak[300];
static int			sgCurScript, bHaveScript, sgScriptBakLen;
int offlinePinCancel = 0;

// AMEX format field 55
static DE55Tag sgAmexTagList[] =
{
	{0x9F26, DE55_LEN_FIXED, 8},
	{0x9F10, DE55_LEN_VAR1,  32},
	{0x9F37, DE55_LEN_FIXED, 4},
	{0x9F36, DE55_LEN_FIXED, 2},
	{0x95,   DE55_LEN_FIXED, 5},
	{0x9A,   DE55_LEN_FIXED, 3},
	{0x9C,   DE55_LEN_FIXED, 1},
	{0x9F02, DE55_LEN_FIXED, 6},
	{0x5F2A, DE55_LEN_FIXED, 2},
	{0x9F1A, DE55_LEN_FIXED, 2},
	{0x82,   DE55_LEN_FIXED, 2},
	{0x9F03, DE55_LEN_FIXED, 6},
	{0x5F34, DE55_LEN_FIXED, 1},
	{0x9F27, DE55_LEN_FIXED, 1},
	{0x9F06, DE55_LEN_VAR1,  16},
	{0x9F09, DE55_LEN_FIXED, 2},
	{0x9F34, DE55_LEN_FIXED, 3},
	{0x9F0E, DE55_LEN_FIXED, 5},
	{0x9F0F, DE55_LEN_FIXED, 5},
	{0x9F0D, DE55_LEN_FIXED, 5},
	{0x84, DE55_LEN_FIXED, 7},
	{0},
};

// Ïû·Ñ/(Ô¤)ÊÚÈ¨,55ÓòEMV±êÇ©, TLV format
// tags data in field 55 of transaction sale/(pre-)authorization (TLV format)
static DE55Tag sgStdEmvTagList[] =
{
	//Packing it Nibss Way
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F37, DE55_MUST_SET, 0},
	{0x9F36, DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9A,   DE55_MUST_SET, 0},
	{0x9C,   DE55_MUST_SET, 0},
	{0x9F02, DE55_MUST_SET, 0},
	{0x5F2A, DE55_MUST_SET, 0},
	{0x82,   DE55_MUST_SET, 0},
	{0x9F1A, DE55_MUST_SET, 0},
	{0x9F34, DE55_MUST_SET, 0},
	{0x9F33, DE55_MUST_SET, 0},
	{0x9F35, DE55_MUST_SET, 0},
	{0x9F03, DE55_MUST_SET, 0},
	{0x84, DE55_MUST_SET, 0},
	{0},

	/*{0x9F26, DE55_MUST_SET, 8},
	{0x9F27, DE55_MUST_SET, 1},
	{0x9F10, DE55_MUST_SET, 20},
	{0x9F37, DE55_MUST_SET, 4},
	{0x9F36, DE55_MUST_SET, 2},
	{0x95,   DE55_MUST_SET, 5},
	{0x9A,   DE55_MUST_SET, 3},
	{0x9C,   DE55_MUST_SET, 1},
	{0x9F02, DE55_MUST_SET, 6},
	{0x5F2A, DE55_MUST_SET, 2},
	{0x82,   DE55_MUST_SET, 2},
	{0x9F1A, DE55_MUST_SET, 2},
	{0x9F34, DE55_MUST_SET, 3},
	{0x9F33, DE55_MUST_SET, 3},
	{0x9F35, DE55_MUST_SET, 1},
	{0x9F03, DE55_MUST_SET, 6},
	{0},

	/*{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F37, DE55_MUST_SET, 0},
	{0x9F36, DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9A,   DE55_MUST_SET, 0},
	{0x9C,   DE55_MUST_SET, 0},
	{0x9F02, DE55_MUST_SET, 0},
	{0x5F2A, DE55_MUST_SET, 0},
	{0x82,   DE55_MUST_SET, 0},
	{0x9F1A, DE55_MUST_SET, 0},
	{0x9F34, DE55_MUST_SET, 0},
	{0x9F33, DE55_MUST_SET, 0},
	{0x9F35, DE55_MUST_SET, 0},
	{0x9F03, DE55_MUST_SET, 0},
	{0},

	/*
	{0x5F2A, DE55_MUST_SET, 0},
	{0x5F34, DE55_MUST_SET, 1}, // notice it's limited to L=1
	{0x82,   DE55_MUST_SET, 0},
	{0x84,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9A,   DE55_MUST_SET, 0},
	{0x9C,   DE55_MUST_SET, 0},
	{0x9F02, DE55_MUST_SET, 0},
	{0x9F03, DE55_MUST_SET, 0},
	{0x9F09, DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F1A, DE55_MUST_SET, 0},
	{0x9F1E, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0x9F33, DE55_MUST_SET, 0},
	{0x9F34, DE55_MUST_SET, 0},
	{0x9F35, DE55_MUST_SET, 0},
	{0x9F36, DE55_MUST_SET, 0},
	{0x9F37, DE55_MUST_SET, 0},
	{0x9F41, DE55_MUST_SET, 0},
	{0},*/
	
};


// Ïû·Ñ/(Ô¤)ÊÚÈ¨,56ÓòEMV±êÇ©, TLV format
static DE55Tag sgStdEmvField56TagList[] =
{
	{0x5A,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9B,   DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0},
};

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// This is NOT a callback of EMV lib
void AppSetMckParam(uchar bEnablePinBypass)
{
	//ÔÚ½»Ò×´¦ÀíÇ°µ÷ÓÃ½Ó¿Úº¯Êý  EMVSetMCKParam ÉèÖÃÊÇ·ñÊ¹ÓÃÅúÊý¾Ý²É¼¯
	EMV_MCKPARAM	stMckParam;
	EMV_EXTMPARAM	stExtMckParam;

	stMckParam.ucBypassPin = (bEnablePinBypass ? 1 : 0);
	stMckParam.ucBatchCapture = 1;
	memset(&stExtMckParam, 0, sizeof(stExtMckParam));
	stExtMckParam.ucUseTermAIPFlg = 1;
	stExtMckParam.aucTermAIP[0] = 0x08;	// always perform term risk management
	stExtMckParam.ucBypassAllFlg = 1;
	stMckParam.pvoid = &stExtMckParam;
	EMVSetMCKParam(&stMckParam);
}

// Set to default EMV parameter, since it may be changed during last transaction.
void InitTransEMVCfg(void)
{
	char asc[5] = {0};
	char bcd[3] = {0};
	char dts[256] = {0};
	char temp[128] = {0};
	AppSetMckParam(TRUE);
	//ShowLogs(1, "Done Setting EmvParam");
	EMVGetParameter(&glEmvParam);
	//memcpy(glEmvParam.Capability, "\xE0\xB0\xC8", 3);//\xE0\xF8\xC8
	//EMVSetTLVData(0x9F33, (uchar *)"\xE0\xB0\xC8", 3);
	memcpy(glEmvParam.Capability, "\xE0\xF8\xC8", 3);
	EMVSetTLVData(0x9F33, (uchar *)"\xE0\xF8\xC8", 3);
	EMVSetTLVData(0x9F35, (uchar *)"\x22", 1);
	EMVSetTLVData(0x9F09, (uchar *)"\x00\x02", 2);
	EMVSetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6);
	EMVSetTLVData(0x5F2A, (uchar *)"\x05\x66", 2);
	EMVSetTLVData(0x9F1A, (uchar *)"\x05\x66", 2);
	EMVSetTLVData(0x9C, (uchar *)"\x00", 1);
	memcpy(glEmvParam.ExCapability,  "\xE0\x00\xF0\xA0\x01", 5);
	memcpy(glEmvParam.TermId,  temp, 8);
	memcpy(glEmvParam.ReferCurrCode,  "\x05\x66", 2);
	memcpy(glEmvParam.CountryCode,  "\x05\x66", 2);
	memcpy(glEmvParam.TransCurrCode,  "\x05\x66", 2);
	memcpy(glEmvParam.MerchCateCode,  bcd, 2);
	glEmvParam.TransCurrExp = 0x02;
	glEmvParam.ReferCurrExp = 0x02;
	glEmvParam.TransType = EMV_GOODS;
	glEmvParam.ForceOnline   = 1;
	glEmvParam.GetDataPIN    = 1;//Try 0x01
	glEmvParam.SurportPSESel = 1;
	

	/*memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMNL", temp);
	memcpy(glEmvParam.MerchName,  temp, 40);
	//ShowLogs(1, "EMV MNL: %s", temp);
	memset(asc, '\0', strlen(asc));
	memset(bcd, '\0', strlen(bcd));
	UtilGetEnvEx("txnMCC", asc);
	//ShowLogs(1, "EMV MCC: %s", asc);
	PubAsc2Bcd(asc, 2, bcd);
	memcpy(glEmvParam.MerchCateCode,  (uchar *)bcd, 2);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	//ShowLogs(1, "EMV MID: %s", temp);
	memcpy(glEmvParam.MerchId,  temp, 15);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	//ShowLogs(1, "EMV TID: %s", temp);
	memcpy(glEmvParam.TermId,  temp, 8);
	memcpy(glEmvParam.ReferCurrCode,  "\x05\x66", 2);
	memcpy(glEmvParam.CountryCode,  "\x05\x66", 2);
	memcpy(glEmvParam.TransCurrCode,  "\x05\x66", 2);
	memcpy(glEmvParam.MerchCateCode,  bcd, 2);
	glEmvParam.TransCurrExp = 0x02;
	glEmvParam.ReferCurrExp = 0x02;
	glEmvParam.TransType = EMV_GOODS;
	glEmvParam.ForceOnline   = 1;
	glEmvParam.GetDataPIN    = 1;//Try 0x01
	glEmvParam.SurportPSESel = 1;*/
	EMVSetParameter(&glEmvParam);
}

// Modified by Kim_LinHB 2014-5-31
// for displaying a application list to card holder to select
// if there is only one application in the chip, then EMV kernel will not call this callback function
int cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum)
{
	int			iRet, iCnt, iAppCnt;
	GUI_MENU		stAppMenu;
	GUI_MENUITEM	stAppMenuItem[MAX_APP_NUM+1];
	APPLABEL_LIST	stAppList[MAX_APP_NUM];
	int iSelected = 0;

	//ShowLogs(1, "Inside cEMVWaitAppSel");
	if( TryCnt!=0 )
	{
		unsigned char szBuff[200];
		sprintf(szBuff, "%s\n%s", _T("NOT ACCEPT"), _T("PLS TRY AGAIN"));
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
	}
	//ShowLogs(1, "Inside cEMVWaitAppSel 1");
	EMVGetLabelList(stAppList, &iAppCnt);
	//ShowLogs(1, "Inside cEMVWaitAppSel 2");
	PubASSERT( AppNum<=MAX_APP_NUM );
	memset(stAppMenuItem, 0, sizeof(stAppMenuItem));
	for(iCnt=0; iCnt<iAppCnt && iCnt<MAX_APP_NUM; iCnt++)
	{
		stAppMenuItem[iCnt].bVisible = TRUE;	
		stAppMenuItem[iCnt].nValue = iCnt;
		stAppMenuItem[iCnt].vFunc = NULL;
 		sprintf((char *)stAppMenuItem[iCnt].szText, "%.16s", stAppList[iCnt].aucAppLabel);
	}
	strcpy(stAppMenuItem[iCnt].szText, "");
	//ShowLogs(1, "Inside cEMVWaitAppSel 3");

	Gui_BindMenu(_T("Select App"), gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stAppMenuItem, &stAppMenu);
	Gui_ClearScr();
	iSelected = 0;
	iRet = Gui_ShowMenuList(&stAppMenu, 0, USER_OPER_TIMEOUT, &iSelected);
	if( iRet != GUI_OK)
	{
		return EMV_USER_CANCEL;
	}
	//ShowLogs(1, "Inside cEMVWaitAppSel 4");
	return iSelected;
}

// it is acallback function for EMV kernel, 
// for displaying a amount input box,  
// developer customize
int cEMVInputAmount(ulong *AuthAmt, ulong *CashBackAmt)
{
	uchar	szTotalAmt[20];
	uchar   szBuff[32];

	if( glProcInfo.stTranLog.szAmount[0]!=0 )
	{
		PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
		*AuthAmt = atol((char *)szTotalAmt);
		if( CashBackAmt!=NULL )
		{
			*CashBackAmt = 0L;
		}
	}
	else
	{
		*AuthAmt = 0L;
		if( CashBackAmt!=NULL )
		{
			*CashBackAmt = 0L;
		}
	}
	
	if (glProcInfo.stTranLog.ucTranType ==CASH)
	{
		if( CashBackAmt==NULL )
		{
            if ((EMVReadVerInfo(szBuff)==EMV_OK) && (memcmp(szBuff, "v2", 2)==0))
            {
                // For EMV2x, "v28_7" etc. Not for EMV4xx
			    // Set cash back amount
			    EMVSetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6);
			    EMVSetTLVData(0x9F04, (uchar *)"\x00\x00\x00\x00", 4);
            }
		}
	}

	return EMV_OK;
}

// ´¦ÀíDOLµÄ¹ý³ÌÖÐ£¬EMV¿âÓöµ½²»Ê¶±ðµÄTAGÊ±»áµ÷ÓÃ¸Ã»Øµ÷º¯Êý£¬ÒªÇóÓ¦ÓÃ³ÌÐò´¦Àí
// Èç¹ûÓ¦ÓÃ³ÌÐòÎÞ·¨´¦Àí£¬ÔòÖ±½Ó·µ»Ø-1£¬Ìá¹©¸Ãº¯ÊýÖ»Îª½â¾öÒ»Ð©²»·ûºÏEMVµÄÌØÊâ
// Ó¦ÓÃ³ÌÐòµÄÒªÇó£¬Ò»°ãÇé¿öÏÂ·µ»Ø-1¼´¿É
// Callback function required by EMV core.
// When processing DOL, if there is a tag that EMV core doesn't know about, core will call this function.
// developer should offer processing for proprietary tag.
// if really unable to, just return -1
int cEMVUnknowTLVData(ushort iTag, uchar *psDat, int iDataLen)
{
	switch( iTag )
	{
		/*
		'C' = CASH DESBUS
		'Z' = ATM CASH
		'O' = COLLEGE SCHOOL
		'H' = HOTEL/SHIP
		'X' = TRANSFER
		'A' = AUTO MOBILE/RENTAL
		'F' = RESTAURANT
		'T' = TELEPHONE ORDER PREAUTH
		'U' = UNIQUE TRANS
		'R' = RENTAL/OTHER TRANS
		*/
	case 0x9F53:		// Transaction Category Code (TCC) - Master Card
		*psDat = 'R';	// 0x52;
		break;

	default:
		return -1;
	}

	return EMV_OK;
}

// Callback function required by EMV core.
// Wait holder enter PIN.
// developer customized.
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
int checkofflinepin = 0;
int cEMVGetHolderPwd(int iTryFlag, int iRemainCnt, uchar *pszPlainPin)
{
	int		iResult;
	uchar	ucRet, szBuff[30], szAmount[15];
	uchar	sPinBlock[8];

	checkofflinepin = 0;
	//ShowLogs(1, "cEMVGetHolderPwd Step 1");
	// online PIN
	if( pszPlainPin==NULL )
	{
		iResult = GetPIN(GETPIN_EMV);
		if( iResult==0 )
		{
			if( glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT )
			{
				checkofflinepin = 0;
				return EMV_OK;
			}
			else
			{
				checkofflinepin = 1;
				return EMV_NO_PASSWORD;
			}
		}
		else if( iResult==ERR_USERCANCEL )
		{
			checkofflinepin = 1;
			return EMV_USER_CANCEL;
		}
		else
		{
			checkofflinepin = 1;
			return EMV_NO_PINPAD;
		}
	}
	//ShowLogs(1, "cEMVGetHolderPwd Step 2");
	// Offline plain/enciphered PIN processing below
	Gui_ClearScr();
	if( iRemainCnt==1 )
	{
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr,_T("LAST ENTER PIN"), gl_stCenterAttr, GUI_BUTTON_NONE, 2, NULL);
	}

	//ShowLogs(1, "cEMVGetHolderPwd Step 3");

	PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szAmount);
	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	if( iTryFlag==0 )
	{
	    GetDispAmount(szAmount, szAmount);
		Gui_DrawText(szAmount, gl_stCenterAttr, 0, 25);
	}
	else
	{
		Gui_DrawText(_T("PIN ERR, RETRY"), gl_stCenterAttr, 0, 25);
	}
	//ShowLogs(1, "cEMVGetHolderPwd Step 4");
	ENTERPIN:
	Gui_DrawText(_T("PLS ENTER PIN"), gl_stCenterAttr, 0, 50);

	if (ChkTermPEDMode(PED_INT_PCI))
	{
		//ShowLogs(1, "cEMVGetHolderPwd Step 5");
		// Add by lirz v1.02.0000
		if(ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS) && ChkIfAmex() )
		{
			Gui_ShowMsgBox(NULL, gl_stTitleAttr, "by-pass not permitted", gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		}
	   // End add by lirz

		// Offline PIN, done by core itself since EMV core V25_T1. Application only needs to display prompt message.
		// In this mode, cEMVGetHolderPwd() will be called twice. the first time is to display message to user,
		// then back to kernel and wait PIN. afterwards kernel call this again and inform the process result.
		if (pszPlainPin[0]==EMV_PED_TIMEOUT)
		{
			// EMV core has processed PIN entry and it's timeout
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PED ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
			//ShowLogs(1, "cEMVGetHolderPwd Step 5a");
			checkofflinepin = 1;
			return EMV_TIME_OUT;
		}
		else if (pszPlainPin[0]==EMV_PED_WAIT)
		{
			// API is called too frequently
			DelayMs(1000);
			Gui_ClearScr();
			// Modified by Kim_LinHB 2014-8-11 v1.01.0003
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

			ScrGotoxy(32, 6);
			//ShowLogs(1, "cEMVGetHolderPwd Step 5b");
			checkofflinepin = 0;
			return EMV_OK;
		}
		else if (pszPlainPin[0]==EMV_PED_FAIL)
		{
			// EMV core has processed PIN entry and PED failed.
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PED ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
			//ShowLogs(1, "cEMVGetHolderPwd Step 5c");
			checkofflinepin = 1;
			return EMV_NO_PINPAD;
		}
		else
		{
			//ShowLogs(1, "cEMVGetHolderPwd Step 5d");
			// EMV PIN not processed yet. So just display.
			ScrGotoxy(32, 6);
			return EMV_OK;
		}
	}
	else if (ChkTermPEDMode(PED_EXT_PP))
	{
		//ShowLogs(1, "cEMVGetHolderPwd Step 6");
		Gui_DrawText(_T("PLS USE PINPAD"), gl_stCenterAttr, 0, 75);
		App_ConvAmountTran(szAmount, szBuff, 0);
		// show amount on PINPAD
		ucRet = PPScrCls();
		if( ucRet )
		{
			checkofflinepin = 1;
			return EMV_NO_PINPAD;
		}
		PPScrPrint(0, 0, szBuff);
		PPScrClrLine(1);

		memset(sPinBlock, 0, sizeof(sPinBlock));
		ucRet = PPEmvGetPwd(4, 12, sPinBlock);
		switch( ucRet )
		{
		case 0x00:
			// PinBlock Format: C L P P P P P/F P/F P/F P/F P/F P/F P/F P/F F F
			// C = 0x02, L = length of PIN, P = PIN digits, F = 0x0F
			PubBcd2Asc0(sPinBlock, 8, szBuff);
			PubTrimTailChars(szBuff, 'F');
			sprintf((char *)pszPlainPin, "%.12s", &szBuff[2]);
			glProcInfo.stTranLog.uiEntryMode |= MODE_OFF_PIN;
			return EMV_OK;

		case 0x06:
		case 0x07:
		case 0xC6:
			//By Wisdom
			offlinePinCancel = 1;
			checkofflinepin = 1;
			return EMV_USER_CANCEL;

		case 0x0A:
			if(!ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS) && ChkIfAmex())
			{
				PPScrCls();
				PPScrPrint(1,0," NOT PERMITTED");
				PPBeep();

				Gui_ClearScr();
				Beef(6, 200);
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NOT PERMITTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);

				goto ENTERPIN;
			}
			else
			{
				checkofflinepin = 1;
				return EMV_NO_PASSWORD;
			}

		default:
			checkofflinepin = 1;
			return EMV_NO_PINPAD;
		}
	} 
	else	// PED_EXT_PCI
	{
		//ShowLogs(1, "cEMVGetHolderPwd Step 7");
		// !!!! extern PCI, to be implemented.
		unsigned char szBuff[200];
		sprintf(szBuff, "%s\n%s", _T("EXT PCI PINPAD"), _T("NOT IMPLEMENTED."));
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
		checkofflinepin = 1;
		return ERR_NO_DISP;
	}
}

// ÓïÒô²Î¿¼´¦Àí
// Èç¹û²»Ö§³Ö£¬Èç¹ûÊÇ¿¨Æ¬·¢ÆðµÄ²Î¿¼£¬
// Ôò¿É¸ù¾Ý·¢¿¨ÐÐÒªÇóÖ±½Ó·µ»ØREFER_DENIAL»òREFER_ONLINE,
// Ò»°ãÇé¿öÏÂ²»Ó¦¸ÃÖ±½Ó·µ»ØREFER_APPROVE(³ý·Ç·¢¿¨ÐÐÒªÇóÕâÃ´×ö)

// Èç¹û²»Ö§³Ö£¬Èç¹ûÊÇ·¢¿¨ÐÐ(Ö÷»ú)·¢ÆðµÄ²Î¿¼£¬
// Ôò¿É¸ù¾Ý·¢¿¨ÐÐÒªÇóÖ±½Ó·µ»ØREFER_DENIAL
// Ò»°ãÇé¿öÏÂ²»Ó¦¸ÃÖ±½Ó·µ»ØREFER_APPROVE(³ý·Ç·¢¿¨ÐÐÒªÇóÕâÃ´×ö)

// ÏÂ±ßÊÇ²Î¿¼µÄ´¦Àí´úÂë£¬¹©²Î¿¼
// Callback function required by EMV core.
// Voice referral.
// If not support, return REFER_DENIAL.
int cEMVReferProc(void)
{
	return REFER_DENIAL;
}

// Callback function required by EMV core.
// TC advise after EMV transaction. If not support, immediately return.
void cEMVAdviceProc(void)
{
	//	ÍÑ»úÍ¨ÖªµÄ´¦Àí£º
	//	Í¨¹ýº¯ÊýEMVGetTLVData()»ñµÃÍ¨ÖªÊý¾Ý°üÐèÒªµÄÊý¾Ý£¬´æÖüµ½½»Ò×ÈÕÖ¾ÖÐ£¬
	//	È»ºóÔÚ½»Ò×½áËãÊ±£¬ÔÙÁª»ú·¢ËÍµ½Ö÷»ú¡£
	//	ÐèÒª×¢ÒâµÄÊÇ£ºÍ¨ÖªÊý¾Ý°üµÄÈÎºÎÊý¾Ý(±ÈÈç½ð¶îµÈ)²»¿ÉÒÔÓÃÓÚ½»Ò×¶ÔÕÊ¡£

	//	Áª»úÍ¨ÖªµÄ´¦Àí£º
	//	(1)²¦ºÅÁ¬½ÓÖ÷»ú¡£
	//	(2)Í¨¹ýº¯ÊýEMVGetTLVData()»ñµÃÍ¨ÖªÊý¾Ý°üÐèÒªµÄÊý¾Ý£¬ÔÙ·¢ËÍµ½Ö÷»ú¡£
	//	ÐèÒª×¢ÒâµÄÊÇ£ºÁª»úÍ¨Öª·½Ê½ÔÚÎÒÃÇµÄPOS»úÖÐÓ¦¸Ã²»»áÊ¹ÓÃ¡£
}

//Áª»ú´¦Àí
/*
	´¦Àí²½Öè£º
	(1)²¦ºÅÁ¬½ÓÖ÷»ú,Èç¹ûËùÓÐ½»Ò×¶¼ÒªÁª»ú£¬ÄÇÃ´¿ÉÒÔÔÚ²åÈëIC¿¨Ê±Ô¤²¦ºÅ,
	   Èç¹û²¦ºÅÊ§°Ü·µ»ØONLINE_FAILED
	(2)Í¨¹ýº¯ÊýEMVGetTLVData()»ñµÃ½»Ò×Êý¾Ý°üÐèÒªµÄÊý¾Ý£¬²¢´ò°ü¡£
	(3)±£´æ³åÕýÊý¾Ý¼°±êÖ¾,È»ºó·¢ËÍ½»Ò×Êý¾Ý°üµ½Ö÷»ú(³åÕý´¦ÀíÍêÈ«ÓÉÓ¦ÓÃÍê³É)
	(4)½ÓÊÕÖ÷»úµÄ»ØÓ¦Êý¾Ý°ü,¸ù¾ÝÖ÷»úµÄ»ØÓ¦£¬×öÈçÏÂ·µ»Ø£º
	   A.Èç¹ûÖ÷»ú·µ»ØÅú×¼£¬Ôò¸ù¾Ý·µ»ØÊý¾ÝÌîÐ´RspCode¡¢AuthCode¡¢AuthCodeLenµÈ
		 ²ÎÊýµÄÖµ£¬²¢·µ»ØONLINE_APPROVE
	   B.Èç¹ûÖ÷»ú¾Ü¾ø½»Ò×,Ôò¸ù¾Ý·µ»ØÊý¾ÝÌîÐ´RspCode,Èç¹ûÆäËû²ÎÊýÒ²ÓÐÊý¾ÝÖµ£¬
		 Í¬ÑùÐèÒªÌîÐ´£¬·µ»ØONLINE_DENIAL
	   C.Èç¹ûÖ÷»úÇëÇóÓïÒô²Î¿¼,Ôò¸ù¾Ý·µ»ØÊý¾ÝÌîÐ´RspCode,Èç¹ûÆäËû²ÎÊýÒ²ÓÐÊý¾ÝÖµ£¬
		 Í¬ÑùÐèÒªÌîÐ´£¬·µ»ØONLINE_REFER¡£ÐèÒªËµÃ÷µÄÊÇ£ººÜ¶àÇé¿ö¿ÉÄÜÃ»ÓÐ²Î¿¼´¦Àí£¬
		 ÔÚÕâÖÖÇé¿öÏÂ£¬Ó¦ÓÃ³ÌÐò¾Í²»ÐèÒª·µ»ØONLINE_REFERÁË

	µÈ½»Ò×´¦Àí³É¹¦ºó£¬Ó¦ÓÃ³ÌÐò²Å¿ÉÒÔÇå³ý³åÕý±êÖ¾¡£
*/


/* Online processing.
    steps:
	(1) Dial. If dial failed, return ONLINE_FAILED
	(2) Use EMVGetTLVData() to retrieve data from core, pack to ISO8583.
	(3) Save reversal data and flag, then send request to host
	(4) Receive from host, then do accordingly:
	   A. If host approved, copy RspCode,AuthCode,AuthCodeLen or so, and return ONLINE_APPROVE
	   B. If host denial, copy RspCode or so, return ONLINE_DENIAL
	   C. If host require voice referral, copy RspCode or so.,return ONLINE_REFER.
	       Note that if not support, needn't return ONLINE_REFER but directly ONLINE_DENIAL

	Reversal flag can only be cleared after all EMV processing, NOT immediately after online.
	Based on pax, decide and know what to do afterwards
*/
int  cEMVOnlineProc(uchar *psRspCode,  uchar *psAuthCode, int *piAuthCodeLen,
					uchar *psAuthData, int *piAuthDataLen,
					uchar *psScript,   int *piScriptLen)
{
	int		iRet, iLength, iRetryPIN, iRev = 0;
	ulong	ulICCDataLen;
	uchar	*psICCData, *psTemp;
	char iccData[500] = {0};
	uchar	sTemp[50], sStore[100];
	char lasthost[128] = {0};
	char verve[128] = {0};
	// initialize output parameters
	*psRspCode      = 0;
	*piAuthCodeLen  = 0;
	*piAuthDataLen  = 0;
	*piScriptLen    = 0;
	
	//Delete
	DisplayInfoNone("", "PROCESSING", 0);
	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		//ShowLogs(1, "E. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "Done getting Cryptogram");
	
	SaveTVRTSI(TRUE);//Commented out by Wisdom - app exception
	//ShowLogs(1, "After SaveTVRTSI");
	//Delete
	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		//ShowLogs(1, "A. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "Done getting Cryptogram");
	glProcInfo.bIsFirstGAC = FALSE;
	//Delete
	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		ShowLogs(1, "B. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "Done getting Cryptogram");
	UpdateEntryModeForOfflinePIN();//Wisdom
	
	
	//ICC is packed here
	//Open up
	// prepare online DE55 data
	//ShowLogs(1, "Trying to see if this is the second time");
	iRet = SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
	if( iRet!=0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}
	
	ShowLogs(1, "1. ICC DATA: %s", glSendPack.sICCData);
	ShowLogs(1, "Done Packing Using SetDE55: %s", glSendPack.testSICCData);

	//PubASSERT( iLength<LEN_ICC_DATA ); //by wisdom
	PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
	memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);	// save for batch upload
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
	PubBcd2Asc(glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen, iccData);


	ShowLogs(1, "After Unpacking IccData");
	ShowLogs(1, "3. ICC DATA: %s", iccData);
	//ShowLogs(1, "cEMVOnlineProc Step 1. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "cEMVOnlineProc Step 1. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "cEMVOnlineProc Step 1. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	
	//Delete
	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		//ShowLogs(1, "0. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "Done getting Cryptogram");

	//CreateWrite("logreads.txt", iccData);//Delete

	if( !ChkIfAmex() )
	{
		//For Amex
		if( !(ChkIfDah() || ChkIfCiti() || ChkIfScb()) && ChkIfAcqNeedDE56() )
		{
			if ( glProcInfo.stTranLog.ucTranType!=AUTH && glProcInfo.stTranLog.ucTranType!=PREAUTH )
			{
				iLength = glSysCtrl.stField56[glCurAcq.ucIndex].uiLength;
				//PubASSERT(iLength<LEN_ICC_DATA);
				if( iLength>0 )
				{
					memcpy(&glSendPack.sICCData2[2], glSysCtrl.stField56[glCurAcq.ucIndex].sData, iLength);
				}
				else
				{
					SetStdEmptyDE56(&glSendPack.sICCData2[2], &iLength);
				}
				PubLong2Char((ulong)iLength, 2, glSendPack.sICCData2);
			}
		}
	}

	// ³åÕý½»Ò×´¦Àí & ÀëÏß½»Ò×ÉÏËÍ
	// ÅÐ¶ÏÉÏ´Î½»Ò×ÊÇ·ñÐèÒª½øÐÐ³åÕýµÈ
	// send reversal here. If required by bank, also send offline here
	/*
	iRet = TranReversal();
	if( iRet!=0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}
	*/
	//ShowLogs(1, "cEMVOnlineProc Step 2. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "cEMVOnlineProc Step 2. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "cEMVOnlineProc Step 2. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	iRetryPIN = 0;
	while( 1 )
	{
		if (ChkIfAmex() || ChkCurAcqName("AMEX", FALSE))
		{
			GetNewInvoiceNo();
		}

		//Delete
		memset(sTemp, 0, sizeof(sTemp));
		memset(sStore, 0, sizeof(sStore));
		iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
		if( iRet==EMV_OK )
		{
			PubBcd2Asc(sTemp, iLength, sStore);
			//ShowLogs(1, "0a. Tag 9F26: %s", sStore);	
		}
		//ShowLogs(1, "Done getting Cryptogram");

		//ShowLogs(1, "Pan: %s", glSendPack.szPan);

		if(glSendPack.szPan[0] == '4')
		{
			//ShowLogs(1, "It is a Visa Card.");
			return ONLINE_DENIAL;
		}else
		{
			if(checkofflinepin == 1)
			{
				//Reset here start
				memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
				memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
				memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
				memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
				memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
				memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
				memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));
				//Reset here stop
				checkofflinepin = 0;
				return ONLINE_DENIAL;
			}
			//ShowLogs(1, "It is not a Visa Card.");
		}
		

		//Check for completion here

		//Send Iso Message Based on TxnType Here. See Tranproc.c Line 89
		/*memset(lasthost, '\0', strlen(lasthost));
		UtilGetEnvEx("lhost", lasthost);
		//ShowLogs(1, "1. Lasthost: %s", lasthost);
		if(strstr(lasthost, "host2") != NULL)
		{
			iRet = SendEmvDataB(iccData, &iRev);
		}else
		{
			iRet = SendEmvData(iccData, &iRev);
		}*/
		//ShowLogs(1, "1a. Pan: %s", glSendPack.szPan);
		//ShowLogs(1, "2b. Host: %s", profileTag.vervehost);
			
		memset(lasthost, '\0', strlen(lasthost));
		iRet = SendEmvData(iccData, &iRev);




		if(iRet != 0)
		{
			//ShowLogs(1, "Failed. SendEmvData Returned: %d", iRet);
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			//return ONLINE_FAILED;//Commented out by Wisdom
		}else
		{
			//ShowLogs(1, "Response came from host");
			break;
		} 
		
		if(iRev == 1)
		{
			//ShowLogs(1, "Sending Reversal. Value: %d", iRev);
			DisplayInfoNone("", "NO RESPONSE", 3);
			iRet = SendReversal();
			if( iRet!=0 )
			{
				DisplayInfoNone("", "FAILED", 2);
				glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
				return ONLINE_FAILED;
			}else
			{
				return 0;
			}
			
		}

		if( memcmp(glRecvPack.szRspCode, "55", 2)!=0 || ++iRetryPIN>3 || !ChkIfNeedPIN() )
		{
			//ShowLogs(1, "Not pin error");
			break;
		}

		// retry EMV online PIN
		iRet = GetPIN(GETPIN_RETRY);
		if( iRet!=0 )
		{
			return ONLINE_DENIAL;
		}
		sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);
		memcpy(&glSendPack.sPINData[0], "\x00\x08", 2);
		memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, 8);
	}

	//ShowLogs(1, "cEMVOnlineProc Step 3. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "cEMVOnlineProc Step 3. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "cEMVOnlineProc Step 3. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	// set response code
	memcpy(psRspCode,  glRecvPack.szRspCode,  LEN_RSP_CODE);
	glProcInfo.ucOnlineStatus = ST_ONLINE_APPV;

	// get response issuer data
	sgAuthDataLen = sgScriptLen = 0;

	//ShowLogs(1, "IccData Gotten: %s", glRecvPack.sICCData);

	//Check by Wisdom to stop emv crashing
	//ShowLogs(1, "Normal Icc Gotten");
	ulICCDataLen = PubChar2Long(glRecvPack.sICCData, 2);
	psICCData = &glRecvPack.sICCData[2];

	//ShowLogs(1, "IccData to be parsed: %s", psICCData);
	if (ChkIfAmex())
	{
		//ShowLogs(1, "Response is Amex");
		iRet = GetDE55Amex(glSendPack.sICCData+2, psICCData, ulICCDataLen);
		if( iRet<0 )
		{
			// if analyze response ICC data failed, return fail
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ONLINE_FAILED;
		}
	}
	else
	{
		for(psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
		{
			// version 1.00.0016 change by Jolie Yang at 2013-08-16
			// due to the application just only get contents of 71\72, and transfer it to EMV kernal.
			iRet = GetTLVItem(&psTemp, psICCData+ulICCDataLen-psTemp, SaveRspICCData, FALSE);
			//iRet = GetTLVItem(&psTemp, psICCData+ulICCDataLen-psTemp, SaveRspICCData, TRUE);
			//if( iRet<0 )
			//{	// if analyze response ICC data failed, return fail
			//	glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			//	return ONLINE_FAILED;
			//}

			//By wisdom to check
			if(iRet != 0)
				break;
		}
	}

	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		//ShowLogs(1, "Q. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "Done getting Cryptogram");



	//ShowLogs(1, "cEMVOnlineProc Step 4. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "cEMVOnlineProc Step 4. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "cEMVOnlineProc Step 4. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	//ShowLogs(1, "Response Not Amex: Step 3");
	memcpy(psAuthData, sAuthData, sgAuthDataLen);
	*piAuthDataLen = sgAuthDataLen;
	//ShowLogs(1, "Response Not Amex: Step 4");
	
	// version 1.00.0016 change by Jolie Yang at 2013-08-16
	// due to the application need not extract the sub-tag of 71/72, just get contents of 71\72, and transfer to EMV kernal
	// AdjustIssuerScript();
	memcpy(psScript, sIssuerScript, sgScriptLen);
	*piScriptLen = sgScriptLen;
	//ShowLogs(1, "Response Not Amex: Step 5");
	if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE)!=0 )
	{
		
	}
	//ShowLogs(1, "Response Not Amex: Step 6");
	// set authorize code only if txn approved
	memcpy(psAuthCode, glRecvPack.szAuthCode, LEN_AUTH_CODE);
	*piAuthCodeLen = strlen((char *)glRecvPack.szAuthCode);

	//DisplayInfoNone("RESPONSE", "APPROVED", 1);
	//ShowLogs(1, "Returning ONLINE_APPROVE");



	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		//ShowLogs(1, "W. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "Done getting Cryptogram");
	return ONLINE_APPROVE;
}

// Èç¹û²»ÐèÒªÌáÊ¾ÃÜÂëÑéÖ¤³É¹¦£¬ÔòÖ±½Ó·µ»Ø¾Í¿ÉÒÔÁË
// Callback function required by EMV core.
// Display "EMV PIN OK" info. (plaintext/enciphered PIN)
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
void cEMVVerifyPINOK(void)
{
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(),  gl_stTitleAttr, _T("PIN OK"), gl_stCenterAttr, GUI_BUTTON_OK, 0, NULL);
	//Gui_ShowMsgBox(GetCurrTitle(),  gl_stTitleAttr, _T("PIN OK"), gl_stCenterAttr, GUI_BUTTON_OK, 1, NULL);
}

// ³Ö¿¨ÈËÈÏÖ¤Àý³Ì
// Callback function required by EMV core.
// Don't need to care about this function
int cCertVerify(void)
{
	//	AppSetMckParam(!ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS));
	return -1;
}

// Callback function required by EMV core.
// in EMV ver 2.1+, this function is called before GPO
int cEMVSetParam(void)
{
	return 0;
}

int FinishEmvTran(void)
{
	int		iRet, iLength, iRev = 0;;
	uchar	ucSW1, ucSW2;
	uchar	sTemp[50], sStore[100];
	char iccData[500] = {0};
	char lasthost[128] = {0};
	char verve[128] = {0};
	char user[20] = {0};

	// ¸ù¾ÝÐèÒªÉèÖÃÊÇ·ñÇ¿ÖÆÁª»ú
	// decide whether need force online
	EMVGetParameter(&glEmvParam);
	//ShowLogs(1, "EMVGetParameter Done");
	//glEmvParam.ForceOnline = ((glProcInfo.stTranLog.ucTranType!=SALE )? 1 : 0); //Check this for MTIP
	glEmvParam.ForceOnline = 1;//By Wisdom
	//ShowLogs(1, "Check if Transaction is to be forced online. Use for Mtip and ADVT");
	EMVSetParameter(&glEmvParam);
	//ShowLogs(1, "EMVSetParameter Done");
	// Only in this transaction, so DON'T back up

	//ShowLogs(1, "FinishEmvTran Step 1. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 1. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 1. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);

	// clear last EMV status
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
	//ShowLogs(1, "Saved EMV Status");
	if (ChkTermPEDMode(PED_INT_PCI))
	{
		//ShowLogs(1, "Inside ChkTermPedMode");
		iRet = EMVSetPCIModeParam(1, (uchar *)"0,4,5,6,7,8", 120000);
		//ShowLogs(1, "EMVSetPCIModeParam return %d", iRet);
	}

	//ShowLogs(1, "About Calling EMVProcTrans");
	iRet = EMVProcTrans();
	//ShowLogs(1, "Home EMVProcTrans return %d", iRet);
	if(iRet == -7 || iRet != 0)
	{
		//return ERR_USERCANCEL;
	}

	if(iRet == -2 || iRet == -7)
	{
		checkofflinepin = 1;
		return ERR_USERCANCEL;
	}

	

	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		//ShowLogs(1, "1. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "Done getting Cryptogram");


	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(0x9F26, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		//ShowLogs(1, "2. Tag 9F26: %s", sStore);	
	}
	//ShowLogs(1, "1. Done getting Cryptogram");


	//Delete here
	//By Wisdom
	//ShowLogs(1, "Value of offlinePinCancel: %d", offlinePinCancel);
	//ShowLogs(1, "Trying to see if this is the second time");
	iRet = SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
	if( iRet != 0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}
	
	//ShowLogs(1, "1. ICC DATA: %s", glSendPack.sICCData);

	//ShowLogs(1, "Done Packing Using SetDE55");

	//PubASSERT( iLength<LEN_ICC_DATA ); //by wisdom
	PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
	memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);	// save for batch upload
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
	PubBcd2Asc(glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen, iccData);

	//ShowLogs(1, "1. Pan: %s", glSendPack.szPan);
	//ShowLogs(1, "2. Host: %s", profileTag.vervehost);
	if(checkofflinepin == 1)
	{
		//Reset here start
		memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
		memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
		memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
		memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
		memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
		memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
		memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
		memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
		memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
		memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));
		//Reset here stop
		checkofflinepin = 0;
		return ERR_USERCANCEL;
	}

	if(iRet == EMV_USER_CANCEL || iRet == EMV_NO_APP || iRet == EMV_DATA_ERR 
		|| iRet == EMV_NO_PASSWORD)
	{
		checkofflinepin = 1;
		return ERR_USERCANCEL;
	}

	if(glSendPack.szPan[0] == '4')
	{
		//ShowLogs(1, "2. It is a Visa Card.");
		memset(lasthost, '\0', strlen(lasthost));
		UtilGetEnvEx("lhost", lasthost);
		//ShowLogs(1, "1. Lasthost: %s", lasthost);
		iRet = SendEmvData(iccData, &iRev);
		


		if(iRet != 0)
		{
			//ShowLogs(1, "Failed. SendEmvData Returned: %d", iRet);
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			//return ONLINE_FAILED;//Commented out by Wisdom
		}else
		{
			//ShowLogs(1, "Response came from host");
		}


		


		if(iRev == 1)
		{
			//ShowLogs(1, "Sending Reversal. Value: %d", iRev);
			DisplayInfoNone("", "NO RESPONSE", 3);
			iRet = SendReversal();
			if( iRet!=0 )
			{
				DisplayInfoNone("", "FAILED", 2);
				glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
				return ONLINE_FAILED;
			}else
			{
				return 0;
			}
			
		}
	}else
	{
		//ShowLogs(1, "2. It is not a Visa Card.");
	}

	//Delete Stop

	//ShowLogs(1, "FinishEmvTran Step 3. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 3. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 3. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	//Problem might not be from here down
	SaveTVRTSI(FALSE);//By Wisdom
	//ShowLogs(1, "FinishEmvTran Step 4. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 4. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 4. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	//ShowLogs(1, "Done with Saving Tags");
	UpdateEntryModeForOfflinePIN();
	//ShowLogs(1, "FinishEmvTran Step 5. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 5. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 5. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	//ShowLogs(1, "Done with UpdateEntryModeForOfflinePIN");
	if( iRet==EMV_TIME_OUT || iRet==EMV_USER_CANCEL )
	{
		//ShowLogs(1, "EMVTIMEOUT or EMVUSERCANCEL");
		return ERR_USERCANCEL;
	}
	if( (glProcInfo.ucOnlineStatus==ST_ONLINE_APPV) && memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)==0 )
	{
		//ShowLogs(1, "Inside ONLINE APPROVE AND RESPONSE CODE");
		if( (glProcInfo.stTranLog.ucTranType!=AUTH) &&
			(glProcInfo.stTranLog.ucTranType!=PREAUTH) &&
			ChkIfAcqNeedDE56())
		{
			//ShowLogs(1, "Inside ONLINE APPROVE AND RESPONSE CODE 1");
			SetDE56(glSysCtrl.stField56[glCurAcq.ucIndex].sData, &iLength);
			glSysCtrl.stField56[glCurAcq.ucIndex].uiLength = (ushort)iLength;
			SaveField56();

			// if online approved, save bit 56 for void/upload etc
			memcpy(glProcInfo.stTranLog.sField56, glSysCtrl.stField56[glCurAcq.ucIndex].sData, iLength);
			glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
		}
		//ShowLogs(1, "update for reversal(maybe have script result)");
		// update for reversal(maybe have script result)
		SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
		//ShowLogs(1, "Done setting tag for sale");
		PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
		SaveRevInfo(TRUE);	// update reversal information
		//ShowLogs(1, "Saved for reversal");
	}
	//ShowLogs(1, "FinishEmvTran Step 6. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 6. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 6. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	if( iRet!=EMV_OK )
	{
		//ShowLogs(1, "EMV IS NOT OKAY");
		//SaveEmvErrLog();//Commented out by Wisdom
		//ShowLogs(1, "Done saving error log");
		EMVGetICCStatus(&ucSW1, &ucSW2);
		//ShowLogs(1, "Done with EMVGetICCStatus");
		if( glProcInfo.bIsFirstGAC && ucSW1==0x69 && ucSW2==0x85 &&
			glProcInfo.stTranLog.szPan[0]=='5' )
		{	// for TIP fallback when 1st GAC return 6985
			//ShowLogs(1, "Need Fallback");
			return ERR_NEED_FALLBACK;
		}

		// for sale completion only send 5A not 57 [1/11/2007 Tommy]
		if( !ChkIfAmex() && (glProcInfo.ucOnlineStatus!=ST_OFFLINE) &&
			(memcmp(glProcInfo.stTranLog.szRspCode, "01", 2)==0 ||
			 memcmp(glProcInfo.stTranLog.szRspCode, "02", 2)==0)
			)
		{
			//ShowLogs(1, "For Upload");
			SetDE55(DE55_UPLOAD, glProcInfo.stTranLog.sIccData, &iLength);
			//ShowLogs(1, "Done Setting Log");
			glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
		}

		if( glProcInfo.stTranLog.szRspCode[0]!=0 &&
			memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
		{	// show reject code from host
			//ShowLogs(1, "Rejection from Host");
			return AfterTranProc();
		}
		//ShowLogs(1, "Err Transaction Failed");
		return ERR_TRAN_FAIL;
	}
	//ShowLogs(1, "FinishEmvTran Step 7. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 7. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 7. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	//ShowLogs(1, "Transaction was approved");
	// transaction approved. save EMV data
	//SaveEmvData();//By Wisdom
	//ShowLogs(1, "FinishEmvTran Step 8. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 8. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 8. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	//ShowLogs(1, "EMV data save for upload");
	if( glProcInfo.ucOnlineStatus!=ST_ONLINE_APPV )
	{
		//ShowLogs(1, "Rushing to FinishOffLine");
		return FinishOffLine();
	}
	//ShowLogs(1, "FinishEmvTran Step 9. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 9. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 9. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	if (ChkIfAcqNeedTC())
	{
		//ShowLogs(1, "Inside ChkIfAcqNeedTC");
		glProcInfo.stTranLog.uiStatus |= TS_NEED_TC;
	}
	//ShowLogs(1, "FinishEmvTran Step 10. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	//ShowLogs(1, "FinishEmvTran Step 10. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	//ShowLogs(1, "FinishEmvTran Step 10. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	//ShowLogs(1, "Before Calling AfterTrans");
	return AfterTranProc();
}

// Set bit 55 data for online transaction.
int SetDE55(DE55_TYPE ucType, uchar *psOutData, int *piOutLen )
{
	//ShowLogs(1, "Inside SetDE55");
	if( ChkIfAmex() )
	{
		SetAmexDE55(sgAmexTagList, psOutData, piOutLen);
		//ShowLogs(1, "Gotten Icc: %s", glSendPack.testSICCData);
		PubAsc2Bcd(glSendPack.testSICCData, strlen(glSendPack.testSICCData), psOutData);
		//piOutLen = strlen(psOutData);
		return 0;
	}
	else
	{
		SetStdDE55((uchar)ucType, sgStdEmvTagList, psOutData, piOutLen);
		//ShowLogs(1, "Gotten Icc: %s", glSendPack.testSICCData);
		PubAsc2Bcd(glSendPack.testSICCData, strlen(glSendPack.testSICCData), psOutData);
		//piOutLen = strlen(psOutData);
		return 0;
	}
}

// set AMEX bit 55, structure of TLV items
int SetAmexDE55(const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[128];

	*piOutLen = 0;
	memcpy(psOutData, "\xC1\xC7\xD5\xE2\x00\x01", 6);	// AMEX header
	psTemp = psOutData+6;

	for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		iLength = 0;
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
		if( (iRet!=EMV_OK ) && (iRet!=EMV_NO_DATA))
		{
			return ERR_TRAN_FAIL;
		}

		if(iRet==EMV_NO_DATA)
		{
			iLength = pstList[iCnt].ucLen;
			memset(sBuff, 0, sizeof(sBuff));
		}

		if( pstList[iCnt].ucOption==DE55_LEN_VAR1 )
		{
			*psTemp++ = (uchar)iLength;
		}
		else if( pstList[iCnt].ucOption==DE55_LEN_VAR2 )
		{
			*psTemp++ = (uchar)(iLength>>8);
			*psTemp++ = (uchar)iLength;
		}
		memcpy(psTemp, sBuff, iLength);
		psTemp += iLength;
	}
	*piOutLen = (psTemp-psOutData);

	return 0;
}

// this function will not check the overflow risk of array pointed by pstList.
int AppendStdTagList(DE55Tag *pstList, ushort uiTag, uchar ucOption, uchar ucMaxLen)
{
	int	iCnt;

	iCnt = 0;
	while (pstList[iCnt].uiEmvTag!=0)
	{
		iCnt++;
	}
	pstList[iCnt].uiEmvTag = uiTag;
	pstList[iCnt].ucOption = ucOption;
	pstList[iCnt].ucLen    = ucMaxLen;
	pstList[iCnt+1].uiEmvTag = 0;
	pstList[iCnt+1].ucOption = 0;
	pstList[iCnt+1].ucLen    = 0;
	return 0;
}

int RemoveFromTagList(DE55Tag *pstList, ushort uiTag)
{
	int	iCnt;

	for (iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		if (pstList[iCnt].uiEmvTag==uiTag)
		{
			break;
		}
	}
	if (pstList[iCnt].uiEmvTag==0)
	{
		return -1;
	}

	for (; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		pstList[iCnt] = pstList[iCnt+1];
	}

	return 0;
}

// set ADVT/TIP bit 55
int SetStdDE55(uchar bForUpLoad, const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[200];
	//DE55Tag	astLocalTaglist[64];//Before now
	DE55Tag	astLocalTaglist[20];
	char temp[258] = {0};
	char save[258] = {0};
	char init[10] = {0}, asc[10] = {0}, bcd[10] = {0};
	*piOutLen = 0;
	psTemp    = psOutData;

	//ShowLogs(1, "Whoops: I am inside SetStdDE55");
	

	// ???? MODE_FALLBACK_MANUAL ????
	if ( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE ) ||
		 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		//ShowLogs(1, "1. Copying some tags");
		if( ChkIfBoc() || ChkIfFubon() || ChkIfBea() )
		{
			memcpy(psTemp, "\xDF\xEC\x01\x01", 4);
			psTemp += 4;
			memcpy(psTemp, "\xDF\xED\x01", 3);
			psTemp += 3;

			if( ChkIfBea() )
			{
				*psTemp++ = LastRecordIsFallback() ? 0x02 : 0x01;
			}
			else
			{
				if( glProcInfo.iFallbackErrCode==EMV_NO_APP ||
					glProcInfo.iFallbackErrCode==ICC_RESET_ERR )
				{
					*psTemp++ = 0x02;
				}
				else
				{
					*psTemp++ = 0x01;
				}
			}
		}
		else
		{
			memcpy(psTemp, "\xDF\x5A\x01\x01", 4);
			psTemp += 4;
			memcpy(psTemp, "\xDF\x39\x01\x01", 4);
			psTemp += 4;
		}
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		//ShowLogs(1, "2a. Copying some tags");
		// Copy from std tag list
		//-----------------------------------------------------------
		memset(astLocalTaglist, 0, sizeof(astLocalTaglist));
		for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
		{
			////ShowLogs(1, "Copying some tags: %d", iCnt);
			astLocalTaglist[iCnt] = pstList[iCnt];
		}

		//Setting Country and Currency Code Based on Profile
		/*memset(init, '\0', strlen(init));
		memset(asc, '\0', strlen(asc));
		memset(bcd, '\0', strlen(bcd));
		UtilGetEnvEx("txnConCode", asc);
		//ShowLogs(1, "EMV Con: %s", asc);
		strcpy(init, "0");
		strcat(init, asc);
		//ShowLogs(1, "Code: %s", init);
		PubAsc2Bcd(init, 4, bcd);
		//EMVSetTLVData(0x9F1A, (uchar *)bcd, 2);
		//EMVSetTLVData(0x5F2A, (uchar *)bcd, 2);
		//Defined by Wisdom
		//EMVSetTLVData(0x9F33, (uchar *)"\xE0\xF8\xC8", 3);//\xE0\xE1\xC8//\xE0\xF8\xC8
		//EMVSetTLVData(0x9F35, (uchar *)"\x34", 1);//22
		//EMVSetTLVData(0x95, (uchar *)"\x00\x80\x00\x08\x00", 5);*/
		
		//EMVSetTLVData(0x9F1A, (uchar *)"\x05\x66", 2);
		//EMVSetTLVData(0x5F2A, (uchar *)"\x05\x66", 2);
		//EMVSetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6);
		//ShowLogs(1, "1. Code......");

		/*memset(init, '\0', strlen(init));
		memset(asc, '\0', strlen(asc));
		memset(bcd, '\0', strlen(bcd));
		//ShowLogs(1, "Gotten: %s", asc);
		strcpy(init, "0");
		strcat(init, asc);
		PubAsc2Bcd(init, 4, bcd);*/
		
		//-----------------------------------------------------------
		// Customize tag list according to different acquirer
		//RemoveFromTagList(astLocalTaglist, 0x5F34);
		//AppendStdTagList(astLocalTaglist, 0x5A, DE55_MUST_SET, 0);

		//ShowLogs(1, "3. Copying some tags");
		//-----------------------------------------------------------
		// Generate data by tag list
		for(iCnt=0; pstList[iCnt].uiEmvTag != 0; iCnt++)
		{
			memset(sBuff, 0, sizeof(sBuff));
			iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
			if( iRet==EMV_OK )
			{
				//ShowLogs(1, "Generating Data Tags: %d -- Length = %d", iCnt, iLength);
				if ((pstList[iCnt].ucLen > 0) && (iLength > pstList[iCnt].ucLen))
				{
					//ShowLogs(1, "Length has changed for %d", iCnt);
					iLength = pstList[iCnt].ucLen;
				}

				if(iCnt == 0)
				{
					memset(temp, '\0', strlen(temp));
					memset(save, '\0', strlen(save));
					PubBcd2Asc(sBuff, iLength, temp);
					sprintf(save, "%02X%02X%s", pstList[iCnt].uiEmvTag, iLength, temp);
					//ShowLogs(1, "Save: %s", save);
					strcpy(glSendPack.testSICCData, save);
				}else
				{
					memset(temp, '\0', strlen(temp));
					memset(save, '\0', strlen(save));
					PubBcd2Asc(sBuff, iLength, temp);
					sprintf(save, "%02X%02X%s", pstList[iCnt].uiEmvTag, iLength, temp);
					//ShowLogs(1, "Save: %s", save);
					strcat(glSendPack.testSICCData, save);
				}
				

				//memset(temp, '\0', strlen(temp));
				//PubBcd2Asc(sBuff, iLength, temp);
				////ShowLogs(1, "Outside Tag: %s", temp);

				//Trace("iso", "emv tag = [%02x%02x] len = [%d] ism = [%d] \r\n", st_doltag.heTag[0], st_doltag.heTag[1], st_doltag.ucDatalen, st_doltag.bIsMandatory);
				//if(iLength > 0)
				//BuildTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
			}
			/*else if( pstList[iCnt].ucOption==DE55_MUST_SET )
			{
				//ShowLogs(1, "Generating Data Tags 2: %d", iCnt);
				BuildTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);
				//ShowLogs(1, "End Step 2");
				//				return ERR_TRAN_FAIL;	// Èç¹û±ØÐë´æÔÚµÄTAG²»´æÔÚ,½»Ò×Ê§°Ü
			}*/
		}

		//ShowLogs(1, "4. Copying some tags");
		//Set Nibss Specific Icc Tag

		memset(temp, '\0', strlen(temp));
		PubBcd2Asc(psTemp, (psTemp-psOutData), temp);
		//ShowLogs(1, "Icc Tag: %s", temp);

		//ShowLogs(1, "2ac. Copying some tags. Done");
		//return 0;


		//-----------------------------------------------------------
		// Generate custom tag content
		if( glProcInfo.stTranLog.szPan[0]=='5' )
		{	// for master card TCC = "R" -- retail
			//By Wisdom
			//BuildTLVString(0x9F53, (uchar *)"R", 1, &psTemp);
		}
		//ShowLogs(1, "5. Copying some tags");
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetScriptResult(sBuff, &iLength);
		if( iRet==EMV_OK )
		{
			BuildTLVString(0xDF5B, sBuff, iLength, &psTemp);
		}
		//ShowLogs(1, "6. Copying some tags");
	}
	else
	{
		//ShowLogs(1, "7. Copying some tags");
		return 0;
	}

	if( ChkIfBoc() )
	{
		//ShowLogs(1, "8. Copying some tags");
		memcpy(psTemp, "\xDF\xEE\x01\x05", 4);
		psTemp += 4;
	}
	//ShowLogs(1, "9. Copying some tags");
	*piOutLen = (psTemp-psOutData);


	//Testing //Delete
	////ShowLogs(1, "About logging Icc");
	////ShowLogs(1, "1. Done logging Icc: %d", strlen(psOutData));
	//PubBcd2Asc(psOutData, 150, temp);
	////ShowLogs(1, "%s", temp);
	////ShowLogs(1, "The End");
	////ShowLogs(1, "10. Copying some tags - 1");
	////ShowLogs(1, "ICC: %s", psOutData);
	return 0;
}

int SetTCDE55(void *pstTranLog, uchar *psOutData, int *piOutLen)
{
    char    sBuff[LEN_ICC_DATA];
    ushort  uiLen;
    int     iRet;

    if (ChkIfICBC_MACAU())
    {
        // ICBC-Macau only need 9F26 in TC DE55
        *piOutLen = 0;
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F26, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        return 0;
    }
    else if (ChkIfDah() || ChkIfWingHang())
    {
        *piOutLen = 0;
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F26, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F27, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        return 0;
    }

    *piOutLen = ((TRAN_LOG *)pstTranLog)->uiIccDataLen;
    memcpy(psOutData, ((TRAN_LOG *)pstTranLog)->sIccData, *piOutLen);
    return 0;
}

//Set 56 field
int SetDE56(uchar *psOutData, int *piOutLen)
{
	*piOutLen = 0;
	if( ChkIfAmex() )
	{
		return 0;
	}

	return SetStdDE56(sgStdEmvField56TagList, psOutData, piOutLen);
}

int SetStdEmptyDE56(uchar *psOutData, int *piOutLen)
{
	if( ChkIfAmex() )
	{
		*piOutLen = 0;
		return 0;
	}

	if( ChkIfBea() )
	{
		memcpy(psOutData, "\xDF\xF0\x0D\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20", 16);
		*piOutLen = 16;
	}
	else if( ChkIfBoc() || ChkIfFubon() )
	{
		memcpy(psOutData, "\xDF\xF0\x07\x00\x20\x20\x20\x20\x20\x20", 10);
		*piOutLen = 10;
	}
	else
	{
		memcpy(psOutData, "\xDF\x5C\x07\x00\x20\x20\x20\x20\x20\x20", 10);
		*piOutLen = 10;
	}

	return 0;
}

int SetStdDE56(const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[110];

	//ShowLogs(1, "Whoops: I am inside SetStdDE56");

	// build header of bit 56
	*piOutLen = 0;
	psTemp    = psOutData;
	if( ChkIfBea() )
	{
		memcpy(psTemp, "\xDF\xF0\x0D\x01", 4);
		psTemp += 4;
		PubLong2Bcd(glProcInfo.stTranLog.ulInvoiceNo, 3, psTemp);
		psTemp += 3;
		PubLong2Bcd(glProcInfo.stTranLog.ulSTAN, 3, psTemp);
		psTemp += 3;
		PubAsc2Bcd(glProcInfo.stTranLog.szRRN, 12, psTemp);
		psTemp += 6;
	}
	else
	{
		if( ChkIfBoc() || ChkIfFubon() )
		{
			memcpy(psTemp, "\xDF\xF0\x07\x01", 4);
		}
		else
		{
			memcpy(psTemp, "\xDF\x5C\x07\x01", 4);
		}
		psTemp += 4;

		PubLong2Bcd(glProcInfo.stTranLog.ulInvoiceNo, 3, psTemp);
		psTemp += 3;
		PubLong2Bcd(glProcInfo.stTranLog.ulSTAN, 3, psTemp);
		psTemp += 3;
	}

	// build common EMV core tags for all HK banks
	for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
		if( iRet==EMV_OK )
		{
			BuildTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
		}
		else
		{
			BuildTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);
		}
	}

	// process special EMC core tags for different banks
	if( ChkIfFubon() )
	{
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9A, sBuff, &iLength);
		BuildTLVString(0x9A, sBuff, iLength, &psTemp);
	}
	if( ChkIfHSBC() )
	{
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9F36, sBuff, &iLength);
		BuildTLVString(0x9F36, sBuff, iLength, &psTemp);
	}

	memset(sBuff, 0, sizeof(sBuff));
	iRet = EMVGetScriptResult(sBuff, &iLength);
	if( iRet!=EMV_OK )
	{
		*piOutLen = (psTemp-psOutData);
		return 0;
	}

	// continue issuer script result process
	if( ChkIfBoc() || ChkIfFubon() || ChkIfBea() )
	{
		memcpy(psTemp, "\xDF\x91", 2);
	}
	else if( ChkIfDah() || ChkIfScb() || ChkIfCiti() )
	{
		memcpy(psTemp, "\x9F\x5B", 2);
	}
	else
	{
		memcpy(psTemp, "\xDF\x5B", 2);
	}
	psTemp   += 2;
	*psTemp++ = (uchar)iLength;
	memcpy(psTemp, sBuff, iLength);
	psTemp += iLength;

	*piOutLen = (psTemp-psOutData);

	return 0;
}

// bExpandAll:       TRUE: expand constructed item, FALSE: not
int GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll)
{
	int			iRet;
	uchar		*psTag, *psSubTag;
	uint		uiTag, uiLenBytes;
	ulong		lTemp;

	// skip null tags
	for(psTag=*ppsTLVString; psTag<*ppsTLVString+iMaxLen; psTag++)
	{
		if( (*psTag!=TAG_NULL_1) && (*psTag!=TAG_NULL_2) )
		{
			break;
		}
	}
	if( psTag>=*ppsTLVString+iMaxLen )
	{
		*ppsTLVString = psTag;
		return 0;	// no tag available
	}

	// process tag bytes
	uiTag = *psTag++;
	if( (uiTag & TAGMASK_FIRSTBYTE)==TAGMASK_FIRSTBYTE )
	{	// have another byte
		uiTag = (uiTag<<8) + *psTag++;
	}
	if( psTag>=*ppsTLVString+iMaxLen )
	{
		return -1;
	}

	// process length bytes
	if( (*psTag & LENMASK_NEXTBYTE)==LENMASK_NEXTBYTE )
	{
		uiLenBytes = *psTag & LENMASK_LENBYTES;
		lTemp = PubChar2Long(psTag+1, uiLenBytes);
	}
	else
	{
		uiLenBytes = 0;
		lTemp      = *psTag & LENMASK_LENBYTES;
	}
	psTag += uiLenBytes+1;
	if( psTag+lTemp>*ppsTLVString+iMaxLen )
	{
		return -2;
	}
	*ppsTLVString = psTag+lTemp;	// advance pointer of TLV string

	// save data
	(*pfSaveData)(uiTag, psTag, (int)lTemp);
	if( !IsConstructedTag(uiTag) || !bExpandAll )
	{
		return 0;
	}

	// constructed data
	for(psSubTag=psTag; psSubTag<psTag+lTemp; )
	{
		iRet = GetTLVItem(&psSubTag, psTag+lTemp-psSubTag, pfSaveData, TRUE);
		if( iRet<0 )
		{
			return iRet;
		}
	}

	return 0;
}

int GetSpecTLVItem(const uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen)
{
	uchar		*psTag, *psTagStr;
	uint		uiTag, uiLenBytes;
	ulong		lTemp;

	// skip null tags
    for (psTag=(uchar *)psTLVString; psTag<psTLVString+iMaxLen; psTag++)
    {
        if ((*psTag!=TAG_NULL_1) && (*psTag!=TAG_NULL_2))
        {
            break;
        }
    }
    if ( psTag>=psTLVString+iMaxLen )
    {
        return -1;	// no tag available
    }
    
    while (1)
    {
        psTagStr = psTag;
        // process tag bytes
        uiTag = *psTag++;
        if ((uiTag & TAGMASK_FIRSTBYTE)==TAGMASK_FIRSTBYTE)
        {	// have another byte
            uiTag = (uiTag<<8) + *psTag++;
        }
        if (psTag>=psTLVString+iMaxLen)
        {
            return -2;	// specific tag not found
        }
        
        // process length bytes
        if ((*psTag & LENMASK_NEXTBYTE)==LENMASK_NEXTBYTE)
        {
            uiLenBytes = *psTag & LENMASK_LENBYTES;
            lTemp = PubChar2Long(psTag+1, uiLenBytes);
        }
        else
        {
            uiLenBytes = 0;
            lTemp      = *psTag & LENMASK_LENBYTES;
        }
        psTag += uiLenBytes+1;
        if (psTag+lTemp>psTLVString+iMaxLen)
        {
            return -2;	// specific tag not found also
        }
        
        // Check if tag needed
        if (uiTag==uiSearchTag)
        {
            *puiOutLen = (ushort)(psTag-psTagStr+lTemp);
            memcpy(psOutTLV, psTagStr, *puiOutLen);
            return 0;
        }
        
        if (IsConstructedTag(uiTag))
        {
            if (GetSpecTLVItem(psTag, (int)lTemp, uiSearchTag, psOutTLV, puiOutLen)==0)
            {
                return 0;
            }
        }
        
        psTag += lTemp;	// advance pointer of TLV string
        if (psTag>=psTLVString+iMaxLen)
        {
            return -2;
        }
    }

    return 0;
}


int GetDE55Amex(const uchar *psSendHeader, const uchar *psRecvDE55, int iLen)
{
	uchar	*psTmp;
	uint	uiLenBytes;

	// invalid length
	if (iLen<6)
	{
		return -1;
	}
	// header mismatch
	if (memcmp(psSendHeader, psRecvDE55, 6)!=0)
	{
		return -1;
	}

	psTmp = (uchar *)psRecvDE55+6;

	// Data Sub Field 1 : Issuer Authentication Data EMV (Tag 91)
	uiLenBytes = *psTmp++;
	if (uiLenBytes>16)
	{
		return -2;
	}
	memcpy(sAuthData, psTmp, uiLenBytes);
	sgAuthDataLen = uiLenBytes;
	psTmp += uiLenBytes;
	if (psTmp-psRecvDE55>iLen)
	{
		return -3;
	}
	if (psTmp-psRecvDE55==iLen)	// end up
	{
		return 0;
	}

	// Data Sub Field 2 : Issuer Script Data
	uiLenBytes = *psTmp++;
	if (uiLenBytes>128)
	{
		return -2;
	}
	sgScriptLen = 0;
	memcpy(&sIssuerScript[sgScriptLen], psTmp, uiLenBytes);
	sgScriptLen += uiLenBytes;
	psTmp += uiLenBytes;
	if (psTmp-psRecvDE55>iLen)
	{
		return -3;
	}
	if (psTmp-psRecvDE55==iLen)	// end up
	{
		return 0;
	}

	return 0;
}

int IsConstructedTag(uint uiTag)
{
	int		i;

	for(i=0; (uiTag&0xFF00) && i<2; i++)
	{
		uiTag >>= 8;
	}

	return ((uiTag & TAGMASK_CONSTRUCTED)==TAGMASK_CONSTRUCTED);
}

// Save Iuuser Authentication Data, Issuer Script.
void SaveRspICCData(uint uiTag, const uchar *psData, int iDataLen)
{
	switch( uiTag )
	{
	case 0x91:
		memcpy(sAuthData, psData, MIN(iDataLen, 16));
		sgAuthDataLen = MIN(iDataLen, 16);
		break;

	case 0x71:
	case 0x72:
		sIssuerScript[sgScriptLen++] = (uchar)uiTag;
		if( iDataLen>127 )
		{
			sIssuerScript[sgScriptLen++] = 0x81;
		}
		sIssuerScript[sgScriptLen++] = (uchar)iDataLen;
		memcpy(&sIssuerScript[sgScriptLen], psData, iDataLen);
		sgScriptLen += iDataLen;
		break;

	case 0x9F36:
//		memcpy(sATC, psData, MIN(iDataLen, 2));	// ignore
		break;

	default:
		break;
	}
}

// Ö»´¦Àí»ù±¾Êý¾ÝÔªËØTag,²»°üÀ¨½á¹¹/Ä£°åÀàµÄTag
// Build basic TLV data, exclude structure/template.
void BuildTLVString(ushort uiEmvTag, const uchar *psData, int iLength, uchar **ppsOutData)
{
	return;

	uchar	*psTemp = (uchar*)malloc((sizeof(uchar) * iLength) + 1);;
	char temp[256] = {0};

	memset(temp, '\0', strlen(temp));
	PubBcd2Asc(psData, iLength, temp);
	//ShowLogs(1, "Inside Tag: %s", temp);
	if( iLength < 0 )
	{
		free(psTemp);
		return;
	}
	// set tags
	psTemp = *ppsOutData;
	//ShowLogs(1, "Inside BuildTLVString 3");
	if( uiEmvTag & 0xFF00 )
	{
		//ShowLogs(1, "Inside BuildTLVString 4");
		*psTemp++ = (uchar)(uiEmvTag >> 8);
	}
	//ShowLogs(1, "Inside BuildTLVString 5");
	*psTemp++ = (uchar)uiEmvTag;

	// set length
	// for now, lengths of all data are less then 127 bytes, but still extend the rest part based on standard
	//ShowLogs(1, "Inside BuildTLVString 6");
	if( iLength<=127 )	
	{	
		//ShowLogs(1, "Inside BuildTLVString 7");
		*psTemp++ = (uchar)iLength;
	}
	else
	{	
		//ShowLogs(1, "Inside BuildTLVString 8");
		// the upper limit is 255 bytes data defined by EMV standard
		*psTemp++ = 0x81;
		*psTemp++ = (uchar)iLength;
	}

	//ShowLogs(1, "Inside BuildTLVString 9");
	// set value
	if( iLength>0 )
	{
		//ShowLogs(1, "Inside BuildTLVString 10");
		memcpy(psTemp, psData, iLength);
		psTemp += iLength;
	}
	//ShowLogs(1, "Inside BuildTLVString 11");
	*ppsOutData = psTemp;
	//ShowLogs(1, "Inside BuildTLVString 12");
	free(psTemp);
}

// Retrieve EMV data from core, for saving record or upload use.
void SaveEmvData(void)
{
	int		iLength;

	//ShowLogs(1, "Whoops: I am inside SaveEmvData");

	EMVGetTLVData(0x9F26, glProcInfo.stTranLog.sAppCrypto, &iLength);
	EMVGetTLVData(0x8A,   glProcInfo.stTranLog.szRspCode,  &iLength);
	EMVGetTLVData(0x95,   glProcInfo.stTranLog.sTVR,       &iLength);
	EMVGetTLVData(0x9B,   glProcInfo.stTranLog.sTSI,       &iLength);
	EMVGetTLVData(0x9F36, glProcInfo.stTranLog.sATC,       &iLength);

	// save for upload
	SetDE55(DE55_UPLOAD, glProcInfo.stTranLog.sIccData, &iLength);
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

	if( glProcInfo.ucOnlineStatus!=ST_ONLINE_APPV )
	{	
		// ICCÍÑ»ú, offline approved
		SaveTVRTSI(TRUE);
		GetNewTraceNo();
		//		sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
		//		sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu", glSysCtrl.ulSTAN);
		if(ChkIfAmex())
		{
			if(glProcInfo.ucOnlineStatus==ST_ONLINE_FAIL)
			{
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y3");
			} 
			else
			{
				// for AMEX, approval code = Y1 while chip off line apporved.
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y1");	
			}		
		}
		else
		{
			sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu", glSysCtrl.ulSTAN);
		}

		if (ChkIfAcqNeedDE56())
		{
			SetDE56(glProcInfo.stTranLog.sField56, &iLength);
			glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
		}
	}
}

// remove at version 1.00.0016 core already fixed this bug
// core cannot process correctly if length of 9F18 is zero
// eg, 71 12 9F 18 00 86 0D 84 1E 00 00 08 11 22 33 44 55 66 77 88
void AdjustIssuerScript(void)
{
	int		iRet;
	uchar	*psTemp;

	memset(sgScriptBak, 0, sizeof(sgScriptBak));
	memset(&sgScriptInfo, 0, sizeof(sgScriptInfo));
	sgCurScript = sgScriptBakLen = 0;
	bHaveScript  = FALSE;
	for(psTemp=sIssuerScript; psTemp<sIssuerScript+sgScriptLen; )
	{
		iRet = GetTLVItem(&psTemp, sIssuerScript+sgScriptLen-psTemp, SaveScriptData, TRUE);
		if( iRet<0 )
		{
			return;
		}
	}
	if( bHaveScript && sgCurScript>0 )
	{
		PackScriptData();
	}

	memcpy(sIssuerScript, sgScriptBak, sgScriptBakLen);
	sgScriptLen = sgScriptBakLen;
}

// callback function for process issuer script
void  SaveScriptData(uint uiTag, const uchar *psData, int iDataLen)
{
	switch( uiTag )
	{
	case 0x71:
	case 0x72:
		if( bHaveScript && sgCurScript>0 )
		{
			PackScriptData();
		}
		sgScriptInfo.uiTag = uiTag;
		bHaveScript = TRUE;
		break;

	case 0x9F18:
		sgScriptInfo.iIDLen = MIN(4, iDataLen);
		memcpy(sgScriptInfo.sScriptID, psData, MIN(4, iDataLen));
		break;

	case 0x86:
		sgScriptInfo.iCmdLen[sgCurScript] = iDataLen;
		memcpy(sgScriptInfo.sScriptCmd[sgCurScript], psData, iDataLen);
		sgCurScript++;
		break;

	default:
		break;
	}
}

void PackTLVData(uint uiTag, const uchar *psData, uint uiDataLen, uchar *psOutData, int *piOutLen)
{
	int		iHeadLen;

	PackTLVHead(uiTag, uiDataLen, psOutData, &iHeadLen);
	memcpy(psOutData+iHeadLen, psData, uiDataLen);
	*piOutLen = (uiDataLen+iHeadLen);
}

void PackTLVHead(uint uiTag, uint uiDataLen, uchar *psOutData, int *piOutLen)
{
	uchar	*psTemp;

	// pack tag bytes
	psTemp = psOutData;
	if( uiTag & 0xFF00 )
	{
		*psTemp++ = uiTag>>8;
	}
	*psTemp++ = uiTag;

	// pack length bytes
	if( uiDataLen<=127 )
	{
		*psTemp++ = (uchar)uiDataLen;
	}
	else
	{
		*psTemp++ = LENMASK_NEXTBYTE|0x01;	// one byte length
		*psTemp++ = (uchar)uiDataLen;
	}

	*piOutLen = (psTemp-psOutData);
}

int CalcTLVTotalLen(uint uiTag, uint uiDataLen)
{
	int		iLen;

	// get length of TLV tag bytes
	iLen = 1;
	if( uiTag & 0xFF00 )
	{
		iLen++;
	}

	// get length of TLV length bytes
	iLen++;
	if( uiDataLen>127 )
	{
		iLen++;
	}

	return (iLen+uiDataLen);
}

// re-generate issuer script(remove issuer script ID, if the length is zero)
void PackScriptData(void)
{
	int		iCnt, iTotalLen, iTempLen;

	iTotalLen = 0;
	if( sgScriptInfo.iIDLen>0 )
	{
		iTotalLen += CalcTLVTotalLen(0x9F18, 4);
	}
	for(iCnt=0; iCnt<sgCurScript; iCnt++)
	{
		iTotalLen += CalcTLVTotalLen(0x86, sgScriptInfo.iCmdLen[iCnt]);
	}
	PackTLVHead(sgScriptInfo.uiTag, iTotalLen, &sgScriptBak[sgScriptBakLen], &iTempLen);
	sgScriptBakLen += iTempLen;

	if( sgScriptInfo.iIDLen>0 )
	{
		PackTLVData(0x9F18, sgScriptInfo.sScriptID, 4, &sgScriptBak[sgScriptBakLen], &iTempLen);
		sgScriptBakLen += iTempLen;
	}
	for(iCnt=0; iCnt<sgCurScript; iCnt++)
	{
		PackTLVData(0x86, sgScriptInfo.sScriptCmd[iCnt], sgScriptInfo.iCmdLen[iCnt], &sgScriptBak[sgScriptBakLen], &iTempLen);
		sgScriptBakLen += iTempLen;
	}

	memset(&sgScriptInfo, 0, sizeof(sgScriptInfo));
	sgCurScript = 0;
}

// save EMV status for FUNC 9
void SaveTVRTSI(uchar bBeforeOnline)
{
	int				iRet, iLength, iCnt;
	uchar			sTermAID[16], sBuff[512];
	uchar			*psTLVData;
	EMV_APPLIST		stEmvApp;
	DE55Tag stList[] =
	{
		{0x5A,   DE55_MUST_SET, 0},
		{0x5F2A, DE55_MUST_SET, 0},
		{0x5F34, DE55_MUST_SET, 0},
		{0x82,   DE55_MUST_SET, 0},
		{0x84,   DE55_MUST_SET, 0},
		{0x8A,   DE55_MUST_SET, 0},
		{0x95,   DE55_MUST_SET, 0},
		{0x9A,   DE55_MUST_SET, 0},
		{0x9C,   DE55_MUST_SET, 0},
		{0x9F02, DE55_MUST_SET, 0},
		{0x9F03, DE55_MUST_SET, 0},
		{0x9F09, DE55_MUST_SET, 0},
		{0x9F10, DE55_MUST_SET, 0},
		{0x9F1A, DE55_MUST_SET, 0},
		{0x9F1E, DE55_MUST_SET, 0},
		{0x9F33, DE55_MUST_SET, 0},
		{0x9F34, DE55_MUST_SET, 0},
		{0x9F35, DE55_MUST_SET, 0},
		{0x9F36, DE55_MUST_SET, 0},
		{0x9F37, DE55_MUST_SET, 0},
		{0x9F41, DE55_MUST_SET, 0},
		{0},
	};

	//ShowLogs(1, "Whoops: I am inside SaveTVRTSI -- 1");

	SetStdDE55(FALSE, stList, glEmvStatus.sTLV+2, &iLength);//I commented this out
	glEmvStatus.sTLV[0] = iLength/256;
	glEmvStatus.sTLV[1] = iLength%256;

	if (glProcInfo.bIsFirstGAC)
	{
		psTLVData = glEmvStatus.sAppCryptoFirst+2;

		EMVGetTLVData(0x9F26, sBuff, &iLength);
		BuildTLVString(0x9F26, sBuff, iLength, &psTLVData);

		EMVGetTLVData(0x9F27, sBuff, &iLength);
		BuildTLVString(0x9F27, sBuff, iLength, &psTLVData);

		iLength = psTLVData - glEmvStatus.sAppCryptoFirst - 2;
		glEmvStatus.sAppCryptoFirst[0] = iLength/256;
		glEmvStatus.sAppCryptoFirst[1] = iLength%256;
	}
	else
	{
		psTLVData = glEmvStatus.sAppCryptoSecond+2;

		EMVGetTLVData(0x9F26, sBuff, &iLength);
		BuildTLVString(0x9F26, sBuff, iLength, &psTLVData);

		EMVGetTLVData(0x9F27, sBuff, &iLength);
		BuildTLVString(0x9F27, sBuff, iLength, &psTLVData);

		iLength = psTLVData - glEmvStatus.sAppCryptoSecond - 2;
		glEmvStatus.sAppCryptoSecond[0] = iLength/256;
		glEmvStatus.sAppCryptoSecond[1] = iLength%256;
	}

	if( bBeforeOnline )
	{
		EMVGetTLVData(0x95,   glEmvStatus.sgTVROld,  &iLength);
		EMVGetTLVData(0x9B,   glEmvStatus.sgTSIOld,  &iLength);
		glEmvStatus.sgARQCLenOld = 0;
		EMVGetTLVData(0x9F10, glEmvStatus.sgARQCOld, &glEmvStatus.sgARQCLenOld);

		EMVGetTLVData(0x9F0E, glEmvStatus.sgIACDeinal, &iLength);
		EMVGetTLVData(0x9F0F, glEmvStatus.sgIACOnline, &iLength);
		EMVGetTLVData(0x9F0D, glEmvStatus.sgIACDefault, &iLength);

		// search TAC from terminal parameter
		memset(sTermAID, 0, sizeof(sTermAID));
		EMVGetTLVData(0x9F06, sTermAID, &iLength);
		for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
		{
			memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
			iRet = EMVGetApp(iCnt, &stEmvApp);
			if( iRet!=EMV_OK )
			{
				continue;
			}
			if( memcmp(sTermAID, stEmvApp.AID, stEmvApp.AidLen)==0 )
			{
				memcpy(glEmvStatus.sgTACDeinal,  stEmvApp.TACDenial,  5);
				memcpy(glEmvStatus.sgTACOnline,  stEmvApp.TACOnline,  5);
				memcpy(glEmvStatus.sgTACDefault, stEmvApp.TACDefault, 5);
				break;
			}
		}
	}
	else
	{
		EMVGetTLVData(0x95,   glEmvStatus.sgTVRNew,  &iLength);
		EMVGetTLVData(0x9B,   glEmvStatus.sgTSINew,  &iLength);
	}
	SaveEmvStatus();
}

void UpdateEntryModeForOfflinePIN(void)
{
	int		iRet, iLength;
	uchar	sTemp[64];

	//ShowLogs(1, "Inside UpdateEntryModeForOfflinePIN 1");

	if ( !(glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) )
	{
		return;
	}

	memset(sTemp, 0, sizeof(sTemp));
	iRet = EMVGetTLVData(0x9F34, sTemp, &iLength);
	//ShowLogs(1, "Whoops: I am inside UpdateEntryModeForOfflinePIN");
	if( iRet==EMV_OK )
	{
		sTemp[0] &= 0x3F;
		if (sTemp[2]==0x02)		// last CVM succeed
		{
			if (sTemp[0]==0x01 ||	// plaintext PIN
				sTemp[0]==0x03 ||	// plaintext PIN and signature
				sTemp[0]==0x04 ||	// enciphered PIN
				sTemp[0]==0x05)	// enciphered PIN and signature
			{
				glProcInfo.stTranLog.uiEntryMode |= MODE_OFF_PIN;
			}
		}
	}
}

// show last EMV status
// Modified by Kim_LinHB 2014-6-8
int ViewTVR_TSI(void)
{
	int		iTemp;
	GUI_PAGELINE stBuff[100];
	unsigned char szHex[1+1];
	int		nLine = 0;
	GUI_PAGE stPage;
	GUI_TEXT_ATTR stLeftAttr_Small = gl_stLeftAttr;

	SetCurrTitle(_T("View TVR TSI")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	LoadEmvStatus();

	memset(stBuff, 0, sizeof(stBuff));
	stLeftAttr_Small.eFontSize = GUI_FONT_SMALL;

	sprintf(stBuff[nLine].szLine,"Before TSI=%02X %02X", glEmvStatus.sgTSIOld[0], glEmvStatus.sgTSIOld[1]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine,"TVR=%02X %02X %02X %02X %02X",
								glEmvStatus.sgTVROld[0], glEmvStatus.sgTVROld[1], glEmvStatus.sgTVROld[2],
								glEmvStatus.sgTVROld[3], glEmvStatus.sgTVROld[4]);
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	sprintf(stBuff[nLine].szLine, "IssuAppData=");
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	for(iTemp=0; iTemp<glEmvStatus.sgARQCLenOld; iTemp++)
	{	
		sprintf(szHex, "%02X", glEmvStatus.sgARQCOld[iTemp]);
		strcat(stBuff[nLine].szLine, szHex);
		if(0 == iTemp % 5)
			++nLine;
	}
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "After TSI=%02X %02X", glEmvStatus.sgTSINew[0], glEmvStatus.sgTSINew[1]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "TVR=%02X %02X %02X %02X %02X",
								glEmvStatus.sgTVRNew[0], glEmvStatus.sgTVRNew[1], glEmvStatus.sgTVRNew[2],
								glEmvStatus.sgTVRNew[3], glEmvStatus.sgTVRNew[4]);
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "TACDenial =");
	PubBcd2Asc0(glEmvStatus.sgTACDeinal, 5, stBuff[nLine].szLine+strlen("TACDenial ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "TACOnline =");
	PubBcd2Asc0(glEmvStatus.sgTACOnline, 5, stBuff[nLine].szLine+strlen("TACOnline ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "IACDenial =");
	PubBcd2Asc0(glEmvStatus.sgIACDeinal, 5, stBuff[nLine].szLine+strlen("IACDenial ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "IACOnline =");
	PubBcd2Asc0(glEmvStatus.sgIACOnline, 5, stBuff[nLine].szLine+strlen("IACOnline ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "IACDefault =");
	PubBcd2Asc0(glEmvStatus.sgIACDefault, 5, stBuff[nLine].szLine+strlen("IACDefault ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	Gui_CreateInfoPage(GetCurrTitle(), gl_stTitleAttr, stBuff, nLine, &stPage);
	
	Gui_ClearScr();
	Gui_ShowInfoPage(&stPage, FALSE, USER_OPER_TIMEOUT);

    if(glEmvStatus.sAppCryptoFirst[1] > 0 ||
       glEmvStatus.sAppCryptoSecond[1] > 0 ||
       glEmvStatus.sTLV[1] > 0) // Added by Kim 20150116 bug612
    {
        Gui_ClearScr();
        if(GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PRINT DETAIL?"), gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL)){
            return 0;
        }

        PrnInit();
        PrnSetNormal();
        PubDebugOutput("FIRST GAC", glEmvStatus.sAppCryptoFirst+2,
                        glEmvStatus.sAppCryptoFirst[1],
                        DEVICE_PRN, TLV_MODE);
        PubDebugOutput("SECOND GAC", glEmvStatus.sAppCryptoSecond+2,
                        glEmvStatus.sAppCryptoSecond[1],
                        DEVICE_PRN, TLV_MODE);
        PubDebugOutput("TRANS TLV", glEmvStatus.sTLV+2,
                        glEmvStatus.sTLV[1],
                        DEVICE_PRN, TLV_MODE);
	}
    return 0;
}

unsigned char  cEMVPiccIsoCommand(unsigned char cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
	return 0;
}

#endif		// #ifdef ENABLE_EMV

unsigned char cPiccIsoCommand_Entry(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char  cPiccIsoCommand_Pboc (unsigned char cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char cPiccIsoCommand_Wave(uchar cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char cPiccIsoCommand_MC(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

//added by kevinliu 2015/10/19
unsigned char cPiccIsoCommand_AE(uchar cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

int cClssCheckExceptionFile_Pboc(uchar *pucPAN, int nPANLen, uchar *pucPANSeq)
{
	return EMV_OK;
}

unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn,unsigned char *paucMsgIn,int nMsglenIn, unsigned char *paucSignIn, int nSignlenIn)
{
	return EMV_OK;
}

unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn,unsigned char *paucResultOut)
{
	return EMV_OK;
}

// end of file

