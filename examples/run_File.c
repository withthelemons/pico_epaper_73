#include "run_File.h"
#include "f_util.h"
#include "pico/stdlib.h"
#include "hw_config.h"

#include <stdio.h>
#include <stdlib.h> // malloc() free()
#include <string.h>

const char *fileList = "fileList.txt";
const char *fileListNew = "fileListNew.txt";
char pathName[fileNumber][fileLen];
int scanFileNum = 0;
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

static void run_cat(const char *path) {
    // char *arg1 = strtok(NULL, " ");
    if (!path) {
        printf("Missing argument\n");
        return;
    }
    FIL fil;
    FRESULT fr = f_open(&fil, path, FA_READ);
    if (FR_OK != fr) {
        printf("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    char buf[256];
    while (f_gets(buf, sizeof buf, &fil)) {
        printf("%s", buf);
    }
    fr = f_close(&fil);
    if (FR_OK != fr) printf("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
}

static bool isSameFile(const char *list, const char *listNew)
{
    if (!list) {
        printf("Missing argument 1\n");
        return false;
    }
    if (!listNew) {
        printf("Missing argument 2\n");
        return false;
    }
    FIL fil, filNew;
    FRESULT fr = f_open(&fil, list, FA_READ);
    if (FR_OK != fr) {
        printf("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }
    FRESULT frNew = f_open(&filNew, listNew, FA_READ);
    if (FR_OK != frNew) {
        printf("f_open error: %s (%d)\n", FRESULT_str(frNew), frNew);
        return false;
    }

    if(f_size(&fil) != f_size(&filNew)) {
        printf("%s is %d, %s is %d\n", list, f_size(&fil), listNew, f_size(&filNew));
        printf("size is different\n");
        return false;
    }

    UINT br;
    char * templist;
    char * templistnew;

    templist = (char *)malloc(fileNumber*fileLen*sizeof(char) + 1);
    templistnew = (char *)malloc(fileNumber*fileLen*sizeof(char) + 1);

    f_read(&fil, templist, f_size(&fil), &br);
    f_read(&filNew, templistnew, f_size(&filNew), &br);

    if(memcmp(templist, templistnew, (f_size(&fil) < f_size(&filNew)) ? f_size(&fil) : f_size(&filNew)) == 0) {
        free(templist);
        free(templistnew);
        f_close(&fil);
        f_close(&filNew);
        printf("data is same\n");
        return true;
    }
    else {
        free(templist);
        free(templistnew);
        f_close(&fil);
        f_close(&filNew);
        printf("data is different\n");
        return false;
    }
}

void ls(const char *dir) {
    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr; /* Return value */
    char const *p_dir;
    if (dir[0]) {
        p_dir = dir;
    } else {
        fr = f_getcwd(cwdbuf, sizeof cwdbuf);
        if (FR_OK != fr) {
            printf("f_getcwd error: %s (%d)\n", FRESULT_str(fr), fr);
            return;
        }
        p_dir = cwdbuf;
    }
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

void ls2file(const char *dir, const char *path) {
    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr; /* Return value */
    char const *p_dir;
    if (dir[0]) {
        p_dir = dir;
    } else {
        fr = f_getcwd(cwdbuf, sizeof cwdbuf);
        if (FR_OK != fr) {
            printf("f_getcwd error: %s (%d)\n", FRESULT_str(fr), fr);
            return;
        }
        p_dir = cwdbuf;
    }
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

    int filNum=0;
    FIL fil;
    fr =  f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE);
    if(FR_OK != fr && FR_EXIST != fr)
        panic("f_open(%s) error: %s (%d) \n", path, FRESULT_str(fr), fr);
    // f_printf(&fil, "{");
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
        f_printf(&fil, "pic/%s\n", fno.fname);
        filNum++;
        fr = f_findnext(&dj, &fno); /* Search for next item */
    }
    // f_printf(&fil, "}");
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    f_closedir(&dj);
}

void sdInitTest(void)
{
    puts("Hello, world!");

    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr) panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    FIL fil;
    const char* const filename = "filename.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    if (f_printf(&fil, "Hello, world!\n") < 0) {
        printf("f_printf failed\n");
    }
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    f_unmount(pSD->pcName);

    puts("Goodbye, world!");
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

void sdScanDir(void)
{   
    run_mount();

    ls2file("0:/pic", fileList);
    printf("ls %s\n", fileList);
    run_cat(fileList);

    run_unmount();
}

void sdScanDirExist(void)
{
    FRESULT fr; /* Return value */

    run_mount();

    ls2file("0:/pic", fileListNew);
    printf("ls %s\n", fileListNew);
    run_cat(fileListNew);

    if(!isSameFile(fileList, fileListNew)) {
        printf("Different fileList\n");
        printf("remove fileList\n");
        fr = f_unlink(fileList);
        if (FR_OK != fr) {
            printf("f_unlink error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        printf("rename fileListNew to fileList\n");
        fr = f_rename(fileListNew, fileList);
        if (FR_OK != fr) {
            printf("f_rename error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }
    else {
        printf("Same fileList\n");
        printf("remove fileListNew\n");
        fr = f_unlink(fileListNew);
            if (FR_OK != fr) {
            printf("f_unlink error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }

    run_unmount();
}

void fil2array(void)
{
    printf("fil2array start\n");
    run_mount();

    FRESULT fr; /* Return value */
    FIL fil;

    fr =  f_open(&fil, fileList, FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("fil2array open error\n");
        run_unmount();
        return;
    }

    // printf("ls array path\n");
    for(int i=0; i<fileNumber; i++) {
        if(f_gets(pathName[i], 999, &fil) == NULL) {
            scanFileNum = i;
            break;
        }
        // printf("%s", pathName[i]);
    }

    f_close(&fil);
    run_unmount();
    printf("fil2array end\n");
}

static void setPathIndex(int index)
{
    FRESULT fr; /* Return value */
    FIL fil;
    UINT br;

    run_mount();

    fr =  f_open(&fil, "index.txt", FA_OPEN_ALWAYS | FA_WRITE);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("setPathIndex open error\n");
        run_unmount();
        return;
    }
    f_printf(&fil, "%d\n", index);
    printf("set index is %d\n", index);

    f_close(&fil);
    run_unmount();
}

static int getPathIndex(void)
{
    int index = 0;
    char indexs[10];
    FRESULT fr; /* Return value */
    FIL fil;

    run_mount();

    fr =  f_open(&fil, "index.txt", FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("getPathIndex open error\n");
        run_unmount();
        return 0;
    }
    f_gets(indexs, 10, &fil);
    sscanf(indexs, "%d", &index);   // char to int
    if(index >= scanFileNum) {
        index = 0;
        printf("get index over scanFileNum\n");
    }
    printf("get index is %d\n", index);
    
    f_close(&fil);
    run_unmount();
    
    return index;
}

void setFilePath(void)
{
    int index = 0;

    fil2array();
    if(fileExists("index.txt")) {
        printf("index.txt exists\n");
        index = getPathIndex();
    }
    else {
        printf("creat and set Index 0\n");
        setPathIndex(0);
    }
    disPath = pathName[index];
    printf("setFilePath is %s", disPath);
}

void updatePathIndex(void)
{
    int index = 0;
    index = getPathIndex();
    index++;
    if(index >= fileNumber)
        index = 0;
    setPathIndex(index);
    printf("updated path index index\n");
}

bool fileExists(const char *path)
{
    FRESULT fr; /* Return value */
    FIL fil;

    run_mount();

    fr =  f_open(&fil, path, FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("%s doesn't exist\n", path);
        run_unmount();
        return 0;
    }
    
    f_close(&fil);
    run_unmount();

    return 1;
}

