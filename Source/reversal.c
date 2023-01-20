#include "global.h"

void formatReversalAmount(char *initAmt, char *finalAmt)
{
	int i, j, k;
	char temp[20] = {0};
	ShowLogs(1, "Init Amount: %s", initAmt);
	for(i = 0, j = 0; i < strlen(initAmt); i++)
	{
		if(initAmt[i] != '0')
		{
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

void parseReversalAmount(char *mainData, char *fin)
{
	char parseData[500] = { 0 };
	char storeData1[50] = { 0 };
	char storeData2[50] = { 0 };
	char finalData[500] = { 0 };

	ShowLogs(1, "Amount to parse: %s", mainData);
	memset(parseData, 0x0, sizeof(parseData));
	memset(storeData1, 0x0, sizeof(storeData1));
	memset(storeData2, 0x0, sizeof(storeData2));
	memset(finalData, 0x0, sizeof(finalData));

	strncpy(storeData1, mainData + 0, 10);
	strncpy(storeData2, mainData + 10, 2);

	ShowLogs(1, "Naira: %s", storeData1);
	ShowLogs(1, "Kobo: %s", storeData2);

	formatReversalAmount(storeData1, parseData);

	strcpy(finalData, "Do you want to \nReverse NGN ");
	strcat(finalData, parseData);
	strcat(finalData, ".");
	strcat(finalData, storeData2);
	strcat(finalData, "?");
	strcpy(fin, finalData);
	ShowLogs(1, "Reversal Amount Formatted: %s", finalData);
}

int parseInnerReversal(char *line, char *rrn, char *omti, char *odatetime, char *ocardno, char *oproc, 
				char *oamt, char *orrn, char *oseqnum, char *oentrymode, char *oposdatacode, char *ostan, char *acq)
{
	int i, loop, j;
	char temp[50] = {0};
	loop = 0;
	j = 0;
	if(strstr(line, rrn) != NULL)
	{
		ShowLogs(1, "2. The Line: %s", line);
		for(i = 0; i < strlen(line); i++)
		{
			if(loop == 0)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					omti[j] = line[i];
					j++;
				}
			}else if(loop == 1)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					odatetime[j] = line[i];
					j++;
				}
			}else if(loop == 2)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					ocardno[j] = line[i];
					j++;
				}
			}else if(loop == 3)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					oproc[j] = line[i];
					j++;
				}
			}else if(loop == 4)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					oamt[j] = line[i];
					j++;
				}
			}else if(loop == 5)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					orrn[j] = line[i];
					j++;
				}
			}else if(loop == 6)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					oseqnum[j] = line[i];
					j++;
				}
			}else if(loop == 7)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					oentrymode[j] = line[i];
					j++;
				}
			}else if(loop == 8)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					oposdatacode[j] = line[i];
					j++;
				}
			}else if(loop == 9)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					ostan[j] = line[i];
					j++;
				}
			}else if(loop == 10)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}
			}else if(loop == 11)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}
			}else if(loop == 12)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}
			}else if(loop == 13)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					acq[j] = line[i];
					j++;
				}
			}
		}
		return 1;
	}else
	{
		return 0;
	}
}

void getPanReversal(char *ocardno, char *pan, char *expdate, char *src)
{
	int i, j = 0, k = 0;
	for(i = 0; i < strlen(ocardno); i++)
	{
		if(ocardno[i] == 'D' || ocardno[i] == '=' || ocardno[i] == 'F'
			|| ocardno[i] == 'd' || ocardno[i] == 'f')
		{
			k = i + 1;
			break;
		}
		else
		{
			pan[j] = ocardno[i];
			j++;
		}
	}
	strncpy(expdate, ocardno + k, 4);
	strncpy(src, ocardno + k + 4, 3);
}

void replaceWord(char* result, const char *s, const char *oldW, const char *newW)
{
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;
            i += oldWlen - 1;
        }
    }
    i = 0;
    while (*s)
    {
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
    result[i] = '\0';
}

int parseEodReversal(char *rrn, char *step)
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
	//mti|datetime|track2|procode|amt|rrn|panseqno|entrymode|posdatacode
	//|stan|authcode|condcode|respcode|
	
	ShowLogs(1, "ORIG MTI: %s", omti);
	ShowLogs(1, "ORIG DATETIME: %s", odatetime);
	ShowLogs(1, "ORIG CARD NUM: %s", ocardno);
	ShowLogs(1, "ORIG PROC: %s", oproc);
	ShowLogs(1, "ORIG AMT: %s", oamt);
	ShowLogs(1, "ORIG RRN: %s", orrn);
	ShowLogs(1, "ORIG SEQ NUM: %s", oseqnum);
	ShowLogs(1, "ORIG ENTRY MODE: %s", oentrymode);
	ShowLogs(1, "ORIG DATA CODE: %s", oposdatacode);

	if(strncmp(glSendPack.szRRN, orrn, 12) == 0)
	{
		parseReversalAmount(oamt, fin);
		Gui_ClearScr();
		iRet = Gui_ShowMsgBox("REVERSAL", gl_stTitleAttr, _T(fin), gl_stCenterAttr, GUI_BUTTON_YandN, 60, NULL);
		if(iRet != GUI_OK)
			return 2;

		strcpy(glSendPack.szAqurId, acq);
		strcpy(glSendPack.szLocalDateTime, odatetime);
		strncpy(glSendPack.szLocalTime, odatetime + 4, 6);
		strncpy(glSendPack.szLocalDate, odatetime, 4);
		strcpy(glSendPack.szPanSeqNo, oseqnum);
		strcpy(glSendPack.szPosDataCode, oposdatacode);
		strcpy(glSendPack.szTrack2, ocardno);
		getPanReversal(ocardno, pan, expdate, src);
		strcpy(glSendPack.szMsgCode, "0420");
		strcpy(glSendPack.szPan, pan);
		strcpy(glSendPack.szProcCode, oproc);
		strcpy(glSendPack.szTranAmt, oamt);
		memset(glProcInfo.stTranLog.szAmount, 0, sizeof(glProcInfo.stTranLog.szAmount));
		strcpy(glProcInfo.stTranLog.szAmount, oamt);
		strcpy(glSendPack.szExpDate, expdate);
		strcpy(glSendPack.szPanSeqNo, oseqnum);
		strcpy(glSendPack.szCondCode, "00");
		strcpy(glSendPack.szPoscCode, "06");
		strcpy(glSendPack.szTransFee, "C00000000");
		strcpy(glSendPack.szTrack2, ocardno);
		strcpy(glSendPack.szServResCode, src);
		strcpy(glSendPack.szSTAN, ostan);
		sprintf((char *)glSendPack.szRRN, "000000%s", ostan);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("tid", temp);
		sprintf((char *)glSendPack.szTermID, "%.*s", LEN_TERM_ID, temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("txnMid", temp);
		sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("curcode", temp);
		sprintf((char *)glSendPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("txnMNL", temp);
		sprintf((char *)glSendPack.szMNL, "%.*s", LEN_MNL, temp);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("txnMCC", temp);
		sprintf((char *)glSendPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, temp);
		strcpy(glSendPack.szReasonCode, "4000");
		strcpy(glSendPack.szEntryMode, "0051");
		strcpy(glSendPack.szOrigDataElement, omti);
		strcat(glSendPack.szOrigDataElement, ostan);
		strcat(glSendPack.szOrigDataElement, odatetime);
		strcat(glSendPack.szOrigDataElement, acq);
		strcat(glSendPack.szOrigDataElement, "0000000000000000");
		strcpy(glSendPack.szReplAmount, "000000000000000000000000C00000000C00000000");
		strcat(step, "\n");
		return 1;
	}
	DisplayInfoNone("", "Transaction not Seen", 3);
	return 0;
}

int parseEodRefund(char *rrn, char *step)
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
	//mti|datetime|track2|procode|amt|rrn|panseqno|entrymode|posdatacode
	//|stan|authcode|condcode|respcode|
	
	ShowLogs(1, "ORIG MTI: %s", omti);
	ShowLogs(1, "ORIG DATETIME: %s", odatetime);
	ShowLogs(1, "ORIG CARD NUM: %s", ocardno);
	ShowLogs(1, "ORIG PROC: %s", oproc);
	ShowLogs(1, "ORIG AMT: %s", oamt);
	ShowLogs(1, "ORIG RRN: %s", orrn);
	ShowLogs(1, "ORIG SEQ NUM: %s", oseqnum);
	ShowLogs(1, "ORIG ENTRY MODE: %s", oentrymode);
	ShowLogs(1, "ORIG DATA CODE: %s", oposdatacode);

	if(strncmp(glSendPack.szRRN, orrn, 12) == 0)
	{
		parseReversalAmount(oamt, fin);
		/*Gui_ClearScr();
		iRet = Gui_ShowMsgBox("REFUND", gl_stTitleAttr, _T(fin), gl_stCenterAttr, GUI_BUTTON_YandN, 60, NULL);
		if(iRet != GUI_OK)
			return 2;*/
		return 1;
	}
	DisplayInfoNone("", "Transaction not Seen", 3);
	return 0;
}

int RefundEntrance()
{
	int iRet = 0, iRev = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key;
	int chk = 0;
	char rrn[13] = {0};
	char txn[1000 * 1024] = {0};
	char res[1000 * 1024] = {0};
	char step[1 * 1024] = {0};
	char lasthost[128] = {0};
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

	iRet = ReadAllData("eod.txt", txn);
	if(strlen(txn) < 10)
	{
		DisplayInfoNone("", "Empty", 2);
		return 0;
	}
	
	DisplayMsg("", "RRN", "0", rrn, 12, 12);
	ShowLogs(1, "Rrn: %s", rrn);
	if(strlen(rrn) < 12)
	{
		DisplayInfoNone("", "Wrong Rrn", 2);
		return 0;
	}


	txnType = 4;
	memset(&glSendPack, 0, sizeof(STISO8583));
	strcpy(glSendPack.szRRN, rrn);
	memset(step, '\0', strlen(step));
	iRet = parseEodRefund(rrn, step);
	if(iRet == 1)
	{
		refundTxn();
	}else if(iRet == 2)
	{
		Beep();
		DisplayInfoNone("", "Canceled", 2);
	}
	return 0;
}

int reversalTxn()
{
	int iRet = 0, iRev = 0;
	ST_EVENT_MSG stEventMsg;
	uchar key;
	int chk = 0;
	char rrn[13] = {0};
	char txn[1000 * 1024] = {0};
	char res[1000 * 1024] = {0};
	char step[1 * 1024] = {0};
	char lasthost[128] = {0};
	char chrev[1024] = {0};
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;

	iRet = ReadAllData("eod.txt", txn);
	if(strlen(txn) < 10)
	{
		DisplayInfoNone("", "Empty", 2);
		return 0;
	}
	
	DisplayMsg("", "RRN", "0", rrn, 12, 12);
	ShowLogs(1, "RRN: %s", rrn);
	if(strlen(rrn) < 12)
	{
		DisplayInfoNone("", "Wrong Rrn", 2);
		return 0;
	}


	txnType = 7;
	memset(&glSendPack, 0, sizeof(STISO8583));
	strcpy(glSendPack.szRRN, rrn);
	memset(step, '\0', strlen(step));

	//char chrev[1024] = {0};
	memset(chrev, '\0', strlen(chrev));
	iRet = ReadAllData("streversal.txt", chrev);
	if(strlen(chrev) < 12)
	{
		ShowLogs(1, "NO APPROVED REVERSAL TO CHECK");
	}else
	{
		ShowLogs(1, "LIST APPROVED REVERSAL TO CHECK: %s", chrev);
		ShowLogs(1, "RRN TO BE CHECKED: %s", glSendPack.szRRN);
		if(strstr(chrev, glSendPack.szRRN) != NULL)
		{
			DisplayInfoNone("", "RRN ALREADY REVERSED", 2);
			return 0;
		}
	}

	iRet = parseEodReversal(rrn, step);
	if(iRet == 1)
	{
		reversalMimic();
		if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE) == 0 )
		{
			memset(txn, '\0', strlen(txn));
			iRet = ReadAllData("eod.txt", txn);
			replaceWord(res, txn, step, "");
			iRet = CreateWrite("eod.txt", res);
		}
		ShowLogs(1, "Response came from host");
		PrintAllReceipt(PRN_NORMAL);
	}else if(iRet == 2)
	{
		Beep();
		DisplayInfoNone("", "Canceled", 2);
	}
	return 0;
}