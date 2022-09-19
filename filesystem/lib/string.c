#include "string.h"
#include "stdint.h"
#include "debug.h"

void memset(void* _dst,uint8_t value,uint32_t size){
    ASSERT(_dst!=NULL);
    uint8_t* dst=(uint8_t*)_dst;
    while(size--){
        *dst++=value;
    }
}

void memcpy(void* _dst,const void* _src,uint32_t size){
    ASSERT(_dst!=NULL&&_src!=NULL);
    uint8_t* dst=(uint8_t*)_dst;
    const uint8_t* src=(const uint8_t*)_src;
    while(size--){
        *dst++=*src++;
    }
}

int8_t memcmp(const void* _a,const void* _b,uint32_t size){
    ASSERT(_a!=NULL&&_b!=NULL);
    const uint8_t* a=(const uint8_t*)_a;
    const uint8_t* b=(const uint8_t*)_b;
    while(size--&&*a==*b){
        ++a;
        ++b;
    }
    return *a<*b?-1:*a>*b;
}

char* strcpy(char* _dst,const char* src){
    ASSERT(_dst!=NULL&&src!=NULL);
    char* dst=_dst;
    while((*dst++=*src++));
    return _dst;
}

uint32_t strlen(const char* str){
    ASSERT(str!=NULL);
    const char* ptr=str;
    while(*ptr++);
    return ptr-str-1;
}

int8_t strcmp(const char* a,const char* b){
    ASSERT(a!=NULL&&b!=NULL);
    while(*a!=0&&*a==*b){
        ++a;++b;
    }
    return *a<*b?-1:*a>*b;
}

char* strchr(const char* str,const uint8_t ch){
    ASSERT(str!=NULL);
    while(*str){
        if(*str==ch){
            return (char*)str;
        }
        ++str;
    }
    return NULL;
}

char* strrchr(const char* str,const uint8_t ch){
    ASSERT(str!=NULL);
    const char *res=NULL;
    while(*str){
        if(*str==ch){
            res=str;
        }
        ++str;
    }
    return (char*)res;
}

char* strcat(char* _dst,const char* src){
    ASSERT(_dst!=NULL&&src!=NULL);
    char* dst=_dst;
    while(*dst){
        ++dst;
    }
    while((*dst++=*src++));
    return _dst;
}

uint32_t strchrs(const char* str,uint8_t ch){
    ASSERT(str!=NULL);
    uint32_t cnt=0;
    while(*str){
        if(*str==ch){
            ++cnt;
        }
        ++str;
    }
    return cnt;
}
