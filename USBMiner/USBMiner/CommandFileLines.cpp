#include "stdafx.h"
#include "file_parser.h"
#include "string.h"
#include "Dev.h"
#include "usb.h"
#include <Windows.h>
#include <iostream>

CommandFileLine::CommandFileLine(CommandFile *file,int master_file_line,const char *line_buffer){
	MasterFile = file;
	MasterFileLine = master_file_line;
	if(line_buffer != NULL){
		CommandLineBuffer = new char[strlen(line_buffer)+1];
		strcpy(CommandLineBuffer,line_buffer);
	} else {
		CommandLineBuffer = NULL;
	}
}

CommandFileLine::~CommandFileLine(){
	if(CommandLineBuffer != NULL)
		delete CommandLineBuffer;
}

///////////////////////////////////////////////////////////
//   CommandFileExpositionLine
///////////////////////////////////////////////////////////
CommandFileExpositionLine::CommandFileExpositionLine(CommandFile *file,int master_file_line,const char *buffer,int divider,int mask) : 
	CommandFileLine(file,master_file_line,buffer){
		Divider = divider;
		Mask = mask;
}

void CommandFileExpositionLine::parse(){
	if(CommandLineBuffer[0] == 'E')
		IsGenOn = true;
	else
		IsGenOn = false;
	if(sscanf(CommandLineBuffer+1,"%d_%d_%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",&FileIndex,&CommonTick,
		Expositions,Expositions+1,Expositions+2,Expositions+3,
		Expositions+4,Expositions+5,Expositions+6,Expositions+7,
		Expositions+8,Expositions+9,Expositions+10,Expositions+11,
		Expositions+12,Expositions+13,Expositions+14,Expositions+15) != 18)
		throw "Can't parse exposition's parameter";
}

int CommandFileExpositionLine::getMask(){
	int cur_bit = 1;
	int mask = 0;
	for(int i = 0;i<16;i++){
		if(Expositions[i] != 0)
			mask |= cur_bit;
		cur_bit <<= 1;
	}
	return mask;
}

void CommandFileExpositionLine::runWithFlag(bool flag){
	bool gen;
	if(flag == true)
		gen = IsGenOn;
	else 
		gen = GenPrevTimeOut > 0;
	if(Mask == 0)
		Mask = getMask();
	ResultSize = devMeasuring(Divider,Mask,gen,CommonTick,Expositions,flag);
	if(flag){
		MainDataBuffer.commitBlockForFile(FileIndex,ResultSize);
		mlog("-Saved ");
		mlog(ResultSize);
		mlog(" bytes in file");
		mlog(FileIndex);
		mlog_endl(NULL);//printf("-Saved %d bytes in file%d\n",ResultSize,FileIndex);
	}
}

void CommandFileExpositionLine::print(){
	if(IsGenOn)
		mlog("On  ");
	else
		mlog("Off ");
	mlog("f");//printf("f%d %d[ ",FileIndex,CommonTime);
	mlog(FileIndex);
	mlog(" ");
	mlog(CommonTick);
	mlog("[");
	for(int i = 0;i<16;i++)
	{ mlog(Expositions[i]); mlog(" ");}//printf("%d ",Expositions[i]);
	mlog("] ");
}

bool CommandFileExpositionLine::isExposition(){
	return true;
}

float CommandFileExpositionLine::getCommonTime(){ 
	return CommonTick * getTick(); 
}
///////////////////////////////////////////////////////////
//   CommandFilePresparkLine
///////////////////////////////////////////////////////////
CommandFilePrespakLine::CommandFilePrespakLine(CommandFile *file,int master_file_line,const char *buffer) : 
	CommandFileLine(file,master_file_line,buffer){
}

void CommandFilePrespakLine::parse(){
	if(sscanf(CommandLineBuffer+1,"%f",&PresparkTime) != 1)
		throw "Can't parse prespark parameter";
}

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

void CommandFilePrespakLine::runWithFlag(bool flag){
	CommandFileExpositionLine *exp = MasterFile->getFirstExposition();
	float exp_time = exp->getCommonTime();
	int time = (PresparkTime-exp_time)*1000;

	struct stat st;
    int result = stat("extra_spark.csv", &st);
	if(result == 0){
		char buffer[129];
		FILE *fl = fopen("extra_spark.csv","rt");
		int real_size = fread(buffer,1,128,fl);
		buffer[real_size] = 0;
		fclose(fl);
		
		int sensor,pixel,width;
		int exp;
		int level;

		sscanf(buffer,"%d;%d;%d;%d;%d;",&sensor,&pixel,&width,&exp,&level);
		mlog("Controlled prespark sn:");
		mlog(sensor); sensor --;
		mlog(" pixel:");
		mlog(pixel);
		mlog(" width:");
		mlog(width);
		mlog(" exp:");
		mlog(exp);
		if(exp <= 1){
			exp = 1;
			mlog("-corrected to 1");
		}
		mlog(" level:");
		mlog(level);

		int from = pixel-width;
		if(from < 32)
			from = 32;
		int to = pixel+width;
		if(to > 4064)
			to = 4064;
		//devSetGeneratorStateByTime(PresparkTime+PresparkTime/2);

		clock_t to_clock;
		to_clock = clock() + CLOCKS_PER_SEC * PresparkTime;

		//devMeasuring(int divider,int mask,bool gen,int common_time,int *expositions,bool read_result)
		int expositions[15];
		expositions[sensor] = exp;

		const int size = 512;
		unsigned char *imageData = new unsigned char[4096*size];
		int nul[4096];
		int y = 0;
		clock_t spark_start = 0;
		int prevLevels[16];
		while(clock() < to_clock){
			devMeasuring(DividerIndex,1<<sensor,true,exp,expositions,true);
			unsigned char *buffer = MainDataBuffer.getBuffer();
			unsigned short data[4096];

			for(int i = 0;i<4096;i++)
				data[i] = ((buffer[i*2+1]&0xF)<<8) | (buffer[i*2] & 0xFF);

			if(y == 0)
				for(int i = 0;i<4096;i++)
					nul[i] = data[i];

			for(int i = 0;i<4096;i++){
				int val = data[i] - nul[i];
				if(val < 0)
					val = 0;
				data[i] = val;
			}
			
			if(y < size){
				int baseAddr = y * 4096;
				for(int i = 0;i<4096;i++){
					int val = data[i]*256/level;
					if(val > 255)
						val = 255;
					imageData[baseAddr+i] = (unsigned char)(val);
				}
				imageData[baseAddr+from-1] = 255;
				imageData[baseAddr+from-2] = 255;
				imageData[baseAddr+to+1] = 255;
				imageData[baseAddr+to+2] = 255;
				y ++;
			}

			int min_level = data[from];
			int max_level = 0;
			for(int i = from;i<= to;i++){
				int val = (data[i-1]+data[i]+data[i+1])/3;
				if(val > max_level)
					max_level = val;
				if(val < min_level)
					min_level = val;
			}

			for(int i = 15;i>0;i--)
				prevLevels[i] = prevLevels[i-1];
			prevLevels[0] = max_level;

			mlog(" curLev=[");
			mlog(max_level); mlog(" "); mlog(prevLevels[1]); mlog(" "); mlog(prevLevels[2]); mlog(" "); mlog(prevLevels[3]);
			mlog("-");
			mlog(min_level);
			mlog("] ");

			if(max_level < 50)
				continue;
			else{
				if(spark_start == 0){
					spark_start = clock();
					continue;
				} else {
					if((clock()-spark_start) < CLOCKS_PER_SEC*2)
						continue;
				}
			}
			mlog("Check level");
			if(max_level > level){
				int depth = 0;
				for(int i = 0;i<14;i++){
					if(prevLevels[i] > level)
						depth ++;
					else
						break;
				}
				mlog(" depth=");
				mlog(depth);
				mlog(" ");
				if(depth > 1){
					to = 0;
					break;
				}
			}
		}

		for(int i = 0;i<from;i++)
			imageData[i] = 255;
		for(int i = to;i<4096;i++)
			imageData[i] = 255;

		mlog(" Save image...");
		devSetGeneratorStateByTickNoWait(5000);
		//savePGM(const char *fileName,int w,int h,unsigned char *data)
		savePGM("prespark.pgm",4096,y,imageData);

		delete imageData;

		mlog(" Signal lower...");
		devSetGeneratorStateByTickNoWait(0);
		Sleep(250);
		devSetGeneratorStateByTickNoWait(10);
		mlog(" Signal high... Prespark done.");
		return;
	}
	//int time = (int)(PresparkTime*1000);
	//int prespark_tick = ;
	if(exp == NULL || PresparkTime < exp->getCommonTime()){
		devSetGeneratorStateByTime(PresparkTime);
		Sleep(PresparkTime*1000);
		mlog(" Simple prespark ");
	} else {
		mlog_endl("");
		mlog(" Dump prespark "); mlog(time);mlog_endl("");
		devSetGeneratorStateByTime(PresparkTime+1);
		Sleep(time);
		mlog_endl(NULL);
		exp->runWithFlag(false);
		mlog(" Dumy read ");mlog(exp_time);
		int to_read = readDumy(exp->ResultSize,exp_time*1000+1000);
		if(to_read > 0){
			mlog("Exception: Can't dumy read... miss ");mlog(to_read);mlog_endl(" bytes...");
			throw "Can't make dumy read...";
		}
		mlog("Done.");
	}//*/
}

void CommandFilePrespakLine::print(){
	mlog("Prespark ");
	mlog(PresparkTime);
	mlog("s");
}

bool CommandFilePrespakLine::isExposition(){
	return false;
}

///////////////////////////////////////////////////////////
//   CommandFileFinal
///////////////////////////////////////////////////////////
CommandFileFinal::CommandFileFinal(CommandFile *file,int master_file_line) : CommandFileLine(file,master_file_line,NULL){
}

void CommandFileFinal::parse(){
}

void CommandFileFinal::runWithFlag(bool flag){
	devSetGeneratorStateByTick(0);
	if(checkExtras() != 0)
		throw "Extra bytes found...";
}

void CommandFileFinal::print(){
	mlog("Close all ");
}

bool CommandFileFinal::isExposition(){
	return false;
}

/*
///////////////////////////////////////////////////////////
//   CommandFileDividerLine
///////////////////////////////////////////////////////////
CommandFileDividerLine::CommandFileDividerLine(CommandFile *file,int master_file_line,const char *buffer) : 
	CommandFileLine(file,master_file_line,buffer){
}

void CommandFileDividerLine::parse(){
	if(sscanf(CommandLineBuffer+1,"%d",&NewDivider) != 1)
		throw "Can't parse prespark parameter";
}

void CommandFileDividerLine::runWithFlag(bool flag){
	Divider = NewDivider;
}

void CommandFileDividerLine::print(){
	printf("NewDivider %d",NewDivider);
}

bool CommandFileDividerLine::isExposition(){
	return false;
}

///////////////////////////////////////////////////////////
//   CommandFileMaskLine
///////////////////////////////////////////////////////////
CommandFileMaskLine::CommandFileMaskLine(CommandFile *file,int master_file_line,const char *buffer) : 
	CommandFileLine(file,master_file_line,buffer){
}

void CommandFileMaskLine::parse(){
	if(sscanf(CommandLineBuffer+1,"%d",&NewMask) != 1)
		throw "Can't parse prespark parameter";
}

void CommandFileMaskLine::runWithFlag(bool flag){
	Mask = NewMask;
}

void CommandFileMaskLine::print(){
	printf("NewDivider %fs ",NewMask);
}

bool CommandFileMaskLine::isExposition(){
	return false;
}
*/
