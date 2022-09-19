#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H

#include "stdint.h"
#include "sync.h"

#define BUFSIZE 64

struct ioqueue{
    struct lock lock;
    struct task_struct* producer;
    struct task_struct* consumer;
    char buf[BUFSIZE];
    uint32_t head;
    uint32_t tail;
};

void ioqueue_init(struct ioqueue* ioq);
bool ioq_full(struct ioqueue* ioq);
bool ioq_empty(struct ioqueue* ioq);
char ioq_getchar(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq,char byte);
uint32_t ioq_length(struct ioqueue* ioq);

#endif
