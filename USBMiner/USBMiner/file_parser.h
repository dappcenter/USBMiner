#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "log.h"

#pragma once

class CommandFile;
class CommandFileExpositionLine;

class CommandFileLine{
protected:
	char *CommandLineBuffer;
	CommandFile *MasterFile;
	int MasterFileLine;
public:
	CommandFileLine(CommandFile *file,int master_file_line,const char *line);
	~CommandFileLine();

	virtual void parse() = 0;
	void runNormal(){print(); mlog(":"); runWithFlag(true);  mlog_endl(NULL);};
	virtual void runWithFlag(bool as_usual) = 0;
	virtual void print() = 0;
	virtual bool isExposition() = 0;
};

class CommandFile{
	char *FileName;
	int TextLineCount;
	CommandFileLine **Lines;
	CommandFileExpositionLine *FirstExpositionCommand;
public:
	CommandFile(const char *file_name);
	~CommandFile();
	void openAndParse();
	void run();
	CommandFileExpositionLine *getFirstExposition(){ return FirstExpositionCommand; }
};

class CommandFileExpositionLine : public CommandFileLine{
	int CommonTick;
	int Expositions[16];
	bool IsGenOn;
	int FileIndex;
	int Divider,Mask;
public:
	int ResultSize,TimeOut;

	CommandFileExpositionLine(CommandFile *file,int master_file_line,const char *buffer,int divider,int mask);
	void parse();
	void runWithFlag(bool as_usual);
	void print();
	bool isExposition();

	int getMask();
	float getCommonTime();
	int getCommonTick(){return CommonTick;}
};


class CommandFilePrespakLine : public CommandFileLine{
	float PresparkTime;
public:
	CommandFilePrespakLine(CommandFile *file,int master_file_line,const char *buffer);
	void parse();
	void runWithFlag(bool flag);
	void print();
	bool isExposition();
};

class CommandFileFinal : public CommandFileLine{
public:
	CommandFileFinal(CommandFile *file,int master_file_line);
	void parse();
	void runWithFlag(bool as_usual);
	void print();
	bool isExposition();
};

/*class CommandFileDividerLine : public CommandFileLine{
	int NewDivider;
public:
	CommandFileDividerLine(CommandFile *file,int master_file_line,const char *buffer);
	void parse();
	void runWithFlag(bool as_usual);
	void print();
	bool isExposition();
};

class CommandFileMaskLine : public CommandFileLine{
	int NewMask;
public:
	CommandFileMaskLine(CommandFile *file,int master_file_line,const char *buffer);
	void parse();
	void runWithFlag(bool as_usual);
	void print();
	bool isExposition();
};*/

#endif
