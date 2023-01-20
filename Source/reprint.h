
#ifndef _REPRINT_H
#define _REPRINT_H


#ifdef __cplusplus
extern "C" {
#endif 

void storeReprint(char *store);
void storeReversal(char *store);
void printDetails();
void reprintLast();
//extern char details[20][128];
void reprintAny(char *number);
void reprintRrn(char *number);
extern int reprintNum;
void endofday();
void dataCollection();
void eodCollection();
extern double totalamt;
extern int transactionpassed;
extern int transactionfailed;
extern int transaction;
extern int checkSame;
void printTestConnection(char *txn, char *rsp);
int getTotalCountEod(char* txn);
void eodCashier();
struct PRINTDATA
{
  char field0[128];
  char field1[128];
  char field2[128];
  char field3[128];
  char field4[128];
  char field5[128];
  char field6[128];
  char field7[128];
  char field8[128];
  char field9[128];
  char field10[128];
  char field11[128];
  char field12[128];
  char field13[128];
  char field14[128];
  char field15[128];
  char field16[128];
  char field17[128];
  char field18[128];
  char field19[128];
};
void daterangeeodCollection();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _MLOGO_H

// end of file
