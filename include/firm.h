/*
*   firm.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef FIRM_INC
#define FIRM_INC

#include "types.h"

void loadFirm_SYSNAND(firmtype firm_type);
void loadFirm_EMUNAND(firmtype firm_type);
void patchFirm(firmtype firm_type, boottype boot_type, u16 path[]);
void launchFirm(firmtype firm_type, bool firmLaunch);

#endif