
/****************************************************************************
NAME
    st8583.h - ����ϵͳ8583�ṹ

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _ST8583_H
#define _ST8583_H

#define	LEN_MSG_CODE			4
#define	LEN_BITMAP				8
#define	LEN_PAN					19
#define	LEN_PROC_CODE			6
#define	LEN_TRAN_AMT			12
#define	LEN_FRN_AMT				12
#define	LEN_DCC_RATE			8
#define	LEN_STAN				6
#define LEN_LOCAL_DATE_TIME		10
#define	LEN_LOCAL_TIME			6
#define	LEN_LOCAL_DATE			4
#define	LEN_MERCHANT_TYPE		4
#define	LEN_EXP_DATE			4
#define	LEN_SETTLE_DATE			4
#define	LEN_ENTRY_MODE			4
#define	LEN_PAN_SEQ_NO			3
#define	LEN_NII					3
#define	LEN_COND_CODE			2
#define	LEN_TRACK2				37
#define	LEN_TRACK3				104
#define	LEN_RRN					12
#define	LEN_AUTH_CODE			6
//#define	LEN_AQUR_ID			    6//Commented out by me
#define	LEN_AQUR_ID			    11
#define	LEN_RSP_CODE			2
#define	LEN_TERM_ID				8
#define	LEN_TRANS_FEE			9
#define	LEN_MERCHANT_ID			15
#define	LEN_POS_CODE			15
#define	LEN_ADDL_RSP			2
#define	LEN_POSC_CODE			2
#define	LEN_TRACK1				76
#define	LEN_MNL					40
#define	LEN_FIELD48				100
#define	LEN_CURCY_CODE			3
#define	LEN_SRES_CODE			3
#define	LEN_PIN_DATA			8
#define	LEN_EXT_AMOUNT			12
#define	LEN_ADDT_AMT			120
#define	LEN_ICC_DATA			260
#define LEN_TRANSECHO_DATA		255
#define LEN_PAYMENT_INFO		999
#define LEN_ACT_IDTF			28
#define	LEN_ICC_DATA2			110
#define	LEN_FIELD60				22
#define	LEN_TMSFIELD60			600
#define	LEN_FIELD61				30
#define	LEN_INVOICE_NO			6
#define LEN_FIELD62             100
#define	LEN_FIELD63				800
#define LEN_MAC					8
#define LEN_HASH				64
#define LEN_NEARFIELD			9999
#define ORIG_DATA_ELEMENT		42
#define MES_REASON_CODE			4
#define BILLER_ISO				999
//ISO8583ʹ��˵��
//Step2: ����Step1�Ķ��壬����ṹ��ʹ�õ�����ʵ������ı�������Attr_UnUsed��Attr_Over�͵�����������
//ע����glEdcDataDef�ṹҪһ��Ҫһһ��Ӧ,�����������.
//ע������glEdcDataDef����ΪAttr_a, Attr_n, Attr_z��ʱ��Ϊsz��ͷ
//����glEdcDataDef����ΪAttr_b��ʱ��Ϊs��ͷ

// Usage of ISO8583 module (For step 1, see in st8583.c; For step 3, see in TranProc.c)
// Step 2
// According to the definition in step 1, defines member variable in below "STISO8583".
// Those of type "Attr_UnUsed" and "Attr_Over" needn't to define.
// NOTICE that it must be corresponding to the sequence in "glEdcDataDef", one by one.
// if in "glEdcDataDef", the bit attribute is "Attr_a", "Attr_n" or "Attr_z", the member name should be start with "sz"
// if in "glEdcDataDef", the bit attribute is "Attr_b", the member name should be start with "s"

// ˵����
// 1. ��Binary���͵��򣬽ṹ��Ա������ǰ��2���ֽ�Ϊ���ݳ���
//    ��ʽΪ����ЧΪ��ǰ�������ֽ�˳��
// 2. �Է�Binary������ֱ��ʹ��C���ַ�������/��ʽ���������и�ֵ����
//    �����2���ֽ���Ϊ�˴洢'\0'�ַ�(Ϊ���㴦��������һ���ַ�)
// 1. To those bit attribute "Attr_b" (member name: sxxxxxxx[]), reserves heading 2 bytes for storing length in hex.
//      The length format is like : "\x01\x2A" when length=0x012A
// 2. To those not "Attr_b", can use "sprintf" to fill data in ASCII.
//      The extra 2 bytes are to store the ending "\x00". (for abandon, use 2 bytes.)
typedef struct _tagSTISO8583
{
	uchar	szVendorId[20+2];						// Vendor Id for callhome
	uchar	extras[258];							// For Inputted Data
	uchar	szMsgCode[LEN_MSG_CODE+2];				// message code
	uchar	sBitMap[2*LEN_BITMAP];					// !!!! No leading 2 length bytes !!!!  ��Ҫ��2
	uchar	szPan[LEN_PAN+2];						// PAN
	uchar	szProcCode[LEN_PROC_CODE+2];			// proc code
	uchar	szTranAmt[LEN_TRAN_AMT+2];				// Txn Amount
	uchar	szFrnAmt[LEN_FRN_AMT+2];				// Foreign amt
	uchar	szDccRate[LEN_DCC_RATE+2];				// DCC Rate
	uchar	szSTAN[LEN_STAN+2];						// STAN
	uchar	szLocalDateTime[LEN_LOCAL_DATE_TIME+2];	// datetime, mmddhhmmss
	uchar	szLocalTime[LEN_LOCAL_TIME+2];			// time, hhmmss
	uchar	szLocalDate[LEN_LOCAL_DATE+2];			// date, YYMM
	uchar	szMerchantType[LEN_MERCHANT_TYPE+2];	// Mcc
	uchar	szExpDate[LEN_EXP_DATE+2];				// Expiry, YYMM
	uchar	szSetlDate[LEN_LOCAL_DATE+2];			// date, Settlement
	uchar	szEntryMode[LEN_ENTRY_MODE+2];			// entry mode
	uchar	szPanSeqNo[LEN_PAN_SEQ_NO+2];			// PAN seq #
	uchar	szNii[LEN_NII+2];						// NII
	uchar	szCondCode[LEN_COND_CODE+2];			// Cond. code
	uchar	szTrack2[LEN_TRACK2+2];					// track 2
	uchar	szTrack3[LEN_TRACK3+2];					// track 3
	uchar	szRRN[LEN_RRN+2];						// RRN
	uchar	szAuthCode[LEN_AUTH_CODE+2];			// auth code 
	uchar	szRspCode[LEN_RSP_CODE+2];				// rsp code
	uchar	szTermID[LEN_TERM_ID+2];				// terminal id
	uchar	szMerchantID[LEN_MERCHANT_ID+2];		// merchant id
	uchar	szPosDataCode[LEN_POS_CODE+2];			// Pos Data Code
	uchar	szAddlRsp[LEN_ADDL_RSP+2];				// add'l rsp
	uchar	szPoscCode[LEN_POSC_CODE+2];			// Pos Capture Code 
	uchar	szTransFee[LEN_TRANS_FEE+2];			// Transaction Fee 
	uchar	szAqurId[LEN_AQUR_ID+2];				//Acquirer Code
	uchar	szFwdInstId[LEN_AQUR_ID+2];				//Forwarding Institution Id Code
	uchar	szMNL[LEN_MNL+2];						//Merchant Name and Location
	uchar 	szServResCode[LEN_SRES_CODE+2];			//Service Restriction Code
	uchar	szTrack1[LEN_TRACK1+2];					// track 1
	uchar	sField48[LEN_FIELD48+2];				// for instalment or cvv2 for visa/master card
	uchar	szTranCurcyCode[LEN_CURCY_CODE+2];		// for DCC, transaction currency
	uchar	szHolderCurcyCode[LEN_CURCY_CODE+2];	// for DCC, holder currency
	uchar	sPINData[LEN_PIN_DATA+2];				// PIN data
	uchar	szExtAmount[LEN_EXT_AMOUNT+2];			// extra amount
	uchar 	szAddtAmount[LEN_ADDT_AMT+2];			// Additional Amount
	uchar	sICCData[LEN_ICC_DATA+2];				// ICC data, or AMEX non-EMV transaction 4DBC
	uchar	szTEchoData[LEN_TRANSECHO_DATA+2];		// Transport Echo Data
	uchar	szPayMentInfo[LEN_PAYMENT_INFO+2];		// Transport Echo Data
	uchar	szActIdent1[LEN_ACT_IDTF+2];			// Account Identification 1
	uchar	szActIdent2[LEN_ACT_IDTF+2];			// Account Identification 2
	uchar	sICCData2[LEN_ICC_DATA2+2];				// ICC data, FOR HK
	uchar	testSICCData[999+2];					// ICC data, FOR HK
	uchar 	szPinBlock[16 + 2];						//Pinblock
	uchar	szField60[LEN_FIELD60+2];
	uchar	szField61[LEN_FIELD61+2];
	uchar	sField62[LEN_FIELD62+2];
	uchar	sField63[LEN_FIELD63+2];
	uchar	sMac[LEN_MAC+2];
	uchar	szHash[LEN_HASH+2];						//For hashing
	uchar 	szNFC[LEN_NEARFIELD+2];					//For near field
	uchar	szOrigDataElement[ORIG_DATA_ELEMENT+2];	//For field 90
	uchar	szReplAmount[ORIG_DATA_ELEMENT+2];		//For field 95
	uchar	szReasonCode[MES_REASON_CODE+2];					//Field 56
	uchar	szBillers[BILLER_ISO+2];		//Field 62
	uchar	tempBillers[BILLER_ISO+2];		//Temp Field 62 For Payment leg
	uchar	tempBillers2[BILLER_ISO+2];		//Temp Field 62 For Payment leg
	uchar	szTranAmtTemp[LEN_TRAN_AMT+2];				// Txn Amount For Vas
	uchar	szTranAmtTempConv[LEN_TRAN_AMT+2];				// Txn Amount For Vas


	//Agency
	uchar	mainamount[10+2];
	uchar	fee[10+2];
	uchar	etopfee[10+2];
	uchar	superagentfee[10+2];
	uchar	aggregatorfee[10+2];
	uchar	bankcode[10+2];
	uchar	destination[10+2];
	uchar	description[100+2];
	uchar	receivername[500+2];
	uchar	bankname[100+2];
	uchar	password[30+2];
	uchar	pin[5+2];
}STISO8583;

typedef struct _tagSTVERVE
{
	//Verve backup
	uchar 	verveField0[4+2];
	uchar 	verveField3[6+2];
	uchar 	verveField4[12+2];
	uchar 	verveField7[10+2];
	uchar 	verveField11[6+2];
	uchar 	verveField12[6+2];
	uchar 	verveField13[4+2];
	uchar 	verveField18[4+2];
	uchar 	verveField22[3+2];
	uchar 	verveField25[2+2];
	uchar 	verveField26[2+2];
	uchar 	verveField28[9+2];
	uchar 	verveField32[6+2];
	uchar 	verveField37[12+2];
	uchar 	verveField41[8+2];
	uchar 	verveField42[15+2];
	uchar 	verveField43[40+2];
	uchar 	verveField49[3+2];
	uchar 	verveField62[BILLER_ISO+2];  
	uchar 	verveField123[15+2];                                                                                                                                                                                                                                                       
}VERVEBACKUP;

typedef struct _tagSTECR
{
	uchar	sMsgLen[128];							//Message Length
	uchar	sMsgType[128];							//Message Length	
	uchar	sAudNum[128];							//Audit Number
	uchar	sEcrId[128];							//Ecr Id
	uchar	sdatetime[128];							//Date Time
	uchar	sAmount[128];							//Amount
	uchar	sEAmount[128];							//Extra Amount
	uchar	sCurr[128];								//Currency Code
	uchar	sTxt[128];								//Text Message
	uchar	sRes[128];								//Response
	uchar	sTid[128];								//Tid
	uchar	sTxnNum[128];							//Txn Number
	uchar	sTxnDate[128];							//Txn Date
	uchar	sTxnMode[128];							//Txn Mode
	uchar	sTxnAven[128];							//Txn Avenue
	uchar	sTxnRrn[128];							//Txn Rrn
	uchar	sMaxPan[128];							//Max Pan
	uchar	sTxnCType[128];							//Txn Card Type
	uchar	sAuthCode[128];							//Auth Code
	uchar	sCardName[128];							//Card Name
	uchar	sExpDate[128];							//Exp Date                                                                                                                                                                                                                                                                   
}ECRISO;

// TMSר�ñ���
// For TMS use.
typedef struct _tagSTTMS8583
{
	uchar	szMsgCode[LEN_MSG_CODE+2];				// message code
	uchar	sBitMap[2*LEN_BITMAP];					// ��Ҫ��2
	uchar	szProcCode[LEN_PROC_CODE+2];			// proc code
	uchar	szSTAN[LEN_STAN+2];						// STAN
	uchar	szLocalTime[LEN_LOCAL_TIME+2];			// time, hhmmss
	uchar	szLocalDate[LEN_LOCAL_DATE+2];			// date, YYMM
	uchar	szNii[LEN_NII+2];						// NII
	uchar	szRspCode[LEN_RSP_CODE+2];				// rsp code
	uchar	szTermID[LEN_TERM_ID+2];				// terminal id
	uchar	szMerchantID[LEN_MERCHANT_ID+2];		// merchant id
	uchar	sField60[LEN_TMSFIELD60+2];
	uchar	szField61[LEN_FIELD61+2];
}STTMS8583;

typedef struct _tagPROFILE
{
	uchar	vervehost[128];							//Verve Host
	uchar	bankcode[128];
	uchar	changeover[128];
}PROFILETAG;

typedef struct _tagVAS
{
	uchar	name[30][128];
	uchar	value[30][128];
	uchar	selected[10][128];
	int count;
}PRINTVAS;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern FIELD_ATTR glEdcDataDef[];		// 8583��Ϣ����
extern FIELD_ATTR glTMSDataDef[];		// 8583��Ϣ����

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _ST8583_H

// end of file
