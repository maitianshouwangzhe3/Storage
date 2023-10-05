#ifndef __STORAGE_WRITE_MESSAGE_H__
#define __STORAGE_WRITE_MESSAGE_H__

#include "public.h"

#include <queue>


struct write_queue {
    std::queue<storage_message*> q;
    int size;
};

void storage_write_thread_close();
void storage_wq_init();
void storage_insert_write_message(storage_message* q);
void* storage_write_message(void* arg);
void storage_send(storage_message* q);


#endif