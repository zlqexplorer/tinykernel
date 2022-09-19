#ifndef __LIB_STRING_H
#define __LIB_STRING_H

#include "stdint.h"

void memset(void* _dst,uint8_t value,uint32_t size);
void memcpy(void* _dst,const void* _src,uint32_t size);
int8_t memcmp(const void* _a,const void* _b,uint32_t size);

char* strcpy(char* _dst,const char* src);
uint32_t strlen(const char* str);
int8_t strcmp(const char* a,const char* b);
char* strchr(const char* str,const uint8_t ch);
char* strrchr(const char* str,const uint8_t ch);
char* strcat(char* _dst,const char* src);
uint32_t strchrs(const char* str,uint8_t ch);

#endif