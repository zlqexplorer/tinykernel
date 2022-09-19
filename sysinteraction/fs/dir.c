#include "dir.h"
#include "ide.h"
#include "inode.h"
#include "memory.h"
#include "fs.h"
#include "string.h"
#include "debug.h"
#include "bitmap.h"
#include "stdio_kernel.h"
#include "stdint.h"
#include "file.h"

struct dir root_dir;

extern struct partition* cur_part;

void open_root_dir(struct partition* part){
    root_dir.inode=inode_open(part,part->sb->root_inode_no);
    root_dir.dir_pos=0;
}

struct dir* dir_open(struct partition* part,uint32_t inode_no){
    struct dir* pdir=(struct dir*)sys_malloc(sizeof(struct dir));
    pdir->inode=inode_open(part,inode_no);
    pdir->dir_pos=0;
    return pdir;
}

bool search_dir_entry(struct partition* part,struct dir* pdir,const char* name,struct dir_entry* dir_e){
    uint32_t block_cnt=140;
    uint32_t* all_blocks=(uint32_t*)sys_malloc(48+512);
    if(all_blocks==NULL){
        printk("search_dir_entry: sys_malloc for all_blocks failed\n");
        return false;
    }
    uint32_t block_idx=0;
    while(block_idx<12){
        all_blocks[block_idx]=pdir->inode->i_sectors[block_idx];
        ++block_idx;
    }
    if(pdir->inode->i_sectors[12]!=0){
        ide_read(part->my_disk,pdir->inode->i_sectors[12],(void*)(all_blocks+12),1);
    }
    uint8_t* buf=(uint8_t*)sys_malloc(SECTOR_SIZE);
    struct dir_entry* p_de=(struct dir_entry*)buf;
    uint32_t dir_entry_size=part->sb->dir_entry_size;
    uint32_t dir_entry_cnt=SECTOR_SIZE/dir_entry_size;
    block_idx=0;
    while(block_idx<block_cnt){
        if(all_blocks[block_idx]!=0){
            ide_read(part->my_disk,all_blocks[block_idx],buf,1);
            uint32_t dir_entry_idx=0;
            while(dir_entry_idx<dir_entry_cnt){
                if(!strcmp(p_de->filename,name)){
                    memcpy(dir_e,p_de,dir_entry_size);
                    sys_free(buf);
                    sys_free(all_blocks);
                    return true;
                }
                ++dir_entry_idx;
                ++p_de;
            }
            memset(buf,0,SECTOR_SIZE);
            p_de=(struct dir_entry*)buf;
        }
        ++block_idx;
    }
    sys_free(buf);
    sys_free(all_blocks);
    return false;
}

void dir_close(struct dir* dir){
    if(dir==&root_dir){
        return;
    }
    inode_close(dir->inode);
    sys_free(dir);
}

void create_dir_entry(char* filename,uint32_t inode_no,uint8_t file_type,struct dir_entry* p_de){
    ASSERT(strlen(filename)<=MAX_FILE_NAME_LEN);
    strcpy(p_de->filename,filename);
    p_de->i_no=inode_no;
    p_de->f_type=file_type;
}

bool delete_dir_entry(struct partition* part,struct dir* pdir,uint32_t inode_no,void* io_buf){
    struct inode* dir_inode=pdir->inode;
    uint32_t block_idx=0,all_blocks[140]={0};
    while(block_idx<12){
        all_blocks[block_idx]=dir_inode->i_sectors[block_idx];
        ++block_idx;
    }
    if(dir_inode->i_sectors[12]!=0){
        ide_read(part->my_disk,dir_inode->i_sectors[12],(void*)(all_blocks+12),1);
    }

    uint32_t dir_entry_size=part->sb->dir_entry_size;
    uint32_t dir_entry_per_sec=SECTOR_SIZE/dir_entry_size;
    struct dir_entry* dir_e=(struct dir_entry*)io_buf;
    struct dir_entry* dir_entry_found=NULL;
    uint8_t dir_entry_idx,dir_entry_cnt;
    bool is_dir_first_block=false;

    block_idx=0;
    while(block_idx<140){
        is_dir_first_block=false;
        if(all_blocks[block_idx]!=0){
            memset(io_buf,0,SECTOR_SIZE);
            ide_read(part->my_disk,all_blocks[block_idx],io_buf,1);
            dir_entry_idx=dir_entry_cnt=0;
            while(dir_entry_idx<dir_entry_per_sec){
                if((dir_e+dir_entry_idx)->f_type!=FT_UNKNOWN){
                    if(!strcmp((dir_e+dir_entry_idx)->filename,".")){
                        is_dir_first_block=true;
                    }else if(strcmp((dir_e+dir_entry_idx)->filename,".")&& \
                    strcmp((dir_e+dir_entry_idx)->filename,"..")){
                        ++dir_entry_cnt;
                        if((dir_e+dir_entry_idx)->i_no==inode_no){
                            ASSERT(dir_entry_found==NULL);
                            dir_entry_found=dir_e+dir_entry_idx;
                        }
                    }
                }
                ++dir_entry_idx;
            }
            if(dir_entry_found!=NULL){
                ASSERT(dir_entry_cnt>=1);
                if(dir_entry_cnt==1&&!is_dir_first_block){
                    uint32_t block_bitmap_idx=all_blocks[block_idx]-part->sb->data_start_lba;
                    bitmap_set(&part->block_bitmap,block_bitmap_idx,0);
                    bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
                    if(block_idx<12){
                        dir_inode->i_sectors[block_idx]=0;
                    }else{
                        uint32_t indirect_blocks=0;
                        uint32_t indirect_block_idx=12;
                        while(indirect_block_idx<140){
                            if(all_blocks[indirect_block_idx]!=0){
                                ++indirect_blocks;
                            }
                        }
                        ASSERT(indirect_blocks>=1);
                        if(indirect_blocks>1){
                            all_blocks[block_idx]=0;
                            ide_write(part->my_disk,dir_inode->i_sectors[12],(void*)(all_blocks+12),1);
                        }else{
                            block_bitmap_idx=dir_inode->i_sectors[12]-part->sb->data_start_lba;
                            bitmap_set(&part->block_bitmap,block_bitmap_idx,0);
                            bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
                            dir_inode->i_sectors[12]=0;
                        }
                    }
                }else{
                    memset(dir_entry_found,0,dir_entry_size);
                    ide_write(part->my_disk,all_blocks[block_idx],io_buf,1);
                }
                ASSERT(dir_inode->i_size>=dir_entry_size);
                dir_inode->i_size-=dir_entry_size;
                memset(io_buf,0,SECTOR_SIZE*2);
                inode_sync(part,dir_inode,io_buf);
                return true;
            }
            ++block_idx;
        }
    }
    return false;
}

bool sync_dir_entry(struct dir* parent_dir,struct dir_entry* p_de,void* io_buf){
    struct inode* dir_inode=parent_dir->inode;
    uint32_t dir_size=dir_inode->i_size;
    uint32_t dir_entry_size=cur_part->sb->dir_entry_size;
    ASSERT(dir_size%dir_entry_size==0);
    uint32_t dir_entry_per_sec=SECTOR_SIZE/dir_entry_size;
    int32_t block_lba=-1;

    uint8_t block_idx=0;
    uint32_t all_blocks[140]={0};

    while(block_idx<12){
        all_blocks[block_idx]=dir_inode->i_sectors[block_idx];
        ++block_idx;
    }
    struct dir_entry* dir_e=(struct dir_entry*)io_buf;
    int32_t block_bitmap_idx=-1;
    block_idx=0;
    while(block_idx<140){
        block_bitmap_idx=-1;
        if(all_blocks[block_idx]==0){
            block_lba=block_bitmap_alloc(cur_part);
            if(block_lba==-1){
                printk("alloc block bitmap for sync_dir_entry failed\n");
                return false;
            }

            block_bitmap_idx=block_lba-cur_part->sb->data_start_lba;
            ASSERT(block_bitmap_idx!=-1);
            bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);

            if(block_idx<12){
                dir_inode->i_sectors[block_idx]=all_blocks[block_idx]=block_lba;
            }else{
                if(block_idx==12){
                    dir_inode->i_sectors[block_idx]=block_lba;
                    block_lba=-1;
                    block_lba=block_bitmap_alloc(cur_part);
                    if(block_lba==-1){
                        bitmap_set(&cur_part->block_bitmap,block_bitmap_idx,0);
                        dir_inode->i_sectors[12]=0;
                        bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
                        printk("alloc block bitmap for sync_dir_entry failed\n");
                        return false;
                    }
                    block_bitmap_idx=-1;
                    block_bitmap_idx=block_lba-cur_part->sb->data_start_lba;
                    ASSERT(block_bitmap_idx!=-1);
                    bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
                }
                all_blocks[block_idx]=block_lba;
                ide_write(cur_part->my_disk,dir_inode->i_sectors[12],(void*)(all_blocks+12),1);
            }
            memset(io_buf,0,SECTOR_SIZE);
            memcpy(io_buf,p_de,dir_entry_size);
            ide_write(cur_part->my_disk,all_blocks[block_idx],io_buf,1);
            dir_inode->i_size+=dir_entry_size;
            return true;
        }
        ide_read(cur_part->my_disk,all_blocks[block_idx],io_buf,1);
        uint8_t dir_entry_idx=0;
        while(dir_entry_idx<dir_entry_per_sec){
            if((dir_e+dir_entry_idx)->f_type==FT_UNKNOWN){
                memcpy(dir_e+dir_entry_idx,p_de,dir_entry_size);
                ide_write(cur_part->my_disk,all_blocks[block_idx],io_buf,1);
                dir_inode->i_size+=dir_entry_size;
                return true;
            }
            ++dir_entry_idx;
        }
        ++block_idx;
    }
    printk("directory is full!\n");
    return false;
}

struct dir_entry* dir_read(struct dir* dir){
    struct dir_entry* dir_e=(struct dir_entry*)dir->dir_buf;
    struct inode* dir_inode=dir->inode;
    uint32_t all_blocks[140]={0},block_cnt=12;
    uint32_t block_idx=0,dir_entry_idx=0;
    while(block_idx<block_cnt){
        all_blocks[block_idx]=dir_inode->i_sectors[block_idx];
        ++block_idx;
    }
    if(dir_inode->i_sectors[12]!=0){
        ide_read(cur_part->my_disk,dir_inode->i_sectors[12],all_blocks+12,1);
        block_cnt=140;
    }
    block_idx=0;
    uint32_t cur_dir_entry_pos=0;
    uint32_t dir_entry_size=cur_part->sb->dir_entry_size;
    uint32_t dir_entry_per_sec=SECTOR_SIZE/dir_entry_size;
    while(dir->dir_pos<dir_inode->i_size){
        if(all_blocks[block_idx]!=0){
            memset(dir_e,0,SECTOR_SIZE);
            ide_read(cur_part->my_disk,all_blocks[block_idx],dir_e,1);
            dir_entry_idx=0;
            while(dir_entry_idx<dir_entry_per_sec){
                if((dir_e+dir_entry_idx)->f_type!=FT_UNKNOWN){
                    if(cur_dir_entry_pos==dir->dir_pos){
                        dir->dir_pos+=dir_entry_size;
                        return dir_e+dir_entry_idx;
                    }
                    ASSERT(cur_dir_entry_pos<dir->dir_pos);
                    cur_dir_entry_pos+=dir_entry_size;
                }
                ++dir_entry_idx;
            }
        }
        ++block_idx;   
    }
    return NULL;
}

bool dir_is_empty(struct dir* dir){
    return dir->inode->i_size==2*cur_part->sb->dir_entry_size;
}

int32_t dir_remove(struct dir* parent_dir,struct dir* child_dir){
    struct inode* child_inode=child_dir->inode;
    uint32_t block_idx=1;
    while(block_idx<13){
        ASSERT(child_inode->i_sectors[block_idx]==0);
        ++block_idx;
    }
    void* io_buf=sys_malloc(SECTOR_SIZE*2);
    if(io_buf==NULL){
        printk("dir_remove: sys_malloc for io_buf faile\n");
        return -1;
    }
    delete_dir_entry(cur_part,parent_dir,child_inode->i_no,io_buf);
    inode_release(cur_part,child_inode->i_no);
    sys_free(io_buf);
    return 0;
}
