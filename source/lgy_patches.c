/*
*   lgy_patches.c
*       by Reisyukaku, CrimsonMaple
*   Copyright (c) 2015-2017 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#include "lgy_patches.h"
#include "memory.h"
#include "fs.h"

u32 patchLgySignatureChecks(u8 *pos, u32 size){
	const u8 pattern[] = {0x47, 0xC1, 0x17, 0x49};

	u8 *temp = memsearch(pos, pattern, size, sizeof(pattern));

	if (temp == NULL) return 1;

	u16 *off = (u16*)(temp + 1);
	off[0] = 0x2000;
	off[1] = 0xB04E;
	off[2] = 0xBD70;

	return 0;
}

u32 patchTwlInvalidSignatureChecks(u8 *pos, u32 size){
	const u8 pattern[] = {0x20, 0xF6, 0xE7, 0x7F};

	u8 *temp = memsearch(pos, pattern, size, sizeof(pattern));

	if (temp == NULL) return 1;

	u16 *off = (u16*)(temp - 1);
	*off = 0x2001; //mov r0, #1

	return 0;
}

u32 patchTwlNintendoLogoChecks(u8 *pos, u32 size){
	const u8 pattern[] = {0xC0, 0x30, 0x06, 0xF0};

	u16 *off = (u16*)memsearch(pos, pattern, size, sizeof(pattern));

	if (off == NULL) return 1;

	off[1] = 0x2000;
	off[2] = 0;

	return 0;
}

u32 patchTwlWhitelistChecks(u8 *pos, u32 size){
	const u8 pattern[] = {0x22, 0x00, 0x20, 0x30};

	u16 *off = (u16*)memsearch(pos, pattern, size, sizeof(pattern));

	if (off == NULL) return 1;

	off[2] = 0x2000;
	off[3] = 0;

	return 0;
}

u32 patchTwlFlashcartChecks(u8 *pos, u32 size){
	const u8 pattern[] = {0x25, 0x20, 0x00, 0x0E};

	u8 *temp = memsearch(pos, pattern, size, sizeof(pattern));

	if (temp == NULL)
		return 1;

	u16 *off = (u16 *)(temp + 3);
	off[0] = off[6] = off[0xC] = 0x2001; //mov r0, #1
	off[1] = off[7] = off[0xD] = 0; //nop

	return 0;
}

u32 patchTwlShaHashChecks(u8 *pos, u32 size){
	const u8 pattern[] = {0x10, 0xB5, 0x14, 0x22};

	u16 *off = (u16*)memsearch(pos, pattern, size, sizeof(pattern));

	if (off == NULL) return 1;

	off[0] = 0x2001; //mov r0, #1
	off[1] = 0x4770;

	return 0;
}