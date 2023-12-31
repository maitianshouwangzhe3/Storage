#include "storage_memory_pool.h"
#include "public.h"
#include <stdio.h>
#include <memory>
#include <memory.h>
#include <pthread.h> 
static pthread_mutex_t mtx;

static memory_pool* mp = nullptr;

static unsigned int alignUpToPowerOfTwo(unsigned int num) {  
    if (num == 0) {  
        return 0;  
    }  
    num--;  
    num |= num >> 1;  
    num |= num >> 2;  
    num |= num >> 4;  
    num |= num >> 8;  
    num |= num >> 16;  
    num++;  
    return num;  
} 

static bool hasOnlyOneBit(unsigned int n) {
    return (n & (n - 1)) == 0;
}

bool storage_memory_init() {
    if(!mp) {
        mp = new memory_pool();
        if(!mp) {
            return false;
        }
        
        mp->data = nullptr;
        mp->next = nullptr;
        mp->mask = 1 << 5;
    }
    return true;
}

void storage_memory_dstory() {
    memory_pool* cur = mp;
    while(cur) {
        memory_node* tmp = cur->data;
        while(tmp) {
            free(tmp->data);
            memory_node* t = tmp->next;
            free(tmp);
            tmp = t;
        }
        memory_pool* c = cur->next;
        delete cur;
        cur = c;
    }
}

void** storage_malloc(unsigned int size) {
    //向上对齐最近的2的幂
    if(!hasOnlyOneBit(size)) {
        size = alignUpToPowerOfTwo(size);
    }

    memory_pool* cur = mp;

    //最小分配单位32
    if(size < (1 << 5)) {
        size = 1 << 5;
        cur = mp;
    }

    pthread_mutex_lock(&mtx);
    memory_node* val = cur->data;
    while(cur && cur->mask <= size) {
        cur = cur->next;
        if(cur) {
            val = cur->data;
        }
        else {
            val = nullptr;
        }
    }

    if(val) {
        cur->data = val->next;
        val->next = nullptr;
        pthread_mutex_unlock(&mtx);
        return &val->data;
    }

    try {
        memory_pool* t = new memory_pool();
        if(!t) {
            pthread_mutex_unlock(&mtx);
            return nullptr;
        }
        t->mask = size;
        memory_pool* c = mp;
        while(c && c->next && c->mask < c->next->mask) {
            c = c->next;
        }
        t->next = c->next;
        c->next = t;

        val = (memory_node*)malloc(sizeof(memory_node));
        if(!val) {
            pthread_mutex_unlock(&mtx);
            return nullptr;
        }

        memset(val, 0, sizeof(val));
        val->size = size;
        val->next = nullptr;
        void* tmp = malloc(size);
        val->data = tmp;
        if(!val->data) {
            free(val);
            pthread_mutex_unlock(&mtx);
            return nullptr;
        }
    }catch(...) {
        printf("Memory allocation failed\n");
        pthread_mutex_unlock(&mtx);
        return nullptr;
    }
    pthread_mutex_unlock(&mtx);
    return &val->data;
}

void storage_free(void** node) {
    if(!node || !*node) {
        return;
    }

    pthread_mutex_lock(&mtx);
    try {
        memory_node* tmp = container_of(node, memory_node, data);
        memory_pool* cur = mp;
        while(cur && cur->mask != tmp->size && cur->mask < tmp->size) {
            cur = cur->next;
        }

        if(!cur) {
            free(tmp->data);
            free(tmp);
            pthread_mutex_unlock(&mtx);
            return;
        }

        tmp->next = cur->data;
        cur->data = tmp;
    }
    catch(...) {
        pthread_mutex_unlock(&mtx);
        printf("storage_free error\n");
    }
    pthread_mutex_unlock(&mtx);
}

int storage_get_memory_node_size(void** node) {
    if(!node) {
        return 0;
    }

    memory_node* tmp = container_of(node, memory_node, data);
    return tmp->size;
}