#ifndef _RUN_FILE_H_
#define _RUN_FILE_H_

#define fileNumber 100
#define fileLen 100

char sdTest(void);
void sdInitTest(void);

void run_mount(void);
void run_unmount(void);

void sdScanDir(void);
void sdScanDirExist(void);

char isFileExist(const char *path);
void setFilePath(void);

void updatePathIndex(void);

#endif
