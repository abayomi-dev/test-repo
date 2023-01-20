#ifndef _UTILITIES_H
#define _UTILITIES_H

void removeSubstr (char *string, char *sub);
void backupVerveWithVal();
int UtilGetEnv(char *key, char *value);
int UtilGetEnvEx(char *key, char *value);
void UtilPutEnv(char *key, char *value);
void TextCenter(char *text, int x, int y);
void TextLeftNormal(char *text, int x, int y);
void TextLeftSmall(char *text, int x, int y);
int DisplayMsg(char* heading, char * demand, char * def, char *input, int min, int max);
void DisplayInfo(char* heading, char *msg, int timeout);
void DisplayInfoCancel(char* heading, char *msg, int timeout);
void DisplayInfoOk(char* heading, char *msg, int timeout);
int DisplayInfoYN(char* heading, char *msg, int timeout);
void DisplayInfoNone(char* heading, char *msg, int timeout);
int StartGprs(char *apn, char *username, char *password);
void GetSignalStr();
int CreateWrite(char *filename, char *data);
int WriteUpdate(char *filename, char *data, long lOffset);
int ReadAllData(char *filename, char *output);
void SysGetTimeIso(char *time);
void SysSetTimeProfile(char *time);
void EmvSetSSLFlag();
void EmvUnsetSSLFlag();
void GetStan(void);
void GetReceiptNumber(void);
void getResponse(char *code, char *output);
void GetTxnCount(void);
extern ulong useStan;
extern ulong RctNum;
extern int txnCount;
extern int downloadBillers; //Used to download billers and display billers

extern int mallocCount;
extern int callocCount;
extern int mallocFreeCount;
void* sysalloc_calloc(int count, int eltsize);
void* sysalloc_malloc(int size);
void sysalloc_free(void* ptr);
int setVerveHost(char *host);
int acctTypeSelection();
//jamb
int jambTypeSelection();
void getJsonValue(char *input, char *name, char *value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _GLOBAL_H