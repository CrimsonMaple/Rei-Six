/*
*   firm.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef FIRM_INC
#define FIRM_INC

#include "types.h"

void loadSplash(void);
void loadFirmLegacy(boottype boot_type, firmtype firm_type);
void loadFirmTest(boottype boot_type, firmtype firm_type);
void loadNandType(boottype boot_type);
void patchFirm(firmtype firm_type, u16 path[]);
void launchFirm(firmtype firm_type, bool firmLaunch);

#endif