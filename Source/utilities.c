#include "global.h"

int downloadBillers = 0;
ulong useStan = 1L;
ulong RctNum = 1L;
int txnCount = 0;

int llen = 0; 
int clen = 0;


int getHeight(int x)
{
    llen = x;
    clen = 1;

    int y = 100 * x;
    int s = y/sg_nScrHeight;
    int p = s + 16;
    return p;
}


void removeSubstr (char *string, char *sub) {
    char *match;
    int len = strlen(sub);
    while ((match = strstr(string, sub))) {
        *match = '\0';
        strcat(string, match+len);
    }
}

void parsetrackVerve(char *track, char *parse)
{
	int i = 0, j = 0;
	for(i = 0; i < strlen(track); i++)
	{
		if(track[i] == '=')
		{
			parse[j] = 'D';
			j++;
		}else
		{
			parse[j] = track[i];
			j++;
		}
	}
}

int setVerveHost(char *host)
{
	int m;
	char temp[128] = {0};
	char lasthost[128] = {0};
	
	if(strstr(host, "host2") != NULL)
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
		UtilPutEnv("proCtmk", temp);
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
		////ShowLogs(1, "Setting CTMK TO HOST 1: %s", temp);
		UtilPutEnv("proCtmk", temp);
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

void getResponse(char *code, char *output)
{
	if(strstr(code, "00") != NULL)
	{
		strcpy(output, "Approved..");
	}else if(strstr(code, "01") != NULL)
	{
		strcpy(output, "                         Declined \n (Refer to card issuer, special condition)");
	}else if(strstr(code, "02") != NULL)
	{
		strcpy(output, "                         Declined \n (Refer to card issuer)");
	}else if(strstr(code, "03") != NULL)
	{
		strcpy(output, "                         Declined \n (Invalid merchant)");
	}else if(strstr(code, "04") != NULL)
	{
		strcpy(output, "                         Declined \n (Pick-up card)");
	}else if(strstr(code, "05") != NULL)
	{
		strcpy(output, "                         Declined \n (Do not honor)");
	}else if(strstr(code, "06") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "07") != NULL)
	{
		strcpy(output, "                         Declined \n (Pick-up card, special condition)");
	}else if(strstr(code, "08") != NULL)
	{
		strcpy(output, "                         Declined \n (Honor with identification)");
	}else if(strstr(code, "09") != NULL)
	{
		strcpy(output, "                         Declined \n (Request in progress)");
	}else if(strstr(code, "10") != NULL)
	{
		strcpy(output, "                         Declined \n (Approved, partial)");
	}else if(strstr(code, "11") != NULL)
	{
		strcpy(output, "                         Declined \n (Approved, VIP)");
	}else if(strstr(code, "12") != NULL)
	{
		strcpy(output, "                         Declined \n (Invalid transaction)");
	}else if(strstr(code, "13") != NULL)
	{
		strcpy(output, "                         Declined \n (Invalid amount)");
	}else if(strstr(code, "14") != NULL)
	{
		strcpy(output, "                         Declined \n (Invalid card number)");
	}else if(strstr(code, "15") != NULL)
	{
		strcpy(output, "                         Declined \n (No such issuer)");
	}else if(strstr(code, "16") != NULL)
	{
		strcpy(output, "                         Declined \n (Approved, update track 3)");
	}else if(strstr(code, "17") != NULL)
	{
		strcpy(output, "                         Declined \n (Customer cancellation)");
	}else if(strstr(code, "18") != NULL)
	{
		strcpy(output, "                         Declined \n (Customer dispute)");
	}else if(strstr(code, "19") != NULL)
	{
		strcpy(output, "                         Declined \n (Re-enter transaction)");
	}else if(strstr(code, "20") != NULL)
	{
		strcpy(output, "                         Declined \n (Invalid response)");
	}else if(strstr(code, "21") != NULL)
	{
		strcpy(output, "                         Declined \n (No action taken)");
	}else if(strstr(code, "22") != NULL)
	{
		strcpy(output, "                         Declined \n (Suspected malfunction)");
	}else if(strstr(code, "23") != NULL)
	{
		strcpy(output, "                         Declined \n (Unacceptable transaction fee)");
	}else if(strstr(code, "24") != NULL)
	{
		strcpy(output, "                         Declined \n (File update not supported)");
	}else if(strstr(code, "25") != NULL)
	{
		strcpy(output, "                         Declined \n (Unable to locate record");
		//strcpy(output, "                         Declined \n (Transaction Reversed)");
	}else if(strstr(code, "26") != NULL)
	{
		strcpy(output, "                         Declined \n (Duplicate record)");
	}else if(strstr(code, "27") != NULL)
	{
		strcpy(output, "                         Declined \n (File update field edit error)");
	}else if(strstr(code, "28") != NULL)
	{
		strcpy(output, "                         Declined \n (File update file locked)");
	}else if(strstr(code, "29") != NULL)
	{
		strcpy(output, "                         Declined \n (File update failed)");
	}else if(strstr(code, "30") != NULL)
	{
		strcpy(output, "                         Declined \n (Format error)");
	}else if(strstr(code, "31") != NULL)
	{
		strcpy(output, "                         Declined \n (Bank not supported)");
	}else if(strstr(code, "32") != NULL)
	{
		strcpy(output, "                         Declined \n (Completed partially)");
	}else if(strstr(code, "33") != NULL)
	{
		strcpy(output, "                         Declined \n (Expired card, pick-up)");
	}else if(strstr(code, "34") != NULL)
	{
		strcpy(output, "                         Declined \n (Suspected fraud, pick-up)");
	}else if(strstr(code, "35") != NULL)
	{
		strcpy(output, "                         Declined \n (Contact acquirer, pick-up)");
	}else if(strstr(code, "36") != NULL)
	{
		strcpy(output, "                         Declined \n (Restricted card, pick-up)");
	}else if(strstr(code, "37") != NULL)
	{
		strcpy(output, "                         Declined \n (Call acquirer security, pick-up)");
	}else if(strstr(code, "38") != NULL)
	{
		strcpy(output, "                         Declined \n (PIN tries exceeded, pick-up)");
	}else if(strstr(code, "39") != NULL)
	{
		strcpy(output, "                         Declined \n (No credit account)");
	}else if(strstr(code, "40") != NULL)
	{
		strcpy(output, "                         Declined \n (Function not supported)");
	}else if(strstr(code, "41") != NULL)
	{
		strcpy(output, "                         Declined \n (Lost card, pick-up)");
	}else if(strstr(code, "42") != NULL)
	{
		strcpy(output, "                         Declined \n (No universal account)");
	}else if(strstr(code, "43") != NULL)
	{
		strcpy(output, "                         Declined \n (Stolen card, pick-up)");
	}else if(strstr(code, "44") != NULL)
	{
		strcpy(output, "                         Declined \n (No investment account)");
	}else if(strstr(code, "45") != NULL)
	{
		strcpy(output, "                         Declined \n (Account closed)");
	}else if(strstr(code, "46") != NULL)
	{
		strcpy(output, "                         Declined \n (Identification required)");
	}else if(strstr(code, "47") != NULL)
	{
		strcpy(output, "                         Declined \n (Identification cross-check required)");
	}else if(strstr(code, "48") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "49") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "50") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "51") != NULL)
	{
		strcpy(output, "                         Declined \n (Insufficient funds)");
	}else if(strstr(code, "52") != NULL)
	{
		strcpy(output, "                         Declined \n (No check account)");
	}else if(strstr(code, "53") != NULL)
	{
		strcpy(output, "                         Declined \n (No savings account)");
	}else if(strstr(code, "54") != NULL)
	{
		strcpy(output, "                         Declined \n (Expired card)");
	}else if(strstr(code, "55") != NULL)
	{
		strcpy(output, "                         Declined \n (Incorrect PIN)");
	}else if(strstr(code, "56") != NULL)
	{
		strcpy(output, "                         Declined \n (No card record)");
	}else if(strstr(code, "57") != NULL)
	{
		strcpy(output, "                         Declined \n (Transaction not permitted to cardholder)");
	}else if(strstr(code, "58") != NULL)
	{
		strcpy(output, "                         Declined \n (Transaction not permitted on terminal)");
	}else if(strstr(code, "59") != NULL)
	{
		strcpy(output, "                         Declined \n (Suspected fraud)");
	}else if(strstr(code, "60") != NULL)
	{
		strcpy(output, "                         Declined \n (Contact acquirer)");
	}else if(strstr(code, "61") != NULL)
	{
		strcpy(output, "                         Declined \n (Exceeds withdrawal limit)");
	}else if(strstr(code, "62") != NULL)
	{
		strcpy(output, "                         Declined \n (Restricted card)");
	}else if(strstr(code, "63") != NULL)
	{
		strcpy(output, "                         Declined \n (Security violation)");
	}else if(strstr(code, "64") != NULL)
	{
		strcpy(output, "                         Declined \n (Original amount incorrect)");
	}else if(strstr(code, "65") != NULL)
	{
		strcpy(output, "                         Declined \n (Exceeds withdrawal frequency)");
	}else if(strstr(code, "66") != NULL)
	{
		strcpy(output, "                         Declined \n (Call acquirer security)");
	}else if(strstr(code, "67") != NULL)
	{
		strcpy(output, "                         Declined \n (Hard capture)");
	}else if(strstr(code, "68") != NULL)
	{
		strcpy(output, "                         Declined \n (Response received too late)");
	}else if(strstr(code, "69") != NULL)
	{
		strcpy(output, "                         Declined \n (Advice received too late)");
	}else if(strstr(code, "70") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "71") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "72") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "73") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "74") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "75") != NULL)
	{
		strcpy(output, "                         Declined \n (PIN tries exceeded)");
	}else if(strstr(code, "76") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "77") != NULL)
	{
		strcpy(output, "                         Declined \n (Intervene, bank approval required)");
	}else if(strstr(code, "78") != NULL)
	{
		strcpy(output, "                         Declined \n (Intervene, bank approval required for partial amount)");
	}else if(strstr(code, "79") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "80") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "81") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "82") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "83") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "84") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "85") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "86") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else if(strstr(code, "87") != NULL)
	{
		strcpy(output, "                         Declined \n (No Response)");
	}else if(strstr(code, "88") != NULL)
	{
		strcpy(output, "                         Declined \n (Transaction Timeout)");
	}else if(strstr(code, "89") != NULL)
	{
		strcpy(output, "                         Declined \n (Not Processed)");
	}else if(strstr(code, "90") != NULL)
	{
		strcpy(output, "                         Declined \n (Cut-off in progress)");
	}else if(strstr(code, "91") != NULL)
	{
		strcpy(output, "                         Declined \n (Issuer or switch inoperative)");
	}else if(strstr(code, "92") != NULL)
	{
		strcpy(output, "                         Declined \n (Routing error)");
	}else if(strstr(code, "93") != NULL)
	{
		strcpy(output, "                         Declined \n (Violation of law)");
	}else if(strstr(code, "94") != NULL)
	{
		strcpy(output, "                         Declined \n (Duplicate transaction)");
	}else if(strstr(code, "95") != NULL)
	{
		strcpy(output, "                         Declined \n (Reconcile error)");
	}else if(strstr(code, "96") != NULL)
	{
		strcpy(output, "                         Declined \n (System malfunction)");
	}else if(strstr(code, "97") != NULL)
	{
		strcpy(output, "                         Declined \n (Reserved for future Postilion use)");
	}else if(strstr(code, "98") != NULL)
	{
		strcpy(output, "                         Declined \n (Exceeds cash limit)");
	}else if(strstr(code, "99") != NULL)
	{
		strcpy(output, "                         Declined \n (Error)");
	}else
	{
		strcpy(output, "                         Declined \n (Not Processed)");
	}
}

void GetTxnCount(void)
{
	char st[7] = {0};
	ReadAllData("count.txt", st);
	txnCount = atol(st);
	txnCount++;
	memset(st, '\0', strlen(st));
	sprintf((char *)st, "%d", txnCount);
	CreateWrite("count.txt", st);
}


void GetReceiptNumber(void)
{
	char st[7] = {0};
	ReadAllData("receipt.txt", st);
	RctNum = atoll(st);
	RctNum++;
	memset(st, '\0', strlen(st));
	sprintf((char *)st, "%lu", RctNum);
	CreateWrite("receipt.txt", st);
}

void GetStan(void)
{
	char st[7] = {0};
	ShowLogs(1, "GetStan 1");
	ReadAllData("stan.txt", st);
	ShowLogs(1, "GetStan 2");
	useStan = atoll(st);
	ShowLogs(1, "GetStan 3");
	useStan++;
	memset(st, '\0', strlen(st));
	ShowLogs(1, "GetStan 4");
	sprintf((char *)st, "%lu", useStan);
	ShowLogs(1, "GetStan 5");
	CreateWrite("stan.txt", st);
	ShowLogs(1, "GetStan 6");
}

void EmvSetSSLFlag()
{
	UtilPutEnv("E_SSL", "1");
	UtilPutEnv("CA_CRT", "CACRT.PEM");
	UtilPutEnv("CLI_CRT", "CLICRT.PEM");
	UtilPutEnv("CLI_KEY", "CLIKEY.PEM");
}

void EmvUnsetSSLFlag()
{
	PutEnv("E_SSL", "0");
	PutEnv("CA_CRT", "");
	PutEnv("CLI_CRT", "");
	PutEnv("CLI_KEY", "");
}


int checkOccurrance(char *str, char *text)
{
	int len;
	int iRet = 0;
	int init = 0;

	len = strlen(str);
	for(init = 0; init < len; init++)
	{
		if(str[init] == text[0]
			&& str[init + 1] == text[1]
			&& str[init + 2] == text[2]
			&& str[init + 3] == text[3])
		{
			iRet = init + 4;
			break;
		}
	}
	return iRet;
}

void GetSignalStr() 
{
	uchar SingnalLevelOut;
	int iRet = 0;
	iRet = WlGetSignal(&SingnalLevelOut);
	ShowLogs(1, "WlGetSignal Return: %d", iRet);
	if (0 != iRet) {
		return;
	}

	switch (SingnalLevelOut) {
		case 0x00:
		ScrSetIcon (ICON_SIGNAL, 6);
		break;

		case 0x01:
		ScrSetIcon (ICON_SIGNAL, 5);
		break;

		case 0x02:
		ScrSetIcon (ICON_SIGNAL, 4);
		break;

		case 0x03:
		ScrSetIcon (ICON_SIGNAL, 3);
		break;

		case 0x04:
		ScrSetIcon (ICON_SIGNAL, 2);
		break;

		case 0x05:
		ScrSetIcon (ICON_SIGNAL, 1);
		break;
		
		default:
		break;
	}
}

void ParseGetData(char *in, char *out, char *str)
{
	int iRet, iFinal;
	iRet = checkOccurrance(in, str);
	//iFinal = strlen(in) - iRet;
	//strncpy(out, in + iRet, iFinal);
	strcpy(out, in + iRet);
}

int WriteUpdate(char *filename, char *data, long lOffset)
{
	int iRet;
	iRet = PubFileWrite(filename,
						lOffset,
						data,
						strlen(data));
	if( iRet!=0 )
	{
		ShowLogs(1, "Error updating file %s :%d", filename, iRet);
		ShowLogs(1, "%s Update Unsuccessful", filename);
		return 1;
	}
	ShowLogs(1, "%s Update Successful", filename);
	return 0;
}

int CreateWrite(char *filename, char *data)
{
	int iRet;
	//ShowLogs(1, "Inside CreateWrite 1");
	remove(filename);
	//ShowLogs(1, "Inside CreateWrite 2");
	iRet = PubFileWrite(filename,
						0L,
						data,
						strlen(data));
	//ShowLogs(1, "Inside CreateWrite 3");
	if( iRet!=0 )
	{
		ShowLogs(1, "Error creating and Saving in file %s :%d", filename, iRet);
		return 1;
	}
	//ShowLogs(1, "Inside CreateWrite 4");
	return 0;
}

int ReadAllData(char *filename, char *output)
{
	int		iRet;

	iRet = PubFileRead(filename, 0L, output, filesize(filename));
	
	if( iRet!=0 )
	{
		ShowLogs(1, "Error reading file %s :%d", filename, iRet);
		return 1;
	}
	return 0;
}


int StartGprs(char *apn, char *username, char *password) {

	int iRet = 0;
	
	ShowLogs(1, "Starting GPRS");
	ScrSetIcon(ICON_PHONE, OPENICON);

	ShowLogs(1, "Comms types is: %d", glSysParam.stTxnCommCfg.ucCommType);

	WlSwitchPower(1);
	
	iRet = WlInit((unsigned char*) "");
	if (0 != iRet && -212 != iRet) {
		ShowLogs(1, "WlInit() returned %i", iRet);
		ScrSetIcon(ICON_PHONE, CLOSEICON);
		return 1;
	}

	ShowLogs(1, "Entering loop for starting gprs");

	while (1) {
		iRet = WlPppCheck();
		if (iRet == 0x00) {
			RouteSetDefault(11);
			break;
		} else if (WL_RET_ERR_DIALING == iRet) {
			DelayMs(500);
			continue;
		} else {
			ShowLogs(1, "try wiplogin with gprs %s, %s and %s", apn, username, password);
			iRet = WlPppLogin((uchar *) apn,
					(uchar *) username,
					(uchar *) password, PPP_ALG_ALL, 120000, 10);
			ShowLogs(1, "WlPppLogin() returned %d", iRet);
			if (0 == iRet) {
				ScrSetIcon(ICON_PHONE, CLOSEICON);
				iRet = RouteSetDefault(11);
				ShowLogs(1, "gprs startup successful");
				return 0;
			} else if (WL_RET_ERR_DIALING == iRet) {
				ShowLogs(1, "Error dialing");
				DelayMs(500);
				continue;
			} else {
				ScrSetIcon(ICON_PHONE, CLOSEICON);
				ShowLogs(1, "gprs startup failed");
				return 1;
			}
		}
	}
	return 1;

}

void DisplayInfoCancel(char* heading, char *msg, int timeout)
{
	char head[32] = {0};
	memset(head, '\0', strlen(head));
	if(!UtilGetEnvEx("dialogheading", head))
	{
		sprintf(head, "ARCAPAY");
	}
	PubStrUpper(msg);
	Gui_ClearScr();
	Gui_ShowMsgBox(head, gl_stTitleAttr, _T(msg), gl_stCenterAttr, GUI_BUTTON_CANCEL, timeout, NULL);
}

void DisplayInfoOk(char* heading, char *msg, int timeout)
{
	char head[32] = {0};
	memset(head, '\0', strlen(head));
	if(!UtilGetEnvEx("dialogheading", head))
	{
		sprintf(head, "ARCAPAY");
	}
	PubStrUpper(msg);
	Gui_ClearScr();
	Gui_ShowMsgBox(head, gl_stTitleAttr, _T(msg), gl_stCenterAttr, GUI_BUTTON_OK, timeout, NULL);
}

int DisplayInfoYN(char* heading, char *msg, int timeout)
{
	int iRet;
	char head[32] = {0};
	memset(head, '\0', strlen(head));
	if(!UtilGetEnvEx("dialogheading", head))
	{
		sprintf(head, "ARCAPAY");
	}
	PubStrUpper(msg);
	Gui_ClearScr();
	iRet = Gui_ShowMsgBox(head, gl_stTitleAttr, _T(msg), gl_stCenterAttr, GUI_BUTTON_YandN, timeout, NULL);
	return iRet;
}

void DisplayInfoNone(char* heading, char *msg, int timeout)
{
	char head[32] = {0};
	memset(head, '\0', strlen(head));

	if(strlen(heading) || heading != NULL)
	{
		strcpy(head, heading);
	}else
	{
		if(!UtilGetEnvEx("dialogheading", head))
		{
			strcpy(head, "ARCAPAY");
		}
	}
	PubStrUpper(msg);
	Gui_ClearScr();
	Gui_ShowMsgBox(head, gl_stTitleAttr, _T(msg), gl_stCenterAttr, GUI_BUTTON_NONE, timeout, NULL);
}

int DisplayMsg(char* heading, char * demand, char * def, char *input, int min, int max)
{
	int iRet;
	uchar	szBuff[30];
	char head[32] = {0};
	GUI_INPUTBOX_ATTR stInputAttr;
	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = min;
	stInputAttr.nMaxLen = max;
	memset(head, '\0', strlen(head));

	if(strlen(heading) || heading != NULL)
	{
		strcpy(head, heading);
	}else
	{
		if(!UtilGetEnvEx("dialogheading", head))
		{
			strcpy(head, "ARCAPAY");
		}
	}
	while (1)
	{
		sprintf((char *)szBuff, "%.25s", def);
		Gui_ClearScr();
		iRet = Gui_ShowInputBox(head, gl_stTitleAttr, _T(demand), gl_stLeftAttr, 
			szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if( iRet == GUI_OK)
		{
			break;
		}else if( iRet == GUI_ERR_USERCANCELLED)
		{
			break;
		}
	}
	ShowLogs(1, "Inside Display: %s", szBuff);
	strcpy(input, szBuff);
	return iRet;
}

void TextLeftSmall(char *text, int x, int y)
{
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	stTextAttr.eAlign = GUI_ALIGN_LEFT;
	stTextAttr.eStyle = GUI_FONT_STD;
	Gui_DrawText(text, stTextAttr, x, y);
}

void TextCenter(char *text, int x, int y)
{
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_NORMAL;
	stTextAttr.eAlign = GUI_ALIGN_CENTER;
	stTextAttr.eStyle = GUI_FONT_BOLD;
	Gui_DrawText(text, stTextAttr, x, y);
}

void TextLeftNormal(char *text, int x, int y)
{
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_NORMAL;
	stTextAttr.eAlign = GUI_ALIGN_LEFT;
	stTextAttr.eStyle = GUI_FONT_STD;
	Gui_DrawText(text, stTextAttr, x, y);
}

int UtilGetEnvEx(char *key, char *value)
{
	int len = 0;
	if(0 == GetEnv(key, value))
	{
	    //ShowLogs(1, "Ex Value for %s fetched: %s", key, value);
	    len = strlen(value);
	    return 1;
	}else
	{
		ShowLogs(1, "Couldnt find value for %s", key);
		return 0;
	}
}

int UtilGetEnv(char *key, char *value)
{
	int len = 0;
	if(0 == GetEnv(key, value))
	{
	    //ShowLogs(1, "Ex Value for %s fetched: %s", key, value);
	    len = strlen(value);
	    return 1;
	}else
	{
		ShowLogs(1, "Couldnt find value for %s", key);
		return 0;
	}
}


/*int UtilGetEnv(char *key, char *value)
{
	int len = 0;
	if(0 == GetEnv(key, value))
	{
	    //ShowLogs(1, "Value for %s fetched: %s", key, value);
	    len = strlen(value);
	    value[len - 1] = '\0';
	    return 1;
	}else
	{
		ShowLogs(1, "Couldnt find value for %s", key);
		return 0;
	}
}*/


void UtilPutEnv(char *key, char *value)
{
	int iRet = 0;

	iRet = PutEnv(key, value);

	if(iRet == 0)
	{
		//ShowLogs(1, "Key: %s :: Value: %s", key, value);
	}else if(iRet == 1)
	{
		ShowLogs(1, "%s Couldnt be stored. Invalid parameter.", key);
	}else
	{
		ShowLogs(1, "%s Couldnt be stored. Not enough space.", key);
	}
}

void SysGetTimeIso(char *time)
{
	char Time[8], year[3], month[3], day[3], hour[3], min[3], sec[3];

	memset(Time, 0, sizeof(Time));
	memset(year, 0, sizeof(year));
	memset(month, 0, sizeof(month));
	memset(day, 0, sizeof(day));
	memset(hour, 0, sizeof(hour));
	memset(min, 0, sizeof(min));
	memset(sec, 0, sizeof(sec));

	GetTime(Time);
	sprintf(year, "%x", Time[0]);
	sprintf(month, "%x", Time[1]);
	sprintf(day, "%x", Time[2]);
	sprintf(hour, "%x", Time[3]);
	sprintf(min, "%x", Time[4]);
	sprintf(sec, "%x", Time[5]);

	//Year
	if(strlen(year) == 1)
	{
		strcpy(time, "0");
		strcat(time, year);
	}
	else
		strcpy(time, year);
	//Month
	if(strlen(month) == 1)
	{
		strcat(time, "0");
		strcat(time, month);
	}
	else
		strcat(time, month);
	//Day
	if(strlen(day) == 1)
	{
		strcat(time, "0");
		strcat(time, day);
	}
	else
		strcat(time, day);
	//Hour
	if(strlen(hour) == 1)
	{
		strcat(time, "0");
		strcat(time, hour);
	}
	else
		strcat(time, hour);
	//Minute
	if(strlen(min) == 1)
	{
		strcat(time, "0");
		strcat(time, min);
	}
	else
		strcat(time, min);
	//Sec
	if(strlen(sec) == 1)
	{
		strcat(time, "0");
		strcat(time, sec);
	}
	else
		strcat(time, sec);
}

void SysSetTimeProfile(char *time)
{
	uchar	szBuff[14+1], sInputTime[6];
	int i = 0, j = 0;
	memset(szBuff,0,sizeof(szBuff));
	ShowLogs(1, "Time Received: %s", time);
	for(i = 0, j = 0; i < strlen(time); i++)
	{
		if(i == 2)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 3)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 5)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 6)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 8)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 9)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 11)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 12)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 14)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 15)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 17)
		{
			szBuff[j] = time[i];
			j++;
		}
		if(i == 18)
		{
			szBuff[j] = time[i];
			j++;
		}
	}
	ShowLogs(1, "Time To be Set: %s", szBuff);
	PubAsc2Bcd(szBuff, 12, sInputTime);
	SetTime(sInputTime);
}

int mallocCount = 0;
int callocCount = 0;
int mallocFreeCount = 0;

void* sysalloc_calloc(int count, int eltsize){
	callocCount++;
	return calloc(count, eltsize);
}

void* sysalloc_malloc(int size){
	mallocCount++;
	return (void*)malloc(size);
}

void sysalloc_free(void* ptr){
	if(ptr){
		mallocFreeCount++;
	}
	free(ptr);
	ptr = NULL;
}


int acctTypeSelection()
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
	char checker[10] = {0};
	uchar k;

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	
	memset(checker, '\0', strlen(checker));
	UtilGetEnvEx("actsels", checker);
	if(strstr(checker, "false") != NULL)
	{
		UtilPutEnv("actype", "00");
		return 0;
	}
	
	numLines = 6;//Line for control

	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "DEFAULT");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "DEFAULT", strlen("DEFAULT"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "SAVINGS");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "SAVINGS", strlen("SAVINGS"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "CURRENT");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "CURRENT", strlen("CURRENT"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "CREDIT");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "CREDIT", strlen("CREDIT"));   

	for(i = 0; i < 4; ++i)
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
        if(strncmp(txnName1[iMenuNo], "DEFAULT", 7) == 0)
		{
            UtilPutEnv("actype", "00");
		}else if(strncmp(txnName1[iMenuNo], "SAVINGS", 7) == 0)
        {
            UtilPutEnv("actype", "10");
        }else if(strncmp(txnName1[iMenuNo], "CURRENT", 7) == 0)
        {   
            UtilPutEnv("actype", "20");
        }else if(strncmp(txnName1[iMenuNo], "CREDIT", 7) == 0)
        {   
            UtilPutEnv("actype", "30");
        }else
        {
        	return 1;
        }
		return 0;
	}else
	{
		return 1;
	}
	return 1;

}



int jambTypeSelection()
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
	char checker[10] = {0};
	uchar k;

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	
	memset(checker, '\0', strlen(checker));
	UtilGetEnvEx("actsels", checker);
	if(strstr(checker, "false") != NULL)
	{
		UtilPutEnv("actype", "00");
		return 0;
	}
	
	numLines = 6;//Line for control
	//"price":"4700","type":"UTME"},{"price":"4700","type":"DE"}],"type":"JAMB"
	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "UTME");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "UTME", strlen("UTME"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "DE");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "DE", strlen("DE"));

    key++;
	for(i = 0; i < 4; ++i)
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
        if(strncmp(txnName1[iMenuNo], "UTME", 7) == 0)
		{
            UtilPutEnv("jambtype", "4700");
		}else if(strncmp(txnName1[iMenuNo], "DE", 7) == 0)
        {
            UtilPutEnv("jambtype", "10");
        }else if(strncmp(txnName1[iMenuNo], "CURRENT", 7) == 0)
        {   
            UtilPutEnv("actype", "20");
        }else if(strncmp(txnName1[iMenuNo], "CREDIT", 7) == 0)
        {   
            UtilPutEnv("actype", "30");
        }else
        {
        	return 1;
        }
		return 0;
	}else
	{
		return 1;
	}
	return 1;

}


int eedcTypeSelection()
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
	char checker[10] = {0};
	uchar k;

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	
	memset(checker, '\0', strlen(checker));
	UtilGetEnvEx("actsels", checker);
	if(strstr(checker, "false") != NULL)
	{
		UtilPutEnv("actype", "00");
		return 0;
	}
	
	numLines = 6;//Line for control
	//"price":"4700","type":"UTME"},{"price":"4700","type":"DE"}],"type":"JAMB"
	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "POSTPAID");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "POSTPAID", strlen("POSTPAID"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "PREPAID");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "PREPAID", strlen("PREPAID"));

    key++;
	for(i = 0; i < 4; ++i)
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
        if(strncmp(txnName1[iMenuNo], "POSTPAID", 7) == 0)
		{
            UtilPutEnv("eedctype", "POSTPAID");
		}else if(strncmp(txnName1[iMenuNo], "PREPAID", 7) == 0)
        {
            UtilPutEnv("eedctype", "PREPAID");
        }else if(strncmp(txnName1[iMenuNo], "CURRENT", 7) == 0)
        {   
            UtilPutEnv("actype", "20");
        }else if(strncmp(txnName1[iMenuNo], "CREDIT", 7) == 0)
        {   
            UtilPutEnv("actype", "30");
        }else
        {
        	return 1;
        }
		return 0;
	}else
	{
		return 1;
	}
	return 1;

}


int eedcTypePayment()
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
	char checker[10] = {0};
	uchar k;

	GUI_MENU stTranMenu;
	GUI_MENUITEM stTranMenuItem[20];
	int iMenuItemNum = 0;
	int i;

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	
	memset(checker, '\0', strlen(checker));
	UtilGetEnvEx("actsels", checker);
	if(strstr(checker, "false") != NULL)
	{
		UtilPutEnv("paytype", "CARD");
		return 0;
	}
	
	numLines = 6;//Line for control
	//"price":"4700","type":"UTME"},{"price":"4700","type":"DE"}],"type":"JAMB"
	sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "CASH");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "CASH", strlen("CASH"));

    key++;
    sprintf((char *)stDefTranMenuItem1[key].szText, "%s", "CARD");
    stDefTranMenuItem1[key].nValue = key;
    stDefTranMenuItem1[key].bVisible = TRUE;
    strncpy(txnName1[key], "CARD", strlen("CARD"));

    key++;
	for(i = 0; i < 4; ++i)
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
        if(strncmp(txnName1[iMenuNo], "CASH", 7) == 0)
		{
            UtilPutEnv("paytype", "CASH");
		}else if(strncmp(txnName1[iMenuNo], "CARD", 7) == 0)
        {
            UtilPutEnv("paytype", "CARD");
        }else
        {
        	return 1;
        }
		return 0;
	}else
	{
		return 1;
	}
	return 1;

}


static int jsoneqqgen(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

void getJsonValue(char *input, char *name, char *value)
{
	int j, k;
	int iRet, i, r;
    int next, previous;
	jsmn_parser p;
	jsmntok_t t[300];
	char storeData[1024] = {0};

	jsmn_init(&p);
	r = jsmn_parse(&p, input, strlen(input), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) 
	{
		Beep();
		DisplayInfoNone("", "USSD ERROR 1", 3);
		ShowLogs(1, "Failed to parse JSON: %d", r);
		return;
	}

	ShowLogs(1, "menu Length Gotten: %d", strlen(input));

	if (r < 1 || t[0].type != JSMN_OBJECT) 
	{
		Beep();
		DisplayInfoNone("", "USSD ERROR 2", 4);
		ShowLogs(1, "Object expected.");
		return 1;
	}

	for (i = 1; i < r; i++) 
	{
		if (jsoneqqgen(input, &t[i], name) == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, input + t[i+1].start);
			strcpy(value, storeData);
			ShowLogs(1, "NAME: %s", name);
			ShowLogs(1, "VALUE: %s", value);
			i++;
		} else
		{
			ShowLogs(1, "Unexpected key: %.*s\n", t[i].end-t[i].start, input + t[i].start);
		}
	}
}
