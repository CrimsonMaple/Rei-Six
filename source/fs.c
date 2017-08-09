/*
*   fc.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include <stddef.h>
#include "fs.h"
#include "fatfs/ff.h"
#include "fmt.h"

static FATFS sdfs, nandfs;
static FIL fp; //Had to make a static file since fatfs hated my file pointers.

u8 mountSD(void){
    if (f_mount(&sdfs, "0:", 1)) return 1;
    return 0;
}

u8 mountNand(void){
    if (f_mount(&nandfs, "1:", 1)) return 1;
    return 0;
}

u8 unmountSD(void){
    if (f_mount(NULL, "0:", 1)) return 1;
    return 0;
}

u8 unmountNand(void){
    if(f_mount(NULL, "1:", 1)) return 1;
    return 0;
}

u8 fopen(const void *filename, const char *mode){
    u8 res;
    if (*mode == 'r' || *mode == 'w' || *mode == 'a')
        res = f_open(&fp, filename, *mode == 'r' ? FA_READ : (FA_WRITE | FA_OPEN_ALWAYS));
    return res == 0 ? 1 : 0;
}

void fclose(void){
    f_close(&fp);
}

void fseek(u32 offset){
    f_lseek(&fp, offset);
}

u8 eof(void){
    return f_eof(&fp);
}

Size fsize(void){
    return f_size(&fp);
}

Size fwrite(const void *buffer, Size elementSize, Size elementCnt){
    UINT br;
    if(f_write(&fp, buffer, elementSize*elementCnt, &br)) return 0;
    if (br == elementSize*elementCnt) br /= elementSize; else return 0;
    return br;
}

Size fread(const void *buffer, Size elementSize, Size elementCnt){
    UINT br;
    if(f_read(&fp, buffer, elementSize*elementCnt, &br)) return 0;
    if (br == elementSize*elementCnt) br /= elementSize; else return 0;
    return br;
}


void debugWrite(const char *filename, char *buffer, Size size){
    fopen(filename, "wb");
    fwrite(buffer, 1, size);
    fclose();
}

u32 lumaFileRead(void *dest, const char *path, u32 maxSize){
    FIL file;
    u32 ret = 0;

    if(f_open(&file, path, FA_READ) != FR_OK) return ret;

    u32 size = f_size(&file);
    if(dest == NULL) ret = size;
    else if(size <= maxSize)
        f_read(&file, dest, size, (unsigned int *)&ret);
    f_close(&file);

    return ret;
}

u32 firmRead(u8 *dest, u32 firm){
    const char *firms[3][2] =  {
        {"00000002", "20000002"}, //Native Firm
        {"00000102", "20000102"}, //TWL Firm
        {"00000202", "20000202"}, //AGB Firm
    };

    char firmPath[35] = { 0 };
    char fullPath[48] = { 0 };

    sprintf(firmPath, "1:/title/00040138/%s/content", firms[firm][ISN3DS ? 1 : 0]);
    u32 firmVersion = 0xDEADBEEF;
    debugWrite("/rei/debugPath.log", firmPath, 35);

    DIR directory;
    FILINFO info = { 0 };

    if (f_opendir(&directory, firmPath) != FR_OK) goto exit;

    while (f_readdir(&directory, &info) == FR_OK && info.fname[0] != 0)
    {
        if (info.fname[9] != 'a' || strlen(info.fname) != 12) continue;

        u32 tempVer = hexAtoi(info.altname, 8);

        if (tempVer < firmVersion) firmVersion = tempVer;
    }
    
    debugWrite("/rei/predirclose", " ", 1);

    f_closedir(&directory);
    
    debugWrite("/rei/postdirclose", " ", 1);

    if (firmVersion == 0xDEADBEEF)
        goto exit;
    
    debugWrite("/rei/postgotoexit2", " ", 1);

    sprintf(fullPath, "%s/%08x.app", firmPath, firmVersion);
    debugWrite("/rei/debugFullPath.log", fullPath, 48);

    if(lumaFileRead(dest, fullPath, 0x400000 + sizeof(Cxi) + 0x200) <= sizeof(Cxi) + 0x400) firmVersion = 0xDEADBEEF;
exit:
    debugWrite("/rei/firmversion.log", (char*)firmVersion, 10);
    return firmVersion;
}
