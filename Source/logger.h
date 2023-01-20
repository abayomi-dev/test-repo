#ifndef _LOGGER_H
#define _LOGGER_H

void ShowLogs(int displaylogs, const char *format, ...);
void SetLogs(int displaylogs);

extern int DisplayLogsCheck;
extern char ucRet;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _GLOBAL_H