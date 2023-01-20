#include "global.h"

void SetEntryModeBal(const TRAN_LOG *pstLog)
{
	ShowLogs(1, "Modification....");
	sprintf((char *)glSendPack.szEntryMode, "0000");

	if( ChkIfAmex() )
	{
		//ShowLogs(1, "Inside SetEntryMode 1");
		SetAmexEntryModeBal(pstLog);
	}
	else
	{
		//ShowLogs(1, "Inside SetEntryMode 2");
		SetStdEntryModeBal(pstLog);
	}
}

void SetAmexEntryModeBal(const TRAN_LOG *pstLog)
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

void SetStdEntryModeBal(const TRAN_LOG *pstLog)
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

void SetCondCodeBal(void)
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

void SetCommReqFieldBal(void)
{
	if(checkMimic == 1)
		return;


	char temp[200] = {0};
	int len = 0;
	char icc[250] = {0};
	#ifdef ENABLE_EMV
		int	iLength;
	#endif
	uchar	szTotalAmt[12+1];
	
	memset(&glSendPack, 0, sizeof(STISO8583));

	sprintf((char *)glSendPack.szMsgCode, "%s", "0100");
	

	sprintf((char *)glSendPack.szPoscCode, "%.*s", LEN_POSC_CODE, "06");
	ShowLogs(1, "Pos Pin Capture Code: %s", glSendPack.szPoscCode);
	sprintf((char *)glSendPack.szTransFee, "%.*s", LEN_TRANS_FEE, "C00000000");
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
	UtilGetEnv("curcode", temp);
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
	glSysCtrl.ulSTAN = useStan;
	sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);  //??
	glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
	ShowLogs(1, "Stan: %s", glSendPack.szSTAN);

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

	sprintf(glSendPack.szTranAmt, "%s", "000000000000");
	
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
	SetEntryModeBal(&glProcInfo.stTranLog);		// bit 22, entry mode
	SetCondCodeBal();		// bit 25, service condition code 

	ShowLogs(1, "Entry Code: %s", glSendPack.szEntryMode);
	ShowLogs(1, "Condition Code: %s", glSendPack.szCondCode);

	//Resume here
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMCC", temp);
	sprintf((char *)glSendPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, temp);
	ShowLogs(1, "Mcc: %s", glSendPack.szMerchantType);

	sprintf((char *)glSendPack.szRRN, "000000%s", glSendPack.szSTAN);
	ShowLogs(1, "RRN: %s", glSendPack.szRRN);

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

int TransBalSub(void)
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
	SetCommReqFieldBal();
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
	SetCommReqFieldBal();
	return TranProcess();
}

int TransCaptureBal(void)
{
	int	iRet;
	char name[50] = {0};
	glProcInfo.stTranLog.ucTranType = SALE;
	displayName(name);
	SetCurrTitle(_T(name));
	iRet = TransBalSub();
	return iRet;
}

int BalProcICCMsg(void)
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
	iRet = TransCaptureBal();
	ShowLogs(1, "2. Card Details Gotten: %d", iRet);

	if( iRet!=0 )
	{
		ShowLogs(1, "3. Card Details Gotten");
		CommOnHook(FALSE);
		return iRet;
	}
	return 0;
}

int balanceEnquiry()
{
	int iRet = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key;
	int chk = 0;
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

	txnType = 5;
	DisplayInfoNone("", "INSERT CARD", 1);
	
	checkMimic = 0;

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
			txnType = 5;
			UtilPutEnv("actype", "31");//Balance Enquiry
			iRet = BalProcICCMsg();
			ShowLogs(1, "Switch ICCARD_MSG Response: %d", iRet);
			PromptRemoveICC();
			ShowLogs(1, "Done with PromptRemoveICC");
			break;
		}
	}
	return 0;
}

int checkMimic = 0;
int reversalMimic()
{
	int iRet = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key;
	int chk = 0;
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

	//txnType = 5;
	txnType = 7;
	checkMimic = 1;
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
			//txnType = 5;
			txnType = 7;
			checkMimic = 1;
			UtilPutEnv("actype", "31");//Balance Enquiry
			iRet = BalProcICCMsg();
			ShowLogs(1, "Switch ICCARD_MSG Response: %d", iRet);
			PromptRemoveICC();
			ShowLogs(1, "Done with PromptRemoveICC");
			break;
		}
	}
	checkMimic = 0;
	return 0;
}