/*
*   patches.c
*       by Reisyukaku, CrimsonMaple
*   Copyright (c) 2015 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#include "patches.h"
#include "memory.h"
#include "fs.h"

//Offsets to redirect to thread code
void patchSigChecks(u8 *pos, u32 size){
    const u8 pattern[] = {0xC0, 0x1C, 0x76, 0xE7},
             pattern2[] = {0xB5, 0x22, 0x4D, 0x0C};

    u16 *off = (u16 *)memsearch(pos, pattern, size, sizeof(pattern));
    u8 *tmp = memsearch(pos, pattern2, size, sizeof(pattern2));
    
    u16 *off2 = (u16 *)(tmp - 1);
    *off = off2[0] = 0x2000;
    off2[1] = 0x4770;
}

//Offset to exe: protocol
void patchFirmWrite(u8 *pos, u32 size){
    const u8 pattern[] = {0x00, 0x28, 0x01, 0xDA};
    uPtr exe = memsearch(pos, "exe:", size, 4);

    u16 *off = (u16 *)memsearch(exe - 0x100, pattern, 0x100, sizeof(pattern));
    
    off[0] = 0x2000;
    off[1] = 0x46C0;
}


u8 *getProcess9Info(u8 *pos, u32 size, u32 *process9Size, u32 *process9MemAddr){
	u8 *temp = memsearch(pos, "NCCH", size, 4);

	if (temp == NULL) debugWrite("/rei/debug.log", "Failed to get Process 9 Info. ", 30);

	Cxi *off = (Cxi *)(temp - 0x100);

	*process9Size = (off->ncch.exeFsSize - 1) * 0x200;
	*process9MemAddr = off->exHeader.systemControlInfo.textCodeSet.address;

	return (u8*)off + (off->ncch.exeFsOffset + 1) * 0x200;
}

u32 patchFirmlaunches(u8 *pos, u32 size, u32 process9MemAddr, u16 path[], const u8 *reboot, u32 rebootSize){
	u32 pathLen = sizeof(path);
	
	for (pathLen = 0; pathLen < 41 && path[pathLen] != 0; pathLen++);

	if (path[pathLen] != 0) return 1;

	//Look for firmlaunch code
	const u8 pattern[] = {0xE2, 0x20, 0x20, 0x90};

	u8 *off = memsearch(pos, pattern, size, sizeof(pattern));

	if (off == NULL)
        shutdown();

	off -= 0x13;

	//Firmlaunch function offset - offset in BLX opcode (A4-16 - ARM DDI 0100E) + 1
	u32 fOpenOffset = (u32)(off + 9 - (-((*(u32 *)off & 0x00FFFFFF) << 2) & (0xFFFFFF << 2)) - pos + process9MemAddr);

	//Copy firmlaunch code
	memcpy(off, reboot, rebootSize);

	//Put the fOpen offset in the right location
	u32 *pos_fopen = (u32 *)memsearch(off, "OPEN", rebootSize, 4);
	*pos_fopen = fOpenOffset;

	u16 *fname = (u16*)memsearch(off, "FILE", rebootSize, 4);
	memcpy(fname, path, 2 * (1 + pathLen));

	return 0;
}

int injectBackdoor(firmHeader* firm){
    const u8 pattern[] = {0x00, 0xB0, 0x9C, 0xE5};
    u32 *tmpk11Free = NULL, *exceptionsPage = NULL, *svcTable = NULL, *baseK11VA = NULL;
	u8  *k11Free = NULL,
        *sect_arm11 = (u8*)firm + firm->section[1].offset;
    
    *exceptionsPage = (u32*)memsearch(sect_arm11, firm->section[1].size, pattern, 4) - 0xB;
    if(exceptionsPage == NULL) return 0; //Failed to get k11 info

    u32 svcOffset = (-(((exceptionsPage)[2] & 0xFFFFFF) << 2) & (0xFFFFFF << 2)) - 8;  // Branch offset + 8 for prefetch

    *baseK11VA = (0xFFFF0008 - svcOffset) & 0xFFFF0000; //This assumes that the pointed instruction has an offset < 0x10000, iirc that's always the case
    svcTable = (u32*)(sect_arm11 + *(u32*)(sect_arm11 + (0xFFFF0008 - svcOffset) - *baseK11VA + 8) - *baseK11VA); //Handler address
        
    while (*svcTable) svcTable++; //Look for SVC0

    for(*tmpk11Free = exceptionsPage; tmpk11Free < *exceptionsPage + 0x400 && *tmpk11Free != 0xFFFFFFFF; tmpk11Free++);
		
    *k11Free = (u8*)tmpk11Free;
	
    if(svcTable[0x7B] = 0){
		const u8 backdoor[] = {0xFF, 0x10, 0xCD, 0xE3, 0x0F, 0x1C, 0x81, 0xE3, 0x28, 0x10, 0x81, 0xE2, 0x00, 0x20, 0x91, 0xE5, 0x00, 0x60, 0x22, 0xE9, 0x02, 0xD0, 0xA0, 0xE1, 0x30, 0xFF, 0x2F, 0xE1, 0x03, 0x00, 0xBD, 0xE8, 0x00, 0xD0, 0xA0, 0xE1, 0x11, 0xFF, 0x2F, 0xE1};
        memcpy(*k11Free, backdoor, sizeof(backdoor));
        svcTable[0x7B] = baseK11VA + *k11Free - *sect_arm11;
        *k11Free += sizeof(backdoor);
    }
    return 0;
}

void getLoader(const void *pos, Size *ldrSize, uPtr *ldrOff){
    u8 *off = (u8*)pos;
    Size s;

    while(1){
        s = *(u32*)(off + 0x104) * 0x200;
        if(*(u32*)(off + 0x200) == 0x64616F6C) break;
        off += s;
    }

    *ldrSize = s;
    *ldrOff = (uPtr)(off - (u8*)pos);
}