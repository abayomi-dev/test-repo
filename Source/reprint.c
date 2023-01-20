#include "global.h"

int reprintNum = 0;
//Format
//recieptno|txntype|date|time|appname|expdate|maskedpan|holdername|aid|amount|cashback|total|paymentdetails|
//rspcode|authcode|stan|rrn

void storeReprintCashier(char *store)
{
	int iRet, iLen;
	ShowLogs(1, "STOREREPRINTCASHIER");
	ShowLogs(1, "1. Filesize of cashierreprint.txt: %lu", filesize("cashierreprint.txt"));
	if(filesize("cashierreprint.txt") < 10)
	{
		iRet = CreateWrite("cashierreprint.txt", store);
		ShowLogs(1, "1. STOREREPRINTCASHIER: %d", iRet);
	}else if(filesize("cashierreprint.txt") > 1700024)
	{
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		iRet = WriteUpdate("cashierreprint.txt", store, filesize("cashierreprint.txt"));
		ShowLogs(1, "2. STOREREPRINTCASHIER: %d", iRet);
	}else
	{
		iRet = WriteUpdate("cashierreprint.txt", store, filesize("cashierreprint.txt")); 
		ShowLogs(1, "3. STOREREPRINTCASHIER: %d", iRet);
	}
	ShowLogs(1, "2. Filesize of cashierreprint.txt: %lu", filesize("cashierreprint.txt"));
}

void storeReprint(char *store)
{
	int iRet, iLen;
	ShowLogs(1, "STOREREPRINT");
	ShowLogs(1, "1. Filesize of reprint.txt: %lu", filesize("reprint.txt"));
	if(filesize("reprint.txt") < 10)
	{
		iRet = CreateWrite("reprint.txt", store);
		ShowLogs(1, "1. STOREREPRINT: %d", iRet);
	}else if(filesize("reprint.txt") > 1700024)
	{
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		DisplayInfoNone("", "Device Memory Full", 1);
		Beep();
		iRet = WriteUpdate("reprint.txt", store, filesize("reprint.txt"));
		ShowLogs(1, "2. STOREREPRINT: %d", iRet);
	}else
	{
		iRet = WriteUpdate("reprint.txt", store, filesize("reprint.txt")); 
		ShowLogs(1, "3. STOREREPRINT: %d", iRet);
	}
	ShowLogs(1, "2. Filesize of reprint.txt: %lu", filesize("reprint.txt"));
	storeReprintCashier(store);
}


void storeReversal(char *store)
{
	int iRet, iLen;
	ShowLogs(1, "STOREREPRINT");
	ShowLogs(1, "1. Filesize of streversal.txt: %lu", filesize("streversal.txt"));
	if(filesize("streversal.txt") < 10)
	{
		iRet = CreateWrite("streversal.txt", store);
		ShowLogs(1, "1. streversal: %d", iRet);
	}else
	{
		iRet = WriteUpdate("streversal.txt", store, filesize("streversal.txt")); 
		ShowLogs(1, "3. streversal: %d", iRet);
	}
	ShowLogs(1, "2. Filesize of streversal.txt: %lu", filesize("streversal.txt"));
}

// Modified by Kim_LinHB 2014-6-8
int DispPrnReprintError(int iErrCode)
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

int StartPrinterReprint(void)
{
	uchar	ucRet;

	if (!ChkIfIrDAPrinter())
	{
		int iRet;
		while( 1 )
		{
			DispPrinting();
			PrintOne();
			ucRet = PrnStart();
			if( ucRet==PRN_OK )
			{
				return 0;	// print success!
			}

			iRet = DispPrnReprintError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_ERR_USERCANCELLED == iRet||
				GUI_ERR_TIMEOUT == iRet)
			{
				Gui_ClearScr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLEASE REPRINT"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
				break;
			}
		}
	}
	else
	{
		int iRet;
		SetOffBase(OffBaseCheckPrint);	//????

		DispPrinting();
		PrnStart();
		PrintOne();
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
			
			iRet = DispPrnReprintError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_OK != iRet)
			{
				break;
			}
			DispPrinting();
			PrnStart();
			PrintOne();
		}
	}
	return ERR_NO_DISP;
}

void CCC(char *s, char *out)
{
    sprintf(out, "%*s%*s\n", 16 + strlen(s) / 2, s, 16 - strlen(s) / 2, "");
}

void TTTC(char *s, char *out)
{
    sprintf(out, "***%*s%*s***\n", 12 + strlen(s) / 2, s, 12 - strlen(s) / 2, "");
}

void printTestConnection(char *txn, char *rsp)
{
	int iRet, iLen;
	char temp[252] = {0};
	char sBuff[252] = {0};
	char szBuff[252] = {0};
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
	DispPrinting();
	PrnInit();
	PrnSelectFont(&font2,NULL);
	PrnStr("\n\n\n\n\n");
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merName", temp);
	CCC(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merAddr", temp);
	CCC(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("bankname", temp);
	CCC(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	PrnStep(5);
	PrnStr("\n");
	MultiLngPrnReceiptStr(_T("MERCHANT ID.    "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("txnMid", temp);
	sprintf(szBuff,"%15.15s\n", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	MultiLngPrnReceiptStr(_T("TERMINAL ID.    "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("tid", temp);
	sprintf(szBuff,"%15.8s\n", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	MultiLngPrnReceiptStr("===============================\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	memset(szBuff, 0, sizeof(szBuff));
	TTTC(txn, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	PrnStep(5);
	MultiLngPrnReceiptStr("===============================\n", GUI_ALIGN_LEFT);
	MultiLngPrnReceiptStr(_T("RESP CODE:     "), GUI_ALIGN_LEFT);
	sprintf(szBuff,"%16.16s\n", rsp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	PrnStep(5);
	MultiLngPrnReceiptStr("===============================\n", GUI_ALIGN_LEFT);
	PrnStr("\n\n\n\n\n");
	StartPrinterReprint();
}

void printDetails()
{
	int iRet, iLen;
	char temp[252] = {0};
	char sBuff[252] = {0};
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
	
	DispPrinting();
	
	PrnInit();
	PrnSelectFont(&font2,NULL);
	PrnStr("\n\n\n");
	

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	MultiLngPrnReceiptStr("TERMINAL PARAMETERS\n", GUI_ALIGN_LEFT);
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStr("\n");

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("bankname", temp);
	sprintf((char *)sBuff, "%-10.10s %20.20s\n", "BANK NAME:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	
	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("tid", temp);
	sprintf((char *)sBuff, "%-12.12s %18.18s\n", "TERMINAL ID:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("txnMid", temp);
	sprintf((char *)sBuff, "%-12.12s %18.18s\n", "MERCHANT ID:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("merName", temp);
	sprintf((char *)sBuff, "%-14.14s %16.16s\n", "MERCHANT NAME:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("merAddr", temp);
	sprintf((char *)sBuff, "%-14.14s %16.16s\n", "MERCHANT ADDR:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	sprintf((char *)sBuff, "%-9.9s %21.21s\n", "ACQUIRER:", "NA");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("txnMCC", temp);
	sprintf((char *)sBuff, "%-4.4s %26.26s\n", "MCC:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("coapn", temp);
	sprintf((char *)sBuff, "%-4.4s %26.26s\n", "APN:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("cosubnet", temp);
	sprintf((char *)sBuff, "%-9.9s %21.21s\n", "USERNAME:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("copwd", temp);
	sprintf((char *)sBuff, "%-9.9s %21.21s\n", "PASSWORD:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("hostip", temp);
	sprintf((char *)sBuff, "%-10.10s %20.20s\n", "SERVER IP:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("hostport", temp);
	sprintf((char *)sBuff, "%-12.12s %18.18s\n", "SERVER PORT:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	sprintf((char *)sBuff, "%-10.10s %20.20s\n", "HOST NAME:", "ctms.nibss-plc.com");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("tcmIP", temp);
	sprintf((char *)sBuff, "%-8.8s %22.22s\n", "TMS IP:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("tcmPort", temp);
	sprintf((char *)sBuff, "%-10.10s %20.20s\n", "TMS PORT:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);


	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	UtilGetEnvEx("chinterval", temp);
	sprintf((char *)sBuff, "%-14.14s %16.16s\n", "CALLHOME TIME:", temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);


	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	sprintf((char *)sBuff, "%-12.12s %18.18s\n", "APP VERSION:", "ARCA  v1.0.0");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);


	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	sprintf((char *)sBuff, "%-13.13s %17.17s\n", "Dial Time: 90", "Redial Times: 3");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	memset(temp, '\0', sizeof(temp));
	memset(sBuff, '\0', sizeof(sBuff));
	sprintf((char *)sBuff, "%-11.11s %19.19s\n", "Timeout: 60", "Resent Times: 3");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);

	PrnStr("\n\n\n\n\n\n");

	StartPrinterReprint();
}


int getLastTxn(char *txn)
{
	int i;
	for(i = strlen(txn) - 2; i > 0; i--)
	{
		if(txn[i] == '\n')
			break;
	}
	return i;
}

void reprintLast()
{
	char txn[2000 * 1024] = {0};
	//char *txn = (char*)sysalloc_malloc((sizeof(char) * filesize("reprint.txt")) + 1);
	char details[20][128];
	char disp[128] = {0};
	struct PRINTDATA data;
	//char *txn = (char*)malloc((sizeof(char) * filesize("reprint.txt")));
	int iRet, j, n = 0, l;
	int i, k = 0;
	char line[1024] = {0};
	char temp[128] = {0};
	int retry = 0;
	memset(txn, '\0', strlen(txn));
	//memset(details, 0, sizeof(details));
	for(l = 0; l < 20; l++)
	{
		memset(details[l], '\0', sizeof(details[l]));
	}
	memset(line, '\0', strlen(line));
	iRet = ReadAllData("reprint.txt", txn);
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		//free(txn);
		return;
	}
	

	j = getLastTxn(txn);
	//memset(disp, 0, strlen(disp));
	//sprintf(disp, "%d. VALUE == %d", retry, j);
	//DisplayInfoNone("", disp, 5);

	for(i = j; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			j = 0;
			if((i + 1) >= strlen(txn))
			{
				break;
			}else
			{
				memset(line, '\0', strlen(line));
			}
			continue;
		}else
		{
			line[k] = txn[i];
			k++;
		}
	}
	
	////ShowLogs(1, "1. Ima mmi: %s", line);
	memset(temp, '\0', strlen(temp));
	for(i = 0; i < strlen(line); i++)
	{
		if(line[i] == '|')
        {
        	memset(details[n], '\0', strlen(details[n]));//James
            strcpy(details[n], temp);
            j = 0;
            n++;
            memset(temp, 0, strlen(temp));
        }else
        {
            temp[j] = line[i];
            j++;
        }
    }



    if(strlen(details[0]))
    {
    	memset(data.field0, '\0', strlen(data.field0));
        strncpy(data.field0, details[0], strlen(details[0]));
    }
    if(strlen(details[1]))
    {
    	memset(data.field1, '\0', strlen(data.field1));
        strncpy(data.field1, details[1], strlen(details[1]));
    }
    if(strlen(details[2]))
    {
    	memset(data.field2, '\0', strlen(data.field2));
        strncpy(data.field2, details[2], strlen(details[2]));
    }
    if(strlen(details[3]))
    {
    	memset(data.field3, '\0', strlen(data.field3));
        strncpy(data.field3, details[3], strlen(details[3]));
    }
    if(strlen(details[4]))
    {
    	memset(data.field4, '\0', strlen(data.field4));
        strncpy(data.field4, details[4], strlen(details[4]));
    }
    if(strlen(details[5]))
    {
    	memset(data.field5, '\0', strlen(data.field5));
        strncpy(data.field5, details[5], strlen(details[5]));
    }
    if(strlen(details[6]))
    {
    	memset(data.field6, '\0', strlen(data.field6));
        strncpy(data.field6, details[6], strlen(details[6]));
    }
    if(strlen(details[7]))
    {
    	memset(data.field7, '\0', strlen(data.field7));
        strncpy(data.field7, details[7], strlen(details[7]));
    }
    if(strlen(details[8]))
    {
    	memset(data.field8, '\0', strlen(data.field8));
        strncpy(data.field8, details[8], strlen(details[8]));
    }
    if(strlen(details[9]))
    {
    	memset(data.field9, '\0', strlen(data.field9));
        strncpy(data.field9, details[9], strlen(details[9]));
    }
    if(strlen(details[10]))
    {
    	memset(data.field10, '\0', strlen(data.field10));
        strncpy(data.field10, details[10], strlen(details[10]));
    }
    if(strlen(details[11]))
    {
    	memset(data.field11, '\0', strlen(data.field11));
        strncpy(data.field11, details[11], strlen(details[11]));
    }
    if(strlen(details[12]))
    {
    	memset(data.field12, '\0', strlen(data.field12));
        strncpy(data.field12, details[12], strlen(details[12]));
    }
    if(strlen(details[13]))
    {
    	memset(data.field13, '\0', strlen(data.field13));
        strncpy(data.field13, details[13], strlen(details[13]));
    }
    if(strlen(details[14]))
    {
    	memset(data.field14, '\0', strlen(data.field14));
        strncpy(data.field14, details[14], strlen(details[14]));
    }
    if(strlen(details[15]))
    {
    	memset(data.field15, '\0', strlen(data.field15));
        strncpy(data.field15, details[15], strlen(details[15]));
    }
    if(strlen(details[16]))
    {
    	memset(data.field16, '\0', strlen(data.field16));
        strncpy(data.field16, details[16], strlen(details[16]));
    }
    if(strlen(details[17]))
    {
    	memset(data.field17, '\0', strlen(data.field17));
        strncpy(data.field17, details[17], strlen(details[17]));
    }
    if(strlen(details[18]))
    {
    	memset(data.field18, '\0', strlen(data.field18));
        strncpy(data.field18, details[18], strlen(details[18]));
    }
    if(strlen(details[19]))
    {
    	memset(data.field19, '\0', strlen(data.field19));
        strncpy(data.field19, details[19], strlen(details[19]));
    }
	reprintDataNormal(&data);
	//free(txn);
}

int checkNumber(char *number, char *line)
{
	char temp[128] = {0};
	int i, j = 0;
	memset(temp, '\0', strlen(temp));
	for(i = 0; i < strlen(line); i++)
	{
		if(line[i] == '|')
			break;
		else
		{
			temp[j] = line[i];
			j++;
		}
	}
	if(strncmp(temp, number, strlen(number)) == 0)
	{
		reprintNum = 1;
		return 1;
	}
	reprintNum = 0;
	return 0;
}

int checkRrn(char *number, char *line)
{
	char *ret;
	ShowLogs(1, "RRN LINE: %s", line);
	ShowLogs(1, "RRN NUMBER: %s", number);
	ret = strstr(line, number);
	if(ret)
	{
		ShowLogs(1, "RRN SEEN");
		reprintNum = 1;
		return 1;
	}
	reprintNum = 0;
	return 0;
}

void reprintAny(char *number)
{
	char details[20][128];
	//char txn[500 * 1024] = {0};
	//char *txn = (char*)sysalloc_malloc((sizeof(char) * filesize("reprint.txt")) + 1);
	char txn[2000 * 1024] = {0};
	struct PRINTDATA data;
	//char *txn = (char*)malloc((sizeof(char) * filesize("reprint.txt")) + 1);
	int iRet, i, j, n = 0;
	char line[1024] = {0};
	char temp[128] = {0};
	char lv[5] = {0};
	int k = 0;
	char *ret;
	ShowLogs(1, "REPRINT ANY: %s", number);
	memset(txn, '\0', strlen(txn));
	//memset(details, 0, sizeof(details));
	//ShowLogs(1, "Reprint 1");
	for(i = 0; i < 20; i++)
	{
		memset(details[i], '\0', sizeof(details[i]));
	}
	memset(line, '\0', strlen(line));
	iRet = ReadAllData("reprint.txt", txn);
	////ShowLogs(1, "Reprint: %s", txn);
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		//free(txn);
		return;
	}
	
	////ShowLogs(1, "Number: %s", number);
	if(strncmp(number, "0", strlen(number)) == 0)
	{
		Beep();
		DisplayInfoNone("", "Wrong Number", 2);
		//free(txn);
		return 0;
	}else if(strncmp(number, "1", strlen(number)) != 0)
	{
		memset(lv, '\0', strlen(lv));
		strcpy(lv, "\n");
		strcat(lv, number);
		strcat(lv, "|");
		//ShowLogs(1, "Reprint 2: %s", lv);
		ret = strstr(txn, lv);
		if(ret)
		{
		    j = strlen(txn) - strlen(ret);
		    //ShowLogs(1, "The Index: %d", j);
		}else
		{
			//ShowLogs(1, "Receipt Not Found");
			DisplayInfoNone("", "Receipt not Seen", 2);
			//free(txn);
			return;
		}
	}else
	{
		j = 0;
	}

	//j = 0;
	for(i = j; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			j = 0;
			if(checkNumber(number, line))
			{
				break;
			}else
			{
				memset(line, '\0', strlen(line));
			}
			continue;
		}else
		{
			line[k] = txn[i];
			k++;
		}
	}
	//ShowLogs(1, "Reprint 3");
	if(reprintNum == 0)
	{
		DisplayInfoNone("", "Receipt not seen", 2);
		//free(txn);
		return;
	}
	ShowLogs(1, "1. Ima Mmi: %s", line);
	memset(temp, '\0', strlen(temp));
	for(i = 0; i < strlen(line); i++)
	{
		if(line[i] == '|')
        {
        	memset(details[n], '\0', strlen(details[n]));//James
            strncpy(details[n], temp, strlen(temp));
            ShowLogs(1, "Imma: %d: %s", n, temp);
            j = 0;
            n++;
            memset(temp, 0, strlen(temp));
        }else
        {
            temp[j] = line[i];
            j++;
        }
    }
    //ShowLogs(1, "Reprint 4");
    if(strlen(details[0]))
    {
    	memset(data.field0, '\0', strlen(data.field0));
        strncpy(data.field0, details[0], strlen(details[0]));
    }
    if(strlen(details[1]))
    {
    	memset(data.field1, '\0', strlen(data.field1));
        strncpy(data.field1, details[1], strlen(details[1]));
    }
    if(strlen(details[2]))
    {
    	memset(data.field2, '\0', strlen(data.field2));
        strncpy(data.field2, details[2], strlen(details[2]));
    }
    if(strlen(details[3]))
    {
    	memset(data.field3, '\0', strlen(data.field3));
        strncpy(data.field3, details[3], strlen(details[3]));
    }
    if(strlen(details[4]))
    {
    	memset(data.field4, '\0', strlen(data.field4));
        strncpy(data.field4, details[4], strlen(details[4]));
    }
    if(strlen(details[5]))
    {
    	memset(data.field5, '\0', strlen(data.field5));
        strncpy(data.field5, details[5], strlen(details[5]));
    }
    if(strlen(details[6]))
    {
    	memset(data.field6, '\0', strlen(data.field6));
        strncpy(data.field6, details[6], strlen(details[6]));
    }
    if(strlen(details[7]))
    {
    	memset(data.field7, '\0', strlen(data.field7));
        strncpy(data.field7, details[7], strlen(details[7]));
    }
    if(strlen(details[8]))
    {
    	memset(data.field8, '\0', strlen(data.field8));
        strncpy(data.field8, details[8], strlen(details[8]));
    }
    if(strlen(details[9]))
    {
    	memset(data.field9, '\0', strlen(data.field9));
        strncpy(data.field9, details[9], strlen(details[9]));
    }
    if(strlen(details[10]))
    {
    	memset(data.field10, '\0', strlen(data.field10));
        strncpy(data.field10, details[10], strlen(details[10]));
    }
    if(strlen(details[11]))
    {
    	memset(data.field11, '\0', strlen(data.field11));
        strncpy(data.field11, details[11], strlen(details[11]));
    }
    if(strlen(details[12]))
    {
    	memset(data.field12, '\0', strlen(data.field12));
        strncpy(data.field12, details[12], strlen(details[12]));
    }
    if(strlen(details[13]))
    {
    	memset(data.field13, '\0', strlen(data.field13));
        strncpy(data.field13, details[13], strlen(details[13]));
    }
    if(strlen(details[14]))
    {
    	memset(data.field14, '\0', strlen(data.field14));
        strncpy(data.field14, details[14], strlen(details[14]));
    }
    if(strlen(details[15]))
    {
    	memset(data.field15, '\0', strlen(data.field15));
        strncpy(data.field15, details[15], strlen(details[15]));
    }
    if(strlen(details[16]))
    {
    	memset(data.field16, '\0', strlen(data.field16));
        strncpy(data.field16, details[16], strlen(details[16]));
    }
    if(strlen(details[17]))
    {
    	memset(data.field17, '\0', strlen(data.field17));
        strncpy(data.field17, details[17], strlen(details[17]));
    }
    if(strlen(details[18]))
    {
    	memset(data.field18, '\0', strlen(data.field18));
        strncpy(data.field18, details[18], strlen(details[18]));
    }
    if(strlen(details[19]))
    {
    	memset(data.field19, '\0', strlen(data.field19));
        strncpy(data.field19, details[19], strlen(details[19]));
    }
	reprintDataNormal(&data);
	//ShowLogs(1, "Reprint 5");
	reprintNum = 0;
	//free(txn);
}

void reprintRrn(char *number)
{
	char details[20][128];
	char txn[2000 * 1024] = {0};
	struct PRINTDATA data;
	int iRet, i, j, n = 0;
	char line[1024] = {0};
	char temp[128] = {0};
	char lv[5] = {0};
	int k = 0;
	char *ret;
	ShowLogs(1, "REPRINT RRN: %s", number);
	memset(txn, '\0', strlen(txn));
	for(i = 0; i < 20; i++)
	{
		memset(details[i], '\0', sizeof(details[i]));
	}
	memset(line, '\0', strlen(line));
	iRet = ReadAllData("reprint.txt", txn);
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		return;
	}
	if(strlen(number) < 12)
	{
		Beep();
		DisplayInfoNone("", "WRONG RRN", 2);
		return 0;
	}else
	{
		j = 0;
	}

	//j = 0;
	for(i = j; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			j = 0;
			k = 0;
			if(checkRrn(number, line))
			{
				break;
			}else
			{
				memset(line, '\0', strlen(line));
			}
			continue;
		}else
		{
			line[k] = txn[i];
			k++;
		}
	}
	
	//ShowLogs(1, "Reprint 3");
	if(reprintNum == 0)
	{
		DisplayInfoNone("", "RECEIPT NOT SEEN", 2);
		return;
	}

	ShowLogs(1, "1. Ima Mmi: %s", line);
	memset(temp, '\0', strlen(temp));
	for(i = 0; i < strlen(line); i++)
	{
		if(line[i] == '|')
        {
        	memset(details[n], '\0', strlen(details[n]));//James
            strncpy(details[n], temp, strlen(temp));
            ShowLogs(1, "Imma: %d: %s", n, temp);
            j = 0;
            n++;
            memset(temp, 0, strlen(temp));
        }else
        {
            temp[j] = line[i];
            j++;
        }
    }
    //ShowLogs(1, "Reprint 4");
    if(strlen(details[0]))
    {
    	memset(data.field0, '\0', strlen(data.field0));
        strncpy(data.field0, details[0], strlen(details[0]));
    }
    if(strlen(details[1]))
    {
    	memset(data.field1, '\0', strlen(data.field1));
        strncpy(data.field1, details[1], strlen(details[1]));
    }
    if(strlen(details[2]))
    {
    	memset(data.field2, '\0', strlen(data.field2));
        strncpy(data.field2, details[2], strlen(details[2]));
    }
    if(strlen(details[3]))
    {
    	memset(data.field3, '\0', strlen(data.field3));
        strncpy(data.field3, details[3], strlen(details[3]));
    }
    if(strlen(details[4]))
    {
    	memset(data.field4, '\0', strlen(data.field4));
        strncpy(data.field4, details[4], strlen(details[4]));
    }
    if(strlen(details[5]))
    {
    	memset(data.field5, '\0', strlen(data.field5));
        strncpy(data.field5, details[5], strlen(details[5]));
    }
    if(strlen(details[6]))
    {
    	memset(data.field6, '\0', strlen(data.field6));
        strncpy(data.field6, details[6], strlen(details[6]));
    }
    if(strlen(details[7]))
    {
    	memset(data.field7, '\0', strlen(data.field7));
        strncpy(data.field7, details[7], strlen(details[7]));
    }
    if(strlen(details[8]))
    {
    	memset(data.field8, '\0', strlen(data.field8));
        strncpy(data.field8, details[8], strlen(details[8]));
    }
    if(strlen(details[9]))
    {
    	memset(data.field9, '\0', strlen(data.field9));
        strncpy(data.field9, details[9], strlen(details[9]));
    }
    if(strlen(details[10]))
    {
    	memset(data.field10, '\0', strlen(data.field10));
        strncpy(data.field10, details[10], strlen(details[10]));
    }
    if(strlen(details[11]))
    {
    	memset(data.field11, '\0', strlen(data.field11));
        strncpy(data.field11, details[11], strlen(details[11]));
    }
    if(strlen(details[12]))
    {
    	memset(data.field12, '\0', strlen(data.field12));
        strncpy(data.field12, details[12], strlen(details[12]));
    }
    if(strlen(details[13]))
    {
    	memset(data.field13, '\0', strlen(data.field13));
        strncpy(data.field13, details[13], strlen(details[13]));
    }
    if(strlen(details[14]))
    {
    	memset(data.field14, '\0', strlen(data.field14));
        strncpy(data.field14, details[14], strlen(details[14]));
    }
    if(strlen(details[15]))
    {
    	memset(data.field15, '\0', strlen(data.field15));
        strncpy(data.field15, details[15], strlen(details[15]));
    }
    if(strlen(details[16]))
    {
    	memset(data.field16, '\0', strlen(data.field16));
        strncpy(data.field16, details[16], strlen(details[16]));
    }
    if(strlen(details[17]))
    {
    	memset(data.field17, '\0', strlen(data.field17));
        strncpy(data.field17, details[17], strlen(details[17]));
    }
    if(strlen(details[18]))
    {
    	memset(data.field18, '\0', strlen(data.field18));
        strncpy(data.field18, details[18], strlen(details[18]));
    }
    if(strlen(details[19]))
    {
    	memset(data.field19, '\0', strlen(data.field19));
        strncpy(data.field19, details[19], strlen(details[19]));
    }
	reprintDataNormal(&data);
	reprintNum = 0;
}

void TTC(char *s, char *out)
{
    sprintf(out, "***%*s%*s***\n", 12 + strlen(s) / 2, s, 12 - strlen(s) / 2, "");
}

double totalamt = 0.00;
int transactionpassed = 0;
int transactionfailed = 0;
int transaction = 0;

void parseAmountEod(char *in, char *out)
{
	int len = strlen(in);
	switch(len)
	{
		case 11:
			strcpy(out, in);
			break;
		case 10:
			strcpy(out, "*");
			strcat(out, in);
			break;
		case 9:
			strcpy(out, "**");
			strcat(out, in);
			break;
		case 8:
			strcpy(out, "***");
			strcat(out, in);
			break;
		case 7:
			strcpy(out, "****");
			strcat(out, in);
			break;
		case 6:
			strcpy(out, "*****");
			strcat(out, in);
			break;
		case 5:
			strcpy(out, "******");
			strcat(out, in);
			break;
		case 4:
			strcpy(out, "*******");
			strcat(out, in);
			break;
		case 3:
			strcpy(out, "********");
			strcat(out, in);
			break;
		case 2:
			strcpy(out, "*********");
			strcat(out, in);
			break;
		default:
			strcpy(out, "***********");
			strcat(out, in);
			break;
	}
}

void convertTotal(char *init, char *out)
{
	int i = 0, j = 0;
	for(i = 0; i < strlen(init); i++)
	{
		if(init[i] == ',')
			continue;
		out[j] = init[i];
		j++;
	}
}

void parseTotal(char *status, char *amt, char *out)
{
    int i, j = 0;
    double init = 0.00;
    char temp[13] = {0};
    char output[13] = {0};
    for(i = 3; i < strlen(amt); i++)
    {
        temp[j] = amt[i];
        j++;
    }
    //ShowLogs(1, "Status: %s.", status);
    parseAmountEod(temp, out);
    ////ShowLogs(1, "Status: %s", status);
    if(strncmp(status, "00", 2) == 0)
	{
		//ShowLogs(1, "Did it enter?");
		memset(output, '\0', strlen(output));
		convertTotal(temp, output);
		init = atof(output);
		//ShowLogs(1, "Init: %.2f.", init);
    	totalamt += init;
    	//ShowLogs(1, "TotalAmt: %.2f.", totalamt);
	}
	//ShowLogs(1, "it did not enter?");
}

void DD(char *s, char *out)
{
    sprintf(out, "%*s%*s\n", 16 + strlen(s) / 2, s, 16 - strlen(s) / 2, "");
}

void parseTimeEod(char *in, char *out)
{
	strncpy(out, in, 5);
}

void parsePanEod(char *in, char *out)
{
	int len = strlen(in) - 1;
	out[0] = in[len - 5];
	out[1] = in[len - 4];
	out[2] = in[len - 3];
	out[3] = in[len - 2];
	out[4] = in[len - 1];
	out[5] = in[len - 0];
}

void parseTxnEod(char *in, char *out)
{
	if(strncmp(in, "PURCHASE", 8) == 0)
	{
		strcpy(out, "P");
	}else if(strncmp(in, "PAYATTITUDE", 11) == 0)
	{
		strcpy(out, "P");
	}else if(strncmp(in, "PRE AUTH", 8) == 0)
	{
		strcpy(out, "P");
	}else if(strncmp(in, "SALES COMPLETION", 16) == 0)
	{
		strcpy(out, "S");
	}else if(strncmp(in, "REFUND", 6) == 0)
	{
		strcpy(out, "R");
	}else if(strncmp(in, "MANUAL ENTRY", 12) == 0)
	{
		strcpy(out, "M");
	}else if(strncmp(in, "NFC", 3) == 0)
	{
		strcpy(out, "N");
	}else if(strncmp(in, "MAGNETIC SWIPE", 14) == 0)
	{
		strcpy(out, "M");
	}else if(strncmp(in, "CASH ADVANCE", 12) == 0)
	{
		strcpy(out, "C");
	}else if(strncmp(in, "CASH BACK", 8) == 0)
	{
		strcpy(out, "C");
	}else 
	{
		strcpy(out, "O");
	}
}

void parseStatusEod(char *in, char *out)
{
	if(strncmp(in, "00", 2) == 0)
	{
		transactionpassed++;
		strcpy(out, "A");
	}else if(strncmp(in, "100", 3) == 0)
	{
		transactionpassed++;
		strcpy(out, "T");
	}else if(strncmp(in, "101", 3) == 0)
	{
		transactionpassed++;
		strcpy(out, "R");
	}else 
	{
		transactionfailed++;
		strcpy(out, "D");
	}
}

int getTotalCountEod(char* txn)
{
	int i, k = 0;
	for(i = 0; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
			k++;
	}
	return k;
}

int checkifDifferentDays(char *day, char *mnth, char *year, char *field2, char *out)
{
	//Time is: 25/04/2019
	//Value 2: APR 25, 2019
	char dmonth[4] = {0};
	char ddmonth[3] = {0};
	char dday[3] = {0};
	char dyear[5] = {0};
	memset(dmonth, '\0', strlen(dmonth));
	dmonth[0] = field2[0];
	dmonth[1] = field2[1];
	dmonth[2] = field2[2];
	memset(dday, '\0', strlen(dday));
	dday[0] = field2[4];
	dday[1] = field2[5];
	memset(dyear, '\0', strlen(dyear));
	dyear[0] = field2[8];
	dyear[1] = field2[9];
	dyear[2] = field2[10];
	dyear[3] = field2[11];
	ShowLogs(1, "Day: %s. - Month: %s. - Year: %s. Field: %s.", day, mnth, year, field2);
	memset(ddmonth, '\0', strlen(ddmonth));
	if(strstr(dmonth, "JAN") != NULL)
		strcpy(ddmonth, "01");
	else if(strstr(dmonth, "FEB") != NULL)
		strcpy(ddmonth, "02");
	else if(strstr(dmonth, "MAR") != NULL)
		strcpy(ddmonth, "03");
	else if(strstr(dmonth, "APR") != NULL)
		strcpy(ddmonth, "04");
	else if(strstr(dmonth, "MAY") != NULL)
		strcpy(ddmonth, "05");
	else if(strstr(dmonth, "JUN") != NULL)
		strcpy(ddmonth, "06");
	else if(strstr(dmonth, "JUL") != NULL)
		strcpy(ddmonth, "07");
	else if(strstr(dmonth, "AUG") != NULL)
		strcpy(ddmonth, "08");
	else if(strstr(dmonth, "SEP") != NULL)
		strcpy(ddmonth, "09");
	else if(strstr(dmonth, "OCT") != NULL)
		strcpy(ddmonth, "10");
	else if(strstr(dmonth, "NOV") != NULL)
		strcpy(ddmonth, "11");
	else 
		strcpy(ddmonth, "12");

	if(strlen(day) < 1)
	{
		strncpy(day, dday, strlen(dday));
		strncpy(mnth, ddmonth, strlen(ddmonth));
		strncpy(year, dyear, strlen(dyear));
		strcpy(out, day);
		strcat(out, "/");
		strcat(out, ddmonth);
		strcat(out, "/");
		strcat(out, dyear);
		checkSame = 1;
		ShowLogs(1, "Out One: %s", out);
		return 1;
	}else
	{
		////ShowLogs(1, "Old Date: %s - %s - %s", day, mnth, year);
		////ShowLogs(1, "Compared Date: %s - %s - %s", dday, ddmonth, dyear);

		if((strstr(dday, day) != NULL) && (strstr(ddmonth, mnth) != NULL) && (strstr(dyear, year) != NULL))
		{
			strcpy(out, day);
			strcat(out, "/");
			strcat(out, ddmonth);
			strcat(out, "/");
			strcat(out, dyear);
			checkSame = 0;
			ShowLogs(1, "Out Two: %s", out);
			return 0;
		}
		strncpy(day, dday, strlen(dday));
		strncpy(mnth, ddmonth, strlen(ddmonth));
		strncpy(year, dyear, strlen(dyear));
		strcpy(out, day);
		strcat(out, "/");
		strcat(out, ddmonth);
		strcat(out, "/");
		strcat(out, dyear);
		checkSame = 1;
		ShowLogs(1, "Out Three: %s", out);
		return 1;
	}
}

void dataCollection()
{
	//char txn[1000 * 1024] = {0};
	char *txn = (char*)sysalloc_malloc((sizeof(char) * filesize("reprint.txt")) + 1);
	char details[20][128];
	int iRet, i, k, loop, j, n = 0, l;
	char line[1024] = {0};
	char temp[128] = {0};
	char displ[128] = {0};
	char szBuff[128] = {0};
	int iLen;
	char sBuff[252] = {0};
	char timeGotten[15] = {0};
	char datetime[15] = {0};
	char dtM[15] = {0};
	char day[3] = {0};
	char mnth[3] = {0};
	char year[5] = {0};
	int olubayo = 0;
	int count = 0;
	int total = 0;

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
	PrnStr("\n\n");

	//memset(txn, '\0', strlen(txn));
	//memset(details, 0, sizeof(details));
	for(l = 0; l < 20; l++)
	{
		memset(details[l], '\0', sizeof(details[l]));
	}
	memset(line, '\0', strlen(line));


	////ShowLogs(1, "Size of reprint.txt: %lu", filesize("reprint.txt"));


	iRet = ReadAllData("reprint.txt", txn);
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		sysalloc_free(txn);
		return;
	}
	totalamt = 0.00;
	total = getTotalCountEod(txn);
	loop = 1;
	j = 0;

	////ShowLogs(1, "%s", txn);

	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("bankname", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merName", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merAddr", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	PrnStep(5);
	PrnStr("\n");

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	MultiLngPrnReceiptStr(_T("MID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("txnMid", temp);
	sprintf(szBuff,"%s\n", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

	MultiLngPrnReceiptStr(_T("TID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("tid", temp);
	sprintf(szBuff,"%s", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);


	SysGetTimeIso(timeGotten);

	datetime[0] = timeGotten[4];
	datetime[1] = timeGotten[5];
	datetime[2] = '/';
	datetime[3] = timeGotten[2];
	datetime[4] = timeGotten[3];
	datetime[5] = '/';
	datetime[6] = '2';
	datetime[7] = '0';
	datetime[8] = timeGotten[0];
	datetime[9] = timeGotten[1];
	

	/*MultiLngPrnReceiptStr(_T("DATE:           "), GUI_ALIGN_LEFT);
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%s\n", datetime);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);*/


	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	MultiLngPrnReceiptStr("  TRANSACTION    \n", GUI_ALIGN_LEFT);
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	//recieptno|txntype|date|time|appname|expdate|maskedpan|holdername|aid|amount|cashback|total|paymentdetails|
	//rspcode|authcode|stan|rrn
	for(i = 0; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			count++;
			memset(displ, '\0', strlen(displ)); 
			sprintf(displ, "EOD PROCESSING \n%d/%d", count, total);
			DisplayInfoNone("", "Printing", 0);

			olubayo++;
			j = 0;
			memset(temp, '\0', strlen(temp));
			//memset(details, 0, sizeof(details));
			for(l = 0; l < 20; l++)
			{
				memset(details[l], '\0', sizeof(details[l]));
			}
			n = 0;
			for(k = 0; k < strlen(line); k++)
			{
				if(line[k] == '|')
		        {
		            strncpy(details[n], temp, strlen(temp));
		            j = 0;
		            n++;
		            memset(temp, 0, strlen(temp));
		        }else
		        {
		            temp[j] = line[k];
		            j++;
		        }
		    }

		    checkSame = 0;
		    if(checkifDifferentDays(day, mnth, year, details[2], dtM))
		    {	
		    	if(checkSame == 1)
		    	{
		    		memset(szBuff, 0, sizeof(szBuff));
		    		MultiLngPrnReceiptStr(_T("DATE:               "), GUI_ALIGN_LEFT);
					sprintf(szBuff,"%s\n", dtM);
					MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		    	}
		    }

		    memset(sBuff, '\0', sizeof(sBuff));
		    parseTimeEod(details[3], sBuff);
		    ShowLogs(1, "TIME: %s", sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
		    parseTxnEod(details[1], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
		    parsePanEod(details[6], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseTotal(details[12], details[9], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseStatusEod(details[12], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			PrnStr("\n");
			memset(line, '\0', strlen(line));
			j = 0;

			while( 1 )
			{
				int ucRet = PrnStart();
				if( ucRet!=PRN_OK )
				{
					iRet = DispPrnReprintError(ucRet);
					if( ucRet!=ERR_PRN_PAPEROUT )
					{
						break;
					}

					if( GUI_ERR_USERCANCELLED == iRet||
						GUI_ERR_TIMEOUT == iRet)
					{
						Gui_ClearScr();
						Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLEASE REPRINT"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
						break;
					}
				}
				break;
			}

			//DispPrinting();
			//StartPrinterReprint();
			//DelayMs(100);
			PrnInit();
			olubayo = 0;

			continue;
		}else
		{
			line[j] = txn[i];
			j++;
		}
	}




	transaction = transactionpassed + transactionfailed;    

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Conclusion\n");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transaction);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Approved Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionpassed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Declined Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionfailed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Approved Amount: \nNGN ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%.2f", totalamt);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n\n\n\n\n\n\n\n\n\n\n");

	//Do summary here
	DispPrinting();
    StartPrinterReprint();

    CreateWrite("receipt.txt", "0");
	CreateWrite("reprint.txt", "0");
	CreateWrite("count.txt", "0");
	CreateWrite("eod.txt", "0");
	CreateWrite("eod2.txt", "0");

    totalamt = 0.00;
    transaction = 0;
    transactionpassed = 0;
	transactionfailed = 0;
	DisplayInfoNone("", "Done", 2);
	sysalloc_free(txn);
}

int checkifToday(char *datetime, char *field2)
{
	//Time is: 25/04/2019
	//Value 2: APR 25, 2019
	char dmonth[4] = {0};
	char ddmonth[3] = {0};
	char dday[3] = {0};
	char dyear[5] = {0};
	char cmonth[3] = {0};
	char cday[3] = {0};
	char cyear[5] = {0};
	memset(dmonth, '\0', strlen(dmonth));
	dmonth[0] = field2[0];
	dmonth[1] = field2[1];
	dmonth[2] = field2[2];
	memset(dday, '\0', strlen(dday));
	dday[0] = field2[4];
	dday[1] = field2[5];
	memset(dyear, '\0', strlen(dyear));
	dyear[0] = field2[8];
	dyear[1] = field2[9];
	dyear[2] = field2[10];
	dyear[3] = field2[11];
	memset(cday, '\0', strlen(cday));
	cday[0] = datetime[0];
	cday[1] = datetime[1];
	memset(cmonth, '\0', strlen(cmonth));
	cmonth[0] = datetime[3];
	cmonth[1] = datetime[4];
	memset(cyear, '\0', strlen(cyear));
	cyear[0] = datetime[6];
	cyear[1] = datetime[7];
	cyear[2] = datetime[8];
	cyear[3] = datetime[9];
	memset(ddmonth, '\0', strlen(ddmonth));
	if(strstr(dmonth, "JAN") != NULL)
		strcpy(ddmonth, "01");
	else if(strstr(dmonth, "FEB") != NULL)
		strcpy(ddmonth, "02");
	else if(strstr(dmonth, "MAR") != NULL)
		strcpy(ddmonth, "03");
	else if(strstr(dmonth, "APR") != NULL)
		strcpy(ddmonth, "04");
	else if(strstr(dmonth, "MAY") != NULL)
		strcpy(ddmonth, "05");
	else if(strstr(dmonth, "JUN") != NULL)
		strcpy(ddmonth, "06");
	else if(strstr(dmonth, "JUL") != NULL)
		strcpy(ddmonth, "07");
	else if(strstr(dmonth, "AUG") != NULL)
		strcpy(ddmonth, "08");
	else if(strstr(dmonth, "SEP") != NULL)
		strcpy(ddmonth, "09");
	else if(strstr(dmonth, "OCT") != NULL)
		strcpy(ddmonth, "10");
	else if(strstr(dmonth, "NOV") != NULL)
		strcpy(ddmonth, "11");
	else 
		strcpy(ddmonth, "12");

	////ShowLogs(1, "Stored Date: %s - %s - %s", dday, ddmonth, dyear);
	////ShowLogs(1, "Compared Date: %s - %s - %s", cday, cmonth, cyear);

	if((strstr(dday, cday) != NULL) && (strstr(ddmonth, cmonth) != NULL) && (strstr(dyear, cyear) != NULL))
		return 0;
	return 1;
}

void endofday()
{
	//char txn[1000 * 1024] = {0};
	char *txn = (char*)sysalloc_malloc((sizeof(char) * filesize("reprint.txt")) + 1);
	char details[20][128];
	int iRet, i, k, loop, j, n = 0, l;
	char line[1024] = {0};
	char temp[128] = {0};
	char displ[128] = {0};
	char szBuff[128] = {0};
	int iLen;
	char sBuff[252] = {0};
	char timeGotten[15] = {0};
	char datetime[15] = {0};
	char dateComp[20] = {0};
	char preDateComp[20] = {0};
	int olubayo = 0;
	int count = 0;
	int total = 0;

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
	PrnStr("\n\n");

	//memset(txn, '\0', strlen(txn));
	//memset(details, 0, sizeof(details));
	for(l = 0; l < 20; l++)
	{
		memset(details[l], '\0', sizeof(details[l]));
	}
	memset(line, '\0', strlen(line));


	////ShowLogs(1, "Size of reprint.txt: %lu", filesize("reprint.txt"));


	iRet = ReadAllData("reprint.txt", txn);
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		sysalloc_free(txn);
		return;
	}
	totalamt = 0.00;
	total = getTotalCountEod(txn);
	loop = 1;
	j = 0;

	////ShowLogs(1, "%s", txn);

	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("bankname", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merName", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merAddr", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	PrnStep(5);
	PrnStr("\n");

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	MultiLngPrnReceiptStr(_T("MID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("txnMid", temp);
	sprintf(szBuff,"%s\n", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

	MultiLngPrnReceiptStr(_T("TID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("tid", temp);
	sprintf(szBuff,"%s", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);


	SysGetTimeIso(timeGotten);
	datetime[0] = timeGotten[4];
	datetime[1] = timeGotten[5];
	datetime[2] = '/';
	datetime[3] = timeGotten[2];
	datetime[4] = timeGotten[3];
	datetime[5] = '/';
	datetime[6] = '2';
	datetime[7] = '0';
	datetime[8] = timeGotten[0];
	datetime[9] = timeGotten[1];
	
	////ShowLogs(1, "Time is: %s", datetime);
	//sprintf(out, "%04d%02d%02d000000", ny, nm, nd);
	memset(preDateComp, '\0', strlen(preDateComp));
	GetDateTime(preDateComp);
	ShowLogs(1, "PRE: %s", preDateComp);
	memset(dateComp, '\0', strlen(dateComp));
    Conv2EngTime2(preDateComp, dateComp);
    ShowLogs(1, "FINAL: %s", dateComp);

	
	MultiLngPrnReceiptStr(_T("DATE:           "), GUI_ALIGN_LEFT);
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%s\n", datetime);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
	
	



	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	MultiLngPrnReceiptStr(" Time TT    PAN         AMT  St\n", GUI_ALIGN_LEFT);
	//MultiLngPrnReceiptStr("DETAILS\n", GUI_ALIGN_CENTER);
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);



	//recieptno|txntype|date|time|appname|expdate|maskedpan|holdername|aid|amount|cashback|total|paymentdetails|
	//rspcode|authcode|stan|rrn

	ShowLogs(1, "Key: %s", dateComp);
	char *luv = strstr(txn, dateComp);
	if(luv == NULL)
    {
        ShowLogs(1, "NOT AVAILABLE");
        DisplayInfoNone("", "Transaction not Seen", 2);
		sysalloc_free(txn);
		return;
    }else
    {
    	int useLen = strlen(txn) - strlen(luv) - 1;
	    ShowLogs(1, "INITIAL USELEN: %d", useLen);
	    while(useLen--)
	    {
	        if((txn[useLen] == '\n')
	           || (useLen == 0))
	            break;
	    }
	    ShowLogs(1, "FINAL USELEN: %d", useLen);
	    i = useLen + 1;
	}

	for(; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			count++;
			memset(displ, '\0', strlen(displ)); 
			sprintf(displ, "EOD PROCESSING \n%d/%d", count, total);
			//DisplayInfoNone("", displ, 0);
			DisplayInfoNone("", "Printing...", 0);

			olubayo++;
			j = 0;
			memset(temp, '\0', strlen(temp));
			//memset(details, 0, sizeof(details));
			for(l = 0; l < 20; l++)
			{
				memset(details[l], '\0', sizeof(details[l]));
			}
			n = 0;
			for(k = 0; k < strlen(line); k++)
			{
				if(line[k] == '|')
		        {
		            strncpy(details[n], temp, strlen(temp));
		            j = 0;
		            n++;
		            memset(temp, 0, strlen(temp));
		        }else
		        {
		            temp[j] = line[k];
		            j++;
		        }
		    }

		    if(checkifToday(datetime, details[2]))
		    	continue;
		    ////ShowLogs(1, "Value 2: %s", details[2]);


		    memset(sBuff, '\0', sizeof(sBuff));
		    parseTimeEod(details[3], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
		    parseTxnEod(details[1], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
		    parsePanEod(details[6], sBuff);
		    MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseTotal(details[12], details[9], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseStatusEod(details[12], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			PrnStr("\n");


			memset(line, '\0', strlen(line));
			j = 0;
			while( 1 )
			{
				int ucRet = PrnStart();
				if( ucRet!=PRN_OK )
				{
					iRet = DispPrnReprintError(ucRet);
					if( ucRet!=ERR_PRN_PAPEROUT )
					{
						break;
					}

					if( GUI_ERR_USERCANCELLED == iRet||
						GUI_ERR_TIMEOUT == iRet)
					{
						Gui_ClearScr();
						Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("Reprint"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
						break;
					}
				}
				break;
			}
			PrnInit();
			olubayo = 0;


			/*if(olubayo == 50)
			{
				DispPrinting();
				StartPrinterReprint();
				PrnInit();
				olubayo = 0;
			}*/
			continue;
		}else
		{
			line[j] = txn[i];
			j++;
		}
	}

	transaction = transactionpassed + transactionfailed;    

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Conclusion\n");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transaction);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Approved Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionpassed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Declined Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionfailed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Approved Amount: \nNGN ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%.2f", totalamt);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n\n\n\n\n\n\n\n\n\n\n");

	//Do summary here
	DispPrinting();
    StartPrinterReprint();

    /*CreateWrite("receipt.txt", "0");
	CreateWrite("reprint.txt", "0");
	CreateWrite("count.txt", "0");
	CreateWrite("eod.txt", "0");
	CreateWrite("eod2.txt", "0");*/

    totalamt = 0.00;
    transaction = 0;
    transactionpassed = 0;
	transactionfailed = 0;
	DisplayInfoNone("", "Done", 2);
	sysalloc_free(txn);
}

int checkSame = 0;
void eodCollection()
{
	//char txn[1000 * 1024] = {0};
	char *txn = (char*)sysalloc_malloc((sizeof(char) * filesize("reprint.txt")) + 1);
	char details[20][128];
	int iRet, i, k, loop, j, n = 0, l;
	char line[1024] = {0};
	char temp[128] = {0};
	char displ[128] = {0};
	char szBuff[128] = {0};
	int iLen;
	char sBuff[252] = {0};
	char timeGotten[15] = {0};
	char datetime[15] = {0};
	char dtM[15] = {0};
	char day[3] = {0};
	char mnth[3] = {0};
	char year[5] = {0};
	int olubayo = 0;
	int count = 0;
	int total = 0;

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

	//DisplayInfoNone("", "PROCESSING", 0);

	PrnInit();
	PrnSelectFont(&font2,NULL);
	PrnStr("\n\n");

	//memset(txn, '\0', strlen(txn));
	//memset(details, 0, sizeof(details));
	for(l = 0; l < 20; l++)
	{
		memset(details[l], '\0', sizeof(details[l]));
	}
	memset(line, '\0', strlen(line));


	////ShowLogs(1, "Size of reprint.txt: %lu", filesize("reprint.txt"));


	iRet = ReadAllData("reprint.txt", txn);
	//ShowLogs(1, "LOGS: %s", txn);//Delete asap
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		sysalloc_free(txn);
		return;
	}
	totalamt = 0.00;
	total = getTotalCountEod(txn);
	loop = 1;
	j = 0;

	////ShowLogs(1, "%s", txn);

	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("bankname", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merName", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merAddr", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	PrnStep(5);
	PrnStr("\n");

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	MultiLngPrnReceiptStr(_T("MID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("txnMid", temp);
	sprintf(szBuff,"%s\n", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

	MultiLngPrnReceiptStr(_T("TID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("tid", temp);
	sprintf(szBuff,"%s", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);


	SysGetTimeIso(timeGotten);

	datetime[0] = timeGotten[4];
	datetime[1] = timeGotten[5];
	datetime[2] = '/';
	datetime[3] = timeGotten[2];
	datetime[4] = timeGotten[3];
	datetime[5] = '/';
	datetime[6] = '2';
	datetime[7] = '0';
	datetime[8] = timeGotten[0];
	datetime[9] = timeGotten[1];
	

	/*MultiLngPrnReceiptStr(_T("DATE:           "), GUI_ALIGN_LEFT);
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%s\n", datetime);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);*/


	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	MultiLngPrnReceiptStr(" Time TT    PAN         AMT  St\n", GUI_ALIGN_LEFT);
	//MultiLngPrnReceiptStr("DETAILS\n", GUI_ALIGN_CENTER);
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	//recieptno|txntype|date|time|appname|expdate|maskedpan|holdername|aid|amount|cashback|total|paymentdetails|
	//rspcode|authcode|stan|rrn
	DisplayInfoNone("", "Printing", 0);
	for(i = 0; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			ShowLogs(1, "LINE: %s", line);
			count++;
			memset(displ, '\0', strlen(displ)); 
			sprintf(displ, "EOD PROCESSING \n%d/%d", count, total);
			//DisplayInfoNone("", displ, 0);
			//DisplayInfoNone("", "PROCESSING...", 0);

			olubayo++;
			j = 0;
			memset(temp, '\0', strlen(temp));
			//memset(details, 0, sizeof(details));
			for(l = 0; l < 20; l++)
			{
				memset(details[l], '\0', sizeof(details[l]));
			}
			n = 0;
			for(k = 0; k < strlen(line); k++)
			{
				if(line[k] == '|')
		        {
		            strncpy(details[n], temp, strlen(temp));
		            j = 0;
		            n++;
		            memset(temp, 0, strlen(temp));
		        }else
		        {
		            temp[j] = line[k];
		            j++;
		        }
		    }

		    checkSame = 0;
		    if(checkifDifferentDays(day, mnth, year, details[2], dtM))
		    {	
		    	if(checkSame == 1)
		    	{
		    		memset(szBuff, 0, sizeof(szBuff));
		    		MultiLngPrnReceiptStr(_T("DATE:               "), GUI_ALIGN_LEFT);
					sprintf(szBuff,"%s\n", dtM);
					MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		    	}
		    }

		    memset(sBuff, '\0', sizeof(sBuff));
		    parseTimeEod(details[3], sBuff);
		    ShowLogs(1, "TIME: %s", sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
		    parseTxnEod(details[1], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
		    parsePanEod(details[6], sBuff);
		    MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseTotal(details[12], details[9], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseStatusEod(details[12], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			PrnStr("\n");
			memset(line, '\0', strlen(line));
			
			j = 0;

			while( 1 )
			{
				int ucRet = PrnStart();
				if( ucRet!=PRN_OK )
				{
					iRet = DispPrnReprintError(ucRet);
					if( ucRet!=ERR_PRN_PAPEROUT )
					{
						break;
					}

					if( GUI_ERR_USERCANCELLED == iRet||
						GUI_ERR_TIMEOUT == iRet)
					{
						Gui_ClearScr();
						Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("Reprint"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
						break;
					}
				}
				break;
			}

			//DispPrinting();
			//StartPrinterReprint();
			//DelayMs(100);
			PrnInit();
			olubayo = 0;
			//Wisdom commented this out

			/*if(olubayo == 50)
			{
				DispPrinting();
				StartPrinterReprint();
				PrnInit();
				olubayo = 0;
			}*/
			continue;
		}else
		{
			line[j] = txn[i];
			j++;
		}
	}




	transaction = transactionpassed + transactionfailed;    

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Conclusion\n");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transaction);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Approved Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionpassed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Declined Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionfailed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Approved Amount: \nNGN ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%.2f", totalamt);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n\n\n\n\n\n\n\n\n\n\n");

	//Do summary here
	DispPrinting();
    StartPrinterReprint();

    totalamt = 0.00;
    transaction = 0;
    transactionpassed = 0;
	transactionfailed = 0;
	DisplayInfoNone("", "Done", 2);
	sysalloc_free(txn);
}

void eodCashier()
{
	//char txn[1000 * 1024] = {0};
	char *txn = (char*)sysalloc_malloc((sizeof(char) * filesize("cashierreprint.txt")) + 1);
	char details[20][128];
	int iRet, i, k, loop, j, n = 0, l;
	char line[1024] = {0};
	char temp[128] = {0};
	char displ[128] = {0};
	char szBuff[128] = {0};
	int iLen;
	char sBuff[252] = {0};
	char timeGotten[15] = {0};
	char datetime[15] = {0};
	char dtM[15] = {0};
	char day[3] = {0};
	char mnth[3] = {0};
	char year[5] = {0};
	int olubayo = 0;
	int count = 0;
	int total = 0;

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

	//DisplayInfoNone("", "PROCESSING", 0);

	PrnInit();
	PrnSelectFont(&font2,NULL);
	PrnStr("\n\n");

	//memset(txn, '\0', strlen(txn));
	//memset(details, 0, sizeof(details));
	for(l = 0; l < 20; l++)
	{
		memset(details[l], '\0', sizeof(details[l]));
	}
	memset(line, '\0', strlen(line));


	////ShowLogs(1, "Size of reprint.txt: %lu", filesize("reprint.txt"));


	iRet = ReadAllData("cashierreprint.txt", txn);
	//ShowLogs(1, "LOGS: %s", txn);//Delete asap
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		sysalloc_free(txn);
		return;
	}
	totalamt = 0.00;
	total = getTotalCountEod(txn);
	loop = 1;
	j = 0;

	////ShowLogs(1, "%s", txn);

	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("bankname", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merName", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merAddr", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	PrnStep(5);
	PrnStr("\n");

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	MultiLngPrnReceiptStr(_T("MID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("txnMid", temp);
	sprintf(szBuff,"%s\n", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

	MultiLngPrnReceiptStr(_T("TID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("tid", temp);
	sprintf(szBuff,"%s", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);


	SysGetTimeIso(timeGotten);

	datetime[0] = timeGotten[4];
	datetime[1] = timeGotten[5];
	datetime[2] = '/';
	datetime[3] = timeGotten[2];
	datetime[4] = timeGotten[3];
	datetime[5] = '/';
	datetime[6] = '2';
	datetime[7] = '0';
	datetime[8] = timeGotten[0];
	datetime[9] = timeGotten[1];
	

	/*MultiLngPrnReceiptStr(_T("DATE:           "), GUI_ALIGN_LEFT);
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%s\n", datetime);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);*/


	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	MultiLngPrnReceiptStr(" Time TT    PAN         AMT  St\n", GUI_ALIGN_LEFT);
	//MultiLngPrnReceiptStr("DETAILS\n", GUI_ALIGN_CENTER);
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	//recieptno|txntype|date|time|appname|expdate|maskedpan|holdername|aid|amount|cashback|total|paymentdetails|
	//rspcode|authcode|stan|rrn
	DisplayInfoNone("", "Printing", 0);
	for(i = 0; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			ShowLogs(1, "LINE: %s", line);
			count++;
			memset(displ, '\0', strlen(displ)); 
			sprintf(displ, "EOD PROCESSING \n%d/%d", count, total);
			//DisplayInfoNone("", displ, 0);
			//DisplayInfoNone("", "PROCESSING...", 0);

			olubayo++;
			j = 0;
			memset(temp, '\0', strlen(temp));
			//memset(details, 0, sizeof(details));
			for(l = 0; l < 20; l++)
			{
				memset(details[l], '\0', sizeof(details[l]));
			}
			n = 0;
			for(k = 0; k < strlen(line); k++)
			{
				if(line[k] == '|')
		        {
		            strncpy(details[n], temp, strlen(temp));
		            j = 0;
		            n++;
		            memset(temp, 0, strlen(temp));
		        }else
		        {
		            temp[j] = line[k];
		            j++;
		        }
		    }

		    checkSame = 0;
		    if(checkifDifferentDays(day, mnth, year, details[2], dtM))
		    {	
		    	if(checkSame == 1)
		    	{
		    		memset(szBuff, 0, sizeof(szBuff));
		    		MultiLngPrnReceiptStr(_T("DATE:               "), GUI_ALIGN_LEFT);
					sprintf(szBuff,"%s\n", dtM);
					MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
		    	}
		    }

		    memset(sBuff, '\0', sizeof(sBuff));
		    parseTimeEod(details[3], sBuff);
		    ShowLogs(1, "TIME: %s", sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			
			memset(sBuff, '\0', sizeof(sBuff));
		    parseTxnEod(details[1], sBuff);
		    MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
		    parsePanEod(details[6], sBuff);
		    MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseTotal(details[12], details[9], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			MultiLngPrnReceiptStr("  ", GUI_ALIGN_LEFT);
			memset(sBuff, '\0', sizeof(sBuff));
			parseStatusEod(details[12], sBuff);
			MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
			PrnStr("\n");
			memset(line, '\0', strlen(line));
			j = 0;

			while( 1 )
			{
				int ucRet = PrnStart();
				if( ucRet!=PRN_OK )
				{
					iRet = DispPrnReprintError(ucRet);
					if( ucRet!=ERR_PRN_PAPEROUT )
					{
						break;
					}

					if( GUI_ERR_USERCANCELLED == iRet||
						GUI_ERR_TIMEOUT == iRet)
					{
						Gui_ClearScr();
						Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("Reprint"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
						break;
					}
				}
				break;
			}

			//DispPrinting();
			//StartPrinterReprint();
			//DelayMs(100);
			PrnInit();
			olubayo = 0;
			//Wisdom commented this out

			/*if(olubayo == 50)
			{
				DispPrinting();
				StartPrinterReprint();
				PrnInit();
				olubayo = 0;
			}*/
			continue;
		}else
		{
			line[j] = txn[i];
			j++;
		}
	}




	transaction = transactionpassed + transactionfailed;    

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Conclusion\n");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transaction);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Approved Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionpassed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Declined Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionfailed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Approved Amount: \nNGN ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%.2f", totalamt);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n\n\n\n\n\n\n\n\n\n\n");

	//Do summary here
	DispPrinting();
    StartPrinterReprint();

    totalamt = 0.00;
    transaction = 0;
    transactionpassed = 0;
	transactionfailed = 0;
	DisplayInfoNone("", "Done", 2);
	sysalloc_free(txn);
}



int getStartAndEnd(char *start, char *end)
{
	int iRet = DisplayMsg("", "Begin(YYYYMMDD)", "YYYYMMDD", start, 8, 8);
	ShowLogs(1, "Start: %s", start);
	if(strlen(start) < 1 || iRet == GUI_ERR_USERCANCELLED)
		return 0;
	iRet = DisplayMsg("", "Stop(YYYYMMDD)", "YYYYMMDD", end, 8, 8);
	ShowLogs(1, "End: %s", end);
	if(strlen(end) < 1 || iRet == GUI_ERR_USERCANCELLED)
		return 0;

	return 1;
}

int days_in_month[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

struct date {
  int day;
  int month;
  int year;
};

int leap_year(int year) 
{
    if(year%400==0) return 1;

    if(year%4==0 && year%100!=0) return 1;

    return 0;
}

int correct(struct date d) 
{
    if(d.day < 1 || d.day > days_in_month[d.month]) return 0;

    if(d.month < 1 || d.month > 12) return 0;

    return 1;
}

int number_of_days(struct date d) 
{
    int result = 0;
    int i;

    for(i=1; i < d.year; i++) {
        if(leap_year(i))
            result += 366;
        else
            result += 365;
    }

    for(i=1; i < d.month; i++) {
        result += days_in_month[i];

        if(leap_year(d.year) && i == 2) result++;
    }

    result += d.day;
    return result;
}

int displayDif(int startDay, int startMonth, int startYear,
               int endDay, int endMonth, int endYear) 
{
    struct date first, second;
    int days;

    first.day = startDay;
    first.month = startMonth;
    first.year = startYear;

    second.day = endDay;
    second.month = endMonth;
    second.year = endYear;

    if(!correct(first) || !correct(second)) {
        ShowLogs(1, "Illegal date");
        return 0;
    }

    days = number_of_days(second) - number_of_days(first);

    ShowLogs(1, "%d Days", days);
    return days;
}



void printNextDate(int yyyy, int mm, int dd, int count, char *out)
{
    int nd,nm,ny,ndays;
    switch(mm)
    {
        case 1:case 3:case 5:case 7:case 8:case 10:case 12:
            ndays=31;
            break;
        case 4:case 6:case 9:case 11:
            ndays=30;
            break;
        case 2:
            if(yyyy%4==0)
                ndays=29;
            else
                ndays=28;
            break;
    }
    nd = dd+count;
    nm = mm;
    ny = yyyy;

    int lop = nd / ndays;
    if(nd>ndays)
    {
        if(lop > 1)
            nd = nd - (2 * ndays) + 1;
        else
            nd = nd - ndays;
        nm = nm + lop;
    }

    if(nm>12)
    {
        nm=1;
        ny++;
    }
    sprintf(out, "%04d%02d%02d000000", ny, nm, nd);
}


void daterangeeodCollection()
{
	char *txn = (char*)sysalloc_malloc((sizeof(char) * filesize("reprint.txt")) + 1);
	char details[20][128];
	int iRet, i, k, loop, j, n = 0, l;
	char line[1024] = {0};
	char temp[128] = {0};
	char displ[128] = {0};
	char szBuff[128] = {0};
	int iLen;
	char sBuff[252] = {0};
	char timeGotten[15] = {0};
	char datetime[15] = {0};
	char dtM[15] = {0};
	char day[3] = {0};
	char mnth[3] = {0};
	char year[5] = {0};
	int olubayo = 0;
	int count = 0;
	int total = 0;

	char start[20] = {0};
	char end[20] = {0};

	if(getStartAndEnd(start, end) == 0)
	{
		DisplayInfoNone("", "Canceled", 2);
		sysalloc_free(txn);
		return;
	}

	ShowLogs(1, "Start: %s", start);
	ShowLogs(1, "End: %s", end);
	if((strlen(start) != 8) || (strlen(end) != 8))
	{
		DisplayInfoNone("", "Error", 2);
		sysalloc_free(txn);
		return;
	}
	char dates[365][15];
    memset(temp, '\0', strlen(temp));
    strncpy(temp, start, 4);
    int sA = atoi(temp);
    memset(temp, '\0', strlen(temp));
    strncpy(temp, start + 4, 2);
    int sB = atoi(temp);
    memset(temp, '\0', strlen(temp));
    strncpy(temp, start + 6, 2);
    int sC = atoi(temp);
    memset(temp, '\0', strlen(temp));
    strncpy(temp, end, 4);
    int eA = atoi(temp);
    memset(temp, '\0', strlen(temp));
    strncpy(temp, end + 4, 2);
    int eB = atoi(temp);
    memset(temp, '\0', strlen(temp));
    strncpy(temp, end + 6, 2);
    int eC = atoi(temp);
    int ret = displayDif(sC, sB, sA, eC, eB, eA);
    ShowLogs(1, "Number: %d", ret);
    if(ret <= 0)
    {
        /* 
		ShowLogs(1, "No date difference");
        DisplayInfoNone("", "Error", 2);
		sysalloc_free(txn);
        return;
		*/
		ret = 1;
    }
    i = 0;
    for(i = 0; i < 365; i++)
	{
		memset(dates[i], '\0', sizeof(dates[i]));
	}

    i = 0;
    for(i = 0; i <= ret; i++)
    {
        char out[13] = {0};
        char printyear[20] = {0};
        memset(out, '\0', strlen(out));
        memset(printyear, '\0', strlen(printyear));
        if(i == 0)
        	printNextDate(sA, sB, sC, 0, printyear);
        else
        	printNextDate(sA, sB, sC, 1, printyear);
        memset(temp, '\0', strlen(temp));
        strncpy(temp, printyear, 4);
        sA = atoi(temp);
        memset(temp, '\0', strlen(temp));
        strncpy(temp, printyear + 4, 2);
        sB = atoi(temp);
        memset(temp, '\0', strlen(temp));
        strncpy(temp, printyear + 6, 2);
        sC = atoi(temp);
        Conv2EngTime2(printyear, out);
        strcpy(dates[i], out);
        if(i > 364)
        	break;
        ShowLogs(1, "%s.", dates[i]);
    }

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

	//DisplayInfoNone("", "PROCESSING", 0);




	PrnInit();
	PrnSelectFont(&font2,NULL);
	PrnStr("\n\n");

	//memset(txn, '\0', strlen(txn));
	//memset(details, 0, sizeof(details));
	for(l = 0; l < 20; l++)
	{
		memset(details[l], '\0', sizeof(details[l]));
	}
	memset(line, '\0', strlen(line));


	////ShowLogs(1, "Size of reprint.txt: %lu", filesize("reprint.txt"));


	iRet = ReadAllData("reprint.txt", txn);
	//ShowLogs(1, "LOGS: %s", txn);//Delete asap
	if(strlen(txn) < 15)
	{
		DisplayInfoNone("", "Empty", 2);
		sysalloc_free(txn);
		return;
	}
	totalamt = 0.00;
	total = getTotalCountEod(txn);
	loop = 1;
	j = 0;

	////ShowLogs(1, "%s", txn);

	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("bankname", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merName", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnv("merAddr", temp);
	DD(temp, szBuff);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_CENTER);
	PrnStep(5);
	PrnStr("\n");

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	MultiLngPrnReceiptStr(_T("MID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("txnMid", temp);
	sprintf(szBuff,"%s\n", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);

	MultiLngPrnReceiptStr(_T("TID:            "), GUI_ALIGN_LEFT);
	memset(temp, 0, sizeof(temp));
	memset(szBuff, 0, sizeof(szBuff));
	UtilGetEnvEx("tid", temp);
	sprintf(szBuff,"%s", temp);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);


	SysGetTimeIso(timeGotten);

	datetime[0] = timeGotten[4];
	datetime[1] = timeGotten[5];
	datetime[2] = '/';
	datetime[3] = timeGotten[2];
	datetime[4] = timeGotten[3];
	datetime[5] = '/';
	datetime[6] = '2';
	datetime[7] = '0';
	datetime[8] = timeGotten[0];
	datetime[9] = timeGotten[1];
	

	/*MultiLngPrnReceiptStr(_T("DATE:           "), GUI_ALIGN_LEFT);
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%s\n", datetime);
	MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);*/


	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	MultiLngPrnReceiptStr(" Time TT    PAN         AMT  St\n", GUI_ALIGN_LEFT);
	//MultiLngPrnReceiptStr("DETAILS\n", GUI_ALIGN_CENTER);
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);

	//recieptno|txntype|date|time|appname|expdate|maskedpan|holdername|aid|amount|cashback|total|paymentdetails|
	//rspcode|authcode|stan|rrn
	DisplayInfoNone("", "Printing...", 0);
	
	int q = 0;
	int seq = 0;
	char prevDate[20] = {0};
	for(q = 0; q <= ret; q++)
	{
		ShowLogs(1, "Key: %s", dates[q]);
		/*memset(prevDate, '\0', strlen(prevDate));
    	if(q > 0)
			strcpy(prevDate, dates[q - 1]);
		ShowLogs(1, "PREVIOUS DATE: %s", prevDate);
		if(strstr(prevDate, dates[q]) != NULL)
		{
			ShowLogs(1, "ALREADY PROCESSED");
	        continue;
		}*/
		//char *luv = NULL;
		char *luv = strstr(txn, dates[q]);
		if(luv == NULL)
	    {
	        ShowLogs(1, "NOT AVAILABLE");
	        continue;
	    }else
	    {
	    	int useLen = strlen(txn) - strlen(luv) - 1;
		    ShowLogs(1, "INITIAL USELEN: %d", useLen);
		    while(useLen--)
		    {
		        if((txn[useLen] == '\n')
		           || (useLen == 0))
		            break;
		    }
		    ShowLogs(1, "FINAL USELEN: %d", useLen);
		    i = useLen + 1;
		    for(; i < strlen(txn); i++)
			{
				if(txn[i] == '\n')
				{
					ShowLogs(1, "LINE: %s", line);
					
					count++;
					memset(displ, '\0', strlen(displ)); 
					sprintf(displ, "EOD PROCESSING \n%d/%d", count, total);

					olubayo++;
					j = 0;
					memset(temp, '\0', strlen(temp));
					//memset(details, 0, sizeof(details));
					for(l = 0; l < 20; l++)
					{
						memset(details[l], '\0', sizeof(details[l]));
					}
					n = 0;
					for(k = 0; k < strlen(line); k++)
					{
						if(line[k] == '|')
				        {
				            strncpy(details[n], temp, strlen(temp));
				            j = 0;
				            n++;
				            memset(temp, 0, strlen(temp));
				        }else
				        {
				            temp[j] = line[k];
				            j++;
				        }
				    }
				    
				    if(strncmp(details[2], dates[q], strlen(details[2])) != 0)
				    {
				    	olubayo--;
						j = 0;
						memset(line, '\0', strlen(line));
						memset(temp, '\0', strlen(temp));
				    	count--;
				    	break;
				    }

				    //

				    //Check here
				    checkSame = 0;
				    if(checkifDifferentDays(day, mnth, year, details[2], dtM))
				    {	
				    	if(checkSame == 1)
				    	{
				    		memset(szBuff, 0, sizeof(szBuff));
				    		MultiLngPrnReceiptStr(_T("DATE:               "), GUI_ALIGN_LEFT);
							sprintf(szBuff,"%s\n", dtM);
							MultiLngPrnReceiptStr(szBuff, GUI_ALIGN_LEFT);
				    	}
				    }

				    memset(sBuff, '\0', sizeof(sBuff));
				    parseTimeEod(details[3], sBuff);
				    ShowLogs(1, "TIME: %s", sBuff);
					MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
					MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
					memset(sBuff, '\0', sizeof(sBuff));
				    parseTxnEod(details[1], sBuff);
					MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
					MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
					memset(sBuff, '\0', sizeof(sBuff));
				    parsePanEod(details[6], sBuff);
					MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
					MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
					memset(sBuff, '\0', sizeof(sBuff));
					parseTotal(details[12], details[9], sBuff);
					MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
					MultiLngPrnReceiptStr(" ", GUI_ALIGN_LEFT);
					memset(sBuff, '\0', sizeof(sBuff));
					parseStatusEod(details[12], sBuff);
					MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
					PrnStr("\n");
					memset(line, '\0', strlen(line));
					j = 0;

					while( 1 )
					{
						int ucRet = PrnStart();
						if( ucRet!=PRN_OK )
						{
							iRet = DispPrnReprintError(ucRet);
							if( ucRet!=ERR_PRN_PAPEROUT )
							{
								break;
							}

							if( GUI_ERR_USERCANCELLED == iRet||
								GUI_ERR_TIMEOUT == iRet)
							{
								Gui_ClearScr();
								Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("Reprint"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
								break;
							}
						}
						break;
					}

					//DispPrinting();
					//StartPrinterReprint();
					//DelayMs(100);
					PrnInit();
					olubayo = 0;
					//Wisdom commented this out

					/*if(olubayo == 50)
					{
						DispPrinting();
						StartPrinterReprint();
						PrnInit();
						olubayo = 0;
					}*/
					continue;
				}else
				{
					line[j] = txn[i];
					j++;
				}
			}
	    }
	}

	transaction = transactionpassed + transactionfailed;    

	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Conclusion\n");
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transaction);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Approved Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionpassed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Number of Declined Transaction: ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%d", transactionfailed);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	memset(sBuff, '\0', sizeof(sBuff));
	strcpy(sBuff, "Approved Amount: \nNGN ");
	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%.2f", totalamt);
	strcat(sBuff, temp);
	MultiLngPrnReceiptStr(sBuff, GUI_ALIGN_LEFT);
	PrnStr("\n");
	MultiLngPrnReceiptStr("-------------------------------\n", GUI_ALIGN_LEFT);
	PrnStep(5);
	PrnStr("\n\n\n\n\n\n\n\n\n\n\n\n");

	//Do summary here
	DispPrinting();
    StartPrinterReprint();

    totalamt = 0.00;
    transaction = 0;
    transactionpassed = 0;
	transactionfailed = 0;
	DisplayInfoNone("", "Done", 2);
	sysalloc_free(txn);
}