#ifndef SQLITE_TEST_H
#define SQLITE_TEST_H
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define SDK_OK                       (1)
#define SDK_EQU                      (0)        //比较大小相等时用这个
#define SDK_ERR                      (-1)
#define SDK_TIME_OUT                 (-2)       //超时
#define SDK_ESC                      (-3)
#define SDK_PARA_ERR                 (-4)

#define   SDK_SQL_ERR   (-102)


extern int sdkCardbinCreate(char* DB_Dir);
extern int sdkCardbinInsert(const char *pAscCardbin,int len);
extern int sdkCardbinDelete(const char *pAscCardbin);
extern int sdkCardbinCheck(const char *pAscCardNO,int len);
extern int sdkCardbinModify(const char *PAscOldCardbin , const char *pAscNewCardbin,int new_len);
extern int sdkCardbinBackup(const char *Bfilename , sqlite3 *pbackup);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */




















#endif
