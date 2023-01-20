#include "global.h"

void formatAmount(char *initAmt, char *finalAmt)
{
	int i, j, k;
	char temp[20] = {0};
	ShowLogs(1, "Init Amount: %s", initAmt);
	for(i = 0, j = 0; i < strlen(initAmt); i++)
	{
		if(initAmt[i] != '0')
		{
			//j = i + 1;
			j = i;
			break;
		}
	}
	strncpy(temp, initAmt + j, strlen(initAmt) - j);
	ShowLogs(1, "Init Amount Temp: %s", temp);
	switch(strlen(temp))
	{
		case 10:
			finalAmt[0] = temp[0];
			finalAmt[1] = ',';
			finalAmt[2] = temp[1];
			finalAmt[3] = temp[2];
			finalAmt[4] = temp[3];
			finalAmt[5] = ',';
			finalAmt[6] = temp[4];
			finalAmt[7] = temp[5];
			finalAmt[8] = temp[6];
			finalAmt[9] = ',';
			finalAmt[10] = temp[7];
			finalAmt[11] = temp[8];
			finalAmt[12] = temp[9];
			finalAmt[13] = '\0';
			break;
		case 9:
			finalAmt[0] = temp[0];
			finalAmt[1] = temp[1];
			finalAmt[2] = temp[2];
			finalAmt[3] = ',';
			finalAmt[4] = temp[3];
			finalAmt[5] = temp[4];
			finalAmt[6] = temp[5];
			finalAmt[7] = ',';
			finalAmt[8] = temp[6];
			finalAmt[9] = temp[7];
			finalAmt[10] = temp[8];
			finalAmt[11] = '\0';
			break;
		case 8:
			finalAmt[0] = temp[0];
			finalAmt[1] = temp[1];
			finalAmt[2] = ',';
			finalAmt[3] = temp[2];
			finalAmt[4] = temp[3];
			finalAmt[5] = temp[4];
			finalAmt[6] = ',';
			finalAmt[7] = temp[5];
			finalAmt[8] = temp[6];
			finalAmt[9] = temp[7];
			finalAmt[10] = '\0';
			break;
		case 7:
			finalAmt[0] = temp[0];
			finalAmt[1] = ',';
			finalAmt[2] = temp[1];
			finalAmt[3] = temp[2];
			finalAmt[4] = temp[3];
			finalAmt[5] = ',';
			finalAmt[6] = temp[4];
			finalAmt[7] = temp[5];
			finalAmt[8] = temp[6];
			finalAmt[9] = '\0';
			break;
		case 6:
			finalAmt[0] = temp[0];
			finalAmt[1] = temp[1];
			finalAmt[2] = temp[2];
			finalAmt[3] = ',';
			finalAmt[4] = temp[3];
			finalAmt[5] = temp[4];
			finalAmt[6] = temp[5];
			finalAmt[7] = '\0';
			break;
		case 5:
			finalAmt[0] = temp[0];
			finalAmt[1] = temp[1];
			finalAmt[2] = ',';
			finalAmt[3] = temp[2];
			finalAmt[4] = temp[3];
			finalAmt[5] = temp[4];
			finalAmt[6] = '\0';
			break;
		case 4:
			finalAmt[0] = temp[0];
			finalAmt[1] = ',';
			finalAmt[2] = temp[1];
			finalAmt[3] = temp[2];
			finalAmt[4] = temp[3];
			finalAmt[5] = '\0';
			break;
		case 3:
		case 2:
		case 1:
		default:
			strcpy(finalAmt, temp);
			break;
	}
	ShowLogs(1, "Amount Formatted: %s", finalAmt);
}

void getBalanceFB(char *data, char *output)
{
	char cCode[4] = {0};
	char decimal[4] = {0};
	char parseAmt[25] = {0};
	char amount[20] = {0};

	strncpy(cCode, data+4, 3);
	strncpy(decimal, data+18, 2);
	strncpy(parseAmt, data+8, 10);

	formatAmount(parseAmt, amount);
	strcpy(output, amount); //Test data
	strcat(output, ".");
	strcat(output, decimal);
	ShowLogs(1, "Balance Leaving: %s", output);
}

void ParseBalance(char *avaAmt, char *legAmt)
{
	char mainData[100] = {0};
	char parseData[500] = { 0 };
	char storeData[500] = { 0 };

	strcpy(mainData, glRecvPack.szAddtAmount);
	ShowLogs(1, "Field 54: %s", mainData);
	memset(parseData, 0x0, sizeof(parseData));
	memset(storeData, 0x0, sizeof(storeData));
	strncpy(parseData, mainData, 20);
	if(strlen(parseData) > 19)
	{
		getBalanceFB(parseData, storeData);
	}

	ShowLogs(1, "Available Balance: %s", storeData);
	strcpy(avaAmt, storeData);


	memset(parseData, 0x0, sizeof(parseData));
	memset(storeData, 0x0, sizeof(storeData));
	strncpy(parseData, mainData + 20, 20);
	if(strlen(parseData) > 19)
	{
		getBalanceFB(parseData, storeData);
	}

	ShowLogs(1, "Ledger Balance: %s", storeData);
	strcpy(legAmt, storeData);
}

// Modified by Kim_LinHB 2014-6-8
int DispPrnReceiptError(int iErrCode)
{
	unsigned char szBuff[100];
	Gui_ClearScr();
	PubBeepErr();
	switch( iErrCode )
	{
	case ERR_PRN_BUSY:
		strcpy(szBuff, _T("PRINTER BUSY"));
		break;

	case ERR_PRN_PAPEROUT:
		strcpy(szBuff, _T("OUT OF PAPER"));
		return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL);
		break;

	case ERR_PRN_WRONG_PACKAGE:
		strcpy(szBuff, _T("PRN DATA ERROR"));
		break;

	case ERR_PRN_OVERHEAT:
		strcpy(szBuff, _T("PRINTER OVERHEAT"));
		break;

	case ERR_PRN_OUTOFMEMORY:
		strcpy(szBuff, _T("PRN OVERFLOW"));
		break;

	default:
		strcpy(szBuff, _T("PRINT FAILED"));
		break;
	}
	return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
}

int PrnCustomLogoReceipt_T(void)
{
	int ret = 0;
	ShowLogs(1, "1. Trying to get logo.bmp");
	if(fexist("logo.bmp") >= 0)
	{
		ShowLogs(1, "logo.bmp exist");
		unsigned char gSigToPrn[20000];
		ShowLogs(1, "PrnCustomLogoReceipt_T 1");
		ret = PrnBmp("logo.bmp", 0, 0, gSigToPrn);
		ShowLogs(1, "PrnCustomLogoReceipt_T 2");
		ShowLogs(1, "Done printing logo: %d", ret);
	}else
	{
		ShowLogs(1, "logo.bmp does not exist");
	}
	return 0;
}


void TrnFormatAmount(char *pasStr, char const *pasAmount, char *cur)
{
    int amt;
    int point = 2;
    int len = 0;
    char tmp[32];

    len = 12 - strspn(pasAmount, "0");

    memset(tmp, 0, sizeof(tmp));

    sprintf(tmp, "%s ", cur);

    if(0 == point)
    {
        if(len > 0)
        {
            sprintf(&tmp[strlen(tmp)], "%s", &pasAmount[12 - len]);
        }
        else
        {
            strcpy(&tmp[strlen(tmp)], "0");
        }
    }
    else if(len <= point)
    {
        amt = atoi(pasAmount);
        sprintf(&tmp[strlen(tmp)], "0.%0*d", point, amt);
    }
    else
    {
        sprintf(&tmp[strlen(tmp)], "%.*s.%.*s", (len - point), &pasAmount[12 - len], point, &pasAmount[12 - point]);
    }
    strcpy(pasStr, tmp);
}


void parseAmount(char* temp, char *store)
{
    char tem[20] = {0};
    memset(tem, '\0', strlen(tem));
    TrnFormatAmount(tem, (char const *)temp, "");
    strcpy(store, tem+1);
    ShowLogs(1, "PARSEAMOUNT AMT: %s", store);
}

//Implement stampduty
void PrnAmountPaymentReceipt(const uchar *pszIndent, uchar ucNeedSepLine, char *temp, int print)
{
	uchar	szBuff[50], szTotalAmt[12+1], szTotalAmt1[12+1], szTotalAmt2[12+1], stro[13], stro2[13];
	uchar   szTempBuff[100];

	ShowLogs(1, "Inside Payments Amount 1");
	App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	memset(szTempBuff, 0, sizeof(szTempBuff));
	strcpy(temp, szBuff);
	//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("AMOUNT PAID:    "), szBuff);
	sprintf((char *)szTempBuff, "%-12.12s %18.18s\n", "AMOUNT PAID:", szBuff);
	if(print == 1)
	{
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
	}
	ShowLogs(1, "Inside Payments Amount 2");
	
	memset(stro, 0, sizeof(stro));
	strcpy(stro, "0000");
	strcat(stro, glSendPack.szTransFee + 1);
	ShowLogs(1, "Formatted Convenience Fee: %s", stro);
	memset(stro2, 0, sizeof(stro2));
	strncpy(stro2, stro, 12);
	App_ConvAmountTran(stro2, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	memset(szTempBuff, 0, sizeof(szTempBuff));
	//strcpy(temp, szBuff);
	sprintf((char *)szTempBuff, "%-16.16s %14.14s\n", "CONVENIENCE FEE:", szBuff);
	if(print == 1)
	{
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
	}
	ShowLogs(1, "Inside Payment Amount 3");
	ShowLogs(1, "2. Formatted: %s", szTempBuff);

	ShowLogs(1, "2. Formatted Convenience Fee: %s", stro);

	PubAscAdd(glProcInfo.stTranLog.szAmount, stro2, 12, szTotalAmt1);
	ShowLogs(1, "Inside Payment Amount 3a");
	App_ConvAmountTran(szTotalAmt1, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	ShowLogs(1, "Inside PayArena Amount 3b");
	memset(szTempBuff, 0, sizeof(szTempBuff));
	//strcpy(temp, szBuff);
	ShowLogs(1, "Inside Payment Amount 8");
	//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("TOTAL AMOUNT:    "), szBuff);
	sprintf((char *)szTempBuff, "%-13.13s %17.17s\n", "TOTAL AMOUNT:", szBuff);
	if(print == 1)
	{
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
	}
	ShowLogs(1, "Inside Payment Amount 3c");
}

//Implement stampduty
void PrnAmountPayArenaReceipt(const uchar *pszIndent, uchar ucNeedSepLine, char *temp, int print)
{
	uchar	szBuff[50], szTotalAmt[12+1], szTotalAmt1[12+1], szTotalAmt2[12+1], stro[13], stro2[13];
	uchar   szTempBuff[100];

	ShowLogs(1, "Inside PayArena Amount 1");
	App_ConvAmountTran(glSendPack.szTranAmtTemp, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	memset(szTempBuff, 0, sizeof(szTempBuff));
	strcpy(temp, szBuff);
	//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("AMOUNT PAID:    "), szBuff);
	sprintf((char *)szTempBuff, "%-12.12s %18.18s\n", "AMOUNT PAID:", szBuff);
	if(print == 1)
	{
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
	}
	ShowLogs(1, "Inside PayArena Amount 2");
	
	memset(stro, 0, sizeof(stro));
	strcpy(stro, "0000");
	strcat(stro, glSendPack.szTransFee + 1);
	ShowLogs(1, "Formatted Convenience Fee: %s", stro);
	memset(stro2, 0, sizeof(stro2));
	strncpy(stro2, stro, 12);
	App_ConvAmountTran(glSendPack.szTranAmtTempConv, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	memset(szTempBuff, 0, sizeof(szTempBuff));
	//strcpy(temp, szBuff);
	//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("CONVENIENCE FEE:"), szBuff);
	sprintf((char *)szTempBuff, "%-16.16s %14.14s\n", "CONVENIENCE FEE:", szBuff);
	if(print == 1)
	{
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
	}
	ShowLogs(1, "Inside PayArena Amount 3");
	ShowLogs(1, "2. Formatted: %s", szTempBuff);

	ShowLogs(1, "2. Formatted Convenience Fee: %s", stro);

	PubAscAdd(glSendPack.szTranAmtTemp, glSendPack.szTranAmtTempConv, 12, szTotalAmt1);
	ShowLogs(1, "Inside PayArena Amount 3a");
	App_ConvAmountTran(szTotalAmt1, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	ShowLogs(1, "Inside PayArena Amount 3b");
	memset(szTempBuff, 0, sizeof(szTempBuff));
	//strcpy(temp, szBuff);
	ShowLogs(1, "Inside PayArena Amount 8");
	//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("TOTAL AMOUNT:    "), szBuff);
	sprintf((char *)szTempBuff, "%-13.13s %17.17s\n", "TOTAL AMOUNT:", szBuff);
	if(print == 1)
	{
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
	}
	ShowLogs(1, "Inside PayArena Amount 3c");
}

//Just to conclude stampduty
void PrnAmountCashBackReceipt(const uchar *pszIndent, uchar ucNeedSepLine, char *tempFin, int print)
{

	char temp[20] = {0};
	char stamp[20] = {0};
	char total[20] = {0};
	uchar   szTempBuff[100];
	char fin[20] = {0};
	char tp[20] = {0};

	memset(temp, '\0', strlen(temp));
	memset(total, '\0', strlen(total));
	memset(stamp, '\0', strlen(stamp));
	parseAmount(glSendPack.szTranAmt, temp);
    double tot = atof(temp);

    UtilGetEnvEx("stampduty", stamp);
    if((strstr(temp, "true") != NULL)
      && (tot >= 1000))
    {
    	//Yet to be clear on this
    	uchar	szBuff[50], szTotalAmt[12+1], szTotalAmt1[12+1], szTotalAmt2[12+1];
		uchar   szTempBuff[100];

		ShowLogs(1, "Inside Cashback Amount 1");
		App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(tempFin, szBuff);

		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("AMOUNT PAID:    "), szBuff);
		sprintf((char *)szTempBuff, "%-12.12s %19.19s\n", "AMOUNT PAID:", szBuff);
		if(print == 1)
		{
			MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		}
		ShowLogs(1, "Inside Cashback Amount 2");
		

		App_ConvAmountTran(glProcInfo.stTranLog.szTipAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(tempFin, szBuff);
		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("CASHBACK AMT:  "), szBuff);
		sprintf((char *)szTempBuff, "%-13.13s %17.17s\n", "CASHBACK AMT:", szBuff);
		if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		ShowLogs(1, "Inside Cashback Amount 3");
		

		App_ConvAmountTran("000000000000", szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(tempFin, szBuff);
		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("CHARGE:        "), szBuff);
		sprintf((char *)szTempBuff, "%-7.7s %23.23s\n", "CHARGE:", szBuff);
		if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		ShowLogs(1, "Inside Cashback Amount 4");

		//PubAscAdd(glSendPack.szTranAmt, "000000000000", 12, szTotalAmt1);
		PubAscAdd(glSendPack.szTranAmt, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt1);
		ShowLogs(1, "Inside Cashback Amount 5: %s", szTotalAmt1);
		ShowLogs(1, "Inside Cashback Amount 6");
		ShowLogs(1, "Inside Cashback Amount 7");

	    App_ConvAmountTran(szTotalAmt1, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(temp, szBuff);
		ShowLogs(1, "Inside Cashback Amount 8");
		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("TOTAL AMOUNT:    "), szBuff);
		sprintf((char *)szTempBuff, "%-13.13s %17.17s\n", "TOTAL AMOUNT:", szBuff);
		if(print == 1)
		{
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		}
    }else
	{
		uchar	szBuff[50], szTotalAmt[12+1], szTotalAmt1[12+1], szTotalAmt2[12+1];
		uchar   szTempBuff[100];

		ShowLogs(1, "Inside Cashback Amount 1");
		App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(tempFin, szBuff);

		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("AMOUNT PAID:    "), szBuff);
		sprintf((char *)szTempBuff, "%-8.8s %22.22s\n", "Amount: ", szBuff);
		if(print == 1)
		{
			MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		}
		ShowLogs(1, "Inside Cashback Amount 2");
		

		App_ConvAmountTran(glProcInfo.stTranLog.szTipAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(tempFin, szBuff);
		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("CASHBACK AMT:  "), szBuff);
		sprintf((char *)szTempBuff, "%-10.10s %20.20s\n", "Cashback: ", szBuff);
		if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		ShowLogs(1, "Inside Cashback Amount 3");
		

		App_ConvAmountTran("000000000000", szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(tempFin, szBuff);
		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("CHARGE:        "), szBuff);
		sprintf((char *)szTempBuff, "%-5.5s %25.25s\n", "Fee: ", szBuff);
		if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		ShowLogs(1, "Inside Cashback Amount 4");

		//PubAscAdd(glSendPack.szTranAmt, "000000000000", 12, szTotalAmt1);
		PubAscAdd(glSendPack.szTranAmt, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt1);
		ShowLogs(1, "Inside Cashback Amount 5: %s", szTotalAmt1);
		ShowLogs(1, "Inside Cashback Amount 6");
		ShowLogs(1, "Inside Cashback Amount 7");

	    App_ConvAmountTran(szTotalAmt1, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(temp, szBuff);
		ShowLogs(1, "Inside Cashback Amount 8");
		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("TOTAL AMOUNT:    "), szBuff);
		sprintf((char *)szTempBuff, "%-7.7s %23.23s\n", "TOTAL: ", szBuff);
		if(print == 1)
		{
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		}
	}
}

void C(char *s, char *out)
{
    sprintf(out, "%*s%*s\n", 16 + strlen(s) / 2, s, 16 - strlen(s) / 2, "");
}

void PrnAmountReceipt(const uchar *pszIndent, uchar ucNeedSepLine, char *tempFin, int print)
{
	char temp[20] = {0};
	char stamp[20] = {0};
	char total[20] = {0};
	uchar   szTempBuff[100];
	char fin[20] = {0};
	char tp[20] = {0};

	memset(temp, '\0', strlen(temp));
	memset(total, '\0', strlen(total));
	memset(stamp, '\0', strlen(stamp));
	parseAmount(glSendPack.szTranAmt, temp);
    double tot = atof(temp);

    UtilGetEnvEx("stampduty", stamp);

    ShowLogs(1, "TOTAL DOUBLE: %lu", tot);
    ShowLogs(1, "TOTAL STAMP: %s", stamp);

    if((strstr(stamp, "true") != NULL)
      && (tot >= 1000))
    {
    	memset(stamp, '\0', strlen(stamp));
    	strcpy(stamp, "000000005000");
        PubAscAdd((const char *)stamp, (const char *)glSendPack.szTranAmt, 12, total);
        
        memset(fin, 0, sizeof(fin));
        memset(tp, 0, sizeof(tp));
        UtilGetEnv("curabbr", tp);
        TrnFormatAmount(fin, (char const *)glSendPack.szTranAmt, tp);
        memset(szTempBuff, 0, sizeof(szTempBuff));
        sprintf((char *)szTempBuff, "%-7.7s %23.23s\n", "AMOUNT:", fin);
        if(print == 1)
		{
			MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		}

        memset(fin, 0, sizeof(fin));
        memset(tp, 0, sizeof(tp));
        UtilGetEnv("curabbr", tp);
        TrnFormatAmount(fin, (char const *)stamp, tp);
        memset(szTempBuff, 0, sizeof(szTempBuff));
        sprintf((char *)szTempBuff, "%-11.11s %19.19s\n", "STAMP DUTY:", fin);
        if(print == 1)
		{
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		}

        memset(temp, 0, sizeof(temp));
        memset(fin, 0, sizeof(fin));
        memset(tp, 0, sizeof(tp));
        UtilGetEnv("curabbr", tp);
        TrnFormatAmount(fin, (char const *)total, tp);
        memset(szTempBuff, 0, sizeof(szTempBuff));

        strcpy(tempFin, fin);

        sprintf((char *)szTempBuff, "%-13.13s %17.17s\n", "TOTAL AMOUNT:", fin);
        if(print == 1)
		{
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		}
    }else
    {
		uchar	szBuff[50], szTotalAmt[12+1];
		uchar   szTempBuff[100];
		ShowLogs(1, "A. Inside Amount: %s", glProcInfo.stTranLog.szAmount);
	    App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(tempFin, szBuff);
		ShowLogs(1, "B. Inside Amount 2: %s", tempFin);
		//sprintf((char *)szTempBuff, "%-7.7s %23.23s\n", "TOTAL: ", szBuff);
		ShowLogs(1, "C. Inside Amount 3: %s", szTempBuff);
		if(print == 1)
		{
			C(szBuff, szTempBuff);
			MultiLngPrnReceiptStr("...............................\n\n", GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("...............................\n\n", GUI_ALIGN_LEFT);
		}
	}
}

void PrnAmountBalanceReceipt(const uchar *pszIndent, uchar ucNeedSepLine, char *temp, int print)
{
	uchar   szTempBuff[100];
	char avaAmt[100] = {0};
	char legAmt[100] = {0};

	ParseBalance(avaAmt, legAmt);

    memset(szTempBuff, 0, sizeof(szTempBuff));
	strcpy(temp, "NGN 0.00");
	//sprintf((char *)szTempBuff, "%s  NGN %12.12s\n",  _T("AVAL. BAL.:    "), avaAmt);
	sprintf((char *)szTempBuff, "%-19.19s %11.11s\n", "Available Balance: ", avaAmt);
	if(print == 1)
	{
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
	}
	
	memset(szTempBuff, 0, sizeof(szTempBuff));
	strcpy(temp, "NGN 0.00");
	//sprintf((char *)szTempBuff, "%s  NGN %12.12s\n",  _T("LEDG. BAL.:    "), avaAmt);
	sprintf((char *)szTempBuff, "%-16.16s %13.13s\n", "Ledger Balance: ", legAmt);
	
	if(print == 1)
	{
		MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
	}
}

int StartPrinterReceipt(void)
{
	uchar	ucRet;

	if (!ChkIfIrDAPrinter())
	{
		int iRet;
		while( 1 )
		{
			//Commented out by Wisdom
			//DispPrinting();
			//PrintOne();
			ucRet = PrnStart();
			if( ucRet==PRN_OK )
			{
				return 0;	// print success!
			}

			iRet = DispPrnReceiptError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_ERR_USERCANCELLED == iRet||
				GUI_ERR_TIMEOUT == iRet)
			{
				Gui_ClearScr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("Pls Reprint"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
				break;
			}
		}
	}
	else
	{
		int iRet;
		SetOffBase(OffBaseCheckPrint);	//????

		//DispPrinting();
		PrnStart();
		//PrintOne();
		while( 1 )
		{
			ucRet = PrnStatus();
			if( ucRet==PRN_OK)
			{
				return PRN_OK;
			}
			else if( ucRet==PRN_BUSY )
			{
				DelayMs(500);
				continue;
			}
			
			iRet = DispPrnReceiptError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_OK != iRet)
			{
				break;
			}
			//DispPrinting();
			PrnStart();
			//PrintOne();
		}
	}

	return ERR_NO_DISP;
}

void MultiLngPrnReceiptStr(const uchar *str, uchar mode)
{
	PrnStr((uchar *)str);
}

void TC(char *s, char *out)
{
    sprintf(out, "   %*s%*s   \n", 12 + strlen(s) / 2, s, 12 - strlen(s) / 2, "");
}



void MaskAllPan(char *pan, char *mskPan)
{
	int iLen, iRet, i = 0, j = 0;
	iLen = strlen(pan);
	iRet = iLen - 4;
	for(i = 0; i < iLen; i++)
	{
		if(i < 6)
		{
			mskPan[j] = pan[i];
			j++;
		}else if(i >= iRet)
		{
			mskPan[j] = pan[i];
			j++;
		}else
		{
			mskPan[j] = '*';
			j++;
		}
	}
}

void PrnAmountReceiptMan(const uchar *pszIndent, uchar ucNeedSepLine, char *tempFin, int print)
{
	char temp[20] = {0};
	char stamp[20] = {0};
	char total[20] = {0};
	uchar   szTempBuff[100];
	char fin[20] = {0};
	char tp[20] = {0};

	memset(temp, '\0', strlen(temp));
	memset(total, '\0', strlen(total));
	memset(stamp, '\0', strlen(stamp));
	parseAmount(glSendPack.szTranAmt, temp);
    double tot = atof(temp);

    UtilGetEnvEx("stampduty", stamp);

    ShowLogs(1, "TOTAL DOUBLE: %lu", tot);
    ShowLogs(1, "TOTAL STAMP: %s", stamp);

    if((strstr(stamp, "true") != NULL)
      && (tot >= 1000))
    {
    	memset(stamp, '\0', strlen(stamp));
    	strcpy(stamp, "000000005000");
        PubAscAdd((const char *)stamp, (const char *)glSendPack.szTranAmt, 12, total);
        
        memset(fin, 0, sizeof(fin));
        memset(tp, 0, sizeof(tp));
        UtilGetEnv("curabbr", tp);
        TrnFormatAmount(fin, (char const *)glSendPack.szTranAmt, tp);
        memset(szTempBuff, 0, sizeof(szTempBuff));
        sprintf((char *)szTempBuff, "%-7.7s %23.23s\n", "AMOUNT:", fin);
        if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);

        memset(fin, 0, sizeof(fin));
        memset(tp, 0, sizeof(tp));
        UtilGetEnv("curabbr", tp);
        TrnFormatAmount(fin, (char const *)stamp, tp);
        memset(szTempBuff, 0, sizeof(szTempBuff));
        sprintf((char *)szTempBuff, "%-11.11s %19.19s\n", "STAMP DUTY:", fin);
        if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);

        memset(temp, 0, sizeof(temp));
        memset(fin, 0, sizeof(fin));
        memset(tp, 0, sizeof(tp));
        UtilGetEnv("curabbr", tp);
        TrnFormatAmount(fin, (char const *)total, tp);
        memset(szTempBuff, 0, sizeof(szTempBuff));

        strcpy(tempFin, fin);

        sprintf((char *)szTempBuff, "%-13.13s %17.17s\n", "TOTAL AMOUNT:", fin);
        if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
    }else
    {
		uchar	szBuff[50], szTotalAmt[12+1];
		uchar   szTempBuff[100];
	    App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		memset(szTempBuff, 0, sizeof(szTempBuff));
		strcpy(temp, szBuff);
		//sprintf((char *)szTempBuff, "%s   %12.12s\n",  _T("AMOUNT PAID:    "), szBuff);
		sprintf((char *)szTempBuff, "%-12.12s %18.18s\n", "AMOUNT PAID:", szBuff);
		if(print == 1)
			MultiLngPrnReceiptStr(szTempBuff, GUI_ALIGN_LEFT);
		strcpy(tempFin, szBuff);
	}
}


int StartPrinterReceiptTest(void)
{
	uchar	ucRet;

	if (!ChkIfIrDAPrinter())
	{
		int iRet;
		while( 1 )
		{
			//Commented out by Wisdom
			//DispPrinting();
			//PrintOne();
			ucRet = PrnStart();
			if( ucRet==PRN_OK )
			{
				return 0;	// print success!
			}

			iRet = DispPrnReceiptError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_ERR_USERCANCELLED == iRet||
				GUI_ERR_TIMEOUT == iRet)
			{
				//Gui_ClearScr();
				//Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("Pls Put Paper"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
				break;
			}
		}
	}
	else
	{
		int iRet;
		SetOffBase(OffBaseCheckPrint);	//????

		//DispPrinting();
		PrnStart();
		//PrintOne();
		while( 1 )
		{
			ucRet = PrnStatus();
			if( ucRet==PRN_OK)
			{
				return PRN_OK;
			}
			else if( ucRet==PRN_BUSY )
			{
				DelayMs(500);
				continue;
			}
			
			iRet = DispPrnReceiptError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_OK != iRet)
			{
				break;
			}
			//DispPrinting();
			PrnStart();
			//PrintOne();
		}
	}

	return ERR_NO_DISP;
}

int testPrinter()
{
	ST_FONT font1,font2;
	font1.CharSet = CHARSET_WEST;
	font1.Width   = 8;
	font1.Height  = 24;
	font1.Bold    = 0;
	font1.Italic  = 0;
	font2.CharSet = CHARSET_WEST;
	font2.Width   = 12;
	font2.Height  = 24;
	font2.Bold    = 0;
	font2.Italic  = 0;

	PrnInit();
	PrnSelectFont(&font2,NULL);
	MultiLngPrnReceiptStr("\n\n", GUI_ALIGN_CENTER);

	return StartPrinterReceiptTest();
}

void Conv2EngTime2(const uchar *pszDateTime, uchar *pszEngTime)
{
    uchar   Month[12][5] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    uchar	ucMonth;

	ucMonth = (uchar)((PubAsc2Long(&pszDateTime[4], 2)-1) % 12);
	sprintf((char *)pszEngTime, "%s %2.2s, %4.4s", Month[ucMonth],
			pszDateTime+6, pszDateTime);
}

void ConvTime(const uchar *pszDateTime, uchar *pszEngTime)
{
	pszEngTime[0] = pszDateTime[0];
    pszEngTime[1] = pszDateTime[1];
    pszEngTime[2] = ':';
    pszEngTime[3] = pszDateTime[2];
    pszEngTime[4] = pszDateTime[3];
    pszEngTime[5] = ':';
    pszEngTime[6] = pszDateTime[4];
    pszEngTime[7] = pszDateTime[5];
    ShowLogs(1, "Final Time: %s", pszEngTime);
}

void changeBiller(char *in, char *out)
{
    int i = 0;
    for(i = 0; i < strlen(in); i++)
    {
        if(in[i] == '^')
            out[i] = '\n';
        else
            out[i] = in[i];
    }
}

int PrintReceiptIcc_T(uchar ucPrnFlag, char *txn, char *store, char *tType)
{	
	uchar	ucNum;
	uchar	szBuff[1000],szBuf1[1000];
	uchar	szIssuerName[1000+1], szTranName[1000+1];
	uchar	temp[1000] = {0};
	int mCpy = 0;
	int cCpy = 0;
	int tCpy = 0;
	char nCpy[5] = {0};

	uchar	szLocalTime[14+1] = {0};
	memset(szLocalTime, '\0', strlen(szLocalTime));
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);

	ST_FONT font1,font2;
	font1.CharSet = CHARSET_WEST;
	font1.Width   = 8;
	font1.Height  = 24;
	font1.Bold    = 0;
	font1.Italic  = 0;
	font2.CharSet = CHARSET_WEST;
	font2.Width   = 12;
	font2.Height  = 24;
	font2.Bold    = 0;
	font2.Italic  = 0;

	memset(nCpy, '\0', strlen(nCpy));
	UtilGetEnv("rptpccn", nCpy);
	cCpy = atoi(nCpy);
	memset(nCpy, '\0', strlen(nCpy));
	UtilGetEnv("rptpmcn", nCpy);
	mCpy = atoi(nCpy);
	tCpy = mCpy + cCpy;

	for(ucNum=0; ucNum<tCpy; ucNum++)
	{
		//ShowLogs(1, "James Printing 1");
		if(ucNum == 1)
		{
			if( memcmp(glProcInfo.stTranLog.szRspCode, "00", LEN_RSP_CODE) != 0 )
			{
				return 0;
			}
		}

		PrnInit();
		memset(temp, 0, sizeof(temp));
		UtilGetEnvEx("rptshowlogo", temp);
		if(strstr(temp, "true") != NULL)
		{
			//PrnSetNormal();//For font size
			PrnCustomLogoReceipt_T();//For Logo
			PrnStep(10);//Set Space after printing logo
		}
		PrnSelectFont(&font2,NULL);
		

		if((ucNum < cCpy) && (cCpy != 0))
		{
			memset(temp, 0, sizeof(temp));
			memset(szBuff, 0, sizeof(szBuff));
			UtilGetEnv("rptcclabel", temp);
			C(temp, szBuff);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
		}
		else
		{
			memset(temp, 0, sizeof(temp));
			memset(szBuff, 0, sizeof(szBuff));
			UtilGetEnv("rptmclabel", temp);
			C(temp, szBuff);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
		}

		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("bankname", temp);
		C(temp, szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);

		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("merName", temp);
		MultiLngPrnReceiptStr(temp, GUI_ALIGN_LEFT);
		PrnStr("\n");
		//ShowLogs(1, "James Printing 6");
		
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("merAddr", temp);
		MultiLngPrnReceiptStr(temp, GUI_ALIGN_LEFT);

		PrnStep(5);
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnvEx("tid", temp);
		sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TERMINAL ID:", temp);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		
		PrnStep(5);
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnvEx("txnMid", temp);
		sprintf((char *)szBuff, "%-12.12s %18.18s\n", "MERCHANT ID:", temp);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnvEx("contactphone", temp);
		sprintf((char *)szBuff, "%-6.6s %24.24s\n", "PHONE:", temp);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		PrnStr("\n\n");
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStep(5);
		memset(temp, 0, sizeof(temp));
		if(rvApr == 1)
			strcpy(temp, "ERROR OCCURRED");
		else
		{
			getResponse(glProcInfo.stTranLog.szRspCode, temp);

		if(strstr(glProcInfo.stTranLog.szRspCode, "00") == NULL)
		{
		PubStrUpper("Failed ");
		}	
		PubStrUpper(temp);
		}


		memset(szBuff, 0, sizeof(szBuff));
		//memset(temp, '\0', strlen(temp));
		C(temp, szBuff);
		
			//UtilGetEnv("paytype", temp);
			/*if(strstr(temp, "CASH") != NULL)
			{
				MultiLngPrnReceiptStr("        CASH APPROVED", GUI_ALIGN_CENTER);
			}
			else
			{
				MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
			}*/
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStr("\n\n");
		PrnStep(5);
		memset(szBuff, 0, sizeof(szBuff));
if(PURCHASETYPE==3) 
			sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TRANSACTION:", "JAMB PIN");
		//sprintf(txn,"%s","JAMB PIN");
		if(PURCHASETYPE==4) 
			sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TRANSACTION:", "EEDC DISCO");
		//sprintf(txn,"%s","EEDC DISCO");
		if(PURCHASETYPE==1) //sprintf(txn,"%s","PAY ATITUDE");
			sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TRANSACTION:", "PAY ATITUDE");

if(PURCHASETYPE==0) //sprintf(txn,"%s","PAY ATITUDE");
			//sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TRANSACTION:", "Purchase");
		sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TRANSACTION:", txn);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		
		PrnStep(5);
		memset(szBuff, 0, sizeof(szBuff));

		if((PURCHASETYPE==1)||(PURCHASETYPE==2)){
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "PHONE NUMBER:", PAYATT_PHONENO);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			//kouhon
		}

		if(PURCHASETYPE==3)
		{
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Customer AccountNumber:",JAMB_CODE);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);

			sprintf((char *)szBuff, "%-13.13s %24.24s\n", "Cus-Details:",jamb_customerName);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);

			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "External Ref:",JAMB_CUS_ID);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
		}
		if(PURCHASETYPE==4){
			
			memset(temp, 0, sizeof(temp));
			memset(szBuff, 0, sizeof(szBuff));
			UtilGetEnv("eedctype", temp);

			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Meter Type:",temp);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);

			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Meter Number:",Cel_TinAccountNumber);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Customer Phone:",eedc_phone);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Tran Ref:",eedc_transaction_reference);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Units:",eedc_units);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Arrears:",eedc_appliedToArrears);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%s  %s\n", "Token:",eedc_token,Cel_Token);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Vat:",eedc_vat);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %20.20s\n", "Customer Name:",eedc_customerName);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Convenience:",eedc_convenience);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			sprintf((char *)szBuff, "%-13.13s %14.14s\n", "Total:",eedc_total);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			PrnStep(5);
			}

		MultiLngPrnReceiptStr(_T("RESPONSE CODE: "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", glProcInfo.stTranLog.szRspCode);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(_T("AUTHCODE:      "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", glProcInfo.stTranLog.szAuthCode);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(_T("STAN:          "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", glRecvPack.szSTAN);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(_T("RRN:           "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", glProcInfo.stTranLog.szRRN);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		memset(szBuf1, 0, sizeof(szBuf1));
		Conv2EngTime2(glProcInfo.stTranLog.szDateTime, temp);
		ConvTime(glSendPack.szLocalTime, szBuff);
		sprintf((char *)szBuf1, "%-12.12s %18.18s\n", temp, szBuff);
		MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
		
		//ShowLogs(1, "James Printing 21");
		switch(txnType)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 6:
			case 7:
			case 12:
			case 13:
			case 18:
				memset(temp, 0, sizeof(temp));
				PrnSelectFont(&font1, NULL);
				PrnAmountReceipt((uchar *)"", TRUE, temp, 1); 
				break;
			case 5:
				memset(temp, 0, sizeof(temp));
				PrnSelectFont(&font1, NULL);
				PrnAmountBalanceReceipt((uchar *)"", TRUE, temp, 1); 
				strcat(store, temp);//Amt
				strcat(store, "|");
				strcat(store, "|");
				strcat(store, "|");
				//strcat(store, "\n");
				break;
			case 8:
				ShowLogs(1, "Inside Case 8");
				memset(temp, 0, sizeof(temp));
				PrnSelectFont(&font1, NULL);
				PrnAmountCashBackReceipt((uchar *)"", TRUE, temp, 1); 
				strcat(store, temp);
				strcat(store, "|");
				strcat(store, "|");
				strcat(store, "|");
				//strcat(store, "\n");
				break;
			case 14:
				ShowLogs(1, "Inside Payments");
				memset(temp, 0, sizeof(temp));
				PrnSelectFont(&font1, NULL);
				PrnAmountPaymentReceipt((uchar *)"", TRUE, temp, 1); 
				strcat(store, temp);
				strcat(store, "|");
				strcat(store, "|");
				strcat(store, "|");
				//strcat(store, "\n");
				break;
			case 15:
			case 16:
			case 17:
				ShowLogs(1, "Inside PayAreana");
				memset(temp, 0, sizeof(temp));
				PrnSelectFont(&font1, NULL);
				PrnAmountPayArenaReceipt((uchar *)"", TRUE, temp, 1); 
				break;
			default:
				break;
		}
		//ShowLogs(1, "James Printing 22");
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		memset(szBuf1, 0, sizeof(szBuf1));
		MaskAllPan(glProcInfo.stTranLog.szPan, szBuff);
		sprintf((char *)szBuf1, "%-3.3s %27.27s\n", "PAN:", szBuff);
		MultiLngPrnReceiptStr(szBuf1,GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "EXPIRY DATE:              %2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2], glProcInfo.stTranLog.szExpDate);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		PrnStr("\n");



		// ADD CARD SCHEME TO THE PRINTOUT
		//MultiLngPrnReceiptStr(szBuf1,GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "CARD SCHEME:              %.12s", &glProcInfo.stTranLog.szAppLabel);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		PrnStr("\n");
		//
		if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
		{
			memset(temp, 0, sizeof(temp));
			memset(szBuff, 0, sizeof(szBuff));
			C("VERIFICATION METHOD: OFFLINE PIN", szBuff);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		}else 
		{
			memset(temp, 0, sizeof(temp));
			memset(szBuff, 0, sizeof(szBuff));
			C("VERIFICATION METHOD: ONLINE PIN", szBuff);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		}
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStr("\n");
		MultiLngPrnReceiptStr(_T("PRINTER NUMBER: "), GUI_ALIGN_LEFT);
		memset(temp, 0, sizeof(temp));
		sprintf((char *)temp, "%lu", RctNum);
		memset(szBuff, 0, sizeof(szBuff));
		sprintf(szBuff,"%15.8s\n", temp);
		
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));

		sprintf((char *)szBuf1, "%-8.8s %22.22s\n", "APP NAME:", "ARCAPAY");
		MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
		PrnStep(5);

		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuf1, "%-8.8s %22.22s\n", "VERSION:", "v1.0");
		MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);


		memset(szBuff, 0, sizeof(szBuff));
		C("Powered By ARCAPAY", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		memset(szBuff, 0, sizeof(szBuff));
		C("+234-01-4538207", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		C("5 Furo Ezimora St, Lekki Phase I 106104, Lekki, Lagos", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		memset(szBuff, 0, sizeof(szBuff));
		C("support@arcang.com", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

	
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("rptfootertext", temp);
		C(temp, szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("rptfnlabel", temp);
		C(temp, szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStr("\n\n");

		ShowLogs(1, "TEST STORE: %s", store);
		
		memset(temp, 0, sizeof(temp));
		UtilGetEnvEx("rptsbarcode", temp);
		if(strstr(temp, "true") != NULL)
		{
			PrnCode128(glSendPack.szRRN, 12, 130, 2);
			PrnStep(10);//Set Space after printing logo
		}
		PrnStr("\n\n\n");

		//ShowLogs(1, "James Printing 29");
		if(1)
		{
			StartPrinterReceipt();
			if(txnType == 5)
    			return 0;

			if( ucNum==0)
			{
				//if( memcmp(glProcInfo.stTranLog.szRspCode, "00", LEN_RSP_CODE) != 0 )
				//	return 0;
	            kbflush();
				Gui_ClearScr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PRESS ANY BUTTON"), gl_stCenterAttr, GUI_BUTTON_NONE, USER_OPER_TIMEOUT, NULL);
			}
		}
		PrnStr("\n\n");
	}
	return 0;
}

int ext = 0;
int PrintTxn(uchar ucPrnFlag, char* txn, char *store, char *tType)
{
	if(titi == 1)
	{
		ShowLogs(1, "Artee Crisis");
		//titi = 0;
		//return 0;
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", "87");
		sprintf(glRecvPack.szSTAN, "%s", glSendPack.szSTAN);
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", "NA");
		sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glSendPack.szRRN);
		sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
		strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);
	}
	//ShowLogs(1, "James Printing Inside 1");
	ext = 0;
	if(1)	
		DisplayInfoNone("", "PRINTING RECEIPT", 0);
	
	PrintReceiptIcc_T(ucPrnFlag, txn, store, tType);
	if(ext == 1)
	{
		ext = 0;
	}
	if(titi == 1)
	{
		titi = 0;
		memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
		memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
		memset(glRecvPack.szRspCode, 0x0, sizeof(glRecvPack.szRspCode));
		memset(glRecvPack.szSTAN, 0x0, sizeof(glRecvPack.szSTAN));
	}

	return 0;
}

void PrintAllReceipt(uchar ucPrnFlag)
{
	char store[2 * 1024] = {0};
	memset(store, '\0', strlen(store));
	char temp[1024] = {0};
//CASH RESPONSE SHOULD 
	memset(temp, '\0', strlen(temp));


	UtilGetEnv("paytype", temp);
	if(strstr(temp, "CASH") != NULL)
	{
		getResponse("00", temp);
	}
	else
	{
		memset(temp, '\0', strlen(temp));			
		getResponse(glProcInfo.stTranLog.szRspCode, temp);
	}
	PubStrUpper(temp);
	DisplayInfoNone("RESPONSE", temp, 3);

	if(revSend)
	{
		ShowLogs(1, "Came from Reversal, No Printing");
		//If from a reversed txn dont print
		//Print now
		revSend = 0;
		//return;
	}

	switch(txnType)
	{
		case 1:
			PrintTxn(ucPrnFlag, "PURCHASE", store, "CHIP");
			break;
		case 2:
			PrintTxn(ucPrnFlag, "CASH ADVANCE", store, "CHIP");
			break;
		case 3:
			PrintTxn(ucPrnFlag, "PREAUTH", store, "CHIP");
			break;
		case 4:
			PrintTxn(ucPrnFlag, "REFUND", store, "CHIP");
			break;
		case 5:
			PrintTxn(ucPrnFlag, "BALANCE ENQUIRY", store, "CHIP");
			break;
		case 6:
			PrintTxn(ucPrnFlag, "SALES COMPLETION", store, "CHIP");
			break;
		case 7:
			PrintTxn(ucPrnFlag, "REVERSAL", store, "MANUAL");
			break;
		case 8:
			PrintTxn(ucPrnFlag, "CASH BACK", store, "CHIP");
			break;
		case 15:
			PrintTxn(ucPrnFlag, "ARCAPAY", store, "CHIP");
			txnType = 0;
			break;
		case 16:
			PrintTxn(ucPrnFlag, "CASH DEPOSIT", store, "CHIP");
			txnType = 0;
			break;
		case 17:
			PrintTxn(ucPrnFlag, "TRANSFER", store, "CHIP");
			txnType = 0;
			break;
		case 18:
			PrintTxn(ucPrnFlag, "CASH WITHDRAWAL", store, "CHIP");
			txnType = 0;
			break;
		case 19:
			PrintTxn(ucPrnFlag, "PAY ATTITUDE", store, "CHIP");
			txnType = 0;
			break;
		case 20:
			PrintTxn(ucPrnFlag, "JAMB PAYMENT", store, "CHIP");
			txnType = 0;
			break;
		case 21:
			PrintTxn(ucPrnFlag,"EEDC PAYMENT", store, "CHIP");
			txnType = 0;
		default:
			break;
	}

	
	//PackCallHomeData();
}

//Start Store
int PrintReceiptStore(char *txn, char *store, char *tType)
{	
	uchar	ucNum;
	uchar	szBuff[1000],szBuf1[1000];
	uchar	szIssuerName[1000+1], szTranName[1000+1];
	uchar	temp[1000] = {0};

	uchar	szLocalTime[14+1] = {0};
	memset(szLocalTime, '\0', strlen(szLocalTime));
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);

	GetReceiptNumber();
	
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	memset(temp, 0, sizeof(temp));
	sprintf((char *)temp, "%lu", RctNum);
	strcpy(store, temp);//Receipt Number
	strcat(store, "|");
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%15.8s\n", temp);
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	//ShowLogs(1, "James Printing 9");
	memset(szBuff, 0, sizeof(szBuff));
	TC(txn, szBuff);
	strcat(store, txn);//Transaction Type
	strcat(store, "|");
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("tid", temp);
	sprintf(szBuff,"%15.8s\n", temp);
	sprintf((char *)szBuff, "%s", _T("DATE:              "));
	Conv2EngTime2(glProcInfo.stTranLog.szDateTime, szBuff);
	strcat(store, szBuff);//Txn Date
	strcat(store, "|");
	memset(szBuf1, 0, sizeof(szBuf1));
	sprintf((char *)szBuff, "%s", _T("TIME:                  "));
	ShowLogs(1, "Local Time: %s", glSendPack.szLocalTime);
	memset(szBuff, 0, sizeof(szBuff));
	ConvTime(glSendPack.szLocalTime, szBuff);//Time
	ShowLogs(1, "James Printing 14: %s", szBuff);
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	strcat(store, szBuff);
	strcat(store, "|");
	//PrnStr("\n");
	memset(szBuf1, 0, sizeof(szBuf1));
	strcat(store, glProcInfo.stTranLog.szAppLabel);//App Name
	strcat(store, "|");
	sprintf((char *)szBuf1, "CARD:              %.12s\n", glProcInfo.stTranLog.szAppLabel);
	//MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
	ShowLogs(1, "James Printing 16");
	memset(szBuff, 0, sizeof(szBuff));
	sprintf((char *)szBuff, "EXPIRY DATE:              %2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2], glProcInfo.stTranLog.szExpDate);
	strcat(store, szBuff);//Card Expiry Date
	strcat(store, "|");
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	ShowLogs(1, "James Printing 17");
	//PrnStr("\n");
	memset(szBuff, 0, sizeof(szBuff));
	MaskAllPan(glProcInfo.stTranLog.szPan, szBuff);
	sprintf((char *)szBuf1, "PAN:    %23.23s", szBuff);
	//ShowLogs(1, "James Printing 18");
	strcat(store, szBuf1);//Masked Pan
	strcat(store, "|");
	//MultiLngPrnReceiptStr(szBuf1,GUI_ALIGN_LEFT);
	//ShowLogs(1, "James Printing 19");
	//PrnStr("\n");
	if(txnType != 7)
	{
		sprintf((char *)szBuff, "%s       ", _T("CLIENT:  "));
		//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		strcat(store, glProcInfo.stTranLog.szHolderName);//Card Holder Name
		strcat(store, "|");
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "%15.15s\n", glProcInfo.stTranLog.szHolderName);
		//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		ShowLogs(1, "James Printing 17 A");
	}else
	{
		strcat(store, "|");
		ShowLogs(1, "James Printing 17 B");
	}
	//ShowLogs(1, "James Printing 20");
	memset(szBuff, 0, sizeof(szBuff));
	strcpy(szBuff, glProcInfo.stTranLog.sAID);
	PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
	PubTrimTailChars(szBuff, 'F');
	memset(szBuf1, 0, sizeof(szBuf1));
	strcat(store, szBuff);
	strcat(store, "|");
	sprintf((char *)szBuf1, "AID:             %.18s\n", szBuff);
	//MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
	ShowLogs(1, "James Printing 21");
	switch(txnType)
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
		case 7:
		case 12:
		case 13:
		case 18:
			ShowLogs(1, "Inside Case 13");
			memset(temp, 0, sizeof(temp));
			PrnAmountReceipt((uchar *)"", TRUE, temp, 0); 
			strcat(store, temp);//Amt
			strcat(store, "|");
			strcat(store, "|");
			strcat(store, "|");
			//strcat(store, "\n");
			break;
		case 5:
			ShowLogs(1, "Inside Case 5");
			memset(temp, 0, sizeof(temp));
			//PrnSelectFont(&font1, NULL);
			PrnAmountBalanceReceipt((uchar *)"", TRUE, temp, 0); 
			strcat(store, temp);//Amt
			strcat(store, "|");
			strcat(store, "|");
			strcat(store, "|");
			//strcat(store, "\n");
			break;
		case 8:
			ShowLogs(1, "Inside Case 8");
			memset(temp, 0, sizeof(temp));
			//PrnSelectFont(&font1, NULL);
			PrnAmountCashBackReceipt((uchar *)"", TRUE, temp, 0); 
			strcat(store, temp);
			strcat(store, "|");
			strcat(store, "|");
			strcat(store, "|");
			//strcat(store, "\n");
			break;
		case 14:
			ShowLogs(1, "Inside Payments");
			memset(temp, 0, sizeof(temp));
			PrnAmountPaymentReceipt((uchar *)"", TRUE, temp, 0); 
			strcat(store, temp);
			strcat(store, "|");
			strcat(store, "|");
			strcat(store, "|");
			//strcat(store, "\n");
			break;
		case 15:
			ShowLogs(1, "Inside PayAreana");
			memset(temp, 0, sizeof(temp));
			PrnAmountPayArenaReceipt((uchar *)"", TRUE, temp, 0); 
			strcat(store, temp);
			strcat(store, "|");
			strcat(store, "|");
			strcat(store, "|");
			//strcat(store, "\n");
			break;
		default:
			break;
	}
	ShowLogs(1, "James Printing 22");
	//PrnStep(5);
	//MultiLngPrnReceiptStr("===============================\n", GUI_ALIGN_LEFT);
	//PrnStep(5);
	memset(temp, 0, sizeof(temp));
	getResponse(glProcInfo.stTranLog.szRspCode, temp);
	PubStrUpper(temp);
	memset(szBuff, 0, sizeof(szBuff));
	C(temp, szBuff);
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	//PrnStep(5);
	//MultiLngPrnReceiptStr("===============================\n", GUI_ALIGN_LEFT);
	//PrnStep(10);
	memset(szBuff, 0, sizeof(szBuff));
	//MultiLngPrnReceiptStr(_T("RESPONSE CODE: "), GUI_ALIGN_LEFT);
	strcat(store, glProcInfo.stTranLog.szRspCode);//Response Code
	strcat(store, "|");
	sprintf(szBuff,"%16.16s\n", glProcInfo.stTranLog.szRspCode);
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	memset(szBuff, 0, sizeof(szBuff));
	//MultiLngPrnReceiptStr(_T("AUTHCODE:      "), GUI_ALIGN_LEFT);
	strcat(store, glProcInfo.stTranLog.szAuthCode);//Auth Code
	strcat(store, "|");
	sprintf(szBuff,"%16.16s\n", glProcInfo.stTranLog.szAuthCode);
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	ShowLogs(1, "James Printing 25");
	memset(szBuff, 0, sizeof(szBuff));
	//MultiLngPrnReceiptStr(_T("STAN:          "), GUI_ALIGN_LEFT);
	strcat(store, glRecvPack.szSTAN);
	strcat(store, "|");
	sprintf(szBuff,"%16.16s\n", glRecvPack.szSTAN);
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	ShowLogs(1, "James Printing 26");
	memset(szBuff, 0, sizeof(szBuff));
	//MultiLngPrnReceiptStr(_T("RRN:           "), GUI_ALIGN_LEFT);
	strcat(store, glSendPack.szRRN);//Rrn
	strcat(store, "|");
	strcat(store, "\n");
	sprintf(szBuff,"%16.16s\n", glProcInfo.stTranLog.szRRN);
	//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	ShowLogs(1, "James Printing 27F");

	ShowLogs(1, "DATA GOING IN: %s", store);
	//PrnStr("\n");
	return 0;
}

void storeprint()
{
	char store[2 * 1024] = {0};
	memset(store, '\0', strlen(store));
	switch(txnType)
	{
		case 1:
			ShowLogs(1, "INSIDE BREAK 1");
			PrintReceiptStore("PURCHASE", store, "CHIP");
			ShowLogs(1, "INSIDE BREAK 1A");
			storeReprint(store);
			ShowLogs(1, "INSIDE BREAK 1B");
			break;
		case 2:
			PrintReceiptStore("CASH ADVANCE", store, "CHIP");
			storeReprint(store);
			break;
		case 3:
			PrintReceiptStore("PREAUTH", store, "CHIP");
			storeReprint(store);
			break;
		case 4:
			PrintReceiptStore("REFUND", store, "CHIP");
			storeReprint(store);
			break;
		case 5:
			PrintReceiptStore("BALANCE ENQUIRY", store, "CHIP");
			break;
		case 6:
			PrintReceiptStore("SALES COMPLETION", store, "CHIP");
			storeReprint(store);
			break;
		case 7:
			PrintReceiptStore("REVERSAL", store, "MANUAL");
			storeReprint(store);
			break;
		case 8:
			PrintReceiptStore("CASH BACK", store, "CHIP");
			storeReprint(store);
			break;
		case 15:
			PrintReceiptStore("ARCAPAY", store, "CHIP");
			storeReprint(store);
			break;
		case 16:
			PrintReceiptStore("CASH DEPOSIT", store, "CHIP");
			storeReprint(store);
			break;
		case 17:
			PrintReceiptStore("TRANSFER", store, "CHIP");
			storeReprint(store);
			break;
		case 18:
			PrintReceiptStore("CASH WITHDRAWAL", store, "CHIP");
			storeReprint(store);
			break;
		default:
			ShowLogs(1, "INSIDE BREAK");
			break;
	}
}
//End Store

//Etop
//recieptno|txntype|date|time|appname|expdate|maskedpan|holdername
//|aid|amount|cashback|total
//|paymentdetails|rspcode|authcode|stan|rrn or Ref
//|shortcode|source|refcode|ussdfee|ussdamt
void reprintDataNormal(struct PRINTDATA *details)
{
	uchar	ucNum;
	uchar	szBuff[1000],szBuf1[1000];
	uchar	szIssuerName[1000+1], szTranName[1000+1];
	uchar	temp[1000] = {0};

	ST_FONT font1,font2;
	font1.CharSet = CHARSET_WEST;
	font1.Width   = 8;
	font1.Height  = 24;
	font1.Bold    = 0;
	font1.Italic  = 0;
	font2.CharSet = CHARSET_WEST;
	font2.Width   = 12;
	font2.Height  = 24;
	font2.Bold    = 0;
	font2.Italic  = 0;

	ShowLogs(1, "Printing");

	DispPrinting();

	for(ucNum=0; ucNum<1; ucNum++)
	{
		PrnInit();
		memset(temp, 0, sizeof(temp));
		UtilGetEnvEx("rptshowlogo", temp);
		if(strstr(temp, "true") != NULL)
		{
			PrnCustomLogoReceipt_T();//For Logo
			PrnStep(10);//Set Space after printing logo
		}
		PrnSelectFont(&font2,NULL);
		

		C("REPRINT COPY", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
		
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("bankname", temp);
		C(temp, szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);

		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("merName", temp);
		MultiLngPrnReceiptStr(temp, GUI_ALIGN_LEFT);
		PrnStr("\n");
		//ShowLogs(1, "James Printing 6");
		
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnv("merAddr", temp);
		MultiLngPrnReceiptStr(temp, GUI_ALIGN_LEFT);

		PrnStep(5);
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnvEx("tid", temp);
		sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TERMINAL ID:", temp);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		
		PrnStep(5);
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnvEx("txnMid", temp);
		sprintf((char *)szBuff, "%-12.12s %18.18s\n", "MERCHANT ID:", temp);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		UtilGetEnvEx("contactphone", temp);
		sprintf((char *)szBuff, "%-6.6s %24.24s\n", "PHONE:", temp);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		PrnStr("\n\n");
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStep(5);
		memset(temp, 0, sizeof(temp));


		getResponse(details->field12, temp);
		if(strstr(temp, "Approved..") == NULL)
	{
		PubStrUpper("Failed ");
	}
		PubStrUpper(temp);
		memset(szBuff, 0, sizeof(szBuff));
		C(temp, szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStr("\n\n");
		
		
		PrnStep(5);
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "%-12.12s %18.18s\n", "TRANSACTION:", details->field1);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		PrnStep(5);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(_T("RESPONSE CODE: "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", details->field12);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(_T("AUTHCODE:      "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", details->field13);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(_T("STAN:          "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", details->field14);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(_T("RRN:           "), GUI_ALIGN_LEFT);
		sprintf(szBuff,"%16.16s\n", details->field15);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		
		
		memset(temp, 0, sizeof(temp));
		memset(szBuff, 0, sizeof(szBuff));
		memset(szBuf1, 0, sizeof(szBuf1));
		sprintf((char *)szBuf1, "%-12.12s %18.18s\n", details->field2, details->field3);
		MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
		
		if(strncmp(details->field1, "CASHBACK", 8) == 0)
		{
			memset(szBuff, 0, sizeof(szBuff));
			sprintf((char *)szBuff, "%-12.12s %18.18s\n", "AMOUNT PAID:", details->field9);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			memset(szBuff, 0, sizeof(szBuff));
			sprintf((char *)szBuff, "%-9.9s %21.21s\n", "CASHBACK:", details->field10);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			memset(szBuff, 0, sizeof(szBuff));
			sprintf((char *)szBuff, "%-7.7s %23.23s\n", "CHARGE:", "NGN 0.00");
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			memset(szBuff, 0, sizeof(szBuff));
			sprintf((char *)szBuff, "%-13.13s %17.17s\n", "TOTAL AMOUNT:", details->field11);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		}else
		{
			memset(szBuff, 0, sizeof(szBuff));
			C(details->field9, szBuff);
			//sprintf((char *)szBuff, "%-12.12s %18.18s\n", "AMOUNT PAID:", details->field9);
			MultiLngPrnReceiptStr("...............................\n\n", GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("...............................\n\n", GUI_ALIGN_LEFT);
		}
		ShowLogs(1, "Field 5: %s", details->field5);
		ShowLogs(1, "Field 6: %s", details->field6);
		ShowLogs(1, "Field 7: %s", details->field7);
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		memset(szBuf1, 0, sizeof(szBuf1));
		MultiLngPrnReceiptStr(details->field6,GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));
		MultiLngPrnReceiptStr(details->field5, GUI_ALIGN_LEFT);
		PrnStr("\n");
		
		memset(szBuff, 0, sizeof(szBuff));
		C("VERIFICATION METHOD: PIN", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStr("\n");
		MultiLngPrnReceiptStr(_T("PRINTER NUMBER: "), GUI_ALIGN_LEFT);
		memset(temp, 0, sizeof(temp));
		sprintf((char *)temp, "%s", details->field0);
		memset(szBuff, 0, sizeof(szBuff));
		sprintf(szBuff,"%15.8s\n", temp);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		memset(szBuff, 0, sizeof(szBuff));

		sprintf((char *)szBuf1, "%-8.8s %22.22s\n", "APP NAME:", "ARCAPAY");
		MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
		PrnStep(5);

		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuf1, "%-8.8s %22.22s\n", "VERSION:", "v1.0");
		MultiLngPrnReceiptStr(szBuf1, GUI_ALIGN_LEFT);
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);

		//memset(szBuff, 0, sizeof(szBuff));
		//C("PTSP: ARCA", szBuff);
		//MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		
		memset(szBuff, 0, sizeof(szBuff));
		C("Powered By ARCAPAY", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		memset(szBuff, 0, sizeof(szBuff));
		C("+234-01-4538207", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	
		memset(szBuff, 0, sizeof(szBuff));
		C("5 Furo Ezimora St, Lekki Phase I 106104, Lekki, Lagos", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

		memset(szBuff, 0, sizeof(szBuff));
		C("support@arcang.com", szBuff);
		MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		
		PrnStep(5);
		MultiLngPrnReceiptStr("...............................\n", GUI_ALIGN_LEFT);
		PrnStr("\n\n");

		memset(temp, 0, sizeof(temp));
		UtilGetEnvEx("rptsbarcode", temp);
		if(strstr(temp, "true") != NULL)
		{
			PrnCode128(details->field15, 12, 130, 2);
			PrnStep(10);//Set Space after printing logo
		}
		PrnStr("\n\n\n\n\n\n\n\n");
		if(1)
		{
			StartPrinterReceipt();
		}
	}
}