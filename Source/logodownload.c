#include "global.h"


void LogoPackGetData(char *ip, int port, char *outData, char *url)
{
	sprintf(outData, "GET %s HTTP/1.1\r\nHost: %s:%d\r\nCache-Control: no-cache\r\n\r\n",
		url, ip, port);
	ShowLogs(1, "%s", outData);
}

int LogoReadHttpStatus(int sock)
{
    char c;
    char buff[1024] = "", *ptr = buff + 1;
    int bytes_received, status;
    ShowLogs(1, "1. LogoReadHttpStatus. Begin Response...");
    while(bytes_received = NetRecv(sock, ptr, 1, 0))
    {
        if(bytes_received < 0)
        {
            ShowLogs(1, "LogoReadHttpStatus Error");
            return -1;
        }

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) 
        	break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;
    sscanf(ptr,"%*s %d ", &status);
    ShowLogs(1, "2. LogoReadHttpStatus. %s",ptr);
    ShowLogs(1, "3. LogoReadHttpStatus. status=%d",status);
    ShowLogs(1, "4. LogoReadHttpStatus. End Response ..");
    return (bytes_received>0) ? status : 0;
}

int LogoParseHeader(int sock)
{
    char c;
    char buff[1024]="", *ptr=buff+4;
    int bytes_received, status;
    ShowLogs(1, "1. LogoParseHeader. Begin HEADER");
    while(bytes_received = NetRecv(sock, ptr, 1, 0))
    {
        if(bytes_received==-1){
            ShowLogs(1, "LogoParseHeader Error");
            return -1;
        }

        if(
            (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
            (ptr[-1]=='\r')  && (*ptr=='\n' )
        ) break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+4;

    if(bytes_received){
        ptr=strstr(ptr,"Content-Length:");
        if(ptr){
            sscanf(ptr,"%*s %d",&bytes_received);

        }else
            bytes_received=-1;

       ShowLogs(1, "Content-Length: %d\n",bytes_received);
    }
    ShowLogs(1, "EndHEADER");
    return  bytes_received ;
}


int LogoRDTcpRxd(char *mode)
{
	char send_data[1024], recv_data[5000000], *p;
	char disp[128] = {0};
	int sock, bytes_received;
	int contentlengh, iRet;
	int timeoutCount = 0;
	int fd;

	//fd = open((char *)pszFileName, O_RDWR|O_CREATE);

	if(strcmp(mode, "true") == 0)
		remove("logo.bmp");
	else
		remove("background.bmp");

	ShowLogs(1, "Recieving data...");

	if(LogoReadHttpStatus(sg_iSocket) && (contentlengh=LogoParseHeader(sg_iSocket)))
	{
        int bytes = 0;

        if(strcmp(mode, "true") == 0)
			fd = open("logo.bmp", O_CREATE);
		else
			fd = open("background.bmp", O_CREATE);
        ShowLogs(1, "Open: %d", fd);
        ShowLogs(1, "Saving data...");
        while(bytes_received = NetRecv(sg_iSocket, recv_data, 5000000, 0))
        {
        	if (bytes_received == -13) {
				DelayMs(200);
				timeoutCount++;
				if (timeoutCount > 3) {
					ShowLogs(1, "Timeout occurred");
					break;
				}
			} else if(bytes_received < 0){
        		ShowLogs(1, "bytes_received < 0");
                break;
            }

            //fwrite(recv_data,1,bytes_received,fd);
            iRet = seek(fd, bytes, SEEK_SET);
			if (iRet < 0)
			{
				close(fd);
				ShowLogs(1, "Seek Error..");
				return -1;
			}
	
			iRet = PubFWriteN(fd, &recv_data, bytes_received);
			if (iRet != bytes_received)
			{
				ShowLogs(1, "Written: %d, %d", iRet, bytes_received);
				close(fd);
				return -1;
			}else
			{
				ShowLogs(1, "Saved: %d", iRet);
			}
            bytes += bytes_received;
            ShowLogs(1, "Bytes recieved: %d from %d", bytes, contentlengh);
            memset(disp, '\0', strlen(disp));
            sprintf(disp, "%dkb OF %dkb", (bytes/1000), (contentlengh/1000));
            //sprintf(disp, "%d %", ((bytes/contentlengh) * 100));
            DisplayInfoNone("", disp, 0);
            if(bytes==contentlengh)
            	break;
        }
        //fclose(fd);
    }
    iRet = close(fd);
    ShowLogs(1, "close: %d", iRet);
    if(strcmp(mode, "true") == 0)
		ShowLogs(1, "Filename: logo.bmp. Filesize: %lu.", filesize("logo.bmp"));
	else
		ShowLogs(1, "Filename: background.bmp. Filesize: %lu.", filesize("background.bmp"));
    return 1;
}

int LogohttpGetData(char *ip, int port, char *url, char *version, 
 char *bankcode, char *mode)
{
	char tempUrl[256];
	int iLen, iRet;
	char temp[128] = {0};
	char *ret;
	long j;
	int i;
	char output[1024] = {0};

	memset(tempUrl, 0, sizeof(tempUrl));
	strcpy(tempUrl, url);
	strcat(tempUrl, bankcode);
	strcat(tempUrl, "/");
	strcat(tempUrl, version);
	strcat(tempUrl, "/");
	strcat(tempUrl, mode);

	if(lastUsedHost != 0)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 0;
	
	memset(output, 0, sizeof(output));
	LogoPackGetData(ip, port, output, tempUrl);

	ShowLogs(1, "FINAL: %s", output);

	EmvUnsetSSLFlag();

	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);

	ShowLogs(1, "ENTERING 1");
	if(strstr(temp, "GPRS") != NULL)
 	{
 		ShowLogs(1, "ENTERING 2");
		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		strcpy(temp, "web.gprs.mtnnigeria.net");
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, "web", 3);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, "web", 3);
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

		ScrBackLight(1);
		Beep();

		DisplayInfoNone("", "PLEASE WAIT", 0);
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "FAILED", 0);
			CommOnHookGPRS(TRUE);
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
		//iRet = CommRxd(output2, 1024 * 1024, 60, &iLen);
		//iRet = RDTcpRxd(output2, sizeof(output2), &iLen);
		iRet = LogoRDTcpRxd(mode);
		ShowLogs(1, "CommRxd Response: %i.", iRet);
		if (iRet < 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}

	 	DisplayInfoNone("", "Successful", 0);
		return HTTP_OK;
	}else
 	{
 		ShowLogs(1, "ENTERING 3");
 		//For wifi
 		glSysParam.stTxnCommCfg.ucCommType = 6;
		memset(temp, '\0', strlen(temp));
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

		ScrBackLight(1);
		Beep();

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
		//iRet = CommRxd(output2, 1024 * 1024, 60, &iLen);
		//iRet = RDTcpRxd(output2, sizeof(output2), &iLen);
		iRet = LogoRDTcpRxd(mode);
		ShowLogs(1, "CommRxd Response: %i.", iRet);
		if (iRet < 0)
		{
			DisplayInfoNone("", "FAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHook(TRUE);
			return -1;
		}

	 	DisplayInfoNone("", "Successful", 0);
		return HTTP_OK;
 	}
}

void LogoDownload(char *version, char *bankcode, char *mode)
{
	//Work on this tomorrow
	char responseServer[1024 * 1024] = {0};
	char ip[20] = {0};
	char port[20] = {0};
	char appname[100] = {0};
	int iRet;
	memset(ip, '\0', strlen(ip));
	UtilGetEnvEx("tcmIP", ip);
	ShowLogs(1, "Ip: %s", ip);
	memset(port, '\0', strlen(port));
	UtilGetEnvEx("tcmPort", port);
	ShowLogs(1, "Port: %s", port);

	memset(responseServer, '\0', strlen(responseServer));
	ShowLogs(1, "ENTERING MAIN");
	if(LogohttpGetData(ip, atoi(port), "/arca/logodownload/download/", version, bankcode, mode) == 0)
	{
		//DisplayInfoNone("", "RECEIPT LOGO SUCCESS", 2);	
	}else
	{
		DisplayInfoNone("", "Receipt Logo Failed", 2);
	}
	Beep();
	Gui_ClearScr();
	DelayMs(50);
}



void logoDownload(int isReceipt)
{
	char version[128] = {0};
	char bankcode[128] = {0};

	if(isReceipt)
	{
		memset(version, '\0', strlen(version));
		UtilGetEnv("logoversion", version);
		ShowLogs(1, "1. logoversion: %s", version);
		ShowLogs(1, "1. logobankcode: %s", profileTag.bankcode);
		DisplayInfoNone("", "CONNECTING", 1);
		LogoDownload(version, profileTag.bankcode, "true");
	}else
	{
		memset(version, '\0', strlen(version));
		UtilGetEnv("logobversion", version);
		ShowLogs(1, "2. logobversion: %s", version);
		ShowLogs(1, "2. logobbankcode: %s", profileTag.bankcode);
		LogoDownload(version, profileTag.bankcode, "false");
		DisplayInfoNone("", "CONNECTING", 5);
	}
}