#include "run_File.h"
#include "f_util.h"
#include "hw_config.h"

#include <stdio.h>
#include <string.h>

char disPath[265] = "pic/";

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


int howManyFilesInDir(const char *dir) {
    FRESULT fr;
    DIR dp;
    FILINFO fno; /* File information */
    memset(&fno, 0, sizeof fno);
    f_opendir(&dp, dir);
    fr = f_readdir(&dp, &fno);
    int filNum = 0;
    while (fr == FR_OK && fno.fname[0]) {
        if (!(fno.fattrib & AM_DIR)) {
            filNum++;
        }
        fr = f_readdir(&dp, &fno);
    }
    f_closedir(&dp);
    return filNum;
}


unsigned int getNthFile(FILINFO *fno, unsigned int requested) {
    unsigned int filNum = 0;
    FRESULT fr;
    DIR dp;      /* Directory object */
    f_opendir(&dp, "pic/");
    fr = f_readdir(&dp, fno);
    while (fr == FR_OK && fno->fname[0]) {
        if (!(fno->fattrib & AM_DIR)) {
            if (filNum == requested) {
                f_closedir(&dp);
                return ++requested;
            }
            filNum++;
        }
        fr = f_readdir(&dp, fno);
    }
    f_closedir(&dp);
    if (requested == 0) {
        printf("It seems there are no files\n");
        return 0; 
    }
    return getNthFile(fno, 0);
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

void setPathIndex(uint32_t index)
{
    FIL fil;
    UINT bw;

    printf("setting index to %lu\n", index);
    FRESULT fr =  f_open(&fil, "index.dat", FA_OPEN_ALWAYS | FA_WRITE);
    if(FR_OK != fr && FR_EXIST != fr) {
        panic("f_open() error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    f_write(&fil, &index, sizeof(uint32_t), &bw);
    f_close(&fil);
}

uint32_t getPathIndex(void)
{
    uint32_t index;
    FIL fil;
    unsigned int br;

    FRESULT fr =  f_open(&fil, "index.dat", FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("index.dat doesn't exist\n");
        return 0;
    }
    f_read(&fil, &index, sizeof(uint32_t),	&br);
    printf("got index %lu\n", index);
    f_close(&fil);
    return index;
}

uint32_t setFilePath(void)
{
    uint32_t index = getPathIndex();
    FILINFO fno; /* File information */
    memset(&fno, 0, sizeof fno);
    index = getNthFile(&fno, index);
    strcat(disPath, fno.fname);
    printf("disPath set to %s \n", disPath);
    return index;
}
