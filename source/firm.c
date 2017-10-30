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
#include "../build/payloads.h"

//Firm vars
firmHeader *firm = (firmHeader*)0x24000000;
Size firmSize = 0;
    
//Patch vars
uPtr firmWriteOffset = 0,
     ldrOffset = 0;

//Load firm into FCRAM
void loadFirmLegacy(boottype boot_type, firmtype firm_type){
    //Read FIRM from SD card and write to FCRAM
    if (firm_type == NATIVE_FIRM){
        fopen("/rei/native_firmware.bin", "rb");
        firmSize = fsize()/2;
        if(PDN_MPCORE_CFG == 1) fseek(firmSize); //If O3DS, load 2nd firm
        fread(firm, 1, firmSize);
        fclose();
        decryptFirm(firm, firmSize);
    
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
            if(PDN_MPCORE_CFG == 1) fseek(firmSize); //If O3DS, load 2nd firm
            fread(firm, 1, firmSize);
            fclose();
            decryptFirm(firm, firmSize);
    
            //Initial setup
            firm = firm;
            u8 *sect_arm9 = (u8*)firm + firm->section[3].offset;
            if(ISN3DS){
                k9loader((Arm9Bin*)sect_arm9);
                firm->arm9Entry = (u8*)0x801301C;
            }            
        }
}

//Load firm into FCRAM
void loadFirmTest(firmtype firm_type){
    //Read firm from NAND.
    u32 firmVersion = firmRead((u8*)firm, firm_type); //Native Firm
    u8 *sect_arm9 = (u8*)firm + firm->section[2].offset;
    
    if(firmVersion == 0xDEADBEEF){
        debugWrite("/rei/debug.log", "Failed to mount CTRNAND. ", 25);
        shutdown();
    }
    
    firmSize = decryptExeFs((Cxi*)((u8*)firm));
    
    if(!firmSize){
        debugWrite("/rei/debug.log", "Failed to decrypt the CTRNAND FIRM. ", 38);
        shutdown();
    }

    if((firm->section[3].offset != 0 ? firm->section[3].address : firm->section[2].address) != (ISN3DS ? (u8*)0x8006000 : (u8*)0x8006800)){
        debugWrite("/rei/debug.log", "This firm isn't for this console. ", 34);
        shutdown();
    }
    
    //Initial setup
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
            patchFirmWrite(firm, firmSize);
        
        //Boot EmuNand if told, unless AGB_FIRM was just launched.
        if(boot_type == EMUNAND && CFG_BOOTENV != 0x7)
            patchEmuCode(sect_arm9, size_k9, offset_p9, size_p9, firm->section[2].address);
        
        //Disable signature checks
        patchSigChecks(firm, firmSize);

        patchFirmlaunches(offset_p9, size_p9, addr_p9, path, reboot_bin, reboot_bin_size);
        
        //Find the Kernel11 SVC table and handler, exceptions page and free space locations
        u32 baseK11VA;
        u8 *freeK11Space;
        u32 *arm11SvcHandler,
            *arm11DAbtHandler,
            *arm11ExceptionsPage,
            *arm11SvcTable = getKernel11Info(sect_arm11, firm->section[1].size, &baseK11VA, &freeK11Space, &arm11SvcHandler, &arm11DAbtHandler, &arm11ExceptionsPage);

        reimplementSvcBackdoor(sect_arm11, arm11SvcTable, baseK11VA, &freeK11Space);
    }
    if (firm_type == AGB_FIRM || firm_type == TWL_FIRM){
        int ret = 0;
        u8 *sect_arm9 = (u8*)firm + firm->section[3].offset;
        u32 process9Size, process9MemAddr;
        u8 *process9Offset = getProcess9Info(sect_arm9, firm->section[3].size, &process9Size, &process9MemAddr);
        
        ret += patchLgySignatureChecks(process9Offset, process9Size);
        
        if(firm_type = TWL_FIRM){
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
    
    //Copy firm partitions to respective memory locations
    for(; sectionNumber < 4 && firm->section[sectionNumber].size != 0; sectionNumber++)
        memcpy(firm->section[sectionNumber].address, (u8*)firm + firm->section[sectionNumber].offset, firm->section[sectionNumber].size);
    
    vu32 *arm11 = (vu32*)0x1FFFFFFC; //boot9strap Arm11 Entry
    
    if(!firmLaunch)
        shutdownLCD();
    
    *arm11 = firm->arm11Entry;
    
    //Final jump to arm9 binary
    u32 entry = firm->arm9Entry;
    ((void (*)())entry)();
}