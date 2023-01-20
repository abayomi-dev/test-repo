#include "logger.h"
#include <stdarg.h>


int DisplayLogsCheck = 0;
char ucRet = 0x03;

void SerialLog(const char *message) 
{
	if (ucRet != 0x00) {
		ucRet = PortOpen(0, (unsigned char *)"115200,8,n,1");
	}
	PortSends(0, (unsigned char *)message, strlen(message));
	DelayMs(50);
}

void ShowLogs(int displaylogs, const char *format, ...)
{
	va_list ap;
	char finalMessage[10 * 1024] = { 0 };
	char store[10 * 1024] = { 0 };
	int maxLen = sizeof(finalMessage) - 5;

	/*if(strlen(finalMessage) > 10240)
		return;

	if(!DisplayLogsCheck)
		return;

	if(!displaylogs || !DisplayLogsCheck) {
		return;
	}

	if (!format)
		return;*/
	va_start(ap, format);
	vsnprintf(finalMessage, maxLen, format, ap);
	va_end(ap);
	strcat(finalMessage, "\r\n");

	SerialLog(finalMessage);
}



void SetLogs(int displaylogs) {
	if(displaylogs)
		DisplayLogsCheck = 1;
	else
		DisplayLogsCheck = 0;
	return;
}