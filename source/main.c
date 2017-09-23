/*
*   main.c
*       by Reisyukaku & CrimsonMaple
*   Copyright (c) 2017 All Rights Reserved
*/

#include "firm.h"
#include "patches.h"
#include "draw.h"
#include "fs.h"

void main(int argc, char **argv, u32 magic){
    bool isFirmLaunch;
    u16 launchedPath[41];
    
    firmtype firm_type;
    boottype boot_type;
    
    if(((magic & 0xFFFF) == 0xBEEF || (magic & 0xFFFF) == 0x2BEEF) && argc >= 1){ // Normal boot + Luma Chainloaded
        u32 i;
        for (i = 0; i < 40 && argv[0][i] != 0; ++i)
            launchedPath[i] = argv[0][i];

        launchedPath[i] = 0;
        isFirmLaunch = false;
    }
    else if(magic == 0xBABE && argc == 2) { // Firm Launch
        u32 i;
        u16 *p = (u16 *)argv[0];
        for (i = 0; i < 40 && p[i] != 0; i++)
            launchedPath[i] = p[i];
        launchedPath[i] = 0;
        isFirmLaunch = true;
    }
    else {
         // Unsupported launcher or launch method.
        debugWrite("/rei/debug.log", "Unsupported launcher or launch method. ", 39);
        shutdown();
    }
    mountSD();
    mountNand();
    loadSplash();
    
    //Boot EMUNAND if /rei/loademunand exists or R is held down. Otherwise boot SYSNAND.
    if(fopen("/rei/loademunand", "rb") || HID_PAD == (1 << 8))
        boot_type = EMUNAND;
    else
        boot_type = SYSNAND;
    
    if(isFirmLaunch){
        firm_type = (firmtype)(argv[1][10] - u'0');

        loadFirmLegacy(boot_type, firm_type);
        patchFirm(firm_type, launchedPath);
    }
    else{
        loadFirmLegacy(boot_type, firm_type); //Test Doesn't work at the moment.
        patchFirm(firm_type, launchedPath); //Awful way to do this but w/e.
    }
    
    launchFirm(firm_type, isFirmLaunch); //and we are done here...
}