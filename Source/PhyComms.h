#ifndef _PHYCOMMS_H
#define _PHYCOMMS_H
#define OK 0
#define FAIL 1
int WirelessInit();
int WirelessDial(uchar *sAPN,uchar *sUser,uchar *sPwd);
int WirelessHangUp();
#endif
