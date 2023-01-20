/*
 *  SSL API 
 *  Author: jhsun
 *  Date: 2007-6-20
 *  Version: 2.0
 *  History:
 *  07-06-20 v1.0 jhsun create the file 
 *  08-04-29 v2.0 jhsun 
 *     1.Added support for certificates
 *     2.Added interface for reading system time
 *     3.Added interface for random number
 *  090309 sunJH
 *      1. SslGetVer interface for using to get version information
 *      2. Cancel compatible 1.0 version API
 *  091022 sunJH
 *     1. Added self-signed error code
 *  091221 sunJH
 *     1. Added SslDecodePem, SslDecodeCertPubkey, SslDecodePrivKey interface��
 *  161103 lvjianyi
 *     1. Added SslCheckServerSubjectCN
 *  170522 qiaojx
 *     1. Added error code and the reason of invalid certificate
*/
#ifndef _SSL_API_H_
#define _SSL_API_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define NET_NONBLOCK 1
//#define NET_NONBLOCK
#define RET_SSL_CONNECTING	1
#define RET_SSL_OK			0

/*
 * SSL Error code
 */
 
#define ERR_AGAIN     2/* retry again */ 
#define ERR_SSL_BASE  100
#define ERR_SSL_PKT       (3+ERR_SSL_BASE)/* packet is too small or tooo big*/
#define ERR_SSL_CT        (4+ERR_SSL_BASE)/* the content type is error */
#define ERR_SSL_HT        (5+ERR_SSL_BASE)/* the handshake is error */
#define ERR_SSL_VER       (6+ERR_SSL_BASE)/* the version is error */
#define ERR_SSL_MEM       (7+ERR_SSL_BASE)/* NO MEM */
#define ERR_SSL_NET       (8+ERR_SSL_BASE)/* net error */
#define ERR_SSL_HANDLE    (9+ERR_SSL_BASE)/* the handle does not exist */
#define ERR_SSL_TIMEOUT   (10+ERR_SSL_BASE)/* time out */
#define ERR_SSL_CONNECTED (11+ERR_SSL_BASE)/* connected already */
#define ERR_SSL_CONNECT   (12+ERR_SSL_BASE)/* not connect */
#define ERR_SSL_CLOSED    (13+ERR_SSL_BASE)/* closed */
#define ERR_SSL_MD        (14+ERR_SSL_BASE)/* message digest fail */
#define ERR_SSL_OPS       (15+ERR_SSL_BASE)/* no net operations */
#define ERR_SSL_DEC       (16+ERR_SSL_BASE)/* decrypt fail */
#define ERR_SSL_CERT      (17+ERR_SSL_BASE)/* bad cert */
#define ERR_SSL_MAC       (18+ERR_SSL_BASE)/* mac function not support now! */
#define ERR_SSL_PKEY      (19+ERR_SSL_BASE)/* we only support RSA */
#define ERR_SSL_TIME      (20+ERR_SSL_BASE)/* cert validity */
#define ERR_SSL_CERTREQ   (21+ERR_SSL_BASE)/* cert req */
#define ERR_SSL_CERTNUM   (22+ERR_SSL_BASE)/* Cert Num is too big */
#define ERR_SSL_NOPRIVKEY (23+ERR_SSL_BASE)/* No  Private Key*/
#define ERR_SSL_CRL       (24+ERR_SSL_BASE)/* Cert Revoked List*/      
#define ERR_SSL_SIGN      (25+ERR_SSL_BASE)/* BAD SIGN */  
#define ERR_SSL_ISSUER      (26+ERR_SSL_BASE)/* no cert for the issuer */  
#define ERR_SSL_APP_CANCEL (27+ERR_SSL_BASE) /* User canceled because certificate cannot be verified*/
#define ERR_SSL_BADPRIVKEY (28+ERR_SSL_BASE)/* bad private key */
#define ERR_SSL_INITCERT   (29+ERR_SSL_BASE)/* Init CertInfo fail */
#define ERR_SSL_LIBVER     (30+ERR_SSL_BASE)/* Library version numbers are inconsistent */
#define ERR_SSL_SELF_SIGN  (31+ERR_SSL_BASE)/* Self-signed certificate is forbidden */
#define ERR_SSL_ILLEGAL_PARA  (32+ERR_SSL_BASE)/* Parameter of receive package is illegal */
#define ERR_SSL_NO_CLIENT_CERT (33+ERR_SSL_BASE) /* Server requested the certificate, but there is no local certificate */
#define ERR_SSL_VERIFY (34+ERR_SSL_BASE) /* Server verifies the client ��VERIFY�� information error */
#define ERR_SSL_FUN_PARA (35+ERR_SSL_BASE)/* Invalid parameter */
#define ERR_SSL_NO_CMD (36+ERR_SSL_BASE)/* No corresponding command when setting the parameters */
#define ERR_SSL_NO_SERVER_CERT (40+ERR_SSL_BASE)/* Unable to get server certificate */
#define ERR_SSL_NO_HOSTNAME (41+ERR_SSL_BASE)/* Unable to get Host name from certificate */
#define ERR_SSL_CHECK_HOSTNAME (42+ERR_SSL_BASE)/* Bad Host name */
#define ERR_SSL_CA_ISSUER (43+ERR_SSL_BASE)/* the issuer of CA could not be found */
#define ERR_SSL_NA        (100+ERR_SSL_BASE)/* unknown error */

/*
 * SslCreate:
 *    Create SSL Context
 * RETURN:
 *    if succeed, return ssl handle(>=0);
 *    if fail, return error code(<0)
 * ERRORS:
 *    SslCreate call fail, if
 *    [ERR_SSL_MEM]    Insufficient buffer space is available.The SSL Context
 *                              cannot be created until sufficient resources are freed.
 *    [ERR_SSL_OPS]     Before use ssl, you must call SslSetNetOps to set operations.
 */
int  SslCreate(void);

#ifndef NET_NONBLOCK
/*
 * SslConnect:
 *    Connect SSL(HTTPS) Server
 * PARAM:
 *    s     SSL Handle
 *    remote_addr     ssl server ip addr, such as "219.142.89.66"
 *    remote_port     ssl server port(host byte order), default is 443
 *    local_port        local host port; if it is 0, SSL will select a port >1024;
 *      flag                NOT USED now, must set be 0
 * RETURN:
 *    if succeed, return 0;otherwise, return erro code(<0)
 * ERRORS:
 *    SslConnect call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_PKT]          receive illegal packet which is either too small or too big
 *    [ERR_SSL_CT]            receive illegal packet the content type is error
 *    [ERR_SSL_HT]            handshake procotol is error
 *    [ERR_SSL_VER]          SSL Server Version is too old, Only Support SSLv3.0,TLSv1.0
 *    [ERR_SSL_MEM]          Insufficient buffer space is available
 *    [ERR_SSL_CONNECTED] connected already
 */
int SslConnect(int s, char *remote_addr, short remote_port, short local_port, long flag);
#else
/*
 * SslConnect:
 *    Connect SSL(HTTPS) Server
 * PARAM:
 *    s     SSL Handle
 *    remote_addr     ssl server ip addr, such as "219.142.89.66"
 *    remote_port     ssl server port(host byte order), default is 443
 *    local_port        local host port; if it is 0, SSL will select a port >1024;
 *      flag                NOT USED now, must set be 0
 * RETURN:
 *    if succeed, return 0;if still doing ssl connect,return 1;otherwise, return erro code(<0)
 * ERRORS:
 *    SslConnect call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_PKT]          receive illegal packet which is either too small or too big
 *    [ERR_SSL_CT]            receive illegal packet the content type is error
 *    [ERR_SSL_HT]            handshake procotol is error
 *    [ERR_SSL_VER]          SSL Server Version is too old, Only Support SSLv3.0,TLSv1.0
 *    [ERR_SSL_MEM]          Insufficient buffer space is available
 *    [ERR_SSL_CONNECTED] connected already
 */
int SslConnect(int s, char *remote_addr, ushort remote_port, ushort local_port, long flag);

/*
 * SslProcess:
 *    Implement SSL Connection 
 * PARAM:
 *    s     SSL Handle
 * RETURN:
 *    if succeed, return 0;if still doing ssl connect,return 1; otherwise, return erro code(<0)
 * ERRORS:
 *    SslConnect call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_PKT]          receive illegal packet which is either too small or too big
 *    [ERR_SSL_CT]            receive illegal packet the content type is error
 *    [ERR_SSL_HT]            handshake procotol is error
 *    [ERR_SSL_VER]          SSL Server Version is too old, Only Support SSLv3.0,TLSv1.0
 *    [ERR_SSL_MEM]          Insufficient buffer space is available
 *    [ERR_SSL_CONNECTED] connected already
 */
int SslProcess(int s);
#endif


/*
 * SslSend
 *    send data to remote host
 * PARAM:
 *    s      SSL Handle
 *    buf   pointer to input data  
 *    size  the size of input data in byte, MUST be <= 4000
 * RETURN:
 *    The call returns the number of characters sent, or <0 if an error occurred
 * ERRORS:
 *    SslSend call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_CONNECT]  before use SslSend, you must call SslConnect
 *    [ERR_SSL_CLOSED]     ssl is closed
 *    [ERR_SSL_MEM]          size > 4000
 */
int SslSend(int s, void *buf, int size);

/*
 * SslSend
 *   receive data from remote host
 * PARAM:
 *    s      SSL Handle
 *    buf   pointer to output data  
 *    size  the size of buffer, in byte
 * RETURN:
 *    The call returns the number of characters received, or <0 if an error occurred
 * ERRORS:
 *    SslRecv call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_CONNECT]  before use SslRecv, you must call SslConnect
 *    [ERR_SSL_CLOSED]     ssl is closed
 *    [ERR_SSL_MD]            message digest fail
 */
int SslRecv(int s, void *buf, int size);

/*
 * SslClose
 *   close ssl connect
 * PARAM:
 *    s      SSL Handle
 * RETURN:
 *    The call returns 0, or <0 if an error occurred
 * ERRORS:
 *    SslClose call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 */
int SslClose(int s);

/*
 * The Network Operate Sets
 */
typedef struct ssl_net_ops_s
{
	int (*net_open)(char *remote_addr, unsigned short remote_port, unsigned short local_port, long flag);
	int (*net_send)(int net_hd, void *buf, int size);
	int (*net_recv)(int net_hd, void *buf, int size);
	int (*net_close)(int net_hd);
}SSL_NET_OPS;

/*
 * SslSetNetOps
 *    tell ssl how to operate TCP/IP network(open, send, rcv and close), 
 * Param:
 *    ops      operate set, such as net_open, net_send, net_recv and net_close etc.
 *                 net_open      open a net connection from local_port to remote_addr:remote_port,
 *                                     if succeed, return net handle;otherwise, return < 0
 *                 net_send      send data in buf to remote host, 
 *                                     returns the number of characters sent, or <0 if an error occurred
 *                 net_recv       receive data from remote host,
 *                                     returns the number of characters received, or <0 if an error occurred
 */
void SslSetNetOps(SSL_NET_OPS *ops);

/*
** Time zone format
** ex1: Time zone is -8:30,then hour = -8, min = 30
** ex2: Time zone is +9:00,then hour = -9, min = 0
**/
typedef struct time_zone
{
	short hour;/* <0 using -, >0 using + */
	short min;
}TIME_ZONE;

/*
** System time format
**/
typedef struct system_time
{
	short year;/* 2000~2050 */
	short month;/* 1~12 */ 
	short day;/* 1~31 */
	short hour;/* 0~23 */
	short min;/* 0~59 */
	short sec;/* 0~59 */
	TIME_ZONE zone;/* Time zone */
}SYSTEM_TIME_T;

/*
** The reason of invalid certificate
**/
typedef enum 
{
	CERT_BAD = 1,  /* Certificate format that can��t be identified*/
	CERT_TIME,     /* The certificateis expired */
	CERT_CRL,      /* The certificatehas been revoked */
	CERT_SIGN,     /* The certificate signingis invalid */
	CERT_CA_TIME,  /* CA certificateis expired */
	CERT_CA_CRL,   /* CA certificatehas been revoked */
	CERT_CA_SIGN,  /* CA certificate signingis invalid */
	CERT_MAC,      /* System does not support the MAC algorithm of signing */
	CERT_MEM,      /* System is out of memory */
	CERT_ISSUER,   /* Cannot find the certificate issuer */
	CERT_SELF_SIGN,/* The certificateis is self-signed */
    CERT_CA_ISSUER,/* the issuer of CA could not be found */
	CERT_NA=100,   /* Unknown reason */
}CERT_INVAL_CODE;

/*
**A set of operations related to the system
**/
typedef struct ssl_sys_ops_s
{
	/* ReadSysTime: An interface for reading system time,
	**   This is used to verify the validity of the certificate;
	**   Note: If the interface is NULL,
	**   then SSL will ignore the validity of the certificate time;
	**   t: Save the acquired time,
	**   Return: succeed return 0, failed to return <0
	**/
	int (*ReadSysTime)(SYSTEM_TIME_T *t/* OUT */);
	/* Random: An interface for generating random number,
	**   The results required are in line with the PCI standard;
	**   Note: If the interface is NULL,
	**   SSL will automatically generate random numbers, but can not meet PCI standards;
	**   buf: Save the value of the random number
	**   len: buf the size of the space
	**  Return: succeed return 0, failed to return <0
	**/
	int (*Random)(unsigned char *buf/* OUT */, int len);
	/* Time: Get the time in seconds,
	** Note: If the interface is NULL,
	** then the SSL acquisition time value is a fixed,
	** This will reduce the security of SSL;
	** Equivalent to system calls time(NULL)*/
	unsigned long (*Time)(unsigned long *);
	/* ServCertAck: The subsequent processing of the validity of the server certificate can not be verified
	**    reason: The reason of invalid certificate
	**    Return: 0 means to ignore the certificate is valid, continue to connect to the server;
	**            <0 means disconnect
	**/
	int (*ServCertAck)(CERT_INVAL_CODE reason);
}SSL_SYS_OPS;

/*
** Set up system-related operations
**  Read_SysTime: Read the system time, succeed return 0, failed to return <0
**  Random:Generate random numbers, succeed return 0, failed to return <0
**  Time:time in seconds,like time(NULL) API
**/
void SslSetSysOps(SSL_SYS_OPS *ops);

typedef struct ssl_buf_s
{
	void *ptr;
	int  size;
}SSL_BUF_T;

/*
** SslCertsSet: Set up information related to the certificate
**    s: Socket handle,
**    cert_chains:A list of trusted certificates,
**            Used to verify the server-side certificate;
**    cert_chains_num: Number of certificates;
**    crl:Revoke the list
**    local_certs: Certificate of local use,
**            The first must be a local certificate for the terminal,
**            Other for CA certificate (used to validate local certificate);
**    local_privatekey:The private key corresponding to the locally used certificate
** Note:
**    1.Certificate, crl, and private key This information is stored in Binary format
**       If other format, first converted
**    2.Use order: SslCertsSet  ---> SslConnect 
**      
**/
int SslCertsSet(int s, SSL_BUF_T *cert_chains, int cert_chains_num,
		SSL_BUF_T *crl, 
		SSL_BUF_T *local_certs, int local_certs_num,
		SSL_BUF_T *local_privatekey);

void SslGetVer(char str[10]);

// Add only when added at the end to prevent the impact of previous commands
enum
{
	SSL_CMD_SET_REQ_CERT  = 1,
	SSL_CMD_SET_TIME_ZONE,
	SSL_CMD_SET_KEY_INDEX,
	SSL_CMD_SET_TLSEXT_SNI,
	SSL_CMD_SET_TLSEXT_NOSIGALG,
	SSL_CMD_SET_CERT_NOSELFSIG,
	SSL_CMD_SET_NO_SESS_ID,
};

int SslParaCtl(int s, int cmd, char *arg, int len);

/*
int SslDecodePem(char *BufIn, int BufInSize, char *BufOut, int BufOutSize)
Param:
BufIn   PEM Buffer Pointer
BufInSize  the size of BufIn
BufOut         BIN Buffer Pointer
BufOutSize  the size of BufOut
Return:
>0       succeed, No error
<0       Fail, Please see error code
Note:
The size of BufOut must be bigger than the size of BufIn
*/
int SslDecodePem(char *BufIn, int BufInSize, char *BufOut, int BufOutSize);

/*
SslDecodeCertPubkey: Extract the RSA Public Key from the certificate 
*/
int SslDecodeCertPubkey(SSL_BUF_T *CertBuf, SSL_BUF_T *Module, SSL_BUF_T *Exp);

/*
SslDecodePrivKey
*/
int SslDecodePrivKey(SSL_BUF_T *PrivKey, SSL_BUF_T *Module, SSL_BUF_T *Exp);

/*
SslVerifyCert:Verify the validity of the certificate
cert: Waiting for a certificate to be verified
cert_chains: Certificate chain
*/
int SslVerifyCert(SSL_BUF_T *cert, 
			SSL_BUF_T *cert_chains, 
			int cert_chains_num);

/*
checkServerCN: The server certificate checks the user domain name
s: Socket handle
url: Verified URL
*/
int SslCheckServerSubjectCN(int s, char *url);

#ifdef  __cplusplus
}
#endif

#endif/* _SSL_API_H_ */
