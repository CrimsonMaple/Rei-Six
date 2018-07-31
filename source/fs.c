/*
*   fc.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#include <stddef.h>
#include "fs.h"
#include "fatfs/ff.h"
#include "i2c.h"
#include "caches.h"
#include "string.h"
#include "draw.h"

static FATFS sdfs, nandfs;
static FIL fp; //Had to make a static file since fatfs hated my file pointers.

void mountSD(void){
    if (f_mount(&sdfs, "0:", 1) != FR_OK)
        shutdown();
}

void mountNand(void){
    if (f_mount(&nandfs, "1:", 1) != FR_OK)
        shutdown();
}

void unmountSD(void){
    if (f_mount(NULL, "0:", 1) != FR_OK)
        shutdown();
}

void unmountNand(void){
    if(f_mount(NULL, "1:", 1) != FR_OK)
        shutdown();
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

u32 fstat(const void *filename){
    FILINFO info;
    if(f_stat(filename, &info) != FR_OK)
        return 0;
    else
        return 1;
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

void debugWrite(const char *filename, char *buffer, Size size){ // Could also dump memory regions. Just pass an address as a buffer.
    fopen(filename, "wb");
    fwrite(buffer, 1, size);
    fclose();
}

void shutdown(void){
    i2cWriteRegister(I2C_DEV_MCU, 0x22, 1 << 0); // Poweroff LCD to prevent MCU hangs
    flushEntireDCache();
    flushEntireICache();
    if (i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0))
        while (true);
}