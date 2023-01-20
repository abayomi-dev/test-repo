#include "global.h"

DL_ISO8583_HANDLER isoHandler;
DL_ISO8583_MSG     isoMsg;
//DL_UINT8           packBuf[5 * 1024];
DL_UINT16          packedSize;

#define MASTERKEYINDEX 1
#define PINKEYINDEX 2
int revSend = 0;
int switchHostManual = 0;
int titi = 0;

#define SUCABOIP "52.31.200.20"//139.162.203.47//62.173.47.4"139.162.204.105"
#define SUCABOPORT "80"//9001/9595

//REMOVE STATIC ipS
char ip[20] = {0};

void HexEnCodeMethod2(unsigned char *psIn, unsigned int uiLength, unsigned char *psOut) 
{
	static const unsigned char ucHexToChar[17] = { "0123456789ABCDEF" };
	unsigned int uiCnt;
	if ((psIn == NULL) || (psOut == NULL)) {
		return;
	}

	for (uiCnt = 0; uiCnt < uiLength; uiCnt++) {
		psOut[2 * uiCnt] = ucHexToChar[(psIn[uiCnt] >> 4)];
		psOut[2 * uiCnt + 1] = ucHexToChar[(psIn[uiCnt] & 0x0F)];
	}
}

void HexEnCodeMethod(char *psIn, unsigned int uiLength, char *pszOut) {
	HexEnCodeMethod2(psIn, uiLength, pszOut);
	pszOut[2 * uiLength] = 0;
	ShowLogs(1, "CAN WE STORE THIS AND MOVE d......");
}

int hexDecode(char *theHexStr, char *theBinBuffer, int theBufferLen, int *copiedLen) 
{
	*copiedLen = strlen(theHexStr) / 2;
	HexDecodeMethod2(theHexStr, *copiedLen * 2, theBinBuffer, theBufferLen);
	return 1;
}

int DecryptDES2ECBMode(char* theEncMsg, int theEncMsgLen,
		char* theUpperCaseHexKey32Len, char *theDecMsg, int theDecMsgLen) 
{
	char *TempKey = NULL;
	int iLen, i;
	memset(theDecMsg, 0x0, theDecMsgLen);
	TempKey = (char*)sysalloc_calloc(strlen(theUpperCaseHexKey32Len), sizeof(char));

	if (!TempKey) {
		sysalloc_free(TempKey);
		TempKey = NULL;
		return 0;
	}

	PubAsc2Bcd(theUpperCaseHexKey32Len, 32, TempKey);

	for (iLen = theEncMsgLen, i = 0; iLen > 0; iLen -= 8, i++) {
		PubDes(TRI_DECRYPT, theEncMsg + i * 8, TempKey, theDecMsg + i * 8);
	}
	sysalloc_free(TempKey);
	TempKey = NULL;
	return 1;
}

void DecryptDes2Hex(char *theEnc, char *theClkey, char *mskey)
{
	char decOutPut[1024] = { 0 };
	char temp[1024] = { 0 };
	int copiedLen;
	memset(temp, 0x0, sizeof(temp));
	hexDecode((char*)theEnc, temp, sizeof(temp), &copiedLen);
	memset(decOutPut, 0x0, sizeof(decOutPut));
	DecryptDES2ECBMode(temp, copiedLen, (char*)theClkey, decOutPut, sizeof(decOutPut));
	memset(temp, 0x0, sizeof(temp));
	HexEnCodeMethod((unsigned char*) decOutPut, 16, (unsigned char*) temp);
	strcpy(mskey, temp);
}

int EncodeDES2ECBMode(char* theInputData, int theInputDataLen, char* theUpperCaseHexKey32Len, char* theEncCipher, int theEncCipherBuLen) 
{
	unsigned char TempKey[32];
	unsigned char TempCipher[2048] = { 0 };
	int iLen, i;
	memset(TempCipher, 0x0, sizeof(TempCipher));
	if (!theUpperCaseHexKey32Len || !*theUpperCaseHexKey32Len || (theEncCipherBuLen < theInputDataLen)) 
	{
		return 0;
	}

	PubAsc2Bcd((unsigned char*) theUpperCaseHexKey32Len, strlen(theUpperCaseHexKey32Len), TempKey);

	for (iLen = theInputDataLen, i = 0; iLen > 0; iLen -= 8, i++) {
		PubDes(TRI_ENCRYPT, (unsigned char*) theInputData + i * 8, TempKey,
				(unsigned char*) theEncCipher + i * 8);
	}
	return 0;
}

void CalcChk(char *csk) 
{
	char outPut[1024] = { 0 };
	char hexoutput[1024] = { 0 };
	char rethexoutput[18];
	int datalen;
	char input[100];
	char tempKey[100] = { '\0' };
	char tempOut[100] = { '\0' };
	memset(input, 0x0, sizeof(input));
	memset(outPut, 0x0, sizeof(outPut));
	memset(hexoutput, 0x0, sizeof(hexoutput));
	strcpy(input, "0000000000000000");
	HexDecodeMethod((unsigned char*) input, 16, (unsigned char*) tempOut);
	EncodeDES2ECBMode(tempOut, 8, (char*) csk, outPut, sizeof(outPut));
	memset(hexoutput, 0x0, sizeof(hexoutput));
	HexEnCodeMethod((unsigned char*) outPut, 16, (unsigned char*) hexoutput);
	memset(rethexoutput, 0x0, sizeof(rethexoutput));
	strncpy(rethexoutput, hexoutput, 16);
}


int InjectMasterSessionKey(char *masterKey, char *pinKey, int keyLen) {
	ST_KEY_INFO keyInfo = {0};
	ST_KCV_INFO kcvInfo = {0};

	//inject masterkey
	keyInfo.ucSrcKeyType = -1;
	keyInfo.ucSrcKeyIdx = 0;
	keyInfo.ucDstKeyType = PED_TMK;
	keyInfo.ucDstKeyIdx = MASTERKEYINDEX;
	keyInfo.iDstKeyLen = 16;
	memcpy(keyInfo.aucDstKeyValue, masterKey, 16);

	kcvInfo.iCheckMode = 0;

	int ret;
	ret = PedWriteKey(&keyInfo, &kcvInfo);

	if(ret != PED_RET_OK){
		ShowLogs(1, "Masterkey injection failed. Returned... %i", ret);
		Beep();
		DisplayInfoNone("", "Pinkey Error", 0);
		return 0;
	}

	//inject pinKey
	keyInfo.ucSrcKeyType = PED_TMK;
	keyInfo.ucSrcKeyIdx = MASTERKEYINDEX;
	keyInfo.ucDstKeyType = PED_TPK;
	keyInfo.ucDstKeyIdx = PINKEYINDEX;
	keyInfo.iDstKeyLen = 16;
	memcpy(keyInfo.aucDstKeyValue, pinKey, 16);

	kcvInfo.iCheckMode = 0;

	ret = PedWriteKey(&keyInfo, &kcvInfo);

	ShowLogs(1, "Response from injecting pinkey into Pinpad.........: %i", ret);
	
	return ret == PED_RET_OK;
}


int InjectMSKey(char *theClkey, char *theEnc) 
{
	char masterKey[32] = { 0 }, sessionKey[32] = { 0 };
	HexDecodeMethod((unsigned char*) theEnc, strlen(theEnc), (unsigned char*) sessionKey);
	HexDecodeMethod((unsigned char*) theClkey, strlen(theClkey), (unsigned char*)masterKey);
	return InjectMasterSessionKey(masterKey, sessionKey, strlen(theEnc) / 2);
}

int GetMasterKey()
{
	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char stan[7] = {0};

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;
	
	memset(packBuf, 0x0, sizeof(packBuf));

	SysGetTimeIso(timeGotten);
	strncpy(datetime, timeGotten + 2, 10);
	strncpy(dt, timeGotten + 2, 4);
	strncpy(tm, timeGotten + 6, 6);

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	(void)DL_ISO8583_MSG_SetField_Str(0, "0800", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(3, "9A0000", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(7, datetime, &isoMsg);
	sprintf((char *)stan, "%06lu", useStan);
	(void)DL_ISO8583_MSG_SetField_Str(11, stan, &isoMsg);
	GetStan();
	(void)DL_ISO8583_MSG_SetField_Str(12, tm, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(13, dt, &isoMsg);
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("tid", temp);
	(void)DL_ISO8583_MSG_SetField_Str(41, temp, &isoMsg);

	/* output ISO message content */
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);

	/* pack ISO message content */
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		
	HexEnCodeMethod((DL_UINT8*)packBuf, packedSize + 2, hexData);
	//ShowLogs(1, "Iso Message Without Length: %s", hexData);
	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize;
	HexEnCodeMethod((DL_UINT8*)packBuf, packedSize + 2, hexData);
	ShowLogs(1, "Iso Message With Length: %s", hexData);
	ShowLogs(1, "Packed size.... %d", packedSize);

	
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s.", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s.", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s.", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s.", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s.", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s.", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s.", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
			ShowLogs(1, "Masterkey going crazy");

	 		EmvSetSSLFlag();
			DisplayInfoNone("Master Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Master Key", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Master Key", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Master Key", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Master Key", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Master Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Master Key", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Master Key", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Master Key", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Master Key", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "Masterkey CommRxd Response: %d", iRet);
			ShowLogs(1, "Masterkey Response: %s", output);

	 		if (iRet != 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "CommRxd failed. .. Master Key failed");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Master Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Master Key", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Master Key", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Master Key", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Master Key", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Master Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Master Key", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Master Key", "Failed", 1);
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Master Key", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Master Key", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);

	 		if (iRet != 0)
			{
				DisplayInfoNone("Master Key", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}

	

	ShowLogs(1, "CommRxd Response: '%s'", &output[2]);

 	ScrBackLight(1);
	//Start here
 	memset(hexData, '\0', strlen(hexData));
	//HexEnCodeMethod(output, strlen(output) + 2, hexData);
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	//HexEnCodeMethod(output, ucByte, hexData);
	ShowLogs(1, "Response: %s\n", hexData);
	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	//How to read data
	int i = 53;
	if ( NULL != isoMsg.field[i].ptr ) 
	{
		DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(39, &isoHandler);
		sprintf(resp, "%s", isoMsg.field[39].ptr);
		ShowLogs(1, "Response Code: %s", resp);
		if(strstr(resp, "00") != NULL)
		{
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			//ShowLogs(1, "Field %03i, %s%s\n",(int)i, isoMsg.field[i].ptr, "");
			sprintf(keyStore, "%s", isoMsg.field[i].ptr);
			memset(temp, '\0', strlen(temp)); 
			
			UtilGetEnvEx("swkcomp1", temp);
			ShowLogs(1, "Transport Key: %s", temp);
			memset(keyFin, '\0', strlen(keyFin)); 
			strncpy(keyFin, keyStore, 32);
			ShowLogs(1, "Encrypted MasterKey: %s", keyFin);
			memset(tempStore, '\0', strlen(tempStore));

			//DecryptDes2Hex(keyFin, temp, tempStore);
			DecryptDes2Hex(keyFin, "DBEECACCB4210977ACE73A1D873CA59F", tempStore); // HARDCODING KEY COMP XOR

			ShowLogs(1, "Clear Master Key: %s", tempStore);
			CreateWrite("isomastkey.txt", tempStore);
			DL_ISO8583_MSG_Free(&isoMsg);
			return 1;
		}
		DL_ISO8583_MSG_Free(&isoMsg);
		return 0;
	}else
		//printf("Field %d is not present in ISO\n", i);
		ShowLogs(1, "Field %d is not present in ISO\n", i);
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}

int GetSessionKey()
{
	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char stan[7] = {0};

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(packBuf, 0x0, sizeof(packBuf));
	SysGetTimeIso(timeGotten);
	strncpy(datetime, timeGotten + 2, 10);
	strncpy(dt, timeGotten + 2, 4);
	strncpy(tm, timeGotten + 6, 6);

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	(void)DL_ISO8583_MSG_SetField_Str(0, "0800", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(3, "9B0000", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(7, datetime, &isoMsg);
	sprintf((char *)stan, "%06lu", useStan);  //??
	(void)DL_ISO8583_MSG_SetField_Str(11, stan, &isoMsg);
	GetStan();
	(void)DL_ISO8583_MSG_SetField_Str(12, tm, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(13, dt, &isoMsg);
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("tid", temp);
	(void)DL_ISO8583_MSG_SetField_Str(41, temp, &isoMsg);
	/* output ISO message content */
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);

	/* pack ISO message */
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		
	HexEnCodeMethod((DL_UINT8*)packBuf, packedSize + 2, hexData);
	//ShowLogs(1, "Iso Message Without Length: %s", hexData);
	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize;
	HexEnCodeMethod((DL_UINT8*)packBuf, packedSize + 2, hexData);
	//ShowLogs(1, "Iso Message With Length: %s", hexData);
	ShowLogs(1, "Iso Message With Length: %s", hexData);
	ShowLogs(1, "Packed size %d", packedSize);

	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Session Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Session Key", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Session Key", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Session Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Session Key", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Session Key", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Session Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Session Key", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Session Key", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Session Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Session Key", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Session Key", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Session Key", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Session Key", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}
 	ScrBackLight(1);
	//Start here
 	memset(hexData, '\0', strlen(hexData));
	//HexEnCodeMethod(output, strlen(output) + 2, hexData);
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	//HexEnCodeMethod(output, ucByte, hexData);
	//ShowLogs(1, "Response: %s\n", hexData);
	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	//How to read data
	int i = 53;
	if ( NULL != isoMsg.field[i].ptr ) 
	{
		DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(39, &isoHandler);
		sprintf(resp, "%s", isoMsg.field[39].ptr);
		ShowLogs(1, "Response Code: %s", resp);
		if(strstr(resp, "00") != NULL)
		{
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			//ShowLogs(1, "Field %03i, %s%s\n",(int)i, isoMsg.field[i].ptr, "");
			sprintf(keyStore, "%s", isoMsg.field[i].ptr);
			memset(temp, '\0', strlen(temp));
			ReadAllData("isomastkey.txt", temp);
			ShowLogs(1, "Clear Masterkey: %s", temp);
			memset(keyFin, '\0', strlen(keyFin)); 
			strncpy(keyFin, keyStore, 32);
			ShowLogs(1, "Encrypted Sessionkey: %s", keyFin);
			memset(tempStore, '\0', strlen(tempStore)); 
			DecryptDes2Hex(keyFin, temp, tempStore);
			ShowLogs(1, "Clear Sessionkey: %s", tempStore);
			CreateWrite("isosesskey.txt", tempStore);
			CalcChk(tempStore);
			DL_ISO8583_MSG_Free(&isoMsg);
			return 1;
		}
		DL_ISO8583_MSG_Free(&isoMsg);
		return 0;
	}else
		//printf("Field %d is not present in ISO\n", i);
		ShowLogs(1, "Field %d is not present in ISO\n", i);
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}

void injectPinKey()
{
	char encpinkey[33] = {0};
	char temp[33] = {0};
	memset(encpinkey, '\0', strlen(encpinkey));
	ReadAllData("encpinkey.txt", encpinkey);
	ShowLogs(1, "Encrypted Pinkey: %s", encpinkey);
	memset(temp, '\0', strlen(temp));
	ReadAllData("isomastkey.txt", temp);
	ShowLogs(1, "Clear Masterkey: %s", temp);
	InjectMSKey(temp, encpinkey);
}

int GetPinKey()
{
	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char stan[7] = {0};

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(packBuf, 0x0, sizeof(packBuf));
	SysGetTimeIso(timeGotten);
	strncpy(datetime, timeGotten + 2, 10);
	strncpy(dt, timeGotten + 2, 4);
	strncpy(tm, timeGotten + 6, 6);
	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	/* set ISO message fields */
	(void)DL_ISO8583_MSG_SetField_Str(0, "0800", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(3, "9G0000", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(7, datetime, &isoMsg);
	sprintf((char *)stan, "%06lu", useStan);  //??
	(void)DL_ISO8583_MSG_SetField_Str(11, stan, &isoMsg);
	GetStan();
	(void)DL_ISO8583_MSG_SetField_Str(12, tm, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(13, dt, &isoMsg);
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("tid", temp);
	(void)DL_ISO8583_MSG_SetField_Str(41, temp, &isoMsg);

	/* output ISO message content */
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);

	/* pack ISO message */
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		
	HexEnCodeMethod((DL_UINT8*)packBuf, packedSize + 2, hexData);
	//ShowLogs(1, "Iso Message Without Length: %s", hexData);
	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize;
	HexEnCodeMethod((DL_UINT8*)packBuf, packedSize + 2, hexData);
	//ShowLogs(1, "Iso Message With Length: %s", hexData);
	ShowLogs(1, "Iso Message With Length: %s", hexData);
	ShowLogs(1, "Packed size %d", packedSize);
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Pin Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Pin Key", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Pin Key", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Pin Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				ShowLogs(1, "Serious dont know why.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Pin Key", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Pin Key", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Pin Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Pin Key", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Pin Key", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Pin Key", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Pin Key", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Pin Key", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Pin Key", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Pin Key", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}
 	ScrBackLight(1);
	//Start here
 	memset(hexData, '\0', strlen(hexData));
	//HexEnCodeMethod(output, strlen(output) + 2, hexData);
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	//HexEnCodeMethod(output, ucByte, hexData);
	//ShowLogs(1, "Response: %s\n", hexData);
	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	//How to read data
	int i = 53;
	if ( NULL != isoMsg.field[i].ptr ) 
	{
		DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(39, &isoHandler);
		sprintf(resp, "%s", isoMsg.field[39].ptr);
		ShowLogs(1, "Response Code: %s", resp);
		if(strstr(resp, "00") != NULL)
		{
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			//ShowLogs(1, "Field %03i, %s%s\n",(int)i, isoMsg.field[i].ptr, "");
			sprintf(keyStore, "%s", isoMsg.field[i].ptr);
			memset(keyFin, '\0', strlen(keyFin)); 
			strncpy(keyFin, keyStore, 32);
			ShowLogs(1, "Encrypted Pinkey: %s", keyFin);
			memset(temp, '\0', strlen(temp));
			ReadAllData("isomastkey.txt", temp);
			ShowLogs(1, "Clear Masterkey: %s", temp);
			InjectMSKey(temp, keyFin);
			memset(keyFin, '\0', strlen(keyFin)); 
			strncpy(keyFin, keyStore, 32);
			memset(temp, '\0', strlen(temp));
			ReadAllData("isomastkey.txt", temp);
			memset(tempStore, '\0', strlen(tempStore)); 
			DecryptDes2Hex(keyFin, temp, tempStore);
			ShowLogs(1, "Clear Pin Key: %s", tempStore);
			CreateWrite("isopinkey.txt", tempStore);
			DL_ISO8583_MSG_Free(&isoMsg);
			return 1;
		}
		DL_ISO8583_MSG_Free(&isoMsg);
		return 0;
	}else
		//printf("Field %d is not present in ISO\n", i);
		ShowLogs(1, "Field %d is not present in ISO\n", i);
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}

int parseParametersOld(char *data)
{
	int iLen = 0;
	int i;
	int loop = 0, j = 0, k = 0, len = 0, p = 0;
	char temp[50] = {0};
	char tempStore[50] = {0};
	char store[50] = {0};
	char mnl[50] = {0};
	char addr[50] = {0};
	char name[50] = {0};

	iLen = strlen(data);
	for(i = 0; i < iLen; i++)
	{
		if(loop == 0)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				//UtilPutEnv("txnTime", store);
			}
		}
		else if(loop == 1)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnMid", store);
			}
		}else if(loop == 2)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnTimeOut", store);
			}
		}else if(loop == 3)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				//UtilPutEnv("txnCurCode", store);
			}
		}else if(loop == 4)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnConCode", store);
			}
		}else if(loop == 5)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnTimeOut", store);
			}
		}else if(loop == 6)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				if(strlen(store) == 4)
					UtilPutEnv("txnMCC", store);
				else
					UtilPutEnv("txnMNL", store);
			}
		}else if(loop == 7)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				if(strlen(store) == 4)
					UtilPutEnv("txnMCC", store);
				else
					UtilPutEnv("txnMNL", store);
			}
		}else
            break;
	}

	
	/*memset(mnl, '\0', strlen(mnl));
	UtilGetEnv("txnMNL", mnl);

	memset(addr, '\0', strlen(addr));
	strncpy(addr, mnl, 23);
	strcat(addr, "\n");
	UtilPutEnv("merName", addr);

	memset(name, '\0', strlen(name));
	strcpy(name, mnl + 23);
	strcat(name, "\n");
	UtilPutEnv("merAddr", name);*/

	//DisplayInfoNone("", "Parameter Loaded", 1);
	return 1;
}

int parseParameters(char *data)
{
	int iLen = 0;
	int i;
	int loop = 0, j = 0, k = 0, len = 0, p = 0;
	char temp[50] = {0};
	char tempStore[50] = {0};
	char store[50] = {0};
	char mnl[50] = {0};
	char addr[50] = {0};
	char name[50] = {0};
	iLen = strlen(data);
	for(i = 0; i < iLen; i++)
	{
		if(loop == 0)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnTime", store);
			}
		}
		else if(loop == 1)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnMid", store);
			}
		}else if(loop == 2)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnTimeOut", store);
			}
		}else if(loop == 3)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				//UtilPutEnv("txnCurCode", store);
			}
		}else if(loop == 4)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnConCode", store);
			}
		}else if(loop == 5)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				UtilPutEnv("txnTimeOut", store);
			}
		}else if(loop == 6)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				if(strlen(store) == 4)
					UtilPutEnv("txnMCC", store);
				else
					UtilPutEnv("txnMNL", store);
			}
		}else if(loop == 7)
		{
			if(j < 2)
			{
				j++;
				memset(temp, '\0', strlen(temp));
				p = 0;
			}else if((j >= 2) && (j < 6))
			{
				temp[p] = data[i];
				p++;
				j++;
			}else
			{
				int m = 0;
				memset(tempStore, '\0', strlen(tempStore));
				if(temp[0] == '0' && temp[1] == '0')
					strncpy(tempStore, temp + 2, 1);
				else if(temp[0] == '0')
					strncpy(tempStore, temp + 1, 2);
				else
					strcpy(tempStore, temp);
				len = atoi(tempStore);
				j = 0;
				loop++;
				m = i - 1;
				memset(store, '\0', strlen(store));
				strncpy(store, data + m, len);
				i = i + len - 2;
				if(strlen(store) == 4)
					UtilPutEnv("txnMCC", store);
				else
					UtilPutEnv("txnMNL", store);
			}
		}else
            break;
	}

	/*memset(mnl, '\0', strlen(mnl));
	UtilGetEnv("txnMNL", mnl);

	memset(addr, '\0', strlen(addr));
	strncpy(addr, mnl, 23);
	strcat(addr, "\n");
	UtilPutEnv("merName", addr);

	memset(name, '\0', strlen(name));
	strcpy(name, mnl + 23);
	strcat(name, "\n");
	UtilPutEnv("merAddr", name);*/
	
	DisplayInfoNone("", "Parameter Loaded", 1);
	return 1;
}

int GetParaMeters()
{
	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handlerr */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char stan[7] = {0};

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(packBuf, 0x0, sizeof(packBuf));

	SysGetTimeIso(timeGotten);
	strncpy(datetime, timeGotten + 2, 10);
	strncpy(dt, timeGotten + 2, 4);
	strncpy(tm, timeGotten + 6, 6);

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	(void)DL_ISO8583_MSG_SetField_Str(0, "0800", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(3, "9C0000", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(7, datetime, &isoMsg);
	sprintf((char *)stan, "%06lu", useStan);  
	(void)DL_ISO8583_MSG_SetField_Str(11, stan, &isoMsg);
	GetStan();
	(void)DL_ISO8583_MSG_SetField_Str(12, tm, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(13, dt, &isoMsg);
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("tid", temp);
	(void)DL_ISO8583_MSG_SetField_Str(41, temp, &isoMsg);
	memset(SN, '\0', strlen(SN)); 
	ReadSN(SN);
	if ('\0' == SN[0]) {
		//No serial
		strcpy(theUHI, "000000009");
	}
	strcpy(theUHI, SN);
	sprintf(theUHISend, "01%03d%s", strlen(theUHI), theUHI);
	(void)DL_ISO8583_MSG_SetField_Str(62, theUHISend, &isoMsg);
	(void) DL_ISO8583_MSG_SetField_Str(64, 0x0, &isoMsg);
	
	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);
	ShowLogs(1, "Iso Message With Length: %s", hexData);
	ShowLogs(1, "Packed size %d", packedSize);
//5432, 9595, 8001

	if (packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(64, outHash, &isoMsg);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Parameters", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Parameters", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return 0;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Parameters", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Parameters", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Parameters", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Parameters", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Parameters", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Parameters", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}
 	ScrBackLight(1);
	//Start here
 	memset(hexData, '\0', strlen(hexData));
	HexEnCodeMethod(output, strlen(output) + 2, hexData);
	ShowLogs(1, "9C - Received From Nibss: %s", hexData);
	ShowLogs(1, "9C - Full response From Nibss: %s", &output[2]);
	
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	//HexEnCodeMethod(output, ucByte, hexData);
	//ShowLogs(1, "Response: %s\n", hexData);
	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	//How to read data
	int i = 62;
	if ( NULL != isoMsg.field[i].ptr ) 
	{
		DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(39, &isoHandler);
		sprintf(resp, "%s", isoMsg.field[39].ptr);
		ShowLogs(1, "Response Code: %s", resp);
		if(strstr(resp, "00") != NULL)
		{
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			memset(keyStore, '\0', strlen(keyFin)); 
			sprintf(keyStore, "%s", isoMsg.field[i].ptr);
			//ShowLogs(1, "Field 62, %s", keyStore);
			CreateWrite("param.txt", keyStore);
			DL_ISO8583_MSG_Free(&isoMsg);
			return parseParameters(keyStore);
		}
		DL_ISO8583_MSG_Free(&isoMsg);
		return 0;
	}else
		//printf("Field %d is not present in ISO\n", i);
		ShowLogs(1, "Field %d is not present in ISO\n", i);
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}

int GetParaMetersTest(char *resCode)
{
    char hexData[5 * 1024] = { 0 };
    char output[5 * 1024] = { 0 };
    DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
    /* get ISO-8583 1987 handler */
    DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
    char timeGotten[15] = {0};
    char datetime[11] = {0};
    char dt[5] = {0};
    char tm[7] = {0};
    int iRet, iLen = 0;
    char temp[128] = {0};
    char keyStore[128] = {0};
    char keyFin[33] = {0};
    char tempStore[128] = {0};
    char resp[3] = {0};
    char SN[33] = {0};
    char theUHI[33] = {0};
    char theUHISend[33] = {0};
    context_sha256_t ctx;
    char tempOut[100] = { '\0' };
    char boutHash[100] = { 0x0 };
    char outHash[100] = { 0x0 };
    char stan[7] = {0};

    if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

    memset(packBuf, 0x0, sizeof(packBuf));

    SysGetTimeIso(timeGotten);
    strncpy(datetime, timeGotten + 2, 10);
    strncpy(dt, timeGotten + 2, 4);
    strncpy(tm, timeGotten + 6, 6);

    /* initialise ISO message */
    DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

    /* set ISO message fields */
    (void)DL_ISO8583_MSG_SetField_Str(0, "0800", &isoMsg);
    (void)DL_ISO8583_MSG_SetField_Str(3, "9C0000", &isoMsg);
    (void)DL_ISO8583_MSG_SetField_Str(7, datetime, &isoMsg);
    sprintf((char *)stan, "%06lu", useStan);  
    (void)DL_ISO8583_MSG_SetField_Str(11, stan, &isoMsg);
    GetStan();
    (void)DL_ISO8583_MSG_SetField_Str(12, tm, &isoMsg);
    (void)DL_ISO8583_MSG_SetField_Str(13, dt, &isoMsg);
    memset(temp, '\0', strlen(temp)); 
    UtilGetEnv("tid", temp);
    (void)DL_ISO8583_MSG_SetField_Str(41, temp, &isoMsg);
    memset(SN, '\0', strlen(SN)); 
    ReadSN(SN);
    if ('\0' == SN[0]) {
        //No serial
        strcpy(theUHI, "000000009");
    }
    strcpy(theUHI, SN);
    sprintf(theUHISend, "01%03d%s", strlen(theUHI), theUHI);
    (void)DL_ISO8583_MSG_SetField_Str(62, theUHISend, &isoMsg);
    (void) DL_ISO8583_MSG_SetField_Str(64, 0x0, &isoMsg);
    
    sha256_starts(&ctx);
    (void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
    HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
    ShowLogs(1, "Packed size %d", packedSize);

    if (packedSize >= 64) 
    {
        packBuf[packedSize - 64] = '\0';
        ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
        memset(temp, '\0', strlen(temp));
        ReadAllData("isosesskey.txt", temp);
        ShowLogs(1, "Session key used for Hashing: %s", temp);
        memset(tempOut, 0x0, sizeof(tempOut));
        HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
        sha256_update(&ctx, (uint8_ts*) tempOut, 16);
        sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
        sha256_finish(&ctx, (uint8_ts*) boutHash);
        HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
        (void) DL_ISO8583_MSG_SetField_Str(64, outHash, &isoMsg);
        memset(packBuf, 0x0, sizeof(packBuf));
        (void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
        packBuf[0] = packedSize >> 8;
        packBuf[1] = packedSize;
        DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
        DL_ISO8583_MSG_Free(&isoMsg);
    }

    //For Gprs
    memset(temp, '\0', strlen(temp)); 
    UtilGetEnv("cotype", temp);
    if(strstr(temp, "GPRS") != NULL)
    {
        glSysParam.stTxnCommCfg.ucCommType = 5;
        memset(temp, '\0', strlen(temp));
        UtilGetEnv("coapn", temp);
        memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
        memset(temp, '\0', strlen(temp));
        UtilGetEnv("cosubnet", temp);
        memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
        memset(temp, '\0', strlen(temp));
        UtilGetEnv("copwd", temp);
        memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
        memset(temp, '\0', strlen(temp));
        UtilGetEnvEx("uhostip", temp);
        memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
        memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
        memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
        memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
        memset(temp, '\0', strlen(temp));
        UtilGetEnvEx("uhostport", temp);
        memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
        memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
        memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
        memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
        memset(temp, '\0', strlen(temp));
        UtilGetEnvEx("uhostssl", temp);
        if(strstr(temp, "true") != NULL)
        {
            ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
            ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
            ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
            ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
            ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
            ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
            ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
            
            EmvSetSSLFlag();
            DisplayInfoNone("Parameters", "Please Wait...", 0);
            iRet = CommInitModule(&glSysParam.stTxnCommCfg);
            ShowLogs(1, "CommsInitialization Response: %d", iRet);
            if (iRet != 0)
            {
                ShowLogs(1, "Comms Initialization failed.");
                DisplayInfoNone("Parameters", "Failed", 0);
                SxxSSLClose();
                return 0;
            }else
            {
                ShowLogs(1, "CommsInitialization Successful");
            }
            iRet = CommDial(DM_DIAL);
            ShowLogs(1, "CommsDial Response: %d", iRet);
            if (iRet != 0)
            {
                ShowLogs(1, "Comms Dial failed.");
                DisplayInfoNone("Parameters", "Failed", 0);
                SxxSSLClose();
                return 0;
            }else
            {
                ShowLogs(1, "CommDial Successful");
            }
            DisplayInfoNone("Parameters", "Sending...", 0);
            iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
            ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
            if (iRet < 0)
            {
                DisplayInfoNone("Parameters", "Failed", 0);
                ShowLogs(1, "SxxSSLTxd failed.");
                SxxSSLClose();
                return 0;
            }else
            {
                ShowLogs(1, "SxxSSLClose Successful");
            }
            DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
            iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
            ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
            ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
            if (iRet < 0)
            {
                DisplayInfoNone("Parameters", "Failed", 0);
                ShowLogs(1, "SxxSSLRxd failed.");
                SxxSSLClose();
                return 0;
            }else
            {
                ShowLogs(1, "SxxSSLRxd Successful");
            }
            SxxSSLClose();
        }else
        {   
            //For non ssl
            EmvUnsetSSLFlag();
            ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
            ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
            ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
            ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
            ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
            ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
            ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

            DisplayInfoNone("Parameters", "Please Wait...", 0);
            iRet = CommInitModule(&glSysParam.stTxnCommCfg);
            //ShowLogs(1, "CommsInitialization Response: %d", iRet);
            if (iRet != 0)
            {
                ShowLogs(1, "Comms Initialization failed.");
                DisplayInfoNone("Parameters", "Failed", 0);
                CommOnHookGPRS(TRUE);
                return 0;
            }
            iRet = CommDial(DM_DIAL);
            //ShowLogs(1, "CommsDial Response: %d", iRet);
            if (iRet != 0)
            {
                ShowLogs(1, "Comms Dial failed.");
                DisplayInfoNone("Parameters", "Failed", 0);
                CommOnHookGPRS(TRUE);
                return 0;
            }
            DisplayInfoNone("Parameters", "Sending...", 0);
            iRet = CommTxd(packBuf, packedSize + 2, 60);
            //ShowLogs(1, "CommTxd Response: %d", iRet);
            if (iRet != 0)
            {
                DisplayInfoNone("Parameters", "Failed", 0);
                ShowLogs(1, "CommTxd failed.");
                CommOnHookGPRS(TRUE);
                return 0;
            }
            DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
            iRet = CommRxd(output, 4 * 1024, 60, &iLen);
            //ShowLogs(1, "CommRxd Response: %d", iRet);
            if (iRet != 0)
            {
                DisplayInfoNone("Parameters", "Failed", 0);
                ShowLogs(1, "CommRxd failed.");
                CommOnHookGPRS(TRUE);
                return 0;
            }
            CommOnHookGPRS(TRUE);
        }
    }else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("Parameters", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("Parameters", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("Parameters", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("Parameters", "Failed", 0);
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Parameters", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			DisplayInfoNone("Parameters", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("Parameters", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}
    ScrBackLight(1);
    //Start here
    memset(hexData, '\0', strlen(hexData));
    HexEnCodeMethod(output, strlen(output) + 2, hexData);
    ShowLogs(1, "Received From Nibss: %s", hexData);
	ShowLogs(1, "Full response From Nibss: %s", &output[2]);

    
    unsigned char ucByte = 0;
    ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
    //HexEnCodeMethod(output, ucByte, hexData);
    //ShowLogs(1, "Response: %s\n", hexData);
    /* initialise ISO message */
    DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
    (void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
    DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
    //How to read data
    int i = 62;
    if ( NULL != isoMsg.field[i].ptr ) 
    {
        DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(39, &isoHandler);
        sprintf(resp, "%s", isoMsg.field[39].ptr);
        strcpy(resCode, resp);
        ShowLogs(1, "Response Code: %s", resp);
        if(strstr(resp, "00") != NULL)
        {
            DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
            memset(keyStore, '\0', strlen(keyFin)); 
            sprintf(keyStore, "%s", isoMsg.field[i].ptr);
            //ShowLogs(1, "Field 62, %s", keyStore);
            CreateWrite("param.txt", keyStore);
            DL_ISO8583_MSG_Free(&isoMsg);
            return parseParameters(keyStore);
        }
        DL_ISO8583_MSG_Free(&isoMsg);
        return 0;
    }else
        //printf("Field %d is not present in ISO\n", i);
        ShowLogs(1, "Field %d is not present in ISO\n", i);
    /* free ISO message */
    DL_ISO8583_MSG_Free(&isoMsg);
    return 0;
}

void parsetrack2(char *track, char *parse)
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

void customStorage(char *szLocalTime, char *reason)
{
	char szLT[14+1] = {0};
	ShowLogs(1, "About doing Forced Storage");
	if(strstr(reason, "100") != NULL)
	{
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", "88");
	}else if(strstr(reason, "101") != NULL)
	{
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", "87");
	}else if(strstr(reason, "102") != NULL)
	{
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", "89");
	}
	
	//UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);
	GetDateTime(szLT);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLT);
	sprintf(glRecvPack.szSTAN, "%s", glSendPack.szSTAN);
	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.2s", "NA");
	sprintf((char *)glRecvPack.szAuthCode, "%.2s", "NA");
	sprintf((char *)glRecvPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
	sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glSendPack.szRRN);
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	//sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);
	strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);
	
	ShowLogs(1, "About Storing Transaction.");
    storeTxn();
    ShowLogs(1, "About Storing Eod.");
	storeEod();
	ShowLogs(1, "About printing.");
	storeprint();

    memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
	memset(glRecvPack.szRspCode, 0x0, sizeof(glRecvPack.szRspCode));
	memset(glRecvPack.szSTAN, 0x0, sizeof(glRecvPack.szSTAN));
	memset(glRecvPack.szAuthCode, 0x0, sizeof(glRecvPack.szAuthCode));
    ShowLogs(1, "Done with Storing.");
}

void getTag(unsigned short Tag, char *output)
{
	char sTemp[128] = {0};
	char sStore[128] = {0};
	int	iRet = 0, iLength = 0;
	memset(sTemp, 0, sizeof(sTemp));
	memset(sStore, 0, sizeof(sStore));
	iRet = EMVGetTLVData(Tag, sTemp, &iLength);
	if( iRet == EMV_OK )
	{
		PubBcd2Asc(sTemp, iLength, sStore);
		ShowLogs(1, "Tag Gotten: %s", sStore);
		strcpy(output, sStore);
	}else
	{
		ShowLogs(1, "COULDNT GET TAG");
	}
}

void commaCheck(char *in, char* out)
{
	int i = 0, j = 0;
	char tme[100] = {0};
	int l = strlen(in);
	memset(tme, '\0', strlen(tme));
	for(i = 0; i < l; i++)
	{
		if(in[i] == ',')
			continue;
		tme[j] = in[i];
		j = j + 1;
	}
	strcpy(out, tme);
}

int cashSend(char *body, char *check)
{
	int iRet, iLen = 0;
	char output[1024] = { 0 };
	char temp[512] = { 0 };
	uchar	szLocalTime[14+1];
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	char out[3 * 1024] = { 0 };

	ShowLogs(1, "GOING: %s", body);

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOIP);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOPORT);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		if(1)
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
 			ShowLogs(1, "CommInitModule Start");
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			ShowLogs(1, "CommsDial Start");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(body, strlen(body), 180);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Receiving...", 0);
			memset(output, '\0', strlen(output));
			iRet = CommRxd(output, 1024, 180, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOIP);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOPORT);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		if(1)
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				return 0;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(body, strlen(body), 180);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Receiving...", 0);
			memset(output, '\0', strlen(output));
			iRet = CommRxd(output, 1024, 180, &iLen);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				return 0;
			}
			CommOnHook(TRUE);
 		}
	}

 	ShowLogs(1, "Done With Transfer");
	ScrBackLight(1);
 	ShowLogs(1, "Received From Interswitch: %s", output);
	Beep();
	if(strstr(output, check) != NULL)
	{
		strncpy(glRecvPack.szRspCode, "00", 2);
		return 1;
	}else
	{
		strncpy(glRecvPack.szRspCode, "06", 2);
		return 0;
	}
}

void cowryCash() {
	char temp[3*1024] = {0};
	char wallet[1*1024] = {0};
	char szTotalAmt[100] = {0};
	char szBuff[100] = {0};
	char dd[100] = {0};
	char amt[100] = {0};
	char tmp[100] = {0};

	memset(tmp, '\0', strlen(tmp));
	UtilGetEnvEx("tid", tmp);
	strcpy(glSendPack.szTermID, tmp);

	memset(temp, '\0', strlen(temp));
	strcpy(temp, "{");
	strcat(temp, "\"totalamount\": \"");

	memset(szTotalAmt, '\0', strlen(szTotalAmt));
	strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
	memset(szBuff, '\0', strlen(szBuff));
	App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	ShowLogs(1, "Converted Total: %s", szBuff);
	ShowLogs(1, "Converted Amount: %s", szBuff + 4);
	memset(amt, '\0', strlen(amt));
	commaCheck(szBuff + 4, amt);
	strcat(temp, amt);
	strcat(temp, "\",\"token\": \"");
	strcat(temp, "PAX");
	strcat(temp, "\",\"username\": \"");
	strcat(temp, "PAX");
	
	strcat(temp, "\",\"transaction\": \"");
	strcat(temp, "COWRY CARD");
	memset(wallet, '\0', strlen(wallet));
	strcpy(wallet, "{");
	strcat(wallet, "\"tid\": \"");
	strcat(wallet, glSendPack.szTermID);
	strcat(wallet, "\",\"etopfee\": \"");
	strcat(wallet, glSendPack.etopfee);
	strcat(wallet, "\",\"superagentfee\": \"");
	strcat(wallet, glSendPack.superagentfee);
	strcat(wallet, "\",\"aggregatorfee\": \"");
	strcat(wallet, glSendPack.aggregatorfee);
	
	memset(szTotalAmt, '\0', strlen(szTotalAmt));
	strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
	memset(szBuff, '\0', strlen(szBuff));
	App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	ShowLogs(1, "Converted Total: %s", szBuff);
	ShowLogs(1, "Converted Amount: %s", szBuff + 4);
	
	strcat(wallet, "\",\"amount\": \"");
	memset(amt, '\0', strlen(amt));
	commaCheck(szBuff + 4, amt);
	strcat(wallet, amt);
	strcat(wallet, "\",\"apikey\": \"");
	strcat(wallet, glSendPack.mainamount);
	strcat(wallet, "\",\"password\": \"");
	strcat(wallet, glSendPack.password);
	strcat(wallet, "\",\"rrn\": \"");
	strcat(wallet, glSendPack.szRRN);
	strcat(wallet, "\",\"pin\": \"");
	strcat(wallet, glSendPack.pin);
	strcat(wallet, "\",\"phone\": \"");
	strcat(wallet, glSendPack.destination);
	strcat(wallet, "\",\"email\": \"");
	strcat(wallet, glSendPack.description);
	strcat(wallet, "\",\"superagent\": \"");
	strcat(wallet, glSendPack.receivername);
	strcat(wallet, "\",\"mode\": \"");
	strcat(wallet, glSendPack.bankname);
	strcat(wallet, "\"}");
		
	ShowLogs(1, "WALLET: %s", wallet);

	strcat(temp, "\",\"mestype\": \"");
	strcat(temp, "ISO");
	strcat(temp, "\",\"hostip\": \"");
	memset(dd, '\0', strlen(dd));
	UtilGetEnvEx("uhostip", dd);
	strcat(temp, dd);
	strcat(temp, "\",\"hostport\": \"");
	memset(dd, '\0', strlen(dd));
	UtilGetEnvEx("uhostport", dd);
	strcat(temp, dd);
	strcat(temp, "\",\"hostssl\": \"");
	memset(dd, '\0', strlen(dd));
	UtilGetEnvEx("uhostssl", dd);
	strcat(temp, dd);
	
	strcat(temp, "\",\"tid\": \"");
	strcat(temp, glSendPack.szTermID);
	memset(dd, '\0', strlen(dd));
	ReadAllData("isosesskey.txt", dd);
	strcat(temp, "\",\"clrsesskey\": \"");
	strcat(temp, dd);
	memset(dd, '\0', strlen(dd));
	ReadAllData("isopinkey.txt", dd);
	strcat(temp, "\",\"clrpinkey\": \"");
	strcat(temp, dd);

	strcat(temp, "\",\"emvdata\": ");
	strcat(temp, "{}");
	strcat(temp, ",\"wallet\": ");
	strcat(temp, wallet);
	strcat(temp, "}\n");
	cashSend(temp, "TRANSACTION SUCCESSFUL");
}

void finishTxn(char *in, char *out, int control) {
	char temp[3*1024] = {0};
	char wallet[1*1024] = {0};
	char szTotalAmt[100] = {0};
	char szBuff[100] = {0};
	char dd[100] = {0};
	char amt[100] = {0};
	char tmp[100] = {0};

	memset(tmp, '\0', strlen(tmp));
	UtilGetEnvEx("tid", tmp);
	strcpy(glSendPack.szTermID, tmp);

	memset(temp, '\0', strlen(temp));
	strcpy(temp, "{");
	strcat(temp, "\"totalamount\": \"");

	memset(szTotalAmt, '\0', strlen(szTotalAmt));
	strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
	memset(szBuff, '\0', strlen(szBuff));
	App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
	ShowLogs(1, "Converted Total: %s", szBuff);
	ShowLogs(1, "Converted Amount: %s", szBuff + 4);
	memset(amt, '\0', strlen(amt));
	commaCheck(szBuff + 4, amt);
	strcat(temp, amt);
	strcat(temp, "\",\"token\": \"");
	strcat(temp, "PAX");
	strcat(temp, "\",\"username\": \"");
	strcat(temp, "PAX");

	if(txnType == 1 || txnType == 2 || txnType == 3 || txnType == 4
	|| txnType == 5 || txnType == 6 || txnType == 7 || txnType == 8)
	{
		strcat(temp, "\",\"transaction\": \"");
		if(txnType == 1)
			strcat(temp, "PURCHASE");
		else if(txnType == 2)
			strcat(temp, "CASH ADVANCE");
		else if(txnType == 3)
			strcat(temp, "PRE AUTH");
		else if(txnType == 4)
			strcat(temp, "REFUND");
		else if(txnType == 5)
			strcat(temp, "BALANCE ENQUIRY");
		else if(txnType == 6)
			strcat(temp, "SALES COMPLETION");
		else if(txnType == 7)
			strcat(temp, "REVERSAL");
		else
			strcat(temp, "CASH BACK");
		memset(wallet, '\0', strlen(wallet));
		strcpy(wallet, "{");
		strcat(wallet, "\"tid\": \"");
		strcat(wallet, glSendPack.szTermID);
		strcat(wallet, "\",\"etopfee\": \"");
		strcat(wallet, "");
		strcat(wallet, "\",\"superagentfee\": \"");
		strcat(wallet, "");
		strcat(wallet, "\",\"aggregatorfee\": \"");
		strcat(wallet, "");
		
		memset(szTotalAmt, '\0', strlen(szTotalAmt));
		strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
		memset(szBuff, '\0', strlen(szBuff));
		App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		ShowLogs(1, "Converted Total: %s", szBuff);
		ShowLogs(1, "Converted Amount: %s", szBuff + 4);
		
		strcat(wallet, "\",\"amount\": \"");
		memset(amt, '\0', strlen(amt));
		commaCheck(szBuff + 4, amt);
		strcat(wallet, amt);
		strcat(wallet, "\",\"mainamount\": \"");
		strcat(wallet, amt);
		strcat(wallet, "\",\"fee\": \"");
		strcat(wallet, "0.00");
		strcat(wallet, "\",\"rrn\": \"");
		strcat(wallet, glSendPack.szRRN);
		strcat(wallet, "\",\"bankcode\": \"");
		strcat(wallet, "");
		strcat(wallet, "\",\"destination\": \"");
		strcat(wallet, "");
		strcat(wallet, "\",\"description\": \"");
		strcat(wallet, "");
		strcat(wallet, "\",\"receivername\": \"");
		strcat(wallet, "");
		strcat(wallet, "\",\"bankname\": \"");
		strcat(wallet, "");
		strcat(wallet, "\"}");
	}else if(txnType == 16)
	{
		strcat(temp, "\",\"transaction\": \"");
		strcat(temp, "CASH DEPOSIT");
		memset(wallet, '\0', strlen(wallet));
		strcpy(wallet, "{");
		strcat(wallet, "\"tid\": \"");
		strcat(wallet, glSendPack.szTermID);
		strcat(wallet, "\",\"etopfee\": \"");
		strcat(wallet, glSendPack.etopfee);
		strcat(wallet, "\",\"superagentfee\": \"");
		strcat(wallet, glSendPack.superagentfee);
		strcat(wallet, "\",\"aggregatorfee\": \"");
		strcat(wallet, glSendPack.aggregatorfee);
		
		memset(szTotalAmt, '\0', strlen(szTotalAmt));
		strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
		memset(szBuff, '\0', strlen(szBuff));
		App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		ShowLogs(1, "Converted Total: %s", szBuff);
		ShowLogs(1, "Converted Amount: %s", szBuff + 4);
		
		strcat(wallet, "\",\"amount\": \"");
		memset(amt, '\0', strlen(amt));
		commaCheck(szBuff + 4, amt);
		strcat(wallet, amt);
		strcat(wallet, "\",\"mainamount\": \"");
		strcat(wallet, glSendPack.mainamount);
		strcat(wallet, "\",\"fee\": \"");
		strcat(wallet, glSendPack.fee);
		strcat(wallet, "\",\"rrn\": \"");
		strcat(wallet, glSendPack.szRRN);
		strcat(wallet, "\",\"bankcode\": \"");
		strcat(wallet, glSendPack.bankcode);
		strcat(wallet, "\",\"destination\": \"");
		strcat(wallet, glSendPack.destination);
		strcat(wallet, "\",\"description\": \"");
		strcat(wallet, glSendPack.description);
		strcat(wallet, "\",\"receivername\": \"");
		strcat(wallet, glSendPack.receivername);
		strcat(wallet, "\",\"bankname\": \"");
		strcat(wallet, glSendPack.bankname);
		strcat(wallet, "\"}");
	}else if(txnType == 17)
	{
		strcat(temp, "\",\"transaction\": \"");
		strcat(temp, "ACCOUNT TRANSFER");
		memset(wallet, '\0', strlen(wallet));
		strcpy(wallet, "{");
		strcat(wallet, "\"tid\": \"");
		strcat(wallet, glSendPack.szTermID);
		strcat(wallet, "\",\"etopfee\": \"");
		strcat(wallet, glSendPack.etopfee);
		strcat(wallet, "\",\"superagentfee\": \"");
		strcat(wallet, glSendPack.superagentfee);
		strcat(wallet, "\",\"aggregatorfee\": \"");
		strcat(wallet, glSendPack.aggregatorfee);
		
		memset(szTotalAmt, '\0', strlen(szTotalAmt));
		strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
		memset(szBuff, '\0', strlen(szBuff));
		App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		ShowLogs(1, "Converted Total: %s", szBuff);
		ShowLogs(1, "Converted Amount: %s", szBuff + 4);
		
		strcat(wallet, "\",\"amount\": \"");
		memset(amt, '\0', strlen(amt));
		commaCheck(szBuff + 4, amt);
		strcat(wallet, amt);
		strcat(wallet, "\",\"mainamount\": \"");
		strcat(wallet, glSendPack.mainamount);
		strcat(wallet, "\",\"fee\": \"");
		strcat(wallet, glSendPack.fee);
		strcat(wallet, "\",\"rrn\": \"");
		strcat(wallet, glSendPack.szRRN);
		strcat(wallet, "\",\"bankcode\": \"");
		strcat(wallet, glSendPack.bankcode);
		strcat(wallet, "\",\"destination\": \"");
		strcat(wallet, glSendPack.destination);
		strcat(wallet, "\",\"description\": \"");
		strcat(wallet, glSendPack.description);
		strcat(wallet, "\",\"receivername\": \"");
		strcat(wallet, glSendPack.receivername);
		strcat(wallet, "\",\"bankname\": \"");
		strcat(wallet, glSendPack.bankname);
		strcat(wallet, "\"}");
	}else if(txnType == 18)
	{
		strcat(temp, "\",\"transaction\": \"");
		strcat(temp, "CASH WITHDRAWAL");
		memset(wallet, '\0', strlen(wallet));
		strcpy(wallet, "{");
		strcat(wallet, "\"tid\": \"");
		strcat(wallet, glSendPack.szTermID);
		strcat(wallet, "\",\"etopfee\": \"");
		strcat(wallet, glSendPack.etopfee);
		strcat(wallet, "\",\"superagentfee\": \"");
		strcat(wallet, glSendPack.superagentfee);
		strcat(wallet, "\",\"aggregatorfee\": \"");
		strcat(wallet, glSendPack.aggregatorfee);
		
		memset(szTotalAmt, '\0', strlen(szTotalAmt));
		strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
		memset(szBuff, '\0', strlen(szBuff));
		App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		ShowLogs(1, "Converted Total: %s", szBuff);
		ShowLogs(1, "Converted Amount: %s", szBuff + 4);
		
		strcat(wallet, "\",\"amount\": \"");
		memset(amt, '\0', strlen(amt));
		commaCheck(szBuff + 4, amt);
		strcat(wallet, amt);
		strcat(wallet, "\",\"mainamount\": \"");
		strcat(wallet, glSendPack.mainamount);
		strcat(wallet, "\",\"fee\": \"");
		strcat(wallet, glSendPack.fee);
		strcat(wallet, "\",\"rrn\": \"");
		strcat(wallet, glSendPack.szRRN);
		strcat(wallet, "\",\"bankcode\": \"");
		strcat(wallet, glSendPack.bankcode);
		strcat(wallet, "\",\"destination\": \"");
		strcat(wallet, glSendPack.destination);
		strcat(wallet, "\",\"description\": \"");
		strcat(wallet, glSendPack.description);
		strcat(wallet, "\",\"receivername\": \"");
		strcat(wallet, glSendPack.receivername);
		strcat(wallet, "\",\"bankname\": \"");
		strcat(wallet, glSendPack.bankname);
		strcat(wallet, "\"}");
	}
	//19 is BILLS PAYMENT
	else if(txnType == 22)
	{
		strcat(temp, "\",\"transaction\": \"");
		strcat(temp, "COWRY CARD");
		memset(wallet, '\0', strlen(wallet));
		strcpy(wallet, "{");
		strcat(wallet, "\"tid\": \"");
		strcat(wallet, glSendPack.szTermID);
		strcat(wallet, "\",\"etopfee\": \"");
		strcat(wallet, glSendPack.etopfee);
		strcat(wallet, "\",\"superagentfee\": \"");
		strcat(wallet, glSendPack.superagentfee);
		strcat(wallet, "\",\"aggregatorfee\": \"");
		strcat(wallet, glSendPack.aggregatorfee);
		
		memset(szTotalAmt, '\0', strlen(szTotalAmt));
		strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
		memset(szBuff, '\0', strlen(szBuff));
		App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		ShowLogs(1, "Converted Total: %s", szBuff);
		ShowLogs(1, "Converted Amount: %s", szBuff + 4);
		
		strcat(wallet, "\",\"amount\": \"");
		memset(amt, '\0', strlen(amt));
		commaCheck(szBuff + 4, amt);
		strcat(wallet, amt);
		strcat(wallet, "\",\"apikey\": \"");
		strcat(wallet, glSendPack.mainamount);
		strcat(wallet, "\",\"password\": \"");
		strcat(wallet, glSendPack.fee);
		strcat(wallet, "\",\"rrn\": \"");
		strcat(wallet, glSendPack.szRRN);
		strcat(wallet, "\",\"pin\": \"");
		strcat(wallet, glSendPack.bankcode);
		strcat(wallet, "\",\"phone\": \"");
		strcat(wallet, glSendPack.destination);
		strcat(wallet, "\",\"email\": \"");
		strcat(wallet, glSendPack.description);
		strcat(wallet, "\",\"superagent\": \"");
		strcat(wallet, glSendPack.receivername);
		strcat(wallet, "\",\"mode\": \"");
		strcat(wallet, glSendPack.bankname);
		strcat(wallet, "\"}");
	}else if(txnType == 23)
	{
		strcat(temp, "\",\"transaction\": \"");
		strcat(temp, "TAX TRANSACTION");
		memset(wallet, '\0', strlen(wallet));
		strcpy(wallet, "{");
		strcat(wallet, "\"tid\": \"");
		strcat(wallet, glSendPack.szTermID);
		strcat(wallet, "\",\"etopfee\": \"");
		strcat(wallet, glSendPack.etopfee);
		strcat(wallet, "\",\"superagentfee\": \"");
		strcat(wallet, glSendPack.superagentfee);
		strcat(wallet, "\",\"aggregatorfee\": \"");
		strcat(wallet, glSendPack.aggregatorfee);
		
		memset(szTotalAmt, '\0', strlen(szTotalAmt));
		strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
		memset(szBuff, '\0', strlen(szBuff));
		App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		ShowLogs(1, "Converted Total: %s", szBuff);
		ShowLogs(1, "Converted Amount: %s", szBuff + 4);
		
		strcat(wallet, "\",\"amount\": \"");
		memset(amt, '\0', strlen(amt));
		commaCheck(szBuff + 4, amt);
		strcat(wallet, amt);
		strcat(wallet, "\",\"mainamount\": \"");
		strcat(wallet, glSendPack.mainamount);
		strcat(wallet, "\",\"fee\": \"");
		strcat(wallet, glSendPack.fee);
		strcat(wallet, "\",\"rrn\": \"");
		strcat(wallet, glSendPack.szRRN);
		strcat(wallet, "\",\"bankcode\": \"");
		strcat(wallet, glSendPack.bankcode);
		strcat(wallet, "\",\"invoicenumber\": \"");
		strcat(wallet, glSendPack.destination);
		strcat(wallet, "\",\"description\": \"");
		strcat(wallet, glSendPack.description);
		strcat(wallet, "\",\"receivername\": \"");
		strcat(wallet, glSendPack.receivername);
		strcat(wallet, "\",\"bankname\": \"");
		strcat(wallet, glSendPack.bankname);
		strcat(wallet, "\"}");
	}else
	{
		strcat(temp, "\",\"transaction\": \"");
		strcat(temp, "PURCHASE");
		memset(wallet, '\0', strlen(wallet));
		strcpy(wallet, "{");
		strcat(wallet, "\"tid\": \"");
		strcat(wallet, glSendPack.szTermID);
		strcat(wallet, "\",\"etopfee\": \"");
		strcat(wallet, glSendPack.etopfee);
		strcat(wallet, "\",\"superagentfee\": \"");
		strcat(wallet, glSendPack.superagentfee);
		strcat(wallet, "\",\"aggregatorfee\": \"");
		strcat(wallet, glSendPack.aggregatorfee);
		
		memset(szTotalAmt, '\0', strlen(szTotalAmt));
		strcpy(szTotalAmt, glProcInfo.stTranLog.szAmount);
		memset(szBuff, '\0', strlen(szBuff));
		App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		ShowLogs(1, "Converted Total: %s", szBuff);
		ShowLogs(1, "Converted Amount: %s", szBuff + 4);
		
		strcat(wallet, "\",\"amount\": \"");
		memset(amt, '\0', strlen(amt));
		commaCheck(szBuff + 4, amt);
		strcat(wallet, amt);
		strcat(wallet, "\",\"mainamount\": \"");
		strcat(wallet, amt);
		strcat(wallet, "\",\"fee\": \"");
		strcat(wallet, "0.00");
		strcat(wallet, "\",\"rrn\": \"");
		strcat(wallet, glSendPack.szRRN);
		strcat(wallet, "\",\"bankcode\": \"");
		strcat(wallet, glSendPack.bankcode);
		strcat(wallet, "\",\"destination\": \"");
		strcat(wallet, glSendPack.destination);
		strcat(wallet, "\",\"description\": \"");
		strcat(wallet, glSendPack.description);
		strcat(wallet, "\",\"receivername\": \"");
		strcat(wallet, glSendPack.receivername);
		strcat(wallet, "\",\"bankname\": \"");
		strcat(wallet, glSendPack.bankname);
		strcat(wallet, "\"}");
	}
	ShowLogs(1, "WALLET: %s", wallet);

	strcat(temp, "\",\"mestype\": \"");
	strcat(temp, "ISO");
	strcat(temp, "\",\"hostip\": \"");
	memset(dd, '\0', strlen(dd));
	UtilGetEnvEx("uhostip", dd);
	strcat(temp, dd);
	strcat(temp, "\",\"hostport\": \"");
	memset(dd, '\0', strlen(dd));
	UtilGetEnvEx("uhostport", dd);
	strcat(temp, dd);
	strcat(temp, "\",\"hostssl\": \"");
	memset(dd, '\0', strlen(dd));
	UtilGetEnvEx("uhostssl", dd);
	strcat(temp, dd);
	
	strcat(temp, "\",\"tid\": \"");
	strcat(temp, glSendPack.szTermID);
	memset(dd, '\0', strlen(dd));
	ReadAllData("isosesskey.txt", dd);
	strcat(temp, "\",\"clrsesskey\": \"");
	strcat(temp, dd);
	memset(dd, '\0', strlen(dd));
	ReadAllData("isopinkey.txt", dd);
	strcat(temp, "\",\"clrpinkey\": \"");
	strcat(temp, dd);

	strcat(temp, "\",\"emvdata\": ");
	strcat(temp, in);
	
	strcat(temp, ",\"wallet\": ");
	strcat(temp, wallet);
	strcat(temp, "}\n");
	strcpy(out, temp);
}

void checkCardStatus(char *out)
{
	char headers[2 * 1024] = {0};
	memset(headers, '\0', strlen(headers));
	strcpy(headers, "tid: ");
	strcat(headers, glSendPack.szTermID);
	strcat(headers, "\r\n");

	strcat(headers, "rrn: ");
	strcat(headers, glSendPack.szRRN);
	strcat(headers, "\r\n");

	
}

int lastUsedHost = 0;
int checkVerve = 0;

int SendEmvData(char *icc, int *reversal)
{
	#ifndef REGULAR
		if(checkMimic == 1)
		{
			txnType = 7;
			SendManualReversal();
			return 0;
		}
	#endif
	

	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/*get ISO-8583 1987 handler*/
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char temp2[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char dataStore[254] = {0};
	uchar szLocalTime[14+1];
	uchar outputData[10240] = {0};
	char parseTrack2[40] = {0};
	int iTemp;

	ShowLogs(1, "INSIDE SEND EMV DATA");
	checkVerve = 0;

	ShowLogs(1, "PAN: %s", glSendPack.szPan);

	titi = 0;
	rvApr = 0;


	
	
	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));


	memset(packBuf, 0x0, sizeof(packBuf));
	//For sales completion
	#ifndef REGULAR
		if(txnType == 6)
		{
			ShowLogs(1, "sales completion txn but dont check");
			/*if(parseEod(glSendPack.szRRN, glSendPack.szAuthCode) != 1)
			{
				sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", "");
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", "");
				sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", "");
				sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  "");
				sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", "");
				return -1;
			}*/
		}else
		{
			ShowLogs(1, "Not a sales completion txn");
		}
	#endif
	

	SysGetTimeIso(timeGotten);
	//memset(glSendPack.szRRN, '\0', strlen(glSendPack.szRRN));
	//sprintf((char *)glSendPack.szRRN, "%.*s", 12, timeGotten);


	strncpy(datetime, timeGotten + 2, 10);
	sprintf((char *)glSendPack.szLocalDateTime, "%.*s", LEN_LOCAL_DATE_TIME, datetime);
	strncpy(dt, timeGotten + 2, 4);
	sprintf((char *)glSendPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dt);
	strncpy(tm, timeGotten + 6, 6);
	sprintf((char *)glSendPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, tm);

	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
	memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
	memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
	memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
	memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
	memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
	memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	if(strlen(glSendPack.szMsgCode) > 0)
	{
		ShowLogs(1, "Getting mti for normal cards: %s", glSendPack.szMsgCode);
		(void)DL_ISO8583_MSG_SetField_Str(0, glSendPack.szMsgCode, &isoMsg);
	}
	if(strlen(glSendPack.szPan) > 0)
	{
		ShowLogs(1, "PAN: %s", glSendPack.szPan);
		(void)DL_ISO8583_MSG_SetField_Str(2, glSendPack.szPan, &isoMsg);
	}
	if(strlen(glSendPack.szProcCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(3, glSendPack.szProcCode, &isoMsg);
		ShowLogs(1, "FIELD 4: %s", glSendPack.szProcCode);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szProcCode), glSendPack.szProcCode);
	}
	if(strlen(glSendPack.szTranAmt) > 0)
	{
		char stamp[20] = {0};
		char tt[20] = {0};
		parseAmount(glSendPack.szTranAmt, tt);
		ShowLogs(1, "Converted Amount: %s.", tt);
	    double tot = atof(tt);
	    UtilGetEnvEx("stampduty", stamp);
	    ShowLogs(1, "Stampduty: %s.", stamp);
	    if((strstr(stamp, "true") != NULL)
	      && (tot >= 1000)
	      && (glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	    {
	    	char fina[13] = {0};
	    	memset(fina, '\0', strlen(fina));
		    PubAscAdd(glSendPack.szTranAmt, "000000005000", 12, fina);
		    ShowLogs(1, "Total: %s", fina);
		    (void)DL_ISO8583_MSG_SetField_Str(4, fina, &isoMsg);
	    }else
			(void)DL_ISO8583_MSG_SetField_Str(4, glSendPack.szTranAmt, &isoMsg);
	}
	if(strlen(glSendPack.szLocalDateTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(7, glSendPack.szLocalDateTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDateTime), glSendPack.szLocalDateTime);
	}
	if(strlen(glSendPack.szSTAN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(11, glSendPack.szSTAN, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szSTAN), glSendPack.szSTAN);
	}
	GetStan();
	if(strlen(glSendPack.szLocalTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(12, glSendPack.szLocalTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalTime), glSendPack.szLocalTime);
	}
	if(strlen(glSendPack.szLocalDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(13, glSendPack.szLocalDate, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDate), glSendPack.szLocalDate);
	}
	if(strlen(glSendPack.szExpDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(14, glSendPack.szExpDate, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szExpDate), glSendPack.szExpDate);
	}
	if(strlen(glSendPack.szMerchantType) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(18, glSendPack.szMerchantType, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantType), glSendPack.szMerchantType);
	}
	if(strlen(glSendPack.szEntryMode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(22, glSendPack.szEntryMode + 1, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szEntryMode), glSendPack.szEntryMode);
	}
	if(strlen(glSendPack.szPanSeqNo) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(23, glSendPack.szPanSeqNo, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPanSeqNo), glSendPack.szPanSeqNo);
	}
	if(strlen(glSendPack.szCondCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(25, glSendPack.szCondCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szCondCode), glSendPack.szCondCode);
	}
	if(strlen(glSendPack.szPoscCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(26, glSendPack.szPoscCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPoscCode), glSendPack.szPoscCode);
	}
	if(strlen(glSendPack.szTransFee) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(28, glSendPack.szTransFee, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTransFee), glSendPack.szTransFee);
	}
	if(strlen(glSendPack.szAqurId) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(32, glSendPack.szAqurId, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAqurId), glSendPack.szAqurId);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szTrack2) > 0)
	{
		memset(parseTrack2, '\0', strlen(parseTrack2));
		parsetrack2(glSendPack.szTrack2, parseTrack2);
		(void)DL_ISO8583_MSG_SetField_Str(35, parseTrack2, &isoMsg);

		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(parseTrack2), parseTrack2);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szRRN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(37, glSendPack.szRRN, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szRRN), glSendPack.szRRN);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szAuthCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(38, glSendPack.szAuthCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAuthCode), glSendPack.szAuthCode);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szServResCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(40, glSendPack.szServResCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szServResCode), glSendPack.szServResCode);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szTermID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(41, glSendPack.szTermID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTermID), glSendPack.szTermID);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szMerchantID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(42, glSendPack.szMerchantID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantID), glSendPack.szMerchantID);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szMNL) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMNL), glSendPack.szMNL);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szTranCurcyCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(49, glSendPack.szTranCurcyCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTranCurcyCode), glSendPack.szTranCurcyCode);
		//DisplayInfoNone("", temp2, 3);
	}
	if(pincheck == 1)
	{
		if(strlen(glSendPack.szPinBlock) > 0)
		{
			(void)DL_ISO8583_MSG_SetField_Str(52, glSendPack.szPinBlock, &isoMsg);
		}
		pincheck = 0;
	}
	if(strlen(glSendPack.testSICCData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, glSendPack.testSICCData, &isoMsg);
	}
	
	/*if(icc != NULL)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, icc, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(icc), icc);
		//DisplayInfoNone("", temp2, 3);
	}*/
	if(strlen(glSendPack.szReasonCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(56, glSendPack.szReasonCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReasonCode), glSendPack.szReasonCode);
		//DisplayInfoNone("", temp2, 3);
	}

	if(strlen(glSendPack.szTEchoData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(59, glSendPack.szTEchoData, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTEchoData), glSendPack.szTEchoData);//log
		//DisplayInfoNone("", temp2, 3);
	}
	//Added for vas
	if(strlen(glSendPack.szBillers) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(62, glSendPack.szBillers, &isoMsg);
	}

	if(strlen(glSendPack.szOrigDataElement) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(90, glSendPack.szOrigDataElement, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d -- %s", strlen(glSendPack.szOrigDataElement), glSendPack.szOrigDataElement);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szReplAmount) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(95, glSendPack.szReplAmount, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReplAmount), glSendPack.szReplAmount);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szPosDataCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(123, glSendPack.szPosDataCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPosDataCode), glSendPack.szPosDataCode);
		//DisplayInfoNone("", temp2, 3);
	}
	(void) DL_ISO8583_MSG_SetField_Str(128, 0x0, &isoMsg);

	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);
	ShowLogs(1, "Iso Message With Length: %s", hexData);

	if (packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(128, outHash, &isoMsg);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}
	//Fine till here

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
			//Investigate from here tomorrow
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				ShowLogs(1, "Ima Mmi: %d", reversal);
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
 			ShowLogs(1, "CommInitModule Start");
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			ShowLogs(1, "CommsDial Start");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}


	
 	ShowLogs(1, "Done With Nibss");
	ScrBackLight(1);
 	memset(hexData, '\0', strlen(hexData));
	HexEnCodeMethod(output, strlen(output) + 2, hexData);
	ShowLogs(1, "Received From Nibss: %s", hexData);
	
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	
	
	int i = 0;

	//memset(&glRecvPack, 0, sizeof(STISO8583));//This is the proble
	
	//ShowLogs(1, "Commented out");
	for(i = 0; i < 129; i++)
	{
		if ( NULL != isoMsg.field[i].ptr ) 
		{
			memset(dataStore, '\0', strlen(dataStore));
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			sprintf(dataStore, "%s", isoMsg.field[i].ptr);
			switch(i)
			{
				case 0:
					memset(glRecvPack.szMsgCode, '\0', strlen(glRecvPack.szMsgCode));
					sprintf((char *)glRecvPack.szMsgCode, "%.*s", LEN_MSG_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMsgCode);
					continue;
				case 1:
					memset(glRecvPack.sBitMap, '\0', strlen(glRecvPack.sBitMap));
					sprintf((char *)glRecvPack.sBitMap, "%.*s", 2*LEN_BITMAP, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sBitMap);
					continue;
				case 2:
					memset(glRecvPack.szPan, '\0', strlen(glRecvPack.szPan));
					sprintf((char *)glRecvPack.szPan, "%.*s", LEN_PAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPan);
					continue;
				case 3:
					memset(glRecvPack.szProcCode, '\0', strlen(glRecvPack.szProcCode));
					sprintf((char *)glRecvPack.szProcCode, "%.*s", LEN_PROC_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szProcCode);
					continue;
				case 4:
					memset(glRecvPack.szTranAmt, '\0', strlen(glRecvPack.szTranAmt));
					sprintf((char *)glRecvPack.szTranAmt, "%.*s", LEN_TRAN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranAmt);
					continue;
				case 7:
					memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
					sprintf((char *)glRecvPack.szFrnAmt, "%.*s", LEN_FRN_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFrnAmt);
					continue;
				case 11:
					memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
					sprintf((char *)glRecvPack.szSTAN, "%.*s", LEN_STAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSTAN);
					continue;
				case 12:
					memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
					sprintf((char *)glRecvPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalTime);
					continue;
				case 13:
					memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
					sprintf((char *)glRecvPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalDate);
					continue;
				case 14:
					memset(glRecvPack.szExpDate, '\0', strlen(glRecvPack.szExpDate));
					sprintf((char *)glRecvPack.szExpDate, "%.*s", LEN_EXP_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szExpDate);
					continue;
				case 15:
					memset(glRecvPack.szSetlDate, '\0', strlen(glRecvPack.szSetlDate));
					sprintf((char *)glRecvPack.szSetlDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSetlDate);
					continue;
				case 18:
					memset(glRecvPack.szMerchantType, '\0', strlen(glRecvPack.szMerchantType));
					sprintf((char *)glRecvPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantType);
					continue;
				case 22:
					memset(glRecvPack.szEntryMode, '\0', strlen(glRecvPack.szEntryMode));
					sprintf((char *)glRecvPack.szEntryMode, "%.*s", LEN_ENTRY_MODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szEntryMode);
					continue;
				case 23:
					memset(glRecvPack.szPanSeqNo, '\0', strlen(glRecvPack.szPanSeqNo));
					sprintf((char *)glRecvPack.szPanSeqNo, "%.*s", LEN_PAN_SEQ_NO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPanSeqNo);
					continue;
				case 25:
					memset(glRecvPack.szCondCode, '\0', strlen(glRecvPack.szCondCode));
					sprintf((char *)glRecvPack.szCondCode, "%.*s", LEN_COND_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szCondCode);
					continue;
				case 28:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 30:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 32:
					memset(glRecvPack.szAqurId, '\0', strlen(glRecvPack.szAqurId));
					sprintf((char *)glRecvPack.szAqurId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAqurId);
					continue;
				case 33:
					memset(glRecvPack.szFwdInstId, '\0', strlen(glRecvPack.szFwdInstId));
					sprintf((char *)glRecvPack.szFwdInstId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFwdInstId);
					continue;
				case 35:
					memset(glRecvPack.szTrack2, '\0', strlen(glRecvPack.szTrack2));
					sprintf((char *)glRecvPack.szTrack2, "%.*s", LEN_TRACK2, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTrack2);
					continue;
				case 37:
					memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
					sprintf((char *)glRecvPack.szRRN, "%.*s", LEN_RRN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRRN);
					continue;
				case 38:
					memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
					sprintf((char *)glRecvPack.szAuthCode, "%.*s", LEN_AUTH_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAuthCode);
					continue;
				case 39:
					memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
					sprintf((char *)glRecvPack.szRspCode, "%.*s", LEN_RSP_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRspCode);
					continue;
				case 40:
					memset(glRecvPack.szServResCode, '\0', strlen(glRecvPack.szServResCode));
					sprintf((char *)glRecvPack.szServResCode, "%.*s", LEN_SRES_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szServResCode);
					continue;
				case 41:
					memset(glRecvPack.szTermID, '\0', strlen(glRecvPack.szTermID));
					sprintf((char *)glRecvPack.szTermID, "%.*s", LEN_TERM_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTermID);
					continue;
				case 42:
					memset(glRecvPack.szMerchantID, '\0', strlen(glRecvPack.szMerchantID));
					sprintf((char *)glRecvPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantID);
					continue;
				case 43:
					memset(glRecvPack.szMNL, '\0', strlen(glRecvPack.szMNL));
					sprintf((char *)glRecvPack.szMNL, "%.*s", LEN_MNL, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMNL);
					continue;
				case 49:
					memset(glRecvPack.szTranCurcyCode, '\0', strlen(glRecvPack.szTranCurcyCode));
					sprintf((char *)glRecvPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranCurcyCode);
					continue;
				case 54:
					memset(glRecvPack.szAddtAmount, '\0', strlen(glRecvPack.szAddtAmount));
					sprintf((char *)glRecvPack.szAddtAmount, "%.*s", LEN_ADDT_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAddtAmount);
					continue;
				case 55:
					memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
					sprintf((char *)glRecvPack.sICCData, "%.*s", LEN_ICC_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sICCData);
					continue;
				case 59:
					memset(glRecvPack.szTEchoData, '\0', strlen(glRecvPack.szTEchoData));
					sprintf((char *)glRecvPack.szTEchoData, "%.*s", LEN_TRANSECHO_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTEchoData);
					continue;
				case 60:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 62:
					memset(glRecvPack.szBillers, '\0', strlen(glRecvPack.szBillers));
					sprintf((char *)glRecvPack.szBillers, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szBillers);
					continue;
				case 102:
					memset(glRecvPack.szActIdent1, '\0', strlen(glRecvPack.szActIdent1));
					sprintf((char *)glRecvPack.szActIdent1, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent1);
					continue;
				case 103:
					memset(glRecvPack.szActIdent2, '\0', strlen(glRecvPack.szActIdent2));
					sprintf((char *)glRecvPack.szActIdent2, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent2);
					continue;
				case 123:
					memset(glRecvPack.szPosDataCode, '\0', strlen(glRecvPack.szPosDataCode));
					sprintf((char *)glRecvPack.szPosDataCode, "%.*s", LEN_POS_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPosDataCode);
					continue;
				case 124:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 128:
					memset(glRecvPack.szNFC, '\0', strlen(glRecvPack.szNFC));
					sprintf((char *)glRecvPack.szNFC, "%.*s", LEN_NEARFIELD, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szNFC);
					continue;
				default:
					continue;
			}
		}
	}
	ShowLogs(1, "Response Code: %s", glRecvPack.szRspCode);
	

	sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
	//UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
	sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);
	FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
	strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);
	
	ShowLogs(1, "2. Response Code: %s", glRecvPack.szRspCode);

	ShowLogs(1, "Commencing Storage.");
	if(strlen(glRecvPack.szRspCode) == 2)
    {
    	if(txnType != 5)
    	{
	    	ShowLogs(1, "About Storing Transaction.");
	    	storeTxn();
	    	ShowLogs(1, "About Storing Eod.");
			storeEod();
			ShowLogs(1, "About printing.");
			storeprint();
		}
    }else
    {
    	customStorage(szLocalTime, "100");
    	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
    	//No response
    	titi = 1;
    }
    ShowLogs(1, "Done with Storing. Modify");
    ShowLogs(1, "3. Response Code: %s", glRecvPack.szRspCode);
	
	if(PURCHASETYPE==4)
	{
	eedcPayment();
	}
		if(PURCHASETYPE==3)
	{
	jambpayment();
	}
	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		DisplayInfoNone("", "Trans Success", 1);
		//PackCallHomeData();
	}else
	{
		DisplayInfoNone("", "Trans Declined", 1);
		//PackCallHomeData();



		if(strcmp(glRecvPack.szRspCode, "") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "06") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "96") == 0)
		{
			//UpdateProfile();
		}


		if(strcmp(glRecvPack.szRspCode, "86") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "87") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "88") == 0)
		{
			UpdateProfile();
		}

		if(strcmp(glRecvPack.szRspCode, "89") == 0)
		{
			UpdateProfile();
		}
		
	}

    
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}




int SendEmvDataXML_WORKING(char *icc, int *reversal)
{
	#ifndef REGULAR
		if(checkMimic == 1)
		{
			txnType = 7;
			SendManualReversal();
			return 0;
		}
	#endif
	

	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char temp2[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char dataStore[254] = {0};
	uchar	szLocalTime[14+1];
	uchar outputData[10240] = {0};
	char parseTrack2[40] = {0};
	int iTemp;

	ShowLogs(1, "INSIDE SEND EMV DATA");
	checkVerve = 0;

	ShowLogs(1, "PAN: %s", glSendPack.szPan);

	titi = 0;
	rvApr = 0;


	
	
	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));


	memset(packBuf, 0x0, sizeof(packBuf));
	//For sales completion
	#ifndef REGULAR
		if(txnType == 6)
		{
			ShowLogs(1, "sales completion txn but dont check");
			/*if(parseEod(glSendPack.szRRN, glSendPack.szAuthCode) != 1)
			{
				sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", "");
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", "");
				sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", "");
				sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  "");
				sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", "");
				return -1;
			}*/
		}else
		{
			ShowLogs(1, "Not a sales completion txn");
		}
	#endif
	

	SysGetTimeIso(timeGotten);
	//memset(glSendPack.szRRN, '\0', strlen(glSendPack.szRRN));
	//sprintf((char *)glSendPack.szRRN, "%.*s", 12, timeGotten);


	strncpy(datetime, timeGotten + 2, 10);
	sprintf((char *)glSendPack.szLocalDateTime, "%.*s", LEN_LOCAL_DATE_TIME, datetime);
	strncpy(dt, timeGotten + 2, 4);
	sprintf((char *)glSendPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dt);
	strncpy(tm, timeGotten + 6, 6);
	sprintf((char *)glSendPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, tm);

	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
	memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
	memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
	memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
	memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
	memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
	memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	if(strlen(glSendPack.szMsgCode) > 0)
	{
		ShowLogs(1, "Getting mti for normal cards: %s", glSendPack.szMsgCode);
		(void)DL_ISO8583_MSG_SetField_Str(0, glSendPack.szMsgCode, &isoMsg);
	}
	if(strlen(glSendPack.szPan) > 0)
	{
		ShowLogs(1, "PAN: %s", glSendPack.szPan);
		(void)DL_ISO8583_MSG_SetField_Str(2, glSendPack.szPan, &isoMsg);
	}
	if(strlen(glSendPack.szProcCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(3, glSendPack.szProcCode, &isoMsg);
		ShowLogs(1, "FIELD 4: %s", glSendPack.szProcCode);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szProcCode), glSendPack.szProcCode);
	}
	if(strlen(glSendPack.szTranAmt) > 0)
	{
		char stamp[20] = {0};
		char tt[20] = {0};
		parseAmount(glSendPack.szTranAmt, tt);
		ShowLogs(1, "Converted Amount: %s.", tt);
	    double tot = atof(tt);
	    UtilGetEnvEx("stampduty", stamp);
	    ShowLogs(1, "Stampduty: %s.", stamp);
	    if((strstr(stamp, "true") != NULL)
	      && (tot >= 1000)
	      && (glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	    {
	    	char fina[13] = {0};
	    	memset(fina, '\0', strlen(fina));
		    PubAscAdd(glSendPack.szTranAmt, "000000005000", 12, fina);
		    ShowLogs(1, "Total: %s", fina);
		    (void)DL_ISO8583_MSG_SetField_Str(4, fina, &isoMsg);
	    }else
			(void)DL_ISO8583_MSG_SetField_Str(4, glSendPack.szTranAmt, &isoMsg);
	}
	if(strlen(glSendPack.szLocalDateTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(7, glSendPack.szLocalDateTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDateTime), glSendPack.szLocalDateTime);
	}
	if(strlen(glSendPack.szSTAN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(11, glSendPack.szSTAN, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szSTAN), glSendPack.szSTAN);
	}
	GetStan();
	if(strlen(glSendPack.szLocalTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(12, glSendPack.szLocalTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalTime), glSendPack.szLocalTime);
	}
	if(strlen(glSendPack.szLocalDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(13, glSendPack.szLocalDate, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDate), glSendPack.szLocalDate);
	}
	if(strlen(glSendPack.szExpDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(14, glSendPack.szExpDate, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szExpDate), glSendPack.szExpDate);
	}
	if(strlen(glSendPack.szMerchantType) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(18, glSendPack.szMerchantType, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantType), glSendPack.szMerchantType);
	}
	if(strlen(glSendPack.szEntryMode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(22, glSendPack.szEntryMode + 1, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szEntryMode), glSendPack.szEntryMode);
	}
	if(strlen(glSendPack.szPanSeqNo) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(23, glSendPack.szPanSeqNo, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPanSeqNo), glSendPack.szPanSeqNo);
	}
	if(strlen(glSendPack.szCondCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(25, glSendPack.szCondCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szCondCode), glSendPack.szCondCode);
	}
	if(strlen(glSendPack.szPoscCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(26, glSendPack.szPoscCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPoscCode), glSendPack.szPoscCode);
	}
	if(strlen(glSendPack.szTransFee) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(28, glSendPack.szTransFee, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTransFee), glSendPack.szTransFee);
	}
	if(strlen(glSendPack.szAqurId) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(32, glSendPack.szAqurId, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAqurId), glSendPack.szAqurId);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szTrack2) > 0)
	{
		memset(parseTrack2, '\0', strlen(parseTrack2));
		parsetrack2(glSendPack.szTrack2, parseTrack2);
		(void)DL_ISO8583_MSG_SetField_Str(35, parseTrack2, &isoMsg);

		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(parseTrack2), parseTrack2);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szRRN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(37, glSendPack.szRRN, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szRRN), glSendPack.szRRN);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szAuthCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(38, glSendPack.szAuthCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAuthCode), glSendPack.szAuthCode);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szServResCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(40, glSendPack.szServResCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szServResCode), glSendPack.szServResCode);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szTermID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(41, glSendPack.szTermID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTermID), glSendPack.szTermID);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szMerchantID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(42, glSendPack.szMerchantID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantID), glSendPack.szMerchantID);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szMNL) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMNL), glSendPack.szMNL);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szTranCurcyCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(49, glSendPack.szTranCurcyCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTranCurcyCode), glSendPack.szTranCurcyCode);
		//DisplayInfoNone("", temp2, 3);
	}
	if(pincheck == 1)
	{
		if(strlen(glSendPack.szPinBlock) > 0)
		{
			(void)DL_ISO8583_MSG_SetField_Str(52, glSendPack.szPinBlock, &isoMsg);
		}
		pincheck = 0;
	}
	if(strlen(glSendPack.testSICCData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, glSendPack.testSICCData, &isoMsg);
	}
	
	/*if(icc != NULL)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, icc, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(icc), icc);
		//DisplayInfoNone("", temp2, 3);
	}*/
	if(strlen(glSendPack.szReasonCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(56, glSendPack.szReasonCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReasonCode), glSendPack.szReasonCode);
		//DisplayInfoNone("", temp2, 3);
	}

	if(strlen(glSendPack.szTEchoData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(59, glSendPack.szTEchoData, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTEchoData), glSendPack.szTEchoData);//log
		//DisplayInfoNone("", temp2, 3);
	}
	//Added for vas
	if(strlen(glSendPack.szBillers) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(62, glSendPack.szBillers, &isoMsg);
	}

	if(strlen(glSendPack.szOrigDataElement) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(90, glSendPack.szOrigDataElement, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d -- %s", strlen(glSendPack.szOrigDataElement), glSendPack.szOrigDataElement);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szReplAmount) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(95, glSendPack.szReplAmount, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReplAmount), glSendPack.szReplAmount);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szPosDataCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(123, glSendPack.szPosDataCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPosDataCode), glSendPack.szPosDataCode);
		//DisplayInfoNone("", temp2, 3);
	}
	(void) DL_ISO8583_MSG_SetField_Str(128, 0x0, &isoMsg);

	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);
	ShowLogs(1, "Iso Message With Length: %s", hexData);

	if (packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(128, outHash, &isoMsg);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}
	//Fine till here

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
			//Investigate from here tomorrow
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				ShowLogs(1, "Ima Mmi: %d", reversal);
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
 			ShowLogs(1, "CommInitModule Start");
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			ShowLogs(1, "CommsDial Start");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}


	
 	ShowLogs(1, "Done With Nibss");
	ScrBackLight(1);
 	memset(hexData, '\0', strlen(hexData));
	HexEnCodeMethod(output, strlen(output) + 2, hexData);
	ShowLogs(1, "Received From Nibss: %s", hexData);
	
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	
	
	int i = 0;

	//memset(&glRecvPack, 0, sizeof(STISO8583));//This is the proble
	
	//ShowLogs(1, "Commented out");
	for(i = 0; i < 129; i++)
	{
		if ( NULL != isoMsg.field[i].ptr ) 
		{
			memset(dataStore, '\0', strlen(dataStore));
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			sprintf(dataStore, "%s", isoMsg.field[i].ptr);
			switch(i)
			{
				case 0:
					memset(glRecvPack.szMsgCode, '\0', strlen(glRecvPack.szMsgCode));
					sprintf((char *)glRecvPack.szMsgCode, "%.*s", LEN_MSG_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMsgCode);
					continue;
				case 1:
					memset(glRecvPack.sBitMap, '\0', strlen(glRecvPack.sBitMap));
					sprintf((char *)glRecvPack.sBitMap, "%.*s", 2*LEN_BITMAP, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sBitMap);
					continue;
				case 2:
					memset(glRecvPack.szPan, '\0', strlen(glRecvPack.szPan));
					sprintf((char *)glRecvPack.szPan, "%.*s", LEN_PAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPan);
					continue;
				case 3:
					memset(glRecvPack.szProcCode, '\0', strlen(glRecvPack.szProcCode));
					sprintf((char *)glRecvPack.szProcCode, "%.*s", LEN_PROC_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szProcCode);
					continue;
				case 4:
					memset(glRecvPack.szTranAmt, '\0', strlen(glRecvPack.szTranAmt));
					sprintf((char *)glRecvPack.szTranAmt, "%.*s", LEN_TRAN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranAmt);
					continue;
				case 7:
					memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
					sprintf((char *)glRecvPack.szFrnAmt, "%.*s", LEN_FRN_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFrnAmt);
					continue;
				case 11:
					memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
					sprintf((char *)glRecvPack.szSTAN, "%.*s", LEN_STAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSTAN);
					continue;
				case 12:
					memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
					sprintf((char *)glRecvPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalTime);
					continue;
				case 13:
					memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
					sprintf((char *)glRecvPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalDate);
					continue;
				case 14:
					memset(glRecvPack.szExpDate, '\0', strlen(glRecvPack.szExpDate));
					sprintf((char *)glRecvPack.szExpDate, "%.*s", LEN_EXP_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szExpDate);
					continue;
				case 15:
					memset(glRecvPack.szSetlDate, '\0', strlen(glRecvPack.szSetlDate));
					sprintf((char *)glRecvPack.szSetlDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSetlDate);
					continue;
				case 18:
					memset(glRecvPack.szMerchantType, '\0', strlen(glRecvPack.szMerchantType));
					sprintf((char *)glRecvPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantType);
					continue;
				case 22:
					memset(glRecvPack.szEntryMode, '\0', strlen(glRecvPack.szEntryMode));
					sprintf((char *)glRecvPack.szEntryMode, "%.*s", LEN_ENTRY_MODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szEntryMode);
					continue;
				case 23:
					memset(glRecvPack.szPanSeqNo, '\0', strlen(glRecvPack.szPanSeqNo));
					sprintf((char *)glRecvPack.szPanSeqNo, "%.*s", LEN_PAN_SEQ_NO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPanSeqNo);
					continue;
				case 25:
					memset(glRecvPack.szCondCode, '\0', strlen(glRecvPack.szCondCode));
					sprintf((char *)glRecvPack.szCondCode, "%.*s", LEN_COND_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szCondCode);
					continue;
				case 28:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 30:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 32:
					memset(glRecvPack.szAqurId, '\0', strlen(glRecvPack.szAqurId));
					sprintf((char *)glRecvPack.szAqurId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAqurId);
					continue;
				case 33:
					memset(glRecvPack.szFwdInstId, '\0', strlen(glRecvPack.szFwdInstId));
					sprintf((char *)glRecvPack.szFwdInstId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFwdInstId);
					continue;
				case 35:
					memset(glRecvPack.szTrack2, '\0', strlen(glRecvPack.szTrack2));
					sprintf((char *)glRecvPack.szTrack2, "%.*s", LEN_TRACK2, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTrack2);
					continue;
				case 37:
					memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
					sprintf((char *)glRecvPack.szRRN, "%.*s", LEN_RRN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRRN);
					continue;
				case 38:
					memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
					sprintf((char *)glRecvPack.szAuthCode, "%.*s", LEN_AUTH_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAuthCode);
					continue;
				case 39:
					memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
					sprintf((char *)glRecvPack.szRspCode, "%.*s", LEN_RSP_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRspCode);
					continue;
				case 40:
					memset(glRecvPack.szServResCode, '\0', strlen(glRecvPack.szServResCode));
					sprintf((char *)glRecvPack.szServResCode, "%.*s", LEN_SRES_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szServResCode);
					continue;
				case 41:
					memset(glRecvPack.szTermID, '\0', strlen(glRecvPack.szTermID));
					sprintf((char *)glRecvPack.szTermID, "%.*s", LEN_TERM_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTermID);
					continue;
				case 42:
					memset(glRecvPack.szMerchantID, '\0', strlen(glRecvPack.szMerchantID));
					sprintf((char *)glRecvPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantID);
					continue;
				case 43:
					memset(glRecvPack.szMNL, '\0', strlen(glRecvPack.szMNL));
					sprintf((char *)glRecvPack.szMNL, "%.*s", LEN_MNL, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMNL);
					continue;
				case 49:
					memset(glRecvPack.szTranCurcyCode, '\0', strlen(glRecvPack.szTranCurcyCode));
					sprintf((char *)glRecvPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranCurcyCode);
					continue;
				case 54:
					memset(glRecvPack.szAddtAmount, '\0', strlen(glRecvPack.szAddtAmount));
					sprintf((char *)glRecvPack.szAddtAmount, "%.*s", LEN_ADDT_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAddtAmount);
					continue;
				case 55:
					memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
					sprintf((char *)glRecvPack.sICCData, "%.*s", LEN_ICC_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sICCData);
					continue;
				case 59:
					memset(glRecvPack.szTEchoData, '\0', strlen(glRecvPack.szTEchoData));
					sprintf((char *)glRecvPack.szTEchoData, "%.*s", LEN_TRANSECHO_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTEchoData);
					continue;
				case 60:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 62:
					memset(glRecvPack.szBillers, '\0', strlen(glRecvPack.szBillers));
					sprintf((char *)glRecvPack.szBillers, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szBillers);
					continue;
				case 102:
					memset(glRecvPack.szActIdent1, '\0', strlen(glRecvPack.szActIdent1));
					sprintf((char *)glRecvPack.szActIdent1, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent1);
					continue;
				case 103:
					memset(glRecvPack.szActIdent2, '\0', strlen(glRecvPack.szActIdent2));
					sprintf((char *)glRecvPack.szActIdent2, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent2);
					continue;
				case 123:
					memset(glRecvPack.szPosDataCode, '\0', strlen(glRecvPack.szPosDataCode));
					sprintf((char *)glRecvPack.szPosDataCode, "%.*s", LEN_POS_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPosDataCode);
					continue;
				case 124:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 128:
					memset(glRecvPack.szNFC, '\0', strlen(glRecvPack.szNFC));
					sprintf((char *)glRecvPack.szNFC, "%.*s", LEN_NEARFIELD, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szNFC);
					continue;
				default:
					continue;
			}
		}
	}
	ShowLogs(1, "Response Code: %s", glRecvPack.szRspCode);
	

	sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
	//UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
	sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);
	FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
	strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);
	
	ShowLogs(1, "2. Response Code: %s", glRecvPack.szRspCode);

	ShowLogs(1, "Commencing Storage.");
	if(strlen(glRecvPack.szRspCode) == 2)
    {
    	if(txnType != 5)
    	{
	    	ShowLogs(1, "About Storing Transaction.");
	    	storeTxn();
	    	ShowLogs(1, "About Storing Eod.");
			storeEod();
			ShowLogs(1, "About printing.");
			storeprint();
		}
    }else
    {
    	customStorage(szLocalTime, "100");
    	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
    	//No response
    	titi = 1;
    }
    ShowLogs(1, "Done with Storing. Modify");
    ShowLogs(1, "3. Response Code: %s", glRecvPack.szRspCode);

	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		DisplayInfoNone("", "Trans Success", 1);
		//PackCallHomeData();
	}else
	{
		DisplayInfoNone("", "Trans Declined", 1);
		//PackCallHomeData();
	}

    
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}


int SendEmvDataXML(char *icc, int *reversal)
{
	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char temp2[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char dataStore[254] = {0};
	uchar	szLocalTime[14+1];
	uchar outputData[10240] = {0};
	char parseTrack2[40] = {0};
	int iTemp;

	char formCard[2*1024] = {0};
	char sendCard[3*1024] = {0};


	ShowLogs(1, "INSIDE SEND EMV DATA");
	checkVerve = 0;

	ShowLogs(1, "PAN: %s", glSendPack.szPan);

	titi = 0;


	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));


	memset(packBuf, 0x0, sizeof(packBuf));
	
	

	SysGetTimeIso(timeGotten);
	strncpy(datetime, timeGotten + 2, 10);
	if(txnType == 6)
	{
		ShowLogs(1, "This is a sales completion");
		strncpy((char *)glSendPack.szLocalDate, glSendPack.szLocalDateTime, 4);
		strncpy((char *)glSendPack.szLocalTime, glSendPack.szLocalDateTime + 4, 6);
	}else
	{
		sprintf((char *)glSendPack.szLocalDateTime, "%.*s", LEN_LOCAL_DATE_TIME, datetime);
		strncpy(dt, timeGotten + 2, 4);
		sprintf((char *)glSendPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dt);
		strncpy(tm, timeGotten + 6, 6);
		sprintf((char *)glSendPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, tm);
	}

	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
	memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
	memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
	memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
	memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
	memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
	memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));

	memset(formCard, '\0', sizeof(formCard));
	strcpy(formCard, "{");

	

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	/* set ISO message fields */

	if(checkMimic == 1)
	{
		txnType = 7;
		strcpy(glSendPack.szMsgCode, "0420");
	}

	if(strlen(glSendPack.szMsgCode) > 0)
	{
		ShowLogs(1, "Getting mti for normal cards: %s", glSendPack.szMsgCode);
		(void)DL_ISO8583_MSG_SetField_Str(0, glSendPack.szMsgCode, &isoMsg);
		strcat(formCard, "\"field0\": \"");
		strcat(formCard, glSendPack.szMsgCode);
	}
	
	if(strlen(glSendPack.szPan) > 0)
	{
		ShowLogs(1, "PAN: %s", glSendPack.szPan);
		(void)DL_ISO8583_MSG_SetField_Str(2, glSendPack.szPan, &isoMsg);
		strcat(formCard, "\",\"field2\": \"");
		strcat(formCard, glSendPack.szPan);
	}
	if(strlen(glSendPack.szProcCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(3, glSendPack.szProcCode, &isoMsg);
		ShowLogs(1, "FIELD 4: %s", glSendPack.szProcCode);
		strcat(formCard, "\",\"field3\": \"");
		strcat(formCard, glSendPack.szProcCode);
	}
	
	if(strlen(glSendPack.szTranAmt) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(4, glSendPack.szTranAmt, &isoMsg);
		strcat(formCard, "\",\"field4\": \"");
		strcat(formCard, glSendPack.szTranAmt);
	}

	if(strlen(glSendPack.szLocalDateTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(7, glSendPack.szLocalDateTime, &isoMsg);
		strcat(formCard, "\",\"field7\": \"");
		strcat(formCard, glSendPack.szLocalDateTime);
	}
	if(strlen(glSendPack.szSTAN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(11, glSendPack.szSTAN, &isoMsg);
		strcat(formCard, "\",\"field11\": \"");
		strcat(formCard, glSendPack.szSTAN);
	}
	GetStan();
	if(strlen(glSendPack.szLocalTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(12, glSendPack.szLocalTime, &isoMsg);
		strcat(formCard, "\",\"field12\": \"");
		strcat(formCard, glSendPack.szLocalTime);
	}
	if(strlen(glSendPack.szLocalDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(13, glSendPack.szLocalDate, &isoMsg);
		strcat(formCard, "\",\"field13\": \"");
		strcat(formCard, glSendPack.szLocalDate);
	}
	if(strlen(glSendPack.szExpDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(14, glSendPack.szExpDate, &isoMsg);
		strcat(formCard, "\",\"field14\": \"");
		strcat(formCard, glSendPack.szExpDate);
	}
	if(strlen(glSendPack.szMerchantType) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(18, glSendPack.szMerchantType, &isoMsg);
		strcat(formCard, "\",\"field18\": \"");
		strcat(formCard, glSendPack.szMerchantType);
		ShowLogs(1, "MCC GOTTEN: %s", glSendPack.szMerchantType);
	}
	if(strlen(glSendPack.szEntryMode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(22, glSendPack.szEntryMode + 1, &isoMsg);
		strcat(formCard, "\",\"field22\": \"");
		strcat(formCard, glSendPack.szEntryMode + 1);
	}
	if(strlen(glSendPack.szPanSeqNo) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(23, glSendPack.szPanSeqNo, &isoMsg);
		strcat(formCard, "\",\"field23\": \"");
		strcat(formCard, glSendPack.szPanSeqNo);
	}
	if(strlen(glSendPack.szCondCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(25, glSendPack.szCondCode, &isoMsg);
		strcat(formCard, "\",\"field25\": \"");
		strcat(formCard, glSendPack.szCondCode);
	}
	if(strlen(glSendPack.szPoscCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(26, glSendPack.szPoscCode, &isoMsg);
		strcat(formCard, "\",\"field26\": \"");
		strcat(formCard, glSendPack.szPoscCode);
	}
	if(strlen(glSendPack.szTransFee) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(28, glSendPack.szTransFee, &isoMsg);
		strcat(formCard, "\",\"field28\": \"");
		strcat(formCard, glSendPack.szTransFee);
	}
	if(strlen(glSendPack.szAqurId) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(32, glSendPack.szAqurId, &isoMsg);
		strcat(formCard, "\",\"field32\": \"");
		strcat(formCard, glSendPack.szAqurId);
	}
	if(strlen(glSendPack.szTrack2) > 0)
	{
		memset(parseTrack2, '\0', strlen(parseTrack2));
		parsetrack2(glSendPack.szTrack2, parseTrack2);
		(void)DL_ISO8583_MSG_SetField_Str(35, parseTrack2, &isoMsg);
		strcat(formCard, "\",\"field35\": \"");
		strcat(formCard, parseTrack2);
	}
	if(strlen(glSendPack.szRRN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(37, glSendPack.szRRN, &isoMsg);
		strcat(formCard, "\",\"field37\": \"");
		strcat(formCard, glSendPack.szRRN);
	}
	if(strlen(glSendPack.szAuthCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(38, glSendPack.szAuthCode, &isoMsg);
		strcat(formCard, "\",\"field38\": \"");
		strcat(formCard, glSendPack.szAuthCode);
	}
	if(strlen(glSendPack.szServResCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(40, glSendPack.szServResCode, &isoMsg);
		strcat(formCard, "\",\"field40\": \"");
		strcat(formCard, glSendPack.szServResCode);
	}
	if(strlen(glSendPack.szTermID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(41, glSendPack.szTermID, &isoMsg);
		strcat(formCard, "\",\"field41\": \"");
		strcat(formCard, glSendPack.szTermID);
	}
	ShowLogs(1, "MID: %s", glSendPack.szMerchantID);
	if(strlen(glSendPack.szMerchantID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(42, glSendPack.szMerchantID, &isoMsg);
		strcat(formCard, "\",\"field42\": \"");
		strcat(formCard, glSendPack.szMerchantID);
	}
	if(strlen(glSendPack.szMNL) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
		strcat(formCard, "\",\"field43\": \"");
		strcat(formCard, glSendPack.szMNL);
	}
	if(strlen(glSendPack.szTranCurcyCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(49, glSendPack.szTranCurcyCode, &isoMsg);
		strcat(formCard, "\",\"field49\": \"");
		strcat(formCard, glSendPack.szTranCurcyCode);
	}
	if(pincheck == 1)
	{
		if(strlen(glSendPack.szPinBlock) > 0)
		{
			(void)DL_ISO8583_MSG_SetField_Str(52, glSendPack.szPinBlock, &isoMsg);
			strcat(formCard, "\",\"field52\": \"");
			strcat(formCard, glSendPack.szPinBlock);
		}
		pincheck = 0;
	}
	if(strlen(glSendPack.testSICCData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, glSendPack.testSICCData, &isoMsg);
		strcat(formCard, "\",\"field55\": \"");
		strcat(formCard, glSendPack.testSICCData);
	}
	
	if(strlen(glSendPack.szReasonCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(56, glSendPack.szReasonCode, &isoMsg);
		strcat(formCard, "\",\"field56\": \"");
		strcat(formCard, glSendPack.szReasonCode);
	}

	if(strlen(glSendPack.szTEchoData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(59, glSendPack.szTEchoData, &isoMsg);
		strcat(formCard, "\",\"field59\": \"");
		strcat(formCard, glSendPack.szTEchoData);
	}
	//Added for vas
	if(strlen(glSendPack.szBillers) > 0)
	{
		//(void)DL_ISO8583_MSG_SetField_Str(62, glSendPack.szBillers, &isoMsg);
	}

	if(strlen(glSendPack.szOrigDataElement) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(90, glSendPack.szOrigDataElement, &isoMsg);
		strcat(formCard, "\",\"field90\": \"");
		strcat(formCard, glSendPack.szOrigDataElement);
	}
	if(strlen(glSendPack.szReplAmount) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(95, glSendPack.szReplAmount, &isoMsg);
		strcat(formCard, "\",\"field95\": \"");
		strcat(formCard, glSendPack.szReplAmount);
	}
	if(strlen(glSendPack.szPosDataCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(123, glSendPack.szPosDataCode, &isoMsg);
		strcat(formCard, "\",\"field123\": \"");
		strcat(formCard, glSendPack.szPosDataCode);
	}
	(void) DL_ISO8583_MSG_SetField_Str(128, 0x0, &isoMsg);
	
	

	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);
	ShowLogs(1, "Iso Message With Length: %s", hexData);

	if(packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(128, outHash, &isoMsg);
		strcat(formCard, "\",\"field128\": \"");
		strcat(formCard, outHash);
		strcat(formCard, "\"}");
		ShowLogs(1, "CARD: %s", formCard);
		memset(sendCard, '\0', sizeof(sendCard));
		finishTxn(formCard, sendCard, 0);
		ShowLogs(1, "FINAL: %s", sendCard);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}
	//Fine till here

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOIP);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOPORT);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		if(1)
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
 			ShowLogs(1, "CommInitModule Start");
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				return -1;
			}
			ShowLogs(1, "CommsDial Start");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);

	 		iRet = CommTxd(sendCard, sizeof(sendCard), 180);
			ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "100");
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 1024, 180, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				//customStorage(szLocalTime, "100");
				//return -1;
			}
			CommOnHookGPRS(TRUE);
		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOIP);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOPORT);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		if(1)
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				customStorage(szLocalTime, "101");
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				customStorage(szLocalTime, "101");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
			
			iRet = CommTxd(sendCard, sizeof(sendCard), 180);
	 		ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				customStorage(szLocalTime, "100");
				*reversal = 1;
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 1024, 180, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				//customStorage(szLocalTime, "100");
				//*reversal = 1;
				//return -1;
			}
			CommOnHook(TRUE);
 		}
	}

	ShowLogs(1, "Done With Nibss");
	ScrBackLight(1);
 	ShowLogs(1, "Received From Nibss: %s", output);
	
	if(strlen(output) < 2)
	{
		Beep();
		DisplayInfoNone("", "Checking Txn Status", 2);
		checkCardStatus(output);
	}

	strcpy(glRecvPack.szRRN, glSendPack.szRRN);
	strcpy(glRecvPack.szSTAN, glSendPack.szSTAN);
	strcpy(glRecvPack.szFrnAmt, glSendPack.szFrnAmt);
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);
	FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
	strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);

	Beep();

	CreateWrite("tempres.txt", output);
	if(txnType == 5)
	{
		ShowLogs(1, "IT IS BALANCE ENQUIRY");
		if(strlen(output) > 59)
		{
			ShowLogs(1, "IT IS BALANCE ENQUIRY APPROVED");
			strcpy(glRecvPack.szAddtAmount, output);
			strcpy(output, "TRANSACTION SUCCESSFUL");
		}
	}

	if(strstr(output, "FUNDS TRANSFER SUCCESSFUL") != NULL)
	{
		strncpy(glRecvPack.szRspCode, "00", 2);
		if(strstr(glSendPack.szMsgCode, "0420") != NULL)
		{
			char strev[15] = {0};
			ShowLogs(1, "STORING FOR REVERSAL....");
			strcpy(strev, glSendPack.szRRN);
			strcat(strev, "\n");
			storeReversal(strev);
			ShowLogs(1, "DONE STORING FOR REVERSAL....");
		}
	}else if(strstr(output, "TRANSACTION SUCCESSFUL") != NULL)
	{
		strncpy(glRecvPack.szRspCode, "00", 2);
	}else if(strstr(output, "Payment Notification Successful") != NULL)
	{
		strncpy(glRecvPack.szRspCode, "00", 2);
	}else if(strlen(output) < 4)
	{
		strncpy(glRecvPack.szRspCode, output, 2);
	}else
	{
		strncpy(glRecvPack.szRspCode, "06", 2);
	}

	ShowLogs(1, "Response Code: %s.", glRecvPack.szRspCode);
	sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
	ShowLogs(1, "2. Response Code: %s", glRecvPack.szRspCode);
	
	ShowLogs(1, "Commencing Storage.");
	if(txnType != 5)
	{
		ShowLogs(1, "About Storing Transaction.");
		//storeTxn();
		ShowLogs(1, "About Storing Eod.");
		storeEod();
		ShowLogs(1, "About printing.");
		storeprint();
	}
    ShowLogs(1, "Done with Storing. Modify");
	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		strcpy(glRecvPack.szAuthCode, "000000");
		DisplayInfoNone("", "TRANSACTION SUCCESS", 1);
		ShowLogs(1, "Card Success....");
	}else
	{
		strcpy(glRecvPack.szAuthCode, "NA");
		DisplayInfoNone("", "TRANSACTION DECLINED", 1);
		//SANUSI 2022-01-18
		if(strcmp(glRecvPack.szRspCode, "") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "06") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "96") == 0)
		{
			UpdateProfile();
		}


		if(strcmp(glRecvPack.szRspCode, "86") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "87") == 0)
		{
			UpdateProfile();
		}
		if(strcmp(glRecvPack.szRspCode, "88") == 0)
		{
			UpdateProfile();
		}

		if(strcmp(glRecvPack.szRspCode, "89") == 0)
		{
			UpdateProfile();
		}

	
	}
	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
	
	
	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		if(glSendPack.szPan[0] == '4')
		{
			ScrBackLight(1);
			PrintAllReceipt(PRN_NORMAL);
		}
	}
	ShowLogs(1, "WISDOM Pin Gotten sendEMVdata : %d", iRet);
//PostCellulantNotificationFCMB();
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 1;
}


int SendPayattitudeaccess(char *icc, int *reversal)
{

	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char temp2[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char stan[6] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char dataStore[254] = {0};
	uchar	szLocalTime[14+1];
	uchar outputData[10240] = {0};
	char parseTrack2[40] = {0};
	int iTemp;

	ShowLogs(1, "INSIDE SEND EMV DATA");
	checkVerve = 0;

	ShowLogs(1, "PAN: %s", glSendPack.szPan);

	titi = 0;
	rvApr = 0;

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	//SaleInput(); 
	SetCommReqField();

	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));


	memset(packBuf, 0x0, sizeof(packBuf));

	SysGetTimeIso(timeGotten);

	strncpy(datetime, timeGotten + 2, 10);
	sprintf((char *)glSendPack.szLocalDateTime, "%.*s", LEN_LOCAL_DATE_TIME, datetime);
	strncpy(dt, timeGotten + 2, 4);
	sprintf((char *)glSendPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dt);
	strncpy(tm, timeGotten + 6, 6);
	sprintf((char *)glSendPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, tm);

	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
	memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
	memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
	memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
	memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
	memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
	memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	/*if(strlen(glSendPack.szMsgCode) > 0)
	{*/
		ShowLogs(1, "Getting mti for normal cards: %s", glSendPack.szMsgCode);
		ShowLogs(1, "Setting mti as: %s", "1200");
		(void)DL_ISO8583_MSG_SetField_Str(0, "1200", &isoMsg);//glSendPack.szMsgCode
	//}

	memset(glSendPack.szPan,0x00,sizeof(glSendPack.szPan));
	sprintf(glSendPack.szPan,"9501000000000001");
//	if(strlen(glSendPack.szPan) > 0)
//	{
		ShowLogs(1, "PAN: %s", glSendPack.szPan);
		(void)DL_ISO8583_MSG_SetField_Str(2, glSendPack.szPan, &isoMsg);
//	}

	memset(glSendPack.szProcCode,0x00,sizeof(glSendPack.szProcCode));
	sprintf(glSendPack.szProcCode,"000000");
//	if(strlen(glSendPack.szProcCode) > 0)
//	{
		(void)DL_ISO8583_MSG_SetField_Str(3, glSendPack.szProcCode, &isoMsg);
		ShowLogs(1, "FIELD 4: %s", glSendPack.szProcCode);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szProcCode), glSendPack.szProcCode);
//	}

	/*if(strlen(glSendPack.szTranAmt) > 0)
	{
		char stamp[20] = {0};
		char tt[20] = {0};
		parseAmount(glSendPack.szTranAmt, tt);
		ShowLogs(1, "Converted Amount: %s.", tt);
	    double tot = atof(tt);
	    UtilGetEnvEx("stampduty", stamp);
	    ShowLogs(1, "Stampduty: %s.", stamp);
	    if((strstr(stamp, "true") != NULL) && (tot >= 1000)
	      && (glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	    {
	    	char fina[13] = {0};
	    	memset(fina, '\0', strlen(fina));
		    PubAscAdd(glSendPack.szTranAmt, "000000005000", 12, fina);
		    ShowLogs(1, "Total: %s", fina);
		    (void)DL_ISO8583_MSG_SetField_Str(4, fina, &isoMsg);
	    }else*/
		sprintf((char *)glSendPack.szTranAmt, "%.*s", LEN_TRAN_AMT, glProcInfo.stTranLog.szAmount);
			(void)DL_ISO8583_MSG_SetField_Str(4, glSendPack.szTranAmt, &isoMsg);
	//}
	if(strlen(glSendPack.szLocalDateTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(7, glSendPack.szLocalDateTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDateTime), glSendPack.szLocalDateTime);
	}

	GetStan();
	if(strlen(glSendPack.szSTAN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(11, glSendPack.szSTAN, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szSTAN), glSendPack.szSTAN);
	}else{
		(void)DL_ISO8583_MSG_SetField_Str(11, "000000", &isoMsg);
	}
	GetStan();
	if(strlen(glSendPack.szLocalTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(12, glSendPack.szLocalTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalTime), glSendPack.szLocalTime);
	}
	//if(strlen(glSendPack.szLocalDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(13, glSendPack.szLocalDate, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDate), glSendPack.szLocalDate);
	}
	//if(strlen(glSendPack.szExpDate) > 0)
	//{
		(void)DL_ISO8583_MSG_SetField_Str(14, "3012", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szExpDate), glSendPack.szExpDate);
	//}
	if(strlen(glSendPack.szMerchantType) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(18, glSendPack.szMerchantType, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantType), glSendPack.szMerchantType);
	}

		(void)DL_ISO8583_MSG_SetField_Str(22, "951", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szEntryMode), glSendPack.szEntryMode);


		(void)DL_ISO8583_MSG_SetField_Str(23, "000", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPanSeqNo), glSendPack.szPanSeqNo);


		(void)DL_ISO8583_MSG_SetField_Str(25, "00", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szCondCode), glSendPack.szCondCode);


		(void)DL_ISO8583_MSG_SetField_Str(26, "06", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPoscCode), glSendPack.szPoscCode);

		(void)DL_ISO8583_MSG_SetField_Str(28,"C00000000", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTransFee), glSendPack.szTransFee);

//	if(strlen(glSendPack.szAqurId) > 0)
//	{
		(void)DL_ISO8583_MSG_SetField_Str(32, "111129", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAqurId), glSendPack.szAqurId);
		//DisplayInfoNone("", temp2, 3);
//	}

	memset(glSendPack.szTrack2,0x00,sizeof(glSendPack.szTrack2));
	sprintf(glSendPack.szTrack2,"9501000000000001D3012");
	//if(strlen(glSendPack.szTrack2) > 0)
	//{
		memset(parseTrack2, '\0', strlen(parseTrack2));
		parsetrack2(glSendPack.szTrack2, parseTrack2);
		(void)DL_ISO8583_MSG_SetField_Str(35, parseTrack2, &isoMsg);

		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(parseTrack2), parseTrack2);
		//DisplayInfoNone("", temp2, 3);
	//}
		sprintf((char *)stan, "%06lu", useStan);  //??
	GetStan();
	sprintf((char *)glSendPack.szRRN,       "000000%.12s", stan);
	if(strlen(glSendPack.szRRN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(37, glSendPack.szRRN, &isoMsg);
	}else{
		(void)DL_ISO8583_MSG_SetField_Str(37, stan, &isoMsg);
	}

	if(strlen(glSendPack.szAuthCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(38, glSendPack.szAuthCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAuthCode), glSendPack.szAuthCode);
		//DisplayInfoNone("", temp2, 3);
	}
	//if(strlen(glSendPack.szServResCode) > 0)
	//{
		(void)DL_ISO8583_MSG_SetField_Str(40, "221", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szServResCode), glSendPack.szServResCode);
		//DisplayInfoNone("", temp2, 3);
	//}

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	strcpy(glSendPack.szTermID, temp);

	if(strlen(glSendPack.szTermID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(41, glSendPack.szTermID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTermID), glSendPack.szTermID);
		//DisplayInfoNone("", temp2, 3);
	}

//set sanusi MID
memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	strcpy(glSendPack.szMerchantID, temp);

	if(strlen(glSendPack.szMerchantID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(42, glSendPack.szMerchantID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTermID), glSendPack.szTermID);
		//DisplayInfoNone("", temp2, 3);
	}
		//(void)DL_ISO8583_MSG_SetField_Str(41, "21035514", &isoMsg);
		//2UBALA000002870
		//(void)DL_ISO8583_MSG_SetField_Str(42, "2103LA010000417", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantID), glSendPack.szMerchantID);
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMNL", temp);
	sprintf((char *)glSendPack.szMNL, "%.*s", LEN_MNL, temp);

	if(strlen(glSendPack.szMNL) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMNL), glSendPack.szMNL);
		//DisplayInfoNone("", temp2, 3);
	}
	else (void)DL_ISO8583_MSG_SetField_Str(43, "POS COLLECTIONS ACCO    LA          LANG", &isoMsg);

		(void)DL_ISO8583_MSG_SetField_Str(49, "566", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTranCurcyCode), glSendPack.szTranCurcyCode);
		//DisplayInfoNone("", temp2, 3);
	/*if(pincheck == 1)
	{
		if(strlen(glSendPack.szPinBlock) > 0)
		{
			(void)DL_ISO8583_MSG_SetField_Str(52, glSendPack.szPinBlock, &isoMsg);
		}
		pincheck = 0;
	}*/
	/*if(strlen(glSendPack.testSICCData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, glSendPack.testSICCData, &isoMsg);
	}*/
	
	/*if(icc != NULL)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, icc, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(icc), icc);
		//DisplayInfoNone("", temp2, 3);
	}*/
	if(strlen(glSendPack.szReasonCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(56, glSendPack.szReasonCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReasonCode), glSendPack.szReasonCode);
		//DisplayInfoNone("", temp2, 3);
	}

	if(strlen(glSendPack.szTEchoData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(59, glSendPack.szTEchoData, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTEchoData), glSendPack.szTEchoData);//log
		//DisplayInfoNone("", temp2, 3);
	}
	//Added for vas
	//if(strlen(glSendPack.szBillers) > 0)
	//{
		memset(PAYATT_DE62,0,sizeof(PAYATT_DE62));
		sprintf(PAYATT_DE62,"%s%s","00698MP0101333",PAYATT_PHONENO);
		(void)DL_ISO8583_MSG_SetField_Str(62, PAYATT_DE62, &isoMsg);
	//}

	if(strlen(glSendPack.szOrigDataElement) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(90, glSendPack.szOrigDataElement, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d -- %s", strlen(glSendPack.szOrigDataElement), glSendPack.szOrigDataElement);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szReplAmount) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(95, glSendPack.szReplAmount, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReplAmount), glSendPack.szReplAmount);
		//DisplayInfoNone("", temp2, 3);
	}
	//if(strlen(glSendPack.szPosDataCode) > 0)
	//{
		(void)DL_ISO8583_MSG_SetField_Str(123, "510101511344002", &isoMsg);
		//(void)DL_ISO8583_MSG_SetField_Str(123, "510101511344001", &isoMsg);
		
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPosDataCode), glSendPack.szPosDataCode);
		//DisplayInfoNone("", temp2, 3);
	//}
	(void) DL_ISO8583_MSG_SetField_Str(128, 0x0, &isoMsg);
	
	/*if((glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	{
		ShowLogs(1, "This is Verve. Please check if there is need to backup");
		if(txnType == 14 || txnType == 15 || 
			txnType == 16 || txnType == 17 || txnType == 18)
		{
			ShowLogs(1, "This is Verve. Please back up");
			backupVerveWithVal();
		}
	}*/

	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);
	ShowLogs(1, "Iso Message With Length: %s", hexData);

	if (packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(128, outHash, &isoMsg);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}
	//Fine till here
	ShowLogs(1, "Iso Message afterhash: %s",packBuf );

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	//UtilGetEnv("cotype", temp);
	strcat(temp,"GPRS");
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		//UtilGetEnvEx("uhostip", temp);
		strcat(temp,"52.31.200.20");//139.162.203.47/62.173.47.4
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		//UtilGetEnvEx("uhostport", temp);
		strcat(temp,"80");//9001/6011
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		//UtilGetEnvEx("uhostssl", temp);
		/*strcat(temp,"false");
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
			//Investigate from here tomorrow
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				ShowLogs(1, "Ima Mmi: %d", reversal);
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	*/
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
 			ShowLogs(1, "CommInitModule Start");
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			ShowLogs(1, "CommsDial Start");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);

			ShowLogs(1, "Sending: %s",packBuf );

	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			CommOnHookGPRS(TRUE);
 		//}
 	}
	
 	ShowLogs(1, "Done With Nibss");
	ScrBackLight(1);
 	memset(hexData, '\0', strlen(hexData));
	HexEnCodeMethod(output, strlen(output) + 2, hexData);
	ShowLogs(1, "Received From Nibss: %s", hexData);
	
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	
	
	int i = 0;

	//memset(&glRecvPack, 0, sizeof(STISO8583));//This is the proble
	
	//ShowLogs(1, "Commented out");
	for(i = 0; i < 129; i++)
	{
		if ( NULL != isoMsg.field[i].ptr ) 
		{
			memset(dataStore, '\0', strlen(dataStore));
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			sprintf(dataStore, "%s", isoMsg.field[i].ptr);
			switch(i)
			{
				case 0:
					memset(glRecvPack.szMsgCode, '\0', strlen(glRecvPack.szMsgCode));
					sprintf((char *)glRecvPack.szMsgCode, "%.*s", LEN_MSG_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMsgCode);
					continue;
				case 1:
					memset(glRecvPack.sBitMap, '\0', strlen(glRecvPack.sBitMap));
					sprintf((char *)glRecvPack.sBitMap, "%.*s", 2*LEN_BITMAP, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sBitMap);
					continue;
				case 2:
					memset(glRecvPack.szPan, '\0', strlen(glRecvPack.szPan));
					sprintf((char *)glRecvPack.szPan, "%.*s", LEN_PAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPan);
					continue;
				case 3:
					memset(glRecvPack.szProcCode, '\0', strlen(glRecvPack.szProcCode));
					sprintf((char *)glRecvPack.szProcCode, "%.*s", LEN_PROC_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szProcCode);
					continue;
				case 4:
					memset(glRecvPack.szTranAmt, '\0', strlen(glRecvPack.szTranAmt));
					sprintf((char *)glRecvPack.szTranAmt, "%.*s", LEN_TRAN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranAmt);
					continue;
				case 7:
					memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
					sprintf((char *)glRecvPack.szFrnAmt, "%.*s", LEN_FRN_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFrnAmt);
					continue;
				case 11:
					memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
					sprintf((char *)glRecvPack.szSTAN, "%.*s", LEN_STAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSTAN);
					continue;
				case 12:
					memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
					sprintf((char *)glRecvPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalTime);
					continue;
				case 13:
					memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
					sprintf((char *)glRecvPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalDate);
					continue;
				case 14:
					memset(glRecvPack.szExpDate, '\0', strlen(glRecvPack.szExpDate));
					sprintf((char *)glRecvPack.szExpDate, "%.*s", LEN_EXP_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szExpDate);
					continue;
				case 15:
					memset(glRecvPack.szSetlDate, '\0', strlen(glRecvPack.szSetlDate));
					sprintf((char *)glRecvPack.szSetlDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSetlDate);
					continue;
				case 18:
					memset(glRecvPack.szMerchantType, '\0', strlen(glRecvPack.szMerchantType));
					sprintf((char *)glRecvPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantType);
					continue;
				case 22:
					memset(glRecvPack.szEntryMode, '\0', strlen(glRecvPack.szEntryMode));
					sprintf((char *)glRecvPack.szEntryMode, "%.*s", LEN_ENTRY_MODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szEntryMode);
					continue;
				case 23:
					memset(glRecvPack.szPanSeqNo, '\0', strlen(glRecvPack.szPanSeqNo));
					sprintf((char *)glRecvPack.szPanSeqNo, "%.*s", LEN_PAN_SEQ_NO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPanSeqNo);
					continue;
				case 25:
					memset(glRecvPack.szCondCode, '\0', strlen(glRecvPack.szCondCode));
					sprintf((char *)glRecvPack.szCondCode, "%.*s", LEN_COND_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szCondCode);
					continue;
				case 28:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 30:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 32:
					memset(glRecvPack.szAqurId, '\0', strlen(glRecvPack.szAqurId));
					sprintf((char *)glRecvPack.szAqurId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAqurId);
					continue;
				case 33:
					memset(glRecvPack.szFwdInstId, '\0', strlen(glRecvPack.szFwdInstId));
					sprintf((char *)glRecvPack.szFwdInstId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFwdInstId);
					continue;
				case 35:
					memset(glRecvPack.szTrack2, '\0', strlen(glRecvPack.szTrack2));
					sprintf((char *)glRecvPack.szTrack2, "%.*s", LEN_TRACK2, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTrack2);
					continue;
				case 37:
					memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
					sprintf((char *)glRecvPack.szRRN, "%.*s", LEN_RRN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRRN);
					continue;
				case 38:
					memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
					sprintf((char *)glRecvPack.szAuthCode, "%.*s", LEN_AUTH_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAuthCode);
					continue;
				case 39:
					memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
					sprintf((char *)glRecvPack.szRspCode, "%.*s", LEN_RSP_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRspCode);
					continue;
				case 40:
					memset(glRecvPack.szServResCode, '\0', strlen(glRecvPack.szServResCode));
					sprintf((char *)glRecvPack.szServResCode, "%.*s", LEN_SRES_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szServResCode);
					continue;
				case 41:
					memset(glRecvPack.szTermID, '\0', strlen(glRecvPack.szTermID));
					sprintf((char *)glRecvPack.szTermID, "%.*s", LEN_TERM_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTermID);
					continue;
				case 42:
					memset(glRecvPack.szMerchantID, '\0', strlen(glRecvPack.szMerchantID));
					sprintf((char *)glRecvPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantID);
					continue;
				case 43:
					memset(glRecvPack.szMNL, '\0', strlen(glRecvPack.szMNL));
					sprintf((char *)glRecvPack.szMNL, "%.*s", LEN_MNL, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMNL);
					continue;
				case 49:
					memset(glRecvPack.szTranCurcyCode, '\0', strlen(glRecvPack.szTranCurcyCode));
					sprintf((char *)glRecvPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranCurcyCode);
					continue;
				case 54:
					memset(glRecvPack.szAddtAmount, '\0', strlen(glRecvPack.szAddtAmount));
					sprintf((char *)glRecvPack.szAddtAmount, "%.*s", LEN_ADDT_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAddtAmount);
					continue;
				case 55:
					memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
					sprintf((char *)glRecvPack.sICCData, "%.*s", LEN_ICC_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sICCData);
					continue;
				case 59:
					memset(glRecvPack.szTEchoData, '\0', strlen(glRecvPack.szTEchoData));
					sprintf((char *)glRecvPack.szTEchoData, "%.*s", LEN_TRANSECHO_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTEchoData);
					continue;
				case 60:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 62:
					memset(glRecvPack.szBillers, '\0', strlen(glRecvPack.szBillers));
					sprintf((char *)glRecvPack.szBillers, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szBillers);
					continue;
				case 102:
					memset(glRecvPack.szActIdent1, '\0', strlen(glRecvPack.szActIdent1));
					sprintf((char *)glRecvPack.szActIdent1, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent1);
					continue;
				case 103:
					memset(glRecvPack.szActIdent2, '\0', strlen(glRecvPack.szActIdent2));
					sprintf((char *)glRecvPack.szActIdent2, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent2);
					continue;
				case 123:
					memset(glRecvPack.szPosDataCode, '\0', strlen(glRecvPack.szPosDataCode));
					sprintf((char *)glRecvPack.szPosDataCode, "%.*s", LEN_POS_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPosDataCode);
					continue;
				case 124:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 128:
					memset(glRecvPack.szNFC, '\0', strlen(glRecvPack.szNFC));
					sprintf((char *)glRecvPack.szNFC, "%.*s", LEN_NEARFIELD, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szNFC);
					continue;
				default:
					continue;
			}
		}
	}
	ShowLogs(1, "Response Code: %s", glRecvPack.szRspCode);
/*
	#ifndef REGULAR
		if(dispVal == 1)
		{
			dispVal = 0;
			DL_ISO8583_MSG_Free(&isoMsg);
			return 0;
		}
	#endif*/
	

	sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
	//UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
	sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);
	FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
	strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);
	
	ShowLogs(1, "2. Response Code: %s", glRecvPack.szRspCode);

	ShowLogs(1, "Commencing Storage.");
	if(strlen(glRecvPack.szRspCode) == 2)
    {
    	if(txnType != 5)
    	{
	    	ShowLogs(1, "About Storing Transaction.");
	    	storeTxn();
	    	ShowLogs(1, "About Storing Eod.");
			storeEod();
			ShowLogs(1, "About printing.");
			storeprint();
		}
    }else
    {
    	customStorage(szLocalTime, "100");
    	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
    	//No response
    	titi = 1;
    }
    ShowLogs(1, "Done with Storing. Modify");
    ShowLogs(1, "3. Response Code: %s", glRecvPack.szRspCode);

	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		DisplayInfoNone("", "Trans Success", 1);
		//PackCallHomeData();
	}else
	{
		DisplayInfoNone("", "Trans Declined", 1);
		//PackCallHomeData();
	}

    
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}



int SendPayattitude(char *icc, int *reversal)
{

	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char temp2[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char stan[6] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char dataStore[254] = {0};
	uchar	szLocalTime[14+1];
	uchar outputData[10240] = {0};
	char parseTrack2[40] = {0};
	int iTemp;

	ShowLogs(1, "INSIDE SEND EMV DATA");
	checkVerve = 0;

	ShowLogs(1, "PAN: %s", glSendPack.szPan);

	titi = 0;
	rvApr = 0;

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	//SaleInput(); 
	SetCommReqField();

	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));


	memset(packBuf, 0x0, sizeof(packBuf));

	SysGetTimeIso(timeGotten);

	strncpy(datetime, timeGotten + 2, 10);
	sprintf((char *)glSendPack.szLocalDateTime, "%.*s", LEN_LOCAL_DATE_TIME, datetime);
	strncpy(dt, timeGotten + 2, 4);
	sprintf((char *)glSendPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dt);
	strncpy(tm, timeGotten + 6, 6);
	sprintf((char *)glSendPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, tm);

	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
	memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
	memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
	memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
	memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
	memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
	memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	/*if(strlen(glSendPack.szMsgCode) > 0)
	{*/
		ShowLogs(1, "Getting mti for normal cards: %s", glSendPack.szMsgCode);
		ShowLogs(1, "Setting mti as: %s", "1200");
		(void)DL_ISO8583_MSG_SetField_Str(0, "1200", &isoMsg);//glSendPack.szMsgCode
	//}

	memset(glSendPack.szPan,0x00,sizeof(glSendPack.szPan));
	sprintf(glSendPack.szPan,"9501000000000001");
//	if(strlen(glSendPack.szPan) > 0)
//	{
		ShowLogs(1, "PAN: %s", glSendPack.szPan);
		(void)DL_ISO8583_MSG_SetField_Str(2, glSendPack.szPan, &isoMsg);
//	}

	memset(glSendPack.szProcCode,0x00,sizeof(glSendPack.szProcCode));
	sprintf(glSendPack.szProcCode,"000000");
//	if(strlen(glSendPack.szProcCode) > 0)
//	{
		(void)DL_ISO8583_MSG_SetField_Str(3, glSendPack.szProcCode, &isoMsg);
		ShowLogs(1, "FIELD 4: %s", glSendPack.szProcCode);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szProcCode), glSendPack.szProcCode);
//	}

	/*if(strlen(glSendPack.szTranAmt) > 0)
	{
		char stamp[20] = {0};
		char tt[20] = {0};
		parseAmount(glSendPack.szTranAmt, tt);
		ShowLogs(1, "Converted Amount: %s.", tt);
	    double tot = atof(tt);
	    UtilGetEnvEx("stampduty", stamp);
	    ShowLogs(1, "Stampduty: %s.", stamp);
	    if((strstr(stamp, "true") != NULL) && (tot >= 1000)
	      && (glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	    {
	    	char fina[13] = {0};
	    	memset(fina, '\0', strlen(fina));
		    PubAscAdd(glSendPack.szTranAmt, "000000005000", 12, fina);
		    ShowLogs(1, "Total: %s", fina);
		    (void)DL_ISO8583_MSG_SetField_Str(4, fina, &isoMsg);
	    }else*/
		sprintf((char *)glSendPack.szTranAmt, "%.*s", LEN_TRAN_AMT, glProcInfo.stTranLog.szAmount);
			(void)DL_ISO8583_MSG_SetField_Str(4, glSendPack.szTranAmt, &isoMsg);
	//}
	if(strlen(glSendPack.szLocalDateTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(7, glSendPack.szLocalDateTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDateTime), glSendPack.szLocalDateTime);
	}

	GetStan();
	if(strlen(glSendPack.szSTAN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(11, glSendPack.szSTAN, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szSTAN), glSendPack.szSTAN);
	}else{
		(void)DL_ISO8583_MSG_SetField_Str(11, "000000", &isoMsg);
	}
	GetStan();
	if(strlen(glSendPack.szLocalTime) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(12, glSendPack.szLocalTime, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalTime), glSendPack.szLocalTime);
	}
	//if(strlen(glSendPack.szLocalDate) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(13, glSendPack.szLocalDate, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szLocalDate), glSendPack.szLocalDate);
	}
	//if(strlen(glSendPack.szExpDate) > 0)
	//{
		(void)DL_ISO8583_MSG_SetField_Str(14, "3012", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szExpDate), glSendPack.szExpDate);
	//}
	if(strlen(glSendPack.szMerchantType) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(18, glSendPack.szMerchantType, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantType), glSendPack.szMerchantType);
	}

		(void)DL_ISO8583_MSG_SetField_Str(22, "051", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szEntryMode), glSendPack.szEntryMode);


		(void)DL_ISO8583_MSG_SetField_Str(23, "000", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPanSeqNo), glSendPack.szPanSeqNo);


		(void)DL_ISO8583_MSG_SetField_Str(25, "00", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szCondCode), glSendPack.szCondCode);


		(void)DL_ISO8583_MSG_SetField_Str(26, "06", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPoscCode), glSendPack.szPoscCode);

		(void)DL_ISO8583_MSG_SetField_Str(28,"C00000000", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTransFee), glSendPack.szTransFee);

//	if(strlen(glSendPack.szAqurId) > 0)
//	{
		(void)DL_ISO8583_MSG_SetField_Str(32, "111129", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAqurId), glSendPack.szAqurId);
		//DisplayInfoNone("", temp2, 3);
//	}

	memset(glSendPack.szTrack2,0x00,sizeof(glSendPack.szTrack2));
	sprintf(glSendPack.szTrack2,"9501000000000001D3012");
	//if(strlen(glSendPack.szTrack2) > 0)
	//{
		memset(parseTrack2, '\0', strlen(parseTrack2));
		parsetrack2(glSendPack.szTrack2, parseTrack2);
		(void)DL_ISO8583_MSG_SetField_Str(35, parseTrack2, &isoMsg);

		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(parseTrack2), parseTrack2);
		//DisplayInfoNone("", temp2, 3);
	//}
	sprintf((char *)stan, "%06lu", useStan);  //??
	GetStan();
	sprintf((char *)glSendPack.szRRN,       "000000%.12s", stan);
	if(strlen(glSendPack.szRRN) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(37, glSendPack.szRRN, &isoMsg);
	}else{
		(void)DL_ISO8583_MSG_SetField_Str(37, stan, &isoMsg);
	}
	if(strlen(glSendPack.szAuthCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(38, glSendPack.szAuthCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szAuthCode), glSendPack.szAuthCode);
		//DisplayInfoNone("", temp2, 3);
	}
	//if(strlen(glSendPack.szServResCode) > 0)
	//{
		(void)DL_ISO8583_MSG_SetField_Str(40, "221", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szServResCode), glSendPack.szServResCode);
		//DisplayInfoNone("", temp2, 3);
	//}

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	strcpy(glSendPack.szTermID, temp);

	if(strlen(glSendPack.szTermID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(41, glSendPack.szTermID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTermID), glSendPack.szTermID);
		//DisplayInfoNone("", temp2, 3);
	}

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMid", temp);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, temp);


	if(strlen(glSendPack.szMerchantID) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(42, glSendPack.szMerchantID, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTermID), glSendPack.szTermID);
		//DisplayInfoNone("", temp2, 3);
	}
	else (void)DL_ISO8583_MSG_SetField_Str(42, "123456789012345", &isoMsg);

//2UBALA000002870
//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMerchantID), glSendPack.szMerchantID);

	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("txnMNL", temp);
	sprintf((char *)glSendPack.szMNL, "%.*s", LEN_MNL, temp);

	if(strlen(glSendPack.szMNL) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szMNL), glSendPack.szMNL);
		//DisplayInfoNone("", temp2, 3);
	}
	//(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
		else (void)DL_ISO8583_MSG_SetField_Str(43, "ETOP NIGERIA LIMITED   LA           LANG", &isoMsg);
		
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTranCurcyCode), glSendPack.szTranCurcyCode);
		//DisplayInfoNone("", temp2, 3);
	/*if(pincheck == 1)
	{
		if(strlen(glSendPack.szPinBlock) > 0)
		{
			(void)DL_ISO8583_MSG_SetField_Str(52, glSendPack.szPinBlock, &isoMsg);
		}
		pincheck = 0;
	}*/
	/*if(strlen(glSendPack.testSICCData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, glSendPack.testSICCData, &isoMsg);
	}*/
	
	/*if(icc != NULL)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, icc, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(icc), icc);
		//DisplayInfoNone("", temp2, 3);
	}*/
	if(strlen(glSendPack.szReasonCode) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(56, glSendPack.szReasonCode, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReasonCode), glSendPack.szReasonCode);
		//DisplayInfoNone("", temp2, 3);
	}

	if(strlen(glSendPack.szTEchoData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(59, glSendPack.szTEchoData, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szTEchoData), glSendPack.szTEchoData);//log
		//DisplayInfoNone("", temp2, 3);
	}
	//Added for vas
	//if(strlen(glSendPack.szBillers) > 0)
	//{
		memset(PAYATT_DE62,0,sizeof(PAYATT_DE62));
		sprintf(PAYATT_DE62,"%s%s","00698MP0101333",PAYATT_PHONENO);
		//sprintf(PAYATT_DE62,"%s%s","00698WD0101333",PAYATT_PHONENO);
		(void)DL_ISO8583_MSG_SetField_Str(62, PAYATT_DE62, &isoMsg);
	//}

	if(strlen(glSendPack.szOrigDataElement) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(90, glSendPack.szOrigDataElement, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d -- %s", strlen(glSendPack.szOrigDataElement), glSendPack.szOrigDataElement);
		//DisplayInfoNone("", temp2, 3);
	}
	if(strlen(glSendPack.szReplAmount) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(95, glSendPack.szReplAmount, &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szReplAmount), glSendPack.szReplAmount);
		//DisplayInfoNone("", temp2, 3);
	}
	//if(strlen(glSendPack.szPosDataCode) > 0)
	//{
		//(void)DL_ISO8583_MSG_SetField_Str(123, "510101511344001", &isoMsg);
		(void)DL_ISO8583_MSG_SetField_Str(123, "510101511344101", &isoMsg);
		//memset(temp2, '\0', strlen(temp2));
		//sprintf(temp2, "%.2d - %s", strlen(glSendPack.szPosDataCode), glSendPack.szPosDataCode);
		//DisplayInfoNone("", temp2, 3);
	//}
	(void) DL_ISO8583_MSG_SetField_Str(128, 0x0, &isoMsg);
	
	/*if((glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	{
		ShowLogs(1, "This is Verve. Please check if there is need to backup");
		if(txnType == 14 || txnType == 15 || 
			txnType == 16 || txnType == 17 || txnType == 18)
		{
			ShowLogs(1, "This is Verve. Please back up");
			backupVerveWithVal();
		}
	}*/

	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);
	ShowLogs(1, "Iso Message With Length: %s", hexData);

	if (packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(128, outHash, &isoMsg);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}
	//Fine till here
	ShowLogs(1, "Iso Message afterhash: %s",packBuf );

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	//UtilGetEnv("cotype", temp);
	strcat(temp,"GPRS");
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		//UtilGetEnvEx("uhostip", temp);
		strcat(temp,"52.31.200.20");//139.162.203.47/62.173.47.4
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		//UtilGetEnvEx("uhostport", temp);
		strcat(temp,"80");//9001/6011
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		//UtilGetEnvEx("uhostssl", temp);
		/*strcat(temp,"false");
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
			//Investigate from here tomorrow
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				customStorage(szLocalTime, "102");
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				customStorage(szLocalTime, "101");
				*reversal = 1;
				ShowLogs(1, "Ima Mmi: %d", reversal);
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	*/
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
 			ShowLogs(1, "CommInitModule Start");
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			ShowLogs(1, "CommsDial Start");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "102");
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);

			ShowLogs(1, "Sending: %s",packBuf );

	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				customStorage(szLocalTime, "101");
				*reversal = 1;
				return -1;
			}
			CommOnHookGPRS(TRUE);
 		//}
 	}
	
 	ShowLogs(1, "Done With Nibss");
	ScrBackLight(1);
 	memset(hexData, '\0', strlen(hexData));
	HexEnCodeMethod(output, strlen(output) + 2, hexData);
	ShowLogs(1, "Received From Nibss: %s", hexData);
	
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	
	
	int i = 0;

	//memset(&glRecvPack, 0, sizeof(STISO8583));//This is the proble
	
	//ShowLogs(1, "Commented out");
	for(i = 0; i < 129; i++)
	{
		if ( NULL != isoMsg.field[i].ptr ) 
		{
			memset(dataStore, '\0', strlen(dataStore));
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			sprintf(dataStore, "%s", isoMsg.field[i].ptr);
			switch(i)
			{
				case 0:
					memset(glRecvPack.szMsgCode, '\0', strlen(glRecvPack.szMsgCode));
					sprintf((char *)glRecvPack.szMsgCode, "%.*s", LEN_MSG_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMsgCode);
					continue;
				case 1:
					memset(glRecvPack.sBitMap, '\0', strlen(glRecvPack.sBitMap));
					sprintf((char *)glRecvPack.sBitMap, "%.*s", 2*LEN_BITMAP, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sBitMap);
					continue;
				case 2:
					memset(glRecvPack.szPan, '\0', strlen(glRecvPack.szPan));
					sprintf((char *)glRecvPack.szPan, "%.*s", LEN_PAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPan);
					continue;
				case 3:
					memset(glRecvPack.szProcCode, '\0', strlen(glRecvPack.szProcCode));
					sprintf((char *)glRecvPack.szProcCode, "%.*s", LEN_PROC_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szProcCode);
					continue;
				case 4:
					memset(glRecvPack.szTranAmt, '\0', strlen(glRecvPack.szTranAmt));
					sprintf((char *)glRecvPack.szTranAmt, "%.*s", LEN_TRAN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranAmt);
					continue;
				case 7:
					memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
					sprintf((char *)glRecvPack.szFrnAmt, "%.*s", LEN_FRN_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFrnAmt);
					continue;
				case 11:
					memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
					sprintf((char *)glRecvPack.szSTAN, "%.*s", LEN_STAN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSTAN);
					continue;
				case 12:
					memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
					sprintf((char *)glRecvPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalTime);
					continue;
				case 13:
					memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
					sprintf((char *)glRecvPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalDate);
					continue;
				case 14:
					memset(glRecvPack.szExpDate, '\0', strlen(glRecvPack.szExpDate));
					sprintf((char *)glRecvPack.szExpDate, "%.*s", LEN_EXP_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szExpDate);
					continue;
				case 15:
					memset(glRecvPack.szSetlDate, '\0', strlen(glRecvPack.szSetlDate));
					sprintf((char *)glRecvPack.szSetlDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szSetlDate);
					continue;
				case 18:
					memset(glRecvPack.szMerchantType, '\0', strlen(glRecvPack.szMerchantType));
					sprintf((char *)glRecvPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantType);
					continue;
				case 22:
					memset(glRecvPack.szEntryMode, '\0', strlen(glRecvPack.szEntryMode));
					sprintf((char *)glRecvPack.szEntryMode, "%.*s", LEN_ENTRY_MODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szEntryMode);
					continue;
				case 23:
					memset(glRecvPack.szPanSeqNo, '\0', strlen(glRecvPack.szPanSeqNo));
					sprintf((char *)glRecvPack.szPanSeqNo, "%.*s", LEN_PAN_SEQ_NO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPanSeqNo);
					continue;
				case 25:
					memset(glRecvPack.szCondCode, '\0', strlen(glRecvPack.szCondCode));
					sprintf((char *)glRecvPack.szCondCode, "%.*s", LEN_COND_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szCondCode);
					continue;
				case 28:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 30:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 32:
					memset(glRecvPack.szAqurId, '\0', strlen(glRecvPack.szAqurId));
					sprintf((char *)glRecvPack.szAqurId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAqurId);
					continue;
				case 33:
					memset(glRecvPack.szFwdInstId, '\0', strlen(glRecvPack.szFwdInstId));
					sprintf((char *)glRecvPack.szFwdInstId, "%.*s", LEN_AQUR_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szFwdInstId);
					continue;
				case 35:
					memset(glRecvPack.szTrack2, '\0', strlen(glRecvPack.szTrack2));
					sprintf((char *)glRecvPack.szTrack2, "%.*s", LEN_TRACK2, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTrack2);
					continue;
				case 37:
					memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
					sprintf((char *)glRecvPack.szRRN, "%.*s", LEN_RRN, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRRN);
					continue;
				case 38:
					memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
					sprintf((char *)glRecvPack.szAuthCode, "%.*s", LEN_AUTH_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAuthCode);
					continue;
				case 39:
					memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
					sprintf((char *)glRecvPack.szRspCode, "%.*s", LEN_RSP_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szRspCode);
					continue;
				case 40:
					memset(glRecvPack.szServResCode, '\0', strlen(glRecvPack.szServResCode));
					sprintf((char *)glRecvPack.szServResCode, "%.*s", LEN_SRES_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szServResCode);
					continue;
				case 41:
					memset(glRecvPack.szTermID, '\0', strlen(glRecvPack.szTermID));
					sprintf((char *)glRecvPack.szTermID, "%.*s", LEN_TERM_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTermID);
					continue;
				case 42:
					memset(glRecvPack.szMerchantID, '\0', strlen(glRecvPack.szMerchantID));
					sprintf((char *)glRecvPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantID);
					continue;
				case 43:
					memset(glRecvPack.szMNL, '\0', strlen(glRecvPack.szMNL));
					sprintf((char *)glRecvPack.szMNL, "%.*s", LEN_MNL, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szMNL);
					continue;
				case 49:
					memset(glRecvPack.szTranCurcyCode, '\0', strlen(glRecvPack.szTranCurcyCode));
					sprintf((char *)glRecvPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranCurcyCode);
					continue;
				case 54:
					memset(glRecvPack.szAddtAmount, '\0', strlen(glRecvPack.szAddtAmount));
					sprintf((char *)glRecvPack.szAddtAmount, "%.*s", LEN_ADDT_AMT, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szAddtAmount);
					continue;
				case 55:
					memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
					sprintf((char *)glRecvPack.sICCData, "%.*s", LEN_ICC_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.sICCData);
					continue;
				case 59:
					memset(glRecvPack.szTEchoData, '\0', strlen(glRecvPack.szTEchoData));
					sprintf((char *)glRecvPack.szTEchoData, "%.*s", LEN_TRANSECHO_DATA, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szTEchoData);
					continue;
				case 60:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 62:
					memset(glRecvPack.szBillers, '\0', strlen(glRecvPack.szBillers));
					sprintf((char *)glRecvPack.szBillers, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szBillers);
					continue;
				case 102:
					memset(glRecvPack.szActIdent1, '\0', strlen(glRecvPack.szActIdent1));
					sprintf((char *)glRecvPack.szActIdent1, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent1);
					continue;
				case 103:
					memset(glRecvPack.szActIdent2, '\0', strlen(glRecvPack.szActIdent2));
					sprintf((char *)glRecvPack.szActIdent2, "%.*s", LEN_ACT_IDTF, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent2);
					continue;
				case 123:
					memset(glRecvPack.szPosDataCode, '\0', strlen(glRecvPack.szPosDataCode));
					sprintf((char *)glRecvPack.szPosDataCode, "%.*s", LEN_POS_CODE, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPosDataCode);
					continue;
				case 124:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				case 128:
					memset(glRecvPack.szNFC, '\0', strlen(glRecvPack.szNFC));
					sprintf((char *)glRecvPack.szNFC, "%.*s", LEN_NEARFIELD, dataStore);
					//ShowLogs(1, "Field %d: %s", i, glRecvPack.szNFC);
					continue;
				default:
					continue;
			}
		}
	}
	ShowLogs(1, "Response Code: %s", glRecvPack.szRspCode);
/*
	#ifndef REGULAR
		if(dispVal == 1)
		{
			dispVal = 0;
			DL_ISO8583_MSG_Free(&isoMsg);
			return 0;
		}
	#endif*/
	

	sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
	//UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
	sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);
	FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
	strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);
	
	ShowLogs(1, "2. Response Code: %s", glRecvPack.szRspCode);

	ShowLogs(1, "Commencing Storage.");
	if(strlen(glRecvPack.szRspCode) == 2)
    {
    	if(txnType != 5)
    	{
	    	ShowLogs(1, "About Storing Transaction.");
	    	storeTxn();
	    	ShowLogs(1, "About Storing Eod.");
			storeEod();
			ShowLogs(1, "About printing.");
			storeprint();
		}
    }else
    {
    	customStorage(szLocalTime, "100");
    	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
    	//No response
    	titi = 1;
    }
    ShowLogs(1, "Done with Storing. Modify");
    ShowLogs(1, "3. Response Code: %s", glRecvPack.szRspCode);

	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		DisplayInfoNone("", "Trans Success", 1);
		//PackCallHomeData();
	}else
	{
		DisplayInfoNone("", "Trans Declined", 1);
		//PackCallHomeData();
	}

    
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}



int sendToSucabo(char *body, char *outlv)
{
	int iRet, iLen = 0;
	char output[1024] = { 0 };
	char temp[512] = { 0 };
	uchar	szLocalTime[14+1];
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	char out[3 * 1024] = { 0 };

	ShowLogs(1, "LOVE GOING: %s", body);

	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOIP);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOPORT);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		if(1)
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
 			ShowLogs(1, "CommInitModule Start");
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			ShowLogs(1, "CommsDial Start");
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(body, strlen(body), 180);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Receiving...", 0);
			memset(output, '\0', strlen(output));
			iRet = CommRxd(output, 1024, 180, &iLen);
			ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				return 0;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOIP);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		strcpy(temp, SUCABOPORT);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		if(1)
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				return 0;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(body, strlen(body), 180);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				return 0;
			}
			DisplayInfoNone("", "Receiving...", 0);
			memset(output, '\0', strlen(output));
			iRet = CommRxd(output, 1024, 180, &iLen);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				return 0;
			}
			CommOnHook(TRUE);
 		}
	}

 	ShowLogs(1, "Done With Transfer");
	ScrBackLight(1);
 	ShowLogs(1, "Received From Sucabo: %s", output);
	Beep();

	strcpy(outlv, output);
	return 1;
}

void forceKeyExchange()
{
	char finalsend[3 * 1024] = {0};
	char temp[1024] = {0};
	char output[2 * 1024] = {0};
	char value[2 * 1024] = {0};
	uchar	szEngTime[16+1];

	strcpy(finalsend, "{\"totalamount\": \"0.00\",\"token\": \"PAX\",\"username\": \"PAX\",");
	strcat(finalsend, "\"transaction\": \"KEY EXCHANGE\",\"tid\":\"");
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("tid", temp);
	strcat(finalsend, temp);
	strcat(finalsend, "\",\"clrsesskey\":\"NA\",\"clrpinkey\":\"NA\",\"emvdata\":{},\"interswitch\":{},\"wallet\":{\"tid\":\"");
	strcat(finalsend, temp);
	strcat(finalsend, "\",\"component\":\"");
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("swkcomponent1", temp);
	strcat(finalsend, temp);
	strcat(finalsend, "\",\"ip\":\"");
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("hostip", temp);
	strcat(finalsend, temp);
	strcat(finalsend, "\",\"port\":");
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("hostport", temp);
	strcat(finalsend, temp);
	strcat(finalsend, ",\"ssl\":");
	memset(temp, '\0', strlen(temp));
	UtilGetEnvEx("hostssl", temp);
	strcat(finalsend, temp);
	strcat(finalsend, "}}\n");
	ShowLogs(1, "NUMBER OF TIMES TO COME HERE");
	sendToSucabo(finalsend, output);

	if(strlen(output) < 50)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(szEngTime, gl_stTitleAttr, _T("KEY EXCHANGE FAILED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		DelayMs(5000);
		//DisplayInfoNone("", "USING GPRS", 2);
		ShowLogs(1, "KEY EXCHANGE FAILED");
		return;
	}

	memset(value, '\0', strlen(value));
	getJsonValue(output, "clrmasterkey", value);
	CreateWrite("isomastkey.txt", value);

	memset(value, '\0', strlen(value));
	getJsonValue(output, "clrsesskey", value);
	CreateWrite("isosesskey.txt", value);

	memset(value, '\0', strlen(value));
	getJsonValue(output, "encpinkey", value);
	CreateWrite("encpinkey.txt", value);
	injectPinKey();

	memset(value, '\0', strlen(value));
	getJsonValue(output, "clrpinkey", value);
	CreateWrite("isopinkey.txt", value);

	memset(value, '\0', strlen(value));
	getJsonValue(output, "paramdownload", value);
	CreateWrite("param.txt", value);
	parseParameters(value);
}

int rvApr = 0;

int SendReversal()
{
	revSend = 1;

	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char dataStore[254] = {0};
	uchar	szLocalTime[14+1];
	uchar	szSTAN[LEN_STAN+2];
	char field90[100];
	char field95[100];

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));

	memset(packBuf, 0x0, sizeof(packBuf));

	SysGetTimeIso(timeGotten);
	strncpy(datetime, timeGotten + 2, 10);
	strncpy(dt, timeGotten + 2, 4);
	strncpy(tm, timeGotten + 6, 6);

	memset(field90, '\0', strlen(field90));
	memset(field95, '\0', strlen(field95));
	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
	memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
	memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
	memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
	memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
	memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
	memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	(void)DL_ISO8583_MSG_SetField_Str(0, "0420", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(2, glSendPack.szPan, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(3, glSendPack.szProcCode, &isoMsg);
	if(strlen(glSendPack.szTranAmt) > 0)
	{
		char stamp[20] = {0};
		char tt[20] = {0};
		parseAmount(glSendPack.szTranAmt, tt);
		ShowLogs(1, "Converted Amount: %s.", tt);
	    double tot = atof(tt);
	    UtilGetEnvEx("stampduty", stamp);
	    ShowLogs(1, "Stampduty: %s.", stamp);
	    if((strstr(stamp, "true") != NULL)
	      && (tot >= 1000)
	      && (glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	    {
	    	char fina[13] = {0};
	    	memset(fina, '\0', strlen(fina));
		    PubAscAdd(glSendPack.szTranAmt, "000000005000", 12, fina);
		    ShowLogs(1, "Total: %s", fina);
		    (void)DL_ISO8583_MSG_SetField_Str(4, fina, &isoMsg);
	    }else
			(void)DL_ISO8583_MSG_SetField_Str(4, glSendPack.szTranAmt, &isoMsg);
	}
	(void)DL_ISO8583_MSG_SetField_Str(7, datetime, &isoMsg);
	
	//glSysCtrl.ulSTAN = useStan;
	//sprintf((char *)szSTAN, "%06lu", glSysCtrl.ulSTAN);  //??
	//glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
	
	(void)DL_ISO8583_MSG_SetField_Str(11, glSendPack.szSTAN, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(12, tm, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(13, dt, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(14, glSendPack.szExpDate, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(18, glSendPack.szMerchantType, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(22, glSendPack.szEntryMode + 1, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(23, glSendPack.szPanSeqNo, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(25, glSendPack.szCondCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(26, glSendPack.szPoscCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(28, glSendPack.szTransFee, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(35, glSendPack.szTrack2, &isoMsg);
	//sprintf((char *)glSendPack.szRRN, "000000%s", szSTAN);
	(void)DL_ISO8583_MSG_SetField_Str(37, glSendPack.szRRN, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(41, glSendPack.szTermID, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(42, glSendPack.szMerchantID, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(49, glSendPack.szTranCurcyCode, &isoMsg);
	if(strlen(glSendPack.testSICCData) > 0)
	{
		(void)DL_ISO8583_MSG_SetField_Str(55, glSendPack.testSICCData, &isoMsg);
	}
	(void)DL_ISO8583_MSG_SetField_Str(56, "4021", &isoMsg);
	strcpy(field90, glSendPack.szMsgCode);
	strcat(field90, glSendPack.szSTAN);
	strcat(field90, glSendPack.szLocalDateTime);
	strcat(field90, "0000011112900000111129");
	(void)DL_ISO8583_MSG_SetField_Str(90, field90, &isoMsg);
	strcpy(field95, glSendPack.szTranAmt);
	strcat(field95, "000000000000");
	strcat(field95, "C00000000");
	strcat(field95, "C00000000");
	(void)DL_ISO8583_MSG_SetField_Str(95, field95, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(123, glSendPack.szPosDataCode, &isoMsg);
	(void) DL_ISO8583_MSG_SetField_Str(128, 0x0, &isoMsg);
		
	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);

	if (packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(128, outHash, &isoMsg);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}
	
	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				//rvApr = 1;
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}
 	ShowLogs(1, "Done With Nibss");
	ScrBackLight(1);
 	memset(hexData, '\0', strlen(hexData));
	HexEnCodeMethod(output, strlen(output) + 2, hexData);
	ShowLogs(1, "Received From Nibss: %s", hexData);
	
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	
	
	int i = 0;

	//memset(&glRecvPack, 0, sizeof(STISO8583));//This is the problem
	
	ShowLogs(1, "Commented out");
	for(i = 0; i < 129; i++)
	{
		if ( NULL != isoMsg.field[i].ptr ) 
		{
			memset(dataStore, '\0', strlen(dataStore));
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			sprintf(dataStore, "%s", isoMsg.field[i].ptr);
			switch(i)
			{
				case 0:
					memset(glRecvPack.szMsgCode, '\0', strlen(glRecvPack.szMsgCode));
					sprintf((char *)glRecvPack.szMsgCode, "%.*s", LEN_MSG_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMsgCode);
					continue;
				case 1:
					memset(glRecvPack.sBitMap, '\0', strlen(glRecvPack.sBitMap));
					sprintf((char *)glRecvPack.sBitMap, "%.*s", 2*LEN_BITMAP, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.sBitMap);
					continue;
				case 2:
					memset(glRecvPack.szPan, '\0', strlen(glRecvPack.szPan));
					sprintf((char *)glRecvPack.szPan, "%.*s", LEN_PAN, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPan);
					continue;
				case 3:
					memset(glRecvPack.szProcCode, '\0', strlen(glRecvPack.szProcCode));
					sprintf((char *)glRecvPack.szProcCode, "%.*s", LEN_PROC_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szProcCode);
					continue;
				case 4:
					memset(glRecvPack.szTranAmt, '\0', strlen(glRecvPack.szTranAmt));
					sprintf((char *)glRecvPack.szTranAmt, "%.*s", LEN_TRAN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranAmt);
					continue;
				case 7:
					memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
					sprintf((char *)glRecvPack.szFrnAmt, "%.*s", LEN_FRN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szFrnAmt);
					continue;
				case 11:
					memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
					sprintf((char *)glRecvPack.szSTAN, "%.*s", LEN_STAN, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szSTAN);
					continue;
				case 12:
					memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
					sprintf((char *)glRecvPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalTime);
					continue;
				case 13:
					memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
					sprintf((char *)glRecvPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalDate);
					continue;
				case 14:
					memset(glRecvPack.szExpDate, '\0', strlen(glRecvPack.szExpDate));
					sprintf((char *)glRecvPack.szExpDate, "%.*s", LEN_EXP_DATE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szExpDate);
					continue;
				case 15:
					memset(glRecvPack.szSetlDate, '\0', strlen(glRecvPack.szSetlDate));
					sprintf((char *)glRecvPack.szSetlDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szSetlDate);
					continue;
				case 18:
					memset(glRecvPack.szMerchantType, '\0', strlen(glRecvPack.szMerchantType));
					sprintf((char *)glRecvPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantType);
					continue;
				case 22:
					memset(glRecvPack.szEntryMode, '\0', strlen(glRecvPack.szEntryMode));
					sprintf((char *)glRecvPack.szEntryMode, "%.*s", LEN_ENTRY_MODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szEntryMode);
					continue;
				case 23:
					memset(glRecvPack.szPanSeqNo, '\0', strlen(glRecvPack.szPanSeqNo));
					sprintf((char *)glRecvPack.szPanSeqNo, "%.*s", LEN_PAN_SEQ_NO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPanSeqNo);
					continue;
				case 25:
					memset(glRecvPack.szCondCode, '\0', strlen(glRecvPack.szCondCode));
					sprintf((char *)glRecvPack.szCondCode, "%.*s", LEN_COND_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szCondCode);
					continue;
				case 28:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 30:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 32:
					memset(glRecvPack.szAqurId, '\0', strlen(glRecvPack.szAqurId));
					sprintf((char *)glRecvPack.szAqurId, "%.*s", LEN_AQUR_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szAqurId);
					continue;
				case 33:
					memset(glRecvPack.szFwdInstId, '\0', strlen(glRecvPack.szFwdInstId));
					sprintf((char *)glRecvPack.szFwdInstId, "%.*s", LEN_AQUR_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szFwdInstId);
					continue;
				case 35:
					memset(glRecvPack.szTrack2, '\0', strlen(glRecvPack.szTrack2));
					sprintf((char *)glRecvPack.szTrack2, "%.*s", LEN_TRACK2, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTrack2);
					continue;
				case 37:
					memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
					sprintf((char *)glRecvPack.szRRN, "%.*s", LEN_RRN, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szRRN);
					continue;
				case 38:
					memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
					//sprintf((char *)glRecvPack.szAuthCode, "%.*s", LEN_AUTH_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szAuthCode);
					continue;
				case 39:
					memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
					sprintf((char *)glRecvPack.szRspCode, "%.*s", LEN_RSP_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szRspCode);
					continue;
				case 40:
					memset(glRecvPack.szServResCode, '\0', strlen(glRecvPack.szServResCode));
					sprintf((char *)glRecvPack.szServResCode, "%.*s", LEN_SRES_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szServResCode);
					continue;
				case 41:
					memset(glRecvPack.szTermID, '\0', strlen(glRecvPack.szTermID));
					sprintf((char *)glRecvPack.szTermID, "%.*s", LEN_TERM_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTermID);
					continue;
				case 42:
					memset(glRecvPack.szMerchantID, '\0', strlen(glRecvPack.szMerchantID));
					sprintf((char *)glRecvPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantID);
					continue;
				case 43:
					memset(glRecvPack.szMNL, '\0', strlen(glRecvPack.szMNL));
					sprintf((char *)glRecvPack.szMNL, "%.*s", LEN_MNL, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMNL);
					continue;
				case 49:
					memset(glRecvPack.szTranCurcyCode, '\0', strlen(glRecvPack.szTranCurcyCode));
					sprintf((char *)glRecvPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranCurcyCode);
					continue;
				case 54:
					memset(glRecvPack.szAddtAmount, '\0', strlen(glRecvPack.szAddtAmount));
					sprintf((char *)glRecvPack.szAddtAmount, "%.*s", LEN_ADDT_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szAddtAmount);
					continue;
				case 55:
					memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
					sprintf((char *)glRecvPack.sICCData, "%.*s", LEN_ICC_DATA, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.sICCData);
					continue;
				case 59:
					memset(glRecvPack.szTEchoData, '\0', strlen(glRecvPack.szTEchoData));
					sprintf((char *)glRecvPack.szTEchoData, "%.*s", LEN_TRANSECHO_DATA, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTEchoData);
					continue;
				case 60:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				/*case 102:
					memset(glRecvPack.szActIdent1, '\0', strlen(glRecvPack.szActIdent1));
					sprintf((char *)glRecvPack.szActIdent1, "%.*s", LEN_ACT_IDTF, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent1);
					continue;
				case 103:
					memset(glRecvPack.szActIdent2, '\0', strlen(glRecvPack.szActIdent2));
					sprintf((char *)glRecvPack.szActIdent2, "%.*s", LEN_ACT_IDTF, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent2);
					continue;*/
				case 123:
					memset(glRecvPack.szPosDataCode, '\0', strlen(glRecvPack.szPosDataCode));
					sprintf((char *)glRecvPack.szPosDataCode, "%.*s", LEN_POS_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPosDataCode);
					continue;
				case 124:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				/*case 128:
					memset(glRecvPack.szNFC, '\0', strlen(glRecvPack.szNFC));
					sprintf((char *)glRecvPack.szNFC, "%.*s", LEN_NEARFIELD, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szNFC);
					continue;*/
				default:
					continue;
			}
		}
	}
	ShowLogs(1, "Reversal Response Code: %s", glRecvPack.szRspCode);
	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		DisplayInfoNone("", "Transaction Reversed", 3);
		memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
		memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
		memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	}else if(strstr(glRecvPack.szRspCode, "25") != NULL)
	{
		DisplayInfoNone("", "Transaction Reversed", 3);
		memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
		memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
		memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	}else
	{
		DisplayInfoNone("", "Transaction Not Reversed", 3);
		memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
		memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
		memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
		memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
		memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	}

	//rvApr = 1;

	//Reset flag
	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, '\0', strlen(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, '\0', strlen(glProcInfo.stTranLog.szRRN));
	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));	


	
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}

int SendManualReversal()
{
	char hexData[5 * 1024] = { 0 };
	char output[5 * 1024] = { 0 };
	DL_UINT8 	packBuf[5 * 1024] = { 0x0 };
	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_GetHandler(&isoHandler);
	char timeGotten[15] = {0};
	char datetime[11] = {0};
	char dt[5] = {0};
	char tm[7] = {0};
	int iRet, iLen = 0;
	char temp[128] = {0};
	char keyStore[128] = {0};
	char keyFin[33] = {0};
	char tempStore[128] = {0};
	char resp[3] = {0};
	char SN[33] = {0};
	char theUHI[33] = {0};
	char theUHISend[33] = {0};
	context_sha256_t ctx;
	char tempOut[100] = { '\0' };
	char boutHash[100] = { 0x0 };
	char outHash[100] = { 0x0 };
	char dataStore[254] = {0};
	uchar	szLocalTime[14+1];
	uchar	szSTAN[LEN_STAN+2];
	char field90[100];
	char field95[100];

	if(lastUsedHost != 1)	
		CommOnHookCustom(TRUE);
	lastUsedHost = 1;

	memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
	memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
	memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
	memset(packBuf, 0x0, sizeof(packBuf));
	
	memset(field90, '\0', strlen(field90));
	memset(field95, '\0', strlen(field95));
	memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
	memset(glRecvPack.szLocalDateTime, '\0', strlen(glRecvPack.szLocalDateTime));
	memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
	memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
	memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
	memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
	memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
	memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
	memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
	memset(glRecvPack.szHolderCurcyCode, '\0', strlen(glRecvPack.szHolderCurcyCode));


	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);

	/* set ISO message fields */
	(void)DL_ISO8583_MSG_SetField_Str(0, "0420", &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(2, glSendPack.szPan, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(3, glSendPack.szProcCode, &isoMsg);
	if(strlen(glSendPack.szTranAmt) > 0)
	{
		char stamp[20] = {0};
		char tt[20] = {0};
		parseAmount(glSendPack.szTranAmt, tt);
		ShowLogs(1, "Converted Amount: %s.", tt);
	    double tot = atof(tt);
	    UtilGetEnvEx("stampduty", stamp);
	    ShowLogs(1, "Stampduty: %s.", stamp);
	    if((strstr(stamp, "true") != NULL)
	      && (tot >= 1000)
	      && (glSendPack.szPan[0] == '5') && (glSendPack.szPan[1] == '0') && (glSendPack.szPan[2] == '6'))
	    {
	    	char fina[13] = {0};
	    	memset(fina, '\0', strlen(fina));
		    PubAscAdd(glSendPack.szTranAmt, "000000005000", 12, fina);
		    ShowLogs(1, "Total: %s", fina);
		    (void)DL_ISO8583_MSG_SetField_Str(4, fina, &isoMsg);
	    }else
			(void)DL_ISO8583_MSG_SetField_Str(4, glSendPack.szTranAmt, &isoMsg);
	}
	(void)DL_ISO8583_MSG_SetField_Str(7, glSendPack.szLocalDateTime, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(11, glSendPack.szSTAN, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(12, glSendPack.szLocalTime, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(13, glSendPack.szLocalDate, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(14, glSendPack.szExpDate, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(18, glSendPack.szMerchantType, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(22, glSendPack.szEntryMode + 1, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(23, glSendPack.szPanSeqNo, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(25, glSendPack.szCondCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(26, glSendPack.szPoscCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(28, glSendPack.szTransFee, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(32, glSendPack.szAqurId, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(35, glSendPack.szTrack2, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(37, glSendPack.szRRN, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(40, glSendPack.szServResCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(41, glSendPack.szTermID, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(42, glSendPack.szMerchantID, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(43, glSendPack.szMNL, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(49, glSendPack.szTranCurcyCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(55, glSendPack.testSICCData, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(56, glSendPack.szReasonCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(90, glSendPack.szOrigDataElement, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(95, glSendPack.szReplAmount, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(123, glSendPack.szPosDataCode, &isoMsg);
	(void) DL_ISO8583_MSG_SetField_Str(128, 0x0, &isoMsg);
		
	sha256_starts(&ctx);
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf, &packedSize);
	HexEnCodeMethod((DL_UINT8*) packBuf, packedSize, hexData);
	ShowLogs(1, "Packed size %d", packedSize);

	if (packedSize >= 64) 
	{
		packBuf[packedSize - 64] = '\0';
		ShowLogs(1, "Packed ISO before hashing : %s", packBuf);
		memset(temp, '\0', strlen(temp));
		ReadAllData("isosesskey.txt", temp);
		ShowLogs(1, "Session key used for Hashing: %s", temp);
		memset(tempOut, 0x0, sizeof(tempOut));
		HexDecodeMethod((unsigned char*)temp, strlen(temp), (unsigned char*) tempOut);
		sha256_update(&ctx, (uint8_ts*) tempOut, 16);
		sha256_update(&ctx, (uint8_ts*) packBuf, (uint32_ts) strlen(packBuf));
		sha256_finish(&ctx, (uint8_ts*) boutHash);
		HexEnCodeMethod((unsigned char*) boutHash, 32, (unsigned char*) outHash);
		(void) DL_ISO8583_MSG_SetField_Str(128, outHash, &isoMsg);
		memset(packBuf, 0x0, sizeof(packBuf));
		(void) DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, &packBuf[2], &packedSize);
		packBuf[0] = packedSize >> 8;
		packBuf[1] = packedSize;
		DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
		DL_ISO8583_MSG_Free(&isoMsg);
	}
	
	//For Gprs
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
 	{
 		glSysParam.stTxnCommCfg.ucCommType = 5;
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("coapn", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szAPN, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("cosubnet", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szUID, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnv("copwd", temp);
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.szPwd, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}
	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHookGPRS(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHookGPRS(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHookGPRS(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			CommOnHookGPRS(TRUE);
 		}
 	}else
	{
		glSysParam.stTxnCommCfg.ucCommType = 6;
		CommSwitchType(CT_WIFI);
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostip", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP, temp, strlen(temp));
				
		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostport", temp);
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort, temp, strlen(temp));
		memset(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, '\0', sizeof(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort));
		memcpy(glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort, temp, strlen(temp));

		ShowLogs(1, "Wifi Ip 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szIP);
		ShowLogs(1, "Wifi Ip 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szIP);
		ShowLogs(1, "Wifi Port 1: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost1.szPort);
		ShowLogs(1, "Wifi Port 2: %s", glSysParam.stTxnCommCfg.stWifiPara.stHost2.szPort);

		memset(temp, '\0', strlen(temp));
		UtilGetEnvEx("uhostssl", temp);
		if(strstr(temp, "true") != NULL)
 		{
 			ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);
	 		
	 		EmvSetSSLFlag();
			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommsInitialization Successful");
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");

	 		iRet = CommDial(DM_DIAL);
	 		ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "CommDial Successful");
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = SxxSSLTxd(packBuf, packedSize + 2, 60);
	 		ShowLogs(1, "SxxSSLTxd Response: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLTxd failed.");
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLClose Successful");
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
	 		iRet = SxxSSLRxd(output, 4 * 1024, 60, &iLen);
			ShowLogs(1, "1. SxxSSLRxd Response Len: %d", iLen);
			ShowLogs(1, "2. SxxSSLRxd Response Ret: %d", iRet);
	 		if (iRet < 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "SxxSSLRxd failed.");
				SxxSSLClose();
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}else
			{
				ShowLogs(1, "SxxSSLRxd Successful");
			}
			SxxSSLClose();
 		}else
 		{	
 			//For non ssl
 			EmvUnsetSSLFlag();
 			ShowLogs(1, "Apn: %s", glSysParam.stTxnCommCfg.stWirlessPara.szAPN);
	 		ShowLogs(1, "Username: %s", glSysParam.stTxnCommCfg.stWirlessPara.szUID);
	 		ShowLogs(1, "Password: %s", glSysParam.stTxnCommCfg.stWirlessPara.szPwd);
	 		ShowLogs(1, "Ip 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szIP);
	 		ShowLogs(1, "Port 1: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost1.szPort);
	 		ShowLogs(1, "Ip 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szIP);
	 		ShowLogs(1, "Port 2: %s", glSysParam.stTxnCommCfg.stWirlessPara.stHost2.szPort);

 			DisplayInfoNone("", "Please Wait...", 0);
	 		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
	 		//ShowLogs(1, "CommsInitialization Response: %d", iRet);
			if (iRet != 0)
			{
				ShowLogs(1, "Comms Initialization failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}

			ShowLogs(1, "About Calling CommSetCfgParam");
			CommSetCfgParam(&glSysParam.stTxnCommCfg);
			ShowLogs(1, "Done Calling CommSetCfgParam");
	 		iRet = CommDial(DM_DIAL);
	 		//ShowLogs(1, "CommsDial Response: %d", iRet);
	 		if (iRet != 0)
			{
				ShowLogs(1, "Comms Dial failed.");
				DisplayInfoNone("", "Failed", 0);
				CommOnHook(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Sending...", 0);
	 		iRet = CommTxd(packBuf, packedSize + 2, 60);
	 		//ShowLogs(1, "CommTxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommTxd failed.");
				CommOnHook(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			DisplayInfoNone("", "Receiving Bytes...", 0);
			iRet = CommRxd(output, 4 * 1024, 60, &iLen);
			//ShowLogs(1, "CommRxd Response: %d", iRet);
	 		if (iRet != 0)
			{
				DisplayInfoNone("", "Failed", 0);
				ShowLogs(1, "CommRxd failed.");
				CommOnHook(TRUE);
				memset(glProcInfo.stTranLog.szRspCode, 0x0, sizeof(glProcInfo.stTranLog.szRspCode));
				memset(glProcInfo.stTranLog.szAuthCode, 0x0, sizeof(glProcInfo.stTranLog.szAuthCode));
				memset(glProcInfo.stTranLog.szRRN, 0x0, sizeof(glProcInfo.stTranLog.szRRN));
				memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
				memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
				return -1;
			}
			CommOnHook(TRUE);
 		}
	}
 	ShowLogs(1, "Done With Nibss");
	ScrBackLight(1);
 	memset(hexData, '\0', strlen(hexData));
	HexEnCodeMethod(output, strlen(output) + 2, hexData);
	ShowLogs(1, "Received From Nibss: %s", hexData);
	
	unsigned char ucByte = 0;
	ucByte = (unsigned char) strtoul(hexData, NULL, 16); 
	DL_ISO8583_MSG_Init(NULL,0,&isoMsg);
	(void)DL_ISO8583_MSG_Unpack(&isoHandler, &output[2], iLen - 2, &isoMsg);
	DL_ISO8583_MSG_Dump("", &isoHandler, &isoMsg);
	
	
	int i = 0;

	//memset(&glRecvPack, 0, sizeof(STISO8583));//This is the problem
	
	ShowLogs(1, "Commented out");
	for(i = 0; i < 129; i++)
	{
		if ( NULL != isoMsg.field[i].ptr ) 
		{
			memset(dataStore, '\0', strlen(dataStore));
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i, &isoHandler);
			sprintf(dataStore, "%s", isoMsg.field[i].ptr);
			switch(i)
			{
				case 0:
					memset(glRecvPack.szMsgCode, '\0', strlen(glRecvPack.szMsgCode));
					sprintf((char *)glRecvPack.szMsgCode, "%.*s", LEN_MSG_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMsgCode);
					continue;
				case 1:
					memset(glRecvPack.sBitMap, '\0', strlen(glRecvPack.sBitMap));
					sprintf((char *)glRecvPack.sBitMap, "%.*s", 2*LEN_BITMAP, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.sBitMap);
					continue;
				case 2:
					memset(glRecvPack.szPan, '\0', strlen(glRecvPack.szPan));
					sprintf((char *)glRecvPack.szPan, "%.*s", LEN_PAN, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPan);
					continue;
				case 3:
					memset(glRecvPack.szProcCode, '\0', strlen(glRecvPack.szProcCode));
					sprintf((char *)glRecvPack.szProcCode, "%.*s", LEN_PROC_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szProcCode);
					continue;
				case 4:
					memset(glRecvPack.szTranAmt, '\0', strlen(glRecvPack.szTranAmt));
					sprintf((char *)glRecvPack.szTranAmt, "%.*s", LEN_TRAN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranAmt);
					continue;
				case 7:
					memset(glRecvPack.szFrnAmt, '\0', strlen(glRecvPack.szFrnAmt));
					sprintf((char *)glRecvPack.szFrnAmt, "%.*s", LEN_FRN_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szFrnAmt);
					continue;
				case 11:
					memset(glRecvPack.szSTAN, '\0', strlen(glRecvPack.szSTAN));
					sprintf((char *)glRecvPack.szSTAN, "%.*s", LEN_STAN, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szSTAN);
					continue;
				case 12:
					memset(glRecvPack.szLocalTime, '\0', strlen(glRecvPack.szLocalTime));
					sprintf((char *)glRecvPack.szLocalTime, "%.*s", LEN_LOCAL_TIME, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalTime);
					continue;
				case 13:
					memset(glRecvPack.szLocalDate, '\0', strlen(glRecvPack.szLocalDate));
					sprintf((char *)glRecvPack.szLocalDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szLocalDate);
					continue;
				case 14:
					memset(glRecvPack.szExpDate, '\0', strlen(glRecvPack.szExpDate));
					sprintf((char *)glRecvPack.szExpDate, "%.*s", LEN_EXP_DATE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szExpDate);
					continue;//ok
				case 15:
					memset(glRecvPack.szSetlDate, '\0', strlen(glRecvPack.szSetlDate));
					sprintf((char *)glRecvPack.szSetlDate, "%.*s", LEN_LOCAL_DATE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szSetlDate);
					continue;
				case 18:
					memset(glRecvPack.szMerchantType, '\0', strlen(glRecvPack.szMerchantType));
					sprintf((char *)glRecvPack.szMerchantType, "%.*s", LEN_MERCHANT_TYPE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantType);
					continue;
				case 22:
					memset(glRecvPack.szEntryMode, '\0', strlen(glRecvPack.szEntryMode));
					sprintf((char *)glRecvPack.szEntryMode, "%.*s", LEN_ENTRY_MODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szEntryMode);
					continue;
				case 23:
					memset(glRecvPack.szPanSeqNo, '\0', strlen(glRecvPack.szPanSeqNo));
					sprintf((char *)glRecvPack.szPanSeqNo, "%.*s", LEN_PAN_SEQ_NO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPanSeqNo);
					continue;
				case 25:
					memset(glRecvPack.szCondCode, '\0', strlen(glRecvPack.szCondCode));
					sprintf((char *)glRecvPack.szCondCode, "%.*s", LEN_COND_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szCondCode);
					continue;
				case 28:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 30:
					memset(glRecvPack.szTransFee, '\0', strlen(glRecvPack.szTransFee));
					sprintf((char *)glRecvPack.szTransFee, "%.*s", LEN_TRANS_FEE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTransFee);
					continue;
				case 32:
					memset(glRecvPack.szAqurId, '\0', strlen(glRecvPack.szAqurId));
					sprintf((char *)glRecvPack.szAqurId, "%.*s", LEN_AQUR_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szAqurId);
					continue;
				case 33:
					memset(glRecvPack.szFwdInstId, '\0', strlen(glRecvPack.szFwdInstId));
					sprintf((char *)glRecvPack.szFwdInstId, "%.*s", LEN_AQUR_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szFwdInstId);
					continue;
				case 35:
					memset(glRecvPack.szTrack2, '\0', strlen(glRecvPack.szTrack2));
					sprintf((char *)glRecvPack.szTrack2, "%.*s", LEN_TRACK2, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTrack2);
					continue;
				case 37:
					memset(glRecvPack.szRRN, '\0', strlen(glRecvPack.szRRN));
					sprintf((char *)glRecvPack.szRRN, "%.*s", LEN_RRN, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szRRN);
					continue;
				case 38:
					memset(glRecvPack.szAuthCode, '\0', strlen(glRecvPack.szAuthCode));
					sprintf((char *)glRecvPack.szAuthCode, "%.*s", LEN_AUTH_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szAuthCode);
					continue;
				case 39:
					memset(glRecvPack.szRspCode, '\0', strlen(glRecvPack.szRspCode));
					sprintf((char *)glRecvPack.szRspCode, "%.*s", LEN_RSP_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szRspCode);
					continue;
				case 40:
					memset(glRecvPack.szServResCode, '\0', strlen(glRecvPack.szServResCode));
					sprintf((char *)glRecvPack.szServResCode, "%.*s", LEN_SRES_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szServResCode);
					continue;
				case 41:
					memset(glRecvPack.szTermID, '\0', strlen(glRecvPack.szTermID));
					sprintf((char *)glRecvPack.szTermID, "%.*s", LEN_TERM_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTermID);
					continue;
				case 42:
					memset(glRecvPack.szMerchantID, '\0', strlen(glRecvPack.szMerchantID));
					sprintf((char *)glRecvPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMerchantID);
					continue;
				case 43:
					memset(glRecvPack.szMNL, '\0', strlen(glRecvPack.szMNL));
					sprintf((char *)glRecvPack.szMNL, "%.*s", LEN_MNL, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szMNL);
					continue;
				case 49:
					memset(glRecvPack.szTranCurcyCode, '\0', strlen(glRecvPack.szTranCurcyCode));
					sprintf((char *)glRecvPack.szTranCurcyCode, "%.*s", LEN_CURCY_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTranCurcyCode);
					continue;
				case 54:
					memset(glRecvPack.szAddtAmount, '\0', strlen(glRecvPack.szAddtAmount));
					sprintf((char *)glRecvPack.szAddtAmount, "%.*s", LEN_ADDT_AMT, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szAddtAmount);
					continue;
				case 55:
					memset(glRecvPack.sICCData, '\0', strlen(glRecvPack.sICCData));
					sprintf((char *)glRecvPack.sICCData, "%.*s", LEN_ICC_DATA, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.sICCData);
					continue;
				case 59:
					memset(glRecvPack.szTEchoData, '\0', strlen(glRecvPack.szTEchoData));
					sprintf((char *)glRecvPack.szTEchoData, "%.*s", LEN_TRANSECHO_DATA, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szTEchoData);
					continue;
				case 60:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				/*case 102:
					memset(glRecvPack.szActIdent1, '\0', strlen(glRecvPack.szActIdent1));
					sprintf((char *)glRecvPack.szActIdent1, "%.*s", LEN_ACT_IDTF, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent1);
					continue;
				case 103:
					memset(glRecvPack.szActIdent2, '\0', strlen(glRecvPack.szActIdent2));
					sprintf((char *)glRecvPack.szActIdent2, "%.*s", LEN_ACT_IDTF, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szActIdent2);
					continue;*/
				case 123:
					memset(glRecvPack.szPosDataCode, '\0', strlen(glRecvPack.szPosDataCode));
					sprintf((char *)glRecvPack.szPosDataCode, "%.*s", LEN_POS_CODE, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPosDataCode);
					continue;
				case 124:
					memset(glRecvPack.szPayMentInfo, '\0', strlen(glRecvPack.szPayMentInfo));
					sprintf((char *)glRecvPack.szPayMentInfo, "%.*s", LEN_PAYMENT_INFO, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szPayMentInfo);
					continue;
				/*case 128:
					memset(glRecvPack.szNFC, '\0', strlen(glRecvPack.szNFC));
					sprintf((char *)glRecvPack.szNFC, "%.*s", LEN_NEARFIELD, dataStore);
					ShowLogs(1, "Field %d: %s", i, glRecvPack.szNFC);
					continue;*/
				default:
					continue;
			}
		}
	}
	ShowLogs(1, "Reversal Response Code: %s", glRecvPack.szRspCode);
	if(strstr(glRecvPack.szRspCode, "00") != NULL)
	{
		DisplayInfoNone("", "Successful", 3);
	}else if(strstr(glRecvPack.szRspCode, "25") != NULL)
	{
		DisplayInfoNone("", "Successful", 3);
	}else
	{
		DisplayInfoNone("", "Not Successful", 3);
	}
	if((txnType != 5) && (iLen != 36) && (strlen(glRecvPack.szRspCode) == 2))
    {
    	//storeTxn();
		//storeEod();
		//storeEodDuplicate();

		ShowLogs(1, "About Storing Transaction.");
    	storeTxn();
    	ShowLogs(1, "About Storing Eod.");
		storeEod();
		ShowLogs(1, "About printing.");
		storeprint();
    }

	sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
	//UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);
	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
	sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);
	FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
	strcpy(glProcInfo.stTranLog.szPan, glSendPack.szPan);
	sprintf((char *)glProcInfo.stTranLog.szExpDate, "%.4s", glSendPack.szExpDate);
	strcpy(glProcInfo.stTranLog.szAmount, glSendPack.szTranAmt);
	
	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);
	return 0;
}