/*
*   draw.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "draw.h"
#include "fs.h"
#include "memory.h"
#include "lcd.h"

u8 clearScreen(void){
    //Only clear screens if they are initialized
    if(GPU_PDN_CNT == 1) return 0;
        memset(fbs->top_left, 0, 0x38400);
        memset(fbs->top_right, 0, 0x38400);
        memset(fbs->bottom, 0, 0x38400);
    return 1;
}

void loadSplash(void){
    //Initialize screen, because fuck b9s doing it for me.
    initScreen();
    //If FB was clear, and the image exists, display
    if(clearScreen()){
        if(fopen("/rei/top_splash.bin", "rb")){
            fread(fbs->top_left, 1, fsize());
            fclose();
        }
        if(fopen("/rei/bottom_splash.bin", "rb")){
            fread(fbs->bottom, 1, fsize());
            fclose();
        }
        u64 i = 0xFFFFFF; while(--i) __asm("mov r0, r0"); //Less Ghetto sleep func
    }
}