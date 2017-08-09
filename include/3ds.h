/*
*   3ds.h
*       by Reisyukaku & CrimsonMaple
*   Copyright (c) 2017 All Rights Reserved
*/

#include <stdint.h>

#define Packed __attribute__((packed))

typedef struct Packed
{
    uint32_t address;
    uint32_t phyRegionSize;
    uint32_t size;
} CodeSetInfo;

typedef struct Packed
{
    uint32_t saveDataSize[2];
    uint32_t jumpID[2];
    uint8_t reserved[0x30];
} SystemInfo;

typedef struct Packed
{
    char appTitle[8];
    uint8_t reserved1[5];
    uint8_t flag;
    uint8_t remasterVersion[2];
    CodeSetInfo textCodeSet;
    uint32_t stackSize;
    CodeSetInfo roCodeSet;
    uint8_t reserved2[4];
    CodeSetInfo dataCodeSet;
    uint32_t bssSize;
    char depends[0x180];
    SystemInfo systemInfo;
} SysControlInfo;

typedef struct Packed
{
    SysControlInfo systemControlInfo;
    uint8_t aci[0x200];
    uint8_t accessDescSig[0x100];
    uint8_t ncchPubKey[0x100];
    uint8_t aciLim[0x200];
} ExHeader;

typedef struct Packed
{
    uint8_t sig[0x100]; //RSA-2048 signature of the NCCH header, using SHA-256
    char magic[4]; //NCCH
    uint32_t contentSize; //Media unit
    uint8_t partitionId[8];
    uint8_t makerCode[2];
    uint16_t version;
    uint8_t reserved1[4];
    uint8_t programID[8];
    uint8_t reserved2[0x10];
    uint8_t logoHash[0x20]; //Logo Region SHA-256 hash
    char productCode[0x10];
    uint8_t exHeaderHash[0x20]; //Extended header SHA-256 hash
    uint32_t exHeaderSize; //Extended header size
    uint32_t reserved3;
    uint8_t flags[8];
    uint32_t plainOffset; //Media unit
    uint32_t plainSize; //Media unit
    uint32_t logoOffset; //Media unit
    uint32_t logoSize; //Media unit
    uint32_t exeFsOffset; //Media unit
    uint32_t exeFsSize; //Media unit
    uint32_t exeFsHashSize; //Media unit
    uint32_t reserved4;
    uint32_t romFsOffset; //Media unit
    uint32_t romFsSize; //Media unit
    uint32_t romFsHashSize; //Media unit
    uint32_t reserved5;
    uint8_t exeFsHash[0x20]; //ExeFS superblock SHA-256 hash
    uint8_t romFsHash[0x20]; //RomFS superblock SHA-256 hash
} Ncch;

typedef struct Packed
{
    Ncch ncch;
    ExHeader exHeader;
} Cxi;

typedef struct Packed
{
    char sigIssuer[0x40];
    uint8_t eccPubKey[0x3C];
    uint8_t version;
    uint8_t caCrlVersion;
    uint8_t signerCrlVersion;
    uint8_t titleKey[0x10];
    uint8_t reserved1;
    uint8_t ticketId[8];
    uint8_t consoleId[4];
    uint8_t titleId[8];
    uint8_t reserved2[2];
    uint16_t ticketTitleVersion;
    uint8_t reserved3[8];
    uint8_t licenseType;
    uint8_t ticketCommonKeyYIndex; //Ticket common keyY index, usually 0x1 for retail system titles.
    uint8_t reserved4[0x2A];
    uint8_t unk[4]; //eShop Account ID?
    uint8_t reserved5;
    uint8_t audit;
    uint8_t reserved6[0x42];
    uint8_t limits[0x40];
    uint8_t contentIndex[0xAC];
} Ticket;
typedef struct Packed
{
    uint8_t keyX[0x10];
    uint8_t keyY[0x10];
    uint8_t ctr[0x10];
    char size[8];
    uint8_t reserved[8];
    uint8_t ctlBlock[0x10];
    char magic[4];
    uint8_t reserved2[0xC];
    uint8_t slot0x16keyX[0x10];
} Arm9Bin;