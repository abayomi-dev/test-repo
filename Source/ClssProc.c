#include "global.h"

#ifdef ENABLE_CONTLESS

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

#define DE55_MUST_SET			0x10	// 必须存在
#define DE55_OPT_SET			0x20	// 可选择存在
#define DE55_COND_SET			0x30	// 根据条件存在

/********************** Internal structure declaration *********************/

// callback function for GetTLVItem() to save TLV value
typedef void (*SaveTLVData)(uint uiTag, const uchar *psData, int iDataLen);

typedef struct _tagDE55Tag
{
	ushort	uiEmvTag;
	uchar	ucOption;
	uchar	ucLen;	
}DE55ClSSTag;

typedef struct _tagClssData
{
	uchar ucCardType;
	uchar ucSchemeId;

}CLSSDATA;

//added by Gillian  2015/9/25
typedef struct
{
	unsigned char aucProgramId[17]; //Application Program ID
	unsigned char ucPrgramIdLen;    //Application Program ID锟斤拷锟斤拷
	Clss_PreProcInfo tPreProcDRL;
}App_ProgramID;

/********************** external variable declaration *********************/
extern uchar gl_aucOutcomeParamSet[8];
extern uchar gl_aucUserInterReqData[22];
extern uchar gl_aucErrIndication[6];
extern CLSS_DISC_DATA_MC gl_DiscretionayData;
static uchar gl_ucDRLSupportFlg = 1;

Clss_PreProcInfo	glPreProcInfo[MAX_APP_NUM];
Clss_ProgramID ptProgInfo;

#define MAX_WAVE_AID_NUM 10//support WAVE ID count
CLSS_OUTCOME_DATA  stOutComeData;

//added by Gillian Chen 2015/9/25
Clss_PreProcInfo glClss_PreProcInfoIn;
uchar aucProID[17];
int nProIDLen = 0;
uchar gl_ucRemovalTimeup = 0;

/********************** Internal functions declaration *********************/

int IsConstructedTag(uint uiTag);
static int  GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll);
int GetSpecTLVItem(uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen);
int IssScrCon(void);
int Clss_transmit(uchar kerId);
int Clss_SetTLVData(unsigned int tag,uchar *data,int datalen,uchar flag);
int Clss_GetTLVData(unsigned int tag, uchar *data, int *datalen, uchar flag);
int disp_clss_err(int err);
void ClssBaseParameterSet_AE();
void SetAEAidParam_AE();
void ClssBaseParameterSet_WAVE();
int ClssTransInit();
int nSetDETData(uchar *pucTag, uchar ucTagLen, uchar *pucData, uchar ucDataLen);
int SetClSSDE55(uchar bForUpLoad, uchar *psOutData, int *piOutLen);
int SetStdDEClSS55(uchar bForUpLoad, DE55ClSSTag *pstList, uchar *psOutData, int *piOutLen);
//uchar SearchSpecTLV(ushort nTag, uchar *sDataIn, ushort nDataInLen, uchar *sDataOut, ushort *pnDataOutLen);
void BuildCLSSTLVString(ushort uiEmvTag, uchar *psData, int iLength, uchar **ppsOutData);
int GetPanFromTrack2(uchar *pszPAN, uchar *pszExpDate,int iLen);//hdadd少了参数
//added by Gillian  2015/9/25
int nAppFindMatchProID(unsigned char *pucProID, int ucProIDLen);
//added by Kevinliu 2015/11/28
int Clss_secondTapCard();
int ClssPreProcTxnParam();
int ClssCompleteTrans_AE(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen);
int ClssCompleteTrans_DPAS(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen);
int ClssCompleteTrans_JCB(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen);
int ClssCompleteTrans_WAVE(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen);


/********************** Internal variables declaration *********************/


// 非接消费55域标签,目前与EMV的标签一致
// F55 TLV format 
static DE55ClSSTag sgStdClssTagList[] =
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
	{0},

	//Nibss Expectation
	/*{0x5F2A, DE55_MUST_SET,  0},
	{0x82,   DE55_MUST_SET,  0},
	{0x84,   DE55_MUST_SET,  0},
	{0x95,   DE55_MUST_SET,  0},
	{0x9A,   DE55_MUST_SET,  0},
	{0x9C,   DE55_MUST_SET,  0},
	{0x9F02, DE55_MUST_SET,  0},
	{0x9F03, DE55_MUST_SET,  0},
	{0x9F09, DE55_MUST_SET,  0},
	{0x9F10, DE55_MUST_SET,  0},
	{0x9F1A, DE55_MUST_SET,  0},
	{0x9F26, DE55_MUST_SET,  0},
	{0x9F27, DE55_MUST_SET,  0},
	{0x9F33, DE55_MUST_SET,  0},
	{0x9F34, DE55_MUST_SET,  0},
	{0x9F35, DE55_MUST_SET,  0},
	{0x9F36, DE55_MUST_SET,  0},
	{0x9F37, DE55_MUST_SET,  0},
	{0x50, 	 DE55_MUST_SET,  0},
	{0x9F1D, DE55_OPT_SET,  0},
	{0},
	
	/*
	{0x57,   DE55_OPT_SET,  0},
	{0x5A,   DE55_OPT_SET,  0},
	{0x5F24, DE55_OPT_SET,  0},
	{0x5F2A, DE55_MUST_SET,  0},
	{0x5F34, DE55_OPT_SET,  0},
	{0x82,   DE55_MUST_SET,  0},
	{0x84,   DE55_MUST_SET,  0},
	{0x95,   DE55_MUST_SET,  0},
	{0x9A,   DE55_MUST_SET,  0},
	{0x9B,   DE55_OPT_SET,  0},
	{0x9C,   DE55_MUST_SET,  0},
	{0x9F02, DE55_MUST_SET,  0},
	{0x9F03, DE55_MUST_SET,  0},
	{0x9F08, DE55_OPT_SET,  0},
	{0x9F09, DE55_OPT_SET,  0},
	{0x9F10, DE55_OPT_SET,  0},
	{0x9F1A, DE55_MUST_SET,  0},
	{0x9F1E, DE55_OPT_SET,  0},
//	{0x9F1F, DE55_OPT_SET,  0},
	{0x9F26, DE55_MUST_SET,  0},
	{0x9F27, DE55_MUST_SET,  0},
	{0x9F33, DE55_MUST_SET,  0},
	{0x9F34, DE55_MUST_SET,  0},
	{0x9F35, DE55_OPT_SET,  0},
	{0x9F36, DE55_MUST_SET,  0},
	{0x9F37, DE55_MUST_SET,  0},
	{0x9F41, DE55_OPT_SET,  0},
	{0x9F5B, DE55_OPT_SET,  0},
	{0},*/
};

// TC-UPLOAD, TLV format
static DE55ClSSTag sgTcClssTagList[] =
{
	{0x5A,   DE55_OPT_SET,  0},
	{0x5F24, DE55_OPT_SET,  0},
	{0x5F2A, DE55_MUST_SET,  0},
	{0x5F34, DE55_OPT_SET,  0},
	{0x82,   DE55_MUST_SET,  0},
	{0x84,   DE55_MUST_SET,  0},
	{0x8A,   DE55_OPT_SET,  0},
	{0x95,   DE55_MUST_SET,  0},
	{0x9A,   DE55_MUST_SET,  0},
	{0x9B,   DE55_OPT_SET,  0},
	{0x9C,   DE55_MUST_SET,  0},
	{0x9F02, DE55_MUST_SET,  0},
	{0x9F03, DE55_MUST_SET,  0},
	{0x9F08, DE55_OPT_SET,  0},
	{0x9F09, DE55_OPT_SET,  0},
	{0x9F10, DE55_OPT_SET,  0},
	{0x9F18, DE55_OPT_SET,  0},
	{0x9F1A, DE55_MUST_SET,  0},
	{0x9F1E, DE55_OPT_SET,  0},
	{0x9F26, DE55_MUST_SET,  0},
	{0x9F27, DE55_MUST_SET,  0},
	{0x9F33, DE55_MUST_SET,  0},
	{0x9F34, DE55_MUST_SET,  0},
	{0x9F35, DE55_OPT_SET,  0},
	{0x9F36, DE55_MUST_SET,  0},
	{0x9F37, DE55_MUST_SET,  0},
	{0x9F41, DE55_OPT_SET,  0},
	{0x9F5B, DE55_OPT_SET,  0},
	{0},
};

// 非接消费56域标签,目前与EMV的标签一致
// F56 TLV format 
static DE55ClSSTag sgStdClssField56TagList[] =
{
	{0x5A,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9B,   DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0},
};

CLSSDATA sgClssData;

TLV_ELEMENT_MC tTempTLV;

EMV_CAPK glCAPKeyList[100] = {0};

static uchar sFinalAid[17];
static int sFinalAidLen;

static uchar sAuthData[16];			// authentication data from issuer
static uchar sIssuerScript[300];	// issuer script
static int sgAuthDataLen, sgScriptLen;
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
//----------------------------------------------------------------------------------
//                                 
//                                     L3回调函数 
//									   L3Callback functions
//
//-----------------------------------------------------------------------------------


int SetClSSDE55(uchar bForUpLoad, uchar *psOutData, int *piOutLen)
{
	if (bForUpLoad)
	{
		return SetStdDEClSS55(bForUpLoad, sgTcClssTagList, psOutData, piOutLen);
	}
	else
	{
		ShowLogs(1, "About doing SetStdDEClSS55");
		return SetStdDEClSS55(bForUpLoad, sgStdClssTagList, psOutData, piOutLen);
	}
}


// set ADVT/TIP bit 55
int SetStdDEClSS55(uchar bForUpLoad, DE55ClSSTag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt;
	int 	iLength;
	uchar	*psTemp, sBuff[200];
	char temp[258] = {0};
	char save[258] = {0};

	*piOutLen = 0;
	psTemp    = psOutData;

	if (sgClssData.ucSchemeId==CLSS_MC_MAG)
	{
		//ShowLogs(1, "SetStdDEClSS55 Step 0");
		return 0;
	}

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
				//strcpy(glSendPack.testSICCData, save);
			}else
			{
				memset(temp, '\0', strlen(temp));
				memset(save, '\0', strlen(save));
				PubBcd2Asc(sBuff, iLength, temp);
				sprintf(save, "%02X%02X%s", pstList[iCnt].uiEmvTag, iLength, temp);
				//ShowLogs(1, "Save: %s", save);
				//strcat(glSendPack.testSICCData, save);
			}
		}
	}

	/*for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		//在非接触L2 的qPBOC及payWave中,'终端性能(9F33)'数据元无法从这两个库中获取。
		if (pstList[iCnt].uiEmvTag == 0x9F33)
		{
			//ShowLogs(1, "SetStdDEClSS55 Step 1");
			EMVGetParameter(&glEmvParam);
			memcpy(sBuff, glEmvParam.Capability, 3);
			iLength = 3;
			BuildCLSSTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
		}
		else
		{
			//ShowLogs(1, "SetStdDEClSS55 Step 2");
			iRet = Clss_GetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength, sgClssData.ucCardType);
			if( iRet==EMV_OK )
			{
				//ShowLogs(1, "SetStdDEClSS55 Step 2a");
				BuildCLSSTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
			}
			else if( pstList[iCnt].ucOption==DE55_MUST_SET )
			{
				//ShowLogs(1, "SetStdDEClSS55 Step 2b");
				BuildCLSSTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);
			}
		}
	}*/

	if( glProcInfo.stTranLog.szPan[0]=='5' )
	{	
		/*
		// for master card TCC = "R" -- retail
		//ShowLogs(1, "SetStdDEClSS55 Step 3");
		BuildCLSSTLVString(0x9F53, (uchar *)"R", 1, &psTemp);
		//ShowLogs(1, "SetStdDEClSS55 Step 4");
		if (bForUpLoad)
		{
			//ShowLogs(1, "SetStdDEClSS55 Step 5");
			memset(sBuff, 0, sizeof(sBuff));
			iRet = Clss_GetTLVData(0x91, sBuff, &iLength, sgClssData.ucCardType);
			//ShowLogs(1, "SetStdDEClSS55 Step 5a");
			if( iRet==EMV_OK )
			{
				//ShowLogs(1, "SetStdDEClSS55 Step 5b");
				BuildCLSSTLVString(0x91, sBuff, iLength, &psTemp);
			}
		}*/
	}

	*piOutLen = (psTemp-psOutData);
	//ShowLogs(1, "SetStdDEClSS55 Step 6");

	return 0;
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

// 只处理基本数据元素Tag,不包括结构/模板类的Tag
// Build Clss basic TLV data, exclude structure/template.
void BuildCLSSTLVString(ushort uiEmvTag, uchar *psData, int iLength, uchar **ppsOutData)
{
	uchar	*psTemp = NULL;

	if( iLength<0 )
	{
		return;
	}

	// 设置TAG
	// write tag
	psTemp = *ppsOutData;
	if( uiEmvTag & 0xFF00 )
	{
		*psTemp++ = (uchar)(uiEmvTag >> 8);
	}
	*psTemp++ = (uchar)uiEmvTag;

	// 设置Length
	// write length
	if( iLength<=127 )	// 目前数据长度均小余127字节,但仍按标准进行处理
	{
		*psTemp++ = (uchar)iLength;
	}
	else
	{	// EMV规定最多255字节的数据
		*psTemp++ = 0x81;
		*psTemp++ = (uchar)iLength;
	}

	// 设置Value
	// write value
	if( iLength>0 )
	{
		memcpy(psTemp, psData, iLength);
		psTemp += iLength;
	}

	*ppsOutData = psTemp;
}

// 从2磁道信息分析出卡号(PAN)
int GetPanFromTrack2(uchar *pszPAN, uchar *pszExpDate, int iLen)
{
	int		iPanLen = 0;
	char	*p = NULL, pszTemp[41] = {0};
	
	// 从2磁道开始到'D'
// 	iPanLen = glProcInfo.szTrack2;
// 	if( iPanLen>0 )
// 	{
// 		memset(pszTemp, 0, sizeof(pszTemp));
// 		PubBcd2Asc0(glProcInfo.szTrack2, iPanLen, pszTemp);
// 	}
// 	else
// 	{	// 2磁道都没有
// 		return ERR_SWIPECARD;
// 	}
	memset(pszTemp, 0, sizeof(pszTemp));
	memcpy(pszTemp, glProcInfo.szTrack2, iLen);
	
	p = strchr((char *)pszTemp, '=');
	if( p==NULL )
	{
		return ERR_SWIPECARD;
	}
	iPanLen = strlen(pszTemp) - strlen(p);
	if( iPanLen<13 || iPanLen>19 )
	{
		return ERR_SWIPECARD;
	}
	
	sprintf((char *)pszPAN, "%.*s", iPanLen, pszTemp);
	sprintf((char *)pszExpDate, "%.4s", p+1);
	
	return 0;
}

//*****************************************************************************************
//callback function
//added by kevinliu 2015/11/24
int Clss_cSetCAPKandRevoc_AE(unsigned char ucIndex, unsigned char *pucRid)
{
	int i = 0, iRet = 0;
	EMV_REVOCLIST tRevocList;

	if(pucRid == NULL)
	{
		return EMV_PARAM_ERR;
	}

	for (i = 0; i < MAX_KEY_NUM; i++)
	{
		if ((glCAPKeyList[i].KeyID == ucIndex) && (0 == strcmp(glCAPKeyList[i].RID, pucRid)))
		{
			iRet = Clss_AddCAPK_AE(&glCAPKeyList[i]);
			if (EMV_OK != iRet)
			{
				return EMV_PARAM_ERR;
			}
			break;
		}
	}
	if (i == MAX_KEY_NUM)	//no one match
	{
//		OsLog(LOG_ERROR, "No CAPK");
		return EMV_PARAM_ERR;
	}

	memset(&tRevocList, 0, sizeof(EMV_REVOCLIST));
	memcpy(tRevocList.ucRid, pucRid, 5);
	tRevocList.ucIndex = ucIndex;
	memcpy(tRevocList.ucCertSn, "\x00\x07\x11", 3);
	Clss_AddRevocList_AE(&tRevocList);

	return EMV_OK;
}

int ClssDetectTapCard(void)
{
	int iRet = 0, iTime = 0;
	uchar ucKey = 0;

	//tap card
	iRet = PiccOpen();
	if(iRet != 0)
	{
		SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("OPEN PICC ERR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
		return ERR_NO_DISP;
	}
	TimerSet(3,500);
	kbflush();
	while(1)
	{
		iTime = TimerCheck(3);
		if(!iTime)
		{
			return ERR_USERCANCEL;
		}

		if(kbhit() != NOKEY)
		{
			ucKey = getkey();
			if(ucKey == KEYCANCEL)
			{
				return ERR_USERCANCEL;
			}
		}
		else	//modified by kevinliu 2015/12/09 It's hard to cancel before modified.
		{
			iRet = PiccDetect(0, NULL, NULL, NULL, NULL);
//			OsLog(LOG_INFO, "%s - %d PiccDetect RET = %d", __FUNCTION__, __LINE__, iRet);
			if(iRet == 0)
			    break;
			else if(iRet == 3|| iRet==5 || iRet==6)
			{
				DelayMs(100);
				continue;
			}
			else if(iRet == 4)
			{
				SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
				Gui_ClearScr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TOO MANY CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
				return ERR_NO_DISP;
			}
			else// if(iRet == 1 || iRet==2)
            {
                return ERR_TRAN_FAIL;
            }
		}
	}
	return 0;
}

int ClssProcFlow_VISA(uchar *finalData, int finalDataLen, Clss_PreProcInterInfo ClssProcInterInfo)
{
	#ifdef CLSS_DEBUG
	Clss_SetDebug_Wave(1);
	#endif
	//	OsLog(LOG_INFO, "%s--%d", __FUNCTION__, __LINE__);
	int iRet = 0;
	uchar ucAcType = 0, ucPathType = 0, ucTemp[100] = "";
	Clss_TransParam ClssTransParam;

	memset(&ClssTransParam, 0, sizeof(Clss_TransParam));
	GetClssTxnParam(&ClssTransParam);

	ClssBaseParameterSet_WAVE();
	vSetFinalSelectAID(finalData+1, finalData[0]);
	iRet = Clss_SetFinalSelectData_Wave(finalData, finalDataLen);
	if(iRet != EMV_OK)
	{
		return iRet;
	}

	//added by Gillian Chen 2015/9/25
	iRet = Clss_SetTLVData_Wave(0x9F5A, "123", 10);
	if(gl_ucDRLSupportFlg == 1)
	{
		if (Clss_GetTLVData_Wave(0x9F5A, aucProID, &nProIDLen) == EMV_OK)
		{
			if (!nAppFindMatchProID(aucProID, nProIDLen))
			{
				if(Clss_SetDRLParam_Wave(ptProgInfo) != EMV_OK)
				{
					return ERR_TRAN_FAIL;
				}
			}
			else
			{
				return ERR_TRAN_FAIL;
			}
		}
		else
		{
			return ERR_TRAN_FAIL;
		}
	}
	iRet = Clss_SetTransData_Wave(&ClssTransParam, &ClssProcInterInfo);
	if(iRet != EMV_OK)
	{
		return iRet;
	}

	ucAcType = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_Proctrans_Wave(ucTemp, &ucAcType);
	sgClssData.ucSchemeId = ucTemp[0];
	ucPathType = ucTemp[0];
	if(iRet)
	{
		if(iRet == CLSS_RESELECT_APP)
		{
			iRet = Clss_DelCurCandApp_Entry();
			if (iRet)
			{
				return iRet;
			}
			return iRet;
		}
		//see phone
		else if((iRet == CLSS_REFER_CONSUMER_DEVICE) && ((ClssProcInterInfo.aucReaderTTQ[0] & 0x20) == 0x20))
		{
			Inter_DisplayMsg(MSG_SEE_PHONE);
			iRet= App_Try_Again;
			DelayMs(1200);
			return iRet;
		}
		else if(iRet == CLSS_USE_CONTACT)
		{
			Inter_DisplayMsg(MSG_TRY_ANOTHER_INTERFACE);
			return CLSS_USE_CONTACT;
		}
		else
		{
			return iRet;
		}
	}

	vAppSetTransPath(ucPathType);

	iRet = nAppTransProc_VISA(ucPathType, ucAcType);

	return iRet;
}

int ClssProcFlow_MC(uchar *finalData, int finalDataLen, Clss_PreProcInterInfo ClssProcInterInfo)
{
	#ifdef CLSS_DEBUG
	Clss_SetDebug_MC(1);
	#endif
	//	OsLog(LOG_INFO, "%s--%d", __FUNCTION__, __LINE__);
	int iRet = 0;
	uchar ucAcType = 0, ucPathType = 0;

	Clss_SetCBFun_SendTransDataOutput_MC(cClssSendTransDataOutput_MC);
	vSetFinalSelectAID(finalData+1, finalData[0]);
	//	SetTermParam_MC();
	ClssTermParamSet_MC();
	nAppLoadTornLog_MC();	//added by Kevin Liu 20160728
	iRet = Clss_SetFinalSelectData_MC(finalData, finalDataLen);
	//the return code is not EMV_OK, Application should get DF8129). [12/29/2014 jiangjy]
	if(iRet == CLSS_RESELECT_APP)
	{
		iRet = Clss_DelCurCandApp_Entry();
		if (iRet != 0)
		{
			vInitPaymentData();
			gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
			gl_aucUserInterReqData[1]=MI_NOT_READY;
			memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
			gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
			gl_DiscretionayData.ucErrIndicFlg = 1;
			//					nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S51.11 S51.12
//			return ERR_TRAN_FAIL;
			return iRet;
		}
		return iRet;
	}
	else if(iRet)
	{
		return iRet;
	}
	iRet = Clss_InitiateApp_MC();
	//the return code is not EMV_OK, Application should get DF8129)
	if (iRet == CLSS_RESELECT_APP) // GPO
	{
		iRet = Clss_DelCurCandApp_Entry();
		if (iRet)
		{
			vInitPaymentData();// paypass 3.0.1 by zhoujie
			gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S51.11 for paypass 3.0.1 by zhoujie
			gl_aucUserInterReqData[1]=MI_NOT_READY;
			memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
			gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
			gl_DiscretionayData.ucErrIndicFlg = 1;
			//					nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S51.11 S51.12
//			return ERR_TRAN_FAIL;
			return iRet;
		}
		return iRet;
	}
	else if(iRet)
	{
		return iRet;
	}
	ucPathType = 0;
	iRet = Clss_ReadData_MC(&ucPathType);
	//f the return code is not EMV_OK, Application should get DF8129)
	if(iRet)
	{
		if(iRet == CLSS_RESELECT_APP)
		{
			iRet = Clss_DelCurCandApp_Entry();
			if (iRet)
			{
				vInitPaymentData();
				gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
				gl_aucUserInterReqData[1]=MI_NOT_READY;
				memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

				gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
				gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
				gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
				gl_DiscretionayData.ucErrIndicFlg = 1;

				return iRet;
			}
			return iRet;
		}
		else
		{
			return iRet;
		}
	}

	vAppSetTransPath(ucPathType);

	iRet = nAppTransProc_MC(ucPathType, &ucAcType);
	if (gl_aucUserInterReqData[0] == MI_SEE_PHONE)
	{
		Inter_DisplayMsg(MSG_SEE_PHONE);
	}
	if (gl_aucOutcomeParamSet[0] == OC_TRY_AGAIN || gl_aucOutcomeParamSet[1] != OC_NA)
	{
		iRet = App_Try_Again;
	}

	return iRet;
}
//added by Kevinliu 2015-12-07
int ClssProcFlow_AE(uchar *finalData, int finalDataLen, Clss_PreProcInterInfo ClssProcInterInfo)
{
	#ifdef CLSS_DEBUG
	Clss_SetDebug_AE(1);
	#endif
	//	OsLog(LOG_INFO, "%s--%d", __FUNCTION__, __LINE__);
	int iRet = 0;
	uchar ucAcType = 0, ucTemp[100] = {0}, ucPathType = 0;
	Clss_TransParam ClssTransParam;

	memset(&ClssTransParam, 0, sizeof(Clss_TransParam));
	GetClssTxnParam(&ClssTransParam);
	ClssBaseParameterSet_AE();
	vSetFinalSelectAID(finalData+1, finalData[0]);
	//returns	EMV_OK	CLSS_PARAM_ERR	EMV_DATA_ERR	CLSS_API_ORDER_ERR
	iRet = Clss_SetFinalSelectData_AE(finalData, finalDataLen);
	if (iRet != EMV_OK)
	{
		return iRet;
	}
	iRet = Clss_SetTransData_AE(&ClssTransParam, &ClssProcInterInfo);
	//returns	EMV_OK	CLSS_PARAM_ERR
	if (iRet != EMV_OK)
	{
		return iRet;
	}

	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_Proctrans_AE(&ucPathType);
	//returns	EMV_OK	CLSS_PARAM_ERR	CLSS_RESELECT_APP	CLSS_API_ORDER_ERR OTHER
	sgClssData.ucSchemeId = ucPathType;
	if(iRet)
	{
		if (iRet == CLSS_RESELECT_APP) // GPO
		{
			iRet = Clss_DelCurCandApp_Entry();
			//returns	EMV_OK
			if (iRet)
			{
				return iRet;
			}
			return iRet;
		}
		else
		{
			return iRet;
		}
	}

	vAppSetTransPath(ucPathType);
	iRet = nAppTransProc_AE(ucPathType, &ucAcType);
	//TODO
    if ((iRet == CLSS_REFER_CONSUMER_DEVICE) && ((ClssProcInterInfo.aucReaderTTQ[0] & 0x20) == 0x20))
    {
        Inter_DisplayMsg(MSG_SEE_PHONE);
        iRet= App_Try_Again;
    }

	return iRet;
}
//added by Kevinliu 2015-12-07
int ClssProcFlow_JCB(uchar *finalData, int finalDataLen, Clss_PreProcInterInfo ClssProcInterInfo)
{
	int iRet = 0;
	uchar ucAcType = 0, ucPathType = 0;

	vSetFinalSelectAID(finalData+1, finalData[0]);
//	SetTermParam_JCB();
	ClssTermParamSet_JCB();
	ClssTxnParamSet_JCB();
	iRet = Clss_SetFinalSelectData_JCB(finalData, finalDataLen);
	if(iRet == CLSS_RESELECT_APP)
	{
		iRet = Clss_DelCurCandApp_Entry();
		if (iRet != 0)
		{
			vInitPaymentData();
			gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
			gl_aucUserInterReqData[1]=MI_NOT_READY;
			memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
			gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
			gl_DiscretionayData.ucErrIndicFlg = 1;
//			return ERR_TRAN_FAIL;
			return iRet;
		}
		return iRet;
	}
	else if(iRet)
	{
		return iRet;
	}

	ucPathType = 0;
	iRet = Clss_InitiateApp_JCB(&ucPathType);
	if (iRet == CLSS_RESELECT_APP) // GPO
	{
		iRet = Clss_DelCurCandApp_Entry();
		if (iRet)
		{
			vInitPaymentData();
			gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
			gl_aucUserInterReqData[1]=MI_NOT_READY;
			memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;
			gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
			gl_DiscretionayData.ucErrIndicFlg = 1;
//			return ERR_TRAN_FAIL;
			return iRet;
		}
		return iRet;
	}
	else if(iRet)
	{
		return iRet;
	}

	iRet = Clss_ReadData_JCB();
	if(iRet)
	{
		if(iRet == CLSS_RESELECT_APP)
		{
			iRet = Clss_DelCurCandApp_Entry();
			if (iRet)
			{
				vInitPaymentData();
				gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
				gl_aucUserInterReqData[1]=MI_NOT_READY;
				memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

				gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
				gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
				gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
				gl_DiscretionayData.ucErrIndicFlg = 1;

				return iRet;
			}
			return iRet;
		}
		else
		{
			return iRet;
		}
	}

	vAppSetTransPath(ucPathType);

	iRet = nAppTransProc_JCB(ucPathType, &ucAcType);
	if (gl_aucUserInterReqData[0] == MI_SEE_PHONE)
	{
		Inter_DisplayMsg(MSG_SEE_PHONE);
	}
	if (gl_aucOutcomeParamSet[0] == OC_TRY_AGAIN || gl_aucOutcomeParamSet[1] != OC_NA)
	{
		iRet = App_Try_Again;
	}

	return iRet;
}

int ClssProcFlow_ZIP(uchar *finalData, int finalDataLen, Clss_PreProcInterInfo ClssProcInterInfo)
{
	int iRet = 0;
	uchar ucAcType = 0, ucPathType = 0;

	vSetFinalSelectAID(finalData+1, finalData[0]);
	ClssTermParamSet_DPAS();
	iRet = Clss_SetFinalSelectData_DPAS(finalData, finalDataLen);
	if(iRet)
	{
		cSendTransDataOutput_DPAS(0x07);
	}
	if(iRet == CLSS_RESELECT_APP)
	{
		iRet = Clss_DelCurCandApp_Entry();
		if (iRet != 0)
		{
			vInitPaymentData();
			gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
			gl_aucUserInterReqData[1]=MI_NOT_READY;
			memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
			gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
			gl_DiscretionayData.ucErrIndicFlg = 1;
			//					nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S51.11 S51.12
//			return ERR_TRAN_FAIL;
			return iRet;
		}
		return iRet;
	}
	else if(iRet)
	{
		return iRet;
	}
	//set 9F66 95 after Clss_SetFinalSelectData_DPAS
	if(memcmp(ClssProcInterInfo.aucAID, "\xA0\x00\x00\x03\x24",5) != 0)// D-PAS need set TTQ
	{
		iRet = nSetDETData((uchar*)"\x9F\x66", 2, ClssProcInterInfo.aucReaderTTQ, 4);
	}
	if(ClssProcInterInfo.ucRdCLFLmtExceed == 1)
	{
		iRet = nSetDETData((uchar*)"\x95", 1, (uchar*)"\x00\x40\x04\x80\x00", 5);	//why? modified by kevinliu
//		iRet = nSetDETData((uchar*)"\x95", 1, (uchar*)"\x00\x00\x00\x80\x00", 5);
	}
	//set dynamic transaction parameters
	ClssTxnParamSet_DPas();
	iRet = Clss_InitiateApp_DPAS(&ucPathType);
	if(iRet)
	{
		cSendTransDataOutput_DPAS(0x07);
		if(iRet == CLSS_RESELECT_APP)
		{
			iRet = Clss_DelCurCandApp_Entry();
			if (iRet != 0)
			{
				vInitPaymentData();
				gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
				gl_aucUserInterReqData[1]=MI_NOT_READY;
				memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

				gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
				gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
				gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;
				gl_DiscretionayData.ucErrIndicFlg = 1;
				//					nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S51.11 S51.12
				return iRet;
			}
			return iRet;
		}
		else
		{
			return iRet;
		}
	}

	vAppSetTransPath(ucPathType);

	//added by Gillian 2015/12/17
	iRet = nAppTransProc_DPAS(ucPathType, &ucAcType);
	if (gl_aucUserInterReqData[0] == MI_SEE_PHONE)
	{
		Inter_DisplayMsg(MSG_SEE_PHONE);
	}
	if (gl_aucOutcomeParamSet[0] == OC_TRY_AGAIN || gl_aucOutcomeParamSet[1] != OC_NA)
	{
		iRet = App_Try_Again;
	}
	return iRet;
}


//added by Kevinliu 2015-12-07
int ClssProcFlow_ALL()
{
	//	OsLog(LOG_INFO, "%s--%d", __FUNCTION__, __LINE__);
	int	 iRet = 0, iLen = 0;
	uchar ucTemp[300] = {0}, ucKernType = 0;
	Clss_PreProcInterInfo ClssProcInterInfo;
	Clss_TransParam ClssTransParam;
	uchar	szTotalAmt[12+1];
	char store_env[50] = {0};
	char szBuff[50] = {0};

	GetClssTxnParam(&ClssTransParam);
	//display price and tap card prompt.
	memset(ucTemp, 0, sizeof(ucTemp));
	memset(szTotalAmt, 0, sizeof(szTotalAmt));
	PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
	
	memset(store_env, '\0', strlen(store_env));
    UtilGetEnv("curabbr", store_env);
	strncat(szBuff, store_env, 3);

	//PubConvAmount(glSysParam.stEdcInfo.stLocalCurrency.szName, szTotalAmt,
	//			glProcInfo.stTranLog.stTranCurrency.ucDecimal,
	//			glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit,
	//			ucTemp, 0);
	PubConvAmount(szBuff, szTotalAmt,
				glProcInfo.stTranLog.stTranCurrency.ucDecimal,
				glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit,
				ucTemp, 0);

	SetClssLightStatus(CLSSLIGHTSTATUS_READYFORTXN);
	strcat((char*)ucTemp, "\n");
	strcat((char*)ucTemp, _T("PLS TAP CARD"));

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, ucTemp, gl_stCenterAttr, GUI_BUTTON_NONE, 0,NULL);
	//	OsLog(LOG_INFO, "%s--%d", __FUNCTION__, __LINE__);
	//detect tap card
	iRet = ClssDetectTapCard();
	//ShowLogs(1, "ClssProcFlow_ALL Step 1");
	//returns ERR_USERCANCEL	ERR_TRAN_FAIL
	if(iRet)
	{
		return iRet;
	}

	SetClssLightStatus(CLSSLIGHTSTATUS_PROCESSING);
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS WAIT..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	glProcInfo.stTranLog.uiEntryMode = MODE_CONTACTLESS;
	Clss_SetMCVersion_Entry(3);// add for paypass 3.0 [12/29/2014 jiangjy]
	//ShowLogs(1, "ClssProcFlow_ALL Step 2");
	//app select
	iRet = Clss_AppSlt_Entry(0,0);
	//ShowLogs(1, "ClssProcFlow_ALL Step 3");
	//returns	EMV_OK	CLSS_PARAM_ERR	ICC_CMD_ERR	ICC_BLOCK	EMV_NO_APP	EMV_APP_BLOCK	EMV_NO_APP_PPSE_ERR
	if(iRet != EMV_OK)
	{
		vAppCreateOutcomeData_MC(iRet);
		return iRet;
	}
	//ShowLogs(1, "ClssProcFlow_ALL Step 4");
	while(1)
	{
		vAppInitPaymentData_MC();
		ucKernType = 0;
		iLen = 0;
		ShowLogs(1, "ClssProcFlow_ALL Step 5");
		memset(ucTemp, 0, sizeof(ucTemp));
		iRet = Clss_FinalSelect_Entry(&ucKernType, ucTemp, &iLen);
		//returns	EMV_OK	CLSS_PARAM_ERR	ICC_CMD_ERR	EMV_RSP_ERR	EMV_NO_APP	EMV_APP_BLOCK	ICC_BLOCK
		//CLSS_USE_CONTACT	EMV_DATA_ERR	CLSS_RESELECT_APP
		if(iRet != EMV_OK)
		{
			return iRet;
		}
		//VISA MASTERCARD AMERICANEXPRESS JCB DISCOVER
		if(ucKernType != KERNTYPE_VIS && ucKernType != KERNTYPE_MC && ucKernType != KERNTYPE_AE
				&& ucKernType != KERNTYPE_JCB && ucKernType != KERNTYPE_ZIP)
		{
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("UNSUPPORT CTLS"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
			return ERR_TRAN_FAIL;
		}

		//modified by kevinliu 2015/12/09	set 9c tag by set TLV function.
		Clss_SetTLVData(0x9c, &ClssTransParam.ucTransType, 1, ucKernType);
		ShowLogs(1, "ClssProcFlow_ALL Step 6");
		//get pre-process data
		memset(&ClssProcInterInfo, 0, sizeof(Clss_PreProcInterInfo));
		iRet = Clss_GetPreProcInterFlg_Entry(&ClssProcInterInfo);
		//returns	EMV_OK	CLSS_PARAM_ERR	CLSS_USE_CONTACT
		ShowLogs(1, "ClssProcFlow_ALL Step 7");
		if(iRet != EMV_OK)
		{
			return iRet;
		}
		iLen = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		iRet = Clss_GetFinalSelectData_Entry(ucTemp, &iLen);
		ShowLogs(1, "ClssProcFlow_ALL Step 8");
		//returns	EMV_OK	CLSS_PARAM_ERR
		if(iRet != EMV_OK) 
		{
			return iRet;
		}

		iRet = nAppSetCurAppType(ucKernType);//CoreInit  //kernel type// scheme id
		ShowLogs(1, "ClssProcFlow_ALL Step 9");
		switch(ucAppGetAppType())
		{
		case KERNTYPE_VIS:
			iRet = ClssProcFlow_VISA(ucTemp, iLen, ClssProcInterInfo);
			ShowLogs(1, "ClssProcFlow_ALL Step 10");
			break;
		case KERNTYPE_MC:
			iRet = ClssProcFlow_MC(ucTemp, iLen, ClssProcInterInfo);
			break;
		case KERNTYPE_AE:
			iRet = ClssProcFlow_AE(ucTemp, iLen, ClssProcInterInfo);
			break;
		case KERNTYPE_JCB:
			iRet = ClssProcFlow_JCB(ucTemp, iLen, ClssProcInterInfo);
			break;
		case KERNTYPE_ZIP:
			iRet = ClssProcFlow_ZIP(ucTemp, iLen, ClssProcInterInfo);
			break;
		default:
			break;
		}
		if(iRet == EMV_OK) {
			break;
		}
		else if(iRet == CLSS_RESELECT_APP){
			continue;
		}
		else {
            appRemovePicc();
			break;
		}
	}
	ShowLogs(1, "ClssProcFlow_ALL Step 11 %d", iRet);
	return iRet;
}

int ClssPreProcTxnParam()
{
	int iRet = 0;
	char timeGotten[15] = {0};
	Clss_TransParam ClssTransParam;
	ShowLogs(1, "ClssPreProcTxnParam Step 1");
	//pre-process
	memset(&ClssTransParam, 0, sizeof(Clss_TransParam));
	ShowLogs(1, "ClssPreProcTxnParam Step 1a %s", glProcInfo.stTranLog.szAmount);
	
	ClssTransParam.ulAmntAuth = atol((char *)glProcInfo.stTranLog.szAmount) + atol((char *)glProcInfo.stTranLog.szTipAmount);
	ShowLogs(1, "ClssPreProcTxnParam Step 1b %lu", ClssTransParam.ulAmntAuth);
	ClssTransParam.ulAmntOther = 0;
	ClssTransParam.ucTransType = 0x00;//EMV_GOODS;

	SysGetTimeIso(timeGotten);
	ShowLogs(1, "Time: %s. %s", timeGotten, timeGotten+6);//190323193543

	ShowLogs(1, "ClssPreProcTxnParam Step 1c %s", glProcInfo.stTranLog.szDateTime+2);
	ShowLogs(1, "ClssPreProcTxnParam Step 1d %s", glProcInfo.stTranLog.szDateTime+8);
	PubAsc2Bcd(timeGotten, 6, ClssTransParam.aucTransDate);
	PubAsc2Bcd(timeGotten+6, 6, ClssTransParam.aucTransTime);

	glSysCtrl.ulSTAN = useStan;
	glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
	ClssTransParam.ulTransNo = glProcInfo.stTranLog.ulSTAN;
	ShowLogs(1, "ClssPreProcTxnParam Step 1e %lu", glProcInfo.stTranLog.ulSTAN);

	SetClssTxnParam(&ClssTransParam);

	ShowLogs(1, "ClssPreProcTxnParam Step 2");
	iRet = Clss_PreTransProc_Entry(&ClssTransParam);
	//returns EMV_OK	CLSS_PARAM_ERR		CLSS_USE_CONTACT
	ShowLogs(1, "ClssPreProcTxnParam Step 3: %d - %d - %d", iRet, EMV_OK, CLSS_USE_CONTACT);
	if(iRet != EMV_OK)
	{
		if (iRet == CLSS_USE_CONTACT)
		{
			disp_clss_err(iRet);
		}
	}
	ShowLogs(1, "ClssPreProcTxnParam Step 4");
	return iRet;
}


/***********************************************************************************************/
void SetClssLightStatus(CLSSLIGHTSTATUS status)
{
	PiccLight(PICC_LED_ALL, 0);
    switch (status)
	{
		case CLSSLIGHTSTATUS_NOTREADY:
			PiccLight(PICC_LED_ALL, 0);
			break;
		case CLSSLIGHTSTATUS_IDLE:
			PiccLight(PICC_LED_BLUE | PICC_LED_CLSS, 1);
			break;
		case CLSSLIGHTSTATUS_READYFORTXN:
			PiccLight(PICC_LED_BLUE | PICC_LED_CLSS, 1);
			break;
		case CLSSLIGHTSTATUS_PROCESSING:
			PiccLight(PICC_LED_BLUE | PICC_LED_YELLOW | PICC_LED_CLSS, 1);
			break;
		case CLSSLIGHTSTATUS_READCARDDONE:
		case CLSSLIGHTSTATUS_REMOVECARD:
		case CLSSLIGHTSTATUS_DIALING:
		case CLSSLIGHTSTATUS_SENDING:
		case CLSSLIGHTSTATUS_RECEIVING1:
		case CLSSLIGHTSTATUS_RECEIVING2:
		case CLSSLIGHTSTATUS_PRINTING:
			PiccLight(PICC_LED_BLUE | PICC_LED_YELLOW | PICC_LED_GREEN | PICC_LED_CLSS, 1);
			break;
		case CLSSLIGHTSTATUS_COMPLETE:
			PiccLight(PICC_LED_BLUE | PICC_LED_YELLOW | PICC_LED_GREEN | PICC_LED_CLSS, 1);
			break;
		case CLSSLIGHTSTATUS_ERROR:
			PiccLight(PICC_LED_RED | PICC_LED_CLSS, 1);
			break;
		default:
			break;
	}
}

/***********************************************************************************************/

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
//modified by kevinliu 2015/11/27
int TransClssSale(void)
{
	int	 iRet = 0, iLen = 0, i = 0;
	uchar ucTemp[100] = {0}, ucTranAct = 0, ucKernType = 0, ucAcType = 0, ucMSDPath = 0;
	uchar ucPathType = 0;
	uchar ucTempPAN[24]={0};
	uchar temp[128]={0};
	char name[50] = {0};
	char szTotalAmt[13] = {0};
	char sTemp[50] = {0};
	char testTmp[24]={0};
	int	iLength;
	char iccData[500] = {0};
	uchar tempIcc[500]={0};

	txnType = 12;
	memset(&glSendPack, 0, sizeof(STISO8583));

	//By Wisdom
	Clss_SetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6, ucKernType);
	Clss_SetTLVData(0x9F33, (uchar *)"\xE0\xF8\xC8", 3, ucKernType);//\xE0\x08\xC8
	Clss_SetTLVData(0x9F35, (uchar *)"\x22", 1, ucKernType);
	Clss_SetTLVData(0x5F2A, (uchar *)"\x05\x66", 2, ucKernType);
	Clss_SetTLVData(0x9F09, (uchar *)"\x00\x02", 2, ucKernType);
	Clss_SetTLVData(0x9F1A, (uchar *)"\x05\x66", 2, ucKernType);
	Clss_SetTLVData(0x9C, (uchar *)"\x00", 1, ucKernType);

	//By Wisdom
	memset(glProcInfo.stTranLog.szAmount, 0, sizeof(glProcInfo.stTranLog.szAmount));


	glProcInfo.stTranLog.ucTranType = SALE;
	displayName(name);
	SetCurrTitle(_T(name));
	ShowLogs(1, "TransClssSale Step 1");
	ClssTransInit();
	ShowLogs(1, "TransClssSale Step 2");
	vAppInitSchemeId();
	ShowLogs(1, "TransClssSale Step 3");
	//get amount
	iRet = GetAmount();
	ShowLogs(1, "TransClssSale Step 4");
	if(iRet)
	{
		ShowLogs(1, "TransClssSale Step 4b");
		return ERR_TRAN_FAIL;
	}
	DisplayInfoNone("", "Please Wait...", 0);
	//prepare transaction parameter
	iRet = ClssPreProcTxnParam();
	ShowLogs(1, "TransClssSale Step 5");
	if(iRet)
	{
		ShowLogs(1, "TransClssSale Step 5b");
		return ERR_TRAN_FAIL;
	}
	ShowLogs(1, "TransClssSale Step 5c");
	while(1)
	{
		//contactless transaction flow
		ShowLogs(1, "TransClssSale Step 6");
		iRet = ClssProcFlow_ALL();
		ShowLogs(1, "TransClssSale Step 6a %d", iRet);
		if (iRet)
		{
			/*
			//TEST
			memset(testTmp, 0, sizeof(testTmp));
			sprintf(testTmp, "iRet = %d", iRet);
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, testTmp, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
			*/
			if (iRet == App_Try_Again)
			{
				DelayMs(1200);
				continue;
			}
			ProcError_Picc(iRet);
			PiccLight(PICC_LED_ALL, 0);
			PiccClose();
			ShowLogs(1, "TransClssSale Step 6c %d", iRet);
			return ERR_TRAN_FAIL;
		}

		ucAcType = ucAppGetTransACType();
		ShowLogs(1, "TransClssSale Step 7 %d", ucAcType);
		/*
		//TEST
		memset(testTmp, 0, sizeof(testTmp));
		sprintf(testTmp, "AcType = %02x", ucAcType);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, testTmp, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
		*/
		//AC TYPE
        if ((iRet == 0) && (ucAppGetTransACType() == AC_AAC)
                && ((ucAppGetAppType() == KERNTYPE_VIS) || (ucAppGetAppType() == KERNTYPE_AE)))
        {
        	ShowLogs(1, "TransClssSale Step 8");
            iRet = EMV_DENIAL;
        }
		//create output
		ShowLogs(1, "TransClssSale Step 9");
		iRet = AppConv_CreateOutCome(iRet, ucAcType, &stOutComeData);
		ShowLogs(1, "TransClssSale Step 9b: %d", iRet);
		if (iRet)
		{
			if (iRet == App_Try_Again)
			{
				DelayMs(1200);
				continue;
			}
			ProcError_Picc(iRet);
			PiccLight(PICC_LED_ALL, 0);
			PiccClose();
			ShowLogs(1, "TransClssSale Step 9c");
			return ERR_TRAN_FAIL;
		}
		else
		{
			ShowLogs(1, "TransClssSale Step 10");
			break;
		}
	}
	ShowLogs(1, "TransClssSale Step 11");
	ucKernType = ucAppGetAppType();
	ShowLogs(1, "TransClssSale Step 12");
	ucPathType = ucAppGetTransPath();
	ShowLogs(1, "TransClssSale Step 13");
	/*
	//TEST
	memset(testTmp, 0, sizeof(testTmp));
	sprintf(testTmp, "TransRet = %02x", stOutComeData.ucTransRet);
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, testTmp, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
	//TEST
	memset(testTmp, 0, sizeof(testTmp));
	sprintf(testTmp, "CVMType = %02x", stOutComeData.ucCVMType);
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, testTmp, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
	//TEST
	memset(testTmp, 0, sizeof(testTmp));
	sprintf(testTmp, "PathType = %02x", stOutComeData.unPathType);
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, testTmp, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
	*/
	//get track data
	memset(glProcInfo.szTrack1, 0, sizeof(glProcInfo.szTrack1));
	memset(glProcInfo.szTrack2, 0, sizeof(glProcInfo.szTrack2));
	//process data
	if(ucKernType == KERNTYPE_VIS)
	{
		ShowLogs(1, "TransClssSale Step 14");
		//get MSD and Wave2 track 1 data(ASCII)
		Clss_nGetTrack1MapData_Wave (glProcInfo.szTrack1, &iLen);

		if(ucPathType == CLSS_VISA_MSD)
		{
			Clss_GetMSDType_Wave(&ucMSDPath);
			//get MSD track 2 data
			Clss_nGetTrack2MapData_Wave (glProcInfo.szTrack2, &iLen);
		}
		//chip or MSD without trk2map data
		if(strlen((char *)glProcInfo.szTrack2) == 0)
		{
			//get track 2 data from ICC
			iLen = 0;
			memset(ucTemp, 0, sizeof(ucTemp));
			iRet = Clss_GetTLVData(0x57, ucTemp, &iLen, ucKernType);
			if(iRet == EMV_OK)
			{
				memset(glProcInfo.szTrack2, 0, sizeof(glProcInfo.szTrack2));
				PubBcd2Asc0(ucTemp, iLen, glProcInfo.szTrack2);
				iLen = iLen*2;
				glProcInfo.szTrack2[iLen] = '\0';
			}
		}
	}
	else if(ucKernType == KERNTYPE_MC)
	{
		iLen = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		//get track 1 data only for paypass
		iRet = Clss_GetTLVData(0x56 , glProcInfo.szTrack1, &iLen, ucKernType);

		//get track 2 data for paypass
		iLen = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		if (ucPathType == CLSS_MC_MAG)
		{
			iRet = Clss_GetTLVData(0x9F6B, ucTemp, &iLen, ucKernType);
		}
		else if (ucPathType == CLSS_MC_MCHIP)
		{
			iRet = Clss_GetTLVData(0x57, ucTemp, &iLen, ucKernType);
		}
		if(iRet == EMV_OK)
		{
			memset(glProcInfo.szTrack2, 0, sizeof(glProcInfo.szTrack2));
			PubBcd2Asc0(ucTemp, iLen, glProcInfo.szTrack2);
			iLen = iLen*2;
			glProcInfo.szTrack2[iLen] = '\0';
		}
	}
	else if(ucKernType == KERNTYPE_AE)
	{
		if(ucPathType == AE_MAGMODE)
		{
			memset(ucTemp, 0, sizeof(ucTemp));
			Clss_nGetTrackMapData_AE(0x01, glProcInfo.szTrack1, (uchar*)&iLen);
			Clss_nGetTrackMapData_AE(0x02, ucTemp, (uchar*)&iLen);
			memcpy(glProcInfo.szTrack2, ucTemp + 1, iLen - 1);
		}
		//chip or MSD without trk2map data
		//		OsLog(LOG_INFO, "%s--%d szTrack2 = %s", __FUNCTION__, __LINE__, glProcInfo.szTrack2);
		if(strlen((char *)glProcInfo.szTrack2) == 0)
		{
			//tag = 0x57;
			iLen = 0;
			memset(ucTemp, 0, sizeof(ucTemp));
			iRet = Clss_GetTLVData(0x57, ucTemp, &iLen, ucKernType);
			if(iRet == EMV_OK)
			{
				memset(glProcInfo.szTrack2, 0, sizeof(glProcInfo.szTrack2));
				PubBcd2Asc0(ucTemp, iLen, glProcInfo.szTrack2);
				iLen = iLen*2;
				glProcInfo.szTrack2[iLen] = '\0';
			}
		}
	}
	else if(ucKernType == KERNTYPE_JCB)
	{
		iLen = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		//get track 1 data only for paypass
		iRet = Clss_GetTLVData(0x56 , glProcInfo.szTrack1, &iLen, ucKernType);

		//get track 2 data for paypass
		iLen = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		if ((ucPathType == CLSS_JCB_EMV) || (ucPathType == CLSS_JCB_LEGACY))
		{
			iRet = Clss_GetTLVData(0x57, ucTemp, &iLen, ucKernType);
		//			OsLog(LOG_INFO, "JCB get Track2 ret = %d", iRet);
		}
		//CLSS_JCB_MAG
		else if(ucPathType == CLSS_JCB_MAG)
		{
			iRet = Clss_GetTLVData(0x9F6B, ucTemp, &iLen, ucKernType);
		}

		if(iRet == EMV_OK)
		{
			memset(glProcInfo.szTrack2, 0, sizeof(glProcInfo.szTrack2));
			PubBcd2Asc0(ucTemp, iLen, glProcInfo.szTrack2);
			iLen = iLen*2;
			glProcInfo.szTrack2[iLen] = '\0';
		}
	}
	else if(ucKernType == KERNTYPE_ZIP)
	{
		if((ucPathType == CLSS_DPAS_MAG) || (ucPathType == CLSS_DPAS_ZIP))
		{
			Clss_GetTrackMapData_DPAS(0x01, glProcInfo.szTrack1, &iLen);
			Clss_GetTrackMapData_DPAS(0x02, glProcInfo.szTrack2, &iLen);
		}
		//chip or MSD without trk2map data
		if(strlen((char *)glProcInfo.szTrack2) == 0)
		{
			//tag = 0x57;
			iLen = 0;
			memset(ucTemp, 0, sizeof(ucTemp));
			iRet = Clss_GetTLVData(0x57, ucTemp, &iLen, ucKernType);
			if(iRet == EMV_OK)
			{
				memset(glProcInfo.szTrack2, 0, sizeof(glProcInfo.szTrack2));
				PubBcd2Asc0(ucTemp, iLen, glProcInfo.szTrack2);
				iLen = iLen*2;
				glProcInfo.szTrack2[iLen] = '\0';
			}
		}
	}

	for(i=0;i<iLen;i++)
	{
		if(glProcInfo.szTrack2[i] == 'D')
		{
			glProcInfo.szTrack2[i] = '=';
			break;
		}
	}

	ShowLogs(1, "Track 2: %s", glProcInfo.szTrack2);
	DisplayInfoNone("", "Please Wait...", 0);
	// get PAN from track 2 (PAN)
	iRet = GetPanFromTrack2(glProcInfo.stTranLog.szPan, glProcInfo.stTranLog.szExpDate, iLen);
	if( iRet!=0 )
	{
		DispMagReadErr();
		PiccLight(PICC_LED_ALL, 0);
		PiccClose();
		return ERR_TRAN_FAIL;
	}
	ShowLogs(1, "Pan: %s", glProcInfo.stTranLog.szPan);
	ShowLogs(1, "Expiry Date: %s", glProcInfo.stTranLog.szExpDate);
	//added by kevin liu 20160628 bug 834
	//get PAN compare with PAN in track2
	
	//Commented out by Wisdom
	/*
	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	//Application Primary Account Number (PAN) Sequence Number
	iRet = Clss_GetTLVData(0x5A, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memset(ucTempPAN, 0, sizeof(ucTempPAN));
		PubBcd2Asc(ucTemp, iLen, ucTempPAN);
		PubTrimTailChars(ucTempPAN, 'F');
		if(0 != strcmp((char *)glProcInfo.stTranLog.szPan, (char *)ucTempPAN)) {
			DispMagReadErr();
			PiccLight(PICC_LED_ALL, 0);
			PiccClose();
			return ERR_TRAN_FAIL;
		}
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x5A: %s", temp);*/

	//get issuer and acquirer from PAN
	iRet = MatchCardTable(glProcInfo.stTranLog.szPan);
	if( iRet!=0 )
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("UNSUPPORTED\nCARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		PiccLight(PICC_LED_ALL, 0);
		PiccClose();
		return ERR_TRAN_FAIL;
	}

	PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
	PubAsc2Bcd(szTotalAmt, 12, sTemp);
	Clss_SetTLVData(0x9F02, sTemp, 6, ucKernType);
	PubLong2Char((ulong)atol((char *)szTotalAmt), 4, sTemp);
	Clss_SetTLVData(0x81, sTemp, 4, ucKernType);


	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F26, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F26: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F26%.2X%s", strlen(temp)/2, temp);
   		strcpy(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F27, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F27: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F27%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F10, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F10: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F10%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F37, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F37: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F37%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F36, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F36: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F36%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x95, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x95: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "95%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9A, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9A: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9A%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9C, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9C: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9C%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F02, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F02: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F02%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x5F2A, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x5F2A: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "5F2A%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x82, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x82: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "82%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F1A, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F1A: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F1A%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	//CVM process 
	if(stOutComeData.ucCVMType == RD_CVM_ONLINE_PIN)
	{
		iRet = GetPIN(FALSE);
		if(iRet != EMV_OK)
		{
			PiccLight(PICC_LED_ALL, 0);
			PiccClose();
			return ERR_TRAN_FAIL;
		}
	}
	else if(stOutComeData.ucCVMType == RD_CVM_SIG)
	{
		DoE_Signature();
	}
	else
	{
		//no cvm
		//offline pin
		//consumer device
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F34, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F34: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F34%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}


	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F35, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F35: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F35%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F33, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F33: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F33%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x5F34, ucTemp, &iLen, ucKernType);
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x5F34: %s", temp);
	if(strlen(temp) > 0)
	{
		sprintf(glSendPack.szPanSeqNo, "0%s", temp);
	}
	


	/*iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	//Application Primary Account Number (PAN) Sequence Number
	iRet = Clss_GetTLVData(0x5f34, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		glProcInfo.stTranLog.ucPanSeqNo = ucTemp[0];
		glProcInfo.stTranLog.bPanSeqOK = TRUE;
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x5f34: %s", temp);

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x50, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppLabel, ucTemp, iLen);
		glProcInfo.stTranLog.szAppLabel[iLen]=0;
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x50: %s", temp);

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x5f24, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		PubBcd2Asc(ucTemp, 2, glProcInfo.stTranLog.szExpDate);
		glProcInfo.stTranLog.szExpDate[4]=0;
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x5f24: %s", temp);

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x95, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.sTVR, ucTemp, iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x95: %s", temp);

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9b, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.sTSI,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9b: %s", temp);
	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9f36, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.sATC,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9f36: %s", temp);

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9f26, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.sAppCrypto,ucTemp,iLen);  //szAppPreferName
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9f26: %s", temp);

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9f26, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9f26: %s", temp);*/

	//Wisdom start
	/*Clss_SetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6, ucKernType);
	Clss_SetTLVData(0x9F33, (uchar *)"\xE0\xF8\xC8", 3, ucKernType);//\xE0\x08\xC8
	Clss_SetTLVData(0x9F35, (uchar *)"\x22", 1, ucKernType);
	Clss_SetTLVData(0x5F2A, (uchar *)"\x05\x66", 2, ucKernType);
	Clss_SetTLVData(0x9F09, (uchar *)"\x00\x02", 2, ucKernType);
	Clss_SetTLVData(0x9F1A, (uchar *)"\x05\x66", 2, ucKernType);
	Clss_SetTLVData(0x9C, (uchar *)"\x00", 1, ucKernType);*/
	/*PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
	PubAsc2Bcd(szTotalAmt, 12, sTemp);
	Clss_SetTLVData(0x9F02, sTemp, 6, ucKernType);
	PubLong2Char((ulong)atol((char *)szTotalAmt), 4, sTemp);
	Clss_SetTLVData(0x81, sTemp, 4, ucKernType);*/
	//Clss_SetTLVData(0x9f27,"\x80",1,ucKernType);

	


	

	/*iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x84, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x84: %s", temp);

	//
	//Wisdom copying AID
	PubAsc2Bcd(temp, strlen(temp), glProcInfo.stTranLog.sAID);
	glProcInfo.stTranLog.ucAidLen = strlen(temp)/2;
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "84%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}*/

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x5F20, ucTemp, &iLen, ucKernType);
	if( iRet==EMV_OK )
	{
		ConvertHolderName(ucTemp, glProcInfo.stTranLog.szHolderName);
	}
	

	/*iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F03, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F03: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F03%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F09, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F09: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F09%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x50, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x50: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "50%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}
	//Wisdom end

	iLen = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_GetTLVData(0x9F1D, ucTemp, &iLen, ucKernType);
	if(iRet == EMV_OK)
	{
		memcpy(glProcInfo.stTranLog.szAppPreferName,ucTemp,iLen);
	}
	memset(temp, 0, sizeof(temp));
	PubBcd2Asc(ucTemp, iLen, temp);
	ShowLogs(1, "Tag 0x9F1D: %s", temp);
	if(strlen(temp) > 0)
	{
		memset(tempIcc, 0, sizeof(tempIcc));
   		sprintf(tempIcc, "9F1D%.2X%s", strlen(temp)/2, temp);
   		strcat(glSendPack.szNFC, tempIcc);
	}*/


	ShowLogs(1, "1. ICC: %s", glSendPack.szNFC);
	strcpy(glSendPack.testSICCData, glSendPack.szNFC);



	//continue transaction according to AC type
	if(stOutComeData.ucTransRet == CLSS_ONLINE_REQUEST)
	{
		memset(sAuthData, 0, sizeof(sAuthData));
		memset(sIssuerScript, 0, sizeof(sIssuerScript));
		sgAuthDataLen = 0;
		sgScriptLen = 0;

		iRet = Clss_transmit(ucKernType);
		if(iRet != EMV_OK)
		//if(1)
		{
			Inter_DisplayMsg(MSG_DECLINED);
			PiccLight(PICC_LED_ALL, 0);
			PiccClose();
			PrintAllReceipt(PRN_NORMAL);//Wisdom
			return ERR_TRAN_FAIL;
		}
		//TODO second tap card
		iRet = Clss_secondTapCard();
		//TODO
		//		if(iRet != EMV_OK)
		//		{
		//			ProcError_Picc(iRet);
		//			return ERR_TRAN_FAIL;
		//		}
	}
	else if(stOutComeData.ucTransRet == CLSS_APPROVE)
	{
		// save for upload
		iLen = 0;
		SetClSSDE55(TRUE, glProcInfo.stTranLog.sIccData, &iLen);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)iLen;

		GetNewTraceNo();
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
		glProcInfo.stTranLog.ulInvoiceNo = glSysCtrl.ulInvoiceNo;
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu", glSysCtrl.ulSTAN);

		SetOffBase(OffBaseDisplay);

		ucTranAct = glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct;

		if (ucTranAct & ACT_INC_TRACE)
		{
			GetNewTraceNo();
		}
		//removed by Kevin Liu 20160614 bug828
		//		DoE_Signature();

		if( ucTranAct & WRT_RECORD )
		{
			glProcInfo.stTranLog.uiStatus |= TS_NOSEND;
			SaveTranLog(&glProcInfo.stTranLog);
		}

		if( ucTranAct & PRN_RECEIPT )	// print slip
		{
			CommOnHook(FALSE);
			GetNewInvoiceNo();
			//PrintReceipt(PRN_NORMAL);
			PrintAllReceipt(PRN_NORMAL);
		}
		DispResult(0);
	}
	else
	{
		Inter_DisplayMsg(MSG_DECLINED);
		iRet = ERR_NO_DISP;
	}
	PiccLight(PICC_LED_ALL, 0);
	PiccClose();
	return iRet;
}

int Clss_secondTapCard()
{
	int iRet = 0;
	unsigned char ucKernelType = 0, aucRspCode[3] = {0}, ucOnlineResult = 0;
	unsigned char aucAuthCode[7] = {0};

	if((sgAuthDataLen == 0) || (sgScriptLen == 0))
	{
		return EMV_OK;
	}

	ucKernelType = ucAppGetAppType();
	ucOnlineResult = glProcInfo.ucOnlineStatus;
	memcpy(aucRspCode, glProcInfo.stTranLog.szRspCode, 2);
	memcpy(aucAuthCode, glProcInfo.stTranLog.szAuthCode, 6);

	SetClssLightStatus(CLSSLIGHTSTATUS_READYFORTXN);
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("SECOND TAP CARD"), gl_stCenterAttr, GUI_BUTTON_NONE, 0,NULL);

	iRet = ClssDetectTapCard();
	if(iRet)
	{
		return iRet;
	}

	SetClssLightStatus(CLSSLIGHTSTATUS_PROCESSING);
	DispProcess();

//	OsLog(LOG_INFO, "%s--%d Second tap card KernelType = %d", __FUNCTION__, __LINE__, ucKernelType);

	SetClssLightStatus(CLSSLIGHTSTATUS_REMOVECARD);
	if(ucKernelType == KERNTYPE_AE)
	{
		iRet = ClssCompleteTrans_AE(ucOnlineResult, aucRspCode, aucAuthCode, sAuthData, sgAuthDataLen, sIssuerScript, sgScriptLen);
	}
	else if(ucKernelType == KERNTYPE_VIS)
	{
		iRet = ClssCompleteTrans_WAVE(ucOnlineResult, aucRspCode, aucAuthCode, sAuthData, sgAuthDataLen, sIssuerScript, sgScriptLen);
	}
	else if(ucKernelType == KERNTYPE_ZIP)
	{
		iRet = ClssCompleteTrans_DPAS(ucOnlineResult, aucRspCode, aucAuthCode, sAuthData, sgAuthDataLen, sIssuerScript, sgScriptLen);
	}
	else if(ucKernelType == KERNTYPE_JCB)
	{
		iRet = ClssCompleteTrans_JCB(ucOnlineResult, aucRspCode, aucAuthCode, sAuthData, sgAuthDataLen, sIssuerScript, sgScriptLen);
	}
//	OsLog(LOG_INFO, "%s--%d Second tap card ret = %d", __FUNCTION__, __LINE__, iRet);

	if (iRet)
	{
		Inter_DisplayMsg(MSG_DECLINED);
		return iRet;
	}
	Inter_DisplayMsg(MSG_APPROVED);
	return EMV_OK;
}

int ClssCompleteTrans_JCB(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen)
{
	uchar ucCardType, ucOnlineResult;
	int nRet;

   if (aucRspCode[0])
    {
	   nSetDETData((uchar*)"\x8A", 1, aucRspCode, 2);
    }
    if (nIAuthDataLen)
    {
    	nSetDETData((uchar*)"\x89", 1, aucAuthCode, nIAuthDataLen);
    }
	if (ucInOnlineResult == 0x01)
		ucOnlineResult = ONLINE_FAILED;
	else if (ucInOnlineResult == 0x00)
		ucOnlineResult = ONLINE_APPROVE;
	else if (!memcmp(aucRspCode, "89", 2))
		ucOnlineResult = ONLINE_ABORT;
	else
		ucOnlineResult = ONLINE_DENIAL;
    if (ucOnlineResult == ONLINE_FAILED)
    {
    	nSetDETData((uchar*)"\x8A", 1, (uchar *)"Z1", 2);
        return CLSS_DECLINE;
    }

	if (nIAuthDataLen == 0 && nScriptLen == 0)
	{
		return EMV_NO_DATA;
	}

	ucCardType = ucAppGetTransPath();

	if (ucCardType == CLSS_JCB_EMV)
	{
        nRet = Clss_IssuerUpdateProc_JCB(aucScript, &nScriptLen);
        if(nRet)
        {
            appRemovePicc();
            return nRet;
        }
	}
	appRemovePicc();
	return EMV_OK;
}

int ClssCompleteTrans_DPAS(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen)
{
	uchar ucCardType, ucOnlineResult;
	int nRet;

   if (aucRspCode[0])
    {
	   nSetDETData((uchar*)"\x8A", 1, aucRspCode, 2);
    }
    if (nIAuthDataLen)
    {
    	nSetDETData((uchar*)"\x89", 1, aucAuthCode, nIAuthDataLen);
    }
	if (ucInOnlineResult == 0x01)
		ucOnlineResult = ONLINE_FAILED;
	else if (ucInOnlineResult == 0x00)
		ucOnlineResult = ONLINE_APPROVE;
	else if (!memcmp(aucRspCode, "89", 2))
		ucOnlineResult = ONLINE_ABORT;
	else
		ucOnlineResult = ONLINE_DENIAL;
    if (ucOnlineResult == ONLINE_FAILED)
    {
    	nSetDETData((uchar*)"\x8A", 1, (uchar *)"Z1", 2);
        return CLSS_DECLINE;
    }

	if (nIAuthDataLen == 0 && nScriptLen == 0)
	{
		return EMV_NO_DATA;
	}

	ucCardType = ucAppGetTransPath();

	if (ucCardType == CLSS_DPAS_EMV)
	{
        nRet = Clss_IssuerUpdateProc_DPAS(ucOnlineResult, aucScript, &nScriptLen);
        if(nRet)
        {
            appRemovePicc();
            return nRet;
        }
	}
	appRemovePicc();
	return EMV_OK;
}

int ClssCompleteTrans_AE(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen)
{
	unsigned char ucOnlineResult;
	unsigned char ucOnlineMode;
	ONLINE_PARAM ptOnlineParam;
	unsigned char pucAdviceFlag;
	uchar aucTmTransCapa[4];
	int iLen, nRet;

	if (nIAuthDataLen == 0 && nScriptLen == 0)
	{
		return EMV_NO_DATA;
	}

	if (ucInOnlineResult == 0x01)
		ucOnlineResult = ONLINE_FAILED;
	else if (ucInOnlineResult == 0x00)
		ucOnlineResult = ONLINE_APPROVE;
	else if (!memcmp(aucRspCode, "89", 2))
		ucOnlineResult = ONLINE_ABORT;
	else
		ucOnlineResult = ONLINE_DENIAL;

	ucOnlineMode = AE_PARTIALONLINE;
	Clss_GetTLVData_AE(0x9F6E, aucTmTransCapa, &iLen);
	if ((aucTmTransCapa[0] & 0x20) != 0)//full online supported
	{
		if (gl_ucRemovalTimeup == 1)
		{
			//鎻愮ず绉诲崱
			ucOnlineMode = AE_PARTIALONLINE;
		}
		else
		{
			ucOnlineMode = AE_FULLONLINE;
		}
	}
	else//full online not supported
	{
		ucOnlineMode = AE_PARTIALONLINE;
	}

	memset(&ptOnlineParam, 0, sizeof(ONLINE_PARAM));
	memcpy(ptOnlineParam.aucRspCode, aucRspCode, 2);
	ptOnlineParam.nAuthCodeLen = strlen((char *)aucAuthCode);
	memcpy(ptOnlineParam.aucAuthCode, aucAuthCode, 6);
	memcpy(ptOnlineParam.aucIAuthData, aucIAuthData, nIAuthDataLen);
	ptOnlineParam.nIAuthDataLen = nIAuthDataLen;
	memcpy(ptOnlineParam.aucScript, aucScript, nScriptLen);
	ptOnlineParam.nIAuthDataLen = nScriptLen;

	nRet = Clss_CompleteTrans_AE(ucOnlineResult,ucOnlineMode, &ptOnlineParam,&pucAdviceFlag);
	if(nRet)
	{
	    appRemovePicc();
	    return nRet;
	}
	appRemovePicc();
	return EMV_OK;
}

int ClssCompleteTrans_WAVE(uchar ucInOnlineResult, uchar aucRspCode[], uchar aucAuthCode[], uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen)
{
	uchar aucBuff[256] = {0};
	uchar aucCTQ[32] = {0};
	int iCTQLen = 0, iLen = 0, iRet = 0;
	uchar ucKernType = 0;

	if (nIAuthDataLen == 0 && nScriptLen == 0)
	{
		return EMV_NO_DATA;
	}

	if (ucInOnlineResult != 0x00)
		return -1;

	memset(aucCTQ, 0, sizeof(aucCTQ));
	if ((glClss_PreProcInfoIn.aucReaderTTQ[2] & 0x80) == 0x80
		&& Clss_GetTLVData_Wave(0x9F6C, aucCTQ, &iCTQLen) == 0
		&& (aucCTQ[1] & 0x40) == 0x40)
	{
        memset(aucBuff, 0, sizeof(aucBuff));
        iRet = Clss_FinalSelect_Entry(&ucKernType, aucBuff, &iLen);
        if (iRet != 0)
        {
//				continue;
            appRemovePicc();
            return iRet;
        }

        if (ucKernType != KERNTYPE_VIS)
        {
            appRemovePicc();
            return -1;
        }
		

        iRet = Clss_IssuerAuth_Wave(aucIAuthData, nIAuthDataLen);
        if(iRet)
        {
            appRemovePicc();
            return iRet;
        }
        iRet = Clss_IssScriptProc_Wave(aucScript, nScriptLen);
        if(iRet)
        {
            appRemovePicc();
            return iRet;
        }
        appRemovePicc();
	}
	return EMV_OK;
}

int ClssTransInit()
{
	int i, iRet;
	EMV_APPLIST EMV_APP;

	Clss_DelAllAidList_Entry();
	Clss_DelAllPreProcInfo();

	memset(glCAPKeyList, 0, sizeof(glCAPKeyList));
	//read all CAPK, save into memory added by kevinliu
	for(i=0; i<MAX_KEY_NUM; i++)
	{
		iRet = EMVGetCAPK(i, &glCAPKeyList[i]);
	}

	for (i=0; i<MAX_APP_NUM; i++)
	{
		memset(&EMV_APP, 0, sizeof(EMV_APPLIST));
		iRet = EMVGetApp(i, &EMV_APP);
		//		OsLog(LOG_INFO, "Load AID APP[%d] = %s", i, EMV_APP.AppName);
		//end modified by Kevinliu
		if(iRet != EMV_OK)
		{
			continue;
		}
		
		iRet = Clss_AddAidList_Entry(EMV_APP.AID, EMV_APP.AidLen, EMV_APP.SelFlag, KERNTYPE_DEF);
		if(iRet != EMV_OK) 
		{
			continue;
		}

		memset(&glClss_PreProcInfoIn, 0, sizeof(Clss_PreProcInfo));
		glClss_PreProcInfoIn.ulTermFLmt = 5000;	//Terminal Offline limit //5000 before
		glClss_PreProcInfoIn.ulRdClssTxnLmt = 100000;	//Reader contactless transaction limit
		glClss_PreProcInfoIn.ulRdCVMLmt = 3000;	//Reader CVM limit
		glClss_PreProcInfoIn.ulRdClssFLmt = 5000;	//Reader contactless Offline limit //5000 before

		memcpy(glClss_PreProcInfoIn.aucAID, EMV_APP.AID, EMV_APP.AidLen);
		glClss_PreProcInfoIn.ucAidLen = EMV_APP.AidLen;

		glClss_PreProcInfoIn.ucKernType = KERNTYPE_DEF;

		glClss_PreProcInfoIn.ucCrypto17Flg = 1;
		glClss_PreProcInfoIn.ucZeroAmtNoAllowed = 0;
		glClss_PreProcInfoIn.ucStatusCheckFlg = 0;
		memcpy(glClss_PreProcInfoIn.aucReaderTTQ, "\x36\x00\x80\x00", 4);//36:onlin pin & signature;  Qvsdc38:34 online pin;  Qvsdc39:32 signature;

		glClss_PreProcInfoIn.ucTermFLmtFlg = 1;
		glClss_PreProcInfoIn.ucRdClssTxnLmtFlg = 1;
		glClss_PreProcInfoIn.ucRdCVMLmtFlg = 1;
		glClss_PreProcInfoIn.ucRdClssFLmtFlg=1;
		Clss_SetPreProcInfo_Entry(&glClss_PreProcInfoIn);
	}
	return 0;
}

int nSetDETData(uchar *pucTag, uchar ucTagLen, uchar *pucData, uchar ucDataLen)
{
	int iRet = 0;
	uchar aucBuff[256] = "",ucBuffLen = 0;

	if(pucTag == NULL || pucData == NULL)
	{
		return CLSS_PARAM_ERR;
	}
	memset(aucBuff, 0, sizeof(aucBuff));
	memcpy(aucBuff, pucTag, ucTagLen);
	ucBuffLen = ucTagLen;
	aucBuff[ucBuffLen++] = ucDataLen;
	memcpy(aucBuff+ucBuffLen, pucData, ucDataLen);
	ucBuffLen += ucDataLen;
	if(ucAppGetAppType() == KERNTYPE_MC)
	{
		iRet = Clss_SetTLVDataList_MC(aucBuff, ucBuffLen);
//		OsLog(LOG_INFO, "Clss_SetTLVDataList_MC ret = %d", iRet);
	}
	else if(ucAppGetAppType() == KERNTYPE_JCB)
	{
		iRet = Clss_SetTLVDataList_JCB(aucBuff, ucBuffLen);
//		OsLog(LOG_INFO, "Clss_SetTLVDataList_JCB ret = %d", iRet);
	}
	else if(ucAppGetAppType() == KERNTYPE_ZIP)
	{
		iRet = Clss_SetTLVDataList_DPAS(aucBuff, ucBuffLen);
//		OsLog(LOG_INFO, "Clss_SetTLVDataList_DPAS ret = %d", iRet);
	}
	return iRet;
}

//int Clss_GetTLVData(unsigned short tag, uchar *data, int *datalen, uchar flag)			//modified by kevinliu 2015/10/21
int Clss_GetTLVData(unsigned int tag, uchar *data, int *datalen, uchar flag)
{
	int iRet = 0, iLen = 0;
	uchar ucTagList[3] = {0};
	uchar ucTagListLen = 0;
	uchar ucDataOut[100] = {0};
	uint uiActualDataOutLen = 0;

	if(flag == KERNTYPE_VIS)
	{
		//ShowLogs(1, "VISA PAY WAVE");
		iRet = Clss_GetTLVData_Wave(tag, data, &iLen);
	}
	//TODO data size
	else if((flag == KERNTYPE_MC) || (flag == KERNTYPE_JCB) || (flag == KERNTYPE_ZIP))
	{
		if(tag < 0xFF){
			ucTagListLen = 1;
		}else if((tag > 0xFF) && (tag < 0xFFFF)){
			ucTagListLen = 2;
		}else{
			ucTagListLen = 3;
		}
		memset(ucTagList, 0 ,sizeof(ucTagList));
		PubLong2Char(tag, ucTagListLen, ucTagList);
		if(flag == KERNTYPE_MC) {
			iRet = Clss_GetTLVDataList_MC(ucTagList, ucTagListLen,
					sizeof(ucDataOut), ucDataOut, &uiActualDataOutLen);
		} else if(flag == KERNTYPE_JCB) {
			iRet = Clss_GetTLVDataList_JCB(ucTagList, ucTagListLen,
					sizeof(ucDataOut), ucDataOut, &uiActualDataOutLen);
		} else if(flag == KERNTYPE_ZIP) {
			iRet = Clss_GetTLVDataList_DPAS(ucTagList, ucTagListLen,
					sizeof(ucDataOut), ucDataOut, &uiActualDataOutLen);
		}
		if(RET_OK == iRet)
		{
			memcpy(data, ucDataOut, uiActualDataOutLen);
			iLen = uiActualDataOutLen;
		}
	}
	else if(flag == KERNTYPE_AE)
	{
		iRet = Clss_GetTLVData_AE(tag,data,&iLen);
	}
	*datalen = iLen;

	return iRet;
}

//int Clss_SetTLVData(unsigned short tag,uchar *data,int datalen,uchar flag)		modified by kevinliu 2015/10/21
int Clss_SetTLVData(unsigned int tag, uchar *data, int datalen, uchar flag)
{
	int iRet = 0;
	uchar ucTagList[3] = {0};
	uchar ucTagListLen = 0;

	if(flag == KERNTYPE_VIS)
	{
		iRet = Clss_SetTLVData_Wave(tag,data,datalen);
	}
	else if(flag == KERNTYPE_AE)
	{
		iRet = Clss_SetTLVData_AE(tag,data,datalen);
	}
	else if((flag == KERNTYPE_MC) || (flag == KERNTYPE_JCB) || (flag == KERNTYPE_ZIP))
	{
		if(tag < 0xFF) {
			ucTagListLen = 1;
		}else if((tag > 0xFF) && (tag < 0xFFFF)) {
			ucTagListLen = 2;
		}else{
			ucTagListLen = 3;
		}
		memset(ucTagList, 0 ,sizeof(ucTagList));
		PubLong2Char(tag, ucTagListLen, ucTagList);
		iRet = nSetDETData(ucTagList, ucTagListLen, data, datalen);
	}
	return iRet;
}

void SetEntryModeCls(const TRAN_LOG *pstLog)
{
	sprintf((char *)glSendPack.szEntryMode, "0000");

	if( ChkIfAmex() )
	{
		//ShowLogs(1, "Inside SetEntryMode 1");
		SetAmexEntryModeCls(pstLog);
	}
	else
	{
		//ShowLogs(1, "Inside SetEntryMode 2");
		SetStdEntryModeCls(pstLog);
	}
}

void SetAmexEntryModeCls(const TRAN_LOG *pstLog)
{
	glSendPack.szEntryMode[3] = '2';
	if (ChkIfPinReqdAllIssuer())
	{
		glSendPack.szEntryMode[3] = '1';	// pin capable
	}

	#ifdef ENABLE_EMV
		if( ChkAcqOption(ACQ_EMV_FEATURE) )
		{
			EMVGetParameter(&glEmvParam);
			if (glEmvParam.Capability[1] & 0x40)
			{
				glSendPack.szEntryMode[3] = '1';	// pin capable
			}
			if (glEmvParam.Capability[1] & 0x90)
			{
				glSendPack.szEntryMode[3] = '3';	// offline pin capable
			}
		}
	#endif

	#ifdef ENABLE_EMV
		if( ChkAcqOption(ACQ_EMV_FEATURE) )
		{
			glSendPack.szEntryMode[1] = '5';
		}
	#endif

	if( pstLog->uiEntryMode & MODE_SWIPE_INPUT )
	{
		if (glProcInfo.stTranLog.uiEntryMode & MODE_SECURITYCODE)
		{
			glSendPack.szEntryMode[2] = '6';
		}
		else
		{
			glSendPack.szEntryMode[2] = '2';
		}
	}
	#ifdef ENABLE_EMV
		else if( pstLog->uiEntryMode & MODE_CHIP_INPUT )
		{
			glSendPack.szEntryMode[2] = '5';
		}
		else if( pstLog->uiEntryMode & MODE_FALLBACK_SWIPE )
		{
			glSendPack.szEntryMode[1] = '6';
			glSendPack.szEntryMode[2] = (glProcInfo.szSecurityCode[0]!=0) ? '6' : '2';
		}
		else if( pstLog->uiEntryMode & MODE_FALLBACK_MANUAL )
		{
			// ????;
		}
	#endif
	else if( pstLog->uiEntryMode & MODE_MANUAL_INPUT )
	{
		glSendPack.szEntryMode[2] = (glProcInfo.szSecurityCode[0]!=0) ? '7' : '1';
	}
}

void SetStdEntryModeCls(const TRAN_LOG *pstLog)
{
	//------------------------------------------------------------------------------
	// Entry mode digit 1
	#ifdef ENABLE_EMV
		if( ChkAcqOption(ACQ_EMV_FEATURE) )
		{
			//ShowLogs(1, "Inside SetStdEntryMode 1");
			if( ChkIfBoc() || ChkIfBea() )
			{
				//ShowLogs(1, "Inside SetStdEntryMode 1a");
				glSendPack.szEntryMode[0] = '5';
			}
		}
	#endif

	//------------------------------------------------------------------------------
	// Entry mode digit 2 and digit 3
	if( pstLog->uiEntryMode & MODE_MANUAL_INPUT )
	{
		//ShowLogs(1, "Inside SetStdEntryMode 2");
		memcpy(&glSendPack.szEntryMode[1], "01", 2);
	}
	else if( pstLog->uiEntryMode & MODE_SWIPE_INPUT )
	{
		//ShowLogs(1, "Inside SetStdEntryMode 3");
		memcpy(&glSendPack.szEntryMode[1], "02", 2);
	}
	else if( pstLog->uiEntryMode & MODE_CHIP_INPUT )
	{
		//ShowLogs(1, "Inside SetStdEntryMode 4");
		memcpy(&glSendPack.szEntryMode[1], "05", 2);
	}
	else if( pstLog->uiEntryMode & MODE_FALLBACK_SWIPE )
	{
		//ShowLogs(1, "Inside SetStdEntryMode 5");
		memcpy(&glSendPack.szEntryMode[1], "80", 2);

		// sort by banks (acquirer)
		if( ChkIfFubon() )
		{
			//ShowLogs(1, "Inside SetStdEntryMode 6");
			glSendPack.szEntryMode[2] = '1';
		}
		else if( ChkIfBoc())
		{
			//ShowLogs(1, "Inside SetStdEntryMode 7");
			if( pstLog->szPan[0]=='4' )
			{
				//ShowLogs(1, "Inside SetStdEntryMode 7a");
				memcpy(&glSendPack.szEntryMode[1], "90", 2);
			}
			else if( pstLog->szPan[0]=='5' )
			{
				//ShowLogs(1, "Inside SetStdEntryMode 7b");
				memcpy(&glSendPack.szEntryMode[1], "80", 2);
			}
			else if ( memcmp(pstLog->szPan, "35", 2)==0 )
			{
				//ShowLogs(1, "Inside SetStdEntryMode 7c");
				memcpy(&glSendPack.szEntryMode[1], "97", 2);	// "971"
			}
		}
	}
	else if( pstLog->uiEntryMode & MODE_FALLBACK_MANUAL )
	{
		//ShowLogs(1, "Inside SetStdEntryMode 8");
	}

	//------------------------------------------------------------------------------
	// Entry mode digit 4
	#ifdef ENABLE_EMV
		if( ChkAcqOption(ACQ_EMV_FEATURE) )
		{
			//ShowLogs(1, "Inside SetStdEntryMode 9");
			glSendPack.szEntryMode[3] = '1';    // default : support offline-PIN
		}
	#endif
}

void SetCondCodeCls(void)
{
	if( ChkIfAmex() )
	{
		// condition code==06: Preauth, Auth, SaleComplete, sale below floor
		sprintf((char *)glProcInfo.stTranLog.szCondCode, "00");
		if( (glProcInfo.stTranLog.ucTranType==PREAUTH))
		{
			sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
		}
		if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
		{
			if( !(glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID)) &&
				 (glProcInfo.stTranLog.ucOrgTranType==SALE_COMP || glProcInfo.stTranLog.ucOrgTranType==SALE) )
			{
				sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
			}
		}
	}
	else
	{
		sprintf((char *)glProcInfo.stTranLog.szCondCode, "00");
		if( glProcInfo.stTranLog.ucTranType==PREAUTH )
		{
			sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
		}
		else if( glProcInfo.stTranLog.ucTranType==VOID || glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
		{
			if( glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
			{
				sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
			}
		}
	}

	sprintf((char *)glSendPack.szCondCode, "%.2s", glProcInfo.stTranLog.szCondCode);
}

void SetCommReqFieldCls(void)
{
	char temp[200] = {0};
	int len = 0;
	char icc[250] = {0};
	#ifdef ENABLE_EMV
		int	iLength;
	#endif
	uchar	szTotalAmt[12+1];
	
	
	sprintf((char *)glSendPack.szMsgCode, "%s", "0200");
	ShowLogs(1, "Bitmap: %s", glSendPack.szMsgCode);
	sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN, glProcInfo.stTranLog.szPan);
	ShowLogs(1, "Field 2: %s", glSendPack.szPan);
	sprintf((char *)glSendPack.szProcCode, "%.*s", LEN_PROC_CODE, "000000");
	ShowLogs(1, "Field 3: %s", glSendPack.szProcCode);
	if( !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) )
	{
		PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
		//PubAddHeadChars(szTotalAmt, 12, '0');		no need: already 12 digits
		sprintf((char *)glSendPack.szTranAmt,   "%.*s", LEN_TRAN_AMT,   szTotalAmt);
		sprintf((char *)glSendPack.szExtAmount, "%.*s", LEN_EXT_AMOUNT, glProcInfo.stTranLog.szTipAmount);
		if (ChkIfAmex())
		{
			if( glProcInfo.stTranLog.ucTranType==REFUND )
			{
				glSendPack.szExtAmount[0] = 0;
			}
			if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
			{
				sprintf(glSendPack.szTranAmt, "%012lu", 0L);
				glSendPack.szExtAmount[0] = 0;
			}
		}
	}
	else
	{
		sprintf((char *)glSendPack.szTranAmt, "%.*s", LEN_TRAN_AMT, glProcInfo.stTranLog.szAmount);
		if( ChkIfAmex() )
		{
			if( (glProcInfo.stTranLog.ucTranType==VOID) || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
			{
				sprintf(glSendPack.szTranAmt, "%012lu", 0L);
				glSendPack.szExtAmount[0] = 0;
			}
		}
	}
	ShowLogs(1, "Field 4: %s", glSendPack.szTranAmt);
	glSysCtrl.ulSTAN = useStan;
	sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);
	glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
	ShowLogs(1, "Field 11: %s", glSendPack.szSTAN);
	sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
	ShowLogs(1, "Field 14: %s", glSendPack.szExpDate);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMCC", temp);
	sprintf((char *)glSendPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, temp);
	ShowLogs(1, "Field 18: %s", glSendPack.szMerchantType);
	//SetEntryModeCls(&glProcInfo.stTranLog);		// bit 22, entry mode
	strcpy(glSendPack.szEntryMode, "0071");
	ShowLogs(1, "Field 22: %s", glSendPack.szEntryMode);
	//sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, glProcInfo.stTranLog.ucPanSeqNo);
	ShowLogs(1, "Field 23: %s", glSendPack.szPanSeqNo);
	SetCondCodeCls();
	ShowLogs(1, "Field 25: %s", glSendPack.szCondCode);
	sprintf((char *)glSendPack.szPoscCode, "%.*s", LEN_POSC_CODE, "06");
	ShowLogs(1, "Field 26: %s", glSendPack.szPoscCode);
	sprintf((char *)glSendPack.szTransFee, "%.*s", LEN_TRANS_FEE, "C00000000");
	ShowLogs(1, "Field 28: %s", glSendPack.szTransFee);
	sprintf((char *)glSendPack.szAqurId, "%.*s", LEN_AQUR_ID, "111129");
	ShowLogs(1, "Field 32: %s", glSendPack.szAqurId);
	sprintf((char *)glSendPack.szTrack2,  "%.*s", LEN_TRACK2,   glProcInfo.szTrack2);
	ShowLogs(1, "Field 35: %s", glSendPack.szTrack2);
	sprintf((char *)glSendPack.szRRN, "000000%s", glSendPack.szSTAN);
	ShowLogs(1, "Field 37: %s", glSendPack.szRRN);
	memset(temp, '\0', strlen(temp));
	strcpy(temp, glSendPack.szTrack2);
	len = strlen(glSendPack.szPan) + 5;
	strncpy(glSendPack.szServResCode, temp + len, 3);
	ShowLogs(1, "Field 40: %s", glSendPack.szServResCode);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	sprintf((char *)glSendPack.szTermID, "%.*s", LEN_TERM_ID, temp);
	ShowLogs(1, "Field 41: %s", glSendPack.szTermID);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, temp);
	ShowLogs(1, "Field 42: %s", glSendPack.szMerchantID);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMNL", temp);
	sprintf((char *)glSendPack.szMNL, "%.*s", LEN_MNL, temp);
	ShowLogs(1, "Field 43: %s", glSendPack.szMNL);
	memset(temp, '\0', strlen(temp));
	UtilGetEnv("curcode", temp);
	sprintf((char *)glSendPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, temp);
	ShowLogs(1, "Field 49: %s", glSendPack.szTranCurcyCode);
	HexEnCodeMethod(glProcInfo.sPinBlock, 8, glSendPack.szPinBlock);
	ShowLogs(1, "Field 52: %s", glSendPack.szPinBlock);

	//sprintf((char *)glSendPack.szTEchoData, "%.*s", LEN_TRANSECHO_DATA, "Reconciler>GENERIC&Option>000");
	//ShowLogs(1, "Field 59: %s", glSendPack.szTEchoData);
	sprintf((char *)glSendPack.szPosDataCode, "%.*s", LEN_POS_CODE, "A10101711344101");//A10101711344101
	ShowLogs(1, "Field 123: %s", glSendPack.szPosDataCode);
}

int Clss_transmit(uchar kerId)
{
	int	iRet = 0, iLength, iRetryPIN = 0, iRev = 0;
	int len = 0;
	ulong	ulICCDataLen = 0;
	uchar	*psICCData = NULL, *psTemp = NULL;
	char iccData[500] = {0};
	uchar	szTotalAmt[12+1], sTemp[6];
	char lasthost[128] = {0};
	char verve[128] = {0};
	
	Clss_SetTLVData(0x9f27,"\x80",1,kerId);
	SetCommReqFieldCls();	

	ShowLogs(1, "About packing Field 55");
	iRet = SetClSSDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
	ShowLogs(1, "Done packing Field 55");
	if( iRet!=0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}
	ShowLogs(1, "Continue after packing Field 55");
	PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
	memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
	PubBcd2Asc(glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen, iccData);
	ShowLogs(1, "ICC DATA: %s", glSendPack.testSICCData);

	/*
	// prepare online DE55 data
	ShowLogs(1, "About packing Field 55");
	iRet = SetClSSDE55(FALSE, &glSendPack.sICCData[2], &iLength);
	ShowLogs(1, "Done packing Field 55");
	if( iRet!=0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}
	ShowLogs(1, "Continue after packing Field 55");
	PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
	memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
	PubBcd2Asc(glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen, iccData);
	ShowLogs(1, "ICC DATA: %s", iccData);

	//Commented out by Wisdom
	/*
	PubASSERT( len<LEN_ICC_DATA );
	PubLong2Char((ulong)len, 2, glSendPack.sICCData);
	memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], len);	// save for batch upload
	glProcInfo.stTranLog.uiIccDataLen = (ushort)len;
	
	iRet = TranReversal();
	if( iRet!=0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	//		return iRet;
	}*/
	
	// Some banks may want TC to be sent prior to sale
	
	iRetryPIN = 0;
	while( 1 )
	{
		if (ChkIfAmex() || ChkCurAcqName("AMEX", FALSE))
		{
			GetNewInvoiceNo();
		}
		
		/*iRet = SendRecvPacket();
		if( iRet!=0 )
		{
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			break;
		}*/
		
		iRet = SendEmvData(iccData, &iRev);

		if(iRet != 0)
		{
			ShowLogs(1, "Failed. SendEmvData Returned: %d", iRet);
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			//return ONLINE_FAILED;//Commented out by Wisdom
		}else
		{
			ShowLogs(1, "Response came from host");
			break;
		} 
		
		if(iRev == 1)
		{
			ShowLogs(1, "Sending Reversal. Value: %d", iRev);
			DisplayInfoNone("", "No Response", 3);
			
			iRet = SendReversal();
			if( iRet!=0 )
			{
				DisplayInfoNone("", "Failed", 2);
				glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
				return ONLINE_FAILED;
			}else
			{
				return 0;
			}
			
		}
		
		if( memcmp(glRecvPack.szRspCode, "55", 2)!=0 || ++iRetryPIN>3 || !ChkIfNeedPIN() )
		{
			break;
		}
		
		// 重新输入PIN
		iRet = GetPIN(GETPIN_RETRY);
		if( iRet!=0 )
		{
			return iRet;
		}
		sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);
		memcpy(&glSendPack.sPINData[0], "\x00\x08", 2);
		memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, 8);
	}

	glProcInfo.ucOnlineStatus = ST_ONLINE_APPV;
	
	if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE)!=0 )
	{
		return ERR_TRAN_FAIL;
	}
	
	ulICCDataLen = PubChar2Long(glRecvPack.sICCData, 2);
	//	PubChar2Long(glRecvPack.sICCData, 2, &ulICCDataLen);
	psICCData = &glRecvPack.sICCData[2];
	if (ulICCDataLen != 0)
	{
		IssScrCon();
	}

	if( (glProcInfo.ucOnlineStatus == ST_ONLINE_APPV) && (memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)==0) && (iRet == 0))
	{
		//Commented out by Wisdom
		/*
		// update for reversal(maybe have script result)
		SetClSSDE55(FALSE, &glSendPack.sICCData[2], &len);
		PubLong2Char((ulong)len, 2, glSendPack.sICCData);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)len;
		SaveRevInfo(TRUE);	// update reversal information
		*/
	}
	//get Issuer Authentication Data and Issuer script
	for(psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
	{
		iRet = GetTLVItem(&psTemp, psICCData+ulICCDataLen-psTemp, SaveRspICCData, FALSE);
	}
	
	// 交易失败处理
	if (iRet)
	{
		SaveRevInfo(FALSE);
		DispResult(iRet);
		return ERR_NO_DISP;
	}	
	if (memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
	{
		SaveRevInfo(FALSE);
		DispResult(ERR_HOST_REJ);
		return ERR_NO_DISP;
	}
	//removed by Kevin Liu 2016014 bug828
	//	DoE_Signature();

	if( ChkIfSaveLog() )
	{
		SaveTranLog(&glProcInfo.stTranLog);
	}
	if( ChkIfPrnReceipt() )
    {
        if( glProcInfo.stTranLog.ucTranType!=VOID )
        {
            GetNewInvoiceNo();
        }
	}
	SaveRevInfo(FALSE);
	
	
	if( glProcInfo.stTranLog.ucTranType!=RATE_BOC &&
        glProcInfo.stTranLog.ucTranType!=RATE_SCB &&
        glProcInfo.stTranLog.ucTranType!=LOAD_CARD_BIN )
    {
		if(ChkIfCiti())
		{
			//Commented out by Wisdom
			//OfflineSend(OFFSEND_TRAN);
		}
		else
		{
			//Commented out by Wisdom
			//OfflineSend(OFFSEND_TC | OFFSEND_TRAN);
		}
    }
	
	CommOnHook(FALSE);
	
	//Wisdom
	if(ChkIfPrnReceipt())
	//if(1)
	{
		//PrintReceipt(PRN_NORMAL);
		PrintAllReceipt(PRN_NORMAL);
	}
	DispResult(0);	
	return 0;
}


int IssScrCon(void)
{
	uchar mode=0,outActype[256],key, *sScriptData;
	int time,len, iLen;
	int ret;
	//iss_scrstrc iss_scrs;
	APDU_SEND send_com;
	APDU_RESP recv_com;

	Gui_ClearScr();
	Gui_ShowMsgBox("SALE", gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	while(1)
	{
		if(PiccOpen() != 0)
		{
			return ERR_TRAN_FAIL;
		}
// 		PiccSetup('R',&PiccPara);
// 		PiccPara.wait_retry_limit_w = 1;
// 		PiccPara.wait_retry_limit_val = 0;
// 		PiccPara.card_buffer_w = 1;
// 		PiccPara.card_buffer_val = 64;
// 		PiccSetup('W',&PiccPara);
		TimerSet(3,500);
		kbflush();
		while(1)
		{
			time = TimerCheck(3);
			if(!time)
			{
				PiccLight(PICC_LED_ALL, 0);
				PiccClose();
				return ERR_TRAN_FAIL;
			}
			if(kbhit() != NOKEY)
			{
				key = getkey();
				PiccLight(PICC_LED_ALL, 0);
				PiccClose();
				if(key == KEYCANCEL) return ERR_TRAN_FAIL;
			}

			ret = PiccDetect(mode,NULL,NULL,NULL,NULL);
			if(ret == 0) break;
			if(ret == 1 ||ret==2) return ERR_TRAN_FAIL;
			if(ret == 3||ret==5||ret==6)
			{
				PiccLight(PICC_LED_ALL, 0);
				PiccClose();
				DelayMs(100);
				continue;
			}
			if(ret == 4)
			{
				PiccLight(PICC_LED_ALL, 0);
				PiccClose();
				return ERR_TRAN_FAIL;
			}
		}
		break;
	}
	

	memcpy(send_com.Command,"\x00\xA4\x04\x00",4);
	send_com.Lc = 14;
	memcpy(send_com.DataIn,"1PAY.SYS.DDF01",14);
	send_com.Le = 256;
	ret = PiccIsoCommand(0,&send_com,&recv_com);
	if(ret != 0) return ERR_TRAN_FAIL;
	if(recv_com.SWA != 0x90 ||recv_com.SWB != 0x00) return ERR_TRAN_FAIL;
	
	
	memcpy(send_com.Command,"\x00\xA4\x04\x00",4);
	memcpy(send_com.DataIn,glProcInfo.stTranLog.sAID, strlen(glProcInfo.stTranLog.sAID));
	send_com.Le = 256;
	ret = PiccIsoCommand(0,&send_com,&recv_com);
	if(ret != 0) return ERR_TRAN_FAIL;
	if(recv_com.SWA != 0x90 ||recv_com.SWB != 0x00) return ERR_TRAN_FAIL;
	
	iLen = PubChar2Long(glRecvPack.sICCData, 2);
	sScriptData = &glRecvPack.sICCData[2];
	if (ChkIfAmex())
	{
		sScriptData += 6;
	}

	memset(outActype, 0, sizeof(outActype));
	len = 0;
	ret = GetSpecTLVItem(sScriptData, iLen, 0x91, outActype, (ushort *)&len);
	if (ret==0)
	{
		ret = Clss_IssuerAuth_Wave (outActype, len);
	}
	memset(outActype, 0, sizeof(outActype));
	len = 0;
	ret = GetSpecTLVItem(sScriptData, iLen, 0x71, outActype, (ushort *)&len);
	if (ret==0)
	{
		ret = Clss_IssScriptProc_Wave (outActype, len);
	}
	memset(outActype, 0, sizeof(outActype));
	len = 0;
	ret = GetSpecTLVItem(sScriptData, iLen, 0x72, outActype, (ushort *)&len);
	if (ret==0)
	{
		ret = Clss_IssScriptProc_Wave (outActype, len);
	}
	if(ret!= EMV_OK) 
	{
		return ERR_TRAN_FAIL;
	}

	return EMV_OK;
}

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

int GetSpecTLVItem(uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen)
{
	uchar		*psTag, *psTagStr, szBuff[10];
	uint		uiTag, uiLenBytes;
	ulong		lTemp;
	
	// skip null tags
    for (psTag=psTLVString; psTag<psTLVString+iMaxLen; psTag++)
    {
        if ((*psTag!=0x00) && (*psTag!=0xFF))
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
        if ((uiTag & 0x1F)==0x1F)
        {	// have another byte
            uiTag = (uiTag<<8) + *psTag++;
        }
        if (psTag>=psTLVString+iMaxLen)
        {
            return -2;	// specific tag not found
        }
        
        // process length bytes
        if ((*psTag & 0x80)==0x80)
        {
            uiLenBytes = *psTag & 0x7F;
			//atoi
			memset(szBuff, 0, sizeof(szBuff));
			memcpy(szBuff, psTag+1, uiLenBytes);
			lTemp = atoi((char *)szBuff);
            //PubChar2Long(psTag+1, uiLenBytes, &lTemp);
        }
        else
        {
            uiLenBytes = 0;
            lTemp      = *psTag & 0x7F;
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

int IsConstructedTag(uint uiTag)
{
	int		i;
	
	for(i=0; (uiTag&0xFF00) && i<2; i++)
	{
		uiTag >>= 8;
	}
	
	return ((uiTag & 0x20)==0x20);
}

//added by Gillian Chen 2015/9/25
int nAppFindMatchProID(unsigned char *pucProID, int ucProIDLen)
{
	EMV_APPLIST EMV_APP;
	int i, iRet;

	if ( pucProID == NULL)
	{
		return EMV_PARAM_ERR;
	}
	else 
	{
		// modify v1.00.0018  [23/09/2015 chenyy]
		for (i=0; i<MAX_APP_NUM; i++) 
		{
			memset(&EMV_APP, 0, sizeof(EMV_APPLIST));
			iRet = EMVGetApp(i, &EMV_APP);
			if(iRet != EMV_OK)
			{
				continue;
			}		
			iRet = Clss_AddAidList_Entry(EMV_APP.AID, EMV_APP.AidLen, EMV_APP.SelFlag, KERNTYPE_DEF);
			if(iRet != EMV_OK) 
			{
				continue;
			}
			memset(&ptProgInfo, 0, sizeof(Clss_ProgramID));
		 	ptProgInfo.ulTermFLmt = glClss_PreProcInfoIn.ulTermFLmt;
		 	ptProgInfo.ulRdClssTxnLmt = glClss_PreProcInfoIn.ulRdClssTxnLmt;
		 	ptProgInfo.ulRdCVMLmt = glClss_PreProcInfoIn.ulRdCVMLmt;
		 	ptProgInfo.ulRdClssFLmt =  glClss_PreProcInfoIn.ulRdClssFLmt;	
			memcpy(ptProgInfo.aucProgramId, pucProID, 17);
			ptProgInfo.ucPrgramIdLen = ucProIDLen;
		 	ptProgInfo.ucAmtZeroNoAllowed = 0;
		 	ptProgInfo.ucStatusCheckFlg = glClss_PreProcInfoIn.ucStatusCheckFlg;
		 	ptProgInfo.ucTermFLmtFlg = glClss_PreProcInfoIn.ucTermFLmtFlg;
		 	ptProgInfo.ucRdClssTxnLmtFlg = glClss_PreProcInfoIn.ucRdClssTxnLmtFlg;
		 	ptProgInfo.ucRdCVMLmtFlg = glClss_PreProcInfoIn.ucRdCVMLmtFlg;
		 	ptProgInfo.ucRdClssFLmtFlg = glClss_PreProcInfoIn.ucRdClssFLmtFlg;
		}
		return EMV_OK;
	}	
}


#else
int TransClssSale(void)
{
	return 0;
}
#endif

