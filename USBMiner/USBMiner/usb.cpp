#include "stdafx.h"
#include <Windows.h>
#include "ftd2xx.h"
#include <time.h>
#include <conio.h>
#include "log.h"
#include "Dev.h"

FT_HANDLE ftHandle;
//#define TRANSFER_SIZE 4096*2*2
#define TRANSFER_SIZE 0xFF

void open(){
//FT_HANDLE ftHandle;
	FT_STATUS ftStatus;
	UCHAR Mask = 0xff;
	UCHAR Mode;
	UCHAR LatencyTimer = 2; //ourdefault setting is 16
	ftStatus = FT_Open(0, &ftHandle);
	if(ftStatus != FT_OK)
	{
		throw "Open error";
	}
	Mode = 0x00; //reset mode
	ftStatus = FT_SetBitMode(ftHandle, Mask, Mode);
	//delay_ms(10);
	Sleep(20);
	Mode = 0x40; //Sync FIFO mode
	ftStatus = FT_SetBitMode(ftHandle, Mask, Mode);
	int done;
	if (ftStatus == FT_OK)
	{
		ftStatus = FT_SetLatencyTimer(ftHandle,LatencyTimer);
		ftStatus = FT_SetUSBParameters(ftHandle,0x10000,0x10000);
		ftStatus = FT_SetFlowControl(ftHandle,FT_FLOW_RTS_CTS,0,0);
	//access data from here
		FT_Purge(ftHandle, FT_PURGE_RX);
		FT_Purge(ftHandle, FT_PURGE_TX);
	}
	else
	{
		throw "FT_SetBitMode FAILED!";
	}
}

void open_old(){
	FT_STATUS ftStatus; 
	FT_DEVICE_LIST_INFO_NODE *devInfo; 
	DWORD numDevs; // create the device information list 
	char error_found = 0;

	ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (ftStatus != FT_OK)
		throw "test";
	
	if(numDevs <= 0)
		throw "No device found.";

	devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs); // get the device information list 
	ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs);
	if (ftStatus != FT_OK)
		throw "Can't get device info";
	
	char *dev_descr = "USB SpectroStar Dev V1";
	size_t dev_descr_len = strlen(dev_descr);
	int found_dev_index = -1;
	for (unsigned int dev_n = 0; dev_n < numDevs; dev_n++){
		if(strncmp(devInfo[dev_n].Description,dev_descr,dev_descr_len) == 0){
			found_dev_index = dev_n;
			break;
		}
	}
	free(devInfo);

	if(found_dev_index < 0)
		throw "Can't find device";

	ftStatus = FT_Open(found_dev_index,&ftHandle); 
	if (ftStatus != FT_OK)
		throw "Can't open device";

	ftStatus = FT_ResetPort(ftHandle);
	if (ftStatus != FT_OK)
		throw "Can't reset port";

	Sleep(20);

	ftStatus = FT_ResetDevice(ftHandle);
	if (ftStatus != FT_OK)
		throw "Can't reset device";

	Sleep(20);

	ftStatus = FT_SetBitMode(ftHandle, 0xFF, 0x0);
	if(ftStatus != FT_OK)
		throw "Can't reset by 0 mode";

	Sleep(20);

	ftStatus = FT_SetBitMode(ftHandle, 0xFF, 0x40);
	if(ftStatus != FT_OK)
		throw "Can't set bit mode";

	ftStatus = FT_SetUSBParameters(ftHandle,0x10000,0x10000);
	if (ftStatus != FT_OK)
		throw "Can't setup buffer";

	Sleep(20);

	ftStatus = FT_SetTimeouts(ftHandle,1,1);
	if(ftStatus != FT_OK)
		throw "Can't time out";

	ftStatus = FT_SetResetPipeRetryCount(ftHandle,100);
	if(ftStatus != FT_OK)
		throw "Can't set reset time";

	ftStatus = FT_SetDeadmanTimeout(ftHandle,0);
	if(ftStatus != FT_OK)
		throw "Can't FT_SetDeadmanTimeout";//*/

	ftStatus = FT_SetLatencyTimer(ftHandle,1); // 5 - does not work
	if(ftStatus != FT_OK)
		throw "Can't FT_SetLatencyTimer";//*/

	ftStatus = FT_SetFlowControl(ftHandle,FT_FLOW_RTS_CTS,0,0);
	if(ftStatus != FT_OK)
		throw "Can't FT_SetFlowControl";//*/

	ftStatus = FT_Purge(ftHandle,FT_PURGE_RX | FT_PURGE_TX);
	if(ftStatus != FT_OK)
		throw "Can't FT_Purge";
}

void close(){
	mlog("Closing...");
	FT_STATUS ftStatus;

	/*ftStatus = FT_ResetPort(ftHandle);
	if (ftStatus != FT_OK)
		throw "Can't reset port";

	ftStatus = FT_ResetDevice(ftHandle);
	if (ftStatus != FT_OK)
		throw "Can't reset device";//*/

	ftStatus = FT_Close(ftHandle);
	if (ftStatus != FT_OK)
		throw "Can't close";
	mlog_endl("Done");
}

typedef unsigned char byte;

void sendCommand(byte *buffer_src,DWORD size){
	Sleep(50);
	// rebuild to new version
	byte buffer[32] = {0, 0xA5};
	buffer[2] = (byte)(size-2);
	for(unsigned int i = 1;i<size;i++)
		buffer[2+i] = buffer_src[i];
	size += 2;
	//-------------------------*/
	mlog(" ");
	for(unsigned int i = 0;i<size;i++){
		mlog_hex(buffer[i]);mlog(" ");
	}
	mlog("  ");
	FT_STATUS ftStatus;
	DWORD written;
	ftStatus = FT_Write(ftHandle,buffer,size,&written);
	if(ftStatus != FT_OK)
		throw "Can't write data";
	if(written != size)
		throw "Can't write all data";
	/*DWORD rx_size,tx_size,event_status;
	do{
		Sleep(1);
		ftStatus = FT_GetStatus(ftHandle,&rx_size,&tx_size,&event_status);
		if (ftStatus != FT_OK)
			throw "Can't check write status";
	} while(tx_size > 0);*/
}

//byte ReadBuffer[0xFFFFFF];
//int ReadBufferIndex = 0;
#define READ_SIZE 512
//int Sizes[1024*1024*128];
int readV2(int len,int time_out,byte *buffer){
	//int rc = 0;
	FT_STATUS ftStatus;
	time_t to = GetTickCount()+time_out+100;
	DWORD to_read,read_byte;
	byte *final_buffer = buffer+len;

	//int time_found = 0,time_empty = 0;
	//int sizes = 0;
	do{
		//to_read = READ_SIZE;
		ftStatus = FT_GetQueueStatus(ftHandle,&to_read);
		//Sizes[sizes++] = to_read;
		if(to_read == 0) {
			//time_empty ++;
			if(GetTickCount() >= to)
				break;
		} else {
			//time_found ++;
			ftStatus = FT_Read(ftHandle,buffer,to_read,&read_byte);
			buffer += read_byte;
			//Sizes[sizes++] = to_read;
		}
	} while(buffer < final_buffer);

	DWORD rx_size,tx_size,event_status;
	ftStatus = FT_GetStatus(ftHandle,&rx_size,&tx_size,&event_status);
	if (ftStatus != FT_OK)
		throw "Can't check write status";
	if(rx_size != 0){
		mlog("Extra bytes found: "); mlog((int)rx_size); mlog_endl(NULL);
		throw "ReadV1 Extra bytes found!!!!";
	}

	int miss = final_buffer-buffer;
	return miss;
}


int readV1(int len,int time_out,byte *buffer){
	//int rc = 0;
	FT_STATUS ftStatus;
	time_t to = GetTickCount()+time_out+10000;
	DWORD to_read,read_byte;
	byte *final_buffer = buffer+len-READ_SIZE;

	do{
		to_read = READ_SIZE;
		ftStatus = FT_Read(ftHandle,buffer,to_read,&read_byte);
		if(read_byte == 0 && GetTickCount() >= to)
			break;
		buffer += read_byte;
	} while(buffer < final_buffer);
	
	final_buffer += READ_SIZE;
	do{
		to_read = READ_SIZE;
		if(to_read > len)
			to_read = len;
		ftStatus = FT_Read(ftHandle,buffer,to_read,&read_byte);
		//if(ftStatus != FT_OK)
		//	throw "Read error";
		if(read_byte == 0) {
			if(GetTickCount() >= to)
				break;
		} else 
			buffer += read_byte;
	} while(buffer < final_buffer);

	DWORD rx_size,tx_size,event_status;
	ftStatus = FT_GetStatus(ftHandle,&rx_size,&tx_size,&event_status);
	if (ftStatus != FT_OK)
		throw "Can't check write status";
	if(rx_size != 0){
		mlog("Extra bytes found: "); mlog((int)rx_size); mlog_endl(NULL);
		throw "ReadV1 Extra bytes found!!!!";
	}

	int miss = final_buffer-buffer;
	/*if(miss > 0 && miss < 100)
	{
		int start = LineSize*2-miss;
		byte *check_buf = buffer-start;
		int sn_cand = 0;
		bool error_found = false;
		for(int i = start;i>=2;i-=2){
			while(sn_cand < 16 && ((Mask>>sn_cand)&1) == 0 )
				sn_cand ++;
			if(sn_cand >= 16){
				sn_cand = 0;
				while(sn_cand < 16 && ((Mask>>sn_cand)&1) == 0 )
					sn_cand ++;
			}
			byte b1 = *buffer-i;
			byte b2 = *(buffer-i+1);
			int sn = b2>>4;
			if(sn != sn_cand){
				error_found = true;
				break;
			}
			sn_cand ++;
		}
		if(error_found == false){
			miss = 0;
		}
	}*/
	return miss;
}

int readDumy(int len,int time_out){
	unsigned char buffer[READ_SIZE];
	FT_STATUS ftStatus;
	time_t to = GetTickCount()+time_out+1000;
	DWORD to_read,read_byte;
	do{
		to_read = READ_SIZE;
		if(to_read > len)
			to_read = len;
		ftStatus = FT_Read(ftHandle,buffer,to_read,&read_byte);
		if(read_byte == 0 && GetTickCount() >= to)
			break;
		len -= read_byte;
	} while(len > 0);

	DWORD rx_size,tx_size,event_status;
	ftStatus = FT_GetStatus(ftHandle,&rx_size,&tx_size,&event_status);
	if (ftStatus != FT_OK)
		throw "Can't check write status";
	if(rx_size != 0){
		mlog("Extra bytes found: "); mlog((int)rx_size); mlog_endl(NULL);
		throw "ReadV1 Extra bytes found!!!!";
	}
	return len;//*/
}

int checkExtras(){
	int extra = 0;
	byte buffer[0xFFFF*8];
	Sleep(500);
	DWORD rx_size,tx_size,event_status;
	FT_STATUS ftStatus = FT_GetStatus(ftHandle,&rx_size,&tx_size,&event_status);
	if (ftStatus != FT_OK)
		throw "Can't check write status";
	if(rx_size == 0)
		return 0;
	Sleep(1000);
	DWORD q_status;
	FT_GetQueueStatus(ftHandle,&q_status);
	return q_status;
	//no_printf(" Q=%d ",q_status);
	/*if(rx_size == 0 && tx_size == 0)
		return 0;
	else{
	//no_printf("rx_size=%d,tx_size=%d,event_status=%d\n",rx_size,tx_size,event_status);
	int miss = 0;
	do{
		ftStatus = FT_Read(ftHandle,buffer,to_read,&read_byte);
		miss = readV1(sizeof(buffer),1000,buffer);
		if(miss != sizeof(buffer)){
			extra += 1024-miss;
			//Sleep(1000);
			//no_printf(".");
		} else
			break;
	}while(true);
	if(extra > 0)
	{	
		mlog(extra);
		mlog_endl("EXTRA BYTES!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}
	}
	FT_Purge(ftHandle,FT_PURGE_RX|FT_PURGE_TX);
	FT_SetBreakOff(ftHandle);
	return extra;*/
}


void tmp_reset(){
	FT_STATUS ftStatus;
	
	ftStatus = FT_ResetDevice(ftHandle);
	if (ftStatus != FT_OK)
		throw "Can't reset device";

	ftStatus = FT_SetBitMode(ftHandle, 0xFF, 0x40);
	if(ftStatus != FT_OK)
		throw "Can't set bit mode";

	ftStatus = FT_SetTimeouts(ftHandle,100,100);
	if(ftStatus != FT_OK)
		throw "Can't time out";//*/

	Sleep(100);
}

void resetBuffer(){
	byte buffer[0xFFFF];
	DWORD read = 1;
	while(read != 0){
		FT_STATUS ftStatus = FT_Read(ftHandle,buffer,0xFFFF,&read);
		if(ftStatus != FT_OK)
			throw "Empty buffer. Can't read";
	}
}