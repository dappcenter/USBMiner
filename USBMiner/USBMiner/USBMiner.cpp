///////////////////////////////////////////////////////////////////////
// USBMiner.cpp : Defines the entry point for the console application.
// Additional library: ftd2xx.lib
// Define: FTD2XX_EXPORTS
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Windows.h>
#include "usb.h"
#include "test.h"
#include "file_parser.h"
#include "Dev.h"
#include "log.h"

class DataBuffer MainDataBuffer;

void error(const char *msg){
	mlog_endl(NULL);
	mlog("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!\n!        Error: "); 
	mlog(msg); 
	mlog("\n!\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); 
}

void runProgramm(const char *file_name){
	for(int i = 0;i<16;i++)
		MainDataBuffer.reset();

	CommandFile file(file_name);
	file.openAndParse();

	for(int i = 0;i<128;i++)
	{
		char file_name[128];
		sprintf(file_name,"data%d",i);
		remove(file_name);
	}

	file.run();
	MainDataBuffer.saveFull();
}

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char** argv)
{
	int ret_code = 0;
	try{
		//Sleep(1000);
		mlog_start();
		HANDLE thread = GetCurrentThread();
		DWORD prior = GetPriorityClass(thread);
		mlog("Start prior:");
		mlog((int)prior);
		SetPriorityClass(thread,REALTIME_PRIORITY_CLASS);
		prior = GetPriorityClass(thread);
		mlog(" new prior:");
		mlog((int)prior);
		mlog_endl(NULL);
		if(argc == 1)
			testAll();
		else{
			if(argc == 2)
				runProgramm(argv[1]);
			else{
				int count;
				int ok = 0,error = 0;
				sscanf(argv[2],"%d",&count);
				for(int i = 0;i<count;i++)
					try{
						runProgramm(argv[1]);
						ok ++;
					} catch(const char *msg){
						error ++;
						//close();
						break;
					}
				mlog_endl(NULL);mlog_endl(NULL);mlog_endl(NULL);
				if(error == 0)
					mlog("  Ok=");
				else
					mlog("  !!!! Faile after ");
				mlog(ok);//mlog(" Errors=");mlog(error);mlog(" ");mlog(ok*100/(ok+error));mlog("%");
			}
		}
	} catch(const char *msg){
		ret_code = 0xFF;
		error(msg);
		printf("\n");
		try{
			devSetGeneratorStateByTick(0);
		} catch(const char *msg){}
		try{
			close();
		} catch(const char *msg){}
	}
	mlog_end();
	return ret_code;
}

