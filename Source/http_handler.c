#include <posapi.h>
#include <posapi_all.h>
#include "zip_api.h"
#include "http_handler.h"
#include "HttpApi.h"
#include "sslapi.h"
#include "PhyComms.h"
//#include "Debug.h"
#include "NetProc.h"
#include "global.h"

extern int SEND_SUCCESSFUL;
extern int function;
extern int sg_iSocket;
extern char AgentPOSID[30];

int SEND_SUCCESSFUL;

char Cel_responseCode[20];
char Cel_responseMessage[70];
extern char Cel_AccountName[50];
extern char Cel_AccountRef[50];
extern char Cel_PlatformID[10];
extern char Cel_Token[90];



//eedc variables
extern char eedc_transaction_reference[20];
extern char eedc_units[20];
extern char eedc_appliedToArrears[20];
extern char eedc_token[90];
extern char eedc_vat[6];
extern char eedc_phone [10];
extern char eedc_customerName[40];
extern char eedc_convenience[10];
extern char eedc_total [10];
//END EEDC 
//JAMB
extern char jamb_customerName[40];
extern int JAMB_CODE[15];
extern int JAMB_CUS_ID[15];

//END JAMB

extern char Cel_TransactionId[22];
extern char Cel_BenAmunt[12];
extern char Cel_TinAccountNumber[12];


char RefDate[25]={0};

void   DisplayStrMiddle(char *pszString, int Pos)
{
	char  szSpaceBuffer[40];
	int   iDatalen, iMaxLine, iTotalLine, i;
	iDatalen = strlen(pszString);
	       
    memset(szSpaceBuffer,0x00,sizeof(szSpaceBuffer));
    for (i=1;i<=(10-(iDatalen/2));i++)
    strcat(szSpaceBuffer," ");

	//if(Pos==0)
		ScrPrint(0,Pos,0,"%s%s",szSpaceBuffer,pszString);
	/*else
		ScrPrint(0,Pos,REVER,"%s%s",szSpaceBuffer,pszString);*/

		//PrnStr("%s%s\n",szSpaceBuffer,pszString);
	//}
}

void   DisplayStrMiddleREVER(char *pszString, int Pos)
{
	char  szSpaceBuffer[40];
	int   iDatalen, iMaxLine, iTotalLine, i;
	iDatalen = strlen(pszString);

	if(Pos==0){
		ScrCls();
		ScrGotoxy(0,0);
		ScrSetIcon (ICON_DOWN, CLOSEICON);
		ScrSetIcon (ICON_UP, CLOSEICON);
	}
	       
    memset(szSpaceBuffer,0x00,sizeof(szSpaceBuffer));
    for (i=1;i<=(23-iDatalen)/2;i++)
    strcat(szSpaceBuffer," ");

	ScrPrint(0,Pos,REVER,"%s%s%s",szSpaceBuffer,pszString,szSpaceBuffer);//REVER
	//ScrPrint(0,0,REVER,TypeRequest);
	ScrClrLine(Pos+1,Pos+1);
		//PrnStr("%s%s\n",szSpaceBuffer,pszString);
	//}
}



void displayMessageWt(const char * header, const char * message){

	ScrCls();
	ScrAttrSet(0);
	//ScrPrint ((21-strlen(header))/2,0, REVER, header);
	DisplayStrMiddleREVER((unsigned char *)header,0);
	//ScrPrint(1, 4, 0, "%*s", NUM_MAXZHCHARS, "");
	//ScrPrint((uchar)((NUM_MAXCOLS-8*strlen(header))/2)+1, 4,0 ,"%.*s", strlen(header), (char *)header);
	//display_at (1, 2, "---------------------", CLR_LINE);

	if(strlen(message) <= 21 ){
       // ScrPrint (0, 4,0, message);
		DisplayStrMiddle(message,4);
		 //ScrPrint ((21-strlen(message))/2, 4,0, message);
		 //ScrPrint(1, 4, 0, "%*s", NUM_MAXZHCHARS, "");
		 //ScrPrint((uchar)((NUM_MAXCOLS-8*strlen(message))/2)+1, 4,0 ,"%.*s", strlen(message), (char *)message);
	}
	else if((strlen(message) > 21) && (strlen(message) < 42)){
        char temp[22];
		memset(temp,0,sizeof(temp));
		strncpy(temp, message, 21);
		ScrPrint (1, 4,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[21], 21);
		ScrPrint (((21-strlen(temp))/2)+1, 5,0, temp);
	}else if((strlen(message) > 42) && (strlen(message) < 63)){
		char temp[22];
		memset(temp,0,sizeof(temp));
		strncpy(temp, message, 21);
		ScrPrint (1, 4,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[21], 21);
		ScrPrint (1, 5,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[42], 21);
		ScrPrint (((21-strlen(temp))/2)+1, 6,0, temp);
	}else if((strlen(message) > 63) && (strlen(message) < 84)){
		char temp[22];
		memset(temp,0,sizeof(temp));
		strncpy(temp, message, 21);
		ScrPrint (1, 4,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[21], 21);
		ScrPrint (1, 5,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[42], 21);
        ScrPrint (1, 6,0, temp);

        memset(temp,0,sizeof(temp));
		strncpy(temp, &message[63], 21);
		ScrPrint(((21-strlen(temp))/2)+1, 7,0, temp);
	}else{
		char temp[22];
		memset(temp,0,sizeof(temp));
		strncpy(temp, message, 21);
		ScrPrint (1, 2,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[21], 21);
		ScrPrint (1, 3,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[42], 21);
        ScrPrint (1, 4,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[63], 21);
        ScrPrint (1, 5,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[84], 21);
        ScrPrint (1, 6,0, temp);

		memset(temp,0,sizeof(temp));
		strncpy(temp, &message[95], 21);
        ScrPrint (1, 7,0, temp);
		
        memset(temp,0,sizeof(temp));
		strncpy(temp, &message[116], 21);
		ScrPrint(((21-strlen(temp))/2)+1, 8,0, temp);
	}

	//display_at ((21-strlen("PRESS ANY KEY"))/2, 8, "PRESS ANY KEY", CLR_LINE);
	//waitkey("");
	DelayMs(1000);

}


int WirelessDial(uchar *sAPN,uchar *sUser,uchar *sPwd)
{
	int   ii,iRet=1,iTimeoutNum;
	
	iTimeoutNum = 0;
	do {
		iTimeoutNum++;
		for (ii=0;ii<3;ii++)
		{
			
			WlPppLogin(sAPN,sUser,sPwd,0xff,0,0);
			
			//DelayMs(2000);
			while (WlPppCheck()==1)
			{
				iRet = FAIL;
				if (kbhit()==OK)
				{
					if (getkey()==KEYCANCEL)
					{
						break;
					}
				}
				//DelayMs(200);
				continue;
			}
			if (WlPppCheck()==0)
			{
				iRet = OK;
				break;
			}
			//ScrPrint(0,2,0,"PPP Logon time:%d",ii);
		}
		if (iRet !=0 && iTimeoutNum ==1)
		{
			
			//ScrPrint(0,3,0,"WlSwitchPower!");
			WlSwitchPower(0);
			DelayMs(5*1000);//10*1000
			WlSwitchPower(1);
		}
	}
	
	while (iRet != 0 && iTimeoutNum == 1);
	return iRet;
}


 void textView(unsigned char col, unsigned char row, layoutT center, layoutT inverse, char* fmt, ...)
{
  short width;
  short len;
  short spaceRequired;
  short halfSpace;
  char display[0x200] = {'\0'};
  char buffer[500];

  char line[0x800] = {'\0'};

  va_list ap; /* points to each unnamed arg in turn */

  char *p, *sval;
  int ival;
  double dval;

  short nextPos = 0;


  va_start(ap, fmt); /* make ap point to 1st unnamed arg */

  for (p = fmt; *p; p++)
  {
    if (*p != '%')
    {
      line[nextPos++] = *p;
      continue;
    }

    switch (*++p)
    {
      case 'd':
        ival = va_arg(ap, int);
        memset(buffer, '\0', sizeof(buffer));
        sprintf(buffer, "%d", ival);
        nextPos += sprintf(&line[nextPos], "%s", buffer);
        break;

      case 'f':
        dval = va_arg(ap, double);

        memset(buffer, '\0', sizeof(buffer));
        sprintf(buffer, "%f", ival);
        nextPos += sprintf(&line[nextPos], "%s", buffer);
        //printf("%f", dval);
        break;

      case 's':
        for (sval = va_arg(ap, char *); *sval; sval++)
          line[nextPos++] = *sval;
        break;
      default:
        line[nextPos++] = *p;
        break;
    }
  }

  va_end(ap); /* clean up when done */
 

  width = 21;
  len = strlen(line);

  spaceRequired = width - len;


  if ((spaceRequired < 0) || ((center != LAYOUT_CENTER) && (inverse != LAYOUT_INVERSE))) {
   ScrPrint(col, row,0, line);
  return;
  }


  halfSpace = spaceRequired / 2;


  if (center == LAYOUT_CENTER && halfSpace)
  {
    char space[60] = {'\0'};

    memset(space, ' ', halfSpace);

    if (!(spaceRequired % 2))
    {
    sprintf(display, "%s%s%s", space, line, space);
    }
    else
    {
    sprintf(display, "%s%s%s ", space, line, space);
    }
  }
  else
 {
  if (spaceRequired % 2){
  sprintf(display, "%s ", line);
 }
else{
  sprintf(display, "%s", line);
  }
}

 
  
  if (inverse == LAYOUT_INVERSE) 
  {
    ScrPrint(col, row, REVER, display);
  }
  else
  {
  ScrPrint(col, row,0, display);
  }
  
}



void DisplayLogs(int displaylogs, const char *format, ...)
{
	va_list ap;
	
	char finalMessage[10 * 50] = { 0 };
	char store[10 * 20] = { 0 };
	int maxLen = sizeof(finalMessage) - 5;

	if(strlen(finalMessage) > 500)
		return;

	va_start(ap, format);
	vsnprintf(finalMessage, maxLen, format, ap);
	va_end(ap);
	//strcat(finalMessage, "\r\n");
	
	
	/*ScrCls();			
	textView(1, 1,LAYOUT_CENTER, LAYOUT_INVERSE,"DisplayLogs" );
	textView(1, 2,LAYOUT_CENTER, LAYOUT_DEFAULT,"%s",finalMessage);
	DelayMs(1000);*/

	//SerialLog(finalMessage);
}


/**
* @param httpMethod
* @param hostURL
* @param postData
* @param post_data_len
* @param headers
* @param header_len
* @param chunk [out] @brief remember to always call free(chunk.memory);
* @return
*/
int sendHttpRequest(unsigned char httpMethod, const char* hostURL, const char* postData, size_t post_data_len, const char** headers, size_t header_len, MemoryStruct* chunk) {
	char params[2000] = { 0 };
	//char myparam[100] = { 0 };
	char temp[2000] = { 0 };
	char tmp[50];
	int sockfd ;
	int ret = -1;
	int sl,i ;
	//POSParamaters myParams;
			
#ifdef APP_DEBUG
	char Debug[2];
	Debug[0] = 1;
	Debug[1] = DEBUG_PRI_CHAR;
	HttpParaCtl(0, HTTP_CMD_SET_DEBUG, Debug, 2);
#endif // APP_DEBUG
	
	//DispDial();
	/*if (0 != (ret=CommDial(DM_DIAL))) {
		showCommError(ret);
		CommOnHook(FALSE);
		return -1;
	}*/
	//LOG_PRINTF("Dialing successful");
	//SET_POSParamaters(&myParams,0);
	
	sockfd = HttpCreate();
	if (sockfd < 0) {
		//DispErrMsg("Check Connection", "HTTP library init failed", 10, DERR_BEEP);
		return -1;
	}
	//logd(("socfd=%d", sockfd));


	/*memset(params, 0, sizeof(params));
	params[0] =  (NULL != strstr(hostURL, "http://")) ? PROTO_HTTPS : PROTO_HTTP;
	HttpParaCtl(sockfd, HTTP_CMD_SET_PROTO, params, 1);*/
	
	memset(params, 0, sizeof(params));
	
	params[0] = (NULL != strstr(hostURL, "https://")) ? PROTO_HTTPS : PROTO_HTTP;
	//logTrace("Is SSL: %d", params[0]);
	HttpParaCtl(sockfd, HTTP_CMD_SET_PROTO, params, 1);

	memset(params, 0, sizeof(params));
	strcat(params, "60");	
	HttpParaCtl(sockfd, HTTP_CMD_SET_TIMEOUT, params, strlen(params));

	memset(params, 0, sizeof(params));
	params[0] = CERT_ISSUER;
	params[1] = CERT_TIME;
	params[2] = CERT_BAD;
	params[3] = CERT_SELF_SIGN;
	params[4] = CERT_CA_ISSUER;
	HttpParaCtl(sockfd, HTTP_CMD_SET_CERT_REASON, params, 5);

	if (headers && (header_len > 0)) {
		memset(params, 0, sizeof(params));
		i = 0;
		for (; i < header_len; i++) {
			strcat(params, headers[i]);
			strcat(params, "\r\n");
		}

		//Adding the serialno
		//if(function == TRAN_TYPE_TMS_GETPROFILE ){
			//LOG_PRINTF("function=%i/%s (function =! TRAN_ACTIVATE_REQUEST)&&(function!=TRAN_ACTIVATION_REQUEST",function,myParams.terminalid);
			//displayMessageWt("TRAN_TYPE_TMS_GETPROFILE-1",myParams.terminalSerial);

			
				/*char SN[15];
				ReadSN((unsigned char *)SN);
				memset(tmp,0x00,sizeof(tmp));
				sprintf(tmp,"serial: %s",SN);
			//	displayMessageWt("TRAN_TYPE_TMS_GETPROFILE-2",tmp);
				strcat(params, tmp);
				strcat(params, "\r\n");*/

			/*memset(tmp,0x00,sizeof(tmp));
			sprintf(tmp,"appversion: %s","1");//
			strcat(params, tmp);
			strcat(params, "\r\n");*/
				
			memset(tmp,0x00,sizeof(tmp));
			sprintf(tmp,"brand: %s","PAX");
			strcat(params, tmp);
			strcat(params, "\r\n");
			
			memset(tmp,0x00,sizeof(tmp));
			sprintf(tmp,"model: %s","S90");//
			strcat(params, tmp);
			strcat(params, "\r\n");
		//}

		//displayMessageWt("params",params);
		//getkey();

		//ShowLogs(1,"params=%s",params);
		/*}else{
			LOG_PRINTF("function=%i ::: (function == TRAN_ACTIVATE_REQUEST)&&(function==TRAN_ACTIVATION_REQUEST",function);
		}*/
		HttpParaCtl(sockfd, HTTP_CMD_ADD_NEW_FIELDS, params, strlen(params));
	}

	memset(params, 0, sizeof(params));
	strcpy(params, "application/json");
	HttpParaCtl(sockfd, HTTP_CMD_SET_CONTENT_TYPE, params, strlen(params));
		
	
	//displayMessageWt("hostURL",hostURL);
	//displayMessageWt("postData",);

//,sIP,ucPort
	//DelayMs(200);
	ShowLogs(1,"sendHttpRequest: sending");
	//DispSend();sendHttpRequest

	sl = 0; 
	if (httpMethod == HTTP_GET) {
		//displayMessageWt("httpMethod","HTTP_GET");
		//memset(params, 0, sizeof(params));
		//strcpy(params, "application/json\r\n");
		//sprintf(params, "application/json\r\nserial:%s\r\nbrand:5\r\n",);
		//HttpParaCtl(sockfd, HTTP_CMD_SET_CONTENT_TYPE, params, strlen(params));

		//displayMessageWt("sendHttpRequest: before post",hostURL);
		ShowLogs(1,"sendHttpRequest: before post");
		ShowLogs(1,hostURL);
		
		ScrCls();
		ScrFontSet(1);
		ScrPrint(0,3,0,"     Sending...");
		sl = HttpGet(sockfd, hostURL);
		ShowLogs(1,"sendHttpRequest: After HttpGet");
	} else {
		
		//memset(params, 0, sizeof(params));
		//strcpy(params, "application/xml");
		//HttpParaCtl(sockfd, HTTP_CMD_SET_CONTENT_TYPE, params, strlen(params));

		ShowLogs(1,"sendHttpRequest: before post");
		ShowLogs(1,hostURL);
		ShowLogs(1,postData);
		//displayMessageWt("HTTP_POST",postData);
		
		ScrCls();
		ScrFontSet(1);
		ScrPrint(0,3,0,"     Sending...");
		sl = HttpPost(sockfd, hostURL, postData, post_data_len);
		ShowLogs(1,"sendHttpRequest: After HttpPost");
	}
ShowLogs(1,"SL: After HttpPost %d",sl);
	if (sl<0) {
		/*
		Net_Close(iNSRet);
					ScrCls();
					ScrFontSet(1);
					ScrPrint(0,0,0,"ERROR SENDING...");
					DelayMs(2000);
					ScrFontSet(0);
		*/
		ShowLogs(1,"HTTP Send failed = %d", sl);
		//displayMessageWt("HTTP Send failed","HttpClose");
		//showCommError(sl);
		HttpClose(sockfd);
		SEND_SUCCESSFUL=0;
		//CommOnHook(FALSE);
		return -1;
	} else {
		memset(tmp,0x00,sizeof(tmp));
		sprintf(tmp,"HTTPSend status = %d", sl);
		//displayMessageWt("send successful",tmp);
		SEND_SUCCESSFUL=1;
	}

	ScrCls();
	ScrFontSet(1);
	ScrPrint(0,3,0,"     Receiving...");
	//DispReceive();
	ShowLogs(1,"receiving\n");

	while (1) {
		sl = HttpRecvContent(sockfd, temp, sizeof(temp) - 1);
		
		memset(tmp,0x00,sizeof(tmp));
		sprintf(tmp,"HttpRecv = %d", sl);
		
		//displayMessageWt(tmp,temp);
		if (sl>0) {
			ShowLogs(1,"Received:%s, Length: %d", temp, sl);
			chunk->memory = realloc(chunk->memory, (chunk->size + sl + 1) * sizeof(char));
			memset(chunk->memory + chunk->size, 0, (sl + 1) * sizeof(char));
			memcpy(chunk->memory + chunk->size, temp, sl);
			chunk->size += sl;
			chunk->memory[chunk->size + sl + 1] = '\0';
		}
		else if (sl == ERR_HTTP_NO_CONTENT) {
			//displayMessageWt("HTTP RCV CONTENT","ERR_HTTP_NO_CONTENT");
			ShowLogs(1,"EOF");
			//ShowLogs(1,"EOFB");
			SEND_SUCCESSFUL=2;
			break;
		}
		else {
			//ShowLogs(1,"recv failed");
			ShowLogs(1,"Check Connection, Connection Failed");
			//displayMessageWt("Check Connection, Connection Failed","RCV Connection failed");
			HttpClose(sockfd);
			//CommOnHook(FALSE);
			return -1;
		}
	}

	HttpClose(sockfd);
	//CommOnHook(FALSE);
	
	return 0;
}


int PostFlutterWaveNotification()
{
		int    iRet=1,iSize=102400,iTimeoutNum;
		int	   ii,len;
		int ContentLength=0;
		char DebugMessage[50] = {0};
		unsigned char sAPN[50] ={0}; 
		unsigned char sUser[50] = {0};
		unsigned char sPwd[50] ={0};
		char sIP[20]= {0};
		int pos,ucPort,iNSRet;
		MemoryStruct chunk = { 0 };
		const char* headers[3] = {"Content-Type: application/json", "Accept: application/json", "Authorization: dGtfZFh4YXhXemxHYTo="};
		char RequestData[800]={0};
		char RcvData[2000]={0};
		char temp[50]={0};
		char maskedPan[50]={0};
		//POSParamaters  MyLocalDevice = {0}; 
		//SET_POSParamaters(&MyLocalDevice,0);
			
		UtilGetEnv("coapn", sAPN);
		UtilGetEnv("cosubnet", sUser);
		UtilGetEnv("copwd", sPwd);
		
		//DisplayStrMiddle("Connecting...",3);

		ScrCls();
		textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "Notification");
		textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
		textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


		WlInit(NULL);
		//APP_printf("WlInit=%d\r\n", ret);
		WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
		//APP_printf("WlPppLogin=%d\r\n", ret);
		WlPppCheck();
					
		PubStrLower((char *)sPwd);
		PubStrLower((char *)sUser);
			
		sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
		//PTransaction=0;
		
		//DisplayStrMiddle("Connecting...",3);
		iNSRet = WirelessDial(sAPN,sUser,sPwd);

		if (iNSRet < 0)
		{
			memset(DebugMessage,0,sizeof(DebugMessage));
			sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
			ScrCls();
			ScrGotoxy(0,0);
			Lcdprintf("%i->%s",ii,DebugMessage);
			getkey();
			ScrFontSet(0);
			//ShowLogs(1,DebugMessage);
			return -2;
		}

		MaskAllPan(glProcInfo.stTranLog.szPan, maskedPan);
		
		len=0;
		len += sprintf(&RequestData[len], "{\"source\":\"%s\",", "ARCA");

		memset(temp,0x00,sizeof(temp));	
		UtilGetEnv("merName", temp);	
		len += sprintf(&RequestData[len], "\"customername\":\"%s\",", temp);
		len += sprintf(&RequestData[len], "\"customerphone\":\"%s\",", "08037719967");
		len += sprintf(&RequestData[len], "\"id\":\"%s\",", AgentPOSID);

		if(strlen(glProcInfo.stTranLog.szRRN)>0)
			len += sprintf(&RequestData[len], "\"rrn\":\"%s\",", glProcInfo.stTranLog.szRRN);
		else
			len += sprintf(&RequestData[len], "\"rrn\":\"%s\",", "00000000000");

		UtilGetEnvEx("tid", temp);
		len += sprintf(&RequestData[len], "\"terminalid\":\"%s\",", temp);
		len += sprintf(&RequestData[len], "\"fee\":\"%s\",", "20.00");
		len += sprintf(&RequestData[len], "\"pan\":\"%s\",", maskedPan);
		//len += sprintf(&RequestData[len], "\"amount\":%ld,", atol("1.00"));
		len += sprintf(&RequestData[len], "\"amount\":%.2f,", atol(glProcInfo.stTranLog.szAmount)/100.0 );
		len += sprintf(&RequestData[len], "\"currencycode\":\"%s\",", "NGN");
		len += sprintf(&RequestData[len], "\"cardholder\" : \"%s\",", glProcInfo.stTranLog.szHolderName);
		len += sprintf(&RequestData[len], "\"expiry\" : \"%s\",", glProcInfo.stTranLog.szExpDate);


		if(strlen(glProcInfo.stTranLog.szAuthCode)>0)
			len += sprintf(&RequestData[len], "\"authcode\" : \"%s\",", glProcInfo.stTranLog.szAuthCode);
		else
			len += sprintf(&RequestData[len], "\"authcode\" : \"%s\",", "000000");

		len += sprintf(&RequestData[len], "\"cardtype\" : \"%s\",", glProcInfo.stTranLog.szAppLabel);
		len += sprintf(&RequestData[len], "\"refcode\":\"%s\",",  glRecvPack.szSTAN);
		len += sprintf(&RequestData[len], "\"type\": \"%s\",", "Purchase");
		len += sprintf(&RequestData[len], "\"requestdate\":\"%s\",", glProcInfo.stTranLog.szDateTime);
		len += sprintf(&RequestData[len], "\"responsedate\":\"%s\",", glProcInfo.stTranLog.szDateTime);

		if(strlen(glProcInfo.stTranLog.szRspCode)>0)
			len += sprintf(&RequestData[len], "\"responsecode\":\"%s\",", glProcInfo.stTranLog.szRspCode);
		else
			len += sprintf(&RequestData[len], "\"responsecode\":\"%s\",", "96");

		memset(temp,0x00,sizeof(temp));	
		getResponse(glProcInfo.stTranLog.szRspCode, temp);
		PubStrUpper(temp);
		len += sprintf(&RequestData[len], "\"responsemessage\":\"%s\",", temp);

		if(atoi(glProcInfo.stTranLog.szRspCode)==0)
			len += sprintf(&RequestData[len], "\"status\":\"%s\"}","success"); 
		else
			len += sprintf(&RequestData[len], "\"status\":\"%s\"}","failed"); 

/*

		ShowLogs(1,"IN - PostFlutterWaveNotification");

		len=0;
		len += sprintf(&RequestData[len], "{\"source\":\"%s\",", "ARCA");
		len += sprintf(&RequestData[len], "\"customername\":\"%s\",", "Merchant");
		len += sprintf(&RequestData[len], "\"customerphone\":\"%s\",", "08037719967");
		len += sprintf(&RequestData[len], "\"id\":\"%s\",", "1234567");
		len += sprintf(&RequestData[len], "\"rrn\":\"%s\",", "12309867");
		len += sprintf(&RequestData[len], "\"terminalid\":\"%s\",", "2101JO65");
		len += sprintf(&RequestData[len], "\"fee\":\"%s\",", "20.00");
		len += sprintf(&RequestData[len], "\"pan\":\"%s\",", "539983****7682");
		len += sprintf(&RequestData[len], "\"amount\":%ld,", atol("1.00"));
		len += sprintf(&RequestData[len], "\"currencycode\":\"%s\",", "NGN");
		len += sprintf(&RequestData[len], "\"cardholder\" : \"%s\",", "SEGSAN");
		len += sprintf(&RequestData[len], "\"expiry\" : \"%s\",", "2403");
		len += sprintf(&RequestData[len], "\"authcode\" : \"%s\",", "42309867");
		len += sprintf(&RequestData[len], "\"cardtype\" : \"%s\",", "Debit Mastercard");
		len += sprintf(&RequestData[len], "\"refcode\":\"%s\",",  "987453276");
		len += sprintf(&RequestData[len], "\"type\": \"%s\",", "Purchase");
		len += sprintf(&RequestData[len], "\"requestdate\":\"%s\",", "2021-11-06 13:15:42");
		len += sprintf(&RequestData[len], "\"responsedate\":\"%s\",", "2021-11-06 13:15:42");
		len += sprintf(&RequestData[len], "\"responsecode\":\"%s\",", "00");
		len += sprintf(&RequestData[len], "\"responsemessage\":\"%s\",", "Approved");
		len += sprintf(&RequestData[len], "\"status\":\"%s\"}","success");*/


		if (sendHttpRequest(HTTP_POST, "https://flutterwavestagingv2.com/flwvpos/api/pos/notification", RequestData, len, headers, 3, &chunk) == 0) {
			//logd((""));
			ShowLogs(1,"Success Response:%s", chunk.memory);
			strcpy(RcvData,chunk.memory);
			//displayMessageWt("response",RcvData);
			
			//if(function == TRAN_TYPE_INTERSWITCH){
				//char Test[200];
				//memset(Test,0x00,sizeof(Test));
				//strncpy(Test,&RcvData[120],120);	
				////displayMessageWt("RcvData>>1",Test);

				//memset(Test,0x00,sizeof(Test));
				//strncpy(Test,&RcvData[240],120);	
				//displayMessageWt("RcvData>>2",Test);
			//}

		
			if (chunk.memory) {
				free(chunk.memory);
			}
			
			ShowLogs(1,"IN - PostFlutterWaveNotification - NetCloseSocket(iNSRet)");
			NetCloseSocket(iNSRet);

			ShowLogs(1,"IN - PostFlutterWaveNotification - return 0");
			//Net_Close(iNSRet);
			return 0;
		}else{
			NetCloseSocket(iNSRet);
			//Net_Close(iNSRet);
			return -1;
		}
}


void GetFlutterDate(char * flutwdate)
{
	 unsigned char time[7];
	 char year[5]={0},month[3]={0},day[3]={0},hour[3]={0},minute[3]={0},second[3]={0};

	 GetTime(time);
	 
	 //////////////////////////////

	/* memset(cTrDate,0,sizeof(cTrDate));
	 memset(cTrTime,0,sizeof(cTrTime));
	 memset(cTrFullTime,0,sizeof(cTrFullTime));*/

	 //For ISO
	 memset(flutwdate,0x00,sizeof(flutwdate));

 //	sprintf(cTrDate, "%c%c/%c%c",(time[1] >> 4) + 0x30, (time[1] & 0x0f) + 0x30,(time[2] >> 4) + 0x30, (time[2] & 0x0f) + 0x30);
 //	sprintf(cTrFullDate, "%c%c/%c%c/%c%c",(time[0] >> 4) + 0x30,(time[0] & 0x0f) + 0x30,(time[1] >> 4) + 0x30, (time[1] & 0x0f) + 0x30,(time[2] >> 4) + 0x30, (time[2] & 0x0f) + 0x30);

	//sprintf(cTrTime,"%c%c:%c%c",(time[3] >> 4) + 0x30, (time[3] & 0x0f) + 0x30,(time[4] >> 4) + 0x30, (time[4] & 0x0f) + 0x30);
	//sprintf(cTrFullTime,"%c%c:%c%c:%c%c",(time[3] >> 4) + 0x30, (time[3] & 0x0f) + 0x30,(time[4] >> 4) + 0x30, (time[4] & 0x0f) + 0x30,(time[5] >> 4) + 0x30, (time[5] & 0x0f) + 0x30);

	/////
	sprintf(year, "%c%c",(time[0] >> 4) + 0x30,(time[0] & 0x0f) + 0x30 );//4
	sprintf(day,"%c%c",(time[2] >> 4) + 0x30, (time[2] & 0x0f) + 0x30);//2
	sprintf(month, "%c%c",(time[1] >> 4) + 0x30, (time[1] & 0x0f) + 0x30 );//2

	sprintf(hour, "%c%c",(time[3] >> 4) + 0x30, (time[3] & 0x0f) + 0x30 );//2
	sprintf(minute, "%c%c",(time[4] >> 4) + 0x30, (time[4] & 0x0f) + 0x30 );//2
	sprintf(second, "%c%c",(time[5] >> 4) + 0x30, (time[5] & 0x0f) + 0x30 );//2

//	2021-10-04 23:45:51

	sprintf(flutwdate, "20%s-%s-%s %s:%s:%s",year,month,day,hour,minute,second);

	memset(RefDate,0x00,sizeof(RefDate));
	sprintf(RefDate, "20%s%s%s%s%s%s",year,month,day,hour,minute,second);
	
	ShowLogs(1, "flutwdate%s",flutwdate); 

	 //////////////////////////////
	 
}



int PostCellulantNotificationFCMB()
{
		int    iRet=1,iSize=102400,iTimeoutNum;
		int	   ii,len;
		long Amount;
		char PortNo[5];
		int ContentLength=0;
		char DebugMessage[50] = {0};
		unsigned char sAPN[50] ={0};
		unsigned char sUser[50] = {0};
		unsigned char sPwd[50] ={0};
		char sIP[20]= {0};
		int pos,ucPort,iNSRet;
		MemoryStruct chunk = { 0 };
		char RequestData[1000]={0};
		char RequestData_transaction[1000]={0};
		char RcvData[2000]={0};
		char temp[50]={0};
		char herder_tid[12]={0};
		char maskedPan[25] = {'\0'};
		char fdate[50]={0};
		//char temp[50]={0};	
		
		//POSParamaters  MyLocalDevice = {0}; 
		//SET_POSParamaters(&MyLocalDevice,0);

		memset(temp,0x00,sizeof(temp));
		UtilGetEnvEx("tid", temp);

		if(strncmp(temp, "2214", 4) != 0)return 0;
			
		UtilGetEnv("coapn", sAPN);
		UtilGetEnv("cosubnet", sUser);
		UtilGetEnv("copwd", sPwd);
		
		//DisplayStrMiddle("Connecting...",3);

		ScrCls();
		textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "Notification");
		textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
		textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


		
		WlInit(NULL);
		//APP_printf("WlInit=%d\r\n", ret);
		WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
		//APP_printf("WlPppLogin=%d\r\n", ret);
		WlPppCheck();
					
		PubStrLower((char *)sPwd);
		PubStrLower((char *)sUser);
			
		sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
		//PTransaction=0;
		
		//DisplayStrMiddle("Connecting...",3);
		iNSRet = WirelessDial(sAPN,sUser,sPwd);

		if (iNSRet < 0)
		{
			memset(DebugMessage,0,sizeof(DebugMessage));
			sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
			ScrCls();
			ScrGotoxy(0,0);
			Lcdprintf("%i->%s",ii,DebugMessage);
			getkey();
			ScrFontSet(0);
			//ShowLogs(1,DebugMessage);
			return -2;
		}

		//memset(ucTmpSend, 0, sizeof(ucTmpSend));
		memset(PortNo,0,sizeof(PortNo));
		sprintf(PortNo,"%d",ucPort);
					
		MaskAllPan(glProcInfo.stTranLog.szPan, maskedPan);

		len=0;
		/*
		len += sprintf(&RequestData[len], "{\"Reference\":\"%s\",", "EP-10101");
		len += sprintf(&RequestData[len], "\"Amount\":\"%.2f\",", atol(glProcInfo.stTranLog.szAmount)/100.0);
		len += sprintf(&RequestData[len], "\"Currency\":\"%s\",", "NGN");
		len += sprintf(&RequestData[len], "\"Type\" : \"%s\",", "INVOICE");
		//len += sprintf(&RequestData[len], "\"TransactionReference\" : \"%s\",", glRecvPack.szSTAN);

		memset(temp,0x00,sizeof(temp));	
		GetFlutterDate(temp);
		memset(temp,0x00,sizeof(temp));	

		UtilGetEnvEx("tid", temp);
		if(strlen(glRecvPack.szSTAN)>0)
			len += sprintf(&RequestData[len], "\"TransactionReference\":\"%s%s%s%s\",",temp,glRecvPack.szSTAN,glProcInfo.stTranLog.szRRN,RefDate);
		else
			len += sprintf(&RequestData[len], "\"TransactionReference\":\"%s\",", "%s00000000000%s%s%s",temp,glProcInfo.stTranLog.szRRN,RefDate);


		if(strlen(glProcInfo.stTranLog.szRRN)>0)
			len += sprintf(&RequestData[len], "\"RetrievalReferenceNumber\":\"%s\",", glProcInfo.stTranLog.szRRN);
		else
			len += sprintf(&RequestData[len], "\"RetrievalReferenceNumber\":\"%s\",", "00000000000");

		//len += sprintf(&RequestData[len], "\"RetrievalReferenceNumber\" : \"%s\",", glProcInfo.stTranLog.szRRN);
		len += sprintf(&RequestData[len], "\"MaskedPAN\" : \"%s\",", maskedPan);
		len += sprintf(&RequestData[len], "\"CardScheme\":\"%s\",", glProcInfo.stTranLog.szAppLabel);
		len += sprintf(&RequestData[len], "\"CustomerName\": \"%s\",", glProcInfo.stTranLog.szHolderName);


		if(strlen(glProcInfo.stTranLog.szRspCode)>0)
			len += sprintf(&RequestData[len], "\"StatusCode\":\"%s\",", glProcInfo.stTranLog.szRspCode);
		else
			len += sprintf(&RequestData[len], "\"StatusCode\":\"%s\",", "96");


		//len += sprintf(&RequestData[len], "\"StatusCode\":\"%s\",", glProcInfo.stTranLog.szRspCode);

		getResponse(glProcInfo.stTranLog.szRspCode, temp);
		PubStrUpper(temp);
		len += sprintf(&RequestData[len], "\"responsemessage\":\"%s\",", temp);

		memset(temp,0x00,sizeof(temp));	
		getResponse(glProcInfo.stTranLog.szRspCode, temp);
		PubStrUpper(temp);

		len += sprintf(&RequestData[len], "\"StatusDescription\":\"%s\",", temp);
		//len += sprintf(&RequestData[len], "\"PaymentDate\":\"%s\"}",glProcInfo.stTranLog.szDateTime); 

		GetFlutterDate(fdate);
		len += sprintf(&RequestData[len], "\"PaymentDate\":\"%s\",",fdate); 
		len += sprintf(&RequestData[len], "\"url\": \"%s\",","http://41.73.252.230:28080/SmartWallet/Proxy/notification");
		len += sprintf(&RequestData[len], "\"headers\": {\"Content-Type\" :\"%s\",","application/json");
		len += sprintf(&RequestData[len], "\"Authorization\": \"Basic %s\",","Q0VMTCE6YzNsbHVsNG50ISFAIw==");
													//   "Authorization: Basic Q0VMTCE6YzNsbHVsNG50ISFAIw=="

		memset(temp,0x00,sizeof(temp));	
		UtilGetEnvEx("tid", temp);
		len += sprintf(&RequestData[len], "\"TerminalId\": \"%s\"}}",temp);
		*/


	len += sprintf(&RequestData[len], "{\"reference\":\"%s%s%s%s\",",temp,glRecvPack.szSTAN,glProcInfo.stTranLog.szRRN,RefDate);
len += sprintf(&RequestData[len], "\"transactionType\": \"Callhome\",");
		GetFlutterDate(fdate);
		len += sprintf(&RequestData[len], "\"transactionDate\":\"%s\",",fdate); 

		len += sprintf(&RequestData[len], "\"responseCode\": \"06\",");
		
		memset(temp,0x00,sizeof(temp));	
		UtilGetEnvEx("tid", temp);
		len += sprintf(&RequestData[len], "\"terminalId\": \"%s\",",temp);


		len += sprintf(&RequestData[len], "\"terminalInfo\": {");
		len += sprintf(&RequestData[len], "\"serialNumber\": \"22141111\",");
		len += sprintf(&RequestData[len], "\"applicationVersion\": \"2.2.61\",");
		len += sprintf(&RequestData[len], "\"ptsp\": \"E-TOP\",");
		len += sprintf(&RequestData[len], "\"batteryStatus\": \"here\",");
		len += sprintf(&RequestData[len], "\"paperStatus\": \"there\",");
		len += sprintf(&RequestData[len], "\"commsType\": \"GPRS\",");
		len += sprintf(&RequestData[len], "\"commsDetails\": \"621300328038986\"},");
		len += sprintf(&RequestData[len], "\"merchantDetails\": {");
		len += sprintf(&RequestData[len], "\"location\" : \"Ikorodu\"},");
		len += sprintf(&RequestData[len], "\"amount\": \"0\",");
		len += sprintf(&RequestData[len], "\"transactionFee\": \"0\",");
		len += sprintf(&RequestData[len], "\"processingFee\": \"0\",");
		len += sprintf(&RequestData[len], "\"callbackUrl\":");
		len += sprintf(&RequestData[len], "\"https://posmonitor.fcmb.com/\",");
		len += sprintf(&RequestData[len], "\"subscriptionReference\": \"86\",");
		len += sprintf(&RequestData[len], "\"reversal\": \"false\"}");
		
		sprintf(herder_tid, "\"terminalId\": \"%s\"",temp);
		const char* headers[5] = {"Content-Type: application/json", "Accept: application/json", "Authorization: Basic Q0VMTCE6YzNsbHVsNG50ISFAIw==","url:https://posmonitor.fcmb.com/api/hardwareevent.php",herder_tid};
		

		
		if (sendHttpRequest(HTTP_POST, "http://173.230.138.119/test.php", RequestData, len, headers, 5, &chunk) == 0) {
			//logd((""));
			
			strcpy(RcvData,chunk.memory);
					
				if (chunk.memory) {
					free(chunk.memory);
				}
				
/*
				len=0;
				len += sprintf(&RequestData_transaction[len], "{\"transactionReference\":\"%s%s%s%s\",",temp,glRecvPack.szSTAN,glProcInfo.stTranLog.szRRN,RefDate);
				len += sprintf(&RequestData_transaction[len], "\"reference\": \"%s\",",glRecvPack.szSTAN);
				len += sprintf(&RequestData_transaction[len], "\"transactionType\": \"Purchase\",");
				
				len += sprintf(&RequestData_transaction[len], "\"transactionDate\": \"%s\",",fdate);

				if(strlen(glProcInfo.stTranLog.szRspCode)>0)
			len += sprintf(&RequestData_transaction[len], "\"responseCode\":\"%s\",", glProcInfo.stTranLog.szRspCode);
		else
			len += sprintf(&RequestData_transaction[len], "\"responseCode\":\"%s\",", "96");

				

						memset(temp,0x00,sizeof(temp));	
						UtilGetEnvEx("tid", temp);
		


				len += sprintf(&RequestData_transaction[len], "\"terminalId\": \"%s\",",temp);
				len += sprintf(&RequestData_transaction[len], "\"pan\": \"%s\",",maskedPan);
				len += sprintf(&RequestData_transaction[len], "\"amount\":\"%.2f\",", atol(glProcInfo.stTranLog.szAmount)/100.0);
				len += sprintf(&RequestData_transaction[len], "\"cardExpiry\": \"00\",");
				len += sprintf(&RequestData_transaction[len], "\"transactionFee\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"processingFee\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"retrievalReferenceNumber\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"authCode\": \"%s\",",glProcInfo.stTranLog.szRRN);
				len += sprintf(&RequestData_transaction[len], "\"merchantCode\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"reversal\": \"false\",");
				len += sprintf(&RequestData_transaction[len], "\"stan\": \"0\"}");


		NetCloseSocket(iNSRet);
				
		WlInit(NULL);
		//APP_printf("WlInit=%d\r\n", ret);
		WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
		//APP_printf("WlPppLogin=%d\r\n", ret);
		WlPppCheck();
					
		PubStrLower((char *)sPwd);
		PubStrLower((char *)sUser);
			
		sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
		//PTransaction=0;
		
		//DisplayStrMiddle("Connecting...",3);
		iNSRet = WirelessDial(sAPN,sUser,sPwd);

		if (iNSRet < 0)
		{
			//memset(DebugMessage,0,sizeof(DebugMessage));
			//sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
			ScrCls();
			ScrGotoxy(0,0);
			Lcdprintf("%i->%s",ii,DebugMessage);
			getkey();
			ScrFontSet(0);
			//ShowLogs(1,DebugMessage);
			return -2;
		}

				const char* headers_transaction[5] = {"Content-Type: application/json", "Accept: application/json", "Authorization: Basic Q0VMTCE6YzNsbHVsNG50ISFAIw==","url:https://posmonitor.fcmb.com/api/transactionevent.php",herder_tid};
				sendHttpRequest(HTTP_POST, "http://173.230.138.119/test.php", RequestData_transaction, len, headers_transaction, 5, &chunk);
		*/
			NetCloseSocket(iNSRet);
			

			//Net_Close(iNSRet);
			return 0;
		}else{
			NetCloseSocket(iNSRet);
			//Net_Close(iNSRet);
			return -1;
		}
}


int PostTransactionNotificationFCMB()
{
		int    iRet=1,iSize=102400,iTimeoutNum;
		int	   ii,len;
		long Amount;
		char PortNo[5];
		int ContentLength=0;
		char DebugMessage[50] = {0};
		unsigned char sAPN[50] ={0};
		unsigned char sUser[50] = {0};
		unsigned char sPwd[50] ={0};
		char sIP[20]= {0};
		int pos,ucPort,iNSRet;
		MemoryStruct chunk = { 0 };
		char RequestData[1000]={0};
		char RequestData_transaction[1000]={0};
		char RcvData[2000]={0};
		char temp[50]={0};
		char herder_tid[12]={0};
		char maskedPan[25] = {'\0'};
		char fdate[50]={0};
		//char temp[50]={0};	
		
		//POSParamaters  MyLocalDevice = {0}; 
		//SET_POSParamaters(&MyLocalDevice,0);

		memset(temp,0x00,sizeof(temp));
		UtilGetEnvEx("tid", temp);

		if(strncmp(temp, "2214", 4) != 0)return 0;
			
		UtilGetEnv("coapn", sAPN);
		UtilGetEnv("cosubnet", sUser);
		UtilGetEnv("copwd", sPwd);
		
		//DisplayStrMiddle("Connecting...",3);

		ScrCls();
		textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "Notification");
		textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
		textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


		
		WlInit(NULL);
		//APP_printf("WlInit=%d\r\n", ret);
		WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
		//APP_printf("WlPppLogin=%d\r\n", ret);
		WlPppCheck();
					
		PubStrLower((char *)sPwd);
		PubStrLower((char *)sUser);
			
		sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
		//PTransaction=0;
		
		//DisplayStrMiddle("Connecting...",3);
		iNSRet = WirelessDial(sAPN,sUser,sPwd);

		if (iNSRet < 0)
		{
			memset(DebugMessage,0,sizeof(DebugMessage));
			sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
			ScrCls();
			ScrGotoxy(0,0);
			Lcdprintf("%i->%s",ii,DebugMessage);
			getkey();
			ScrFontSet(0);
			//ShowLogs(1,DebugMessage);
			return -2;
		}

		//memset(ucTmpSend, 0, sizeof(ucTmpSend));
		memset(PortNo,0,sizeof(PortNo));
		sprintf(PortNo,"%d",ucPort);
					
		MaskAllPan(glProcInfo.stTranLog.szPan, maskedPan);

		len=0;
		/*
		len += sprintf(&RequestData[len], "{\"Reference\":\"%s\",", "EP-10101");
		len += sprintf(&RequestData[len], "\"Amount\":\"%.2f\",", atol(glProcInfo.stTranLog.szAmount)/100.0);
		len += sprintf(&RequestData[len], "\"Currency\":\"%s\",", "NGN");
		len += sprintf(&RequestData[len], "\"Type\" : \"%s\",", "INVOICE");
		//len += sprintf(&RequestData[len], "\"TransactionReference\" : \"%s\",", glRecvPack.szSTAN);

		memset(temp,0x00,sizeof(temp));	
		GetFlutterDate(temp);
		memset(temp,0x00,sizeof(temp));	

		UtilGetEnvEx("tid", temp);
		if(strlen(glRecvPack.szSTAN)>0)
			len += sprintf(&RequestData[len], "\"TransactionReference\":\"%s%s%s%s\",",temp,glRecvPack.szSTAN,glProcInfo.stTranLog.szRRN,RefDate);
		else
			len += sprintf(&RequestData[len], "\"TransactionReference\":\"%s\",", "%s00000000000%s%s%s",temp,glProcInfo.stTranLog.szRRN,RefDate);


		if(strlen(glProcInfo.stTranLog.szRRN)>0)
			len += sprintf(&RequestData[len], "\"RetrievalReferenceNumber\":\"%s\",", glProcInfo.stTranLog.szRRN);
		else
			len += sprintf(&RequestData[len], "\"RetrievalReferenceNumber\":\"%s\",", "00000000000");

		//len += sprintf(&RequestData[len], "\"RetrievalReferenceNumber\" : \"%s\",", glProcInfo.stTranLog.szRRN);
		len += sprintf(&RequestData[len], "\"MaskedPAN\" : \"%s\",", maskedPan);
		len += sprintf(&RequestData[len], "\"CardScheme\":\"%s\",", glProcInfo.stTranLog.szAppLabel);
		len += sprintf(&RequestData[len], "\"CustomerName\": \"%s\",", glProcInfo.stTranLog.szHolderName);


		if(strlen(glProcInfo.stTranLog.szRspCode)>0)
			len += sprintf(&RequestData[len], "\"StatusCode\":\"%s\",", glProcInfo.stTranLog.szRspCode);
		else
			len += sprintf(&RequestData[len], "\"StatusCode\":\"%s\",", "96");


		//len += sprintf(&RequestData[len], "\"StatusCode\":\"%s\",", glProcInfo.stTranLog.szRspCode);

		getResponse(glProcInfo.stTranLog.szRspCode, temp);
		PubStrUpper(temp);
		len += sprintf(&RequestData[len], "\"responsemessage\":\"%s\",", temp);

		memset(temp,0x00,sizeof(temp));	
		getResponse(glProcInfo.stTranLog.szRspCode, temp);
		PubStrUpper(temp);

		len += sprintf(&RequestData[len], "\"StatusDescription\":\"%s\",", temp);
		//len += sprintf(&RequestData[len], "\"PaymentDate\":\"%s\"}",glProcInfo.stTranLog.szDateTime); 

		GetFlutterDate(fdate);
		len += sprintf(&RequestData[len], "\"PaymentDate\":\"%s\",",fdate); 
		len += sprintf(&RequestData[len], "\"url\": \"%s\",","http://41.73.252.230:28080/SmartWallet/Proxy/notification");
		len += sprintf(&RequestData[len], "\"headers\": {\"Content-Type\" :\"%s\",","application/json");
		len += sprintf(&RequestData[len], "\"Authorization\": \"Basic %s\",","Q0VMTCE6YzNsbHVsNG50ISFAIw==");
													//   "Authorization: Basic Q0VMTCE6YzNsbHVsNG50ISFAIw=="

		memset(temp,0x00,sizeof(temp));	
		UtilGetEnvEx("tid", temp);
		len += sprintf(&RequestData[len], "\"TerminalId\": \"%s\"}}",temp);
		*/


	


		len += sprintf(&RequestData_transaction[len], "{\"transactionReference\":\"%s%s%s%s\",",temp,glRecvPack.szSTAN,glProcInfo.stTranLog.szRRN,RefDate);
				len += sprintf(&RequestData_transaction[len], "\"reference\": \"%s\",",glRecvPack.szSTAN);
				len += sprintf(&RequestData_transaction[len], "\"transactionType\": \"Purchase\",");
				
				len += sprintf(&RequestData_transaction[len], "\"transactionDate\": \"%s\",",fdate);

				if(strlen(glProcInfo.stTranLog.szRspCode)>0)
			len += sprintf(&RequestData_transaction[len], "\"responseCode\":\"%s\",", glProcInfo.stTranLog.szRspCode);
		else
			len += sprintf(&RequestData_transaction[len], "\"responseCode\":\"%s\",", "96");

				

						memset(temp,0x00,sizeof(temp));	
						UtilGetEnvEx("tid", temp);
				len += sprintf(&RequestData_transaction[len], "\"terminalId\": \"%s\",",temp);
				len += sprintf(&RequestData_transaction[len], "\"pan\": \"%s\",",maskedPan);
				len += sprintf(&RequestData_transaction[len], "\"amount\":\"%.2f\",", atol(glProcInfo.stTranLog.szAmount)/100.0);
				len += sprintf(&RequestData_transaction[len], "\"cardExpiry\": \"00\",");
				len += sprintf(&RequestData_transaction[len], "\"transactionFee\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"processingFee\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"retrievalReferenceNumber\":\"%s%s%s%s\",",temp,glRecvPack.szSTAN,glProcInfo.stTranLog.szRRN,RefDate);
				len += sprintf(&RequestData_transaction[len], "\"authCode\": \"%s\",",glProcInfo.stTranLog.szRRN);
				len += sprintf(&RequestData_transaction[len], "\"merchantCode\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"reversal\": \"false\",");
				len += sprintf(&RequestData_transaction[len], "\"ptsp\": \"Arca\",");
				len += sprintf(&RequestData_transaction[len], "\"stan\": \"%s\"}",glRecvPack.szSTAN);
		const char* headers[5] = {"Content-Type: application/json", "Accept: application/json", "Authorization: Basic Q0VMTCE6YzNsbHVsNG50ISFAIw==","url:https://posmonitor.fcmb.com/api/transactionevent.php",herder_tid};
		

		
		if (sendHttpRequest(HTTP_POST, "http://173.230.138.119/test.php", RequestData, len, headers, 5, &chunk) == 0) {
			//logd((""));
			
			strcpy(RcvData,chunk.memory);
					
				if (chunk.memory) {
					free(chunk.memory);
				}
				
/*
				len=0;
				len += sprintf(&RequestData_transaction[len], "{\"transactionReference\":\"%s%s%s%s\",",temp,glRecvPack.szSTAN,glProcInfo.stTranLog.szRRN,RefDate);
				len += sprintf(&RequestData_transaction[len], "\"reference\": \"%s\",",glRecvPack.szSTAN);
				len += sprintf(&RequestData_transaction[len], "\"transactionType\": \"Purchase\",");
				
				len += sprintf(&RequestData_transaction[len], "\"transactionDate\": \"%s\",",fdate);

				if(strlen(glProcInfo.stTranLog.szRspCode)>0)
			len += sprintf(&RequestData_transaction[len], "\"responseCode\":\"%s\",", glProcInfo.stTranLog.szRspCode);
		else
			len += sprintf(&RequestData_transaction[len], "\"responseCode\":\"%s\",", "96");

				

						memset(temp,0x00,sizeof(temp));	
						UtilGetEnvEx("tid", temp);
		


				len += sprintf(&RequestData_transaction[len], "\"terminalId\": \"%s\",",temp);
				len += sprintf(&RequestData_transaction[len], "\"pan\": \"%s\",",maskedPan);
				len += sprintf(&RequestData_transaction[len], "\"amount\":\"%.2f\",", atol(glProcInfo.stTranLog.szAmount)/100.0);
				len += sprintf(&RequestData_transaction[len], "\"cardExpiry\": \"00\",");
				len += sprintf(&RequestData_transaction[len], "\"transactionFee\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"processingFee\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"retrievalReferenceNumber\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"authCode\": \"%s\",",glProcInfo.stTranLog.szRRN);
				len += sprintf(&RequestData_transaction[len], "\"merchantCode\": \"0\",");
				len += sprintf(&RequestData_transaction[len], "\"reversal\": \"false\",");
				len += sprintf(&RequestData_transaction[len], "\"stan\": \"0\"}");


		NetCloseSocket(iNSRet);
				
		WlInit(NULL);
		//APP_printf("WlInit=%d\r\n", ret);
		WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
		//APP_printf("WlPppLogin=%d\r\n", ret);
		WlPppCheck();
					
		PubStrLower((char *)sPwd);
		PubStrLower((char *)sUser);
			
		sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
		//PTransaction=0;
		
		//DisplayStrMiddle("Connecting...",3);
		iNSRet = WirelessDial(sAPN,sUser,sPwd);

		if (iNSRet < 0)
		{
			//memset(DebugMessage,0,sizeof(DebugMessage));
			//sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
			ScrCls();
			ScrGotoxy(0,0);
			Lcdprintf("%i->%s",ii,DebugMessage);
			getkey();
			ScrFontSet(0);
			//ShowLogs(1,DebugMessage);
			return -2;
		}

				const char* headers_transaction[5] = {"Content-Type: application/json", "Accept: application/json", "Authorization: Basic Q0VMTCE6YzNsbHVsNG50ISFAIw==","url:https://posmonitor.fcmb.com/api/transactionevent.php",herder_tid};
				sendHttpRequest(HTTP_POST, "http://173.230.138.119/test.php", RequestData_transaction, len, headers_transaction, 5, &chunk);
		*/
			NetCloseSocket(iNSRet);
			

			//Net_Close(iNSRet);
			return 0;
		}else{
			NetCloseSocket(iNSRet);
			//Net_Close(iNSRet);
			return -1;
		}
}

int PostCelTinTransfer(char data [800])
{
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};
	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	const char* headers[2] = {"Content-Type: application/xml", "Accept: application/xml"};
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char maskedPan[25] = {'\0'};
	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "Transfer - Cellulant");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - PostCellulantNotificationFCMB");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	memset(PortNo,0,sizeof(PortNo));
	sprintf(PortNo,"%d",ucPort);
			
	/*sprintf(RequestData, "POST /%s?%s HTTP/1.1\r\n" \
				"Host: %s:%s\r\n" \
				"Accept: application/xml\r\n" \
				"Content-Type: application/xml\r\n" \
				"Content-Length: %d\r\n\r\n%s\r\n","/SmartWallet/Proxy/SendToBank",data,"41.73.252.236", "28080",strlen(RequestData),RequestData);
	*/
	sprintf(RequestData, "%s?%s","http://41.73.252.230:28080/SmartWallet/Proxy/SendToBank",data);


	ShowLogs(1,"IN - PostCelTinTransfer: %s",RequestData);  

	len=strlen(data);


	if (sendHttpRequest(HTTP_POST, RequestData, data, len, headers, 0, &chunk) == 0) {
		//logd((""));
		ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);
				
		if (chunk.memory) {
			free(chunk.memory);
		}
			
		//NetCloseSocket(iNSRet);

		ShowLogs(1,"PostCelTinTransfer: return 0");

		//function = TRAN_TYPE_TMS_NOTIFICATION;
		ParseJson((char *)RcvData);

		displayMessageWt(Cel_responseCode,Cel_responseMessage);
		
		char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");


		//Net_Close(iNSRet);
		return 0;
	}else{
		char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");
		NetCloseSocket(iNSRet);
		//Net_Close(iNSRet);
		return -1;
	}
}


int GetCelLogin(char pin[15])
{
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};



	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	const char* headers[5] = {"Content-Type: application/json", "Accept: application/json","webkey: 8af5e1092ad24521a4a5353b73132145","accountid: 100002805","Authorization: Basic bWljaGVsa2FsYXZhbmRhQGdtYWlsLmNvbTpkZXZARXRvcA=="};
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char data[100] = {'\0'};
	char Confirmtemp[150]={0};
	int		iResult;
	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "Jamb Validation");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - GetCelPlatformID");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	memset(PortNo,0,sizeof(PortNo));
	sprintf(PortNo,"%d",ucPort);
			
	/*sprintf(RequestData, "POST /%s?%s HTTP/1.1\r\n" \
				"Host: %s:%s\r\n" \
				"Accept: application/xml\r\n" \
				"Content-Type: application/xml\r\n" \
				"Content-Length: %d\r\n\r\n%s\r\n","/SmartWallet/Proxy/SendToBank",data,"41.73.252.236", "28080",strlen(RequestData),RequestData);
	*/

	char walletid[20]={0};

	UtilGetEnvEx("walletid", walletid);
	/*
	{
"billerCode": "jamb",
"customerAccountNumber": "6310390245",
"field1": "UTME"
}*/
	sprintf(data,"{\"billerCode\": \"jamb\",\"customerAccountNumber\": \"%s\",\"field1\": \"UTME\"}",pin);

	

	//sprintf(RequestData, "%s?%s","http://41.73.252.230:28080/SmartWallet/Proxy/NameEnquiry",data);
	sprintf(RequestData, "%s?%s","http://41.73.252.230:28080/SmartWallet/Proxy/Login",data);

	ShowLogs(1,"IN - GetCelLogin: %s",RequestData);  

	len=strlen(data);

if (sendHttpRequest(HTTP_POST, "https://80.88.8.245:9591/api/lookup", data, len, headers, 5, &chunk)==0)
	{			
	//if (sendHttpRequest(HTTP_GET, RequestData, data, len, headers, 0, &chunk) == 0) {
		//logd((""));
		ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);
				
			if (chunk.memory) {
				free(chunk.memory);
			}
			
		ShowLogs(1,"Response:%s / %s", Cel_responseCode,Cel_responseMessage);
		NetCloseSocket(iNSRet);

		
		Gui_ClearScr();
		ParseJson((char *)RcvData);
		sprintf(Confirmtemp,"Name\r\n%s\r\nAmount\r\nNGN %d\r\nAccount No\r\n%s",Cel_AccountName,atol(Cel_BenAmunt)/100,Cel_TinAccountNumber);
		
		iResult = Gui_ShowMsgBox("Jamb Validation", gl_stTitleAttr,RcvData , gl_stCenterAttr, GUI_BUTTON_YandN,25, NULL);
		if(iResult == ERR_USERCANCEL || iResult == GUI_ERR_TIMEOUT){
			ShowLogs(1,"GetBenefAccountNumber - DisplayInfoYN - return 0");
			return 0;
			//return ERR_USERCANCEL;
		}
		if(strcmp(Cel_responseCode, "00") == 0){
			return 1;
		}
		//function = TRAN_TYPE_TMS_NOTIFICATION;
		
		//ShowLogs(1,"Cel_PlatformID = %s",Cel_PlatformID);
		//iResult = Gui_ShowMsgBox("Transfer Info", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN, 15, NULL);
		
		//displayMessageWt(Cel_responseCode,Cel_PlatformID);
		
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");*/


		//Net_Close(iNSRet);
		return 0;
	}else{
		ShowLogs(1,"GetCelPlatformID return -1");
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");
		NetCloseSocket(iNSRet);*/
		//Net_Close(iNSRet);
		return -1;
	}
}


int GetCelPlatformID(void)
{
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};
	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	const char* headers[3] = {"Content-Type: application/json", "Accept: application/json","Authorization: Basic ZXRvcDpFdG9wKjEyMw=="};
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char data[100] = {'\0'};


	
	char Confirmtemp[150]={0};
	int		iResult;

	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "EKEDC - Token ID");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - GetCelPlatformID");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	memset(PortNo,0,sizeof(PortNo));
	sprintf(PortNo,"%d",ucPort);
			
memset(Cel_responseCode, 0, sizeof(Cel_responseCode));
memset(Cel_responseMessage, 0, sizeof(Cel_responseMessage));

	/*sprintf(RequestData, "POST /%s?%s HTTP/1.1\r\n" \
				"Host: %s:%s\r\n" \
				"Accept: application/xml\r\n" \
				"Content-Type: application/xml\r\n" \
				"Content-Length: %d\r\n\r\n%s\r\n","/SmartWallet/Proxy/SendToBank",data,"41.73.252.236", "28080",strlen(RequestData),RequestData);
	*/

	char walletid[20]={0};

	UtilGetEnvEx("walletid", walletid);
	sprintf(data,"&Phone=%s&Channel=5",walletid);
	//sprintf(RequestData, "%s?%s","http://67.205.165.41/tapi/etop/authenticate-token.php",data);
	sprintf(RequestData, "%s","http://67.205.165.41/tapi/etop/authenticate-token.php");

	ShowLogs(1,"IN - GetCelPlatformID: %s",RequestData);  

	len=strlen(data);
//if (sendHttpRequest(HTTP_POST, "http://173.230.138.119/test.php", RequestData, len, headers, 5, &chunk) == 0) {
		
//: http://67.205.165.41/tapi/etop/search-eedc.php?customer=9330376246 
if (sendHttpRequest(HTTP_POST, "http://67.205.165.41/tapi/etop/authenticate-token.php", RequestData, len, headers, 3, &chunk)==0)
	{			
	//if (sendHttpRequest(HTTP_GET, RequestData, data, len, headers, 0, &chunk) == 0) {
		//logd((""));
		//ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);		
			if (chunk.memory) {
				free(chunk.memory);
			}
		//ShowLogs(1,"Response:%s / %s", Cel_responseCode,Cel_responseMessage);
		NetCloseSocket(iNSRet);
		Gui_ClearScr();
		ParseJson((char *)RcvData);
		sprintf(Confirmtemp,"Token\r\n%s\r\nAmount\r\nNGN %d\r\nAccount No\r\n%s",Cel_Token,atol(Cel_BenAmunt)/100,Cel_TinAccountNumber);
		
		iResult = Gui_ShowMsgBox("EEDC Information", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN,25, NULL);
		if(iResult == ERR_USERCANCEL || iResult == GUI_ERR_TIMEOUT){
			ShowLogs(1,"GetBenefAccountNumber - DisplayInfoYN - return 0");
			return 0;
			//return ERR_USERCANCEL;
		}
		if(strcmp(Cel_responseCode, "00") == 0){
			return 1;
		}
		//function = TRAN_TYPE_TMS_NOTIFICATION;
		
		//ShowLogs(1,"Cel_PlatformID = %s",Cel_PlatformID);
		//iResult = Gui_ShowMsgBox("Transfer Info", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN, 15, NULL);
		
		//displayMessageWt(Cel_responseCode,Cel_PlatformID);
		
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");*/


		//Net_Close(iNSRet);
		return 0;
	}else{
		ShowLogs(1,"GetCelPlatformID return -1");
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");
		NetCloseSocket(iNSRet);*/
		//Net_Close(iNSRet);
		return -1;
	}
	
}


int eedcCusDetails()
{
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};
	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	char temptoken[100]={0};

	sprintf(temptoken,"Authorization: Bearer %s", Cel_Token);
	const char* headers[3] = {"Content-Type: application/json", "Accept: application/json",temptoken};
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char data[100] = {'\0'};


	
	char Confirmtemp[150]={0};
	int		iResult;

	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "EKEDC - Token ID");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - GetCelPlatformID");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	memset(PortNo,0,sizeof(PortNo));
	sprintf(PortNo,"%d",ucPort);
			
	/*sprintf(RequestData, "POST /%s?%s HTTP/1.1\r\n" \
				"Host: %s:%s\r\n" \
				"Accept: application/xml\r\n" \
				"Content-Type: application/xml\r\n" \
				"Content-Length: %d\r\n\r\n%s\r\n","/SmartWallet/Proxy/SendToBank",data,"41.73.252.236", "28080",strlen(RequestData),RequestData);
	*/

	char walletid[20]={0};

	UtilGetEnvEx("walletid", walletid);
	sprintf(data,"&Phone=%s&Channel=5",walletid);





	//getting input

	uchar szBuff[30]={0};
	GUI_INPUTBOX_ATTR stInputAttr;
	char buffer[50]={0};

	memset(&Cel_TinAccountNumber, 0, sizeof(Cel_TinAccountNumber));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 25;
	stInputAttr.bSensitive = 0;

	//sprintf((char *)szBuff, "%.25s", AgentPOSID);

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	//stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	Gui_ClearScr();

//	Enter Tingg Account Number

	iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Customer Phone"), gl_stLeftAttr, eedc_phone, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
return 1;

	//end getting input
	sprintf(RequestData, "%s?customer=%s","http://67.205.165.41/tapi/etop/search-eedc.php",eedc_phone);
	//sprintf(RequestData, "%s","http://67.205.165.41/tapi/etop/authenticate-token.php");

	ShowLogs(1,"%s",RequestData);  

	len=strlen(data);
//if (sendHttpRequest(HTTP_POST, "http://173.230.138.119/test.php", RequestData, len, headers, 5, &chunk) == 0) {
		//rl: http://67.205.165.41/tapi/etop/search-eedc.php?customer=9330376246 
 

//: http://67.205.165.41/tapi/etop/search-eedc.php?customer=9330376246 
if (sendHttpRequest(HTTP_GET, RequestData, data, len, headers, 3, &chunk)==0)
	{			
	//if (sendHttpRequest(HTTP_GET, RequestData, data, len, headers, 0, &chunk) == 0) {
		//logd((""));
		ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);
				
			if (chunk.memory) {
				free(chunk.memory);
			}
			
		ShowLogs(1,"Response:%s / %s", Cel_responseCode,Cel_responseMessage);
		NetCloseSocket(iNSRet);

		
		Gui_ClearScr();
		ParseJson((char *)RcvData);
		sprintf(Confirmtemp,"Token\r\n%s\r\nAmount\r\nNGN %d\r\nAccount No\r\n%s",RcvData,atol(Cel_BenAmunt)/100,Cel_TinAccountNumber);
		
		iResult = Gui_ShowMsgBox("EEDC Information", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN,25, NULL);
		if(iResult == ERR_USERCANCEL || iResult == GUI_ERR_TIMEOUT){
			ShowLogs(1,"GetBenefAccountNumber - DisplayInfoYN - return 0");
			return 0;
			//return ERR_USERCANCEL;
		}
		if(strcmp(Cel_responseCode, "00") == 0){
			return 1;
		}
		//function = TRAN_TYPE_TMS_NOTIFICATION;
		
		//ShowLogs(1,"Cel_PlatformID = %s",Cel_PlatformID);
		//iResult = Gui_ShowMsgBox("Transfer Info", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN, 15, NULL);
		
		//displayMessageWt(Cel_responseCode,Cel_PlatformID);
		
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");*/


		//Net_Close(iNSRet);
		return 1;
	}else{
		ShowLogs(1,"GetCelPlatformID return -1");
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");
		NetCloseSocket(iNSRet);*/
		//Net_Close(iNSRet);
		return -1;
	}
	
}



int jambpayment()
{
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};
	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	char temptoken[100]={0};

	const char* headers[5] = {"Content-Type: application/json", "Accept: application/json","webkey: 8af5e1092ad24521a4a5353b73132145","accountid: 100002805","Authorization: Basic bWljaGVsa2FsYXZhbmRhQGdtYWlsLmNvbTpkZXZARXRvcA=="};
	
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char data[100] = {'\0'};


	if(strncmp(glProcInfo.stTranLog.szRspCode, "00", 8) != 0)return 0;
	char Confirmtemp[150]={0};
	int		iResult;

	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "JAMB PAYMENT");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - GetCelPlatformID");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	memset(PortNo,0,sizeof(PortNo));
	sprintf(PortNo,"%d",ucPort);
			
	/*sprintf(RequestData, "POST /%s?%s HTTP/1.1\r\n" \
				"Host: %s:%s\r\n" \
				"Accept: application/xml\r\n" \
				"Content-Type: application/xml\r\n" \
				"Content-Length: %d\r\n\r\n%s\r\n","/SmartWallet/Proxy/SendToBank",data,"41.73.252.236", "28080",strlen(RequestData),RequestData);
	*/

	char walletid[20]={0};

	UtilGetEnvEx("walletid", walletid);
	len=0;
	/*

        
        
        


        
*///getting input

	uchar szBuff[30]={0};
	GUI_INPUTBOX_ATTR stInputAttr;
	char buffer[50]={0};

	memset(&Cel_TinAccountNumber, 0, sizeof(Cel_TinAccountNumber));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 25;
	stInputAttr.bSensitive = 0;

	//sprintf((char *)szBuff, "%.25s", AgentPOSID);

	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	//stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	Gui_ClearScr();

//	Enter Tingg Account Number
	memset(JAMB_CUS_ID, 0, sizeof(JAMB_CUS_ID));
	iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("External Ref"), gl_stLeftAttr, JAMB_CUS_ID, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		return 0;
	}

	sprintf(JAMB_CUS_ID,"%s",JAMB_CUS_ID);
	//end getting input
	len += sprintf(&RequestData[len], "{    \"externalref\": \"%s\",    \"billerCode\": \"JAMB\",    \"productId\": \"1141\",    \"transDetails\": [",JAMB_CUS_ID);
	len += sprintf(&RequestData[len], "{          \"fieldName\": \"Amount\",            \"fieldValue\": \"%.2f\",            \"fieldControlType\": \"TEXTBOX\"},",atol(glProcInfo.stTranLog.szAmount)/100.0);
	len += sprintf(&RequestData[len],"{            \"fieldName\": \"Candidate Details\",            \"fieldValue\": \"%s\",            \"fieldControlType\": \"TEXTBOX\"        },",jamb_customerName);
	len += sprintf(&RequestData[len],"{            \"fieldName\": \"Email\",            \"fieldValue\": \"seye@.com\",            \"fieldControlType\": \"TEXTBOX\"        },");	
	len += sprintf(&RequestData[len],"        {            \"fieldName\": \"Phone Number\",            \"fieldValue\": \"08123456789\",            \"fieldControlType\": \"TEXTBOX\"        },");

	len += sprintf(&RequestData[len],"        {            \"fieldName\": \"Phone Number\",            \"fieldValue\": \"08123456789\",            \"fieldControlType\": \"TEXTBOX\"        },");

	len += sprintf(&RequestData[len],"{            \"fieldName\": \"Select an Option\",            \"fieldValue\": \"UTME\",            \"fieldControlType\": \"LOOKUP\"        }    ]}");


	sprintf(data,"&Phone=%s&Channel=5",walletid);





	
	//sprintf(RequestData, "%s?%s","http://67.205.165.41/tapi/etop/search-eedc.php",Cel_TinAccountNumber);
	//sprintf(RequestData, "%s","https://80.88.8.245:9591/api/process-transaction");

	ShowLogs(1,"%s",RequestData);  

	//len=strlen(data);
//if (sendHttpRequest(HTTP_POST, "http://173.230.138.119/test.php", RequestData, len, headers, 5, &chunk) == 0) {
		//rl: http://67.205.165.41/tapi/etop/search-eedc.php?customer=9330376246 
 

//: http://67.205.165.41/tapi/etop/search-eedc.php?customer=9330376246 
if (sendHttpRequest(HTTP_POST, "https://80.88.8.245:9591/api/process-transaction",RequestData,  len, headers, 5, &chunk)==0)
	{			
	//if (sendHttpRequest(HTTP_GET, RequestData, data, len, headers, 0, &chunk) == 0) {
		//logd((""));
		ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);
				
			if (chunk.memory) {
				free(chunk.memory);
			}
			
		ShowLogs(1,"Response:%s / %s", Cel_responseCode,Cel_responseMessage);
		NetCloseSocket(iNSRet);

		
		Gui_ClearScr();
		ParseJson((char *)RcvData);
		sprintf(Confirmtemp,"Token\r\n%s\r\nAmount\r\nNGN %d\r\nAccount No\r\n%s",RcvData,atol(Cel_BenAmunt)/100,Cel_TinAccountNumber);
		
		iResult = Gui_ShowMsgBox("EEDC Information", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN,25, NULL);
		if(iResult == ERR_USERCANCEL || iResult == GUI_ERR_TIMEOUT){
			ShowLogs(1,"GetBenefAccountNumber - DisplayInfoYN - return 0");
			return 0;
			//return ERR_USERCANCEL;
		}
		if(strcmp(Cel_responseCode, "00") == 0){
			return 1;
		}
		//function = TRAN_TYPE_TMS_NOTIFICATION;
		
		//ShowLogs(1,"Cel_PlatformID = %s",Cel_PlatformID);
		//iResult = Gui_ShowMsgBox("Transfer Info", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN, 15, NULL);
		
		//displayMessageWt(Cel_responseCode,Cel_PlatformID);
		
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");*/


		//Net_Close(iNSRet);
		return 0;
	}else{
		ShowLogs(1,"GetCelPlatformID return -1");
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");
		NetCloseSocket(iNSRet);*/
		//Net_Close(iNSRet);
		return -1;
	}
	
}

int eedcPayment()
{
	ShowLogs(1,"IN - GetCelPlatformID");
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};
	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	char temptoken[100]={0};

	sprintf(temptoken,"Authorization: Bearer %s", Cel_Token);
	const char* headers[3] = {"Content-Type: application/json", "Accept: application/json",temptoken};
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char data[100] = {'\0'};
		char fdate[50]={0};

	
	char Confirmtemp[150]={0};
	int		iResult;
//if(strncmp(glProcInfo.stTranLog.szRspCode, "00", 8) != 0)return 0;

	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "EKEDC - Token ID");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - GetCelPlatformID");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

memset(Cel_responseCode, 0, sizeof(Cel_responseCode));
memset(Cel_responseMessage, 0, sizeof(Cel_responseMessage));
	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	//memset(PortNo,0,sizeof(PortNo));
	//sprintf(PortNo,"%d",ucPort);
			
	/*sprintf(RequestData, "POST /%s?%s HTTP/1.1\r\n" \
				"Host: %s:%s\r\n" \
				"Accept: application/xml\r\n" \
				"Content-Type: application/xml\r\n" \
				"Content-Length: %d\r\n\r\n%s\r\n","/SmartWallet/Proxy/SendToBank",data,"41.73.252.236", "28080",strlen(RequestData),RequestData);
	*/

	char walletid[20]={0};
GetFlutterDate(fdate);
	UtilGetEnvEx("walletid", walletid);
				len=0;
				len += sprintf(&RequestData[len], "{\"tran_ref\":\"%s\",",RefDate);
				len += sprintf(&RequestData[len],  "\"amount\":\"%.2f\",",atol(glProcInfo.stTranLog.szAmount)/100.0);
				len += sprintf(&RequestData[len], "\"phoneNumber\": \"%s\",",eedc_phone);
				
				





	//getting input

	uchar szBuff[30]={0};
	GUI_INPUTBOX_ATTR stInputAttr;
	char buffer[50]={0};

	memset(&Cel_TinAccountNumber, 0, sizeof(Cel_TinAccountNumber));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 25;
	stInputAttr.bSensitive = 0;
	//sprintf((char *)szBuff, "%.25s", AgentPOSID);
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	//stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	Gui_ClearScr();
	//Enter Tingg Account Number

	iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Enter Metter AcctNo"), gl_stLeftAttr, Cel_TinAccountNumber, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		return 0;
	}
	len += sprintf(&RequestData[len], "\"payment_type\": \"%s\",","CARD");
	len += sprintf(&RequestData[len], "\"meterNumber\": \"%s\",",Cel_TinAccountNumber);
	UtilGetEnvEx("eedctype", temp);
	len += sprintf(&RequestData[len], "\"paymentPlan\": \"%s\",",temp);
	memset(temp,0,sizeof(temp));
	UtilGetEnvEx("tid", temp);
	len += sprintf(&RequestData[len], "\"terminal_id\": \"%s\"}",temp);
	ShowLogs(1,"%s",RequestData);  
	len=strlen(RequestData);
if (sendHttpRequest(HTTP_POST,"http://67.205.165.41/tapi/etop/pay-eedc.php",RequestData, len, headers, 3, &chunk)==0)
	{			
	//if (sendHttpRequest(HTTP_GET, RequestData, data, len, headers, 0, &chunk) == 0) {
	
		ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);
				
			if (chunk.memory) {
				free(chunk.memory);
			}
			
		ShowLogs(1,"Response:%s / %s", Cel_responseCode,Cel_responseMessage);
		NetCloseSocket(iNSRet);
		Gui_ClearScr();
		ParseJson((char *)RcvData);
		sprintf(Confirmtemp,"Token\r\n%s\r\nAmount\r\nNGN %s\r\nAccount No\r\n%s",Cel_Token,eedc_token,Cel_TinAccountNumber);
		iResult = Gui_ShowMsgBox("EEDC Information", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN,25, NULL);
		if(iResult == ERR_USERCANCEL || iResult == GUI_ERR_TIMEOUT){
			ShowLogs(1,"GetBenefAccountNumber - DisplayInfoYN - return 0");
			return 0;
			//return ERR_USERCANCEL;
		}
		ShowLogs(1,"2 IN - GetCelPlatformID");
		if(strcmp(Cel_responseCode, "00") == 0){
			return 0;
		}
		//function = TRAN_TYPE_TMS_NOTIFICATION;
		
		//ShowLogs(1,"Cel_PlatformID = %s",Cel_PlatformID);
		//iResult = Gui_ShowMsgBox("Transfer Info", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN, 15, NULL);
		
		//displayMessageWt(Cel_responseCode,Cel_PlatformID);
		
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");*/


		//Net_Close(iNSRet);
		return 0;
	}else{
		ShowLogs(1,"GetCelPlatformID return -1");
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");
		NetCloseSocket(iNSRet);*/
		//Net_Close(iNSRet);
		return -1;
	}
	
}

int eedcPaymentCash()
{
	ShowLogs(1,"IN - eedcPaymentCash");
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};
	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	char temptoken[100]={0};

	sprintf(temptoken,"Authorization: Bearer %s", Cel_Token);
	const char* headers[3] = {"Content-Type: application/json", "Accept: application/json",temptoken};
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char data[100] = {'\0'};
		char fdate[50]={0};

	
	char Confirmtemp[150]={0};
	int		iResult;
//if(strncmp(glProcInfo.stTranLog.szRspCode, "00", 8) != 0)return 0;

	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "EKEDC - Token ID");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - GetCelPlatformID");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

memset(Cel_responseCode, 0, sizeof(Cel_responseCode));
memset(Cel_responseMessage, 0, sizeof(Cel_responseMessage));
	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	//memset(PortNo,0,sizeof(PortNo));
	//sprintf(PortNo,"%d",ucPort);
			
	/*sprintf(RequestData, "POST /%s?%s HTTP/1.1\r\n" \
				"Host: %s:%s\r\n" \
				"Accept: application/xml\r\n" \
				"Content-Type: application/xml\r\n" \
				"Content-Length: %d\r\n\r\n%s\r\n","/SmartWallet/Proxy/SendToBank",data,"41.73.252.236", "28080",strlen(RequestData),RequestData);
	*/

	char walletid[20]={0};
GetFlutterDate(fdate);
	UtilGetEnvEx("walletid", walletid);
				len=0;
				len += sprintf(&RequestData[len], "{\"tran_ref\":\"%s\",",RefDate);
				len += sprintf(&RequestData[len],  "\"amount\":\"%.2f\",",atol(glProcInfo.stTranLog.szAmount)/100.0);
				len += sprintf(&RequestData[len], "\"phoneNumber\": \"%s\",",eedc_phone);
				
				





	//getting input

	uchar szBuff[30]={0};
	GUI_INPUTBOX_ATTR stInputAttr;
	char buffer[50]={0};

	memset(&Cel_TinAccountNumber, 0, sizeof(Cel_TinAccountNumber));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 25;
	stInputAttr.bSensitive = 0;
	//sprintf((char *)szBuff, "%.25s", AgentPOSID);
	GUI_TEXT_ATTR stTextAttr = gl_stLeftAttr;
	//stTextAttr.eFontSize = GUI_FONT_SMALL;
	
	Gui_ClearScr();
	//Enter Tingg Account Number

	iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Enter Metter AcctNo"), gl_stLeftAttr, Cel_TinAccountNumber, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		return 0;
	}
	len += sprintf(&RequestData[len], "\"payment_type\": \"%s\",","CASH");
	len += sprintf(&RequestData[len], "\"meterNumber\": \"%s\",",Cel_TinAccountNumber);
	UtilGetEnvEx("eedctype", temp);
	len += sprintf(&RequestData[len], "\"paymentPlan\": \"%s\",",temp);
	memset(temp,0,sizeof(temp));
	UtilGetEnvEx("tid", temp);
	len += sprintf(&RequestData[len], "\"terminal_id\": \"%s\"}",temp);
	ShowLogs(1,"%s",RequestData);  
	len=strlen(RequestData);
if (sendHttpRequest(HTTP_POST,"http://67.205.165.41/tapi/etop/pay-eedc.php",RequestData, len, headers, 3, &chunk)==0)
	{			
	//if (sendHttpRequest(HTTP_GET, RequestData, data, len, headers, 0, &chunk) == 0) {
	
		ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);
				
			if (chunk.memory) {
				free(chunk.memory);
			}
			
		ShowLogs(1,"Response:%s / %s", Cel_responseCode,Cel_responseMessage);
		NetCloseSocket(iNSRet);
		Gui_ClearScr();
		ParseJson((char *)RcvData);
		sprintf(Confirmtemp,"Token\r\n%s\r\nAmount\r\nNGN %s\r\nAccount No\r\n%s",Cel_Token,eedc_token,Cel_TinAccountNumber);
		iResult = Gui_ShowMsgBox("EEDC Information", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN,25, NULL);
		if(iResult == ERR_USERCANCEL || iResult == GUI_ERR_TIMEOUT){
			ShowLogs(1,"GetBenefAccountNumber - DisplayInfoYN - return 0");
			return 0;
			//return ERR_USERCANCEL;
		}
		ShowLogs(1,"2 IN - GetCelPlatformID");
		if(strcmp(Cel_responseCode, "00") == 0){
			return 0;
		}
		//function = TRAN_TYPE_TMS_NOTIFICATION;
		
		//ShowLogs(1,"Cel_PlatformID = %s",Cel_PlatformID);
		//iResult = Gui_ShowMsgBox("Transfer Info", gl_stTitleAttr,Confirmtemp , gl_stCenterAttr, GUI_BUTTON_YandN, 15, NULL);
		
		//displayMessageWt(Cel_responseCode,Cel_PlatformID);
		
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");*/


		//Net_Close(iNSRet);
		return 0;
	}else{
		ShowLogs(1,"GetCelPlatformID return -1");
		/*char store[2 * 1024] = {0};
		Print_Cel_Transfer("TRANSFER", store, "CHIP");
		NetCloseSocket(iNSRet);*/
		//Net_Close(iNSRet);
		return -1;
	}
	
}


int PostNameLookup(char data [250])
{
	int iRet=1,iSize=102400,iTimeoutNum;
	int ii,len;
	long Amount;
	char PortNo[5];
	int ContentLength=0;
	char DebugMessage[50] = {0};
	unsigned char sAPN[50] ={0};
	unsigned char sUser[50] = {0};
	unsigned char sPwd[50] ={0};
	char sIP[20]= {0};
	int pos,ucPort,iNSRet;
	MemoryStruct chunk = { 0 };
	const char* headers[2] = {"Content-Type: application/xml", "Accept: application/xml"};
	char RequestData[1000]={0};
	char RcvData[2000]={0};
	char temp[50]={0};
	char maskedPan[25] = {'\0'};
	//POSParamaters  MyLocalDevice = {0}; 
	//SET_POSParamaters(&MyLocalDevice,0);
		
	UtilGetEnv("coapn", sAPN);
	UtilGetEnv("cosubnet", sUser);
	UtilGetEnv("copwd", sPwd);

	//DisplayStrMiddle("Connecting...",3);

	ScrCls();
	textView(1, 1,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s" , "ACCOUNT NAME ENQ.");
	textView(1, 6,  LAYOUT_CENTER, LAYOUT_INVERSE,"%s","PLEASE WAIT!");
	textView(1, 3,  LAYOUT_CENTER, LAYOUT_CENTER,"%s","...Connecting...");


	ShowLogs(1,"IN - PostNameLookup");

	WlInit(NULL);
	//APP_printf("WlInit=%d\r\n", ret);
	WlPppLogin(sAPN,sUser,sPwd,0xFF,0,600);
	//APP_printf("WlPppLogin=%d\r\n", ret);
	WlPppCheck();
				
	PubStrLower((char *)sPwd);
	PubStrLower((char *)sUser);
		
	sprintf(DebugMessage,"Logon:%s-%s-%s", sAPN,sUser,sPwd);
	//PTransaction=0;

	//DisplayStrMiddle("Connecting...",3);
	iNSRet = WirelessDial(sAPN,sUser,sPwd);

	if (iNSRet < 0)
	{
		memset(DebugMessage,0,sizeof(DebugMessage));
		sprintf(DebugMessage,"NetSocket failed:%d", iNSRet);
		ScrCls();
		ScrGotoxy(0,0);
		Lcdprintf("%i->%s",ii,DebugMessage);
		getkey();
		ScrFontSet(0);
		//ShowLogs(1,DebugMessage);
		return -2;
	}

	//memset(ucTmpSend, 0, sizeof(ucTmpSend));
	memset(PortNo,0,sizeof(PortNo));
	sprintf(PortNo,"%d",ucPort);


	sprintf(RequestData, "%s?%s","http://41.73.252.230:28080/SmartNameEnquiry/SmartWallet/Proxy/NameEnquiry",data);

	ShowLogs(1,"IN - PostNameLookup: %s",RequestData);  

	len=strlen(data);


	if (sendHttpRequest(HTTP_POST, RequestData, data, len, headers, 0, &chunk) == 0) {
		//logd((""));
		ShowLogs(1,"Success Response:%s", chunk.memory);
		strcpy(RcvData,chunk.memory);
				
		if (chunk.memory) {
			free(chunk.memory);
		}
		
		NetCloseSocket(iNSRet);

		//ShowLogs(1,"PostCelulantNotification: return 0");

		//function = TRAN_TYPE_TMS_NOTIFICATION;
		//ParseJson((char *)RcvData);
			
		//00|ATASOGHOHENRY COLLINS|100005211002094811401679445389
		
		int i,cp=0,j=0;
				
		ShowLogs(1,"Start parsng:%s", RcvData);
		
		for (i=0;i<=strlen(RcvData);i++){
			if(RcvData[i]=='|'){
				cp++;
				j=0;
				continue;
			}
			if(cp==1){
				Cel_AccountName[j++]=RcvData[i];
				//ShowLogs(1,"Cel_AccountName:%s",Cel_AccountName);
			}	
			if(cp==2){
				Cel_AccountRef[j++]=RcvData[i];
				//ShowLogs(1,"Cel_AccountName:%s",Cel_AccountName);
			}				
			//if (cp>1)
			//	break;	
		}

		if(cp==0)
		{
			ShowLogs(1,"Account Name error:%s",RcvData);
			displayMessageWt("Account Name error",RcvData);
			return 0;
		}else{
			ShowLogs(1,"Account Name:%s",Cel_AccountName);
			//displayMessageWt("Account Name",Cel_AccountName);
			return 1;
		}
		//Net_Close(iNSRet);
		return 0;
	}else{
		NetCloseSocket(iNSRet);
		//Net_Close(iNSRet);
		return 0;
	}
}



