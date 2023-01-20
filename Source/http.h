#ifndef _HTTP_H
#define _HTTP_H

#define HTTP_OK						0
#define HTTP_SEND_ERR				-12
#define HTTP_RECV_ERR				-13
#define HTTP_RESP_ERR				-14
#define MAX_DATA_LEN 200240 

int httpPostData(char *ip, char *port, char *url, char *data, char *fromServer);
int httpGetData(char *ip, int port, char *tid, char *url, char *fromServer);
int httpGetDataSsl(char *ip, int port, char *tid, char *url, char *fromServer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _GLOBAL_H