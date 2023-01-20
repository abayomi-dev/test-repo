#ifndef  _NETPROC_H
#define _NETPROC_H
#define TCPMAXSENDLEN		102400
int Net_Open(char *remote_addr, short remote_port);

int Net_Send(int net_hd, void *buf, int size);
int Net_Recv(int net_hd, void *buf, int size);
int Net_Close(int net_hd);

#endif
