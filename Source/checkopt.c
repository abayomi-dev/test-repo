
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal constants declaration *********************/
/********************** Internal functions declaration *********************/
static uchar ChkIfAcqAvail(uchar ucIndex);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/


/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// Check Term Model
uchar ChkTerm(uchar ucTermType)
{
	return (glSysParam.sTermInfo[HWCFG_MODEL]==ucTermType);
}

// Check Term Hardware Config, by Checking the info returned by GetTermInfo() (buffered in glSysParam.sTermInfo[])
// Modified by Kim_LinHB 2014-08-21 v1.01.0004
uchar ChkHardware(uchar ucChkType, uchar ucValue)
{
	PubASSERT(ucChkType<HWCFG_END);

#if !defined(_Dxxx_) || defined(_MIPS_)
	return (glSysParam.sTermInfo[ucChkType]==ucValue);	// return value: TRUE/FALSE
#else
	if(HWCFG_BLTH != ucChkType)
	{
		return (glSysParam.sTermInfo[ucChkType]==ucValue);	// return value: TRUE/FALSE
	}
	else
	{
		return ((glSysParam.sTermInfo[19] & 0x08) == ucValue);
	}
#endif
}

// Check whether it is an IrDA-communication printer
uchar ChkIfIrDAPrinter(void)
{
	return (ChkTerm(_TERMINAL_S60_));
}

// 1.00.0009 delete
//uchar ChkIfSupportCommType(uchar ucCommType)

// check if terminal is using the specific PED mode
uchar ChkTermPEDMode(uchar ucMode)
{
	return (glSysParam.stEdcInfo.ucPedMode==ucMode);
}

// Scan all Acquirer and check whether one of them support EMV
uchar ChkIfEmvEnable(void)
{
#ifdef ENABLE_EMV
	uchar	i;
	//ShowLogs(1, "Inside ChkIfEmvEnable: %d", glSysParam.ucAcqNum);
	for(i=0; i<glSysParam.ucAcqNum; i++)
	{
		//ShowLogs(1, "glSysParam.stAcqList[i].sOption: %s", glSysParam.stAcqList[i].sOption);
		//ShowLogs(1, "ACQ_EMV_FEATURE: %d", ACQ_EMV_FEATURE);
		if( ChkOptionExt(glSysParam.stAcqList[i].sOption, ACQ_EMV_FEATURE) )
		{
			//ShowLogs(1, "Return True");
			return TRUE;
		}
	}
#endif
	//ShowLogs(1, "Return False");
	return FALSE;
}

// Check current Issuer OPtion
uchar ChkIssuerOption(ushort uiOption)
{
	PubASSERT( (uiOption>>8)<sizeof(glCurIssuer.sOption) );
	return ChkOptionExt(glCurIssuer.sOption, uiOption);
}

// Check Current Acquirer option
uchar ChkAcqOption(ushort uiOption)
{
	PubASSERT( (uiOption>>8)<sizeof(glCurAcq.sOption) );
	return ChkOptionExt(glCurAcq.sOption, uiOption);
}

// Check EDC option
uchar ChkEdcOption(ushort uiOption)
{
	PubASSERT( (uiOption>>8)<sizeof(glSysParam.stEdcInfo.sOption) );
	return ChkOptionExt(glSysParam.stEdcInfo.sOption, uiOption);
}

// Extension of option checking
uchar ChkOptionExt(const uchar *psExtOpt, ushort uiOption)
{
	return (psExtOpt[uiOption>>8] & (uiOption & 0xFF));
}

void SetOptionExt(uchar *psExtOptInOut, ushort uiOption)
{
	psExtOptInOut[uiOption>>8] |= (uiOption & 0xFF);
}

// Check whether need PIN for current Issuer TRUE need FALSE no need
uchar ChkIfNeedPIN(void)
{
	return ChkIssuerOption(ISSUER_EN_PIN);
}

uchar ChkIfPinReqdAllIssuer(void)
{
	uchar	i;

	for(i=0; i<glSysParam.ucIssuerNum; i++)
	{
		if( ChkOptionExt(glSysParam.stIssuerList[i].sOption, ISSUER_EN_PIN) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

uchar ChkIfAmex(void)
{
	return ChkAcqOption(ACQ_AMEX_SPECIFIC_FEATURE);
}

// add by lirz v1.01.0007 
uchar ChkIfAmexName(void)
{
	return ChkCurAcqName("AMEX", FALSE);
}

uchar ChkIfBoc(void)
{
	return ChkCurAcqName("BOC", TRUE);
}

// Citibank HK
uchar ChkIfCiti(void)
{
	return ChkCurAcqName("CITI", TRUE);
}

// Fubon Bank
uchar ChkIfFubon(void)
{
	return ChkCurAcqName("FUBO", TRUE);
}

// If now acquirer is DahSing bank
uchar ChkIfDah(void)
{
	return (ChkCurAcqName("DAHSING", TRUE) || ChkCurAcqName("DSB", TRUE));
}

// Bank of East Asia
uchar ChkIfBea(void)
{
	return ChkCurAcqName("BEA", TRUE);
}

// Standard Chartered
uchar ChkIfScb(void)
{
	return ChkCurAcqName("SCB", TRUE);
}

uchar ChkIfWordCard(void)
{
	return ChkCurAcqName("WORLDCARD", TRUE);
}

uchar ChkIfUob(void)
{
	return (memcmp(glCurAcq.szNii, "002", 3)==0);
//	return ChkCurAcqName("UOB", TRUE);
}

uchar ChkIfUobIpp(void)
{
	return ChkCurAcqName("UOB-IPP", TRUE);
}

// Diners for non-specific bank
uchar ChkIfDiners(void)
{
	return ChkCurAcqName("DINERS", FALSE);
}

// JCB for non-specific bank
uchar ChkIfJCB(void)
{
	return ChkCurAcqName("JCB", FALSE);
}

// ICBC(ASIA) in HK
uchar ChkIfICBC(void)
{
	return ChkCurAcqName("ICBC_HK", FALSE);
}

// ICBC Macau branch
uchar ChkIfICBC_MACAU(void)
{
	return ChkCurAcqName("ICBC_MACAU", FALSE);
}

// Wing Hang Bank
uchar ChkIfWingHang(void)
{
    return ChkCurAcqName("WINGHANG", FALSE);
}

// Shanghai Commercial Bank (HK)
uchar ChkIfShanghaiCB(void)
{
    return ChkCurAcqName("SHCB", TRUE);
}

uchar ChkIfHSBC(void)
{
	if( ChkIfAmex()   || ChkIfBoc()  || ChkIfBea() || ChkIfFubon() ||
		ChkIfDah()    || ChkIfCiti() || ChkIfScb() || ChkIfUob()   ||
		ChkIfUobIpp() || ChkIfWordCard() || ChkIfDiners() || ChkIfJCB() ||
		ChkIfICBC() || ChkIfICBC_MACAU() || ChkIfWingHang() || ChkIfShanghaiCB())
	{
		return FALSE;
	}

	return TRUE;
}

// 检查磁卡交易金额是否低于Floor Limit
// Check whether below floor limit. Only for Non-EMV.
uchar ChkIfBelowMagFloor(void)
{
	int		ii;
	uchar	szBuff[12+1];

	sprintf((char *)szBuff, "%lu", glCurIssuer.ulFloorLimit);
	for (ii=0; ii<glSysParam.stEdcInfo.stLocalCurrency.ucDecimal; ii++)
	{
		strcat(szBuff, "0");
	}
	AmtConvToBit4Format(szBuff, glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
	// Now szBuff is local floor limit
	if( memcmp(glProcInfo.stTranLog.szAmount, szBuff, 12)>=0 )
	{
		return FALSE;
	}

	//return TRUE;
	return FALSE;
}

// select the current transaction type automatically if it is not set
void CheckCapture(void)
{
	if( glProcInfo.stTranLog.ucTranType!=SALE_OR_AUTH )
	{
		return;
	}

	if (ChkInstalmentAllAcq() && ChkEdcOption(EDC_ENABLE_INSTALMENT))	// all acquirers enable installment plan
	{
		glProcInfo.stTranLog.ucTranType = INSTALMENT;
	}
	else if( ChkEdcOption(EDC_CASH_PROCESS) )
	{
		glProcInfo.stTranLog.ucTranType = CASH;
	}
	else if( ChkIssuerOption(ISSUER_CAPTURE_TXN) )
	{
		glProcInfo.stTranLog.ucTranType = SALE;
	}
	else
	{
		if( ChkEdcOption(EDC_AUTH_PREAUTH) )
		{
			glProcInfo.stTranLog.ucTranType = AUTH;
		}
		else
		{
			glProcInfo.stTranLog.ucTranType = PREAUTH;
		}
	}
	// Added by Kim_LinHB 2014-8-8 v1.01.0002 bug506
	//TransInit(glProcInfo.stTranLog.ucTranType);//Commented Out by WIsdom
	TransInitPurchase(SALE);
}

uchar ChkInstalmentAllAcq(void)
{
	uchar	sBuff[256];
	uchar	i;

	memset(sBuff, 0, sizeof(sBuff));
	for(i=0; i<glSysParam.ucPlanNum; i++)
	{
		sBuff[glSysParam.stPlanList[i].ucAcqIndex] = 1;
	}

	for(i=0; i<glSysParam.ucAcqNum; i++)
	{
		if( sBuff[i]==0 )
		{
			return FALSE;
		}
	}

	return TRUE;
}

uchar ChkIfDispMaskPan2(void)
{
	return ChkOptionExt(glCurIssuer.sPanMask, 0x0040);
}

uchar ChkIfInstalmentPara(void)
{
	if( !ChkEdcOption(EDC_ENABLE_INSTALMENT) )
	{
		return FALSE;
	}

	return (glSysParam.ucPlanNum > 0);
}

uchar ChkIfTransMaskPan(uchar ucCurPage)
{
	PubASSERT(ucCurPage<4);
	//PubASSERT(ucCurPage>=0 && ucCurPage<4);
	
	if (ChkIfAmex())
	{
		uchar ucTrans;
		ucTrans = glProcInfo.stTranLog.ucTranType;
		if(ucTrans==VOID)
		{
			ucTrans = glProcInfo.stTranLog.ucOrgTranType;
		}

		if( (ucTrans==PREAUTH) || (ucTrans==AUTH) )
		{
			if(ucCurPage != 1)
			{
				return FALSE;
			}
			if(glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT)
			{
				return FALSE;
			}
		}
		if(ucTrans == OFF_SALE)
		{
			return FALSE;
		}
		if(glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT)
		{
			if (ucCurPage!=1)
			{
				return FALSE;
			}
		}
	}
	else
	{
		if ( ((glProcInfo.stTranLog.ucTranType==PREAUTH) || (glProcInfo.stTranLog.ucTranType==AUTH)) &&
			!ChkEdcOption(EDC_AUTH_PAN_MASKING))
		{
			return FALSE;
		}
	}

	return TRUE;
}

// compare acquirer name in upper case
uchar ChkCurAcqName(const void *pszKeyword, uchar ucPrefix)
{
	uchar	szBuff[10+1];

	sprintf((char *)szBuff, "%.10s", glCurAcq.szName);
	PubStrUpper(szBuff);

	if (ucPrefix)	// the specific string is only allowed at the beginning of the acquirer name
	{
		if( memcmp(szBuff, pszKeyword, strlen((char *)pszKeyword))==0 )
		{
			return TRUE;
		}
	} 
	else	// the specific string is allowed at any place within acquirer name string
	{
		if (strstr(szBuff, pszKeyword)!=NULL)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
uchar ChkIfTranAllow(uchar ucTranType)
{
	if( ucTranType==SETTLEMENT || ucTranType==UPLOAD )
	{
		return TRUE;
	}

	if( GetTranLogNum(ACQ_ALL)>=MAX_TRANLOG )
	{
		// Modified by Kim_LinHB 2014-5-31
		Gui_ClearScr();
		Gui_ShowMsgBox(_T("BATCH FULL"), gl_stTitleAttr, _T("PLS SETTLE BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return FALSE;
	}

	if( (glProcInfo.stTranLog.ucTranType==INSTALMENT) && !ChkIfInstalmentPara() )
	{
		return FALSE;
	}

	return TRUE;
}

// Modified by Kim_LinHB 2014-5-31
uchar ChkIfZeroAmt(const uchar *pszIsoAmountStr)
{
	if(!pszIsoAmountStr)
		return TRUE;
	return (memcmp(pszIsoAmountStr, "000000000000",strlen(pszIsoAmountStr))==0);
}

uchar ChkIfBatchEmpty(void)
{
	int	ii;
	for (ii=0; ii<MAX_TRANLOG; ii++)
	{
		if (glSysCtrl.sAcqKeyList[ii] != INV_ACQ_KEY)
		{
			return FALSE;
		}
	}
	return TRUE;
}

uchar ChkIfZeroTotal(const void *pstTotal)
{
	TOTAL_INFO	*pTotal = (TOTAL_INFO *)pstTotal;

	if ( pTotal->uiSaleCnt==0 && pTotal->uiRefundCnt==0 &&
		pTotal->uiVoidSaleCnt==0 && pTotal->uiVoidRefundCnt==0 )
	{
		return TRUE;
	}
	return FALSE;
}

uchar ChkSettle(void)
{
	if( glSysCtrl.sAcqStatus[glCurAcq.ucIndex]==S_PENDING )
	{
		// Modified by Kim_LinHB 2014-5-31
		Gui_ClearScr();
		Gui_ShowMsgBox(_T("SETTLE PENDING"), gl_stTitleAttr, _T("PLS SETTLE BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return FALSE;
	}

	return TRUE;
}

uchar ChkIfNeedTip(void)
{
	if( ChkEdcOption(EDC_TIP_PROCESS) &&
		((glProcInfo.stTranLog.ucTranType==SALE)     ||
		 (glProcInfo.stTranLog.ucTranType==OFF_SALE) ||
		 (glProcInfo.stTranLog.ucTranType==CASH))      )
	{
		return TRUE;
	}

	return FALSE;
}

uchar ChkIfAcqAvail(uchar ucIndex)
{
	return (glSysCtrl.sAcqStatus[ucIndex]!=S_RESET);
}

uchar ChkIfDccBOC(void)	// BOC DCC acquirer
{
	return (ChkCurAcqName("DCC_BOC", TRUE) && ChkIfAcqAvail((uchar)(glCurAcq.ucIndex+1)));
}

uchar ChkIfDccAcquirer(void)
{
	return ChkCurAcqName("DCC", FALSE);
}

uchar ChkIfDccBocOrTas(void)	// !!!! to be applied.
{
	return (ChkCurAcqName("DCC_BOC", TRUE));
}

uchar ChkIfIccTran(ushort uiEntryMode)
{
	if( (uiEntryMode & MODE_CHIP_INPUT) ||
		(uiEntryMode & MODE_FALLBACK_SWIPE) ||
		(uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		return TRUE;
	}

	return FALSE;
}

uchar ChkIfPrnReceipt(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & PRN_RECEIPT);
}

uchar ChkIfNeedReversal(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & NEED_REVERSAL);
}

uchar ChkIfSaveLog(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & WRT_RECORD);
}

uchar ChkIfThermalPrinter(void)
{
	return (glSysParam.stEdcInfo.ucPrinterType==1);
}

uchar ChkIfNeedSecurityCode(void)
{
	if( glProcInfo.stTranLog.ucTranType==OFF_SALE || glProcInfo.stTranLog.ucTranType==VOID )
	{
		return FALSE;
	}

	if( ChkIfAmex() )
	{
		// add by lirz v1.01.0007
		if( glProcInfo.stTranLog.ucTranType==REFUND && !ChkAcqOption(ACQ_ONLINE_REFUND) )
		{
			return FALSE;
		}
	   // end add
	}
	else if( glProcInfo.stTranLog.ucTranType==REFUND )
	{
		return FALSE;
	}

	if( (glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT) ||
		(glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) )
	{
		if( ChkIssuerOption(ISSUER_SECURITY_SWIPE) )
		{
			return TRUE;
		}
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		if( ChkIssuerOption(ISSUER_SECURITY_MANUL) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

uchar ChkIfNeedMac(void)
{
	return FALSE;
}

uchar ChkIfAcqNeedTC(void)
{
	if (ChkIfICBC_MACAU() || ChkIfDah() || ChkIfWingHang())
	{
		return TRUE;
	}
	
	// add by lirz v1.01.0007
	if(ChkIfAmex())
	{
		return TRUE;
	}
	// end add by lirz

	// more banks may need to add in later
	return FALSE;
}

uchar ChkIfAcqNeedDE56(void)
{
    if (ChkIfAcqNeedTC())
    {
        return FALSE;
    }
    return TRUE;
}

// check if allow to exit
uchar ChkIfAllowExit(void)
{
#ifdef _WIN32
	return FALSE;
#else
	APPINFO	stTempAppInf;

	if (ReadAppInfo(0, &stTempAppInf)==0)
	{
		return TRUE;
	}
	return FALSE;
#endif
}

// end of file

