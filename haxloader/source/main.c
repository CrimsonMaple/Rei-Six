/*
*   This file is part of Luma3DS
*   Copyright (C) 2016 Aurora Wright, TuxSH
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b of GPLv3 applies to this file: Requiring preservation of specified
*   reasonable legal notices or author attributions in that material or in the Appropriate Legal
*   Notices displayed by works containing it.
*/

#include "memory.h"
#include "cache.h"
#include "i2c.h"
#include "types.h"
#include "fatfs/ff.h"

static FATFS fs;

void main(void)
{
    if(f_mount(&fs, "0:", 0) == FR_OK) // If SD is mounted read arm9loaderhax.bin from the SD.
    {
        FIL payload;
        
        bool payloadFound = f_open(&payload, "arm9loaderhax.bin", FA_READ) == FR_OK;
    }

    //Ensure that all memory transfers have completed and that the data cache has been flushed
    flushEntireDCache();

    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while(true);
}