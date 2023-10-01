#include "run_File.h"
#include "f_util.h"
#include "pico/stdlib.h"
#include "hw_config.h"

#include <stdio.h>
#include <stdlib.h> // malloc() free()
#include <string.h>

char *disPath;

static sd_card_t *sd_get_by_name(const char *const name) {
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->pcName, name)) return sd_get_by_num(i);
    // DBG_PRINTF("%s: unknown name %s\n", __func__, name);
    return NULL;
}

static FATFS *sd_get_fs_by_name(const char *name) {
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->pcName, name)) return &sd_get_by_num(i)->fatfs;
    // DBG_PRINTF("%s: unknown name %s\n", __func__, name);
    return NULL;
}

void run_mount() {
    const char *arg1 = strtok(NULL, " ");
    if (!arg1) arg1 = sd_get_by_num(0)->pcName;
    FATFS *p_fs = sd_get_fs_by_name(arg1);
    if (!p_fs) {
        printf("Unknown logical drive number: \"%s\"\n", arg1);
        return;
    }
    FRESULT fr = f_mount(p_fs, arg1, 1);
    if (FR_OK != fr) {
        printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    sd_card_t *pSD = sd_get_by_name(arg1);
    // myASSERT(pSD);
    pSD->mounted = true;
}

void run_unmount() {
    const char *arg1 = strtok(NULL, " ");
    if (!arg1) arg1 = sd_get_by_num(0)->pcName;
    FATFS *p_fs = sd_get_fs_by_name(arg1);
    if (!p_fs) {
        printf("Unknown logical drive number: \"%s\"\n", arg1);
        return;
    }
    FRESULT fr = f_unmount(arg1);
    if (FR_OK != fr) {
        printf("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    sd_card_t *pSD = sd_get_by_name(arg1);
    // myASSERT(pSD);
    pSD->mounted = false;
}

void ls(const char *dir) {
    FRESULT fr; /* Return value */
    char const *p_dir = dir;
    printf("Directory Listing: %s\n", p_dir);
    DIR dj;      /* Directory object */
    FILINFO fno; /* File information */
    memset(&dj, 0, sizeof dj);
    memset(&fno, 0, sizeof fno);
    fr = f_findfirst(&dj, &fno, p_dir, "*");
    if (FR_OK != fr) {
        printf("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    while (fr == FR_OK && fno.fname[0]) { /* Repeat while an item is found */
        /* Create a string that includes the file name, the file size and the
         attributes string. */
        const char *pcWritableFile = "writable file",
                   *pcReadOnlyFile = "read only file",
                   *pcDirectory = "directory";
        const char *pcAttrib;
        /* Point pcAttrib to a string that describes the file. */
        if (fno.fattrib & AM_DIR) {
            pcAttrib = pcDirectory;
        } else if (fno.fattrib & AM_RDO) {
            pcAttrib = pcReadOnlyFile;
        } else {
            pcAttrib = pcWritableFile;
        }
        /* Create a string that includes the file name, the file size and the
         attributes string. */
        printf("%s [%s] [size=%llu]\n", fno.fname, pcAttrib, fno.fsize);

        fr = f_findnext(&dj, &fno); /* Search for next item */
    }
    f_closedir(&dj);
}

unsigned int howManyFilesInDir(const char *dir) {
    FRESULT fr; /* Return value */
    char const *p_dir  = dir;
    DIR dj;      /* Directory object */
    FILINFO fno; /* File information */
    memset(&dj, 0, sizeof dj);
    memset(&fno, 0, sizeof fno);
    fr = f_findfirst(&dj, &fno, p_dir, "*");
    if (FR_OK != fr) {
        panic("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    unsigned int filNum = 0;
    while (fr == FR_OK && fno.fname[0]) {
        if (fno.fattrib & AM_DIR) {
            fr = f_findnext(&dj, &fno); /* Search for next item */
            continue;
        }
        filNum++;
        fr = f_findnext(&dj, &fno); /* Search for next item */
    }
    f_closedir(&dj);
    return filNum;
}


FILINFO getNthFile(const char *dir, unsigned int requested) {
    FRESULT fr; /* Return value */
    char const *p_dir = dir;
    DIR dj;      /* Directory object */
    FILINFO fno; /* File information */
    memset(&dj, 0, sizeof dj);
    memset(&fno, 0, sizeof fno);
    fr = f_findfirst(&dj, &fno, p_dir, "*");
    if (FR_OK != fr) {
        panic("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    unsigned int filNum = 0;
    while (fr == FR_OK && fno.fname[0]) {
        if (fno.fattrib & AM_DIR) {
            fr = f_findnext(&dj, &fno); /* Search for next item */
            continue;
        }
        if (filNum == requested) {
            f_closedir(&dj);
            return fno;
        }
        filNum++;
        fr = f_findnext(&dj, &fno); /* Search for next item */
    }
    f_closedir(&dj);
}

bool sdTest(void)
{
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if(FR_OK != fr) {
        return false;
    }
    else {
        f_unmount(pSD->pcName);
        return true;
    }
}

static void setPathIndex(uint32_t index)
{
    FRESULT fr; /* Return value */
    FIL fil;
    UINT bw;

    fr =  f_open(&fil, "index.txt", FA_OPEN_ALWAYS | FA_WRITE);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("setPathIndex open error\n");
        return;
    }
    f_write (&fil, &index, sizeof(uint32_t),	&bw);
    printf("set index is %lu\n", index);

    f_close(&fil);
}

uint32_t getPathIndex(void)
{
    uint32_t index;
    FRESULT fr;
    FIL fil;
    unsigned int br;

    fr =  f_open(&fil, "index.txt", FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("getPathIndex open error\n");
        return 0;
    }
    f_read(&fil, &index, sizeof(uint32_t),	&br);
    unsigned int this_many = howManyFilesInDir("pic/");
    if (index > this_many) {
        index = 0;
    }
    printf("get index is %lu\n", index);
    
    f_close(&fil);
    
    return index;
}

uint32_t setFilePath(void)
{
    uint32_t index = 0;

    if(fileExists("index.txt")) {
        printf("index.txt exists\n");
        index = getPathIndex();
    }
    char* base = "pic/";
    FILINFO file_info = getNthFile(base, index);
    if (disPath == NULL) {
        disPath = (char*)calloc(255,1);
    }
    strcpy(disPath, base);
    strcat(disPath, file_info.fname);
    printf("setFilePath is %s \n", disPath);
    return index;
}

void updatePathIndex(uint32_t index)
{
    index++;
    setPathIndex(index);
}

bool fileExists(const char *path)
{
    FRESULT fr; /* Return value */
    FIL fil;

    fr =  f_open(&fil, path, FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("%s doesn't exist\n", path);
        return 0;
    }
    
    f_close(&fil);

    return 1;
}

