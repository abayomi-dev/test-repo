#include "global.h"

void SetEntryModeComp(const TRAN_LOG *pstLog)
{
	sprintf((char *)glSendPack.szEntryMode, "0000");

	if( ChkIfAmex() )
	{
		//ShowLogs(1, "Inside SetEntryMode 1");
		SetAmexEntryModeComp(pstLog);
	}
	else
	{
		//ShowLogs(1, "Inside SetEntryMode 2");
		SetStdEntryModeComp(pstLog);
	}
}

void SetAmexEntryModeComp(const TRAN_LOG *pstLog)
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

void SetStdEntryModeComp(const TRAN_LOG *pstLog)
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

void SetCondCodeComp(void)
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


void SetCommReqFieldComp(void)
{
	char temp[200] = {0};
	int len = 0;
	char icc[250] = {0};
	#ifdef ENABLE_EMV
		int	iLength;
	#endif
	uchar	szTotalAmt[12+1];
	
	char timeGotten[30] = {0};
	char datetime[30] = {0};
	

	//memset(&glSendPack, 0, sizeof(STISO8583));

	sprintf((char *)glSendPack.szMsgCode, "%s", "0220");
	

	sprintf((char *)glSendPack.szPoscCode, "%.*s", LEN_POSC_CODE, "06");
	ShowLogs(1, "Pos Pin Capture Code: %s", glSendPack.szPoscCode);
	sprintf((char *)glSendPack.szTransFee, "%.*s", LEN_TRANS_FEE, "D00000000");
	ShowLogs(1, "Transaction Amount: %s", glSendPack.szTransFee);
	sprintf((char *)glSendPack.szAqurId, "%.*s", LEN_AQUR_ID, "111129");
	ShowLogs(1, "Aquirer Id: %s", glSendPack.szAqurId);
	sprintf((char *)glSendPack.szPosDataCode, "%.*s", LEN_POS_CODE, "510101511344101");
	ShowLogs(1, "Pos Data Code: %s", glSendPack.szPosDataCode);

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("actype", temp);
	strcat(temp, "0000");
	sprintf((char *)glSendPack.szProcCode, "%.*s", LEN_PROC_CODE, temp);

	//sprintf((char *)glSendPack.szMsgCode, "%.*s", LEN_MSG_CODE, glTranConfig[glProcInfo.stTranLog.ucTranType].szTxMsgID);
	//sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, glTranConfig[glProcInfo.stTranLog.ucTranType].szProcCode);
	//ModifyProcessCode();
	ShowLogs(1, "Mti: %s", glSendPack.szMsgCode);
	ShowLogs(1, "Proc: %s", glSendPack.szProcCode);

	// modify bit 3, process code
	
	sprintf((char *)glSendPack.szNii,        "%.*s", LEN_NII,         glCurAcq.szNii);
	//ShowLogs(1, "Nii: %s", glSendPack.szNii);
	//sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,     glCurAcq.szTermID);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	sprintf((char *)glSendPack.szTermID, "%.*s", LEN_TERM_ID, temp);
	ShowLogs(1, "Tid: %s", glSendPack.szTermID);

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, temp);
	ShowLogs(1, "Mid: %s", glSendPack.szMerchantID);

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnCurCode", temp);
	sprintf((char *)glSendPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, temp);
	ShowLogs(1, "Currency Code: %s", glSendPack.szTranCurcyCode);

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMNL", temp);
	sprintf((char *)glSendPack.szMNL, "%.*s", LEN_MNL, temp);
	ShowLogs(1, "Merchant Name: %s", glSendPack.szMNL);

	//Pinblock
	HexEnCodeMethod(glProcInfo.sPinBlock, 8, glSendPack.szPinBlock);
	ShowLogs(1, "Pinblock: %s", glSendPack.szPinBlock);
	
	//glSysCtrl.ulSTAN++;//By Wisdom
	//GetStan();
	//glSysCtrl.ulSTAN = useStan;
	//sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);  //??
	//glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
	ShowLogs(1, "Stan: %s", glSendPack.szSTAN);
	/*SysGetTimeIso(timeGotten);
	strncpy(datetime, timeGotten + 2, 10);
	sprintf((char *)glSendPack.szLocalDateTime, "%.*s", LEN_LOCAL_DATE_TIME, datetime);
	strcpy(glSendPack.szOrigDataElement, "0100");
	strcat(glSendPack.szOrigDataElement, glSendPack.szSTAN);
	//strcat(glSendPack.szOrigDataElement, glSendPack.szLocalDateTime);
	strcat(glSendPack.szOrigDataElement, "0227194534");
	strcat(glSendPack.szOrigDataElement, glSendPack.szAqurId);
	strcat(glSendPack.szOrigDataElement, "0000000000000000");
	strcpy(glSendPack.szReplAmount, "000000000000000000000000C00000000C00000000");*/

	if( glProcInfo.stTranLog.ucTranType==SETTLEMENT || glProcInfo.stTranLog.ucTranType==UPLOAD ||
		glProcInfo.stTranLog.ucTranType==LOGON )
	{
		ShowLogs(1, "Trying to check if its settlement, upload or logon");
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
		ShowLogs(1, "Amount: %s", glSendPack.szTranAmt);
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
		//sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);
		//sprintf((char *)glSendPack.szAuthCode,  "%.6s",  glProcInfo.stTranLog.szAuthCode);
	}
	else if( glProcInfo.stTranLog.ucTranType==VOID )
	{
		sprintf((char *)glSendPack.szLocalTime, "%.6s",  &glProcInfo.stTranLog.szDateTime[8]);
		sprintf((char *)glSendPack.szLocalDate, "%.4s",  &glProcInfo.stTranLog.szDateTime[4]);
		//sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);       // jiale 2006.12.12
		//sprintf((char *)glSendPack.szAuthCode,  "%.6s",  glProcInfo.stTranLog.szAuthCode);	// jiale for void send 37.38field
	}
	else
	{
		sprintf((char *)glSendPack.szLocalTime, "%.6s",  &glProcInfo.stTranLog.szDateTime[8]);
	    sprintf((char *)glSendPack.szLocalDate, "%.4s",  &glProcInfo.stTranLog.szDateTime[4]);
	}

	ShowLogs(1, "LocalTime: %s", glSendPack.szLocalTime);
	ShowLogs(1, "LocalDate: %s", glSendPack.szLocalDate);
	//ShowLogs(1, "RRN: %s", glSendPack.szRRN);
	//ShowLogs(1, "AuthCode: %s", glSendPack.szAuthCode);

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

	ShowLogs(1, "Pan: %s", glSendPack.szPan);
	ShowLogs(1, "Exp Date: %s", glSendPack.szExpDate);
	ShowLogs(1, "Card Sequence Number: %s", glSendPack.szPanSeqNo);
	memset(temp, '\0', strlen(temp));
	strcpy(temp, glSendPack.szTrack2);
	len = strlen(glSendPack.szPan) + 5;
	strncpy(glSendPack.szServResCode, temp + len, 3);
	ShowLogs(1, "Service Restriction Code: %s", glSendPack.szServResCode);
	ShowLogs(1, "Track 2: %s", glSendPack.szTrack2);
	
	//Start here	
	SetEntryModeComp(&glProcInfo.stTranLog);		// bit 22, entry mode
	SetCondCodeComp();		// bit 25, service condition code 

	ShowLogs(1, "Entry Code: %s", glSendPack.szEntryMode);
	ShowLogs(1, "Condition Code: %s", glSendPack.szCondCode);

	//Resume here
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMCC", temp);
	sprintf((char *)glSendPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, temp);
	ShowLogs(1, "Mcc: %s", glSendPack.szMerchantType);

	// bit 48 or 55, CVV2 or 4DBC
	if( ChkIfNeedSecurityCode() && !ChkIfAmex() )
	{
		memcpy(glSendPack.sField48, "\x00\x03", 2);
		sprintf((char *)&glSendPack.sField48[2], "%-3.3s", glProcInfo.szSecurityCode);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT )
	{
		PubLong2Char((ulong)LEN_PIN_DATA, 2, glSendPack.sPINData);
		memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, LEN_PIN_DATA);
	}
	
	// process bit 55,56
	if( glProcInfo.stTranLog.ucTranType==CASHADVANCE )
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
}

int TransCompSub(void)
{
	int		iRet;

	ShowLogs(1, "Transaction is Allowed");
	iRet = GetAmount();
	if( iRet!=0 )
	{
		return iRet;
	}
	iRet = GetInstalPlan();
	if( iRet!=0 )
	{
		return iRet;
	}
	ShowLogs(1, "Trying to Get Descriptor");
	iRet = GetDescriptor();
	if( iRet!=0 )
	{
		return iRet;
	}
	ShowLogs(1, "Get Descriptor Done");
	iRet = GetAddlPrompt();
	if( iRet!=0 )
	{
		return iRet;
	}
	ShowLogs(1, "AddlPrompt Gotten");
	iRet = GetPIN(FALSE);
	ShowLogs(1, "Pin Gotten: %d", iRet);
	if( iRet!=0 )
	{
		return iRet;
	}
	ShowLogs(1, "Pin Gotten");
	ShowLogs(1, "About Checking if BelowMagFloor and not expired and not check if icc trans");
	if( ChkIfBelowMagFloor() && !glProcInfo.bExpiryError && !ChkIfIccTran(glProcInfo.stTranLog.uiEntryMode) )
	{
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%02ld", glSysCtrl.ulSTAN % 100);
		glProcInfo.stTranLog.uiStatus |= TS_CHANGE_APPV|TS_FLOOR_LIMIT;
		return FinishOffLine();
	}
	SetCommReqFieldComp();
	iRet = TranProcess();//Goes out here
	if( iRet!=ERR_NEED_FALLBACK )
	{
		return iRet;
	}
	glProcInfo.bIsFallBack = TRUE;
	glProcInfo.stTranLog.uiEntryMode &= 0xF0;
	iRet = GetCard(FALLBACK_SWIPE|CARD_SWIPED);
	if( iRet!=0 )
	{
		return iRet;
	}
	SetCommReqFieldComp();
	return TranProcess();
}

int TransCaptureComp(void)
{
	int	iRet;
	char name[50] = {0};
	glProcInfo.stTranLog.ucTranType = SALE;
	displayName(name);
	SetCurrTitle(_T(name));
	iRet = TransCompSub();
	return iRet;
}

int CompProcICCMsg(void)
{
	int		iRet;

	char name[50] = {0};
	glProcInfo.stTranLog.ucTranType = SALE;
	displayName(name);
	SetCurrTitle(_T(name));

	amtCount = 0;
	iRet = GetCard(SKIP_DETECT_ICC|CARD_INSERTED); //Amount is here
	if( iRet!=0 )
	{
		return iRet;
	}

	ShowLogs(1, "1. Card Details Gotten");
	iRet = TransCaptureComp();
	ShowLogs(1, "2. Card Details Gotten: %d", iRet);

	if( iRet!=0 )
	{
		ShowLogs(1, "3. Card Details Gotten");
		CommOnHook(FALSE);
		return iRet;
	}

	return 0;
}


int parseEodCompletion(char *rrn)
{
	int iRet, i, j;
	char txn[100 * 1024] = {0};
	char fin[200] = {0};
	char pan[20] = {0};
	char expdate[5] = {0};
	char src[4] = {0};
	char omti[20] = {0};
	char odatetime[11] = {0};
	char ocardno[50] = {0};
	char oproc[7] = {0};
	char oamt[13] = {0};
	char orrn[13] = {0};
	char oseqnum[13] = {0};
	char oentrymode[13] = {0};
	char oposdatacode[20] = {0};
	char ostan[7] = {0};
	char temp[200] = {0};
	char acq[7] = {0};
	char step[1 * 1024] = {0};
	uchar	szSTAN[LEN_STAN+2];

	iRet = ReadAllData("eod.txt", txn);

	j = 0;
	for(i = 0; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			if(parseInnerReversal(step, rrn, omti, odatetime, ocardno, oproc, oamt, 
				orrn, oseqnum, oentrymode, oposdatacode, ostan, acq))
				break;
			j = 0;
			memset(step, '\0', strlen(step));
		}else
		{
			step[j] = txn[i];
			j++;
		}
	}
	
	ShowLogs(1, "ORIG MTI: %s", omti);
	ShowLogs(1, "ORIG DATETIME: %s", odatetime);
	ShowLogs(1, "ORIG STAN: %s", ostan);
	ShowLogs(1, "ORIG PROC: %s", oproc);
	ShowLogs(1, "ORIG AMT: %s", oamt);
	ShowLogs(1, "ORIG RRN: %s", orrn);
	ShowLogs(1, "ORIG SEQ NUM: %s", oseqnum);
	ShowLogs(1, "ORIG ENTRY MODE: %s", oentrymode);
	ShowLogs(1, "ORIG DATA CODE: %s", oposdatacode);

	if(strncmp(glSendPack.szRRN, orrn, 12) == 0)
	{
		//strcpy(glSendPack.szAqurId, acq);
		strcpy(glSendPack.szLocalDateTime, odatetime);
		strcpy(glSendPack.szSTAN, ostan);
		strcpy(glSendPack.szOrigDataElement, omti);
		strcat(glSendPack.szOrigDataElement, ostan);
		strcat(glSendPack.szOrigDataElement, odatetime);
		strcat(glSendPack.szOrigDataElement, "00000");
		strcat(glSendPack.szOrigDataElement, acq);
		strcat(glSendPack.szOrigDataElement, "00000");
		//strcat(glSendPack.szOrigDataElement, acq);
		strcat(glSendPack.szOrigDataElement, "424367");
		strcpy(glSendPack.szReplAmount, oamt);
		strcat(glSendPack.szReplAmount, oamt);
		strcat(glSendPack.szReplAmount, "D00000000D00000000");

		return 1;
	}
	DisplayInfoNone("", "Transaction not Seen", 3);
	return 0;
}

int salesCompletion()
{
	int iRet = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key;
	int chk = 0;
	char rrn[13] = {0};
	char auth[7] = {0};
	char tmp[7] = {0};
	char txn[100 * 1024] = {0};
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

	iRet = ReadAllData("eod.txt", txn);
	if(strlen(txn) < 10)
	{
		DisplayInfoNone("", "Terminal Empty", 2);
		return 0;
	}
	
	DisplayMsg("", "Rrn", "0", rrn, 12, 12);
	ShowLogs(1, "Rrn: %s", rrn);
	if(strlen(rrn) < 12)
	{
		DisplayInfoNone("", "Short Rrn", 2);
		return 0;
	}
	
	DisplayMsg("", "Auth Code", "0", auth, 6, 6);
	ShowLogs(1, "Auth Code: %s", auth);
	if(strlen(auth) < 6)
	{
		DisplayInfoNone("", "Short Auth Code", 2);
		return 0;
	}

	memset(&glSendPack, 0, sizeof(STISO8583));
	strcpy(glSendPack.szRRN, rrn);
	strcpy(glSendPack.szAuthCode, auth);
	strcpy(glSendPack.szReasonCode, "2300");

	if(parseEodCompletion(glSendPack.szRRN) == 0)
		return 0;

	/*memset(tmp, '\0', strlen(tmp));
	tmp[0] = glSendPack.szRRN[6];
	tmp[1] = glSendPack.szRRN[7];
	tmp[2] = glSendPack.szRRN[8];
	tmp[3] = glSendPack.szRRN[9];
	tmp[4] = glSendPack.szRRN[10];
	tmp[5] = glSendPack.szRRN[11];
	strcpy(glSendPack.szSTAN, tmp);*/

	txnType = 6;
	DisplayInfoNone("", "INSERT CARD", 1);
	
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
			txnType = 6;
			UtilPutEnv("actype", "61");//Refund
			iRet = CompProcICCMsg();
			ShowLogs(1, "Switch ICCARD_MSG Response: %d", iRet);
			PromptRemoveICC();
			ShowLogs(1, "Done with PromptRemoveICC");
			break;
		}
	}
	return 0;
}