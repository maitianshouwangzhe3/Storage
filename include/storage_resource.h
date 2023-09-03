#ifndef __STORAGE_RESOURCE_H__
#define __STORAGE_RESOURCE_H__

#include "public.h"

class resource{
public:
    static resource* getinstance();
    static void DstoryInstance();
private:
    resource();
    ~resource();

public:
    Container* get_avl_resource();

    hashtable_map* get_hash_resource();

    hashtable_map* get_orderedlist_hash_resource();

private:
    static resource* ress;

    Container* con;

    hashtable_map* tb;

    hashtable_map* orderedlist_zset_tb;
};


#endif