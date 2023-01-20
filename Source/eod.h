
#ifndef _EOD_H
#define _EOD_H


#ifdef __cplusplus
extern "C" {
#endif 

void storeEod();
int parseEod(char *rrn, char *authcode);
void storeEodDuplicate();
extern int compl;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _MLOGO_H

// end of file
