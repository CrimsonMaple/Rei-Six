/*
*   lgy_patches.h
*       by Reisyukaku, CrimsonMaple
*   Copyright (c) 2015-2017 All Rights Reserved
*/

/*
*   This file is uses code from Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
*/

#ifndef LGYPATCHES_INC
#define LGYPATCHES_INC

#include "types.h"

u32 patchLgySignatureChecks(u8 *pos, u32 size);
u32 patchTwlInvalidSignatureChecks(u8 *pos, u32 size);
u32 patchTwlNintendoLogoChecks(u8 *pos, u32 size);
u32 patchTwlWhitelistChecks(u8 *pos, u32 size);
u32 patchTwlFlashcartChecks(u8 *pos, u32 size);
u32 patchTwlShaHashChecks(u8 *pos, u32 size);

#endif