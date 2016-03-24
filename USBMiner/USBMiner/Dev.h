#ifndef DEV_H
#define DEV_H
#include <memory.h>

#define TICK 0.00876F

extern int LineCount;
extern int LineSize;
extern int DividerIndex;
extern int Mask;

extern float getTick();
extern int GenPrevTimeOut;

extern void devOpen();
extern void devClose();
extern int devMeasuring(int divider,int mask,bool gen,int common_time,int *expositions,bool read_result);
extern void devSetGeneratorStateByTime(float time_out);
extern void devSetGeneratorStateByTick(int time_out);
extern void devSetGeneratorStateByTickNoWait(int time_out);
extern void setDivider(int divider);
extern void setMask(int mask);
extern void reset();

class DataBuffer{
	unsigned char Buffer[1024*1024*128];
	int BufferPosition;
#define FileBlockMaxCount 0xFF
	int FileBlockPositions[FileBlockMaxCount];
	int FileBlockSize[FileBlockMaxCount];
public:
	DataBuffer():BufferPosition(0){
		/*for(int i = 0;i<FileBlockMaxCount;i++)
			FileBlockPositions[i] = FileBlockSize[i] = -1;
		for(int i = 0;i<sizeof(Buffer);i++)
			Buffer[i] = (unsigned char)(i&0xF);*/
		//reset();
	}
	void reset(){
		BufferPosition = 0;
		for(int i = 0;i<FileBlockMaxCount;i++)
			FileBlockPositions[i] = FileBlockSize[i] = -1;
		//for(int i = 0;i<sizeof(Buffer);i++)
		//	Buffer[i] = (unsigned char)(i&0xF);
		memset(Buffer,0xA5,sizeof(Buffer));
	}
	void prepare(int len){
		for(int i = 0;i<8;i++)
			memset(Buffer+BufferPosition,0xA5,len);
	}
	unsigned char *getBuffer(){ return Buffer+BufferPosition; }
	void commitBlockForFile(int file_index,int len){
		FileBlockPositions[file_index] = BufferPosition;
		FileBlockSize[file_index] = len;
		BufferPosition += len;
	}
	void saveFull();
};

extern class DataBuffer MainDataBuffer;
#endif
