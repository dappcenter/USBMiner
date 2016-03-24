#include "stdafx.h"
#include "log.h"
#include <iostream>
#include <time.h>

using namespace std;

void savePGM(const char *fileName,int w,int h,unsigned char *data){
	FILE *f = fopen(fileName,"wb");
	fprintf(f,"P5\n%d %d\n255\n",w,h);
	fwrite(data,1,w*h,f);
	fclose(f);
}

FILE *FPtr;
void mlog_start(){
	FPtr = fopen("measuring_log.txt","wt");
}

void mlog_end(){
	fclose(FPtr);
}

bool NeedTimeStamp = true;
clock_t LogStart = 0;
void mlog(const char *str){
	if(NeedTimeStamp){
		NeedTimeStamp = false;
		clock_t cur = clock();
		if(LogStart == 0)
			LogStart = cur;
		float f = 1;
		f /= CLOCKS_PER_SEC;
		f *= cur-LogStart;
		char buffer[128];
		if(f > 10)
			mlog("0");
		sprintf(buffer,"%1.2f ",f);
		mlog(buffer);
	}
	cout << str;
	fwrite(str,1,strlen(str),FPtr);
}

void mlog_endl(const char *str){
	if(str != NULL)
		cout << str;
	cout << endl;
	fwrite("\n",1,1,FPtr);
	NeedTimeStamp = true;
}

void mlog_hex(int val){
	char buf[128];
	sprintf(buf,"%x",val);
	mlog(buf);
}

void mlog(int val){
	char buf[128];
	sprintf(buf,"%d",val);
	mlog(buf);
}

void mlog(float val){
	char buf[128];
	sprintf(buf,"%f",val);
	mlog(buf);
}

