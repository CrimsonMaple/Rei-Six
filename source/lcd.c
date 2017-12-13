/*
*   lcd.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#include "lcd.h"
#include "i2c.h"

static volatile Arm11Op *operation = (volatile Arm11Op*)0x1FF80004;

static void invokeArm11Function(Arm11Op op){
    while(*operation != ARM11_READY);
    *operation = op;
    while(*operation != ARM11_READY); 
}

void prepareFirmlaunch(void){
    invokeArm11Function(PREPARE_ARM11_FOR_FIRMLAUNCH);
}

void shutdownLCD(void){
    if(GPU_PDN_CNT == 1) invokeArm11Function(DEINIT_SCREENS);
}

void clearScreen(void){
    for(int i = 0; i < 2; i++){
        struct fb *fbTemp = &fbs[i];
        *(volatile struct fb*)ARM11_PARAMETERS_ADDRESS = *fbTemp;
        invokeArm11Function(CLEAR_SCREENS);
    }
}

void swapFramebuffers(bool isAlternate){
    *(volatile bool*)ARM11_PARAMETERS_ADDRESS = isAlternate;
    invokeArm11Function(SWAP_FRAMEBUFFERS);
}

void initScreen(void){
	if (GPU_PDN_CNT != 1){
        *(vu32*)ARM11_PARAMETERS_ADDRESS = 0x5F;
        memcpy((void*)(ARM11_PARAMETERS_ADDRESS + 4), fbs, sizeof(fbs));
        invokeArm11Function(INIT_SCREENS);

        //Turn on backlight
        i2cWriteRegister(I2C_DEV_MCU, 0x22, 0x2A);
	}

    memcpy((void*)ARM11_PARAMETERS_ADDRESS, fbs, sizeof(fbs));
    invokeArm11Function(SETUP_FRAMEBUFFERS);
    
    clearScreen();
    swapFramebuffers(false);
}

struct fb fbs[2] = {
    {
        .top_left  = (u8 *)0x18300000,
        .top_right = (u8 *)0x18300000,
        .bottom    = (u8 *)0x18346500,
    },
    {
        .top_left  = (u8 *)0x18400000,
        .top_right = (u8 *)0x18400000,
        .bottom    = (u8 *)0x18446500,
    },
};