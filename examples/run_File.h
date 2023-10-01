#ifndef _RUN_FILE_H_
#define _RUN_FILE_H_

#include <stdbool.h>
#include <stdint.h>

bool sdTest(void);

void run_mount(void);
void run_unmount(void);

bool fileExists(const char *path);
uint32_t setFilePath(void);

void updatePathIndex(uint32_t index);

#endif
