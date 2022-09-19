#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "debug.h"
#include "bitmap.h"
#include "string.h"
#include "thread.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "sync.h"

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

struct arena{
    struct mem_block_desc* desc;
    uint32_t cnt;
    bool large;
};

struct pool kernel_pool,user_pool;
struct virtual_addr kernel_vaddr;

static struct lock kvlock;

struct mem_block_desc k_block_desc[DESC_CNT];

static void* vaddr_get(enum pool_flags pf,uint32_t pg_cnt){
    int vaddr_start=0,bit_idx_start=-1;
    struct virtual_addr* vaddr;
    if(pf==PF_KERNEL){
        vaddr=&kernel_vaddr;
        lock_acquire(&kvlock);
    }else{
        vaddr=&(running_thread()->userprog_vaddr);
    }
    bit_idx_start=bitmap_scan(&vaddr->vaddr_bitmap,pg_cnt);
    if(bit_idx_start==-1){
        return NULL;
    }
    while(pg_cnt--){
        bitmap_set(&vaddr->vaddr_bitmap,bit_idx_start+pg_cnt,1);
    }
    if(pf==PF_KERNEL){
        lock_release(&kvlock);
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
    lock_acquire(&m_pool->lock);
    int bit_idx=bitmap_scan(&m_pool->pool_bitmap,1);
    if(bit_idx==-1){
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap,bit_idx,1);
    lock_release(&m_pool->lock);
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
    struct pool* mem_pool=(pf&PF_KERNEL)?&kernel_pool:&user_pool;
    lock_acquire(&mem_pool->lock);
    struct task_struct* cur=running_thread();
    int32_t bit_idx=-1;
    if(cur->pgdir!=NULL&&pf==PF_USER){
        bit_idx=(vaddr-cur->userprog_vaddr.vaddr_start)/PG_SIZE;
        ASSERT(bit_idx>=0);
        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap,bit_idx,1);
    }else if(cur->pgdir==NULL&&pf==PF_KERNEL){
        bit_idx=(vaddr-kernel_vaddr.vaddr_start)/PG_SIZE;
        ASSERT(bit_idx>=0);
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

static struct mem_block* arena2block(struct arena* a,uint32_t idx){
    return (struct mem_block*)((uint32_t)a+sizeof(struct arena)+idx*a->desc->block_size);
}

static struct arena* block2arena(struct mem_block* b){
    return (struct arena*)((uint32_t)b&0xfffff000);
}

void* sys_malloc(uint32_t size){
    enum pool_flags PF;
    struct pool* mem_pool;
    uint32_t pool_size;
    struct mem_block_desc* desc;
    struct task_struct* cur=running_thread();
    if(cur->pgdir==NULL){
        PF=PF_KERNEL;
        pool_size=kernel_pool.pool_size;
        mem_pool=&kernel_pool;
        desc=k_block_desc;
    }else{
        PF=PF_USER;
        pool_size=user_pool.pool_size;
        mem_pool=&user_pool;
        desc=cur->u_block_desc;
    }
    if(!(size>0&&size<pool_size)){
        return NULL;
    }
    struct arena* a;
    struct mem_block* b;
    lock_acquire(&mem_pool->lock);

    if(size>1024){
        uint32_t pg_cnt=DIV_ROUND_UP(size+sizeof(struct arena),PG_SIZE);
        a=malloc_page(PF,pg_cnt);
        if(a!=NULL){
            memset(a,0,pg_cnt*PG_SIZE);
        }
        a->desc=NULL;
        a->cnt=pg_cnt;
        a->large=true;
        lock_release(&mem_pool->lock);
        return (void*)(a+1);
    }else{
        uint8_t desc_idx;
        for(desc_idx=0;desc_idx<DESC_CNT;++desc_idx){
            if(size<=desc[desc_idx].block_size){
                break;
            }
        }
        if(list_empty(&desc[desc_idx].free_list)){
            a=malloc_page(PF,1);
            if(a==NULL){
                lock_release(&mem_pool->lock);
                return NULL;
            }
            memset(a,0,PG_SIZE);
            a->desc=&desc[desc_idx];
            a->cnt=desc[desc_idx].blocks_per_arena;
            a->large=false;
            uint32_t block_idx;
            enum intr_status old_status=intr_disable();
            for(block_idx=0;block_idx<desc[desc_idx].blocks_per_arena;++block_idx){
                b=arena2block(a,block_idx);
                ASSERT(!elem_find(&a->desc->free_list,&b->free_elem));
                list_append(&a->desc->free_list,&b->free_elem);
            }
            intr_set_status(old_status);
        }
        b=elem2entry(struct mem_block,free_elem,list_pop(&(desc[desc_idx].free_list)));
        memset(b,0,desc[desc_idx].block_size);
        a=block2arena(b);
        --a->cnt;
        lock_release(&mem_pool->lock);
        return (void*)b;
    }
}

void pfree(uint32_t paddr){
    struct pool* mem_pool;
    uint32_t bit_idx=0;
    if(paddr>=user_pool.phy_addr_start){
        mem_pool=&user_pool;
    }else{
        mem_pool=&kernel_pool;
    }
    bit_idx=(paddr-mem_pool->phy_addr_start)/PG_SIZE;
    bitmap_set(&mem_pool->pool_bitmap,bit_idx,0);
}

static void page_table_pte_remove(uint32_t vaddr){
    uint32_t* pte=pte_ptr(vaddr);
    (*pte)&=~PG_P_1;
    asm volatile("invlpg %0"::"m"(vaddr):"memory");
}

static void vaddr_remove(enum pool_flags pf,void* _vaddr,uint32_t pg_cnt){
    uint32_t vaddr=(uint32_t)_vaddr,bit_idx_start=-1;
    struct virtual_addr* viraddr=pf==PF_KERNEL?&kernel_vaddr: \
&(running_thread()->userprog_vaddr);
    bit_idx_start=(vaddr-viraddr->vaddr_start)/PG_SIZE;
    ASSERT(bitmap_scan_test(&viraddr->vaddr_bitmap,bit_idx_start));
    while(pg_cnt--){
        bitmap_set(&viraddr->vaddr_bitmap,bit_idx_start+pg_cnt,0);
    }
    ASSERT(!bitmap_scan_test(&viraddr->vaddr_bitmap,bit_idx_start));
}

void mfree_page(enum pool_flags pf,void* _vaddr,uint32_t pg_cnt){
    uint32_t paddr,vaddr=(uint32_t)_vaddr,cnt=0;
    ASSERT(pg_cnt>0&&vaddr%PG_SIZE==0);
    paddr=addr_v2p(vaddr);
    ASSERT(paddr%PG_SIZE==0&&paddr>=0x102000);
    struct pool* mem_pool=paddr>=user_pool.phy_addr_start?&user_pool:&kernel_pool;
    while(cnt++<pg_cnt){
        paddr=addr_v2p(vaddr);
        ASSERT(paddr%PG_SIZE==0&&paddr>=mem_pool->phy_addr_start);
        pfree(paddr);
        page_table_pte_remove(vaddr);
        vaddr+=PG_SIZE;
    }
    vaddr_remove(pf,_vaddr,pg_cnt);
}

void sys_free(void* ptr){
    ASSERT(ptr!=NULL);
    if(ptr==NULL){
        return;
    }
    enum pool_flags pf;
    struct pool* mem_pool;
    if(running_thread()->pgdir==NULL){
        ASSERT((uint32_t)ptr>=K_HEAP_START);
        pf=PF_KERNEL;
        mem_pool=&kernel_pool;
    }else{
        pf=PF_USER;
        mem_pool=&user_pool;
    }
    lock_acquire(&mem_pool->lock);
    struct mem_block* b=ptr;
    struct arena* a=block2arena(b);
    ASSERT(a->large==false||a->large==true);
    if(a->desc==NULL&&a->large==true){
        mfree_page(pf,(void*)a,a->cnt);
    }else{
        list_append(&a->desc->free_list,&b->free_elem);
        if(++a->cnt==a->desc->blocks_per_arena){
            uint32_t block_idx;
            for(block_idx=0;block_idx<a->desc->blocks_per_arena;++block_idx){
                struct mem_block* btofree=arena2block(a,block_idx);
                ASSERT(elem_find(&a->desc->free_list,&btofree->free_elem));
                list_remove(&btofree->free_elem);
            }
            mfree_page(pf,(void*)a,1);
        }
    }
    lock_release(&mem_pool->lock);
}

void block_desc_init(struct mem_block_desc* desc_array){
    uint16_t desc_idx,block_size=16;
    for(desc_idx=0;desc_idx<DESC_CNT;++desc_idx){
        desc_array[desc_idx].block_size=block_size;
        desc_array[desc_idx].blocks_per_arena=(PG_SIZE-sizeof(struct arena))/block_size;
        list_init(&desc_array[desc_idx].free_list);
        block_size<<=1;
    }
}

void* get_a_page_without_opvaddrbitmap(enum pool_flags pf,uint32_t vaddr){
    struct pool* mem_pool=pf==PF_KERNEL?&kernel_pool:&user_pool;
    lock_acquire(&mem_pool->lock);
    void* paddr=palloc(mem_pool);
    if(paddr==NULL){
        lock_release(&mem_pool->lock);
        return NULL;
    }
    page_table_add((void*)vaddr,paddr);
    lock_release(&mem_pool->lock);
    return (void*)vaddr;
}

void free_a_phy_page(uint32_t pg_phy_addr){
    struct pool* mem_pool;
    uint32_t bit_idx=0;
    if(pg_phy_addr>=user_pool.phy_addr_start){
        mem_pool=&user_pool;
    }else{
        mem_pool=&kernel_pool;
    }
    bit_idx=(pg_phy_addr-mem_pool->phy_addr_start)/PG_SIZE;
    bitmap_set(&mem_pool->pool_bitmap,bit_idx,0);
}

void mem_init(void){
    put_str("mem_init start\n");
    uint32_t mem_bytes_total=*((uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);
    lock_init(&kvlock);
    block_desc_init(k_block_desc);
    put_str("mem_init done\n");
}
