/*
*   lcd.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef LCD_INC
#define LCD_INC

#include "types.h"

#define GPU_PDN_CNT (*(vu8*)0x10141200)

#define ARM11_PARAMETERS_ADDRESS 0x1FFFF000

static struct fb {
    u8 *top_left;
    u8 *top_right;
    u8 *bottom;
};

typedef enum {
    INIT_SCREENS = 0,
    SETUP_FRAMEBUFFERS,
    CLEAR_SCREENS,
    SWAP_FRAMEBUFFERS,
    DEINIT_SCREENS,
    PREPARE_ARM11_FOR_FIRMLAUNCH,
    ARM11_READY,
} Arm11Op;

extern struct fb fbs[2];

void prepareFirmlaunch(void);
void shutdownLCD(void);
void clearScreen(void);
void initScreen(void);
#endif