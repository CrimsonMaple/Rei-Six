/*
*   patches.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef PATCHES_INC
#define PATCHES_INC

#include "types.h"

void getSigChecks(const void *pos, Size size, uPtr *off, uPtr *off2);
void getFirmWrite(const void *pos, Size size, uPtr *off);
u8 *getProcess9Info(u8 *pos, u32 size, u32 *process9Size, u32 *process9MemAddr);
u32 patchFirmlaunches(u8 *pos, u32 size, u32 process9MemAddr, u16 path[], const u8 *reboot, u32 rebootSize);
void getLoader(const void *pos, Size *ldrSize, uPtr *ldrOff);
int injectBackdoor(firmHeader *firm);
u32 patchLgySignatureChecks(u8 *pos, u32 size);
u32 patchTwlInvalidSignatureChecks(u8 *pos, u32 size);
u32 patchTwlNintendoLogoChecks(u8 *pos, u32 size);
u32 patchTwlWhitelistChecks(u8 *pos, u32 size);
u32 patchTwlFlashcartChecks(u8 *pos, u32 size);
u32 patchTwlShaHashChecks(u8 *pos, u32 size);

#endif