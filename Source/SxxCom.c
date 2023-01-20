
#include "global.h"
#include "SxxCom.h"

/********************** Internal macros declaration ************************/
#define TCPMAXSENDLEN		10240

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static uchar SocketCheck(int sk);

/********************** Internal variables declaration *********************/
//static int sg_iSocket = -1;
int sg_iSocket = -1;

/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// Sxx TCP connection
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpConnect(const char *pszIP, ushort sPort, uchar bSSL, uchar ucDialMode, int iTimeoutSec)
{
	int		iRet = 0;
	struct net_sockaddr stServer_addr;
	uchar szCA[120] = {0};
	uchar szCert[120] = {0};
	uchar szKey[120] = {0};
	//Recompile
	//ShowLogs(1, "Inside TCP CONNECT");

	ShowLogs(1, "xxx TRACKING IP: %s", pszIP);
	ShowLogs(1, "xxx TRACKING PORT: %d", sPort);

	//Modified by Kevin_Wu, for support SSL noneblock mode 20160913
	//PreDial Mode
	if(ucDialMode == DM_PREDIAL)
	{
		ShowLogs(1, "Inside ucDialMode == DM_PREDIAL");
        if(bSSL)
		{
			if(GetEnv("CA_CRT", szCA) != 0)
	        {
	            return ERR_COMM_INV_PARAM;
	        }
	        if(GetEnv("CLI_CRT", szCert) != 0)
	        {
	            return ERR_COMM_INV_PARAM;
	        }

	        if(GetEnv("CLI_KEY", szKey) != 0)
	        {
	            return ERR_COMM_INV_PARAM;
	        }
			SxxSSLObjInit(szCA, szCert, szKey);
			//iRet = SxxSSLConnect((char*)pszIP, sPort, iTimeoutSec);//By Wisdomm
			iRet = SxxSSLConnectWisdom((char*)pszIP, sPort, iTimeoutSec);
		}		
		return iRet;
	}

	//Normal Dial mode
	if(ucDialMode == DM_DIAL)
	{	
		ShowLogs(1, "Inside ucDialMode == DM_DIAL");
		//ShowLogs(1, "Inside Normal Dial Mode");
		if(bSSL)
		{	
			if(SxxSSLGetSocket() >= 0)
			{
				iRet = SxxSSLConnectCheck(iTimeoutSec);
			}
			else
			{
				if(GetEnv("CA_CRT", szCA) != 0)
				{
					return ERR_COMM_INV_PARAM;
				}

				if(GetEnv("CLI_CRT", szCert) != 0)
				{
					return ERR_COMM_INV_PARAM;
				}

				if(GetEnv("CLI_KEY", szKey) != 0)
				{
					return ERR_COMM_INV_PARAM;
				}
				
				SxxSSLObjInit(szCA, szCert, szKey);
				
				//iRet = SxxSSLConnect((char*)pszIP, sPort, iTimeoutSec);//By Wisdom
				iRet = SxxSSLConnectWisdom((char*)pszIP, sPort, iTimeoutSec);
				if (iRet < 0)
				{
					return iRet;
				}
				
				iRet = SxxSSLConnectCheck(iTimeoutSec);
			}
		}
		else
		{
			//ShowLogs(1, "Trying to setup socket");
			// Setup socket
			iRet = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
			ShowLogs(1, "Socket Setup Response: %d", iRet);
			if (iRet < 0) // Modified By Kim 2014-12-10
			{
				return iRet;	
			}
			sg_iSocket = iRet;
			// set connection timeout
			if (iTimeoutSec<3)
			{
				iTimeoutSec = 3;
			}
			
			//ShowLogs(1, "Trying set Netioctl");
			iRet = Netioctl(sg_iSocket, CMD_TO_SET, iTimeoutSec*1000);
			ShowLogs(1, "NetioCtl: %d", iRet);
			if (iRet<0) // Modified By Kim 2014-12-10
			{
				return iRet;
			}
			
			//ShowLogs(1, "Trying to bind Ip");
			// Bind IP
			iRet = SockAddrSet(&stServer_addr, (char *)pszIP, sPort);
			ShowLogs(1, "Binding Response: %d", iRet);
			if (iRet!=0)
			{
				NetCloseSocket(sg_iSocket);
				return iRet;
			}
			//ShowLogs(1, "Trying to connect to remote Ip");
			// Connect to remote IP
			iRet = NetConnect(sg_iSocket, &stServer_addr, sizeof(stServer_addr));
			ShowLogs(1, "Connecting to remote Ip Response: %d", iRet);
			if (iRet!=0)
			{
				NetCloseSocket(sg_iSocket);
				sg_iSocket = -1;
			}
		}
	}
	
	return iRet;
}

//Sxx TCP/IP send data
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpTxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	int iRet;
	int iSendLen;
	int iSumLen;
	
	//ShowLogs(1, "Data to be sent\n%s", psTxdData);

	if(bSSL)
	{
		SxxSSLTxd(psTxdData, uiDataLen, 30);
	}
	else
	{
		iRet = Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);
		//ShowLogs(1, "NetioClt Return: %d", iRet);

		if (iRet < 0)
		{
			return iRet;
		}

		ScrSetIcon(ICON_UP, OPENICON);

		iSumLen = 0;
		while(1)
		{
			if (uiDataLen > TCPMAXSENDLEN)
			{
				iSendLen = TCPMAXSENDLEN;
				uiDataLen = uiDataLen - TCPMAXSENDLEN;
			}
			else
			{
				iSendLen = uiDataLen;
			}
			
			iRet = NetSend(sg_iSocket, (uchar*)psTxdData+iSumLen, iSendLen, 0);
			
			if (iRet < 0)
			{
				return iRet;
			}
			if (iRet != iSendLen)
			{
				return -1;
			}
			iSumLen = iSumLen + iSendLen;
			if (iSendLen <= TCPMAXSENDLEN)
			{
				break;
			}	
		}
		ScrSetIcon(ICON_UP, CLOSEICON);
		ShowLogs(1, "Length of sent: %d", iRet);
	}
	return 0;
}

//Sxx TCP/IP receive
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen)
{
	int iRet, iTemp;
	int iCountTimeout = 0;

	//DelayMs(200);
	
	ShowLogs(1, "Inside Receive....");

	if(bSSL)
	{
		//DelayMs(200);
		ShowLogs(1, "SxxTcpRxd 1");
		iRet = SxxSSLRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		if (iRet < 0)
		{
			return iRet;
		}
	}
	else
	{
		ShowLogs(1, "SxxTcpRxd 2");
		iRet = 	Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);
		//iRet = 	Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*100);
		ShowLogs(1, "Receive NetioCtl: %d", iRet);
		if (iRet < 0)
		{
			return iRet;
		}

		ScrSetIcon(ICON_DOWN, OPENICON);
		//By Wisdom
		iTemp = 0;
		while(1)
		{
			DelayMs(200);
			//iRet = NetRecv(sg_iSocket, psRxdData+iTemp, 10*1000, 0);
			iRet = NetRecv(sg_iSocket, psRxdData+iTemp, uiExpLen, 0);
			if (iRet == -13) {
				iCountTimeout++;
				if (iCountTimeout > 2) {
					ShowLogs(1, "Socket Timeout");
					break;
				}
			} else if(iRet < 0) {
				ShowLogs(1, "Nothing Received Again From Socket");
				break;
			}
			if (iRet < 0)
			{	
				ShowLogs(1, "Nothing Received Again From Socket");
				break;
			}
			iTemp += iRet;
			if (iTemp >= uiExpLen)
			{
				ShowLogs(1, "Received is greater than Expected. Dont Stop Sha");
				//break;
			}
			//Comment back if you want user to cancel request
			/*
			if (kbhit()==0)
			{
				if (getkey()==KEYCANCEL) 
					break;
			}
			*/
		}
		//ShowLogs(1, "Receive NetRecv: %s", psRxdData);
		ShowLogs(1, "Receive NetRecv: %d. iTemp: %d. Length: %d", iRet, iTemp, strlen(psRxdData));
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
		
	}
	ScrSetIcon(ICON_DOWN, CLOSEICON);
	return 0;
}




// Sxx TCP/IP close socket
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpOnHook(uchar bSSL)
{
	int iRet;

	if(bSSL){
	    SxxSSLClose();
	    return 0;
	}
	
	iRet = NetCloseSocket(sg_iSocket);
	if (iRet < 0)
	{
		sg_iSocket = -1;
		return iRet;
	}
	sg_iSocket = -1;
	return 0;
}

uchar SocketCheck(int sk)
{
	int event;
	if(sk<0) return RET_TCPCLOSED;
	
	event = Netioctl(sk, CMD_EVENT_GET, 0);
	if(event<0)
	{
         NetCloseSocket(sk);
         return RET_TCPCLOSED;
	}	
	
	if(event&(SOCK_EVENT_CONN|SOCK_EVENT_WRITE|SOCK_EVENT_READ))
	{
         return RET_TCPOPENED;
	}
	else if(event&(SOCK_EVENT_ERROR))
	{
         NetCloseSocket(sk);
         return RET_TCPCLOSED;
	}

	return RET_TCPOPENING;
}

int SxxDhcpStart(uchar ucForceStart, uchar ucTimeOutSec)
{
	int	iRet;

	if (ucForceStart && (DhcpCheck()==0))
	{
		DhcpStop();
	}

	if (ucForceStart || (DhcpCheck()!=0))
	{
		iRet = DhcpStart();
		if (iRet < 0)
		{
			return iRet;
		}

		TimerSet(TIMER_TEMPORARY, (ushort)(ucTimeOutSec*10));
		while (TimerCheck(TIMER_TEMPORARY)!=0)
		{
			DelayMs(200);
			iRet = DhcpCheck();
			if (iRet==0)
			{
				return 0;
			}
		}

		return iRet;
	}

	return 0;
}

int SxxLANTcpDial(const TCPIP_PARA *pstTcpPara, uchar ucDialMode)
{
	int		iRet;
	uchar	ucRedoDhcp, ucSecondIP;
	uchar   szIpFromDNS[25 + 1] = {0};

	uchar bSSL = 0;
    uchar szSSL[120];

	if(0 == GetEnv("E_SSL", szSSL))
    {
        bSSL = atoi(szSSL);
    }
	
	//Modify by Kevin_Wu, for supporting SSL none block mode 20160913
	if(!(bSSL && (SxxSSLGetSocket() >=0)))
	{
		CommOnHook(FALSE);
	}

	if (pstTcpPara->ucDhcp)
	{
		iRet = SxxDhcpStart(FALSE, 30);
		if (iRet!=0)
		{	
			return iRet;
		}
	}

	ucRedoDhcp = FALSE;
	ucSecondIP = FALSE;

TAG_RETRY_IP:

	// Connect to remote IP
	if (ucSecondIP)
	{
	    if(!ChkIfValidIp(pstTcpPara->stHost2.szIP)){
            iRet = DnsResolve((char *)pstTcpPara->stHost2.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
            if(RET_OK == iRet)
			{
				iRet = SxxTcpConnect(szIpFromDNS, (ushort)atoi((char *)pstTcpPara->stHost2.szPort), bSSL, ucDialMode, 8);	
			}	
        }
        else
		{	
			iRet = SxxTcpConnect(pstTcpPara->stHost2.szIP, (ushort)atoi((char *)pstTcpPara->stHost2.szPort), bSSL, ucDialMode, 8);
		}
	}
	else
	{
	    if(!ChkIfValidIp(pstTcpPara->stHost1.szIP) > 0){
			iRet = DnsResolve((char *)pstTcpPara->stHost1.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
	        if(RET_OK == iRet)
			{
				iRet = SxxTcpConnect(szIpFromDNS, (ushort)atoi((char *)pstTcpPara->stHost1.szPort), bSSL, ucDialMode, 8);
			}
			
	    }
	    else
		{
			iRet = SxxTcpConnect(pstTcpPara->stHost1.szIP, (ushort)atoi((char *)pstTcpPara->stHost1.szPort), bSSL, ucDialMode, 8);
		}
	}
	
	//Modified by Kevin_Wu 20160913
	if (iRet < 0)
	{
		if (!ucSecondIP)
		{
			if (pstTcpPara->ucDhcp && !ucRedoDhcp)
			{
				// If fail, suspect the DHCP
				iRet = SxxDhcpStart(FALSE, 10);
				if (iRet!=0)
				{
					return iRet;
				}
				ucRedoDhcp = TRUE;
				goto TAG_RETRY_IP;
			}

			if ( ChkIfValidPort(pstTcpPara->stHost2.szPort) &&
			      ChkIfValidIp(pstTcpPara->stHost2.szIP) &&
				strcmp((char *)(pstTcpPara->stHost2.szIP), (char *)(pstTcpPara->stHost2.szIP)))
			{
				ucSecondIP = TRUE;
				goto TAG_RETRY_IP;
			}
		}
	}

	return iRet;
}

// initial the wireless module
int SXXWlInit(const WIRELESS_PARAM *pstWlPara)
{
	int iRet;

	WlSelSim(pstWlPara->ucUsingSlot);

	iRet = WlInit(pstWlPara->szSimPin);
	
	if(iRet == WL_RET_ERR_INIT_ONCE)
	{
		iRet = 0;
	}

	if(iRet < 0)
	{
		return iRet;
	}

	SXXWlDispSignal();
	
	iRet = WlPppCheck();
	if ((iRet == 0) || (iRet == WL_RET_ERR_DIALING) || (iRet == 1))	// ret = 1 means module busy
	{
		return 0;
	}

	iRet = WlPppLogin((uchar*)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, 0, 3600);
	if(iRet == NET_ERR_TIMEOUT) //Indicate none block mode
	{
		iRet = 0;
	}

	return iRet;
}

// check if PPP is linked, if not, build PPP link firstly, and then build TCP link,
// otherwise build TCP link directly
int SXXWlDial(const WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval, uchar ucDialMode)
{
	int		iRet;
	int		iRetryTime;
	uchar	ucSecondIP;
	uchar   szIpFromDNS[25] = {0};

	uchar bSSL = 0;
    uchar szSSL[120];

	if(0 == GetEnv("E_SSL", szSSL))
    {
    	ShowLogs(1, "SSL activated");
        bSSL = atoi(szSSL);
    }
	
	//Modify by Kevin_Wu, for supporting SSL none block mode 20160913
	if(!(bSSL && (SxxSSLGetSocket() >=0)))
	{
		ShowLogs(1, "Failed Ssl Get Socket");
		CommOnHook(FALSE);
	}

	ShowLogs(1, "Done with SSL Get Socket");

	SXXWlDispSignal();
	
	ShowLogs(1, "Done with Displaying Signal");

	if (iTimeOut < 1)
	{
		iTimeOut = 1;
	}

	// ********** Pre-dial **********
	if (ucDialMode == DM_PREDIAL)
	{
		ShowLogs(1, "It is PREDIAL");
		iRet = WlPppCheck();

		//If Connect success, establish SSL connect
		if(bSSL && (SxxSSLGetSocket() < 0) && iRet == 0)
		{
			goto TCPCONNECT;
		}
		
		if ((iRet==0) || (iRet==WL_RET_ERR_DIALING) || (iRet==1))	// ret=1 means module busy
		{
			return 0;
		}

		iRet = WlPppLogin((uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, 0, iAliveInterval);
		return 0;
	}

	// ********** Full-dial **********
	ShowLogs(1, "TIMEOUT: %d", iTimeOut);
	//iTimeOut = 10;
	// ********** Check PPP connection **********
	TimerSet(TIMER_TEMPORARY, (ushort)(iTimeOut*10));
	while (TimerCheck(TIMER_TEMPORARY)!=0)
	{
		iRet = WlPppCheck();
		//ShowLogs(1, "WlPppCheck = %d", iRet);
		if (iRet == 0)
		{
			ShowLogs(1, "GOING TO TCPCONNECT");
			goto TCPCONNECT;
		}else if(iRet < 0)
		{
			ShowLogs(1, "RECONNECTING");
			break;
		}
	}

	// ********** Take PPP dial action **********
	iRetryTime = 3;
	ShowLogs(1, "TAKE PPP DIAL ACTION");
	while(iRetryTime--)
	{
		ShowLogs(1, "APN: %s. UID: %s. PWD: %s", 
			(uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd);
		iRet = WlPppLogin((uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, iTimeOut*1000, iAliveInterval);
		ShowLogs(1, "WlPppLogin = %d", iRet);
		if (iRet != 0)
		{
			DelayMs(100);
			continue;
		}
		ShowLogs(1, "PppChecking....");
		iRet = WlPppCheck();
		ShowLogs(1, "WlPppCheck = %d", iRet);
		if (iRet == 0)
		{
			break;
		}
	}

	if (iRetryTime <= 0 && iRet != 0)
	{
		ShowLogs(1, "IMPROMPTU = %d", iRet);
		return iRet;
	}

	// ********** Connect IP **********
TCPCONNECT:	
	ShowLogs(1, "INSIDE TCP CONNECT 1");
	if(!bSSL)
	{
		ShowLogs(1, "INSIDE TCP CONNECT 2");
		iRet = SocketCheck(sg_iSocket);  //come from R&D, tom
		ShowLogs(1, "INSIDE TCP CONNECT X: %d", iRet);
	//	ScrPrint(0, 7, ASCII, "tang[SocketCheck(%i)]",iRet); DelayMs(1000);
		if (iRet == RET_TCPOPENED)
		{
			return 0;
		}
	}
	ShowLogs(1, "GOING DOWNSTAIRS");
	ucSecondIP = FALSE;

//Wisdom commented all these out
_RETRY_SECOND_IP:
	if (ucSecondIP)
	{
		ShowLogs(1, "_RETRY_SECOND_IP IF");
	    if(!ChkIfValidIp(pstWlPara->stHost2.szIP)){
            iRet = DnsResolve((char *)pstWlPara->stHost2.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
            if(RET_OK == iRet)
		    {
				iRet = SxxTcpConnect(szIpFromDNS, (ushort)atoi((char *)pstWlPara->stHost2.szPort), bSSL, ucDialMode, 8);	
		    }   
        }
        else
		{   
			iRet = SxxTcpConnect(pstWlPara->stHost2.szIP, (ushort)atoi((char *)pstWlPara->stHost2.szPort), bSSL, ucDialMode, 8);
		}
	}
	else
	{
		ShowLogs(1, "_RETRY_SECOND_IP ELSE");
	    if(!ChkIfValidIp(pstWlPara->stHost1.szIP)){
			ShowLogs(1, "_RETRY_SECOND_IP ELSE 1");
            iRet = DnsResolve((char *)pstWlPara->stHost1.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
			ShowLogs(1, "_RETRY_SECOND_IP ELSE: X %d", iRet);
            if(RET_OK == iRet)
			{
				iRet = SxxTcpConnect(szIpFromDNS, (ushort)atoi((char *)pstWlPara->stHost1.szPort), bSSL, ucDialMode, 8);
			} 
			ShowLogs(1, "_RETRY_SECOND_IP 2 SAID: %d", iRet);
        }
        else
		{
			ShowLogs(1, "_RETRY_SECOND_IP CAME HERE");
			iRet = SxxTcpConnect(pstWlPara->stHost1.szIP, (ushort)atoi((char *)pstWlPara->stHost1.szPort), bSSL, ucDialMode, 8);
			ShowLogs(1, "_RETRY_SECOND_IP 2 SAID 3: %d", iRet);
		}
	}

	if (iRet < 0)
	{
		ShowLogs(1, "CHECKER 2 SAID: %d FOR PORT: %s", iRet, pstWlPara->stHost2.szPort);
	    if ( ChkIfValidPort(pstWlPara->stHost2.szPort) &&
             ChkIfValidIp(pstWlPara->stHost2.szIP) &&
             strcmp((char *)(pstWlPara->stHost2.szIP), (char *)(pstWlPara->stHost2.szIP)))
        {
			ShowLogs(1, "CHECK LOVER: %d", iRet);
			ucSecondIP = TRUE;
			goto _RETRY_SECOND_IP;
		}
		ShowLogs(1, "CHECK LEAVING: %d", iRet);
		return iRet;
	}
	ShowLogs(1, "CHECK FINAL: %d", iRet);
	return 0;
}

// send data (wireless)
int SXXWlSend(const uchar *psTxdData, ushort usDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	return SxxTcpTxd(psTxdData, usDataLen, bSSL, uiTimeOutSec);
}

// receive data (wireless)
int SXXWlRecv(uchar *psRxdData, ushort usExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *pusOutLen)
{
	DelayMs(200);
	return SxxTcpRxd(psRxdData, usExpLen, uiTimeOutSec, bSSL, pusOutLen);
}

// close the TCP link
int SXXWlCloseTcp(uchar bSSL)
{
	return SxxTcpOnHook(bSSL);
}

// close the PPP link
void SXXWlClosePPP(void)
{
	WlPppLogout(); 
	return;
}

// display the wireless signal
void SXXWlDispSignal(void)
{
	uchar	ucRet, ucLevel;
	
	
	ucRet = WlGetSignal(&ucLevel);
	if( ucRet!=RET_OK )
	{
		ScrSetIcon(ICON_SIGNAL, CLOSEICON);
		return;
	}
	
	ScrSetIcon(ICON_SIGNAL, (uchar)(5-ucLevel));
}


