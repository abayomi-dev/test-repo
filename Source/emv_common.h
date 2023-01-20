
#ifndef _EMV_COMMON_H
#define _EMV_COMMON_H
  
#ifndef ushort 
#define ushort unsigned short
#endif
#ifndef uchar
#define uchar unsigned char
#endif
#ifndef ulong
#define ulong unsigned long
#endif
#ifndef uint
#define uint unsigned int
#endif

typedef struct  
{
    unsigned long  ulAmntAuth;     // 授权金额(unsigned long), 若为返现, 则该金额需包括ulAmntOther的金额
    unsigned long  ulAmntOther;    // 其他金额(unsigned long) 
    unsigned long  ulTransNo;      // 交易序列计数器(4 BYTE)
	unsigned char  ucTransType;    // 交易类型'9C', 0-消费/服务 1-现金/返现
	unsigned char  aucTransDate[4]; // 交易日期 YYMMDD
	unsigned char  aucTransTime[4]; // 交易时间 HHMMSS
}Clss_TransParam;

typedef struct{
    int MaxLen;
    unsigned short Tag;
    unsigned short Attr;
	unsigned short usTemplate[2];// 所属的模版，没有则为0
	unsigned char ucSource;// 三个取值EMV_SRC_TM，EMV_SRC_ICC，EMV_SRC_ISS
}ELEMENT_ATTR;


typedef struct  
{
	unsigned char	ucAidLen;				//AID Length
	unsigned char	ucAid[17];			//Current aid
	unsigned char	ucScriptLen;				//issuer script data len
	unsigned char	ucScriptData[300];			//issuer script data	
}iss_scrstrc;

#endif
