/*
*   main.c
*       by Reisyukaku & CrimsonMaple
*   Copyright (c) 2017 All Rights Reserved
*/

#include "firm.h"
#include "patches.h"
#include "emunand.h"
#include "draw.h"
#include "fs.h"

void main(int argc, char **argv, u32 magic){
    bool isFirmLaunch;
    u16 launchedPath[41];
    
    firmtype firm_type;
    boottype boot_type;
    
    if(((magic & 0xFFFF) == 0xBEEF || (magic & 0xFFFF) == 0x2BEEF) && argc >= 1){ // Normal boot or Luma Chainloaded (Currently based on latest stable release)
        u32 i;
        for (i = 0; i < 40 && argv[0][i] != 0; ++i)
            launchedPath[i] = argv[0][i];

        launchedPath[i] = 0;
        isFirmLaunch = false;
    }
    else if(magic == 0xBABE && argc == 2){ // Firmlaunch
        u32 i;
        u16 *p = (u16 *)argv[0];
        for (i = 0; i < 40 && p[i] != 0; i++)
            launchedPath[i] = p[i];
        launchedPath[i] = 0;
        isFirmLaunch = true;
    }
    else{
         // Unsupported launcher or launch method.
        debugWrite("/rei/debug.log", "Unsupported launcher or launch method. ", 39);
        shutdown();
    }
    
    mountSD();
    mountNand();
    loadSplash();
    
    //Boot EMUNAND if /rei/loademunand exists or R is held down. Otherwise boot SYSNAND.
    if(fopen("/rei/loademunand", "rb") || HID_PAD == (1 << 8)){
        boot_type = EMUNAND;
        getEmunand(boot_type); //Have to make sure emunand exists somewhere on SD
    }
    else
        boot_type = SYSNAND;
    
    if(isFirmLaunch){
        if(argv[1][14] == 2)
            firm_type = (firmtype)(argv[1][10] - '0');
        else{
            debugWrite("/rei/debug.log", "Unsupported FIRM. ", 18); // This should happen if SAFE_FIRM/SYSUPDATER is launched.
            shutdown();
        }

        loadFirmLegacy(firm_type);
        patchFirm(firm_type, boot_type, launchedPath);
    }
    else{
        loadFirmLegacy(firm_type);
        patchFirm(firm_type, boot_type, launchedPath);
    }
    
    launchFirm(firm_type, isFirmLaunch);
}