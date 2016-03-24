#include "stdafx.h"
#include "Dev.h"
#include "usb.h"
#include <stdio.h>
#include "log.h"

int LineCount = 0;
int LineSize = 0;
int DividerIndex = 0;
int Mask = 0;

float getTick(){
	switch(DividerIndex){
	case 0: return TICK;
	case 1: return TICK*2;
	case 2: return TICK*4;
	case 3: return TICK*8;
	default: throw "Unknown divider";
	}
}

void devOpen(){
	open();
	byte c_reset[] = {0xA5, 0x2, 0xFF};
	sendCommand(c_reset,sizeof(c_reset));

	devSetGeneratorStateByTick(0);
}

void devClose(){
	close();
}

int PrevDivider = -1;
int PrevMask = -1;

void setDivider(int divider){
	if(PrevDivider != divider){
		byte c_divider[] = {0xA5, 0x3, divider};
		sendCommand(c_divider,sizeof(c_divider));
		PrevDivider = divider;
	}
}

void setMask(int mask){
	if(PrevMask != mask){
		byte c_mask[] = {0xA5, 0x4, mask&0xFF, (mask>>8)&0xFF};
		sendCommand(c_mask,sizeof(c_mask));
		PrevMask = mask;
	}
}

void reset(){
	byte c_reset[] = {0xA5, 0x2, 0xFF};
	sendCommand(c_reset,sizeof(c_reset));

	resetBuffer();
}

int devMeasuringNoFGen(int divider,int mask,int common_time,int *expositions,bool read_result){
	setDivider(divider);
	setMask(mask);
	byte c_read[] = {0xa5, 0x5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, common_time&0xFF, (common_time>>8)&0xFF};
	int result_size = 0;
	for(int i = 0;i<15;i++)
		if(expositions[i] > 0){
			c_read[2+i] = expositions[i]-1;
			result_size += LineSize*2*(common_time/expositions[i]);
		}else
			c_read[2+i] = 0;
	MainDataBuffer.prepare(result_size);
	sendCommand(c_read,sizeof(c_read));
	if(read_result){
		int time_out = (common_time*getTick())*1000+500;
		int miss = readV1(result_size,time_out,MainDataBuffer.getBuffer());
		if(miss != 0){
			mlog("Read error: "); mlog(result_size-miss); mlog(" done, miss "); mlog(miss); mlog(" bytes"); mlog_endl(NULL);
			throw "Read error...";
		}
	}
	return result_size;
}


int devMeasuring(int divider,int mask,bool gen,int common_time,int *expositions,bool read_result){
	if(gen)
		devSetGeneratorStateByTick(common_time+50);
	else{
		devSetGeneratorStateByTick(0);
		mlog_endl("Null Clean Start:");
		devMeasuringNoFGen(divider,mask,common_time,expositions,read_result);
	}
	setDivider(divider);
	setMask(mask);
	mlog_endl("Start:");
	//int devMeasuringNoFGen(int divider,int mask,int common_time,int *expositions,bool read_result){
	return devMeasuringNoFGen(divider,mask,common_time,expositions,read_result);
/*	byte c_read[] = {0xa5, 0x5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, common_time&0xFF, (common_time>>8)&0xFF};
	int result_size = 0;
	for(int i = 0;i<15;i++)
		if(expositions[i] > 0){
			c_read[2+i] = expositions[i]-1;
			result_size += LineSize*2*(common_time/expositions[i]);
		}else
			c_read[2+i] = 0;
	MainDataBuffer.prepare(result_size);
	sendCommand(c_read,sizeof(c_read));
	if(read_result){
		int time_out = (common_time*getTick())*1000+500;
		int miss = readV1(result_size,time_out,MainDataBuffer.getBuffer());
		if(miss != 0){
			mlog("Read error: "); mlog(result_size-miss); mlog(" done, miss "); mlog(miss); mlog(" bytes"); mlog_endl(NULL);
			throw "Read error...";
		}
	}
	return result_size;*/
}

void devSetGeneratorStateByTime(float time_out){
	int tick = (int)(time_out / getTick());
	devSetGeneratorStateByTick(tick);
}

int GenPrevTimeOut = -1;
void devSetGeneratorStateByTick(int time_out){
	if(time_out == 0 && GenPrevTimeOut == 0)
		return;
	time_out = (int)(time_out * 10);
	byte c_gen[] = {0xA5, 0x6, 2, 0, 0, 0, 0, 0, time_out&0xFF, (time_out>>8)&0xFF};
	//byte c_gen[] = {0xA5, 0x6, 2, 0, 0, 0, 0, 0, 0xFF, 0xF};
	sendCommand(c_gen,sizeof(c_gen));
	if(GenPrevTimeOut > 0 && time_out <= 0){
		mlog_endl("");
		mlog(" Wait for stop 333ms ");
		Sleep(333);
		mlog_endl("");
	}
	GenPrevTimeOut = time_out;
}

void devSetGeneratorStateByTickNoWait(int time_out){
	time_out = (int)(time_out * 10);
	byte c_gen[] = {0xA5, 0x6, 2, 0, 0, 0, 0, 0, time_out&0xFF, (time_out>>8)&0xFF};
	sendCommand(c_gen,sizeof(c_gen));
}

///////////////////////////////////////////////////////////////////////
//  DataBuffer
///////////////////////////////////////////////////////////////////////
void DataBuffer::saveFull(){
	for(int i = 0;i<FileBlockMaxCount;i++)
		if(FileBlockSize[i] > 0){
			char file_name[64];
			sprintf(file_name,"data%d",i);
			FILE *fptr = fopen(file_name,"wb");
			int done = fwrite(Buffer+FileBlockPositions[i],1,FileBlockSize[i],fptr);
			if(done != FileBlockSize[i]){
				mlog("Read write: done "); mlog(done); mlog(" bytes but need "); mlog(FileBlockSize[i]); mlog_endl(NULL);;
				throw "Can't write result";
			}
			printf(" + Written %d bytes into file%d\n",FileBlockSize[i],i);
			fclose(fptr);
		}
}
