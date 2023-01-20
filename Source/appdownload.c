#include "global.h"

#define rcvBufferSize 5000 * 1024

int AppPackGetData(char *ip, int port, char *outData, char *url)
{
	int uiLen = 0;
	char *szBuff = NULL;
	szBuff = (char*)sysalloc_calloc(1024, sizeof(char));
	sprintf(szBuff, "GET %s HTTP/1.1\r\nHost: %s:%d\r\nCache-Control: no-cache\r\n\r\n",
		url, ip, port);
	uiLen = strlen(szBuff);
	ShowLogs(1, "%s", szBuff);
	strcpy(outData, szBuff);
	sysalloc_free(szBuff);
	szBuff = NULL;
	return uiLen;
}

int ReadHttpStatus(int sock)
{
    char c;
    char buff[1024] = "", *ptr = buff + 1;
    int bytes_received, status;
    ShowLogs(1, "1. ReadHttpStatus. Begin Response...");
    while(bytes_received = NetRecv(sock, ptr, 1, 0))
    {
        if(bytes_received < 0)
        {
            ShowLogs(1, "ReadHttpStatus Error");
            return -1;
        }

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) 
        	break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;
    sscanf(ptr,"%*s %d ", &status);
    ShowLogs(1, "2. ReadHttpStatus. %s",ptr);
    ShowLogs(1, "3. ReadHttpStatus. status=%d",status);
    ShowLogs(1, "4. ReadHttpStatus. End Response ..");
    return (bytes_received>0) ? status : 0;
}

int ParseHeader(int sock)
{
    char c;
    char buff[1024]="", *ptr=buff+4;
    int bytes_received, status;
    ShowLogs(1, "1. ParseHeader. Begin HEADER");
    while(bytes_received = NetRecv(sock, ptr, 1, 0))
    {
        if(bytes_received==-1){
            ShowLogs(1, "ParseHeader Error");
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


int RDTcpRxd()
{
	char send_data[1024], recv_data[5000000], *p;
	char disp[128] = {0};
	int sock, bytes_received;
	int contentlengh, iRet;
	int timeoutCount = 0;
	int fd;

	//fd = open((char *)pszFileName, O_RDWR|O_CREATE);

	remove("ARCA.zip");

	ShowLogs(1, "Recieving data...");

	if(ReadHttpStatus(sg_iSocket) && (contentlengh=ParseHeader(sg_iSocket)))
	{
        int bytes = 0;

        fd = open("ARCA.zip", O_CREATE);
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
            sprintf(disp, "%dkb / %dkb", (bytes/1000), (contentlengh/1000));
            //sprintf(disp, "%d %", ((bytes/contentlengh) * 100));
            DisplayInfoNone("", disp, 0);
            if(bytes==contentlengh)
            	break;
        }
        //fclose(fd);
    }
    iRet = close(fd);
    ShowLogs(1, "close: %d", iRet);
    ShowLogs(1, "Filename: ARCA.zip. Filesize: %lu.", filesize("ARCA.zip"));
    iRet = Unzip("ARCA.zip");
	ShowLogs(1, "Unzip returned: %d", iRet);
	if(iRet == 0)
	{
		iRet = FileToApp("ARCA.bin");
		ShowLogs(1, "FILETOAPP: %d", iRet);
		Beep();
		if(iRet != 0)
		{
			ShowLogs(1, "Could not convert file to application");
			return -1;
		}
		remove("ARCA.bin");
		remove("ARCA.zip");
		Beep();
		DelayMs(500);
		UpdateProfile();
		//Reboot();
	}
	return -1;
}

int ApphttpGetData(char *ip, int port, char *url, char *brand, char *model, char *version, 
 char *fromServer, char *appname)
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
	strcat(tempUrl, brand);
	strcat(tempUrl, "/");
	strcat(tempUrl, model);
	strcat(tempUrl, "/");
	strcat(tempUrl, version);

	if(lastUsedHost != 0)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 0;
	
	memset(output, 0, sizeof(output));
	iLen = AppPackGetData(ip, port, output, tempUrl);
	EmvUnsetSSLFlag();

	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);

	if(strstr(temp, "GPRS") != NULL)
 	{
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

		DisplayInfoNone("", "REMOTE DOWNLOAD\nPLEASE WAIT", 0);
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
			CommOnHookGPRS(TRUE);
		}
		iRet = CommDial(DM_DIAL);
		ShowLogs(1, "CommsDial Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Dial failed.");
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
			CommOnHookGPRS(TRUE);
			return -1;
		}
		DisplayInfoNone("", "REMOTE DOWNLOAD\nSENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}
		DisplayInfoNone("", "REMOTE DOWNLOAD\nRECEIVING", 0);
		//iRet = CommRxd(output2, 1024 * 1024, 60, &iLen);
		//iRet = RDTcpRxd(output2, sizeof(output2), &iLen);
		iRet = RDTcpRxd();
		ShowLogs(1, "CommRxd Response: %i.", iRet);
		if (iRet < 0)
		{
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHookGPRS(TRUE);
			return -1;
		}

	 	DisplayInfoNone("", "REMOTE DOWNLOAD\nSUCCESSFUL", 0);
		return HTTP_OK;
	}else
 	{
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

		DisplayInfoNone("", "REMOTE DOWNLOAD\nPLEASE WAIT", 0);
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		ShowLogs(1, "CommsInitialization Response: %d", iRet);
		if (iRet != 0)
		{
			ShowLogs(1, "Comms Initialization failed.");
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
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
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "REMOTE DOWNLOAD\nSENDING", 0);
		iRet = CommTxd(output, strlen(output), 60);
		ShowLogs(1, "CommTxd Response: %d", iRet);
		if (iRet != 0)
		{
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
			ShowLogs(1, "CommTxd failed.");
			CommOnHook(TRUE);
			return -1;
		}
		DisplayInfoNone("", "REMOTE DOWNLOAD\nRECEIVING", 0);
		//iRet = CommRxd(output2, 1024 * 1024, 60, &iLen);
		//iRet = RDTcpRxd(output2, sizeof(output2), &iLen);
		iRet = RDTcpRxd();
		ShowLogs(1, "CommRxd Response: %i.", iRet);
		if (iRet < 0)
		{
			DisplayInfoNone("", "REMOTE DOWNLOAD\nFAILED", 0);
			ShowLogs(1, "CommRxd failed.");
			CommOnHook(TRUE);
			return -1;
		}

	 	DisplayInfoNone("", "REMOTE DOWNLOAD\nSUCCESSFUL", 0);
		return HTTP_OK;
 	}
}

void AppDownload(char *appversion, char *appbrand, char *appmodel)
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

	if(ApphttpGetData(ip, atoi(port), "/ARCA/appdownload/download/", appbrand, appmodel, appversion, 
		responseServer, appname) == 0)
	{
		ShowLogs(1, "Successful. Length of App download: %i", strlen(responseServer));
		/*if(strlen(appname))
		{
			CreateWrite(appname, responseServer);
			DisplayInfoNone("", "RESTARTING....", 0);
			iRet = FileToApp(appname);
			ShowLogs(1, "FILETOAPP: %d", iRet);
			Beep();
			DelayMs(500);
			Reboot();
		}else
		{
			DisplayInfoNone("", "UNSUCCESSFUL", 0);
		}*/

			Reboot();
	}else
	{
		ShowLogs(1, "Not Successful. Length of App download: %d", strlen(responseServer));
	}
	Gui_ClearScr();
	DelayMs(50);
}

void checkForNewApp()
{
	//return;
	char serverIp[20] = {0};
	char serverPort[20] = {0};
	char serverIpStore[20] = {0};
	char serverPortStore[20] = {0};
	char responseServer[10 * 1024] = {0};
	char tid[20] = {0};
	char input[200] = {0};
	uchar outputData[10240] = {0};
	char storeData[20] = {0};
	char store[30] = {0};
	char temp[128] = {0};
	int iRet;
	uchar k;

	char appversion[128] = {0};
	char appbrand[128] = {0};
	char appmodel[128] = {0};
//REMOVE STATIC ipS
char ip[20] = {0};
char port[20] = {0};

memset(ip, '\0', strlen(ip));
UtilGetEnvEx("tcmIP", ip);
memset(port, '\0', strlen(port));
UtilGetEnvEx("tcmPort", port);
if(httpGetData(ip, atoi(port), tid, "/tms/profile/download", responseServer) == 0)
		//if(httpGetData("139.162.204.105", atoi("8088"), tid, "/tms/profile/download", responseServer) == 0)
	{
		int iRet;
		checkNew = 1;
		ShowLogs(1, "PROFILE DATA Successful %s",responseServer);
		iRet = CreateWrite("profile.txt", responseServer);
		ShowLogs(1, "Http Fresh Profile Successful");
		if( iRet == 0 )
		{
			ShowLogs(1, "Writing Profile to file successful. Reading From File");
			memset(outputData, '\0', strlen(outputData));
			iRet = ReadAllData("profile.txt", outputData);
			//ShowLogs(1, "%s", outputData);
			if(iRet == 0)
			{
				if(parseProfile(outputData))
				{
					ShowLogs(1, "Profile Parse not successful");
					//FreshProfile();
				}else
				{
					memset(store, '\0', strlen(store));
					UtilGetEnv("tmDt", store);
	    			SysSetTimeProfile(store);
    			}
				//forceKeyExchange();

    			
    			
			}else
			{
				ShowLogs(1, "profile.txt Reading Error %d", iRet);
				DisplayInfoNone("", "TMS ERROR 1", 2);
			}
		}else
		{
			ShowLogs(1, "profile.txt Create and Writing Error %d", iRet);
			DisplayInfoNone("", "TMS ERROR 2", 2);
		}
	}
	

	
	memset(appversion, '\0', strlen(appversion));
	UtilGetEnv("appversion", appversion);
	ShowLogs(1, "Appversion: %s", appversion);

	memset(appbrand, '\0', strlen(appbrand));
	UtilGetEnv("appbrand", appbrand);
	ShowLogs(1, "appbrand: %s", appbrand);

	memset(appmodel, '\0', strlen(appmodel));
	UtilGetEnv("appmodel", appmodel);
	ShowLogs(1, "appmodel: %s", appmodel);

	if((strlen(appversion) < 1) || (strlen(appbrand) < 1) || (strlen(appmodel) < 1))
	{
		ShowLogs(1, "No app to be downloaded - Yes");
		//DisplayInfoNone("", "No Application....", 2);
		return;
	}

	if(strstr(appversion, "null") == NULL)
	{
		if(strstr(appbrand, "null") == NULL)
		{
			if(strstr(appmodel, "null") == NULL)
			{
				int count = 0;
				while(count < 5)
				{
					Beep();
					ShowLogs(1, "There is app to be downloaded");
					DisplayInfoNone("", "Remote Update", 2);
					Beep();
					DisplayInfoNone("", "Remote Update\nLoading....", 2);
					AppDownload(appversion, appbrand, appmodel);
					count += 1;
				}
				ShowLogs(1, "APPLICATION REBOOTING .................");
				Reboot();
			}else
				ShowLogs(1, "No app to be downloaded");
		}else
			ShowLogs(1, "No app to be downloaded");
	}else
		ShowLogs(1, "No app to be downloaded");
}