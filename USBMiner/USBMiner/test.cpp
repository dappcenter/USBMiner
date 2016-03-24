#include "stdafx.h"
#include "usb.h"
#include "stdio.h"
#include "usb.h"
#include "test.h"
#include <math.h>

int DividerFrom = 0;
int DividerTo = 3;

void testAll(){
	//byte c_read[] = {0xA5, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0};
	//sendCommand(c_read,sizeof(c_read));
	test1(0);
	test1(1);
	test1(2);
	test1(3);//*/
	test11(0);
	test11(1);
	test11(2);
	test11(3);
	test2(0);
	test2(1);
	test2(2);
	test2(3);//*/
}

void test1(byte divider){
	CHECK_DIVIDER;
	byte buffer[4096*2*8];
	printf("Simpe read with divider %d\n",divider);
	open();
	byte c_reset[] = {0xA5, 0x2, 0xFF};
	sendCommand(c_reset,sizeof(c_reset));
	byte c_divider[] = {0xA5, 0x3, divider};
	sendCommand(c_divider,sizeof(c_divider));
	byte c_mask[] = {0xA5, 0x4, 0xFF, 0x0};
	sendCommand(c_mask,sizeof(c_mask));
	byte c_read[] = {0xA5, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0};
	sendCommand(c_read,sizeof(c_read));
	int len = 8*4096*2;
	int miss = readV1(len,1000,buffer);
	if(miss == 0){
		printf("       Done\n");
	}else
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!!!!!!! Error: miss %d bytes from %d\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",miss,len);
	checkExtras();
	close();
}

void test11(byte divider){
	CHECK_DIVIDER;
	byte buffer[4096*8*2];
	printf("Simpe read 4 times with divider %d\n",divider);
	open();
	byte c_reset[] = {0xA5, 0x2, 0xFF};
	sendCommand(c_reset,sizeof(c_reset));
	byte c_divider[] = {0xA5, 0x3, divider};
	sendCommand(c_divider,sizeof(c_divider));
	byte c_mask[] = {0xA5, 0x4, 0xFF, 0x0};
	sendCommand(c_mask,sizeof(c_mask));
	byte c_read[] = {0xA5, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0};
	
	sendCommand(c_read,sizeof(c_read));
	int len = 8*4096*2;
	int miss_cur = readV1(len,2000,buffer);
	int miss = miss_cur;
	if(miss_cur != 0) printf("Error mis=%d\n",miss_cur);
	checkExtras();
	//tmp_reset();

	sendCommand(c_read,sizeof(c_read));
	miss_cur = readV1(len,2000,buffer);
	if(miss_cur != 0) printf("Error mis=%d\n",miss_cur);
	miss += miss_cur;
	checkExtras();
	//tmp_reset();
	
	sendCommand(c_read,sizeof(c_read));
	miss_cur = readV1(len,2000,buffer);
	if(miss_cur != 0) printf("Error mis=%d\n",miss_cur);
	miss += miss_cur;
	checkExtras();
	//tmp_reset();
	
	sendCommand(c_read,sizeof(c_read));
	miss_cur= readV1(len,2000,buffer);
	if(miss_cur != 0) printf("Error mis=%d\n",miss_cur);
	miss += miss_cur;
	checkExtras();
	//tmp_reset();
	
	if(miss == 0){
		printf("       Done\n");
	}else
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!!!!!!! Error: miss %d bytes from %d\n\
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",miss,len*4);
	checkExtras();
	close();
}

//a5 5 f f 1f 1f 3f 3f 7f 7f 0 0 0 0 0 0 0 0 80 0
void test2(byte divider){
	CHECK_DIVIDER;
#define SIZE (2+2*2+2*4+2*8)*4096*2
	byte buffer[SIZE];
	printf("Multi read %d\n",divider);
	open();
	byte c_reset[] = {0xA5, 0x2, 0xFF};
	sendCommand(c_reset,sizeof(c_reset));
	byte c_divider[] = {0xA5, 0x3, divider};
	sendCommand(c_divider,sizeof(c_divider));
	byte c_mask[] = {0xA5, 0x4, 0xFF, 0x0};
	sendCommand(c_mask,sizeof(c_mask));
	byte c_read[] = {0xa5, 0x5, 0xf, 0xf, 0x1f, 0x1f, 0x3f, 0x3f, 0x7f, 0x7f, 0, 0, 0, 0, 0, 0, 0, 0, 0x80, 0};
	sendCommand(c_read,sizeof(c_read));
	int len = (2+2*2+2*4+2*8)*4096*2;
	int miss = readV1(len,3000*pow((double)2,divider+1.0),buffer);
	if(miss == 0){
		printf("       Done\n");
	}else
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!!!!!!! Error: miss %d bytes from %d\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",miss,len);
	checkExtras();
	close();
}