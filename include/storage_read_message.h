#ifndef __STORAGE_READ_MESSAGE_H__
#define __STORAGE_READ_MESSAGE_H__
#include "public.h"
#include <queue>

struct read_queue {
    std::queue<storage_message*> q;
    int size;
};

void storage_read_thread_close();
void storage_rq_init();
void storage_insert_read_message(storage_message* q);
void* storage_read_message(void* arg);


#endif