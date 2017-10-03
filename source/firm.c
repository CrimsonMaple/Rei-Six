/*
*   firm.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "firm.h"
#include "patches.h"
#include "memory.h"
#include "fs.h"
#include "emunand.h"
#include "lcd.h"
#include "crypto.h"
#include "../build/payloads.h"

//Firm vars
const void *firmLocation = (void*)0x24000000;
firmHeader *firm = (firmHeader*)0x24000000;
Size firmSize = 0;

//Emu vars
uPtr emuOffset = 0,
     emuHeader = 0,
     emuRead = 0,
     emuWrite = 0,
     sdmmcOffset = 0,
     mpuOffset = 0,
     emuCodeOffset = 0;
    
//Patch vars
uPtr sigPatchOffset1 = 0,
     sigPatchOffset2 = 0,
     threadOffset1 = 0,
     threadOffset2 = 0,
     threadCodeOffset = 0,
     firmWriteOffset = 0,
     ldrOffset = 0,
     rebootOffset = 0, 
     fOpenOffset = 0;
     
//Load firm into FCRAM
void loadFirmLegacy(boottype boot_type, firmtype firm_type){
    //Read FIRM from SD card and write to FCRAM
    if (firm_type == NATIVE_FIRM){
        fopen("/rei/native_firmware.bin", "rb");
        firmSize = fsize()/2;
        if(PDN_MPCORE_CFG == 1) fseek(firmSize); //If O3DS, load 2nd firm
        fread(firmLocation, 1, firmSize);
        fclose();
        decryptFirm(firmLocation, firmSize);
    
        //Initial setup
        firm = firmLocation;
        u8 *sect_arm9 = (u8*)firm + firm->section[2].offset;
        if(ISN3DS){
            k9loader((Arm9Bin*)sect_arm9);
            firm->arm9Entry = (u8*)0x801B01C;
        }
    
        //Set MPU for emu code region
        getMPU(firmLocation, firmSize, &mpuOffset);
        *(u32*)mpuOffset = PERMS(RW_RW, N_N, SIZE_256MB);        //Area4:0x10100000
        *(u32*)(mpuOffset+0x18) = PERMS(RW_RW, R_R, SIZE_128KB); //Area6:0x8000000
        *(u32*)(mpuOffset+0x24) = PERMS(RW_RW, R_R, SIZE_32KB);  //Area7:0x8020000
    
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
        } else
            memcpy(firm->section[0].address, firmLocation + firm->section[0].offset, firm->section[0].size);
        }
        if(firm_type == AGB_FIRM || firm_type == TWL_FIRM){ // Legacy Firms
            if(firm_type == AGB_FIRM)
                fopen("/rei/agb_firmware.bin", "rb");
            else
                fopen("/rei/twl_firmware.bin", "rb");
            firmSize = fsize()/2;
            if(PDN_MPCORE_CFG == 1) fseek(firmSize); //If O3DS, load 2nd firm
            fread(firmLocation, 1, firmSize);
            fclose();
            decryptFirm(firmLocation, firmSize);
    
            //Initial setup
            firm = firmLocation;
            u8 *sect_arm9 = (u8*)firm + firm->section[3].offset;
            if(ISN3DS){
                k9loader((Arm9Bin*)sect_arm9);
                firm->arm9Entry = (u8*)0x801301C;
            }            
        }
        
    //Dont boot emu if AGB game was just played, or if START was held.
    getEmunandSect(&emuOffset, &emuHeader);
    loadNandType(boot_type);
}

//Load firm into FCRAM
void loadFirmTest(boottype boot_type, firmtype firm_type){
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

    if((firm->section[3].offset != 0 ? firm->section[3].address : firm->section[2].address) != (ISN3DS ? (u8 *)0x8006000 : (u8 *)0x8006800)){
        debugWrite("/rei/debug.log", "This firm isn't for this console. ", 34);
        shutdown();
    }
    
    //Initial setup
    if(ISN3DS){
        k9loader((Arm9Bin*)sect_arm9);
        firm->arm9Entry = (u8*)0x801B01C;
    }
    
    //Set MPU for emu code region
    getMPU(firmLocation, firmSize, &mpuOffset);
    *(u32*)mpuOffset = PERMS(RW_RW, N_N, SIZE_256MB);        //Area4:0x10100000
    *(u32*)(mpuOffset+0x18) = PERMS(RW_RW, R_R, SIZE_128KB); //Area6:0x8000000
    *(u32*)(mpuOffset+0x24) = PERMS(RW_RW, R_R, SIZE_32KB);  //Area7:0x8020000
    
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
    }else{
        memcpy(firm->section[0].address, firmLocation + firm->section[0].offset, firm->section[0].size);
    }

    getEmunandSect(&emuOffset, &emuHeader);
    loadNandType(boot_type);
}

void loadNandType(boottype boot_type){
    if(boot_type == EMUNAND){
        //Read emunand code from SD
        getEmuCode(firmLocation, firmSize, &emuCodeOffset);
        memcpy((void*)emuCodeOffset, emunand_bin, emunand_bin_size);
        u32 branchAddr = (u32)((uPtr)emuCodeOffset - (uPtr)firmLocation - firm->section[2].offset + firm->section[2].address);
    
        //Setup Emunand code
        uPtr *pos_sdmmc = (uPtr*)old_memsearch((void*)emuCodeOffset, "SDMC", emunand_bin_size, 4);
        uPtr *pos_offset = (uPtr*)old_memsearch((void*)emuCodeOffset, "NAND", emunand_bin_size, 4);
        uPtr *pos_header = (uPtr*)old_memsearch((void*)emuCodeOffset, "NCSD", emunand_bin_size, 4);
        getSDMMC(firmLocation, firmSize, (void*)&sdmmcOffset);
        getEmuRW(firmLocation, firmSize, (void*)&emuRead, (void*)&emuWrite);
        *pos_sdmmc = sdmmcOffset;
        *pos_offset = emuOffset;
        *pos_header = emuHeader;
        *(u32*)emuRead = *(u32*)emuWrite = 0x47A04C00;
    
        //Add Emunand hooks
        *(u32*)(emuRead+4) = *(u32*)(emuWrite+4) = branchAddr;
    } else {
        //Disable firm partition update if sighax is installed
        if(ISSIGHAX){
            getFirmWrite(firmLocation, firmSize, &firmWriteOffset);
            *(u32*)firmWriteOffset = 0x46C02000;
        }
    }
}

//Patches arm9 things on Sys/Emu
void patchFirm(firmtype firm_type, u16 path[]){
    if (firm_type == NATIVE_FIRM){
        //Disable signature checks
        getSigChecks(firmLocation, firmSize, &sigPatchOffset1, &sigPatchOffset2);
        *(u16*)sigPatchOffset1 = 0x2000;
        *(u32*)sigPatchOffset2 = 0x47702000;

        u8 *sect_arm9 = (u8*)firm + firm->section[2].offset;
        u32 process9Size, process9MemAddr;
        u8 *process9Offset = getProcess9Info(sect_arm9, firm->section[2].size, &process9Size, &process9MemAddr);

        patchFirmlaunches(process9Offset, process9Size, process9MemAddr, path, reboot_bin, reboot_bin_size);
        //injectBackdoor((firmHeader*)firm);
    }
    if (firm_type == AGB_FIRM){
        int ret = 0;
        u8 *sect_arm9 = (u8*)firm + firm->section[3].offset;
        u32 process9Size, process9MemAddr;
        u8 *process9Offset = getProcess9Info(sect_arm9, firm->section[3].size, &process9Size, &process9MemAddr);
        
        ret += patchLgySignatureChecks(process9Offset, process9Size);
        
        if(ret != 0)
            debugWrite("/rei/debugagb.log", "Failed to patch AGB FIRM. ", 26);
    }
    if (firm_type == TWL_FIRM){
        int ret = 0;
        u8 *sect_arm9 = (u8*)firm + firm->section[3].offset;
        u32 process9Size, process9MemAddr;
        u8 *process9Offset = getProcess9Info(sect_arm9, firm->section[3].size, &process9Size, &process9MemAddr);
        
        ret += patchLgySignatureChecks(process9Offset, process9Size);
		ret += patchTwlInvalidSignatureChecks(process9Offset, process9Size);
		ret += patchTwlNintendoLogoChecks(process9Offset, process9Size);
		ret += patchTwlWhitelistChecks(process9Offset, process9Size);
		ret += patchTwlFlashcartChecks(process9Offset, process9Size);
		ret += patchTwlShaHashChecks(process9Offset, process9Size);

        if(ret != 0)
            debugWrite("/rei/debugtwl.log", "Failed to patch TWL FIRM. ", 26);
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
        memcpy(firm->section[sectionNumber].address, (u8*)firmLocation + firm->section[sectionNumber].offset, firm->section[sectionNumber].size);
    
    vu32 *arm11 = (vu32*)0x1FFFFFFC; //boot9strap Arm11 Entry
	
    if(!firmLaunch)
        shutdownLCD();
    
    *arm11 = firm->arm11Entry;
    
    //Final jump to arm9 binary
    u32 entry = firm->arm9Entry;
    ((void (*)())entry)();
}