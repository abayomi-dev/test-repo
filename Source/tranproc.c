
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void PackInvoice(STISO8583 *pstSendPack, ulong ulInvoice);
static void ModifyProcessCode(void);
static void SetEntryMode(const TRAN_LOG *pstLog);
static void SetAmexEntryMode(const TRAN_LOG *pstLog);
static void SetStdEntryMode(const TRAN_LOG *pstLog);
static void SetCondCode(void);
static void SetInstAndAirTicketData(void);
static int  TransCashSub(void);
static int  TransSaleSub(void);
int menuFlag = 0;

int PURCHASETYPE;
char PAYATT_PHONENO[50];
char PAYATT_DE62[50];



char AgentPOSID[30];

char Cel_TinAccountNumber[12]={0};
char Cel_TinAccountPin[12]={0};
char Cel_TinBankName[22]={0};
char Cel_TinBankCode[12]={0};
char Cel_BenAccountNumber[22]={0};
char Cel_BenAmunt[22]={0};
char Cel_AccountName[50]={0};
char Cel_TransactionId[22]={0};
char Cel_TimeStamp[22]={0};
char Cel_WalletId[36]={0};
char Cel_PlatformID[10]={0};

char chDisplayFlag = 0;
char Cel_AccountRef[50];


char Cel_Token[90]={0};
char Cel_Fname[20]={0};
char Cel_PasswordId[36]={0};
char Cel_Lname[20]={0};
char Cel_AgentID[15]={0};

//eedc variables
char eedc_transaction_reference[20];
char eedc_units[20];
char eedc_appliedToArrears[20];
char eedc_token[90];
char eedc_vat[6];
char eedc_customerName[40];
char eedc_convenience[10];
char eedc_phone[10];
char eedc_total [10];
//END EEDC 

//JAMB
int JAMB_CODE[15]={0};
int JAMB_CUS_ID[15]={0};
char jamb_customerName[40];
/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void PackInvoice(STISO8583 *pstSendPack, ulong ulInvoice)
{
	memcpy(pstSendPack->sField62, "\x00\x06", 2);
	sprintf((char *)&(pstSendPack->sField62[2]), "%06lu", ulInvoice);
}

void parseTrack2(char *data, int *len)
{
	int i;
	for(i = 0; i < strlen(data); i++)
	{
		if((data[i] == 'D') || data[i] == 'F' || data[i] == '=')
			break;
	}
	len = i;
}
// ����������Ĺ���bits
// set bit content of ISO8583.
// Work as the "IsoInitSendMsg()" did in old style.......
void SetCommReqField(void)
{
	ShowLogs(1, "PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
	char temp[200] = {0};
	char typ[7] = {0};
	int len = 0;
	char icc[250] = {0};
	#ifdef ENABLE_EMV
		int	iLength;
	#endif
	uchar	szTotalAmt[12+1];
ShowLogs(1, "PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
	
	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		ShowLogs(1, " 2 PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
		memset(&glTMSSend, 0, sizeof(STTMS8583));
		sprintf((char *)glTMSSend.szMsgCode, "%.*s", LEN_MSG_CODE,
				glTranConfig[glProcInfo.stTranLog.ucTranType].szTxMsgID);
		sprintf((char *)glTMSSend.szProcCode,   "%.*s", LEN_PROC_CODE,
				glTranConfig[glProcInfo.stTranLog.ucTranType].szProcCode);
		sprintf((char *)glTMSSend.szNii,    "%.*s", LEN_NII,     glCurAcq.szNii);
		sprintf((char *)glTMSSend.szTermID, "%.*s", LEN_TERM_ID, glCurAcq.szTermID);
		sprintf((char *)glTMSSend.szSTAN, "%06lu", glSysCtrl.ulSTAN);
		ShowLogs(1, " 2 PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
		return;
	}
ShowLogs(1, " 2 PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
	//ISO8583ʹ��˵��
	//Step3: ���������ô��ʱ��Ҫ�����ݣ�����ֵ���ͣ�Ϊ��ʱ����(Ĭ�ϵĳ�ʼֵȫΪ��)
	//���еı��Ķ���Ҫ�õ��������//set common bits�½���ͳһ��ֵ
	//�������ݽ��ײ�ͬ������ͬʱ���Ϳɲ��͵ķֱ���
	//��ע��sz����s�Ϳ�ͷ��Ա����,sz��ֱ�Ӹ�����Ҫ���͵�ֵ��s����Ҫ��ǰ��λ��ֵΪ����ĳ���
	//����:	// bit 62, ROC/SOC����Ϊ  sField62
	//���ȸ�����ֵmemcpy(glSendPack.sField62, "\x00\x06", 2);
	//�ٸ�����ֵsprintf((char *)&glSendPack.sField62[2], "%06lu", glProcInfo.stTranLog.ulInvoiceNo);

	// Usage of ISO8583 module (For step 2, see in st8583.h)
	// Step 3: To set the data need in packing here.
	// If the glSendPack.xxxxx is filled with value, this bit will be sent, and vice versa.

	// Note that there're 2 kinds of member in glSendPack:
	//   glSendPack.szxxxxxx : this type means "string end with zero", the actual length can be determined by strlen()
	//   glSendPack.sxxxxxx  : the first 2 bytes contains length infomation, if length=0x01A0, should be "\x01\xA0"
	//     e.g. : bit62
	//             memcpy(glSendPack.sField62, "\x00\x06", 2);
	//             sprintf((char *)&glSendPack.sField62[2], "%06lu", glProcInfo.stTranLog.ulInvoiceNo);

	// set common bits
	//By Wisdom. Form based on transaction type
	memset(&glSendPack, 0, sizeof(STISO8583));

	if(txnType == 1 || txnType == 13)//For txn type. 1 == Purchase
	{
		//For Purchase
		if(magstp == 1)
		{
			sprintf((char *)glSendPack.szPosDataCode, "%.*s", LEN_POS_CODE, "510101210244101");
			ShowLogs(1, "Pos Data Code: %s", glSendPack.szPosDataCode);
			magstp = 0;
		}else
		{
			sprintf((char *)glSendPack.szPosDataCode, "%.*s", LEN_POS_CODE, "510101511344101");
			////ShowLogs(1, "Pos Data Code: %s", glSendPack.szPosDataCode);
		}
		sprintf((char *)glSendPack.szMsgCode, "%s", "0200");
		sprintf((char *)glSendPack.szPoscCode, "%.*s", LEN_POSC_CODE, "06");
		////ShowLogs(1, "Pos Pin Capture Code: %s", glSendPack.szPoscCode);
		sprintf((char *)glSendPack.szTransFee, "%.*s", LEN_TRANS_FEE, "C00000000");
		////ShowLogs(1, "Transaction Amount: %s", glSendPack.szTransFee);
		sprintf((char *)glSendPack.szAqurId, "%.*s", LEN_AQUR_ID, "111129");
		////ShowLogs(1, "Aquirer Id: %s", glSendPack.szAqurId);
	}
	
	ShowLogs(1, " 3 PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("actype", temp);
	memset(typ, '\0', strlen(typ));
	strcpy(typ, "00");
	strcat(typ, temp);
	strcat(typ, "00");
	sprintf((char *)glSendPack.szProcCode, "%.*s", LEN_PROC_CODE, typ);

	//sprintf((char *)glSendPack.szMsgCode, "%.*s", LEN_MSG_CODE, glTranConfig[glProcInfo.stTranLog.ucTranType].szTxMsgID);
	//sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, glTranConfig[glProcInfo.stTranLog.ucTranType].szProcCode);
	//ModifyProcessCode();
	////ShowLogs(1, "Mti: %s", glSendPack.szMsgCode);
	////ShowLogs(1, "Proc: %s", glSendPack.szProcCode);

	// modify bit 3, process code
	
	sprintf((char *)glSendPack.szNii,        "%.*s", LEN_NII,         glCurAcq.szNii);
	//////ShowLogs(1, "Nii: %s", glSendPack.szNii);
	//sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,     glCurAcq.szTermID);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	sprintf((char *)glSendPack.szTermID, "%.*s", LEN_TERM_ID, temp);
	////ShowLogs(1, "Tid: %s", glSendPack.szTermID);

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, temp);
	////ShowLogs(1, "Mid: %s", glSendPack.szMerchantID);

	memset(temp, '\0', strlen(temp));
	UtilGetEnv("curcode", temp);
	ShowLogs(1, "Currency Code: %s", temp);
	sprintf((char *)glSendPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, temp);
	////ShowLogs(1, "Currency Code: %s", glSendPack.szTranCurcyCode);

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMNL", temp);
	sprintf((char *)glSendPack.szMNL, "%.*s", LEN_MNL, temp);
	////ShowLogs(1, "Merchant Name: %s", glSendPack.szMNL);

	//Pinblock
	//HexEnCodeMethod(glProcInfo.sPinBlock, 8, glSendPack.szPinBlock);
	//////ShowLogs(1, "Pinblock: %s", glSendPack.szPinBlock);
	
	//glSysCtrl.ulSTAN++;//By Wisdom
	//GetStan();
	glSysCtrl.ulSTAN = useStan;
	sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);  //??
	glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
	////ShowLogs(1, "Stan: %s", glSendPack.szSTAN);

	if( glProcInfo.stTranLog.ucTranType==SETTLEMENT || glProcInfo.stTranLog.ucTranType==UPLOAD ||
		glProcInfo.stTranLog.ucTranType==LOGON )
	{
		////ShowLogs(1, "Trying to check if its settlement, upload or logon");
		return;
	}

	// bit 4, transaction amount
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
		////ShowLogs(1, "Amount: %s", glSendPack.szTranAmt);
		if( ChkIfAmex() )
		{
			if( (glProcInfo.stTranLog.ucTranType==VOID) || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
			{
				sprintf(glSendPack.szTranAmt, "%012lu", 0L);
				glSendPack.szExtAmount[0] = 0;
			}
		}
	}

	if( (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) ||
		(glProcInfo.stTranLog.ucTranType==TC_SEND) )
	{
		sprintf((char *)glSendPack.szLocalTime, "%.6s",  &glProcInfo.stTranLog.szDateTime[8]);
		sprintf((char *)glSendPack.szLocalDate, "%.4s",  &glProcInfo.stTranLog.szDateTime[4]);
		sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);
		sprintf((char *)glSendPack.szAuthCode,  "%.6s",  glProcInfo.stTranLog.szAuthCode);
	}
	else if( glProcInfo.stTranLog.ucTranType==VOID )
	{
		sprintf((char *)glSendPack.szLocalTime, "%.6s",  &glProcInfo.stTranLog.szDateTime[8]);
		sprintf((char *)glSendPack.szLocalDate, "%.4s",  &glProcInfo.stTranLog.szDateTime[4]);
		sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);       // jiale 2006.12.12
		sprintf((char *)glSendPack.szAuthCode,  "%.6s",  glProcInfo.stTranLog.szAuthCode);	// jiale for void send 37.38field
	}
	else
	{
		sprintf((char *)glSendPack.szLocalTime, "%.6s",  &glProcInfo.stTranLog.szDateTime[8]);
	    sprintf((char *)glSendPack.szLocalDate, "%.4s",  &glProcInfo.stTranLog.szDateTime[4]);
	}

	////ShowLogs(1, "LocalTime: %s", glSendPack.szLocalTime);
	////ShowLogs(1, "LocalDate: %s", glSendPack.szLocalDate);
	//////ShowLogs(1, "RRN: %s", glSendPack.szRRN);
	//////ShowLogs(1, "AuthCode: %s", glSendPack.szAuthCode);

	// PAN/track 1/2/3/expiry etc
	if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		if( (glProcInfo.szTrack2[0]==0) ||
			(glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) ||
			(glProcInfo.stTranLog.ucTranType==TC_SEND) )
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
		}
		else
		{
		    sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
		    //Added by me
		    sprintf((char *)glSendPack.szTrack2,  "%.*s", LEN_TRACK2,   glProcInfo.szTrack2);
		    sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, glProcInfo.stTranLog.ucPanSeqNo);
		}
		if( ChkIfCiti() || ChkIfDah() )
		{
			if( glProcInfo.stTranLog.ucTranType!=SETTLEMENT )
			{
				if( glProcInfo.stTranLog.bPanSeqOK )
				{
					sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, glProcInfo.stTranLog.ucPanSeqNo);
				}
			}
		}
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) )
	{
		if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND || glProcInfo.stTranLog.ucTranType==VOID )
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
		}
		else
		{
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, glProcInfo.szTrack2);
			if ( glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE )
			{
				sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, glProcInfo.szTrack3);
			}
			if ( !ChkIfBoc() )
			{
				sprintf((char *)glSendPack.szTrack1, "%.*s", LEN_TRACK1, glProcInfo.szTrack1);
			}
		}
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
		sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
	}

	////ShowLogs(1, "Pan: %s", glSendPack.szPan);
	////ShowLogs(1, "Exp Date: %s", glSendPack.szExpDate);
	////ShowLogs(1, "Card Sequence Number: %s", glSendPack.szPanSeqNo);
	memset(temp, '\0', strlen(temp));
	strcpy(temp, glSendPack.szTrack2);
	len = strlen(glSendPack.szPan) + 5;
	strncpy(glSendPack.szServResCode, temp + len, 3);
	////ShowLogs(1, "Service Restriction Code: %s", glSendPack.szServResCode);
	////ShowLogs(1, "Track 2: %s", glSendPack.szTrack2);
	
	//Start here	
	SetEntryMode(&glProcInfo.stTranLog);		// bit 22, entry mode
	SetCondCode();		// bit 25, service condition code 

	////ShowLogs(1, "Entry Code: %s", glSendPack.szEntryMode);
	////ShowLogs(1, "Condition Code: %s", glSendPack.szCondCode);

	//Resume here
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMCC", temp);
	sprintf((char *)glSendPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, temp);
	memset(temp, '\0', strlen(temp));
	sprintf(temp, "{\"lastTransactionTime\":\"\",\"%.s\":\"\",\"latitude\":\"\",\"battery level\":\"\"}","1234455");
	//sprintf((char *)glSendPack.szTEchoData, "%s", LEN_TRANSECHO_DATA, temp);
	//ShowLogs(1, "szTEchoData 59: %s", temp);
	//ShowLogs(1, "Mcc: %s", glSendPack.szMerchantType);

	sprintf((char *)glSendPack.szRRN, "000000%s", glSendPack.szSTAN);
	////ShowLogs(1, "RRN: %s", glSendPack.szRRN);

	// bit 48 or 55, CVV2 or 4DBC
	if( ChkIfNeedSecurityCode() && !ChkIfAmex() )
	{
		memcpy(glSendPack.sField48, "\x00\x03", 2);
		sprintf((char *)&glSendPack.sField48[2], "%-3.3s", glProcInfo.szSecurityCode);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT )
	{
		//PubLong2Char((ulong)LEN_PIN_DATA, 2, glSendPack.sPINData);
		//memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, LEN_PIN_DATA);
	}

	SetInstAndAirTicketData();	// bit 48 and 63
	// bit 49
	////ShowLogs(1, "About Packing Tags");
	// process bit 55,56
	if( glProcInfo.stTranLog.ucTranType==AUTH || glProcInfo.stTranLog.ucTranType==PREAUTH ||
		glProcInfo.stTranLog.ucTranType==SALE || glProcInfo.stTranLog.ucTranType==CASH    ||
		glProcInfo.stTranLog.ucTranType==INSTALMENT )
	{
		if( ChkIfAmex() && ChkIfNeedSecurityCode() && (glProcInfo.szSecurityCode[0]!=0) )
		{
			memcpy(glSendPack.sICCData, "\x00\x04", 2);
			sprintf((char *)&glSendPack.sICCData[2], "%-4.4s", glProcInfo.szSecurityCode);
		}
	#ifdef ENABLE_EMV
			else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
				(glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
			{
				////ShowLogs(1, "In Transproc trying to Pack Tags");

				SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
				PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
				memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);
				glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
			}
	#endif
	}

	#ifdef ENABLE_EMV
		if( glProcInfo.stTranLog.ucTranType==VOID && ChkIfFubon() )
		{
			PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
			memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
			PubLong2Char((ulong)glProcInfo.stTranLog.uiField56Len, 2, glSendPack.sICCData2);
			memcpy(&glSendPack.sICCData2[2], glProcInfo.stTranLog.sField56, glProcInfo.stTranLog.uiField56Len);
		}
		if( glProcInfo.stTranLog.ucTranType==VOID && ChkIfHSBC() &&
			(glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) )
		{
			PubLong2Char((ulong)glProcInfo.stTranLog.uiField56Len, 2, glSendPack.sICCData2);
			memcpy(&glSendPack.sICCData2[2], glProcInfo.stTranLog.sField56, glProcInfo.stTranLog.uiField56Len);
		}
		if( ChkIfBea() || ChkIfScb() )
		{
			if( (glProcInfo.stTranLog.ucTranType==VOID) ||
				(glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) ||
				(glProcInfo.stTranLog.ucTranType==TC_SEND) )
			{
				PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
				memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
			}
		}
	#endif

	// bit 62, ROC/SOC
	if(PURCHASETYPE==1){
		//char buf[50]={0};
		ShowLogs(1, " 4 PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
		memset(PAYATT_DE62,0,sizeof(PAYATT_DE62));
		sprintf(PAYATT_DE62,"%s%s","00698WD0101333",PAYATT_PHONENO);
		ShowLogs(1, " 4 PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
		//PackInvoice_62(&glSendPack,buf );
	} else 
	PackInvoice(&glSendPack, glProcInfo.stTranLog.ulInvoiceNo);
	// bit 63
	// ...

	if( ChkIfNeedMac() )
	{
		PubLong2Char((ulong)LEN_MAC, 2, glSendPack.sMac);
	}
	ShowLogs(1, "end PURCHASETYPE transaction >> SendPayattitude SetCommReqField");
}

// Bit 3 definition:
// AMEX 0200:
// For a sale transaction									00 40 0x
// For a refund transaction									20 40 0x
// For an on line void of on line sale						02 40 0x
// For an on line void of on line refund					22 40 0x
// For an on line void of off line sale seen by host		02 40 0x
// For an on line void of off line sale not seen by host	00 40 0x
//
// AMEX 0220:
// For a sale transaction (referred/sale comp or off line sale)
//													00 40 0x
// For an sale adjustment (i.e. add tip or void of sale):
//		When original sale not seen by host			00 40 0x
//		When original sale seen by host				02 40 0x
// For an off line refund transaction				20 40 0x
// For a void refund transaction:
//		When Trickle fed refund to host				22 40 0x
//		When Void off line refund not trickle fed	20 40 0x
void ModifyProcessCode(void)
{
	if( ChkIfAmex() )
	{
		glSendPack.szProcCode[2] = '4';
		if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
		{
			if( glProcInfo.stTranLog.ucOrgTranType==SALE     ||
				glProcInfo.stTranLog.ucOrgTranType==OFF_SALE ||
				glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
			{
				if( glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID) )
				{
					if( glProcInfo.stTranLog.szRRN[0]!=0 )
					{
						glSendPack.szProcCode[1] = '2';
					}
				}
			}
			else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
			{
				glSendPack.szProcCode[0] = '2';
				if( glProcInfo.stTranLog.uiStatus & TS_VOID )
				{
					if( glProcInfo.stTranLog.ucTranType!=SETTLEMENT )
					{	// trickle feed
						glSendPack.szProcCode[1] = '2';
					}
				}
			}
		}
		else if( glProcInfo.stTranLog.ucTranType==VOID )
		{
			if( glProcInfo.stTranLog.ucOrgTranType==SALE )
			{
				glSendPack.szProcCode[1] = '2';
			}
			else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
			{
				glSendPack.szProcCode[0] = '2';
				glSendPack.szProcCode[1] = '2';
			}
			else if( glProcInfo.stTranLog.ucOrgTranType==OFF_SALE ||
					 glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
			{
				if( glProcInfo.stTranLog.szRRN[0]!=0 )
				{
					glSendPack.szProcCode[1] = '2';
				}
			}
		}
	}
	else
	{
		if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
		{
			if( glProcInfo.stTranLog.ucOrgTranType==SALE     ||
				glProcInfo.stTranLog.ucOrgTranType==OFF_SALE ||
				glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
			{
				if( glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID) )
				{
					if( glProcInfo.stTranLog.szRRN[0]!=0 )
					{
						glSendPack.szProcCode[1] = '2';
					}
				}
			}
			else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
			{
				glSendPack.szProcCode[0] = '2';
				if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
				{
					if( glProcInfo.stTranLog.szRRN[0]!=0 )
					{
						glSendPack.szProcCode[1] = '2';
					}
				}
			}
		}
		else if( glProcInfo.stTranLog.ucTranType==VOID )
		{
			if( glProcInfo.stTranLog.ucOrgTranType==SALE )
			{
				glSendPack.szProcCode[1] = '2';
			}
			else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
			{
				glSendPack.szProcCode[0] = '2';
				glSendPack.szProcCode[1] = '2';
			}
		}

        // Processing code 3rd digit
		// ...
	}

    PubStrUpper(glSendPack.szProcCode);
}

void SetEntryMode(const TRAN_LOG *pstLog)
{
	sprintf((char *)glSendPack.szEntryMode, "0000");

	if( ChkIfAmex() )
	{
		//////ShowLogs(1, "Inside SetEntryMode 1");
		SetAmexEntryMode(pstLog);
	}
	else
	{
		//////ShowLogs(1, "Inside SetEntryMode 2");
		SetStdEntryMode(pstLog);
	}
}

void SetAmexEntryMode(const TRAN_LOG *pstLog)
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

void SetStdEntryMode(const TRAN_LOG *pstLog)
{
	//------------------------------------------------------------------------------
	// Entry mode digit 1
	#ifdef ENABLE_EMV
		if( ChkAcqOption(ACQ_EMV_FEATURE) )
		{
			////ShowLogs(1, "Inside SetStdEntryMode 1");
			if( ChkIfBoc() || ChkIfBea() )
			{
				////ShowLogs(1, "Inside SetStdEntryMode 1a");
				glSendPack.szEntryMode[0] = '5';
			}
		}
		////ShowLogs(1, "Inside SetStdEntryMode 1b");
	#endif

	//------------------------------------------------------------------------------
	// Entry mode digit 2 and digit 3
	if( pstLog->uiEntryMode & MODE_MANUAL_INPUT )
	{
		////ShowLogs(1, "Inside SetStdEntryMode 2");
		memcpy(&glSendPack.szEntryMode[1], "01", 2);
	}
	else if( pstLog->uiEntryMode & MODE_SWIPE_INPUT )
	{
		////ShowLogs(1, "Inside SetStdEntryMode 3");
		memcpy(&glSendPack.szEntryMode[1], "02", 2);
	}
	else if( pstLog->uiEntryMode & MODE_CHIP_INPUT )
	{
		////ShowLogs(1, "Inside SetStdEntryMode 4");
		memcpy(&glSendPack.szEntryMode[1], "05", 2);
	}
	else if( pstLog->uiEntryMode & MODE_FALLBACK_SWIPE )
	{
		////ShowLogs(1, "Inside SetStdEntryMode 5");
		memcpy(&glSendPack.szEntryMode[1], "80", 2);

		// sort by banks (acquirer)
		if( ChkIfFubon() )
		{
			////ShowLogs(1, "Inside SetStdEntryMode 6");
			glSendPack.szEntryMode[2] = '1';
		}
		else if( ChkIfBoc())
		{
			////ShowLogs(1, "Inside SetStdEntryMode 7");
			if( pstLog->szPan[0]=='4' )
			{
				////ShowLogs(1, "Inside SetStdEntryMode 7a");
				memcpy(&glSendPack.szEntryMode[1], "90", 2);
			}
			else if( pstLog->szPan[0]=='5' )
			{
				////ShowLogs(1, "Inside SetStdEntryMode 7b");
				memcpy(&glSendPack.szEntryMode[1], "80", 2);
			}
			else if ( memcmp(pstLog->szPan, "35", 2)==0 )
			{
				////ShowLogs(1, "Inside SetStdEntryMode 7c");
				memcpy(&glSendPack.szEntryMode[1], "97", 2);	// "971"
			}
		}
	}
	else if( pstLog->uiEntryMode & MODE_FALLBACK_MANUAL )
	{
		////ShowLogs(1, "Inside SetStdEntryMode 8");
	}

	//------------------------------------------------------------------------------
	// Entry mode digit 4
	#ifdef ENABLE_EMV
		//if( ChkAcqOption(ACQ_EMV_FEATURE) )
		if(1)
		{
			////ShowLogs(1, "Inside SetStdEntryMode 9");
			glSendPack.szEntryMode[3] = '1';    // default : support offline-PIN
		}
		////ShowLogs(1, "Inside SetStdEntryMode 9b");
	#endif
}

// set bit 25
void SetCondCode(void)
{
	if( ChkIfAmex() )
	{
		// condition code==06: Preauth, Auth, SaleComplete, sale below floor
		sprintf((char *)glProcInfo.stTranLog.szCondCode, "00");
		if( (glProcInfo.stTranLog.ucTranType==PREAUTH) || (glProcInfo.stTranLog.ucTranType==AUTH) )
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

// RFU for HK (bit 48, 63)
void SetInstAndAirTicketData(void)
{
	uchar	sBuff[32];

	if (ChkIfBea())
	{
		if (glProcInfo.stTranLog.ucInstalment!=0)
		{
			memcpy(glSendPack.sField63, "\x00\x02", 2);
			PubLong2Bcd((ulong)glProcInfo.stTranLog.ucInstalment, 1, sBuff);
			PubBcd2Asc0(sBuff, 1, glSendPack.sField63+2);
		}
	}
}

void displayName(char *name)
{
	switch(txnType)
	{
		case 1:
			strcpy(name, "PURCHASE");
			break;
		case 2:
			strcpy(name, "CASH ADVANCE");
			break;
		case 3:
			strcpy(name, "PREAUTH");
			break;
		case 4:
			strcpy(name, "REFUND");
			break;
		case 5:
			strcpy(name, "BALANCE ENQUIRY");
			break;
		case 6:
			strcpy(name, "SALES COMPLETION");
			break;
		case 7:
			strcpy(name, "REVERSAL");
			break;
		case 8:
			strcpy(name, "CASH BACK");
			break;
		case 9:
			strcpy(name, "MANUAL ENTRY");
			break;
		case 10:
			strcpy(name, "MAGNETIC STRIPE");
			break;
		case 11:
			strcpy(name, "PAYATTITUDE");
			break;
		case 12:
			strcpy(name, "NFC");
			break;
		
		/*default:
			strcpy(name, "ADMIN");
			break;*/
		default:
			strcpy(name, "");
			break;
	}
}

// ���׳�ʼ��:��齻���Ƿ�����,��ʾ���ױ���
// initiate transaction, check allowance, display title. 
int TransInitPurchase(uchar ucTranType)
{
	char name[50] = {0};
	glProcInfo.stTranLog.ucTranType = ucTranType;
	
	displayName(name);

	SetCurrTitle(_T(name));
	ShowLogs(1, "Inside TransInitPurchase 1");
	
	if( !ChkIfTranAllow(ucTranType) )
	{
		ShowLogs(1, "Inside TransInitPurchase 1a");
		return ERR_NO_DISP;
	}
	ShowLogs(1, "Inside TransInitPurchase 2");
	return 0;
}


// ���׳�ʼ��:��齻���Ƿ�����,��ʾ���ױ���
// initiate transaction, check allowance, display title. 
int TransInit(uchar ucTranType)
{
	/*
	glProcInfo.stTranLog.ucTranType = ucTranType;
	SetCurrTitle(_T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel));

	if( !ChkIfTranAllow(ucTranType) )
	{
		return ERR_NO_DISP;
	}

	return 0;*/
	//Edited by Wisdom

	char name[50] = {0};
	glProcInfo.stTranLog.ucTranType = ucTranType;
	
	displayName(name);

	SetCurrTitle(_T(name));

	if( !ChkIfTranAllow(ucTranType) )
	{
		return ERR_NO_DISP;
	}

	return 0;
}

// Modified by Kim_LinHB 2014-8-8 v1.01.0002 bug506
int TransCapture(void)
{//For please wait check here
	int	iRet;

	////ShowLogs(1, "Inside Transaction Capture SaleSub");
	TransInit(glProcInfo.stTranLog.ucTranType);
	////ShowLogs(1, "TransCapture. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	////ShowLogs(1, "TransCapture. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	////ShowLogs(1, "TransCapture. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	iRet = TransSaleSub();
	return iRet;
}


int TransCashSub(void)
{
	int		iRet;

	if( !ChkIfTranAllow(glProcInfo.stTranLog.ucTranType) )
	{
		return ERR_NO_DISP;
	}
	if( !ChkSettle() )
	{
		return ERR_NO_DISP;
	}

	iRet = GetAmount();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetDescriptor();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetAddlPrompt();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetPIN(FALSE);
	if( iRet!=0 )
	{
		return iRet;
	}

	SetCommReqField();
	iRet = TranProcess();
	ShowLogs(1, "WISDOM Pin Gotten TranProcess : %d", iRet);
	//PostCellulantNotificationFCMB();
	if( iRet!=ERR_NEED_FALLBACK )
	{
		return iRet;
	}

	// continue fallback process
	glProcInfo.bIsFallBack = TRUE;
	glProcInfo.stTranLog.uiEntryMode &= 0xF0;

	iRet = GetCard(FALLBACK_SWIPE|CARD_SWIPED);
	if( iRet!=0 )
	{
		return iRet;
	}

	SetCommReqField();
	return TranProcess();
}

// ��ͨ���ѡ���������
// sale or installment
int TransSale(uchar ucInstallment)
{
	int		iRet;
	uchar	ucEntryMode;

	iRet = TransInit((uchar)(ucInstallment ? INSTALMENT : SALE));
	if( iRet!=0 )
	{
		return iRet;
	}

	ucEntryMode = CARD_SWIPED|CARD_KEYIN;
	if (!ucInstallment)
	{
		ucEntryMode |= CARD_INSERTED;
	}

	iRet = GetCard(ucEntryMode);
	if( iRet!=0 )
	{
		return iRet;
	}

	return TransSaleSub();
}

// ����SALE��INSTALLMENT
// for sale and installment
int TransSaleSub(void)
{
	int		iRet;

	////ShowLogs(1, "Checking if Transaction is Allowed");
	if( !ChkIfTranAllow(glProcInfo.stTranLog.ucTranType) )
	{
		return ERR_NO_DISP;
	}
	////ShowLogs(1, "Transaction is Allowed");

	////ShowLogs(1, "Checking if Transaction is not Installment");
	// instalmentʱ������ѡ��plan֮�������ȷ��ACQ��������ڲ���Ҫ���settle״̬
	// when doing installment, ACQ will be confirmed after selecting plan, so don't need to check if need to settle now.
	if (glProcInfo.stTranLog.ucTranType!=INSTALMENT)
	{
		if( !ChkSettle() )
		{
			return ERR_NO_DISP;
		}
	}
	////ShowLogs(1, "Transaction is not Installment");

	////ShowLogs(1, "Trying to get Amount Again");
	iRet = GetAmount();
	if( iRet!=0 )
	{
		return iRet;
	}
	////ShowLogs(1, "Amount Gotten Again");

	////ShowLogs(1, "Trying to GetInstall Plan");
	iRet = GetInstalPlan();
	if( iRet!=0 )
	{
		return iRet;
	}
	////ShowLogs(1, "GetInstall Plan Gotten");

	// instalmentʱ������ѡ��plan֮�������ȷ��ACQ
	// when doing installment, ACQ is confirmed after selecting plan
	if (glProcInfo.stTranLog.ucTranType==INSTALMENT)
	{
		if( !ChkSettle() )
		{
			return ERR_NO_DISP;
		}
	}

	////ShowLogs(1, "Trying to Get Descriptor");
	iRet = GetDescriptor();
	if( iRet!=0 )
	{
		return iRet;
	}
	////ShowLogs(1, "Get Descriptor Done");

	////ShowLogs(1, "Trying to Get AddlPrompt");
	iRet = GetAddlPrompt();
	if( iRet!=0 )
	{
		return iRet;
	}
	////ShowLogs(1, "AddlPrompt Gotten");

	////ShowLogs(1, "About Getting Pin");
	iRet = GetPIN(FALSE);
	ShowLogs(1, "WISDOM Pin Gotten: %d", iRet);
	if( iRet!=0 )
	{
		return iRet;
	}
	////ShowLogs(1, "Pin Gotten");

	////ShowLogs(1, "About Checking if BelowMagFloor and not expired and not check if icc trans");
	if( ChkIfBelowMagFloor() && !glProcInfo.bExpiryError && !ChkIfIccTran(glProcInfo.stTranLog.uiEntryMode) )
	{
	//		sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%02ld", glSysCtrl.ulSTAN % 100);
		glProcInfo.stTranLog.uiStatus |= TS_CHANGE_APPV|TS_FLOOR_LIMIT;
		return FinishOffLine();
	}
	////ShowLogs(1, "Everything is fine");

	////ShowLogs(1, "About Setting Comm Req Field");
	SetCommReqField(); //This is where most isodata is set in struct glSendPack
	////ShowLogs(1, "Done Setting Comm Req");
	//Fine Till here
	////ShowLogs(1, "TransSaleSub. Visa Mastercard Added: glSysParam.ucAcqNum: %d", glSysParam.ucAcqNum);
	////ShowLogs(1, "TransSaleSub. Issuer Added: glSysParam.ucIssuerNum: %d", glSysParam.ucIssuerNum);
	////ShowLogs(1, "TransSaleSub. Card Table Added: glSysParam.ucCardNum: %d", glSysParam.ucCardNum);
	////ShowLogs(1, "About doing transaction processing");
	iRet = TranProcess();//Goes out here
	ShowLogs(1,"BEFORE CRASHING- PostCellulantNotificationFCMB");
	//to test crashing
	//PostCellulantNotificationFCMB();
	//PostTransactionNotificationFCMB();
	if(PURCHASETYPE==3)
	{
jambpayment();
	}
	ShowLogs(1, "Done with Tran Process. Return: %d", iRet);
	if( iRet!=ERR_NEED_FALLBACK )
	{
		////ShowLogs(1, "Done with Tran Process 2");
		return iRet;
	}
	////ShowLogs(1, "About Getting Pin");

	// continue fallback process
	glProcInfo.bIsFallBack = TRUE;
	glProcInfo.stTranLog.uiEntryMode &= 0xF0;

	iRet = GetCard(FALLBACK_SWIPE|CARD_SWIPED);
	if( iRet!=0 )
	{
		return iRet;
	}

	SetCommReqField();

	return TranProcess();
}


int FinishOffLine(void)
{
	uchar	ucTranAct;

	SetOffBase(OffBaseDisplay);

	DispProcess();

	if( !(glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) &&
		(glProcInfo.stTranLog.ucTranType!=SALE_COMP) )
	{
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
	}
	glProcInfo.stTranLog.ulInvoiceNo = glSysCtrl.ulInvoiceNo;
	ucTranAct = glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct;

	if (ucTranAct & ACT_INC_TRACE)
	{
		GetNewTraceNo();
	}

	DoE_Signature();

	if( ucTranAct & WRT_RECORD )
	{
		glProcInfo.stTranLog.uiStatus |= TS_NOSEND;
		SaveTranLog(&glProcInfo.stTranLog);
	}

	CommOnHook(FALSE);
	GetNewInvoiceNo();
	PrintAllReceipt(PRN_NORMAL);

	/*if( ucTranAct & PRN_RECEIPT )	// print slip
	{
		CommOnHook(FALSE);
		GetNewInvoiceNo();
		//PrintReceipt(PRN_NORMAL);
		PrintAllReceipt(PRN_NORMAL);
	}*/

	DispResult(0);
	// PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout); // Hidden by Kim_LinHB 2014/9/11 v1.01.0008 bug523

	return 0;
}

int TranReversal(void)
{
	#ifdef ENABLE_EMV
		int	iLength;
	#endif
		int	iRet;
		SYS_PROC_INFO	stProcInfoBak;

		if( glProcInfo.stTranLog.ucTranType==LOAD_PARA ||
			glProcInfo.stTranLog.ucTranType==ECHO_TEST ||
			glProcInfo.stTranLog.ucTranType==LOAD_CARD_BIN )
		{
			return 0;
		}

		if( !glSysCtrl.stRevInfo[glCurAcq.ucIndex].bNeedReversal )
		{
			return 0;
		}

		// backup current process information
		memcpy(&glProcInfo.stSendPack, &glSendPack, sizeof(STISO8583));
		memcpy(&stProcInfoBak, &glProcInfo, sizeof(SYS_PROC_INFO));
		// Modified by Kim_LinHB 2014-8-8 v1.01.0002 bug506
		//glProcInfo.stTranLog.ucTranType = REVERSAL;
		TransInit(REVERSAL);

		memcpy(&glSendPack, &glSysCtrl.stRevInfo[glCurAcq.ucIndex].stRevPack, sizeof(STISO8583));
		sprintf((char *)glSendPack.szMsgCode, "0400");
		if( ChkIfBoc() )  // Boc erase F55
		{
			memset(glSendPack.sICCData, 0, 2);
		}
		if( ChkIfAmex() )
		{
			memset(glSendPack.sICCData, 0, 2);
			memset(glSendPack.szLocalDate, 0, sizeof(glSendPack.szLocalDate));
			memset(glSendPack.szLocalTime, 0, sizeof(glSendPack.szLocalTime));
			memset(glSendPack.szRRN,       0, sizeof(glSendPack.szRRN));
			memset(glSendPack.szAuthCode,  0, sizeof(glSendPack.szAuthCode));
		}
		memset(glSendPack.sPINData, 0, sizeof(glSendPack.sPINData));	// erase PIN block

	#ifdef ENABLE_EMV
		if( (glSysCtrl.stRevInfo[glCurAcq.ucIndex].uiEntryMode & MODE_CHIP_INPUT) &&
			ChkIfAcqNeedDE56() )
		{
			iLength = glSysCtrl.stField56[glCurAcq.ucIndex].uiLength;
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
		if( ChkIfDah() || ChkIfCiti() )
		{
			memset(glSendPack.sICCData2, 0, 2);
		}
		if( (glSysCtrl.stRevInfo[glCurAcq.ucIndex].uiEntryMode & MODE_FALLBACK_SWIPE) ||
			(glSysCtrl.stRevInfo[glCurAcq.ucIndex].uiEntryMode & MODE_FALLBACK_MANUAL) )
		{
			if (ChkIfBoc())
			{
				memset(glSendPack.sICCData2, 0, 2);
			}
		}
	#endif

		if( ChkIfBoc() )
		{
			memset(glSendPack.szLocalDate, 0, sizeof(glSendPack.szLocalDate));
			memset(glSendPack.szLocalTime, 0, sizeof(glSendPack.szLocalTime));
			memset(glSendPack.szRRN, 0, sizeof(glSendPack.szRRN));
			memset(glSendPack.szAuthCode, 0, sizeof(glSendPack.szAuthCode));
		}

		while( 1 )
		{
			iRet = SendRecvPacket();
			if( iRet!=0 )
			{
				break;
			}
			if( memcmp(glRecvPack.szRspCode, "00", 2)==0 )
			{
				break;
			}
			if( ChkIfAmex() && (memcmp(glRecvPack.szRspCode, "08", 2)==0 || memcmp(glRecvPack.szRspCode, "88", 2)==0) )
			{
				break;
			}

			sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
			DispResult(ERR_HOST_REJ);
	// 		iRet = ERR_NO_DISP;
			iRet = ERR_TRAN_FAIL;
			break;
		}
	if( iRet==0 )
	{	// clear reversal flag
		SaveRevInfo(FALSE);
	}

	if (iRet==0)
	{
		// increase invoice for coming AMEX transaction
		if (ChkIfAmex())
		{
			if (glTranConfig[stProcInfoBak.stTranLog.ucTranType].ucTranAct & PRN_RECEIPT)
			{
				stProcInfoBak.stTranLog.ulInvoiceNo = GetNewInvoiceNo();
				PackInvoice(&stProcInfoBak.stSendPack, stProcInfoBak.stTranLog.ulInvoiceNo);
			}
		}
	}

	// restore process information
	//memcpy(&glProcInfo, &stProcInfoBak, sizeof(SYS_PROC_INFO));
	memcpy(&glSendPack, &glProcInfo.stSendPack, sizeof(STISO8583));

	TransInit(glProcInfo.stTranLog.ucTranType); // Added by Kim_LinHB 2014-8-8 v1.01.0002

	return iRet;
}

// ����
// Magnetic Stripe
int TransMagStripe(void)
{
	int		iRet;

	iRet = TransInit(OFF_SALE);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetCard(SKIP_CHECK_ICC|CARD_SWIPED|CARD_KEYIN);
	if( iRet!=0 )
	{
		return iRet;
	}

	if( !ChkSettle() )
	{
		return ERR_NO_DISP;
	}

	if( !ChkIssuerOption(ISSUER_EN_OFFLINE) )
	{
		DispBlockFunc();
		return ERR_NO_DISP;
	}

	iRet = GetAmount();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetDescriptor();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetAddlPrompt();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetPreAuthCode();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetPIN(FALSE);
	if( iRet!=0 )
	{
		return iRet;
	}

	return FinishOffLine();
}

// �˻�
//refund
int TransRefund(void)
{
	int		iRet;

	iRet = TransInit(REFUND);
	if( iRet!=0 )
	{
		return iRet;
	}

	if( !ChkEdcOption(EDC_NOT_REFUND_PWD) )
	{
		if( PasswordRefund()!=0 )
		{
			return ERR_NO_DISP;
		}
	}

	iRet = GetCard(CARD_SWIPED|CARD_KEYIN);
	if( iRet!=0 )
	{
		return iRet;
	}

	if( !ChkSettle() )
	{
		return ERR_NO_DISP;
	}

	if( ChkIssuerOption(ISSUER_NO_REFUND) )
	{
		DispBlockFunc();
		return ERR_NO_DISP;
	}

	iRet = GetAmount();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetDescriptor();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetAddlPrompt();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetPIN(FALSE);
	if( iRet!=0 )
	{
		return iRet;
	}

	if( ChkAcqOption(ACQ_ONLINE_REFUND) )
	{
		SetCommReqField();
		return TranProcess();
	}
	else
	{
		return FinishOffLine();
	}
}


//GUI_MENUITEM stDefTranMenuItem[20] = {{0}};
//char txnName[20][128];
int retaman = 0;

void getName(char *data, int *len, int key, GUI_MENUITEM *stDefTranMenuItem)
{
    int iLen = 0;
	int i, j = 0;
	char name[128] = {0};
	iLen = strlen(data);
	ShowLogs(1, "Inside getName: %d", iLen);
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
    ShowLogs(1, "Transaction name: %s", name);
    sprintf((char *)stDefTranMenuItem[key].szText, "%s", name);
    //if(strncmp(name, "RETAILMAN", 9) == 0)
    //	retaman = 1;
    stDefTranMenuItem[key].nValue = key;
    stDefTranMenuItem[key].bVisible = TRUE;
}

int parseTxn(char *data, GUI_MENUITEM *stDefTranMenuItem)
{
	int iLen = 0;
	int i, j = 0, key = 0;
	char name[128] = {0};
	iLen = strlen(data);
	//ShowLogs(1, "Inside parseTxn: %d", iLen);
	char *ret;
	//ShowLogs(1, "Inside parseTxn 2");
	for(i = 0; i < iLen; i++)
    {
    	ret = strstr(data + i, "name - ");
    	if(ret)
        {
            j = iLen - strlen(ret);
            getName(ret + 7, &i, key, stDefTranMenuItem);
            i = j;
            key++;
        }
    }
    //ShowLogs(1, "Inside parseTxn 3");
    return key--;
}

int TransPurchase(void)
{
	int iRet = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key;
	int chk = 0;
	char temp[5] = {0};
	int		iResult;
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

	txnType = 1;

	if(PURCHASETYPE!=1  && PURCHASETYPE!=2 && PURCHASETYPE!=3 && PURCHASETYPE!=4 ){
		if(acctTypeSelection())
			return;
	}else{
		UtilPutEnv("actype", "00");
	}
	//EEDSC 

	if(PURCHASETYPE==4)
	{
		eedcTypeSelection();
		GetCelPlatformID();
		eedcCusDetails();
		eedcTypePayment();
		
		/*jambTypeSelection();

		GUI_INPUTBOX_ATTR stInputAttr;
		memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
		memset(EEDC_CUS_ID, 0, sizeof(EEDC_CUS_ID));
		stInputAttr.eType = GUI_INPUT_MIX;
		stInputAttr.nMinLen = 0;
		stInputAttr.nMaxLen = 16;
		//Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox("Enter Customer ID", gl_stTitleAttr, EEDC_CUS_ID, gl_stLeftAttr, EEDC_CUS_ID, gl_stRightAttr, &stInputAttr , USER_OPER_TIMEOUT)) //GetCurrTitle()
		{
		return ERR_USERCANCEL;
		}*/
		//





	}
	//jamb
	if(PURCHASETYPE==3){
		jambTypeSelection();

		//get code

		GUI_INPUTBOX_ATTR stInputAttr;
		memset(&stInputAttr, 0, sizeof(stInputAttr));
		memset(JAMB_CODE, 0, sizeof(JAMB_CODE));
		stInputAttr.eType = GUI_INPUT_MIX;
		stInputAttr.nMinLen = 0;
		stInputAttr.nMaxLen = 16;
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox("Enter Jamb Code", gl_stTitleAttr, JAMB_CODE, gl_stLeftAttr, JAMB_CODE, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT)) //GetCurrTitle()
		{
		return ERR_USERCANCEL;
		}
		ShowLogs(1, "PURCHASETYPE=%i JAMB CODE: %s", PURCHASETYPE,JAMB_CODE);
		iResult = GetCelLogin(JAMB_CODE);
		//jambpayment();
		if (iResult == 0) return;
	}
	
	

				
				memset(temp, '\0', strlen(temp));
				UtilGetEnv("paytype", temp);
				//ShowLogs(1, "EEDC TRANSACTION PAYMENT TYPE =%i JAMB CODE: %s", PURCHASETYPE,temp);
				if(strstr(temp, "CASH") != NULL)
				{
					memset(glProcInfo.stTranLog.szAmount, '\0', strlen(glProcInfo.stTranLog.szAmount));
					iRet = GetAmount();
					if( iRet!=0 )
					{
						return iRet;
					}
					//eedcPayment();
					eedcPaymentCash();	
					return AfterTranProc();
				}


	DisplayInfoNone("", "INSERT CARD", 0);
	//DelayMs(2000);
		
if(PURCHASETYPE==1){
		ShowLogs(1, "Wisdom Trying to get Amount Again");
		txnType=1;
		memset(glProcInfo.stTranLog.szAmount, '\0', strlen(glProcInfo.stTranLog.szAmount));
		iRet = GetAmount();
		if( iRet!=0 )
		{
			return iRet;
		}
		if(PURCHASETYPE==1){ //extern char PAYATT_PHONENO[50];
			GUI_INPUTBOX_ATTR stInputAttr;
		
			memset(&stInputAttr, 0, sizeof(stInputAttr));
			memset(PAYATT_PHONENO, 0, sizeof(PAYATT_PHONENO));
			stInputAttr.eType = GUI_INPUT_MIX;
			stInputAttr.nMinLen = 0;
			stInputAttr.nMaxLen = 16;

			Gui_ClearScr();
			if(GUI_OK != Gui_ShowInputBox("PHONE NUMBER", gl_stTitleAttr, PAYATT_PHONENO, gl_stLeftAttr, PAYATT_PHONENO, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT)) //GetCurrTitle()
			{
				return ERR_USERCANCEL;
			}
			ShowLogs(1, "PURCHASETYPE=%i Phone number: %s", PURCHASETYPE,PAYATT_PHONENO);

		}

		SetCommReqField();
		iRet = TranProcess();		
		//ShowLogs(1, "GIONG TO PRINT PAYATTITUDE Tran Process. Return: %d", iRet);
		//PrintAllReceipt(PRN_NORMAL);

		//ShowLogs(1, "Done with PAYATTITUDE Tran Process. Return: %d", iRet);
}


else if(PURCHASETYPE==2){
		////ShowLogs(1, "Trying to get Amount Again");
		//txnType=0;
		txnType=1;
		memset(glProcInfo.stTranLog.szAmount, '\0', strlen(glProcInfo.stTranLog.szAmount));
		iRet = GetAmount();
		if( iRet!=0 )
		{
			return iRet;
		}


		if(PURCHASETYPE==2){ //extern char PAYATT_PHONENO[50];
			GUI_INPUTBOX_ATTR stInputAttr;
		
			memset(&stInputAttr, 0, sizeof(stInputAttr));
			memset(PAYATT_PHONENO, 0, sizeof(PAYATT_PHONENO));
			stInputAttr.eType = GUI_INPUT_MIX;
			stInputAttr.nMinLen = 0;
			stInputAttr.nMaxLen = 16;
			Gui_ClearScr();
			if(GUI_OK != Gui_ShowInputBox("PHONE NUMBER", gl_stTitleAttr, PAYATT_PHONENO, gl_stLeftAttr, PAYATT_PHONENO, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT)) //GetCurrTitle()
			{
				return ERR_USERCANCEL;
			}
			ShowLogs(1, "PURCHASETYPE=%i Phone number: %s", PURCHASETYPE,PAYATT_PHONENO);

		}

		SetCommReqField();
		iRet = TranProcess();
		ShowLogs(1, "Done with Tran Process. Return: %d", iRet);
}

else{
		while(1)
		{
			if (kbhit()==0) { 
				if(getkey()==KEYCANCEL)
				{
					Beep();
					PromptRemoveICC();
					break;
				}
			}
			
			if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
			{
				//check if it a cash flow
				char temp[5] = {0};
				memset(temp, '\0', strlen(temp));
				UtilGetEnv("paytype", temp);
				if(strstr(temp, "CASH") != NULL)
				{
				eedcPayment();	
				break;
				}
				{ 
				txnType = 1;
				iRet = ProcICCMsg();
				////ShowLogs(1, "Switch ICCARD_MSG Response: %d", iRet);
				PromptRemoveICC();

				////ShowLogs(1, "Done with PromptRemoveICC");
				break;
				}
			}
		}
}
}
/*
int TransPurchase(void)
{
	int iRet = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key;
	int chk = 0;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

	txnType = 1;

	if(acctTypeSelection())
		return;

	DisplayInfoNone("", "INSERT CARD", 0);
	//DelayMs(2000);

	while(1)
	{
		if (kbhit()==0) { 
			if(getkey()==KEYCANCEL)
			{
				Beep();
				PromptRemoveICC();
				break;
			}
		}
		
		if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
		{
			txnType = 1;
			iRet = ProcICCMsg();
			////ShowLogs(1, "Switch ICCARD_MSG Response: %d", iRet);
			PromptRemoveICC();
			////ShowLogs(1, "Done with PromptRemoveICC");
			break;
		}
	}
}*/

void getHost(char *str, int len, char *out)
{
    int chk = 0;
    int i = 0;
    for(i = len; i < strlen(str); i++)
    {
        if(str[i] == '#')
        {
            break;
        }else
        {
            out[chk] = str[i];
            chk++;
        }
    }
}

void resetHostDetails()
{
	char temp[128] = {0};
	char lasthost[128] = {0};

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("uhostmestype", temp);
	////ShowLogs(1, "Reset Message type: %s", temp);
	if(strstr(temp, "iso") != NULL)
	{
		return;
	}
	memset(lasthost, '\0', strlen(lasthost));
	UtilGetEnvEx("lhost", lasthost);
	////ShowLogs(1, "1. Lasthost: %s", lasthost);
	if(strstr(lasthost, "host2") != NULL)
	{
		////ShowLogs(1, "Resetting Host to do Iso");
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2name", temp);
		UtilPutEnv("uhostname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2ip", temp);
		UtilPutEnv("uhostip", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2port", temp);
		UtilPutEnv("uhostport", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2ssl", temp);
		UtilPutEnv("uhostssl", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2fname", temp);
		UtilPutEnv("uhostfname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2mestype", temp);
		////ShowLogs(1, "Message type: %s", temp);
		UtilPutEnv("uhostmestype", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2remarks", temp);
		UtilPutEnv("uhostremarks", temp);
		UtilPutEnv("lhost", "host2");
	}else
	{
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostname", temp);
		UtilPutEnv("uhostname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostip", temp);
		UtilPutEnv("uhostip", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostport", temp);
		UtilPutEnv("uhostport", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostssl", temp);
		UtilPutEnv("uhostssl", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostfname", temp);
		UtilPutEnv("uhostfname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostmestype", temp);
		////ShowLogs(1, "Message type: %s", temp);
		UtilPutEnv("uhostmestype", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostremarks", temp);
		UtilPutEnv("uhostremarks", temp);
		UtilPutEnv("lhost", "host1");
	}
}



int setHostDetails(char *txn)
{
	char hostarray[1024] = {0};
	int m;
	char output[6] = {0};
	char temp[128] = {0};
	char lasthost[128] = {0};

	
	memset(hostarray, '\0', strlen(hostarray));
	m = ReadAllData("host.txt", hostarray);

	memset(output, '\0', strlen(output));
	char *ret;
	ret = strstr(hostarray, txn);
	getHost(ret, strlen(txn) + 3, output);
	////ShowLogs(1, "Host of current transaction: %s", output);
	if(strstr(output, "host2") != NULL)
    {
    	memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2name", temp);
		UtilPutEnv("uhostname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2ip", temp);
		UtilPutEnv("uhostip", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2port", temp);
		UtilPutEnv("uhostport", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2ssl", temp);
		UtilPutEnv("uhostssl", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2fname", temp);
		UtilPutEnv("uhostfname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2mestype", temp);
		////ShowLogs(1, "Message type: %s", temp);
		UtilPutEnv("uhostmestype", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("host2remarks", temp);
		UtilPutEnv("uhostremarks", temp);
		memset(lasthost, '\0', strlen(lasthost));
		UtilGetEnvEx("lhost", lasthost);
		if(strstr(lasthost, "host2") != NULL)
    	{
    		////ShowLogs(1, "Same host as last transaction - host 2");
    		UtilPutEnv("lhost", "host2");
			return 0;
    	}
    	////ShowLogs(1, "1. Different Host from previous transaction. Last Host - %s", lasthost);
    	memset(temp, '\0', strlen(temp));
		ReadAllData("hosb.txt", temp);
		////ShowLogs(1, "Setting CTMK TO HOST 2: %s", temp);
		//UtilPutEnv("proCtmk", temp);
    	memset(temp, '\0', strlen(temp));
    	UtilGetEnv("host2mestype", temp);
    	if(strstr(temp, "string") != NULL)
    	{
    		UtilPutEnv("lhost", "host2");
			return 0;
    	}
		UtilPutEnv("lhost", "host2");
		return 0;
    }else
    {
    	memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostname", temp);
		UtilPutEnv("uhostname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostip", temp);
		UtilPutEnv("uhostip", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostport", temp);
		UtilPutEnv("uhostport", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostssl", temp);
		UtilPutEnv("uhostssl", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostfname", temp);
		UtilPutEnv("uhostfname", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostmestype", temp);
		////ShowLogs(1, "Message type: %s", temp);
		UtilPutEnv("uhostmestype", temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("hostremarks", temp);
		UtilPutEnv("uhostremarks", temp);
		memset(lasthost, '\0', strlen(lasthost));
		UtilGetEnvEx("lhost", lasthost);
		////ShowLogs(1, "3. Lasthost: %s", lasthost);
		if(strstr(lasthost, "host1") != NULL)
    	{
    		////ShowLogs(1, "Same host as last transaction - host 1");
    		UtilPutEnv("lhost", "host1");
			return 0;
    	}
    	////ShowLogs(1, "2. Different Host from previous transaction. Last Host - %s", lasthost);
    	memset(temp, '\0', strlen(temp));
		ReadAllData("hosa.txt", temp);
		ShowLogs(1, "Setting CTMK TO HOST 1: %s", temp);
		//UtilPutEnv("proCtmk", temp);
    	memset(temp, '\0', strlen(temp));
    	UtilGetEnv("hostmestype", temp);
    	if(strstr(temp, "string") != NULL)
    	{
    		UtilPutEnv("lhost", "host1");
			return 0;
    	}
		UtilPutEnv("lhost", "host1");
		return 0;
    }
}


void paymentOptions()
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
	
	
	numLines = 6;//Line for control

	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "PURCHASE");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "PURCHASE", strlen("PURCHASE"));

    key++;


    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "PURCHASE WITH CASHBACK");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "PURCHASE WITH CASHBACK", strlen("PURCHASE WITH CASHBACK"));
	key++;

    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "PURCHASE WITH USSD");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "PURCHASE WITH USSD", strlen("PURCHASE WITH USSD"));  

	key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "PAYATTITUDE");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "PAYATTITUDE", strlen("PAYATTITUDE"));  
	key++;

	/*
	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "MERCHANT PAYATTITUDE");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "MERCHANT PAYATTITUDE", strlen("MERCHANT PAYATTITUDE"));  
	key++;
	*/

	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "JAMB PAYMENT");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "JAMB PAYMENT", strlen("JAMB PAYMENT"));  
	key++;

	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "EEDC PAYMENT");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "EEDC PAYMENT", strlen("EEDC PAYMENT"));  
	key++;

	for(i = 0; i < 7; ++i)
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
        if(strncmp(txnName1[iMenuNo], "PURCHASE WITH CASHBACK", 22) == 0)
        {
            cashBack();
        }else if(strncmp(txnName1[iMenuNo], "PURCHASE WITH USSD", 18) == 0)
        {   
            Beep();
            return;
        }else if(strncmp(txnName1[iMenuNo], "PURCHASE", 8) == 0)
		{
            TransPurchase();
		}
		else if(strncmp(txnName1[iMenuNo], "PAYATTITUDE", 11) == 0)
		{
			PURCHASETYPE=1;
			//payAttitude();
			//payAttitude();
            TransPurchase();
			PURCHASETYPE=0;
		}
		//JAMB
		else if(strncmp(txnName1[iMenuNo], "JAMB PAYMENT", 12) == 0)
		{
			PURCHASETYPE=3;
			//payAttitude();
			//payAttitude();
            TransPurchase();
			PURCHASETYPE=0;
		}

		else if(strncmp(txnName1[iMenuNo], "EEDC PAYMENT", 12) == 0)
		{
			PURCHASETYPE=4;
			//payAttitude();
			//payAttitude();
            TransPurchase();
			PURCHASETYPE=0;
		}
		else if(strncmp(txnName1[iMenuNo], "MERCHANT PAYATTITUDE", 20) == 0)
		{
			PURCHASETYPE=2;
			//payAttitude();
			//payAttitude();
            TransPurchase();
			PURCHASETYPE=0;
		}
		else 
        {
        	Beep();
        	return;
        }
        Beep();
		return;
	}else
	{
		Beep();
		return;
	}
	return 1;
}

int noguide = 0; 
int checkBoard = 0;
// Modified by Kim_LinHB 2014-7-11
int TransOther(void)
{
	int		iRet, iUse = 0, iMenuNo, m, i;
	uchar	ucTranType, k;
	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	GUI_MENUITEM stDefTranMenuItem[20] = {{0}};
	
	char store_env[120] = {0};
	char ecrcheck[120] = {0};
	char protectedlist[2 * 1024] = {0};
	char txn[3 * 1024] = {0};
	int iMenuItemNum = 0;
	int co = 0;

	CreateWrite("TRANLOG.DAT", "");//Self made
	CreateWrite("SYSPARAM.DAT", "");//Self made
	CreateWrite("SYSCTRL.DAT", "");//Self made

	//Cleared buffer
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

	memset(&printVas, 0, sizeof(PRINTVAS));
	//ShowLogs(1, "Menu Step 0");
	//menuFlag = 1;
	memset(txn, '\0', strlen(txn));
	iRet = ReadAllData("menus.txt", txn);
	ShowLogs(1, "Menu Step 1");
	//ShowLogs(1, "Menu Step 2");
	i = 0;
	iRet = parseTxn(txn, stDefTranMenuItem);
	//ShowLogs(1, "Menu Step 3");
	iUse = iRet;
	//ShowLogs(1, "Menu Step 4");
	memset(glProcInfo.stTranLog.szRspCode, '\0', strlen(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szDateTime, '\0', strlen(glProcInfo.stTranLog.szDateTime));
	memset(glProcInfo.stTranLog.szAuthCode, '\0', strlen(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, '\0', strlen(glProcInfo.stTranLog.szRRN));
	memset(glProcInfo.stTranLog.szCondCode, '\0', strlen(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szFrnAmount, '\0', strlen(glProcInfo.stTranLog.szFrnAmount));
	//ShowLogs(1, "Menu Step 5");
	memset(glProcInfo.stTranLog.szAppLabel, '\0', strlen(glProcInfo.stTranLog.szAppLabel));//Reset app name done here
	memset(protectedlist, '\0', strlen(protectedlist));
	m = ReadAllData("protectlist.txt", protectedlist);
	//ShowLogs(1, "Menu Step 6");
	//ShowLogs(1, "Menu Step 7");
	revFlag = 0;

	numLines = 6;//Line for control
	noguide = 1;
	checkBoard = 0;
	checkMimic = 0;
    while(1)
    {
    	
	    for(i = 0; i < iRet; ++i)
	    {
	        if(stDefTranMenuItem[i].bVisible)
	        {
	        	//ShowLogs(1, "James Agada: %s", stDefTranMenuItem[i].szText);
	        	memcpy(&stTranMenuItem[iMenuItemNum], &stDefTranMenuItem[i], sizeof(GUI_MENUITEM));
	            sprintf(stTranMenuItem[iMenuItemNum].szText, "%s", stDefTranMenuItem[i].szText);
	            ++iMenuItemNum;
	        }
	    }

	    stTranMenuItem[iMenuItemNum].szText[0] = 0;

		Gui_BindMenu("", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stTranMenuItem, &stTranMenu);
		
		Gui_ClearScr();
		iMenuNo = 0;
		//iRet = Gui_ShowMenuList(&stTranMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iMenuNo);......
		memset(store_env, '\0', strlen(store_env));
        UtilGetEnv("chinterval", store_env);
		iRet = Gui_ShowMenuList(&stTranMenu, GUI_MENU_DIRECT_RETURN, atoi(store_env), &iMenuNo);
		if(GUI_OK == iRet)
		{
			checkBoard = 0;
			if(strncmp(stDefTranMenuItem[iMenuNo].szText, "PURCHASE", 3) == 0)//PAY
			{
				checkBoard = 0;
				if(setHostDetails("PURCHASE"))//PAY
					TransOther();
				if(testPrinter() != PRN_OK)
					TransOther();
				if(strstr(protectedlist, "name - PURCHASE###") != NULL)//PAY
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						 TransPurchase();
						//paymentOptions();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					 TransPurchase();
					//paymentOptions();
					TransOther();
				}
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "ADMIN", 5) == 0)
			{
				checkBoard = 0;
				if(strstr(protectedlist, "name - ADMIN###") != NULL)
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						iRet = adminFunc();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					iRet = adminFunc();
					TransOther();
				}
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "REPORTS", 7) == 0)
			{
				checkBoard = 0;
				iRet = Reports();
				TransOther();
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "CASH ADVANCE", 12) == 0)
			{
				checkBoard = 0;
				if(setHostDetails("CASH ADVANCE"))
					TransOther();
				if(testPrinter() != PRN_OK)
					TransOther();
				if(strstr(protectedlist, "name - CASH ADVANCE###") != NULL)//CASH ADVANCE
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						iRet = cashAdvance();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					iRet = cashAdvance();
					TransOther();
				}
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "CASH BACK", 12) == 0)
			{
				checkBoard = 0;
				if(setHostDetails("CASH BACK"))
					TransOther();
				if(testPrinter() != PRN_OK)
					TransOther();
				if(strstr(protectedlist, "name - CASH BACK###") != NULL)//CASH BACK
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						iRet = cashBack();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					iRet = cashBack();
					TransOther();
				}
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "PRE AUTH", 8) == 0)
			{
				checkBoard = 0;
				if(testPrinter() != PRN_OK)
					TransOther();
				if(setHostDetails("PRE AUTH"))
					TransOther();
				if(strstr(protectedlist, "name - PRE AUTH###") != NULL)
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						iRet = preAuth();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					iRet = preAuth();
					TransOther();
				}
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "REFUND", 6) == 0)
			{
				checkBoard = 0;
				if(testPrinter() != PRN_OK)
					TransOther();
				if(setHostDetails("REFUND"))
					TransOther();
				if(strstr(protectedlist, "name - REFUND###") != NULL)
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						iRet = RefundEntrance();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					iRet = RefundEntrance();
					TransOther();
				}
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "BALANCE ENQUIRY", 15) == 0)
			{
				checkBoard = 0;
				if(testPrinter() != PRN_OK)
					TransOther();
				if(setHostDetails("BALANCE ENQUIRY"))
					TransOther();
				ShowLogs(1, "Inside Balance Enquiry 1");
				if(strstr(protectedlist, "name - BALANCE ENQUIRY###") != NULL)
				{
					ShowLogs(1, "Inside Balance Enquiry 2");
					if(merchantPin() == 1)
					{
						ShowLogs(1, "Inside Balance Enquiry 3");
						menuFlag = 0;
						noguide = 0;
						iRet = balanceEnquiry();
						TransOther();
					}else
					{
						ShowLogs(1, "Inside Balance Enquiry 4");
						TransOther();
					}
				}else
				{
					ShowLogs(1, "Inside Balance Enquiry 5");
					menuFlag = 0;
					noguide = 0;
					iRet = balanceEnquiry();
					TransOther();
				}
				ShowLogs(1, "Inside Balance Enquiry 6");
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "SALES COMPLETION", 16) == 0)
			{
				checkBoard = 0;
				if(testPrinter() != PRN_OK)
					TransOther();
				if(setHostDetails("SALES COMPLETION"))
					TransOther();
				if(strstr(protectedlist, "name - SALES COMPLETION###") != NULL)
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						iRet = salesCompletion();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					iRet = salesCompletion();
					TransOther();
				}
			}else if(strncmp(stDefTranMenuItem[iMenuNo].szText, "REVERSAL", 8) == 0)
			{
				checkBoard = 0;
				if(testPrinter() != PRN_OK)
					TransOther();
				if(setHostDetails("REVERSAL"))
					TransOther();
				if(strstr(protectedlist, "name - REVERSAL###") != NULL)
				{
					if(merchantPin() == 1)
					{
						menuFlag = 0;
						noguide = 0;
						iRet = reversalTxn();
						TransOther();
					}else
					{
						TransOther();
					}
				}else
				{
					menuFlag = 0;
					noguide = 0;
					iRet = reversalTxn();
					TransOther();
				}
			}
			CommOnHook(FALSE);
			Gui_ClearScr(); // Added by Kim_LinHB 9/9/2014 v1.01.0007 bug515
		}else if(GUI_ERR_TIMEOUT == iRet)
		{
			checkBoard = 0;
			noguide = 0;
			Gui_ClearScr();
			PackCallHomeData();
        	EtopLogo();
        	while(1)
			{
				if( 0==kbhit() )
				{
					Beep();
					k = getkey();
					break;
				}
				DelayMs(200);
			}
			Gui_ClearScr();
			TransOther();//Testing something
		}else if(GUI_ERR_USERCANCELLED == iRet)
		{
			TransOther();//Testing User Cancel
		}else
		{
			TransOther();//Testing User Cancel
		}
	}
	return ERR_NO_DISP;
}



// end of file

