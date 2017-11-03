// 3ds crypto lib from: http://github.com/b1l1s/ctr
// CTRNAND code from: https://github.com/AuroraWright/Luma3ds

#include "crypto.h"

#include <stddef.h>
#include "memory.h"
#include "fatfs/sdmmc/sdmmc.h"
#include "fatfs/ff.h"
#include "fmt.h"
#include "fs.h"

/****************************************************************
*                   Crypto Libs
****************************************************************/

/* original version by megazig */

#ifndef __thumb__
#define BSWAP32(x) {\
    __asm__\
    (\
        "eor r1, %1, %1, ror #16\n\t"\
        "bic r1, r1, #0xFF0000\n\t"\
        "mov %0, %1, ror #8\n\t"\
        "eor %0, %0, r1, lsr #8\n\t"\
        :"=r"(x)\
        :"0"(x)\
        :"r1"\
    );\
};

#define ADD_u128_u32(u128_0, u128_1, u128_2, u128_3, u32_0) {\
__asm__\
    (\
        "adds %0, %4\n\t"\
        "addcss %1, %1, #1\n\t"\
        "addcss %2, %2, #1\n\t"\
        "addcs %3, %3, #1\n\t"\
        : "+r"(u128_0), "+r"(u128_1), "+r"(u128_2), "+r"(u128_3)\
        : "r"(u32_0)\
        : "cc"\
    );\
}
#else
#define BSWAP32(x) {x = __builtin_bswap32(x);}

#define ADD_u128_u32(u128_0, u128_1, u128_2, u128_3, u32_0) {\
__asm__\
    (\
        "mov r4, #0\n\t"\
        "add %0, %0, %4\n\t"\
        "adc %1, %1, r4\n\t"\
        "adc %2, %2, r4\n\t"\
        "adc %3, %3, r4\n\t"\
        : "+r"(u128_0), "+r"(u128_1), "+r"(u128_2), "+r"(u128_3)\
        : "r"(u32_0)\
        : "cc", "r4"\
    );\
}
#endif /*__thumb__*/

void aes_setkey(u8 keyslot, const void* key, u32 keyType, u32 mode)
{
    if(keyslot <= 0x03) return; // Ignore TWL keys for now
    u32* key32 = (u32*)key;
    *REG_AESCNT = (*REG_AESCNT & ~(AES_CNT_INPUT_ENDIAN | AES_CNT_INPUT_ORDER)) | mode;
    *REG_AESKEYCNT = (*REG_AESKEYCNT >> 6 << 6) | keyslot | AES_KEYCNT_WRITE;

    REG_AESKEYFIFO[keyType] = key32[0];
    REG_AESKEYFIFO[keyType] = key32[1];
    REG_AESKEYFIFO[keyType] = key32[2];
    REG_AESKEYFIFO[keyType] = key32[3];
}

void aes_use_keyslot(u8 keyslot)
{
    if(keyslot > 0x3F)
        return;

    *REG_AESKEYSEL = keyslot;
    *REG_AESCNT = *REG_AESCNT | 0x04000000; /* mystery bit */
}

void aes_setiv(const void* iv, u32 mode)
{
    const u32* iv32 = (const u32*)iv;
    *REG_AESCNT = (*REG_AESCNT & ~(AES_CNT_INPUT_ENDIAN | AES_CNT_INPUT_ORDER)) | mode;

    // Word order for IV can't be changed in REG_AESCNT and always default to reversed
    if(mode & AES_INPUT_NORMAL)
    {
        REG_AESCTR[0] = iv32[3];
        REG_AESCTR[1] = iv32[2];
        REG_AESCTR[2] = iv32[1];
        REG_AESCTR[3] = iv32[0];
    }
    else
    {
        REG_AESCTR[0] = iv32[0];
        REG_AESCTR[1] = iv32[1];
        REG_AESCTR[2] = iv32[2];
        REG_AESCTR[3] = iv32[3];
    }
}

void aes_advctr(void* ctr, u32 val, u32 mode)
{
    u32* ctr32 = (u32*)ctr;
    
    int i;
    if(mode & AES_INPUT_BE)
    {
        for(i = 0; i < 4; ++i) // Endian swap
            BSWAP32(ctr32[i]);
    }
    
    if(mode & AES_INPUT_NORMAL)
    {
        ADD_u128_u32(ctr32[3], ctr32[2], ctr32[1], ctr32[0], val);
    }
    else
    {
        ADD_u128_u32(ctr32[0], ctr32[1], ctr32[2], ctr32[3], val);
    }
    
    if(mode & AES_INPUT_BE)
    {
        for(i = 0; i < 4; ++i) // Endian swap
            BSWAP32(ctr32[i]);
    }
}

void aes_change_ctrmode(void* ctr, u32 fromMode, u32 toMode)
{
    u32* ctr32 = (u32*)ctr;
    int i;
    if((fromMode ^ toMode) & AES_CNT_INPUT_ENDIAN)
    {
        for(i = 0; i < 4; ++i)
            BSWAP32(ctr32[i]);
    }

    if((fromMode ^ toMode) & AES_CNT_INPUT_ORDER)
    {
        u32 temp = ctr32[0];
        ctr32[0] = ctr32[3];
        ctr32[3] = temp;

        temp = ctr32[1];
        ctr32[1] = ctr32[2];
        ctr32[2] = temp;
    }
}

void aes_batch(void* dst, const void* src, u32 blockCount)
{
    *REG_AESBLKCNT = blockCount << 16;
    *REG_AESCNT |=  AES_CNT_START;
    
    const u32* src32    = (const u32*)src;
    u32* dst32          = (u32*)dst;
    
    u32 wbc = blockCount;
    u32 rbc = blockCount;
    
    while(rbc)
    {
        if(wbc && ((*REG_AESCNT & 0x1F) <= 0xC)) // There's space for at least 4 ints
        {
            *REG_AESWRFIFO = *src32++;
            *REG_AESWRFIFO = *src32++;
            *REG_AESWRFIFO = *src32++;
            *REG_AESWRFIFO = *src32++;
            wbc--;
        }
        
        if(rbc && ((*REG_AESCNT & (0x1F << 0x5)) >= (0x4 << 0x5))) // At least 4 ints available for read
        {
            *dst32++ = *REG_AESRDFIFO;
            *dst32++ = *REG_AESRDFIFO;
            *dst32++ = *REG_AESRDFIFO;
            *dst32++ = *REG_AESRDFIFO;
            rbc--;
        }
    }
}

void aes(void* dst, const void* src, u32 blockCount, void* iv, u32 mode, u32 ivMode)
{
    *REG_AESCNT =   mode |
                    AES_CNT_INPUT_ORDER | AES_CNT_OUTPUT_ORDER |
                    AES_CNT_INPUT_ENDIAN | AES_CNT_OUTPUT_ENDIAN |
                    AES_CNT_FLUSH_READ | AES_CNT_FLUSH_WRITE;

    u32 blocks;
    while(blockCount != 0)
    {
        if((mode & AES_ALL_MODES) != AES_ECB_ENCRYPT_MODE
        && (mode & AES_ALL_MODES) != AES_ECB_DECRYPT_MODE)
            aes_setiv(iv, ivMode);

        blocks = (blockCount >= 0xFFFF) ? 0xFFFF : blockCount;

        // Save the last block for the next decryption CBC batch's iv
        if((mode & AES_ALL_MODES) == AES_CBC_DECRYPT_MODE)
        {
            memcpy(iv, src + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
            aes_change_ctrmode(iv, AES_INPUT_BE | AES_INPUT_NORMAL, ivMode);
        }

        // Process the current batch
        aes_batch(dst, src, blocks);

        // Save the last block for the next encryption CBC batch's iv
        if((mode & AES_ALL_MODES) == AES_CBC_ENCRYPT_MODE)
        {
            memcpy(iv, dst + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
            aes_change_ctrmode(iv, AES_INPUT_BE | AES_INPUT_NORMAL, ivMode);
        }
        
        // Advance counter for CTR mode
        else if((mode & AES_ALL_MODES) == AES_CTR_MODE)
            aes_advctr(iv, blocks, ivMode);

        src += blocks * AES_BLOCK_SIZE;
        dst += blocks * AES_BLOCK_SIZE;
        blockCount -= blocks;
    }
}

static void sha_wait_idle()
{
    while(*REG_SHA_CNT & 1);
}

void sha(void *res, const void *src, u32 size, u32 mode)
{
    sha_wait_idle();
    *REG_SHA_CNT = mode | SHA_CNT_OUTPUT_ENDIAN | SHA_NORMAL_ROUND;

    const u32 *src32 = (const u32 *)src;
    int i;
    while(size >= 0x40)
    {
        sha_wait_idle();
        for(i = 0; i < 4; ++i)
        {
            *REG_SHA_INFIFO = *src32++;
            *REG_SHA_INFIFO = *src32++;
            *REG_SHA_INFIFO = *src32++;
            *REG_SHA_INFIFO = *src32++;
        }

        size -= 0x40;
    }

    sha_wait_idle();
    memcpy((void *)REG_SHA_INFIFO, src32, size);

    *REG_SHA_CNT = (*REG_SHA_CNT & ~SHA_NORMAL_ROUND) | SHA_FINAL_ROUND;

    while(*REG_SHA_CNT & SHA_FINAL_ROUND);
    sha_wait_idle();

    u32 hashSize = SHA_256_HASH_SIZE;
    if(mode == SHA_224_MODE)
        hashSize = SHA_224_HASH_SIZE;
    else if(mode == SHA_1_MODE)
        hashSize = SHA_1_HASH_SIZE;

    memcpy(res, (void *)REG_SHA_HASH, hashSize);
}


void xor(u8 *dest, const u8 *data1, const u8 *data2, Size size){
    u32 i; for(i = 0; i < size; i++) *((u8*)dest+i) = *((u8*)data1+i) ^ *((u8*)data2+i);
}

/****************************************************************
*                   Nand/FIRM Crypto stuff
****************************************************************/
ALIGNED(4) static u8 nandCtr[AES_BLOCK_SIZE];
static u8 nandSlot;
static u32 fatStart;

int nandInit(void){
    ALIGNED(4) u8 cid[AES_BLOCK_SIZE],
               shaSum[SHA_256_HASH_SIZE];
    
    sdmmc_get_cid(1, (u32*)cid);
    sha(shaSum, cid, AES_BLOCK_SIZE, SHA_256_MODE);
    memcpy(nandCtr, shaSum, sizeof(nandCtr));
    
    if(ISN3DS){ fatStart = 0x5CAD7; nandSlot = 0x05; } else { fatStart = 0x5CAE5; nandSlot = 0x04; }
    
    return 1;
}

u32 nandRead(u32 sector, u32 sectorCount, u8 *buffer){ //Only Reads from the Physical Nand for simplicity.
    u32 result;
    ALIGNED(4) u8 tmpCtr[AES_BLOCK_SIZE]; //Set variable for result and copy nandCtr to a Temp variable.
    
    memcpy(tmpCtr, nandCtr, sizeof(nandCtr)); //copy nandCtr to tmpCtr
    aes_advctr(tmpCtr, ((sector + fatStart) * 0x200) / AES_BLOCK_SIZE, AES_INPUT_BE | AES_INPUT_NORMAL);
    
    result = sdmmc_nand_readsectors(sector + fatStart, sectorCount, buffer);
    
    aes_use_keyslot(nandSlot);
    aes(buffer, buffer, (sectorCount * 0x200) / AES_BLOCK_SIZE, tmpCtr, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);
    
    return result;
}

bool decryptExeFs(Cxi *cxi){
    if(memcmp(cxi->ncch.magic, "NCCH", 4) != 0) 
        return false;
    
    u8 *exeFsOffset = (u8*)cxi + 6 * 0x200;
    u32 exeFsSize = (cxi->ncch.exeFsSize - 1) * 0x200;
    ALIGNED(4) u8 ncchCtr[AES_BLOCK_SIZE] = {0};
    
    for(u32 i = 0; i < 8; i++)
        ncchCtr[7 - i] = cxi->ncch.partitionId[i];
    
    ncchCtr[8] = 2;
    aes_setkey(0x2C, cxi, AES_KEYY, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_advctr(ncchCtr, 0x200 / AES_BLOCK_SIZE, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x2C);
    aes(cxi, exeFsOffset, exeFsSize / AES_BLOCK_SIZE, ncchCtr, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);
    
    return memcmp(cxi, "FIRM", 4) == 0;
}

const u8 memeKey[0x10] = { //Megumin best girl, fite me Rei.
    0x52, 0x65, 0x69, 0x20, 0x69, 0x73, 0x20, 0x62, 0x65, 0x73, 0x74, 0x20, 0x67, 0x69, 0x72, 0x6C
};

void initKeyslot11(void){
    ALIGNED(4) static u8 shasum[SHA_256_HASH_SIZE];
    sha(shasum, (void *)0x10012000, 0x90, SHA_256_MODE);

    aes_setkey(0x11, shasum, AES_KEYX, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_setkey(0x11, shasum + AES_BLOCK_SIZE, AES_KEYY, AES_INPUT_BE | AES_INPUT_NORMAL);
}

void checkKeyHash(u8* key1, u8* key2){
    //Compare key hashes
    ALIGNED(4) u8 computed_key1[SHA_256_HASH_SIZE];
    ALIGNED(4) u8 computed_key2[SHA_256_HASH_SIZE];

    ALIGNED(4) const u8 key1_hash[SHA_256_HASH_SIZE] = { 
        0x18, 0x08, 0x60, 0xF9, 0x82, 0x44, 0x46, 0xB8, 0xAA, 0x2D, 0xFF, 0xB8, 0x71, 0x40, 0x27, 0x32, 0x73, 0xB6, 0xB6, 0xA9, 0xC8, 0x37, 0x72, 0x02, 0xF9, 0xC4, 0xF9, 0x72, 0x3E, 0xE9, 0x94, 0xB8
    };
    ALIGNED(4) const u8 key2_hash[SHA_256_HASH_SIZE] = {
        0x9F, 0x80, 0x48, 0x66, 0xF1, 0xF0, 0x4B, 0x78, 0x8D, 0xEE, 0x09, 0x17, 0xB1, 0xBB, 0xD5, 0x5F, 0x6B, 0x87, 0xC4, 0x31, 0x37, 0x10, 0x49, 0x04, 0xF5, 0xC3, 0x4D, 0x97, 0xB2, 0x33, 0xCD, 0xDF
    };

    //Compute & Compare SHA Hashes
    sha(computed_key1, key1, 0x10, SHA_256_MODE);
    sha(computed_key2, key2, 0x10, SHA_256_MODE);
    
    int ret = 0;
    
    ret += memcmp(key1_hash, computed_key1, SHA_256_HASH_SIZE);
    ret += memcmp(key2_hash, computed_key2, SHA_256_HASH_SIZE);
    
    if(ret != 0){
        debugWrite("/rei/debug.log", "Secret Sector is corrupted. ", 29);
        shutdown();
    }
}

//Emulates the K9L process and then some
void k9loader(Arm9Bin* sect_arm9){
    u32 k9lVer = 0;
    
    ALIGNED(4) u8 key1[AES_BLOCK_SIZE];
    ALIGNED(4) u8 key2[AES_BLOCK_SIZE];
    
    switch (sect_arm9->magic[3]) {
        case 0xFF:
            k9lVer = 0;
            break;
        case '1':
            k9lVer = 1;
            break;
        default:
            k9lVer = 2;
            break;
    }
    
    u32 *arm9BinStart = (u32*)((u8*)sect_arm9 + 0x800);

    if(*arm9BinStart == 0x47704770 || *arm9BinStart == 0xB0862000)
        return; //Already decrypted

    ALIGNED(4) u8 secretSector[512]; //Setup Buffer.
    sdmmc_nand_readsectors(0x96, 1, secretSector); //Read Secret Sector into Buffer.

    initKeyslot11(); //Init Keyslot 0x11
    aes_use_keyslot(0x11);
    for (u32 i = 0; i < 32; ++i) //Encrypt Key Sector
        aes(secretSector + (AES_BLOCK_SIZE * i), secretSector + (AES_BLOCK_SIZE * i), 1, NULL, AES_ECB_DECRYPT_MODE, 0);

    //Copy keys from buffer.
    memcpy(key1, secretSector, AES_BLOCK_SIZE);
    memcpy(key2, secretSector + AES_BLOCK_SIZE, AES_BLOCK_SIZE);
        
    //Check Key1+Key2 Hashes
    //checkKeyHash(key1, key2);

    //Clear buffer
    memset(secretSector, 0, 512);

    aes_setkey(0x11, (k9lVer == 2) ? key2 : key1, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);
    memset(key1, 0, AES_BLOCK_SIZE);
    memset(key2, 0, AES_BLOCK_SIZE);

    u8 keySlot = k9lVer == 0 ? 0x15 : 0x16;

    // KeyX
    ALIGNED(4) u8 keyX[AES_BLOCK_SIZE];
    aes_use_keyslot(0x11);
    aes(keyX, k9lVer == 0 ? sect_arm9->keyX : sect_arm9->slot0x16keyX, 1, NULL, AES_ECB_DECRYPT_MODE, 0);
    aes_setkey(keySlot, keyX, AES_KEYX, AES_INPUT_BE | AES_INPUT_NORMAL);
    memset(keyX, 0, AES_BLOCK_SIZE);

    // KeyY
    ALIGNED(4) u8 keyY[AES_BLOCK_SIZE];
    memcpy(keyY, sect_arm9->keyY, sizeof(keyY));
    aes_setkey(keySlot, keyY, AES_KEYY, AES_INPUT_BE | AES_INPUT_NORMAL);
    memset(keyY, 0, AES_BLOCK_SIZE);

    // CTR
    ALIGNED(4) u8 CTR[AES_BLOCK_SIZE];
    memcpy(CTR, sect_arm9->ctr, sizeof(CTR));

    aes_use_keyslot(keySlot);
    aes(arm9BinStart, arm9BinStart, decAtoi(sect_arm9->size, sizeof(sect_arm9->size)) / AES_BLOCK_SIZE, CTR, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);
    memset(CTR, 0, AES_BLOCK_SIZE);
    
    if(*arm9BinStart != 0x47704770 && *arm9BinStart != 0xB0862000){
        shutdown();
        debugWrite("/rei/debugCrypto.log", "Failed to decrypt arm9 binary... ", 33);
    }
}

//Decrypt firmware blob
void decryptFirm(void *firm, Size firmSize){
    u8 firmIV[0x10] = {0};
    aes_setkey(0x16, memeKey, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x16);
    aes(firm, firm, firmSize / AES_BLOCK_SIZE, firmIV, AES_CBC_DECRYPT_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);
}