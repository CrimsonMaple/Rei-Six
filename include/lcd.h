/*
*   lcd.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef LCD_INC
#define LCD_INC

#include "types.h"

#define GPU_PDN_CNT (*(vu8*)0x10141200)

static struct fb {
    u8 *top_left;
    u8 *top_right;
    u8 *bottom;
} __attribute__((packed));

extern struct fb fbs[2];

void shutdownLCD(void);
void initScreen(void);
#endif