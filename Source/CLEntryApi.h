/*****************************************************/
/* ClEntryApi.h                                      */
/* Define the Application Program Interface          */
/* of Entry Point for all PAX Readers                */
/* Created by Liuxl at July 30, 2009                 */
/*****************************************************/

#ifndef _CLSS_ENTRY_LIB_H
#define _CLSS_ENTRY_LIB_H

#include "posapi.h"

#include "CL_common.h"


// 20090721
int  Clss_ReadVerInfo_Entry(char *paucVer);
int Clss_CoreInit_Entry(void);

//for monitor platform only
void Clss_GetICCStatus_Entry(unsigned char *SWA, unsigned char *SWB);
void Clss_SetDebug_Entry(int EnableFlag);

int Clss_SetPreProcInfo_Entry(Clss_PreProcInfo *ptPreProcInfoIn);
int Clss_PreTransProc_Entry(Clss_TransParam *pstTransParam);
int Clss_AppSlt_Entry(int Slot, int ReadLogFlag);
int Clss_FinalSelect_Entry(uchar *pucKernType, uchar *pucDtOut, int *pnDtLen);
int Clss_GetPreProcInterFlg_Entry(Clss_PreProcInterInfo *ptInfo);
//add for paypass 3.0.1 [1/22/2013 ZhouJie]
int Clss_GetErrorCode_Entry(int *pnErrorCode);
int Clss_SetMCVersion_Entry(uchar ucVer);
//add for PayWave
int Clss_SetExtendFunction_Entry(uchar *paucExFunc);

// 20090721 
void Clss_DelAllAidList_Entry(void);
int Clss_DelAidList_Entry(uchar  *pucAID, uchar  ucAidLen);
int Clss_AddAidList_Entry(uchar *pucAID, uchar ucAidLen, uchar ucSelFlg, uchar ucKernType);
int Clss_DelCurCandApp_Entry(void);

int Clss_GetFinalSelectData_Entry(uchar *paucDtOut, int *pnDtLen);

// 20090902
void Clss_DelAllPreProcInfo(void);
int Clss_DelPreProcInfo_Entry(uchar  *pucAID, uchar  ucAidLen);

unsigned char cPiccIsoCommand_Entry(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv);

//wanmin add on 2012.3.27
int clss_AppSelect_Entry_UnlockApp(Clss_TransParam *ptTransParam, ClssTmAidList *ptTermAid);
#endif
