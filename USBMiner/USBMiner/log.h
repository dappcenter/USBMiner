#ifndef LOG_H
#define LOG_H

extern void mlog_start();
extern void mlog_end();

extern void mlog(const char *str);
extern void mlog_endl(const char *str);
extern void mlog_hex(int val);
extern void mlog(int val);
extern void mlog(float val);

extern void savePGM(const char *fileName,int w,int h,unsigned char *data);
#endif
