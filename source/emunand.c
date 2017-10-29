/*
*   emunand.c
*       by Reisyukaku, CrimsonMaple
*   Copyright (c) 2015 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#include "fs.h"
#include "emunand.h"
#include "memory.h"
#include "fatfs/ff.h"
#include "fatfs/sdmmc/sdmmc.h"
#include "../build/payloads.h"

u32 emuOffset, emuHeader;

void getEmunand(boottype boot_type){
    Size nandSize = getMMCDevice(0)->total_size;
    static u8 __attribute__((aligned(4))) temp[0x200];
    
    sdmmc_sdcard_readsectors(0, 1, temp);
    
    for(u32 i = 0; i < 3; i++){
        static const u32 rndSize[] = { 0x1D8000, 0x25E000 };
        u32 nandOffset;
        
        switch(i){
            case 0:
                nandOffset = boot_type == EMUNAND ? 0 : (nandSize > 0x200000 ? 0x400000 : 0x200000);
                break; // "Legacy"
            case 1:
                nandOffset = (((nandSize + 1) + 0x2000 - 1) & (~(0x2000 - 1)));
                break; // "Default"
            case 2:
                nandOffset = rndSize[ISN3DS ? 1 : 0];
                break; // "Min. Size"
        }
    
        if(partitionStart >= nandOffset + rndSize[ISN3DS ? 1 : 0]){
            //Check for RedNAND
            if(!sdmmc_sdcard_readsectors(nandOffset + 1, 1, temp) && memcmp(temp + 0x100, "NCSD", 4) == 0){
                emuOffset = nandOffset + 1;
                emuHeader = nandOffset + 1;
                return;
            }

            //Check for Gateway EmuNAND
            else if(i != 2 && !sdmmc_sdcard_readsectors(nandOffset + nandSize, 1, temp) && memcmp(temp + 0x100, "NCSD", 4) == 0){
                emuOffset = nandOffset;
                emuHeader = nandOffset + nandSize;
                return;
            }
        }
    }
}

static u32 getSDMMC(u8 *pos, u32 size, u32 *sd_struct){
    //Look for struct code
    const u8 pattern[] = {0x21, 0x20, 0x18, 0x20};
    const u8 *off = memsearch(pos, pattern, size, sizeof(pattern));

    *sd_struct = *(u32 *)(off + 9) + *(u32 *)(off + 0xD);
    
    return 0;
}


void patchEmuRW(u8 *pos, u32 size, u32 branchOff){
    //Look for read/write code
    const u8 pattern[] = {0x1E, 0x00, 0xC8, 0x05};
    
    u16 *readOff = (u16*)memsearch(pos, pattern, size, sizeof(pattern)) - 3;
    u16 *writeOff = (u16*)memsearch((u8*)(readOff + 5), pattern, 0x100, sizeof(pattern)) - 3;
    
    *readOff = *writeOff = 0x4C00;
    readOff[1] = writeOff[1] = 0x47A0;
    ((u32*)writeOff)[1] = ((u32*)readOff)[1] = branchOff;
}

void patchMPU(u8 *pos, u32 size){
    //Look for MPU pattern
    const u8 pattern[] = {0x03, 0x00, 0x24, 0x00};
    u16 *off = (u16 *)memsearch(pos, pattern, size, sizeof(pattern));
    
    off[1] = 0x0036;
    off[0xC] = off[0x12] = 0x0603;
}

void patchEmuCode(u8 *sect_arm9, u32 size_k9, u8 *offset_p9, u32 size_p9, u8 *addr_k9){
    //Finds start of 0xFF field
    const u8 pattern[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    u8 *space_k9 = memsearch(sect_arm9, pattern, size_k9, sizeof(pattern)) + 0x455;
    
    if(*space_k9 == NULL)
        shutdown();
    
    memcpy(space_k9, emunand_bin, emunand_bin_size); // Copy EmuNAND code
    
    u32 *posOffset = (u32*)memsearch(space_k9, "NAND", emunand_bin_size, 4),
        *posHeader = (u32*)memsearch(space_k9, "NCSD", emunand_bin_size, 4); // Add EmuNAND data
        
        *posOffset = emuOffset;
        *posHeader = emuHeader;

    // Get SDMMC Struct
    u32 *posSdmmc = (u32*)memsearch(space_k9, "SDMC", emunand_bin_size, 4);
    u32 sd_struct;
    int ret = getSDMMC(offset_p9, size_p9, &sd_struct);
    
    if(!ret)
        *posSdmmc = sd_struct;
    
    u32 branchOff = (u32)(space_k9 - sect_arm9 + addr_k9);
    patchEmuRW(offset_p9, size_p9, branchOff); // Patch EmuNAND read/write
    
    patchMPU(sect_arm9, size_k9); // Patch MPU
}