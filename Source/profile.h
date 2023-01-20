#ifndef _PROFILE_H
#define _PROFILE_H

void FreshProfile();
void UpdateProfile();
void KeyProfile();
void OldProfile();
void parseParameter(char *orig, char* format);
extern int checkNew;
extern int numLines;
extern int fresh;
extern int cancelWifi;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _GLOBAL_H