#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

void bitmap_init(struct bitmap* btmp){
    memset(btmp->bits,0,btmp->btmp_bytes_len);
}

bool bitmap_scan_test(struct bitmap* btmp,uint32_t bit_idx){
    return btmp->bits[bit_idx>>3]&(BITMAP_MASK<<(bit_idx&0x7))?true:false;
}

int bitmap_scan(struct bitmap* btmp,uint32_t cnt){
    uint32_t idx_bit=0,bits_len=btmp->btmp_bytes_len,count=cnt;
    while(idx_bit<bits_len&&btmp->bits[idx_bit]==0xff){
        ++idx_bit;
    }
    idx_bit<<=3;
    bits_len<<=3;
    while(idx_bit<bits_len){
        if(!bitmap_scan_test(btmp,idx_bit)){
            if(--count==0){
                return (int)(idx_bit-cnt+1);
            }
        }else{
            count=cnt;
        }
        ++idx_bit;
    }
    return -1;
}

void bitmap_set(struct bitmap* btmp,uint32_t bit_idx,uint8_t value){
    ASSERT(value==0||value==1);
    if(value){
	   btmp->bits[bit_idx>>3]|=1<<(bit_idx&0x7);
    }else{
	   btmp->bits[bit_idx>>3]&=~(1<<(bit_idx&0x7));
     }
}
