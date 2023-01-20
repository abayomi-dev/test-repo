#include "global.h"

//Version Number here
int PackGetData(char *ip, int port, char* tid, char *outData, char *url)
{
	int uiLen = 0;
	char szBuff[MAX_DATA_LEN] = {0};
	char vernum[] = "2.2.61";
	char SN[33] = {0};
	char MANU[100] = {0};
	char MODEL[100] = {0};
	memset(szBuff, 0, sizeof(szBuff));
	memset(SN, '\0', strlen(SN)); 
	ReadSN(SN);
	memset(MANU, '\0', strlen(MANU));
	strcpy(MANU, "brand: PAX");
	memset(MODEL, '\0', strlen(MODEL));
	strcpy(MODEL, "model: s90");
	sprintf(szBuff, "GET %s HTTP/1.1\r\nAccept: */*\r\nAccept-Encoding: gzip, deflate\r\nCache-Control: no-cache\r\nProxy-Connection: Keep-Alive\r\nHost: %s:%d\r\n%s\r\n%s\r\nserial: %s\r\nappversion: %s\r\n\r\n",
		url, ip, port, MANU, MODEL, SN, vernum);
	uiLen = strlen(szBuff);
	ShowLogs(1, "%s", szBuff);
	strcpy(outData, szBuff);
	return uiLen;
}

int TcpDialCustom(char *ip, char *port)
{
	int iRet;
	ShowLogs(1, "TcpDialCustom 1");
	NET_SOCKADDR stServer_addr;
	ShowLogs(1, "TcpDialCustom 2");
	iRet = NetSocket(1, 1, 0);
	ShowLogs(1, "TcpDialCustom 3");
	if (iRet < 0)
	{
		ShowLogs(1, "NetSocket error[%d]", iRet);
		return iRet;	
	}
	sg_iSocket = iRet;
	iRet = SockAddrSet(&stServer_addr, ip, (short)atoi(port));
	ShowLogs(1, "TcpDialCustom 4");
	if (iRet < 0)
	{
		ShowLogs(1, "SockAddrSet error[%d][%s][%d]", iRet, ip, atoi(port));
		return iRet;
	}

	iRet = NetConnect(sg_iSocket, &stServer_addr, sizeof(stServer_addr));
	ShowLogs(1, "TcpDialCustom 5");
	if (iRet < 0)
	{
		ShowLogs(1, "NetConnect error[%d][%d]", iRet, sg_iSocket);
		return iRet;
	}
	ShowLogs(1, "TcpDialCustom 6");
	return 0;
}

int httpGetData(char *ip, int port, char *tid, char *url, char *fromServer)
{
	char output[MAX_DATA_LEN];
	char output2[MAX_DATA_LEN];
	char tempUrl[256];
	int iLen, iRet;
	char temp[128] = {0};
	
	memset(tempUrl, 0, sizeof(tempUrl));
	strcpy(tempUrl, url);
	strcat(tempUrl, tid);

	memset(output, 0, sizeof(output));
	//ShowLogs(1, "Form Url: %s", tempUrl);

	if(lastUsedHost != 0)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 0;
	

	iLen = PackGetData(ip, port, tid, output, tempUrl);
	//ShowLogs(1, "2. Packed Data: %s\n\nLength: %d", output, iLen);
	//Setting Parameter
	EmvUnsetSSLFlag();

	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	ShowLogs(1, "Profile Download Connection Type: %s", temp);
	//if(strstr(temp, "GPRS") != NULL)
	//{
		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, "Etisalat", 8);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, "", 0);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, "", 0);

		memset(temp, '\0', strlen(temp));
		strcpy(temp, ip);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		sprintf(temp, "%d", port);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		DisplayInfoNone("", "PLEASE WAIT", 0);
		ShowLogs(1, "CommInitModule Start");
		

		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHookGPRS(TRUE);
		}

		ShowLogs(1, "CommDial Start");
		iRet = CommDial(DM_DIAL);
		ShowLogs(1, "CommsDial Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Dial failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHookGPRS(TRUE);
			return -1;
		}

		DisplayInfoNone("", "SENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}
		DisplayInfoNone("", "RECEIVING", 0);
		memset(output, '\0', strlen(output));
		iRet = CommRxd(output, 200 * 1024, 60, &iLen);
		//Working
		//iRet = CommRxd(output, 10 * 1024, 15, &iLen);
		ShowLogs(1, "CommRxd Response: %d. Length: %d", iRet, iLen);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}

		ShowLogs(1, "1. Strlen of received: %d", strlen(output));
		ShowLogs(1, "1.1 Data received: '%s'", output);

		CommOnHookGPRS(TRUE);

	 	if(strstr(output, "HTTP/1.1 500") != NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "Tid Error", 0);
			return HTTP_RESP_ERR;
	 	}else if(strstr(output, "HTTP/1.1 200 OK") == NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "FAILED", 0);
			return HTTP_RESP_ERR;
	 	}

	 	DisplayInfoNone("", "Successful", 0);
	 	//ShowLogs(1, "Data gotten: \n%s", output);
	 	memset(output2, '\0', strlen(output2));
	 	ParseGetData(output, output2, "\r\n\r\n");
	 	ShowLogs(1, "2. Strlen of received: %d", strlen(output2));
	 	strcpy(fromServer, output2);
	 	ShowLogs(1, "3. Strlen of received: %d", strlen(fromServer));
		return HTTP_OK;
	/*}else
	{
		//Wifi
		ShowLogs(1, "Inside Wifi");
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);

		memset(temp, '\0', strlen(temp));
		strcpy(temp, ip);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		sprintf(temp, "%d", port);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		DisplayInfoNone("", "PLEASE WAIT", 0);
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHook(TRUE);
		}
		
		ShowLogs(1, "About Calling CommSetCfgParam");
		CommSetCfgParam(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "Done Calling CommSetCfgParam");

		iRet = CommDial(DM_DIAL);
		ShowLogs(1, "CommsDial Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Dial failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "SENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "RECEIVING", 0);
		memset(output, '\0', strlen(output));
		iRet = CommRxd(output, 200 * 1024, 60, &iLen);
		//Working
		//iRet = CommRxd(output, 10 * 1024, 15, &iLen);
		ShowLogs(1, "CommRxd Response: %d. Length: %d", iRet, iLen);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHook(TRUE);
			return -1;
		}

		ShowLogs(1, "1. Strlen of received: %d", strlen(output));

		CommOnHook(TRUE);

	 	if(strstr(output, "HTTP/1.1 500") != NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "Tid Error", 0);
			return HTTP_RESP_ERR;
	 	}else if(strstr(output, "HTTP/1.1 200 OK") == NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "FAILED", 0);
			return HTTP_RESP_ERR;
	 	}

	 	DisplayInfoNone("", "Successful", 0);
	 	//ShowLogs(1, "Data gotten: \n%s", output);
	 	memset(output2, '\0', strlen(output2));
	 	ParseGetData(output, output2, "\r\n\r\n");
	 	ShowLogs(1, "2. Strlen of received: %d", strlen(output2));
	 	strcpy(fromServer, output2);
	 	ShowLogs(1, "3. Strlen of received: %d", strlen(fromServer));
		return HTTP_OK;
	}*/
}

int httpGetDataSsl(char *ip, int port, char *tid, char *url, char *fromServer)
{
	char output[MAX_DATA_LEN];
	char output2[MAX_DATA_LEN];
	char tempUrl[256];
	int iLen, iRet;
	char temp[128] = {0};
	
	memset(tempUrl, 0, sizeof(tempUrl));
	strcpy(tempUrl, url);
	strcat(tempUrl, tid);

	memset(output, 0, sizeof(output));
	//ShowLogs(1, "Form Url: %s", tempUrl);

	if(lastUsedHost != 0)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 0;
	

	iLen = PackGetData(ip, port, tid, output, tempUrl);
	//ShowLogs(1, "2. Packed Data: %s\n\nLength: %d", output, iLen);
	//Setting Parameter
	EmvUnsetSSLFlag();

	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	ShowLogs(1, "Profile Download Connection Type: %s", temp);
	if(strstr(temp, "GPRS") != NULL)
	{
		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, "Etisalat", 8);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, "", 0);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, "", 0);

		memset(temp, '\0', strlen(temp));
		strcpy(temp, ip);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		sprintf(temp, "%d", port);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		DisplayInfoNone("", "PLEASE WAIT", 0);
		ShowLogs(1, "CommInitModule Start");
		

		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHookGPRS(TRUE);
		}

		ShowLogs(1, "CommDial Start");
		iRet = CommDial(DM_DIAL);
		ShowLogs(1, "CommsDial Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Dial failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHookGPRS(TRUE);
			return -1;
		}

		DisplayInfoNone("", "SENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}
		DisplayInfoNone("", "RECEIVING", 0);
		memset(output, '\0', strlen(output));
		iRet = CommRxd(output, 200 * 1024, 60, &iLen);
		//Working
		//iRet = CommRxd(output, 10 * 1024, 15, &iLen);
		ShowLogs(1, "CommRxd Response: %d. Length: %d", iRet, iLen);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}

		ShowLogs(1, "1. Strlen of received: %d", strlen(output));

		CommOnHookGPRS(TRUE);

	 	if(strstr(output, "HTTP/1.1 500") != NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "Tid Error", 0);
			return HTTP_RESP_ERR;
	 	}else if(strstr(output, "HTTP/1.1 200 OK") == NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "FAILED", 0);
			return HTTP_RESP_ERR;
	 	}

	 	DisplayInfoNone("", "Successful", 0);
	 	//ShowLogs(1, "Data gotten: \n%s", output);
	 	memset(output2, '\0', strlen(output2));
	 	ParseGetData(output, output2, "\r\n\r\n");
	 	ShowLogs(1, "2. Strlen of received: %d", strlen(output2));
	 	strcpy(fromServer, output2);
	 	ShowLogs(1, "3. Strlen of received: %d", strlen(fromServer));
		return HTTP_OK;
	}else
	{
		//Wifi
		ShowLogs(1, "Inside Wifi");
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);

		memset(temp, '\0', strlen(temp));
		strcpy(temp, ip);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		sprintf(temp, "%d", port);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		DisplayInfoNone("", "PLEASE WAIT", 0);
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHook(TRUE);
		}
		
		ShowLogs(1, "About Calling CommSetCfgParam");
		CommSetCfgParam(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "Done Calling CommSetCfgParam");

		iRet = CommDial(DM_DIAL);
		ShowLogs(1, "CommsDial Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Dial failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "SENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "RECEIVING", 0);
		memset(output, '\0', strlen(output));
		iRet = CommRxd(output, 200 * 1024, 60, &iLen);
		//Working
		//iRet = CommRxd(output, 10 * 1024, 15, &iLen);
		ShowLogs(1, "CommRxd Response: %d. Length: %d", iRet, iLen);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHook(TRUE);
			return -1;
		}

		ShowLogs(1, "1. Strlen of received: %d", strlen(output));

		CommOnHook(TRUE);

	 	if(strstr(output, "HTTP/1.1 500") != NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "Tid Error", 0);
			return HTTP_RESP_ERR;
	 	}else if(strstr(output, "HTTP/1.1 200 OK") == NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "FAILED", 0);
			return HTTP_RESP_ERR;
	 	}

	 	DisplayInfoNone("", "Successful", 0);
	 	//ShowLogs(1, "Data gotten: \n%s", output);
	 	memset(output2, '\0', strlen(output2));
	 	ParseGetData(output, output2, "\r\n\r\n");
	 	ShowLogs(1, "2. Strlen of received: %d", strlen(output2));
	 	strcpy(fromServer, output2);
	 	ShowLogs(1, "3. Strlen of received: %d", strlen(fromServer));
		return HTTP_OK;
	}
}

void PackPostData(char *ip, int port,  char *url, char *inData, char *outData)
{
	char szBuff[MAX_DATA_LEN] = {0};
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff, "POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %d\r\n\r\n%s",
		url, ip, port, strlen(inData), inData);
	strcpy(outData, szBuff);
}

int httpPostData(char *ip, char *port, char *url, char *data, char *fromServer)
{
	
	char output[MAX_DATA_LEN];
	int iLen, iRet;
	char temp[10] = {0};
	memset(output, 0, sizeof(output));
	PackPostData(ip, atoi(port), url, data, output);
	

	if(lastUsedHost != 0)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 0;


	ShowLogs(1, "%s", output);
	
	EmvUnsetSSLFlag();

	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.szAPN));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.szUID, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.szUID));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.szPwd));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		//strcpy(temp, "45.33.18.98");//Wisdom
		strcpy(temp, ip);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, port);
		//strcpy(temp, "8890");//Wisdom
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		DisplayInfoNone("", "PLEASE WAIT", 0);
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHookGPRS(TRUE);
			return -1;
		}
		iRet = CommDial(DM_DIAL);
		ShowLogs(1, "CommsDial Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Dial failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHookGPRS(TRUE);
			return -1;
		}
		DisplayInfoNone("", "SENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}
		DisplayInfoNone("", "RECEIVING", 0);
		iRet = CommRxd(fromServer, 10 * 1024, 20, &iLen);
		ShowLogs(1, "CommRxd Response: %d. Length: %d", iRet, iLen);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}

		CommOnHookGPRS(TRUE);
		ShowLogs(1, "Response Length: %d", strlen(fromServer));
		ShowLogs(1, "Response: %s", fromServer);

	 	if(strstr(fromServer, "HTTP/1.1 500") != NULL)
	 	{
	 		DisplayInfoNone("", "FAILED", 0);
			return HTTP_RESP_ERR;
	 	}else if(strstr(fromServer, "HTTP/1.1 200 OK") == NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "Failed. Retry", 0);
			return HTTP_RESP_ERR;
	 	}

	 	DisplayInfoNone("", "Successful", 0);
	 	return HTTP_OK;
 	}else
 	{
 		//Wifi
 		ShowLogs(1, "Inside Wifi");
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);

		memset(temp, '\0', strlen(temp));
		strcpy(temp, ip);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		sprintf(temp, "%s", port);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);


		DisplayInfoNone("", "PLEASE WAIT", 0);
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHook(TRUE);
			return -1;
		}
		iRet = CommDial(DM_DIAL);
		ShowLogs(1, "CommsDial Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Dial failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "SENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "RECEIVING", 0);
		iRet = CommRxd(fromServer, 10 * 1024, 20, &iLen);
		ShowLogs(1, "CommRxd Response: %d. Length: %d", iRet, iLen);
		if (iRet != 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHook(TRUE);
			return -1;
		}

		CommOnHook(TRUE);
		ShowLogs(1, "Response Length: %d", strlen(fromServer));
		ShowLogs(1, "Response: %s", fromServer);

	 	if(strstr(fromServer, "HTTP/1.1 500") != NULL)
	 	{
	 		DisplayInfoNone("", "FAILED", 0);
			return HTTP_RESP_ERR;
	 	}else if(strstr(fromServer, "HTTP/1.1 200 OK") == NULL)
	 	{
	 		//ShowLogs(1, "Response not Successful");
	 		DisplayInfoNone("", "Failed. Retry", 0);
			return HTTP_RESP_ERR;
	 	}

	 	DisplayInfoNone("", "Successful", 0);
	 	return HTTP_OK;
 	}
}
