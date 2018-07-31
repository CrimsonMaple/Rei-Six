/*
*   types.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef TYPES_INC
#define TYPES_INC

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include "3ds.h"

//Memory regions
#define CFG_BOOTENV      *(vu32*)0x10010000
#define CFG_SYSPROT9     ((*(vu8*)0x10000000) & 0x2)
#define CFG_TWLUNITINFO  (*(vu8*)0x10010014)
#define OTP_TWLCONSOLEID (*(vu64*)0x10012100)
#define HID_PAD          (*(vu32*)0x10146000 ^ 0xFFF)
#define PDN_MPCORE_CFG   *(u8*)0x10140FFC
#define PDN_SPI_CNT      *(vu32*)0x101401C0
#define UNITINFO         *(u8*)0x10010010

// Values
#define ISN3DS           (PDN_MPCORE_CFG == 7)
#define ISA9LH           (!PDN_SPI_CNT)
#define ISSIGHAX         (ISA9LH && CFG_SYSPROT9)
#define ISSYSNAND        0 // temp value.

//MPU setting macro (Kernel9)
#define PERMS(dataAccess,instrAccess,regionSize) ((regionSize<<17)|(instrAccess<<8)|dataAccess)

//RegionSizes
#define SIZE_32KB 14
#define SIZE_64KB 15
#define SIZE_128KB 16
#define SIZE_256KB 17
#define SIZE_256MB 27

//AccessPerms (priv_user)
#define N_N 0
#define RW_N 1
#define RW_R 2
#define RW_RW 3
#define R_R 6

// FIRM Paths
#define o3ds_native_firm    "1:/title/00040138/00000002/content/00000064.app"
#define n3ds_native_firm    "1:/title/00040138/20000002/content/00000035.app"
#define o3ds_twl_firm       "1:/title/00040138/00000102/content/00000018.app"
#define n3ds_twl_firm       "1:/title/00040138/20000102/content/00000004.app"
#define o3ds_agb_firm       "1:/title/00040138/00000202/content/0000000b.app"
#define n3ds_agb_firm       "1:/title/00040138/20000202/content/00000000.app"

#define ALIGNED(x)           __attribute__((aligned(x)))

//Common data types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef volatile uint8_t vu8;
typedef volatile uint16_t vu16;
typedef volatile uint32_t vu32;
typedef volatile uint64_t vu64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uintptr_t uPtr;
typedef size_t Size;

// CFW Launch Variables
typedef enum firmtype {
    NATIVE_FIRM = 0,
    TWL_FIRM,
    AGB_FIRM
}firmtype;

typedef enum boottype {
    SYSNAND = 0,
    EMUNAND
}boottype;

extern bool isFirmLaunch;
extern u16 launchedPath[41];

//FIRM Header layout
typedef struct firmSectionHeader {
    u32 offset;
    u8 *address;
    u32 size;
    u32 procType;
    u8 hash[0x20];
} firmSectionHeader;

typedef struct firmHeader {
    char magic[4];
    u32 reserved1;
    u32 arm11Entry;
    u32 arm9Entry;
    u8 reserved2[0x30];
    firmSectionHeader section[4];
} firmHeader;

#endif