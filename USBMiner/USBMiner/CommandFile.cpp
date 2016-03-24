#include "stdafx.h"
#include "file_parser.h"
#include <string.h>
#include <stdio.h>
#include "usb.h"

#include "Dev.h"

CommandFile::CommandFile(const char *file_name){
	FileName = new char[strlen(file_name)+1];
	strcpy(FileName,file_name);
	Lines = NULL;
	FirstExpositionCommand = NULL;
}

CommandFile::~CommandFile(){
	delete FileName;
	if(Lines != NULL){
		for(int i = 0;i<TextLineCount;i++)
			delete Lines[i];
		delete Lines;
	}
}

void CommandFile::openAndParse(){
	FILE *fptr = fopen(FileName,"rb");
	if(fptr == NULL)
		throw "Can't open file";
	if(fseek(fptr,0,SEEK_END))
		throw "Can't fseek";
	int size = ftell(fptr);
	char *buffer = new char[size+1];
	if(fseek(fptr,0,SEEK_SET))
		throw "Can't check file size";
	if(fread(buffer,1,size,fptr) != size)
		throw "Can't read all data";
	fclose(fptr);
	buffer[size] = 0;
	
	TextLineCount = 0;
	for(int i = 0;i<size;i++){
		if(buffer[i] == 0xD)
			buffer[i] = ' ';
		if(buffer[i] == 0xA || buffer[i] == ';'){
			buffer[i] = 0;
			TextLineCount ++;
		}
	}

	Lines = new CommandFileLine*[++TextLineCount];

	int line = 0;
	for(int i = 0;i<size;i++){
		switch(buffer[i]){
		case 0:
		case ' ':
			break;
		case 'n':
			if(sscanf(buffer+i,"n%d",&LineCount) != 1)
				throw "Can't parse 'n' parameter";
			break;
		case 's':
			if(sscanf(buffer+i,"s%d",&LineSize) != 1)
				throw "Can't parse 's' parameter";
			break;
		case 'd':
			if(sscanf(buffer+i,"d%d",&DividerIndex) != 1)
				throw "Can't parse 'd' parameter";
			/*Lines[line] = new CommandFileDividerLine(this,line,buffer+i);
			Lines[line]->parse();
			line ++;*/
			break;
		case 'm':
			if(sscanf(buffer+i,"m%d",&Mask) != 1)
				throw "Can't parse 'm' parameter";
			/*Lines[line] = new CommandFileMaskLine(this,line,buffer+i);
			Lines[line]->parse();
			line ++;*/
			break;
		case 'E':
		case 'e':{
			CommandFileExpositionLine *exp = new CommandFileExpositionLine(this,line,buffer+i,DividerIndex,Mask);
			Lines[line] = exp;
			Lines[line]->parse();
			if(Mask == 0)
				Mask = exp->getMask();
			line ++;
			if(FirstExpositionCommand == NULL)
				FirstExpositionCommand = exp;
			break;}
		case 'p':
			Lines[line] = new CommandFilePrespakLine(this,line,buffer+i);
			Lines[line]->parse();
			line ++;
			break;
		default:
			throw "Unknown commands...";
		}
		for(;i<size && buffer[i] != 0;i++);
	}
	Lines[line] = new CommandFileFinal(this,line);
	TextLineCount = line+1;
}

void CommandFile::run(){
	mlog("SensorCount   = ");mlog(LineCount);mlog_endl(NULL);
	mlog("SensorSize    = ");mlog(LineSize);mlog_endl(NULL);
	mlog("StartDivider  = ");mlog(DividerIndex);mlog_endl(NULL);
	mlog("StartMask     = ");mlog(Mask);mlog_endl(NULL);

	devOpen();
	setDivider(DividerIndex);
	setMask(Mask);
	printf("\n");
	for(int i = 0;i<TextLineCount;i++)
		Lines[i]->runNormal();

	/*devSetGeneratorStateByTime(5);
	Sleep(5000);
	mlog_endl("");
	mlog("Mark");*/

	devClose();
}

