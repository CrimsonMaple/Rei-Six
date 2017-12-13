/*
*   draw.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "draw.h"
#include "fs.h"
#include "memory.h"
#include "lcd.h"

void loadSplash(void){
    //Initialize screens.
    initScreen();
    //If FB was clear, and the image exists, display
    clearScreen();
    if(fopen("/rei/top_splash.bin", "rb")){
        fread(fbs->top_left, 1, fsize());
        fclose();
    }
    if(fopen("/rei/bottom_splash.bin", "rb")){
        fread(fbs->bottom, 1, fsize());
        fclose();
    }
    //Less ghetto sleep function
    u64 i = 0xFFFFFF; while(--i) __asm("mov r0, r0");
}