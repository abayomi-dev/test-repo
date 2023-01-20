#pragma once
//#include "global.h"
#include <posapi.h>
#include <posapi_all.h>
#include "Applib.h"
#include <string.h>
#include "PhyComms.h"
#include "NetProc.h"
enum {HTTP_POST, HTTP_GET};

#define DEFAULT_REQUEST_TIMEOUT 60L

/*typedef struct MemoryStruct {
	char *memory;
	size_t size;
}MemoryStruct;*/

 typedef enum layoutT
{
  LAYOUT_CENTER,
  LAYOUT_INVERSE,
  LAYOUT_DEFAULT
}layoutT;

typedef struct MemoryStruct {
	char *memory;
	size_t size;
	int http_status;
	int is_remote_download;
}MemoryStruct;

int PostFlutterWaveNotification();
int PostCellulantNotificationFCMB();
int PostTransactionNotificationFCMB();
int PostCelTinTransfer(char data [800]);
int PostNameLookup(char data [150]);
int GetCelPlatformID(void);
int GetCelLogin(char pin[15]);

/**
* @param httpMethod
* @param hostURL
* @param postData
* @param post_data_len
* @param headers
* @param header_len
* @param chunk [out] @brief remember to always call free(chunk.memory);
* @return
*/

int sendHttpRequest(uchar httpMethod, 
	const char* hostURL, const char* postData, size_t post_data_len, const char** headers, size_t header_len, MemoryStruct* chunk);
	
int sendHttpDldRequest(unsigned char httpMethod, const char* hostURL, const char* postData, size_t post_data_len, const char** headers, size_t header_len, MemoryStruct* chunk);

int sendHttpRemoteDRequest(unsigned char httpMethod, const char* hostURL, const char* postData, size_t post_data_len, const char** headers, size_t header_len, MemoryStruct* chunk);