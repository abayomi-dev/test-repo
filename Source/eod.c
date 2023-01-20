#include "global.h"

int compl = 0;
//Format
//mti|datetime|track2|procode|amt|rrn|panseqno|entrymode|posdatacode|stan|authcode|condcode|respcode|AcquiringCode|
void formatTrack2(char *orig, char *format)
{
	int i = 0, j = 0;
	for(i = 0, j = 0; i < strlen(orig); i++, j++)
	{
		if(orig[i] == '=')
			format[j] = 'D';
		else
			format[j] = orig[i];
	}
}

void formatTrackB(char *orig, char *format)
{
	int i = 0, j = 0;
	for(i = 0, j = 0; i < strlen(orig); i++, j++)
	{
		if(orig[i] == '=' || orig[i] == 'D' || orig[i] == 'F')
			return;
		else
			format[j] = orig[i];
	}
}

void packNewEod(char *cTxn)
{
	char format[40] = {0};
	memset(format, '\0', strlen(format));
	strcpy(cTxn, glSendPack.szMsgCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szLocalDateTime);
	strcat(cTxn, "|");
	formatTrack2(glSendPack.szTrack2, format);
	strcat(cTxn, format);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szProcCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szTranAmt);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szRRN);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szPanSeqNo);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szEntryMode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szPosDataCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szSTAN);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szAuthCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szCondCode);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szRspCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szAqurId);
	strcat(cTxn, "|");
	strcat(cTxn, "\n");
}	

void packOldEod(char *cTxn)
{
	char format[40] = {0};
	memset(format, '\0', strlen(format));
	strcpy(cTxn, glSendPack.szMsgCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szLocalDateTime);
	strcat(cTxn, "|");
	formatTrack2(glSendPack.szTrack2, format);
	strcat(cTxn, format);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szProcCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szTranAmt);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szRRN);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szPanSeqNo);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szEntryMode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szPosDataCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szSTAN);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szAuthCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szCondCode);
	strcat(cTxn, "|");
	strcat(cTxn, glRecvPack.szRspCode);
	strcat(cTxn, "|");
	strcat(cTxn, glSendPack.szAqurId);
	strcat(cTxn, "|");
	strcat(cTxn, "\n");
}

void storeEodDuplicate()
{
	int iRet, iLen;
	char txn[100 * 1024] = {0};
	iRet = ReadAllData("eod.txt", txn);
	iRet = CreateWrite("eod2.txt", txn);
}

void storeEod()
{
	int iRet, iLen;
	char txn[512] = {0};
	memset(txn, '\0', strlen(txn));

	ShowLogs(1, "1. Filesize of eod.txt: %lu", filesize("eod.txt"));
	if(filesize("eod.txt") < 10)
	{
		packNewEod(txn);
		iRet = CreateWrite("eod.txt", txn);
	}else
	{
		packOldEod(txn);
		iRet = WriteUpdate("eod.txt", txn, filesize("eod.txt")); 
	}
	if(filesize("cashier.txt") < 10)
	{
		iRet = CreateWrite("cashier.txt", txn);
	}else
	{
		iRet = WriteUpdate("cashier.txt", txn, filesize("cashier.txt")); 
	}
	ShowLogs(1, "2. Filesize of eod.txt: %lu", filesize("eod.txt"));
	ShowLogs(1, "2. Filesize of eod.txt: %lu", filesize("cashier.txt"));
}

int parseInner(char *line, char *rrn, char *auth, char *omti, char *odatetime, 
	char *ocardno, char *oproc, char *oamt, char *oauth, char *orrn, char *ostan)
{
	int i, loop, j;
	char temp[50] = {0};
	loop = 0;
	j = 0;
	if((strstr(line, rrn) != NULL) && (strstr(line, auth) != NULL))
	{
		//ShowLogs(1, "2. The Line: %s", line);
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
					//odatetime[j] = line[i];
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
					//odatetime[j] = line[i];
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
					//odatetime[j] = line[i];
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
				}else
				{
					oauth[j] = line[i];
					j++;
				}
			}else if(loop == 11)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					//odatetime[j] = line[i];
					j++;
				}
			}else if(loop == 12)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					//odatetime[j] = line[i];
					j++;
				}
			}else if(loop == 13)
			{
				if(line[i] == '|')
				{
					j = 0;
					loop++;
				}else
				{
					//odatetime[j] = line[i];
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

int parseEod(char *rrn, char *authcode)
{
	int iRet, i, j;
	//char txn[100 * 1024] = {0};
	char *txn = (char*)malloc((sizeof(char) * filesize("eod.txt")) + 1);
	char step[1 * 1024] = {0};
	char omti[20] = {0};
	char odatetime[11] = {0};
	char ocardno[50] = {0};
	char oproc[20] = {0};
	char oamt[20] = {0};
	char oauth[20] = {0};
	char orrn[13] = {0};
	char ostan[20] = {0};
	char format[40] = {0};
	char format2[40] = {0};

	iRet = ReadAllData("eod.txt", txn);
	//ShowLogs(1, "2. EOD: %s", txn);
	j = 0;
	for(i = 0; i < strlen(txn); i++)
	{
		if(txn[i] == '\n')
		{
			if(parseInner(step, rrn, authcode, omti, odatetime, ocardno, oproc, oamt, oauth, orrn, ostan))
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
	ShowLogs(1, "ORIG CARD NUM: %s", ocardno);
	ShowLogs(1, "ORIG PROC: %s", oproc);
	ShowLogs(1, "ORIG AMT: %s", oamt);
	ShowLogs(1, "ORIG AUTH: %s", oauth);
	ShowLogs(1, "AUTH: %s", glSendPack.szAuthCode);
	ShowLogs(1, "ORIG RRN: %s", orrn);
	ShowLogs(1, "RRN: %s", glSendPack.szRRN);
	ShowLogs(1, "ORIG STAN: %s", ostan);

	if(strncmp(glSendPack.szRRN, orrn, 12) == 0)
	{
		if(strncmp(glSendPack.szAuthCode, oauth, 6) == 0)
		{
			memset(format, '\0', strlen(format));
			formatTrackB(glSendPack.szTrack2, format);
			memset(format2, '\0', strlen(format2));
			formatTrackB(ocardno, format2);
			ShowLogs(1, "Current Pan: %s", format);
			ShowLogs(1, "Previous Pan: %s", format2);
			//formatTrack2(glSendPack.szTrack2, format);
			if(strncmp(format, format2, strlen(format2)) == 0)
			{
				strcpy(glSendPack.szSTAN, ostan);
				strcpy(glSendPack.szOrigDataElement, omti);
				strcat(glSendPack.szOrigDataElement, ostan);
				strcat(glSendPack.szOrigDataElement, odatetime);
				strcat(glSendPack.szOrigDataElement, "0000011112900000111129");
				strcpy(glSendPack.szReplAmount, oamt);
				strcat(glSendPack.szReplAmount, "000000000000");
				strcat(glSendPack.szReplAmount, "C00000000");
				strcat(glSendPack.szReplAmount, "C00000000");
				DisplayInfoNone("", "SEEN", 1);
				compl = 0;
				free(txn);
				return 1;
			}
		}else
			ShowLogs(1, "Authcode not Marches");
	}else
		ShowLogs(1, "RRN not Marches");

	DisplayInfoNone("", "NOT SEEN", 3);
	compl = 1;
	free(txn);
	return 0;
}
