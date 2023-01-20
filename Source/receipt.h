#ifndef _RECEIPT_H
#define _RECEIPT_H


void PrintAllReceipt(uchar ucPrnFlag);
void PrintAllReceipt(uchar ucPrnFlag);
void reprintDataNormal();
void MaskAllPan(char *pan, char *mskPan);
void storeprint();
void Conv2EngTime2(const uchar *pszDateTime, uchar *pszEngTime);
void parseAmount(char* temp, char *store);
int testPrinter();
extern int ext;
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _GLOBAL_H