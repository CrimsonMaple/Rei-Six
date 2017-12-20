/*
*   firm.c
*       by Reisyukaku, CrimsonMaple
*   Copyright (c) 2015-2017 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#include "firm.h"
#include "patches.h"
#include "lgy_patches.h"
#include "memory.h"
#include "fs.h"
#include "emunand.h"
#include "lcd.h"
#include "crypto.h"
#include "caches.h"
#include "../build/payloads.h"

//Firm vars
firmHeader *firm = (firmHeader*)0x20001000;
Size firmSize = 0;
    
//Patch vars
uPtr firmWriteOffset = 0,
     ldrOffset = 0;
     
static __attribute__((noinline)) bool overlaps(u32 as, u32 ae, u32 bs, u32 be){
    if(as <= bs && bs <= ae)
        return true;
    if(bs <= as && as <= be)
        return true;
    return false;
}

static __attribute__((noinline)) bool inRange(u32 as, u32 ae, u32 bs, u32 be){
   if(as >= bs && ae <= be)
        return true;
   return false;
}
     
static bool firmCheck(Size firmSize){
    bool arm9EpFound = false;
    bool arm11EpFound = false;
    
    if(memcmp(firm->magic, "FIRM", 4) !=0 || firm->arm9Entry == NULL)
        return false;
    
    Size size = 0x200;
    for(int i = 0; i < 4; i++)
        size += firm->section[i].size;
    
    if(firmSize < size) 
        return false;
    
    for(int i = 0; i < 4; i++){
        firmSectionHeader *section = &firm->section[i];
        
        // Allow empty FIRM sections
        if(section->size == 0)
            continue;
        
        if((section->offset < 0x200) ||
           (section->address + section->size < section->address) || //Overflow check
           ((u32)section->address & 3) || (section->offset & 0x1FF) || (section->size & 0x1FF) || //Alignment check
           (overlaps((u32)section->address, (u32)section->address + section->size, (u32)firm, (u32)firm + size)) ||
           ((!inRange((u32)section->address, (u32)section->address + section->size, 0x08000000, 0x08000000 + 0x00100000)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x18000000, 0x18000000 + 0x00600000)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x1FF00000, 0x1FFFFC00)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x20000000, 0x20000000 + 0x8000000))))
                return false;

        ALIGNED(4) u8 hash[0x20];

        sha(hash, (u8 *)firm + section->offset, section->size, SHA_256_MODE);

        if(memcmp(hash, section->hash, 0x20) != 0)
            return false;

        if(firm->arm9Entry >= section->address && firm->arm9Entry < (section->address + section->size))
            arm9EpFound = true;

        if(firm->arm11Entry >= section->address && firm->arm11Entry < (section->address + section->size))
            arm11EpFound = true;
    }
    return arm9EpFound && (firm->arm11Entry == NULL || arm11EpFound);
}

//Load firm into FCRAM
void loadFirm(firmtype firm_type){
    //Read FIRM from SD card and write to FCRAM
    if (firm_type == NATIVE_FIRM){ // Native Firm
        fopen("/rei/native_firmware.bin", "rb");
        firmSize = fsize()/2;
        
        if(PDN_MPCORE_CFG == 1) 
            fseek(firmSize); //If O3DS, load 2nd firm
        
        fread(firm, 1, firmSize);
        fclose();
        decryptFirm(firm, firmSize);
    
        if(!firmCheck(firmSize)){
            debugWrite("/rei/debug.log", "External FIRM is invalid or corrupt. If this issue persists after a reinstall, make an issue.", 93);
            shutdown();
        }
    
        //Initial setup
        u8 *sect_arm9 = (u8*)firm + firm->section[2].offset;
        if(ISN3DS){
            k9loader((Arm9Bin*)sect_arm9);
            firm->arm9Entry = (u8*)0x801B01C;
        }
    
        //Inject custom loader if exists
        if(fopen("/rei/loader.cxi", "rb")){
            u8 *arm11SysMods = (u8*)firm + firm->section[0].offset;
            Size ldrInFirmSize;
            Size ldrFileSize = fsize();
            getLoader(arm11SysMods, &ldrInFirmSize, &ldrOffset);
            memcpy(firm->section[0].address, arm11SysMods, ldrOffset);
            fread(firm->section[0].address + ldrOffset, 1, ldrFileSize);
            memcpy(firm->section[0].address + ldrOffset + ldrFileSize, arm11SysMods + ldrOffset + ldrInFirmSize, firm->section[0].size - (ldrOffset + ldrInFirmSize));
            fclose();
        }
        else{
            memcpy(firm->section[0].address, firm + firm->section[0].offset, firm->section[0].size);
        }
    }
        if(firm_type == AGB_FIRM || firm_type == TWL_FIRM){ // Legacy Firms
            if(firm_type == AGB_FIRM)
                fopen("/rei/agb_firmware.bin", "rb");
            else
                fopen("/rei/twl_firmware.bin", "rb");
            firmSize = fsize()/2;
            
            if(PDN_MPCORE_CFG == 1) 
                fseek(firmSize); //If O3DS, load 2nd firm
            
            fread(firm, 1, firmSize);
            fclose();
            decryptFirm(firm, firmSize);
    
            if(!firmCheck(firmSize)){
                debugWrite("/rei/debug.log", "External FIRM is invalid or corrupt. If this issue persists after a reinstall, make an issue.", 93);
                shutdown();
            }
    
            //Initial setup
            firm = firm;
            u8 *sect_arm9 = (u8*)firm + firm->section[3].offset;
            if(ISN3DS){
                k9loader((Arm9Bin*)sect_arm9);
                firm->arm9Entry = (u8*)0x801301C;
            }            
        }
}

//Patches arm9 things on Sys/Emu
void patchFirm(firmtype firm_type, boottype boot_type, u16 path[]){
    if (firm_type == NATIVE_FIRM){
        u8 *sect_arm9 = (u8*)firm + firm->section[2].offset;
        u8 *sect_arm11 = (u8*)firm +firm->section[1].offset;
        u32 size_p9, addr_p9;
        
        u8 *offset_p9 = getProcess9Info(sect_arm9, firm->section[2].size, &size_p9, &addr_p9);
        u32 size_k9 = (u32)(offset_p9 - sect_arm9) - sizeof(Cxi) - 0x200;
        
         //Disable firm partition update if sighax is installed, and the user is using sysnand.
        if(ISSIGHAX && boot_type == SYSNAND)
            patchFirmWrite((u8*)firm, firmSize);
        
        //Boot EmuNand if told, unless AGB_FIRM was just launched.
        if(boot_type == EMUNAND && CFG_BOOTENV != 0x7)
            patchEmuCode(sect_arm9, size_k9, offset_p9, size_p9, firm->section[2].address);
        
        //Disable signature checks
        patchSigChecks((u8*)firm, firmSize);

        //Injects Reboot Code
        patchFirmlaunches(offset_p9, size_p9, addr_p9, path, reboot_bin, reboot_bin_size);
        
        //Find the Kernel11 SVC table and handler, exceptions page and free space locations
        u32 baseK11VA;
        u8 *freeK11Space;
        u32 *arm11SvcHandler,
            *arm11DAbtHandler,
            *arm11ExceptionsPage,
            *arm11SvcTable = getKernel11Info(sect_arm11, firm->section[1].size, &baseK11VA, &freeK11Space, &arm11SvcHandler, &arm11DAbtHandler, &arm11ExceptionsPage);

        //Re-inject 0x7B for NTR/Other k11 uses.
        reimplementSvcBackdoor(sect_arm11, arm11SvcTable, baseK11VA, &freeK11Space);
    }
    if (firm_type == AGB_FIRM || firm_type == TWL_FIRM){
        int ret = 0;
        u8 *sect_arm9 = (u8*)firm + firm->section[3].offset;
        u32 process9Size, process9MemAddr;
        u8 *process9Offset = getProcess9Info(sect_arm9, firm->section[3].size, &process9Size, &process9MemAddr);
        
        //Signature Checks for the AGB and TWL firms
        ret += patchLgySignatureChecks(process9Offset, process9Size);
        
        if(firm_type == TWL_FIRM){
            ret += patchTwlInvalidSignatureChecks(process9Offset, process9Size);
            ret += patchTwlNintendoLogoChecks(process9Offset, process9Size);
            ret += patchTwlWhitelistChecks(process9Offset, process9Size);
            ret += patchTwlFlashcartChecks(process9Offset, process9Size);
            ret += patchTwlShaHashChecks(process9Offset, process9Size);
        }

        if(ret != 0){
            debugWrite("/rei/debug.log", "Failed to patch a Legacy FIRM. ", 31);
            shutdown();
        }
    }
}

void launchFirm(firmtype firm_type, bool firmLaunch){
    int sectionNumber;
    if (firm_type == NATIVE_FIRM)
        sectionNumber = 1;
    else
        sectionNumber = 0;
    
    // Prepares arm11 for Firmlaunch
    prepareFirmlaunch();
    
    //Copy firm partitions to respective memory locations
    for(; sectionNumber < 4 && firm->section[sectionNumber].size != 0; sectionNumber++)
        memcpy(firm->section[sectionNumber].address, (u8*)firm + firm->section[sectionNumber].offset, firm->section[sectionNumber].size);
    
    flushEntireDCache();
    flushEntireICache();
    
    vu32 *arm11 = (vu32*)0x1FFFFFFC; //boot9strap Arm11 Entry
    
    if(!firmLaunch)
        shutdownLCD();
    
    *arm11 = firm->arm11Entry;
    
    //Final jump to arm9 binary
    u32 entry = firm->arm9Entry;
    ((void (*)())entry)();
}