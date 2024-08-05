#ifndef _RUN_FILE_H_
#define _RUN_FILE_H_

#include <stdbool.h>
#include <stdint.h>

bool sdTest(void);

void run_unmount(void);

uint32_t setFilePath(void);
void setPathIndex(uint32_t index);

#endif
