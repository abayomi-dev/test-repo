
#ifndef _CALLHOME_H
#define _CALLHOME_H


#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _CellStationInformation CellStationInformation;
struct _CellStationInformation {
	int cellID;
	int locationAreaCode;
	int mobileCountryCode;
	int mobileNetworkCode;
	int signalStrength;
};

void PackCallHomeData();
void storeTxn();
int getTotalCount(char* txn);
extern int sendcount;
extern int curSending;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _MLOGO_H

// end of file
