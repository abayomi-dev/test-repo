
/****************************************************************************
NAME
    SxxComSSL.c

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    Kevin_Wu    2016.08.26      - created
****************************************************************************/

#include "global.h"

#define DEF_TIME_OUT (20 * 1000)

#define PEM_FILE			"PAXCLIENT.PEM"	
#define MAX_PEM_ELN			2048*3
//------------------------- Static Variable Area -----------------------------

SSLCon sslCon = {-1, {0}, {0}, {0}};

SSL_BUF_T  gCaCertBuffT = {NULL, 0};
SSL_BUF_T  gPosCertBuffT = {NULL, 0};
SSL_BUF_T  gPosPriKeyBuffT = {NULL, 0};

int conTimeOut = 0;
int sndTimeOut = 0;
int rcvTimeOut = 0;

//By Wisdom Startt
#define NUM_CA_CERTS 1
#define ETH_DEV 0
#define GPRS_DEV 11
static unsigned char g_SimPin[30]="";
static SSL_BUF_T  g_CaCertBuffT[NUM_CA_CERTS];
static SSL_BUF_T  g_PosCertBuffT;
static SSL_BUF_T  g_PosPriKeyBuffT;
static int g_SslSock;
//By Wisdom End



static char Bcd2Byte(char bVal)
{
	return ((bVal >> 4) * 10) + (bVal & 0x0F);
}

static DWORD Bcd2Int(const char* pbBuf, char bLen)
{
	DWORD dwVal = 0;

	while(bLen--)
	{
		dwVal *= 100;
		dwVal += Bcd2Byte(*pbBuf++);
	}

	return dwVal;
}


static int net_connect(char *remote_addr, ushort remote_port, ushort local_port, long flag)
{
	int iSocket, iRet;
	int iError; 
	struct net_sockaddr addr;

	ShowLogs(1, "Inside Connect: Ip: %s", remote_addr);
	ShowLogs(1, "Inside Connect: Port: %d", remote_port);
	ShowLogs(1, "Inside Connect: Local Port: %d", local_port);

	//1. Create TCP Socket
	iSocket = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
	if (iSocket < 0)
	{
		ShowLogs(1, "TCP Socket Failed.");
		return iSocket;	// FAIL
	}else
	{
		ShowLogs(1, "TCP Socket Created.");
	}
	
	//3. Set timeout time ms
	//Netioctl(iSocket, CMD_TO_SET, (conTimeOut > 0) ? conTimeOut : DEF_TIME_OUT);
	iRet = Netioctl(iSocket, CMD_TO_SET, 60000);
	if(iRet < 0)
	{
		ShowLogs(1, "NetioCtl Failed.");
		NetCloseSocket(iSocket);
		return iError;
	}else
	{
		ShowLogs(1, "NetioCtl Passed.");
	}

	//2. Set IP address
	memset(&addr, 0, sizeof(addr));
	
	iError = SockAddrSet(&addr, remote_addr, remote_port);
	if (iError < 0)
	{
		ShowLogs(1, "TCP Socket Address Set Failed.");
		NetCloseSocket(iSocket);
		return iError;
	}else
	{
		ShowLogs(1, "TCP Socket Address Passed.");
	}

	//4. Connect
	iError = NetConnect(iSocket, &addr, sizeof(addr));
	if (iError < 0)
	{	
		ShowLogs(1, "Connection Unsuccessful. %d", iError);
		NetCloseSocket(iSocket);

		switch (iError)
		{
			case NET_ERR_TIMEOUT:
				return -ERR_SSL_TIMEOUT;
			case NET_ERR_RST:
				return -ERR_SSL_NET;
			default:
				return iError;
		}
	}else
	{
		ShowLogs(1, "Connection Successful. %d", iError);
	}
	ShowLogs(1, "Return Socket. %d", iSocket);
	return iSocket;
}

static int net_send(int net_hd, void *buf, int size)
{
	int iRet;
	
	Netioctl(net_hd, CMD_TO_SET, (sndTimeOut > 0) ? sndTimeOut : DEF_TIME_OUT);
	
	iRet = NetSend(net_hd, buf, size, 0);

	if (iRet < 0)
	{
		switch (iRet)
		{
			case NET_ERR_TIMEOUT:
				return -ERR_SSL_TIMEOUT;
			default:
				return iRet;
		}
	}

	return iRet;
}

static int net_recv(int net_hd, void *buf, int size)
{
	int iBytesReceived = 0;
	
	Netioctl(net_hd, CMD_TO_SET, (rcvTimeOut > 0) ? rcvTimeOut : DEF_TIME_OUT);
	
	iBytesReceived = NetRecv(net_hd, buf, size, 0);
	
	if (iBytesReceived < 0)
	{
		switch (iBytesReceived)
		{
			case NET_ERR_TIMEOUT:
				return -ERR_SSL_TIMEOUT;
			default:
				return iBytesReceived;
		}
	}

	return iBytesReceived;
}

static int net_close(int net_hd)
{	
	NetCloseSocket(net_hd);
	
	return 0;
}

static int ReadSysTime(SYSTEM_TIME_T *t)
{
	uchar abDateTime[7];

	GetTime(abDateTime);
	t->year = 2000 + (unsigned short)Bcd2Int(abDateTime, 1);
	t->month = (char)Bcd2Int(&abDateTime[1], 1);
	t->day = (char)Bcd2Int(&abDateTime[2], 1);
	t->hour = (char)Bcd2Int(&abDateTime[3], 1);
	t->min = (char)Bcd2Int(&abDateTime[4], 1);
	t->sec = (char)Bcd2Int(&abDateTime[5], 1);
	t->zone.hour = 8;
	t->zone.min = 0;
	
	return 0;
}

static int Random(unsigned char *buf, int len)
{
	char abRandom8[8];
	int wLength;
	int i = 0;

	while (len > 0)
	{
		PciGetRandom(abRandom8);
		
		wLength = len > 8 ? 8 : len;
		
		memcpy(buf, abRandom8, wLength);
		
		len -= wLength;
		buf += wLength;
	}
	
	return 0;
}

static unsigned long Time(unsigned long *t)
{
	char abTime[7];
	char bHour, bMinute, bSecs;
	DWORD dwCurrentTimeSecs;

	// Return elasped time in current day in seconds
	GetTime(abTime);
	
	bHour = (char)Bcd2Int(&abTime[3], 1);
	bMinute = (char)Bcd2Int(&abTime[4], 1);
	bSecs = (char)Bcd2Int(&abTime[5], 1);

	dwCurrentTimeSecs = (DWORD)(bSecs + 60 * (bMinute + 60 * bHour));

	return dwCurrentTimeSecs;
}

static char *ReasonStr(CERT_INVAL_CODE eReason)
{
	char *szDescr[] =
	{
		"CERT_BAD",
		"CERT_TIME",
		"CERT_CRL",
		"CERT_SIGN",
		"CERT_CA_TIME",
		"CERT_CA_CRL",
		"CERT_CA_SIGN",
		"CERT_MAC",
		"CERT_MEM",
		"CERT_ISSUER",
	};

	if ( eReason > 0 && eReason <= (sizeof(szDescr)/sizeof(char*)) )
	{
		return szDescr[eReason - 1];
	}

	return "N/A";
}

static int CertAck(CERT_INVAL_CODE eReason)
{
	int iRet;

	if (eReason == CERT_ISSUER)
	{
		iRet = 0;	//ignore, conintue;
	}
	else
	{
		iRet = -1;
	}

	ScrCls();
	ScrPrint(0, 0, 0, "Ssl Cer Err\n");
	ScrPrint(0, 1, 0, "%d %s", eReason, ReasonStr(eReason));
	
	return iRet;
}

SSL_NET_OPS ssl_ops =
{
	net_connect,
	net_send,
	net_recv,
	net_close
};

SSL_SYS_OPS sys_ops =
{
	ReadSysTime,
	Random,
	Time,
	CertAck
};

//Read file content to buf
int RdFile2Buf(char* fName, char* buf, long fLen)
{
	int iRet = -1;
	int fd = -1;
	
	if((fd = open(fName, O_RDWR)) < 0)
	{
		return iRet;
	}

	if((iRet = read(fd, buf, fLen)) < 0)
	{
		close(fd);
		return iRet;
	}
	
	close(fd);
	
	return iRet;
}

//Get pem content start offset and length
int GetPemFContInfo(char* orgStr, int* startOffset, int* contLen,
	char* preFix, char* tailFix)
{
	char* startPos = NULL;
	char* endPos = NULL;
	char* pos = NULL;
	
	if(orgStr == NULL || preFix == NULL || tailFix == NULL)
	{
		return -1;
	}
	
	if((pos = strstr(orgStr, preFix)) == NULL)
	{
		return -1;
	}

	startPos = pos + strlen(preFix);
	
	*startOffset = (startPos - orgStr);
	
	if((endPos = strstr(orgStr, tailFix)) == NULL)
	{
		return -1;
	}

	*contLen = endPos - startPos;
	
	return *contLen;
}

//Read pem file to buf
//1. Only support read out one cert from a pem file now(not support read out multipule cert 
//		from one pem file)
//2. *pemCont will dynamic alloc in this function, donot forget free it
int RdPemF2Buf(char* pemF, char** pemCont, int* contLen, 
	char* preFix, char* tailFix)
{
	int iRet = -1;
	long fSize = 0;
	char* pfBuf = NULL;
	int startOffset;
	int i = 0;

	if(pemF == NULL || pemCont == NULL || contLen == NULL 
		|| preFix == NULL || tailFix == NULL)
	{
		goto Exit;
	}
	
	if((fSize = filesize(pemF)) < 0)
	{
		goto Exit;
	}
	
	if((pfBuf = malloc(fSize + 1)) == NULL)
	{
		goto Exit;
	}
	
	memset(pfBuf, '\0', fSize + 1);
	
	//1. Read pem all file content to buff
	if((iRet = RdFile2Buf(pemF, pfBuf, fSize)) < 0)
	{
		goto Exit;
	}
	
	//2. Get pem valid content start offset and length
	if(GetPemFContInfo(pfBuf, &startOffset, contLen, preFix, tailFix) 
		< 0)
	{
		goto Exit;
	}

	//3. Copy valid content to buff
	if(pfBuf != NULL && startOffset >= 0 && (*contLen) >= 0)
	{
		*pemCont = malloc((*contLen) + 1);
		if(*pemCont == NULL)
		{
			goto Exit;
		}
		
		memset(*pemCont, 0, (*contLen) + 1);
		
		strncpy(*pemCont, (pfBuf + startOffset) , *contLen);
	}

Exit:	

	if(pfBuf != NULL)
	{
		free(pfBuf);
		pfBuf = NULL;
	}

	return iRet;
}

//Init sslCon Object
int SxxSSLObjInit(char *ca, char *cert, char *key)
{	
	if(ca == NULL || cert == NULL || key == NULL)
	{
		return ERR_COMM_INV_PARAM;
	}
	
	memset(&sslCon, 0, sizeof(sslCon));
	
	sslCon.sslSock = -1;

	strncpy(sslCon.szCA, ca, strlen(ca));
	strncpy(sslCon.szCert, cert, strlen(cert));
	strncpy(sslCon.szKey, key, strlen(key));
	
	return 0;
}

int SxxSSLGetSocket()
{
	ShowLogs(1, "SxxSSLGetSocket: %d", sslCon.sslSock);
	return sslCon.sslSock;
}


int ReadPemFile(uchar *pszAppFileName, uchar *psCA, uchar *psCert, uchar *psPrivatekey) {
	int fd, iRet, iTempLen;
	uchar *ptr1, *ptr2, *ptr3;
	uchar ucBuf[MAX_PEM_ELN + 1];
	uchar pos_ca_buf[2048];
	uchar pos_cer_buf[2048];
	uchar pos_privatekey_buf[2048];

	if (pszAppFileName == NULL) {
		return -1;
	}

	fd = open(pszAppFileName, O_RDWR);
	if (fd < 0) {
		ShowLogs(1, "Open of pem file %s failed. Are you sure it was loaded?", pszAppFileName);
		return fd;
	}

	memset(ucBuf, 0, sizeof(ucBuf));
	iRet = read(fd, ucBuf, MAX_PEM_ELN);
	if (iRet < 0) {
		close(fd);
		return iRet;
	}
	close(fd);

	//Get CERTIFICATE
	if ((ptr1 = strstr(ucBuf, "-----BEGIN CERTIFICATE-----"))
			&& (ptr2 = strstr(ucBuf, "-----END CERTIFICATE-----"))) {
		memset(pos_cer_buf, 0, sizeof(pos_cer_buf));
		iTempLen = strlen(ptr1) - strlen(ptr2) - strlen("-----BEGIN CERTIFICATE-----");
		memcpy(pos_cer_buf, ptr1 + strlen("-----BEGIN CERTIFICATE-----"),
				iTempLen);
	} else {
		return -2;
	}

	//Get PRIVATE KEY
	if ((ptr1 = strstr(ucBuf, "-----BEGIN RSA PRIVATE KEY-----")) && (ptr2 =
					strstr(ucBuf, "-----END RSA PRIVATE KEY-----"))) {
		memset(pos_privatekey_buf, 0, sizeof(pos_privatekey_buf));
		iTempLen = strlen(ptr1) - strlen(ptr2) - strlen("-----BEGIN RSA PRIVATE KEY-----");
		memcpy(pos_privatekey_buf, ptr1 + strlen("-----BEGIN RSA PRIVATE KEY-----"), iTempLen);
	} else {
		return -3;
	}

	//Get CA
	if ((ptr1 = strstr(ptr2, "-----BEGIN CERTIFICATE-----"))
			&& (ptr3 = strstr(ptr2, "-----END CERTIFICATE-----"))) {
		memset(pos_ca_buf, 0, sizeof(pos_ca_buf));
		iTempLen = strlen(ptr1) - strlen(ptr3) - strlen("-----BEGIN CERTIFICATE-----");
		memcpy(pos_ca_buf, ptr1 + strlen("-----BEGIN CERTIFICATE-----"), iTempLen);
	} else {
		return -4;
	}

	//out data
	memcpy(psCA, pos_ca_buf, strlen(pos_ca_buf));
	memcpy(psCert, pos_cer_buf, strlen(pos_cer_buf));
	memcpy(psPrivatekey, pos_privatekey_buf, strlen(pos_privatekey_buf));

	return 0;
}


int SxxSSLConnectWisdom(const char *pszIP, ushort sPort, int iTimeout)
{
	int iRet;
	
	char pos_ca_pem[2048] = {0};
	char pos_cert_pem[2048] = {0};
	char pos_privatekey_pem[2048] = {0};

	char pos_ca_buf[2048];
	char pos_cert_buf[2048];
	char pos_privatekey_buf[2048];

	SSL_BUF_T pos_ca;
	SSL_BUF_T pos_cert;
	SSL_BUF_T pos_privatekey;
	SSL_BUF_T pos_certs[2];

	ShowLogs(1, "xxxx WIsdom Ip: %s", pszIP);
	ShowLogs(1, "xxxx Wisdom Port: %d", sPort);

	SslSetNetOps(&ssl_ops);

	memset(pos_ca_buf, 0, sizeof(pos_ca_buf));
	memset(pos_cert_buf, 0, sizeof(pos_cert_buf));
	memset(pos_privatekey_buf, 0, sizeof(pos_privatekey_buf));


	iRet = ReadPemFile(PEM_FILE, pos_ca_pem, pos_cert_pem, pos_privatekey_pem);
	if (iRet != 0) {
		ShowLogs(1, "Could not read pem file %s", PEM_FILE);
		return -1;
	}

	memset(pos_ca_buf, 0, sizeof(pos_ca_buf));
	iRet = SslDecodePem(pos_ca_pem, strlen(pos_ca_pem), pos_ca_buf, sizeof(pos_ca_buf));
	if (iRet <= 0) {
		ShowLogs(1, "Could not decode CaPem");
		return -1;
	}
	pos_ca.size = iRet;
	pos_ca.ptr = pos_ca_buf;

	memset(pos_cert_buf, 0, sizeof(pos_cert_buf));
	iRet = SslDecodePem(pos_cert_pem, strlen(pos_cert_pem), pos_cert_buf, sizeof(pos_cert_buf));
	if (iRet <= 0) {
		ShowLogs(1, "Could not decode CertPem");
		return -1;
	}
	pos_cert.size = iRet;
	pos_cert.ptr = pos_cert_buf;

	memset(pos_privatekey_buf, 0, sizeof(pos_privatekey_buf));
	iRet = SslDecodePem(pos_privatekey_pem, strlen(pos_privatekey_pem), pos_privatekey_buf, sizeof(pos_privatekey_buf));
	if (iRet <= 0) {
		ShowLogs(1, "Could not decode Private Key");
		return -1;
	}
	pos_privatekey.size = iRet;
	pos_privatekey.ptr = pos_privatekey_buf;

	iRet = SslCreate();
	if (iRet < 0) {
		ShowLogs(1, "Ssl Create Return: %d", iRet);
		return -1;
	}
	
	sslCon.sslSock = iRet;
	ShowLogs(1, "LOVELY: %d", iRet);

	iRet = SslConnect(sslCon.sslSock, (char*)pszIP, sPort, 0, 0);
	ShowLogs(1, "LOVELY 2: %d", iRet);
	if (iRet < 0) {
		ShowLogs(1, "SslConnect Return: %d", iRet);
		return -1;
	}
	ShowLogs(1, "LOVELY 3: %d", iRet);
	return iRet;
}

int SxxSSLConnectCheck(int iTimeoutSec)
{
	int iRet = 0;
	return 0;
	/*TimerSet(TIMER_TEMPORARY, (ushort)(iTimeoutSec * 3 * 10));
	
	while(TimerCheck(TIMER_TEMPORARY) != 0)
	{
		iRet = SslProcess(sslCon.sslSock);
		if(iRet < 0)
		{
			break;
		}
		
		if(iRet == 0)
		{
			break;
		}
	}
	
	if(iRet == 1 && TimerCheck(TIMER_TEMPORARY) == 0)
	{
		iRet = NET_ERR_TIMEOUT;
	}
	
	return iRet;*/
}

int SxxSSLTxd(const uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec)
{
	int iRet;
	ScrSetIcon(ICON_UP, OPENICON);
	sndTimeOut = uiTimeOutSec * 1000;
	
	iRet = SslSend(sslCon.sslSock, (void *)psTxdData, uiDataLen);
	ShowLogs(1, "SslSend: Length of Data Sent: %d", iRet);
	if (iRet < 0)
	{
		SslClose(sslCon.sslSock);
		sslCon.sslSock = -1;
	}
	ScrSetIcon(ICON_UP, CLOSEICON);
	return iRet;
}

int SxxSSLRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	
	int iRet = 0;
	int iTemp = 0;
	int iCountTimeout = 0;
	
	ScrSetIcon(ICON_DOWN, OPENICON);
	rcvTimeOut = uiTimeOutSec * 1000;
	while(1)
	{
		DelayMs(200);

		iRet = SslRecv(sslCon.sslSock, psRxdData+iTemp, uiExpLen);
		/*if (iRet == -13) {
			iCountTimeout++;
			if (iCountTimeout > 2) {
				ShowLogs(1, "Socket Timeout");
				break;
			}
		} else if(iRet < 0) {
			ShowLogs(1, "Nothing Received Again From Socket");
			break;
		}*/
		
		if (iRet < 0)
		{	
			break;
		}
				
		iTemp += iRet;
		if (iTemp >= uiExpLen)
			break;
	}

	if(iTemp > 0)
	{
		*puiOutLen = iTemp;
		ScrSetIcon(ICON_DOWN, CLOSEICON);
		return 0;
	}
	
	if (iRet < 0)
	{
		ScrSetIcon(ICON_DOWN, CLOSEICON);
		return iRet;
	}
	ScrSetIcon(ICON_DOWN, CLOSEICON);
	return iRet;
}


int SxxSSLCloseCustom(void)
{
	int iRet;
	
	if(sslCon.sslSock >= 0)
	{
		iRet = SslClose(sslCon.sslSock);
		
		memset(&sslCon, 0, sizeof(sslCon));
		sslCon.sslSock = -1;
	}
	
	return iRet;
}

int SxxSSLClose(void)
{
	char temp[10] = {0};
	memset(temp, '\0', strlen(temp)); 
	UtilGetEnv("cotype", temp);
	if(strstr(temp, "GPRS") != NULL)
	{
		ShowLogs(1, "Taking this step for Gprs Sake");
		return SxxSSLCloseCustom();
	}else
	{
		ShowLogs(1, "Taking this step for Wifi Sake");
		return SxxSSLCloseCustom();	
	}


	int iRet;
	
	if(sslCon.sslSock >= 0)
	{
		iRet = SslClose(sslCon.sslSock);
		
		memset(&sslCon, 0, sizeof(sslCon));
		sslCon.sslSock = -1;
	}
	
	return iRet;
}

int SxxSSLSingleProcess()
{
	return 0;
	//return  SslProcess(sslCon.sslSock);
}


//Wisdom start here

void BuffDump(char *p, int size, char *tip)
{
	int i;
	
	ShowLogs(1, "%s", tip);
	for(i=0; i<size; i++)
	{
		ShowLogs(1, "%02x ", p[i]);
	}
	ShowLogs(1, "");
	
	return;
}

int PrintIp(int dev)
{
	int sl;
	char HostIp[20], mask[20], gateway[20], dns[20];

	memset(HostIp, 0, sizeof(HostIp));
	memset(mask, 0, sizeof(mask));
	memset(gateway, 0, sizeof(gateway));
	memset(dns, 0, sizeof(dns));

	sl=NetDevGet(dev, HostIp, mask, gateway, dns);
	if(sl!=0)
	{
		ShowLogs(1, "NetDevGet fail %d", sl);
		return -1;
	}

	ShowLogs(1, "HostIp %s mask %s gata %s dns %s \r\n", HostIp, mask, gateway, dns);

	return 0;
}

//Recompile

static int PppLoginTst(const char *APNName, 
		char *uid, char *pwd, long auth, 
		int timeout, int AliveInterval)
{
	int sl;

	sl=WlInit(g_SimPin);
	if(sl!=0 && sl != WL_RET_ERR_INIT_ONCE)
	{
		ShowLogs(1, "WlInit result:%d ",sl);
		return 1;
	}

	DelayMs(200);
	
	while(1)
	{
		sl=WlPppLogin(APNName, uid, pwd, auth, timeout, AliveInterval);
		ShowLogs(1, "PPP Login Ret %d ", sl);
		
		while(1)
		{
			sl=WlPppCheck();
			if(sl==1) {
				DelayMs(200);
				continue;
			} else {
				break;
			}
		}
		
		if(sl!=0) {
			ShowLogs(1, " Err Check PPP:%d ",sl);
			//DelayMs(5000);
			//continue;
			return 1;
		} else {
			break;
		}
	}
	
	return 0;
}
