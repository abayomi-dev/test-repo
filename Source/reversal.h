
#ifndef _REVERSAL_H
#define _REVERSAL_H


#ifdef __cplusplus
extern "C" {
#endif 

int reversalTxn();
int parseInnerReversal(char *line, char *rrn, char *omti, char *odatetime, char *ocardno, char *oproc, 
				char *oamt, char *orrn, char *oseqnum, char *oentrymode, char *oposdatacode, char *ostan, char *acq);
void parseReversalAmount(char *mainData, char *fin);
void formatReversalAmount(char *initAmt, char *finalAmt);
int RefundEntrance();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _MLOGO_H

// end of file
