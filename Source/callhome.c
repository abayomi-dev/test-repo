#include "global.h"


void parseAmt(char *init, char *fin)
{
	int i = 0, j = 0, k = 0, m = 0, n = 0;
	char store[13] = {0};
	for(i = 0; i < 12; i++)
	{
		if(init[i] != '0')
		{
			j = i;
			break;
		}
	}
	strncpy(store, init + j, strlen(init) - j);
	if(strlen(store) == 1)
    {
        sprintf(fin, "0.0%s", store);
        return;
    }
	for(k = 0; k < strlen(store); k++)
	{
		m = strlen(store) - k;
		if(m == 2)
		{
			if(strlen(fin) < 1)
			{
				fin[n] = '0';
				n++;
			}
			fin[n] = '.';
			n++;
			fin[n] = store[k];
			n++;
		}else
		{
			fin[n] = store[k];
			n++;
		}
	}
}

void MaskAllPanReceipt(char *pan, char *mskPan)
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

//stan|pan|authcode|rrn|amount|timestamp|mti|ps|resp|extras\n
void packNew(char *cTxn)
{
	char pan[20] = {0};
	char amt[15] = {0};
	char timeGotten[15] = {0};
	strcpy(cTxn, glSendPack.szSTAN);
	strcat(cTxn, "|");
	memset(pan, '\0', strlen(pan));
	MaskAllPanReceipt(glSendPack.szPan, pan);
	strcat(cTxn, pan);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szAuthCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szRRN);
	strcat(cTxn, "|");
	memset(amt, '\0', strlen(amt));
	parseAmt(glSendPack.szTranAmt, amt);
	strcat(cTxn, amt);
	strcat(cTxn, "|");
	strcat(cTxn, "20");
	memset(timeGotten, '\0', strlen(timeGotten));
	SysGetTimeIso(timeGotten);
	strcat(cTxn, timeGotten);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szMsgCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szProcCode);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szRspCode);
	strcat(cTxn, "|");
	strcat(cTxn, glProcInfo.stTranLog.szAppLabel);
	strcat(cTxn, "|");

	switch(txnType)
	{
		case 1:
			strcat(cTxn, "PURCHASE");
			break;
		case 2:
			strcat(cTxn, "CASH ADVANCE");
			break;
		case 3:
			strcat(cTxn, "PREAUTH");
			break;
		case 4:
			strcat(cTxn, "REFUND");
			break;
		case 5:
			strcat(cTxn, "BALANCE ENQUIRY");
			break;
		case 6:
			strcat(cTxn, "SALES COMPLETION");
			break;
		case 7:
			strcat(cTxn, "REVERSAL");
			break;
		case 8:
			strcat(cTxn, "CASH BACK");
			break;
		case 15:
			strcat(cTxn, "ARCAPAY");
			break;
		case 16:
			strcat(cTxn, "CASH DEPOSIT");
			break;
		case 17:
			strcat(cTxn, "TRANSFER");
			break;
		case 18:
			strcat(cTxn, "CASH WITHDRAWAL");
			break;
		default:
			strcat(cTxn, "PURCHASE");
			break;
	}
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.extras);
	strcat(cTxn, "|");
	strcat(cTxn, glProcInfo.stTranLog.szHolderName);
	strcat(cTxn, "|");
	strcat(cTxn, "\n");
}	

void packOld(char *cTxn)
{
	char pan[20] = {0};
	char amt[15] = {0};
	char timeGotten[15] = {0};
	strcpy(cTxn, glSendPack.szSTAN);
	strcat(cTxn, "|");
	memset(pan, '\0', strlen(pan));
	MaskAllPanReceipt(glSendPack.szPan, pan);
	strcat(cTxn, pan);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szAuthCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szRRN);
	strcat(cTxn, "|");
	memset(amt, '\0', strlen(amt));
	parseAmt(glSendPack.szTranAmt, amt);
	strcat(cTxn, amt);
	strcat(cTxn, "|");
	strcat(cTxn, "20");
	memset(timeGotten, '\0', strlen(timeGotten));
	SysGetTimeIso(timeGotten);
	strcat(cTxn, timeGotten);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szMsgCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szProcCode);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szRspCode);
	strcat(cTxn, "|");
	strcat(cTxn, glProcInfo.stTranLog.szAppLabel);
	strcat(cTxn, "|");
	switch(txnType)
	{
		case 1:
			strcat(cTxn, "PURCHASE");
			break;
		case 2:
			strcat(cTxn, "CASH ADVANCE");
			break;
		case 3:
			strcat(cTxn, "PREAUTH");
			break;
		case 4:
			strcat(cTxn, "REFUND");
			break;
		case 5:
			strcat(cTxn, "BALANCE ENQUIRY");
			break;
		case 6:
			strcat(cTxn, "SALES COMPLETION");
			break;
		case 7:
			strcat(cTxn, "REVERSAL");
			break;
		case 8:
			strcat(cTxn, "CASH BACK");
			break;
		case 15:
			strcat(cTxn, "ARCAPAY");
			break;
		case 16:
			strcat(cTxn, "CASH DEPOSIT");
			break;
		case 17:
			strcat(cTxn, "TRANSFER");
			break;
		case 18:
			strcat(cTxn, "CASH WITHDRAWAL");
			break;
		default:
			strcat(cTxn, "PURCHASE");
			break;
	}
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.extras);
	strcat(cTxn, "|");
	strcat(cTxn, glProcInfo.stTranLog.szHolderName);
	strcat(cTxn, "|");
	strcat(cTxn, "\n");
	/*char pan[20] = {0};
	char amt[15] = {0};
	char timeGotten[15] = {0};
	//strcpy(cTxn, txn);
	strcpy(cTxn, ",{\"stan\": \"");
	strcat(cTxn, glRecvPack.szSTAN);
	strcat(cTxn, "\",\"hPan\": \"");
	strcat(cTxn, "not done yet");
	strcat(cTxn, "\",\"mPan\": \"");
	memset(pan, '\0', strlen(pan));
	MaskAllPanReceipt(glProcInfo.stTranLog.szPan, pan);
	strcat(cTxn, pan);
	strcat(cTxn, "\",\"rrn\": \"");
	strcat(cTxn, glRecvPack.szRRN);
	strcat(cTxn, "\",\"acode\": \"");
	strcat(cTxn, glRecvPack.szAuthCode);
	strcat(cTxn, "\",\"amount\": \"");
	parseAmt(glRecvPack.szTranAmt, amt);
	strcat(cTxn, amt);
	strcat(cTxn, "\",\"timestamp\": \"20");
	SysGetTimeIso(timeGotten);
	strcat(cTxn, timeGotten);
	strcat(cTxn, "\",\"mti\": \"");
	strcat(cTxn, glSendPack.szMsgCode);
	strcat(cTxn, "\",\"ps\": \"");
	strcat(cTxn, glSendPack.szProcCode);
	strcat(cTxn, "\",\"resp\": \"");
	strcat(cTxn, glRecvPack.szRspCode);
	strcat(cTxn, "\",\"tap\": \"");
	strcat(cTxn, "false");
	strcat(cTxn, "\",\"rr\": \"");
	strcat(cTxn, "true");
	strcat(cTxn, "\",\"rep\": \"");
	strcat(cTxn, "true");
	strcat(cTxn, "\",\"vm\": \"");
	strcat(cTxn, "online");
	strcat(cTxn, "\",\"ostan\": \"");
	strcat(cTxn, glSendPack.szSTAN);
	strcat(cTxn, "\",\"orrn\": \"");
	strcat(cTxn, glSendPack.szRRN);
	strcat(cTxn, "\",\"oacode\": \"");
	strcat(cTxn, "******");
	strcat(cTxn, "\",\"tid\": \"");
	strcat(cTxn, glSendPack.szTermID);
	strcat(cTxn, "\",\"mid\": \"");
	strcat(cTxn, glSendPack.szMerchantID);
	strcat(cTxn, "\",\"mPin\": \"");
	strcat(cTxn, "false");
	strcat(cTxn, "\"}");*/
}

//First concern
void storeTxn()
{
	int iRet, iLen;
	char txn[512] = {0};
	memset(txn, '\0', strlen(txn));
	ShowLogs(1, "1. Filesize of tlog.txt: %lu", filesize("tlog.txt"));
	if(filesize("tlog.txt") < 10)
	{
		packNew(txn);
		iRet = CreateWrite("tlog.txt", txn);
	}else
	{
		packOld(txn);
		iRet = WriteUpdate("tlog.txt", txn, filesize("tlog.txt")); 
	}
	ShowLogs(1, "2. Filesize of tlog.txt: %lu", filesize("tlog.txt"));
	/*int iRet, iLen;
	//char *txn = (char*)malloc((sizeof(char) * filesize("tlog.txt")) + 1);
	//char *cTxn = (char*)malloc((sizeof(char) * filesize("tlog.txt")) + 500);
	char txn[200 * 1024] = {0};
	char cTxn[200 * 1024] = {0};
	ShowLogs(1, "About Storing Transaction 1.");
	//memset(txn, '\0', strlen(txn));
	//memset(cTxn, '\0', strlen(cTxn));
	ShowLogs(1, "About Storing Transaction 1.");
	iRet = ReadAllData("tlog.txt", txn);
	ShowLogs(1, "About Storing Transaction 2.");
	if(strlen(txn) < 50)
	{	
		ShowLogs(1, "About Storing Transaction 3a.");
		packNew(cTxn);
		ShowLogs(1, "About Storing Transaction 4a.");
		iRet = CreateWrite("tlog.txt", cTxn);
		ShowLogs(1, "About Storing Transaction 5b.");
	}else
	{
		ShowLogs(1, "About Storing Transaction 3b.");
		packOld(txn, cTxn);
		ShowLogs(1, "About Storing Transaction 4b.");
		iRet = CreateWrite("tlog.txt", cTxn);
		ShowLogs(1, "About Storing Transaction 5b.");
	}
	ShowLogs(1, "About Storing Transaction 6.");
	ShowLogs(1, "Stored Transaction: %s", cTxn);
	//free(txn);
	//free(cTxn);
	ShowLogs(1, "Filesize of tlog.txt: %lu", filesize("tlog.txt"));*/
}

void GetPrinterStatus(char *status) 
{
	uchar ucRet = 0;
	PrnInit();
	ucRet = PrnStatus();
	if (ucRet == 0x00) {
		strcpy(status, "Printer Ok");
	}else if (ucRet == 0xfc) {
		strcpy(status, "Lack of Font");
	}else if (ucRet == 0x01) {
		strcpy(status, "Printer busy");
	}else if (ucRet == 0x02) {
		strcpy(status, "Out of paper");
	}else if (ucRet == 0x03) {
		strcpy(status, "Data Error");
	}else if (ucRet == 0x04) {
		strcpy(status, "Printer problems");
	}else if (ucRet == 0x08) {
		strcpy(status, "Printer over heating");
	}else if (ucRet == 0x09) {
		strcpy(status, "Printer voltage is too low");
	}else if (ucRet == 0xf0) {
		strcpy(status, "Print unfinished");
	}else if (ucRet == 0xfe) {
		strcpy(status, "Package too long");
	}else
	{
		strcpy(status, "Unknown Error");
	}
}


int GetCellInfo(CellStationInformation *cellStationInformation){
	return 0;
}

int curSending = 0;//Used to track what to copy
int sendcount = 0;//Used to track sent and remaining

int getTotalCount(char* txn)
{
	int i, k = 0;
	for(i = 0; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
			k++;
	}
	return k;
}

int getCallhome(char* txn, char *temp)
{
	int i, k = 0;
	for(i = 0; i < strlen(txn); i++)
	{
		if(k >= 10)
			break;

		if(txn[i] == '\n')
			k++;
		temp[i] = txn[i];
	}
	ShowLogs(1, "1. Length of to Send: %d", strlen(temp));
	return k;
}

void PackCallHomeData()
{
	CellStationInformation cellStationInformation = { 0 };
	char myDataBuffer[100] = { '\0' };
	char printer[100] = {0};
	char cFin[60 * 1024] = {0};
	char txn[30 * 1024] = {0};
	char tpp[30 * 1024] = {0};
	char tpb[30 * 1024] = {0};
	char SN[33] = {0};
	uchar	szCurTime[16+1];
	uchar bl;
	int ibl = 0, j, o = 0;
	char cibl[4] = {0};
	uchar cs[100] = {0};
	uchar hb[10] = {0};
	char temp[100] = {0};
	char timeGotten[15] = {0};
	char serverIp[20] = {0};
	char serverPort[20] = {0};
	char tid[9] = {0};
	char mid[16] = {0};
	int port = 0;
	char responseServer[1024] = {0};
	char displ[100] = {0};
	long ind = 0L;

	memset(cFin, '\0', strlen(cFin)); 
	memset(txn, '\0', strlen(txn)); 
	memset(tpp, '\0', strlen(tpp)); 
	memset(tpb, '\0', strlen(tpb)); 

	ReadAllData("tlog.txt", txn);
	sendcount = getTotalCount(txn);
	if(strlen(txn) > 10)
	{
		ShowLogs(1, "Inside Check");
		memset(tpp, '\0', strlen(tpp)); 
		o = getCallhome(txn, tpp);
	}else
		strcpy(tpp, "");

	curSending = o;
	ShowLogs(1, "Total Filesize: %lu", filesize("tlog.txt"));
	ShowLogs(1, "Total to be Sent: %d", sendcount);
	ShowLogs(1, "Current Sending: %d", curSending);
	ShowLogs(1, "2. Length of to Send: %d", strlen(tpp));
	
	ind = strlen(tpp);

	for(j = 0; ; j++)
	{
		ScrBackLight(1);
		DisplayInfoNone("", "TMS SYNC...", 2);
		memset(displ, '\0', strlen(displ)); 
		sprintf(displ, "TXNS %d/%d", curSending, sendcount);

		DisplayInfoNone("", displ, 2);

		GetCellInfo(&cellStationInformation);
		sprintf(myDataBuffer, "cid:%i,lac:%i,mcc:%i,mnc:%i,ss:%i",
				cellStationInformation.cellID,
				cellStationInformation.locationAreaCode,
				cellStationInformation.mobileCountryCode,
				cellStationInformation.mobileNetworkCode,
				cellStationInformation.signalStrength);
		ShowLogs(1, "Inside PackCallHomeData");
		memset(cFin, '\0', strlen(cFin)); 
		strcpy(cFin, "{\"terminalInformation\": {");
		strcat(cFin, "\"state\": {");
		strcat(cFin, "\"serial\": \"");
		memset(SN, '\0', strlen(SN)); 
		ReadSN(SN);
		strcat(cFin, SN);
		strcat(cFin, "\",\"ctime\": \"");
		GetEngTime(szCurTime);
		strcat(cFin, szCurTime);
		strcat(cFin, "\",\"bl\": \"");
		bl = BatteryCheck();
		switch(bl)
		{
			case 0:
				ibl = 15;
				strcpy(cs, "Not Charging");
				strcpy(hb, "false");
				break;
			case 1:
				ibl = 30;
				strcpy(cs, "Not Charging");
				strcpy(hb, "true");
				break;
			case 2:
				ibl = 45;
				strcpy(cs, "Not Charging");
				strcpy(hb, "true");
				break;
			case 3:
				ibl = 60;
				strcpy(cs, "Not Charging");
				strcpy(hb, "true");
				break;
			case 4:
				ibl = 80;
				strcpy(cs, "Not Charging");
				strcpy(hb, "true");
				break;
			case 5:
				ibl = 90;
				strcpy(cs, "Charging");
				strcpy(hb, "true");
				break;
			case 6:
				ibl = 100;
				strcpy(cs, "Fully Charged");
				strcpy(hb, "true");
				break;
		}
		sprintf(cibl, "%d", ibl);
		strcat(cFin, cibl);
		strcat(cFin, "\",\"cs\": \"");
		strcat(cFin, cs);
		strcat(cFin, "\",\"ps\": \"");
		GetPrinterStatus(printer);
		strcat(cFin, printer);
		strcat(cFin, "\",\"tid\": \"");
		UtilGetEnv("tid", tid);
		strcat(cFin, tid);
		strcat(cFin, "\",\"mid\": \"");
		UtilGetEnvEx("txnMid", mid);//Corrected by Wisdom
		strcat(cFin, mid);
		strcat(cFin, "\",\"coms\": \"");
		memset(temp, '\0', strlen(temp)); 
		UtilGetEnv("cotype", temp);
		strcat(cFin, temp);
		strcat(cFin, "\",\"cloc\": \"");
		strcat(cFin, myDataBuffer);
		strcat(cFin, "\",\"tmn\": \"");
		strcat(cFin, "S90");
		strcat(cFin, "\",\"tmanu\": \"");
		strcat(cFin, "PAX");
		strcat(cFin, "\",\"hb\": \"");
		strcat(cFin, hb);
		strcat(cFin, "\",\"sv\": \"");
		strcat(cFin, EDC_VER_PUB);
		strcat(cFin, "\",\"lTxnAt\": \"20");
		SysGetTimeIso(timeGotten);
		strcat(cFin, timeGotten);
		strcat(cFin, "\",\"pads\": \"");
		strcat(cFin, "\"},");
		strcat(cFin, "\"ejournals\": {");
		strcat(cFin, "\"ejournal\": [");
		
		//Format here - start
		//stan|pan|authcode|rrn|amount|timestamp|mti|ps|resp\n
		if(strlen(tpp))
		{
			int w, loop = 0, u = 0, s = 1;
			char inp[50] = {0};
			memset(tpb, '\0', strlen(tpb));
			memset(inp, '\0', strlen(inp));
			for(w = 0; w < strlen(tpp); w++)
			{
				if(tpp[w] == '\n')
				{
					loop = 0;
					continue;
				}

				if(loop == 0)
				{
					if(tpp[w] == '|')
					{
						char curr[10] = {0};
						memset(curr, '\0', strlen(curr));
						UtilGetEnv("curcode", curr);

						loop++;
						u = 0;
						if(s == 1)
						{
							strcpy(tpb, "{\"stan\": \"");
							s = 0;
						}
						else
						{
							strcat(tpb, ",{\"stan\": \"");
						}
						strcat(tpb, inp);
						strcat(tpb, "\",\"transCur\": \"");
						strcat(tpb, curr);
						strcat(tpb, "\",\"hPan\": \"");
						strcat(tpb, "not done yet");
						strcat(tpb, "\",\"mPan\": \"");
						strcat(tpb, "\",\"ostan\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 1)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"mPan\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 2)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"acode\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 3)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"rrn\": \"");
						strcat(tpb, inp);
						strcat(tpb, "\",\"orrn\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 4)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"amount\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 5)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"timestamp\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 6)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"mti\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 7)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"ps\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 8)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"resp\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 9)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"cardType\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 10)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"paymentmethod\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 11)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"extras\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}else if(loop == 12)
				{
					if(tpp[w] == '|')
					{
						loop++;
						u = 0;
						strcat(tpb, "\",\"cardHolder\": \"");
						strcat(tpb, inp);
						strcat(tpb, "\",\"tap\": \"");
						strcat(tpb, "false");
						strcat(tpb, "\",\"rr\": \"");
						strcat(tpb, "true");
						strcat(tpb, "\",\"rep\": \"");
						strcat(tpb, "true");
						strcat(tpb, "\",\"vm\": \"");
						strcat(tpb, "online");
						strcat(tpb, "\",\"oacode\": \"");
						strcat(tpb, "******");
						memset(inp, '\0', strlen(inp));
						UtilGetEnv("tid", inp);
						strcat(tpb, "\",\"tid\": \"");
						strcat(tpb, inp);
						memset(inp, '\0', strlen(inp));
						UtilGetEnvEx("txnMid", inp);
						strcat(tpb, "\",\"mid\": \"");
						strcat(tpb, inp);
						strcat(tpb, "\",\"mPin\": \"");
						strcat(tpb, "false");
						strcat(tpb, "\"}");
						memset(inp, '\0', strlen(inp));
					}else
					{
						inp[u] = tpp[w];
						u++;
					}
				}
			}
			strcat(cFin, tpb);
		}else
		{
			ShowLogs(1, "Inside Third check");
			strcat(cFin, tpp);
		}
		//Format here - stop

		strcat(cFin, "]}}}");

		UtilGetEnvEx("tcmIP", serverIp);
		UtilGetEnvEx("tcmPort", serverPort);
		memset(responseServer, '\0', strlen(responseServer));
		if(httpPostData(serverIp, serverPort, "/arca/callhome/push", cFin,
			responseServer) == 0)
		{
			if(curSending < sendcount)
			{
				memset(tpp, '\0', strlen(tpp));
				CreateWrite("tlog.txt", txn + ind + 1);
				o = getCallhome(txn + ind + 1, tpp);
				curSending = curSending + o;
				ind = ind + strlen(tpp);
				ShowLogs(1, "%d. Length of to Send: %d", j, strlen(tpp));
				ShowLogs(1, "%d. Total Length Sent: %lu", j, ind);

				ShowLogs(1, "%d. Total to be Sent: %d", j, sendcount);
				ShowLogs(1, "%d. Current Sending: %d", j, curSending);
			}else
			{
				break;
			}
			
		}else
		{
			Gui_ClearScr();
			//free(txn);
			//free(cFin);
			//free(tpp);
			//free(tpb);
			return;
		}
	}
	CreateWrite("tlog.txt", "1");
	CreateWrite("count.txt", "0");
	curSending = 0;
	sendcount = 0;
	//free(txn);
	//free(cFin);
	//free(tpp);
	//free(tpb);
	DisplayInfoNone("", "TMS SYNC OK", 2);
	Gui_ClearScr();
}