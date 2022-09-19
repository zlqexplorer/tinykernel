#include "list.h"
#include "stdint.h"
#include "interrupt.h"


void list_init(struct list* plist){
    plist->head.prev=NULL;
    plist->head.next=&plist->tail;
    plist->tail.prev=&plist->head;
    plist->tail.next=NULL;
}

void list_insert_before(struct list_elem* before,struct list_elem* elem){
    enum intr_status old_status=intr_disable();
    before->prev->next=elem;
    elem->prev=before->prev;
    elem->next=before;
    before->prev=elem;
    intr_set_status(old_status);
}

void list_push(struct list* plist,struct list_elem* elem){
    list_insert_before(plist->head.next,elem);
}

void list_append(struct list* plist,struct list_elem* elem){
    list_insert_before(&plist->tail,elem);
}

void list_remove(struct list_elem* elem){
    enum intr_status old_status=intr_disable();
    elem->prev->next=elem->next;
    elem->next->prev=elem->prev;
    intr_set_status(old_status);
}

struct list_elem* list_pop(struct list* plist){
    struct list_elem* res=plist->head.next;
    list_remove(res);
    return res;
}

bool list_empty(struct list* plist){
    return plist->head.next==&plist->tail;
}

uint32_t list_len(struct list* plist){
    struct list_elem* cur=plist->head.next;
    int res=0;
    while(cur!=&plist->tail){
        cur=cur->next;
        ++res;
    }
    return res;
}

struct list_elem* list_traversal(struct list* plist,function func,int arg){
    struct list_elem* cur=plist->head.next;
    while(cur!=&plist->tail){
        if(func(cur,arg)){
            return cur;
        }
        cur=cur->next;
    }
    return NULL;
}

bool elem_find(struct list* plist,const struct list_elem* elem){
    const struct list_elem* cur=plist->head.next;
    while(cur!=&plist->tail){
        if(cur==elem){
            return true;
        }
        cur=cur->next;
    }
    return false;
}
