/*
*   lcd.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "lcd.h"
#include "i2c.h"

vu32 *arm11Entry = (vu32*)0x1FFFFFFC;

void shutdownLCD(void){
    void __attribute__((naked)) ARM11(void){
        //Disable interrupts
        __asm(".word 0xF10C01C0");
        
        //Clear ARM11 entry offset and wait for ARM11 to set it, and jumps
        *arm11Entry = 0;
        
        //Clear LCDs
        *(vu32*)0x10202A44 = 0;
        *(vu32*)0x10202244 = 0;
        *(vu32*)0x10202014 = 0;
        
        while (!*arm11Entry);
        ((void (*)())*arm11Entry)();
    }
    if(GPU_PDN_CNT != 1){
        *arm11Entry = (u32)ARM11;
        while(*arm11Entry);
    }
}

void set_brightness(int brightness_level){
    void __attribute__((naked)) ARM11(void){
        __asm(".word 0xF10C01C0");
        
        *arm11Entry = 0;
        
        static const u32 brightness[4] = {0x5F, 0x4C, 0x39, 0x26};
        
        *(vu32*)0x10202240 = brightness[brightness_level];
		*(vu32*)0x10202A40 = brightness[brightness_level];
        
        while(!*arm11Entry);
        ((void (*)())*arm11Entry)();
    }
    if(GPU_PDN_CNT != 1){
        *arm11Entry = (u32)ARM11;
        while(*arm11Entry);
    }
}

#define WAIT_FOR_ARM9() *arm11Entry = 0; while(!*arm11Entry); ((void (*)())*arm11Entry)();

void  __attribute__((naked)) arm11Stub(void)
{
	WAIT_FOR_ARM9();
}

static void invokeArm11Function(void(*func)())
{
	static bool hasCopiedStub = false;

	if (!hasCopiedStub)
	{
		memcpy((void *)0x1FFFFF00, (void *)arm11Stub, 0x2C);
		hasCopiedStub = true;
	}

	*arm11Entry = (u32)func;
	while (*arm11Entry);
	*arm11Entry = 0x1FFFFF00;
	while (*arm11Entry);
}

void initScreen(void){
    void __attribute__((naked)) initSequence(void)
	{
		//Disable interrupts
		__asm(".word 0xF10C01C0");

		*(vu32 *)0x10141200 = 0x1007F;
		*(vu32 *)0x10202014 = 0x00000001;
		*(vu32 *)0x1020200C &= 0xFFFEFFFE;
		*(vu32 *)0x10202240 = 0x5F; //Lowest Brightness Level.
		*(vu32 *)0x10202A40 = 0x5F;
		*(vu32 *)0x10202244 = 0x1023E;
		*(vu32 *)0x10202A44 = 0x1023E;

		//Top screen
		*(vu32 *)0x10400400 = 0x000001c2;
		*(vu32 *)0x10400404 = 0x000000d1;
		*(vu32 *)0x10400408 = 0x000001c1;
		*(vu32 *)0x1040040c = 0x000001c1;
		*(vu32 *)0x10400410 = 0x00000000;
		*(vu32 *)0x10400414 = 0x000000cf;
		*(vu32 *)0x10400418 = 0x000000d1;
		*(vu32 *)0x1040041c = 0x01c501c1;
		*(vu32 *)0x10400420 = 0x00010000;
		*(vu32 *)0x10400424 = 0x0000019d;
		*(vu32 *)0x10400428 = 0x00000002;
		*(vu32 *)0x1040042c = 0x00000192;
		*(vu32 *)0x10400430 = 0x00000192;
		*(vu32 *)0x10400434 = 0x00000192;
		*(vu32 *)0x10400438 = 0x00000001;
		*(vu32 *)0x1040043c = 0x00000002;
		*(vu32 *)0x10400440 = 0x01960192;
		*(vu32 *)0x10400444 = 0x00000000;
		*(vu32 *)0x10400448 = 0x00000000;
		*(vu32 *)0x1040045C = 0x00f00190;
		*(vu32 *)0x10400460 = 0x01c100d1;
		*(vu32 *)0x10400464 = 0x01920002;
		*(vu32 *)0x10400468 = 0x18300000;
		*(vu32 *)0x10400470 = 0x80341;
		*(vu32 *)0x10400474 = 0x00010501;
		*(vu32 *)0x10400478 = 0;
		*(vu32 *)0x10400490 = 0x000002D0;
		*(vu32 *)0x1040049C = 0x00000000;

		//Disco register
		for (u32 i = 0; i < 256; i++)
			*(vu32 *)0x10400484 = 0x10101 * i;

		//Bottom screen
		*(vu32 *)0x10400500 = 0x000001c2;
		*(vu32 *)0x10400504 = 0x000000d1;
		*(vu32 *)0x10400508 = 0x000001c1;
		*(vu32 *)0x1040050c = 0x000001c1;
		*(vu32 *)0x10400510 = 0x000000cd;
		*(vu32 *)0x10400514 = 0x000000cf;
		*(vu32 *)0x10400518 = 0x000000d1;
		*(vu32 *)0x1040051c = 0x01c501c1;
		*(vu32 *)0x10400520 = 0x00010000;
		*(vu32 *)0x10400524 = 0x0000019d;
		*(vu32 *)0x10400528 = 0x00000052;
		*(vu32 *)0x1040052c = 0x00000192;
		*(vu32 *)0x10400530 = 0x00000192;
		*(vu32 *)0x10400534 = 0x0000004f;
		*(vu32 *)0x10400538 = 0x00000050;
		*(vu32 *)0x1040053c = 0x00000052;
		*(vu32 *)0x10400540 = 0x01980194;
		*(vu32 *)0x10400544 = 0x00000000;
		*(vu32 *)0x10400548 = 0x00000011;
		*(vu32 *)0x1040055C = 0x00f00140;
		*(vu32 *)0x10400560 = 0x01c100d1;
		*(vu32 *)0x10400564 = 0x01920052;
		*(vu32 *)0x10400568 = 0x18300000 + 0x46500;
		*(vu32 *)0x10400570 = 0x80301;
		*(vu32 *)0x10400574 = 0x00010501;
		*(vu32 *)0x10400578 = 0;
		*(vu32 *)0x10400590 = 0x000002D0;
		*(vu32 *)0x1040059C = 0x00000000;

		//Disco register
		for (u32 i = 0; i < 256; i++)
			*(vu32 *)0x10400584 = 0x10101 * i;

		WAIT_FOR_ARM9();
	}

	static bool needToSetup = true;

	if (needToSetup){
		if (GPU_PDN_CNT != 1){
			invokeArm11Function(initSequence);

			//Turn on backlight
			i2cWriteRegister(I2C_DEV_MCU, 0x22, 0x2A);
		}
		needToSetup = false;
	}
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