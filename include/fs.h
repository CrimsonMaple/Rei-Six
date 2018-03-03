/*
*   fc.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef FS_INC
#define FS_INC

#include "types.h"

void mountSD(void);
void mountNand(void);
void unmountSD(void);
void unmountNand(void);

u8 fopen(const void *filename, const char *mode);
void fclose(void);
void fseek(u32 offset);
u8 eof(void);
Size fsize(void);
Size fwrite(const void *buffer, Size elementSize, Size elementCnt);
Size fread(const void *buffer, Size elementSize, Size elementCnt);

//char read_firmware(u8 *dest, u32 firm);
void debugWrite(const char *filename, char *buffer, Size size);
void shutdown(void);

#endif
