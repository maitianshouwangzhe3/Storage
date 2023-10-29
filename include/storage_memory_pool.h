#ifndef __STORAGE_MEMORY_POOL_H__
#define __STORAGE_MEMORY_POOL_H__

struct memory_node {
    int size;
    memory_node* next;
    void* data;
};

struct memory_pool {
    int mask;
    memory_pool* next;
    memory_node* data;
};

bool storage_memory_init();
void storage_memory_dstory();
void** storage_malloc(unsigned int size);
void storage_free(void** node);
int storage_get_memory_node_size(void** node);

#endif