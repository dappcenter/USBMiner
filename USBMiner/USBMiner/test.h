#ifndef TEST_H
#define TEST_H

#include "usb.h"

extern int DividerFrom;
extern int DividerTo;
#define CHECK_DIVIDER if(divider < DividerFrom || divider > DividerTo) return;

extern void test1(byte divider);
extern void test11(byte divider);
extern void test2(byte divider);

extern void testAll();

#endif