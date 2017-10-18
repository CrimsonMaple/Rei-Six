/*
*   patches.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#ifndef PATCHES_INC
#define PATCHES_INC

#include "types.h"

void patchSigChecks(u8 *pos, u32 size);
void patchFirmWrite(u8 *pos, u32 size);
u8 *getProcess9Info(u8 *pos, u32 size, u32 *process9Size, u32 *process9MemAddr);
u32 patchFirmlaunches(u8 *pos, u32 size, u32 process9MemAddr, u16 path[], const u8 *reboot, u32 rebootSize);
void getLoader(const void *pos, Size *ldrSize, uPtr *ldrOff);
int injectBackdoor(firmHeader *firm);

#endif