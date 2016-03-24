#ifndef USB_H
#define USB_H

#include <Windows.h>

#ifndef byte
typedef unsigned char byte;
#endif

extern void open();
extern void close();
extern void sendCommand(byte *buffer_src,DWORD size);
// returns number of unread bytes... if 0 - all done.
extern int readV1(int len,int time_out,byte *buffer);
extern inline int readDumy(int len,int time_out);
extern int checkExtras();
extern void resetBuffer();

#endif
