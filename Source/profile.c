#include "global.h"
#include "http.h"

int checkNew = 0;
int numLines = 6;
void parseParameter(char *orig, char* format)
{
	int i = 0, j = 0;
	for(i = 0; i < strlen(orig); i++)
	{
		if(orig[i] == ' ' || orig[i] == '\n')
		{

		}else
		{
			format[j] = orig[i];
			j++;
		}
	} 
}

int fresh = 0;


void KeyProfile()
{
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
//outputData
	fresh = 1;
	ScrBackLight(1);

	Beep();
	memset(input, '\0', strlen(input));
	/*
	iRet = DisplayMsg("SETUP", "TMS IP", "139.162.204.105", input, 7, 16);
	ShowLogs(1, "Server Ip: %s", input);
	if(strlen(input) < 1 || iRet == GUI_ERR_USERCANCELLED)
		FreshProfile();
	else
		strcpy(serverIp, input);


	Gui_ClearScr();
	ScrBackLight(1);
	//Beep();
	memset(input, '\0', strlen(input));
	iRet = DisplayMsg("SETUP", "TMS PORT", "8088", input, 2, 7);
	ShowLogs(1, "Server Port: %s", input);
	if(strlen(input) < 1 || iRet == GUI_ERR_USERCANCELLED)
	{
		FreshProfile();
		strcpy(serverPort, "8088");
	}
	else
		strcpy(serverPort, input);

	Gui_ClearScr();
	ScrBackLight(1);
	//Beep();
	*/
//remove static IPs
char ip[20] = {0};
char port[20] = {0};

memset(ip, '\0', strlen(ip));
UtilGetEnvEx("tcmIP", ip);
memset(port, '\0', strlen(port));
UtilGetEnvEx("tcmPort", port);

	//strcpy(serverIp, "139.162.204.105");
	//strcpy(serverPort, "8088");

	strcpy(serverIp, ip);
	strcpy(serverPort, port);
	strcpy(tid, "");
	ShowLogs(1, "Ip: %s Port: %s Tid: %s", serverIp, serverPort, tid);
	

	CreateWrite("stan.txt", "1");//Set Stan for new Setup
	CreateWrite("receipt.txt", "0");//Set Receipt for new Setup
	CreateWrite("reprint.txt", "0");//clear reprint file
	CreateWrite("count.txt", "0");//Set Count for new Setup
	CreateWrite("eod.txt", "0");//clear eod file

	Gui_ClearScr();
	
	

	DisplayInfoNone("", "USE WIFI?\nPRESS ENTER", 1);
	kbflush();
	while(1)
	{
		if( 0==kbhit() )
		{
			Beep();
			k = getkey();
			if(KEYCANCEL == k)
			{
				glSysParam.stTxnCommCfg.ucCommType = 5;//By Wisdom
				DisplayInfoNone("", "USING GPRS", 2);
				UtilPutEnv("cotype", "GPRS\n");
				UtilPutEnv("cosubnet", "web\n");
				UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
				UtilPutEnv("copwd", "web\n");
				break;
			}else if(KEYENTER == k)
			{
				DisplayInfoNone("", "USING WIFI", 2);
				UtilPutEnv("cotype", "WIFI\n");
				while(1)
				{
					//Temp, pls deleter
					//CreateWrite("wifiPwd.txt", "rfRHNJrtnKd3NT");

					useExisting = 1;
					if(wifiSetup())
						break;
					if(cancelWifi == 1)
					{
						glSysParam.stTxnCommCfg.ucCommType = 5;//By Wisdom
						UtilPutEnv("cotype", "GPRS\n");
						UtilPutEnv("cosubnet", "web\n");
						UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
						UtilPutEnv("copwd", "web\n");
						DisplayInfoNone("", "RETURNING TO GPRS", 1);
						break;
					}
				}
			}
			break;
		}
		DelayMs(200);
	}


	parseParameter(serverIp, serverIpStore);
	parseParameter(serverPort, serverPortStore);
	UtilPutEnv("tcmIP", serverIpStore);
	UtilPutEnv("tcmPort", serverPortStore);

	memset(responseServer, '\0', strlen(responseServer));
	EmvUnsetSSLFlag();
	
	ShowLogs(1,"FreshProfile request: serverIpStore=%s\n serverPortStore=%s\n tid%s",serverIpStore,serverPortStore,tid);
	
	int resp=httpGetData(serverIpStore, atoi(serverPortStore), tid, "/tms/profile/download", responseServer);
		
	ShowLogs(1,"responseServer=%s",responseServer);
	if(resp==0)
	{
		int iRet;
		checkNew = 1;
		
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
					FreshProfile();
				}else
				{
					memset(store, '\0', strlen(store));
					UtilGetEnv("tmDt", store);
	    			SysSetTimeProfile(store);
    			}

				//forceKeyExchange();

				if(GetMasterKey() == 1)
{
	DisplayInfoNone("", "TMK OK", 3);
	if(GetSessionKey() == 1)
	{
		DisplayInfoNone("", "TSK OK", 3);
		if(GetPinKey() == 1)
		{
			DisplayInfoNone("", "TPK OK", 3);
			if(GetParaMeters() == 1)
			{
				DisplayInfoNone("", "PARAMS OK", 3);
			}else
			{
				DisplayInfoNone("", "PARAMS FAILED", 3);
				memset(outputData, '\0', strlen(outputData));
				iRet = ReadAllData("param.txt", outputData);
				if(iRet == 0)
				{
					if(parseParametersOld(outputData))
					{
						ShowLogs(1, "Parameters Parse successful");
					}
				}
			}
		}else
			DisplayInfoNone("", "TPK FAILED", 2);
	}else
		DisplayInfoNone("", "TPK FAILED", 2);
}else
	DisplayInfoNone("", "TMK FAILED", 2);

    			while(1)
    			{
    				if(blockedTerminal())
    					break;
    			}
    			
    			memset(temp, '\0', strlen(temp));
    			ReadAllData("hosa.txt", temp);
    			ShowLogs(1, "Setting CTMK TO PRIORITY: %s", temp);
    			UtilPutEnv("proCtmk", temp);
    			checkForNewApp();
				memset(responseServer, '\0', strlen(responseServer));
				
				ScrBackLight(1);
				Beep();
				UtilPutEnv("fresh", "0");

				logoDownload(1);
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
	}else
	{
		ShowLogs(1, "Http Fresh Profile Not Successful");
		FreshProfile();
	}
	DelayMs(200);
	Gui_ClearScr();
}

void FreshProfile()
{
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

	fresh = 1;
	ScrBackLight(1);

	Beep();
	memset(input, '\0', strlen(input));
	iRet = DisplayMsg("SETUP", "TMS IP", /*"139.162.204.105"*/"52.31.200.20", input, 7, 16);//139.162.203.47/62.173.47.4
	ShowLogs(1, "Server Ip: %s", input);
	if(strlen(input) < 1 || iRet == GUI_ERR_USERCANCELLED)
		FreshProfile();
	else
		strcpy(serverIp, input);


	Gui_ClearScr();
	ScrBackLight(1);
	//Beep();
	memset(input, '\0', strlen(input));
	iRet = DisplayMsg("SETUP", "TMS PORT", "80", input, 2, 7);//9001/8088
	ShowLogs(1, "Server Port: %s", input);
	if(strlen(input) < 1 || iRet == GUI_ERR_USERCANCELLED)
	{
		FreshProfile();
		strcpy(serverPort, "80");//8088
	}
	else
		strcpy(serverPort, input);

	Gui_ClearScr();
	ScrBackLight(1);
	//Beep();

	
	strcpy(tid, "");
	ShowLogs(1, "Ip: %s Port: %s Tid: %s", serverIp, serverPort, tid);
	

	CreateWrite("stan.txt", "1");//Set Stan for new Setup
	CreateWrite("receipt.txt", "0");//Set Receipt for new Setup
	CreateWrite("reprint.txt", "0");//clear reprint file
	CreateWrite("count.txt", "0");//Set Count for new Setup
	CreateWrite("eod.txt", "0");//clear eod file

	Gui_ClearScr();
	
	

	DisplayInfoNone("", "USE WIFI?\nPRESS ENTER", 1);
	kbflush();
	while(1)
	{
		if( 0==kbhit() )
		{
			Beep();
			k = getkey();
			if(KEYCANCEL == k)
			{
				glSysParam.stTxnCommCfg.ucCommType = 5;//By Wisdom
				DisplayInfoNone("", "USING GPRS", 2);
				UtilPutEnv("cotype", "GPRS\n");
				UtilPutEnv("cosubnet", "web\n");
				UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
				UtilPutEnv("copwd", "web\n");
				break;
			}else if(KEYENTER == k)
			{
				DisplayInfoNone("", "USING WIFI", 2);
				UtilPutEnv("cotype", "WIFI\n");
				while(1)
				{
					//Temp, pls deleter
					//CreateWrite("wifiPwd.txt", "rfRHNJrtnKd3NT");

					useExisting = 1;
					if(wifiSetup())
						break;
					if(cancelWifi == 1)
					{
						glSysParam.stTxnCommCfg.ucCommType = 5;//By Wisdom
						UtilPutEnv("cotype", "GPRS\n");
						UtilPutEnv("cosubnet", "web\n");
						UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
						UtilPutEnv("copwd", "web\n");
						DisplayInfoNone("", "RETURNING TO GPRS", 1);
						break;
					}
				}
			}
			break;
		}
		DelayMs(200);
	}


	parseParameter(serverIp, serverIpStore);
	parseParameter(serverPort, serverPortStore);
	UtilPutEnv("tcmIP", serverIpStore);
	UtilPutEnv("tcmPort", serverPortStore);

	memset(responseServer, '\0', strlen(responseServer));
	EmvUnsetSSLFlag();
	if(httpGetData(serverIpStore, atoi(serverPortStore), tid, "/tms/profile/download", responseServer) == 0)
	{
		int iRet;
		checkNew = 1;
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
					FreshProfile();
				}else
				{
					memset(store, '\0', strlen(store));
					UtilGetEnv("tmDt", store);
	    			SysSetTimeProfile(store);
    			}
				//forceKeyExchange();
				if(GetMasterKey() == 1)
				{
					DisplayInfoNone("", "TMK OK", 3);
					if(GetSessionKey() == 1)
					{
						DisplayInfoNone("", "TSK OK", 3);
						if(GetPinKey() == 1)
						{
							DisplayInfoNone("", "TPK OK", 3);
							if(GetParaMeters() == 1)
							{
								DisplayInfoNone("", "PARAMS OK", 3);
							}else
							{
								DisplayInfoNone("", "PARAMS FAILED", 3);
								memset(outputData, '\0', strlen(outputData));
								iRet = ReadAllData("param.txt", outputData);
								if(iRet == 0)
								{
									if(parseParametersOld(outputData))
									{
										ShowLogs(1, "Parameters Parse successful");
									}
								}
							}
						}else
							DisplayInfoNone("", "TPK FAILED", 2);
					}else
						DisplayInfoNone("", "TPK FAILED", 2);
				}else
					DisplayInfoNone("", "TMK FAILED", 2);
				while(1)
				{
					if(blockedTerminal())
						break;
				}
							
				memset(temp, '\0', strlen(temp));
				ReadAllData("hosa.txt", temp);
				ShowLogs(1, "Setting CTMK TO PRIORITY: %s", temp);
				UtilPutEnv("proCtmk", temp);
				checkForNewApp();
				memset(responseServer, '\0', strlen(responseServer));
				
				ScrBackLight(1);
				Beep();
				UtilPutEnv("fresh", "0");

				logoDownload(1);

				Gui_ClearScr();
				PackCallHomeData();
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
	}else
	{
		ShowLogs(1, "Http Fresh Profile Not Successful");
		FreshProfile();
	}
	DelayMs(200);
	Gui_ClearScr();
}



void UpdateProfile()
{
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

	fresh = 1;
	ScrBackLight(1);

	Beep();
	memset(input, '\0', strlen(input));
	iRet = DisplayMsg("SETUP", "TMS IP", /*"139.162.204.105"*/"62.173.47.4", input, 7, 16);
	ShowLogs(1, "Server Ip: %s", input);
	if(strlen(input) < 1 || iRet == GUI_ERR_USERCANCELLED)
		FreshProfile();
	else
		strcpy(serverIp, input);


	Gui_ClearScr();
	ScrBackLight(1);
	//Beep();
	memset(input, '\0', strlen(input));
	iRet = DisplayMsg("SETUP", "TMS PORT", "80", input, 2, 7);//8088
	ShowLogs(1, "Server Port: %s", input);
	if(strlen(input) < 1 || iRet == GUI_ERR_USERCANCELLED)
	{
		FreshProfile();
		strcpy(serverPort, "80");//8088
	}
	else
		strcpy(serverPort, input);

	Gui_ClearScr();
	ScrBackLight(1);
	//Beep();

	
	strcpy(tid, "");
	ShowLogs(1, "Ip: %s Port: %s Tid: %s", serverIp, serverPort, tid);
	

	CreateWrite("stan.txt", "1");//Set Stan for new Setup
	CreateWrite("receipt.txt", "0");//Set Receipt for new Setup
	CreateWrite("reprint.txt", "0");//clear reprint file
	CreateWrite("count.txt", "0");//Set Count for new Setup
	CreateWrite("eod.txt", "0");//clear eod file

	Gui_ClearScr();
	
	

	DisplayInfoNone("", "USE WIFI?\nPRESS ENTER", 1);
	kbflush();
	while(1)
	{
		if( 0==kbhit() )
		{
			Beep();
			k = getkey();
			if(KEYCANCEL == k)
			{
				glSysParam.stTxnCommCfg.ucCommType = 5;//By Wisdom
				DisplayInfoNone("", "USING GPRS", 2);
				UtilPutEnv("cotype", "GPRS\n");
				UtilPutEnv("cosubnet", "web\n");
				UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
				UtilPutEnv("copwd", "web\n");
				break;
			}else if(KEYENTER == k)
			{
				DisplayInfoNone("", "USING WIFI", 2);
				UtilPutEnv("cotype", "WIFI\n");
				while(1)
				{
					//Temp, pls deleter
					//CreateWrite("wifiPwd.txt", "rfRHNJrtnKd3NT");

					useExisting = 1;
					if(wifiSetup())
						break;
					if(cancelWifi == 1)
					{
						glSysParam.stTxnCommCfg.ucCommType = 5;//By Wisdom
						UtilPutEnv("cotype", "GPRS\n");
						UtilPutEnv("cosubnet", "web\n");
						UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
						UtilPutEnv("copwd", "web\n");
						DisplayInfoNone("", "RETURNING TO GPRS", 1);
						break;
					}
				}
			}
			break;
		}
		DelayMs(200);
	}


	parseParameter(serverIp, serverIpStore);
	parseParameter(serverPort, serverPortStore);
	UtilPutEnv("tcmIP", serverIpStore);
	UtilPutEnv("tcmPort", serverPortStore);

	memset(responseServer, '\0', strlen(responseServer));
	EmvUnsetSSLFlag();
	if(httpGetData(serverIpStore, atoi(serverPortStore), tid, "/tms/profile/download", responseServer) == 0)
	{
		int iRet;
		checkNew = 1;
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
					FreshProfile();
				}else
				{
					memset(store, '\0', strlen(store));
					UtilGetEnv("tmDt", store);
	    			SysSetTimeProfile(store);
    			}

				//forceKeyExchange();
				if(GetMasterKey() == 1)
{
	DisplayInfoNone("", "TMK OK", 3);
	if(GetSessionKey() == 1)
	{
		DisplayInfoNone("", "TSK OK", 3);
		if(GetPinKey() == 1)
		{
			DisplayInfoNone("", "TPK OK", 3);
			if(GetParaMeters() == 1)
			{
				DisplayInfoNone("", "PARAMS OK", 3);
			}else
			{
				DisplayInfoNone("", "PARAMS FAILED", 3);
				memset(outputData, '\0', strlen(outputData));
				iRet = ReadAllData("param.txt", outputData);
				if(iRet == 0)
				{
					if(parseParametersOld(outputData))
					{
						ShowLogs(1, "Parameters Parse successful");
					}
				}
			}
		}else
			DisplayInfoNone("", "TPK FAILED", 2);
	}else
		DisplayInfoNone("", "TPK FAILED", 2);
}else
	DisplayInfoNone("", "TMK FAILED", 2);
    			while(1)
    			{
    				if(blockedTerminal())
    					break;
    			}
    			
    			memset(temp, '\0', strlen(temp));
    			ReadAllData("hosa.txt", temp);
    			ShowLogs(1, "Setting CTMK TO PRIORITY: %s", temp);
    			UtilPutEnv("proCtmk", temp);
    			//checkForNewApp();
				memset(responseServer, '\0', strlen(responseServer));
				
				ScrBackLight(1);
				Beep();
				UtilPutEnv("fresh", "0");

				logoDownload(1);
				Gui_ClearScr();
				PackCallHomeData();
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
	}else
	{
		ShowLogs(1, "Http Fresh Profile Not Successful");
		FreshProfile();
	}
	DelayMs(200);
	Gui_ClearScr();
}

int cancelWifi = 0;
void OldProfile()
{
	char serverIp[20] = {0};
	char serverPort[20] = {0};
	char responseServer[100 * 1024] = {0};
	char tid[20] = {0};
	char tidTemp[20] = {0};
	char input[200] = {0};
	uchar outputData[10240] = {0};
	int iRet = 0;
	char store[20] = {0};
	char temp[128] = {0};

	fresh = 0;

	GetStan();//Get Stan

	UtilGetEnvEx("tcmIP", serverIp);
	UtilGetEnvEx("tcmPort", serverPort);

	if(adminTid)
	{
		UtilGetEnvEx("tTermi", tid);
	}else
		UtilGetEnv("tid", tid);


	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	ShowLogs(1, "Checking Connection Type");
	//cancelWifi = 0;
	if(strstr(temp, "GPRS") == NULL)
	{
		ShowLogs(1, "Connection Type is Wifi");
		DisplayInfoNone("", "CON TYPE: WIFI", 0);
		while(1)
		{
			useExisting = 1;
			cancelWifi = 0;
			if(wifiSetup())
				break;
			if(cancelWifi == 1)
			{
				glSysParam.stTxnCommCfg.ucCommType = 5;
				UtilPutEnv("cotype", "GPRS\n");
				UtilPutEnv("cosubnet", "web\n");
				UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
				UtilPutEnv("copwd", "web\n");
				DisplayInfoNone("", "RETURNING TO GPRS", 1);
				break;
			}
		}
	}



	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	ShowLogs(1, "Checking Connection Type");
	//cancelWifi = 0;
	memset(temp, '\0', strlen(temp)); 
	memset(responseServer, '\0', strlen(responseServer));
	EmvUnsetSSLFlag();

	strcpy(tid, "");
	if(httpGetData(serverIp, atoi(serverPort), tid, "/tms/profile/download", responseServer) == 0)
	{
		iRet = CreateWrite("profile.txt", responseServer);
		if( iRet == 0 )
		{
			if(adminTid)
			{
				memset(tid, '\0', strlen(tid));
				UtilGetEnvEx("tTermi", tid);
				memset(tidTemp, '\0', strlen(tidTemp));
				sprintf(tidTemp, "%s ", tid);
				UtilPutEnv("tid", tidTemp);
				adminTid = 0;
			}

			ShowLogs(1, "Writing Profile to file successful. Reading From File");
			memset(outputData, '\0', strlen(outputData));
			iRet = ReadAllData("profile.txt", outputData);
			if(iRet == 0)
			{
				if(parseProfile(outputData))
				{
					ShowLogs(1, "Profile Not Parse successful");
				}else
				{
					memset(store, '\0', strlen(store));
					UtilGetEnv("tmDt", store);
	    			SysSetTimeProfile(store);
				}

				//forceKeyExchange();
				if(GetMasterKey() == 1)
{
	DisplayInfoNone("", "TMK OK", 3);
	if(GetSessionKey() == 1)
	{
		DisplayInfoNone("", "TSK OK", 3);
		if(GetPinKey() == 1)
		{
			DisplayInfoNone("", "TPK OK", 3);
			if(GetParaMeters() == 1)
			{
				DisplayInfoNone("", "PARAMS OK", 3);
			}else
			{
				DisplayInfoNone("", "PARAMS FAILED", 3);
				memset(outputData, '\0', strlen(outputData));
				iRet = ReadAllData("param.txt", outputData);
				if(iRet == 0)
				{
					if(parseParametersOld(outputData))
					{
						ShowLogs(1, "Parameters Parse successful");
					}
				}
			}
		}else
			DisplayInfoNone("", "TPK FAILED", 2);
	}else
		DisplayInfoNone("", "TPK FAILED", 2);
}else
	DisplayInfoNone("", "TMK FAILED", 2);
				while(1)
    			{
    				if(blockedTerminal())
    					break;
    			}
    			
				memset(temp, '\0', strlen(temp));
    			ReadAllData("hosa.txt", temp);
    			ShowLogs(1, "Setting CTMK TO PRIORITY: %s", temp);
    			UtilPutEnv("proCtmk", temp);
				checkForNewApp();
				memset(responseServer, '\0', strlen(responseServer));

 				ScrBackLight(1);
				Beep();
				UtilPutEnv("fresh", "0");

				logoDownload(1);

				Gui_ClearScr();
				PackCallHomeData();
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
	}else
	{
		memset(outputData, '\0', strlen(outputData));
		iRet = ReadAllData("profile.txt", outputData);
		if(iRet == 0)
		{
			if(parseProfileOld(outputData))
			{
				ShowLogs(1, "Old Profile Parse successful");
			}
		}

		while(1)
		{
			if(blockedTerminal())
				break;
		}

		if(cancelWifi == 1)
		{
			UtilPutEnv("cotype", "GPRS\n");
			UtilPutEnv("cosubnet", "web\n");
			UtilPutEnv("coapn", "web.gprs.mtnnigeria.net\n");
			UtilPutEnv("copwd", "web\n");
			//DisplayInfoNone("", "RETURNING TO GPRS", 1);
		}

		ScrBackLight(1);
		Beep();
		/*if(GetParaMeters())
			DisplayInfoNone("", "Ready For Transaction", 2);
		else*/
		if(GetMasterKey() == 1)
		{
			DisplayInfoNone("", "TMK OK", 2);
			if(GetSessionKey() == 1)
			{
				DisplayInfoNone("", "TSK OK", 2);
				if(GetPinKey() == 1)
				{
					DisplayInfoNone("", "TPK OK", 2);
					if(GetParaMeters() == 1)
					{
						DisplayInfoNone("", "PARAMS OK", 2);
					}else
					{
						DisplayInfoNone("", "PARAMS FAILED", 2);
						memset(outputData, '\0', strlen(outputData));
						iRet = ReadAllData("param.txt", outputData);
						if(iRet == 0)
						{
							if(parseParametersOld(outputData))
							{
								ShowLogs(1, "Parameters Parse successful");
							}
						}
					}
				}else
					DisplayInfoNone("", "TPK FAILED", 2);
			}else
				DisplayInfoNone("", "TSK FAILED", 2);
		}else
			DisplayInfoNone("", "TMK FAILED", 2);
		//getHost2Keys();//Get Host two keys and resetting it to host one
	}
	DelayMs(200);
	Gui_ClearScr();
}