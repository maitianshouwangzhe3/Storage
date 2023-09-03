#ifndef __STORAGE_READ_MESSAGE_H__
#define __STORAGE_READ_MESSAGE_H__
#include "public.h"
#include "storage_hashtable.h"
#include "storage_resource.h"
#include "storage_avltree.h"
#include "storage_orderedlist.h"
#include "storage_write_message.h"
#include "storage.h"
#include <queue>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>
#include <iostream>

struct read_queue {
    std::queue<storage_message*> q;
    int size;
};

void storage_read_thread_close();
void storage_rq_init();
void storage_insert_read_message(storage_message* q);
void* storage_read_message(void* arg);


#endif