#ifndef _ISOKEYEXCHANGE_H
#define _ISOKEYEXCHANGE_H

extern int rvApr;
extern int PURCHASETYPE;
int GetMasterKey();
int GetSessionKey();
void injectPinKey();
int sendToSucabo(char *body, char *outlv);
void forceKeyExchange();
int GetPinKey();
int GetParaMeters();
int GetParaMetersTest(char *resCode);
int parseParameters(char *data);
int parseParametersOld(char *data);
int SendEmvData(char *icc, int *reversal);
int SendReversal();
int cashSend(char *body, char *check);
void cowryCash();
int SendManualReversal();
void customStorage(char *szLocalTime, char *reason);
extern int lastUsedHost;
extern int revSend;
extern int switchHostManual;
extern int titi;
extern int checkVerve;
//Monitor
extern DL_ISO8583_HANDLER isoHandler;
extern DL_ISO8583_MSG     isoMsg;
//extern DL_UINT8           packBuf[5 * 1024];
extern DL_UINT16          packedSize;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _GLOBAL_H