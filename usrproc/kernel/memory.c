#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "debug.h"
#include "bitmap.h"
#include "string.h"
#include "sync.h"
#include "thread.h"

#define PG_SIZE 4096
#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START 0xc0100000

#define PDE_IDX(addr) ((addr&0xffc00000)>>22)
#define PTE_IDX(addr) ((addr&0x003ff000)>>12)

struct pool{
    struct bitmap pool_bitmap;
    uint32_t phy_addr_start;
    uint32_t pool_size;
    struct lock lock;
};

struct pool kernel_pool,user_pool;
struct virtual_addr kernel_vaddr;

static void* vaddr_get(enum pool_flags pf,uint32_t pg_cnt){
    int vaddr_start=0,bit_idx_start=-1;
    struct virtual_addr* vaddr=pf==PF_KERNEL?&kernel_vaddr: \
&(running_thread()->userprog_vaddr);
    bit_idx_start=bitmap_scan(&vaddr->vaddr_bitmap,pg_cnt);
    if(bit_idx_start==-1){
        return NULL;
    }
    while(pg_cnt--){
        bitmap_set(&vaddr->vaddr_bitmap,bit_idx_start+pg_cnt,1);
    }
    vaddr_start=vaddr->vaddr_start+bit_idx_start*PG_SIZE;
    return (void*)vaddr_start;
}

uint32_t* pte_ptr(uint32_t vaddr){
    return (uint32_t*)(0xffc00000+((vaddr&0xffc00000)>>10)+PTE_IDX(vaddr)*4);
}

uint32_t* pde_ptr(uint32_t vaddr){
    return (uint32_t*)(0xfffff000+PDE_IDX(vaddr)*4);
}

static void* palloc(struct pool* m_pool){
    int bit_idx=bitmap_scan(&m_pool->pool_bitmap,1);
    if(bit_idx==-1){
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap,bit_idx,1);
    return (void*)(bit_idx*PG_SIZE+m_pool->phy_addr_start);
}

static void page_table_add(void* _vaddr,void* _paddr){
    uint32_t vaddr=(uint32_t)_vaddr,paddr=(uint32_t)_paddr;
    uint32_t* pde=pde_ptr(vaddr),*pte=pte_ptr(vaddr);
    if(*pde&0x00000001){
        ASSERT(!(*pte&0x00000001));
        if(!(*pte&0x00000001)){
            *pte=paddr|PG_US_U|PG_RW_W|PG_P_1;
        }else{
            PANIC("pte repeat");
        }
    }else{
        uint32_t pde_paddr=(uint32_t)palloc(&kernel_pool);
        *pde=pde_paddr|PG_US_U|PG_RW_W|PG_P_1;
        memset((void*)((int)pte&0xfffff000),0,PG_SIZE);
        ASSERT(!(*pte&0x00000001));
        *pte=paddr|PG_US_U|PG_RW_W|PG_P_1;
    }
}

void* malloc_page(enum pool_flags pf,uint32_t pg_cnt){
    ASSERT(pg_cnt>0&&pg_cnt<3840);
    void* vaddr_start=vaddr_get(pf,pg_cnt);
    if(vaddr_start==NULL){
        return NULL;
    }
    uint32_t vaddr=(uint32_t)vaddr_start;
    struct pool* m_pool=pf&PF_KERNEL?&kernel_pool:&user_pool;
    while(pg_cnt--){
        void* paddr=palloc(m_pool);
        if(paddr==NULL){
            return NULL;
        }
        page_table_add((void*)vaddr,paddr);
        vaddr+=PG_SIZE;
    }
    return vaddr_start;
}

void* get_kernel_pages(uint32_t pg_cnt){
    void* vaddr=malloc_page(PF_KERNEL,pg_cnt);
    if(vaddr!=NULL){
        memset(vaddr,0,pg_cnt*PG_SIZE);
    }
    return vaddr;
}

void* get_user_pages(uint32_t pg_cnt){
    lock_acquire(&user_pool.lock);
    void* addr=malloc_page(PF_USER,pg_cnt);
    lock_release(&user_pool.lock);
    memset(addr,0,pg_cnt*PG_SIZE);
    return addr;
}

void* get_a_page(enum pool_flags pf,uint32_t vaddr){
    struct pool* mem_pool=pf&PF_KERNEL?&kernel_pool:&user_pool;
    lock_acquire(&mem_pool->lock);
    struct task_struct* cur=running_thread();
    int32_t bit_idx=-1;
    if(cur->pgdir!=NULL&&pf==PF_USER){
        bit_idx=(vaddr-cur->userprog_vaddr.vaddr_start)/PG_SIZE;
        ASSERT(bit_idx>0);
        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap,bit_idx,1);
    }else if(cur->pgdir==NULL&&pf==PF_KERNEL){
        bit_idx=(vaddr-kernel_vaddr.vaddr_start)/PG_SIZE;
        ASSERT(bit_idx>0);
        bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx,1);
    }else{
        PANIC("get_a_page:not allow kernel alloc userspace or user alloc kernel space\n");
    }
    void* paddr=palloc(mem_pool);
    if(paddr==NULL){
        return NULL;
    }
    page_table_add((void*)vaddr,paddr);
    lock_release(&mem_pool->lock);
    return (void*)vaddr;
}

uint32_t addr_v2p(uint32_t vaddr){
    uint32_t* pte=pte_ptr(vaddr);
    return (*pte&0xfffff000)+(vaddr&0x00000fff);
}

static void mem_pool_init(uint32_t all_mem){
    put_str("mem_pool_init start\n");
    uint32_t page_table_size=PG_SIZE*256;
    uint32_t used_mem=page_table_size+0x100000;
    uint32_t free_mem=all_mem-used_mem;
    uint16_t all_free_pages=free_mem/PG_SIZE;
    uint16_t kernel_free_pages=all_free_pages>>1;
    uint16_t user_free_pages=all_free_pages>>1;
    uint32_t kbm_length=kernel_free_pages>>3;
    uint32_t ubm_length=user_free_pages>>3;
    uint32_t kp_start=used_mem;
    uint32_t up_start=kp_start+kernel_free_pages*PG_SIZE;
    kernel_pool.pool_bitmap.btmp_bytes_len=kbm_length;
    kernel_pool.pool_bitmap.bits=(void*)MEM_BITMAP_BASE;
    kernel_pool.phy_addr_start=kp_start;
    kernel_pool.pool_size=kernel_free_pages*PG_SIZE;
    lock_init(&kernel_pool.lock);
    user_pool.pool_bitmap.btmp_bytes_len=ubm_length;
    user_pool.pool_bitmap.bits=(void*)(MEM_BITMAP_BASE+kbm_length);
    user_pool.phy_addr_start=up_start;
    user_pool.pool_size=user_free_pages*PG_SIZE;
    lock_init(&user_pool.lock);
    put_str("kernel_pool_bitmap_start:");
    put_int((int)kernel_pool.pool_bitmap.bits);
    put_str("  kernel_pool_phy_addr_start:");
    put_int(kernel_pool.phy_addr_start);
    put_char('\n');
    put_str("user_pool_bitmap_start:");
    put_int((int)user_pool.pool_bitmap.bits);
    put_str("  user_pool_phy_addr_start:");
    put_int(user_pool.phy_addr_start);
    put_char('\n');
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len=kbm_length;
    kernel_vaddr.vaddr_bitmap.bits=(void*)(MEM_BITMAP_BASE+kbm_length+ubm_length);
    kernel_vaddr.vaddr_start=K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    put_str("mem_pool_init done\n");
}

void mem_init(void){
    put_str("mem_init start\n");
    uint32_t mem_bytes_total=*((uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);
    put_str("mem_init done\n");
}
