/*
*   emunand.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef EMU_INC
#define EMU_INC

#include "types.h"

#define NCSD_MAGIC (0x4453434E)
#define partitionStart (*(u32 *)(temp + 0x1C6))

void getEmunand(boottype boot_type);
static u32 getSDMMC(u8 *pos, u32 size, u32 *sd_struct);
void patchEmuRW(u8 *pos, u32 size, u32 branchOff);
void patchMPU(u8 *pos, u32 size);
void patchEmuCode(u8 *sect_arm9, u32 size_k9, u8 *offset_p9, u32 size_p9, u8 *addr_k9);

#endif