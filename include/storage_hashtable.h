#ifndef __STORAGE_HASHTABLE_H__
#define __STORAGE_HASHTABLE_H__

#include "public.h"
#include <assert.h>
#include <stdlib.h>
#include <string>

const int k_max_load_factor = 8;
const int k_resizing_work = 128;
const size_t k_max_args = 1024;

hashtable_node* hashtable_find(hashtable_map* hp, hashtable_node* key, bool (*cmp)(hashtable_node*, hashtable_node*));
    
void hashtable_add(hashtable_map* hp, hashtable_node* node);
    
hashtable_node* hashtable_pop(hashtable_map* hp, hashtable_node* node, bool (*cmp)(hashtable_node*, hashtable_node*));
    
void hashtable_destroy(hashtable_map* hp);
    
uint64_t str_hash(const uint8_t* data, size_t len);

#endif