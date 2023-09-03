#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include "public.h"
#include "spinlock.h"
#include <assert.h>
#include <stdlib.h>
#include <string>

const int k_max_load_factor = 8;
const int k_resizing_work = 128;
const size_t k_max_args = 1024;

struct HNode
{
    HNode* next = nullptr;
    uint64_t    code = 0;
};

struct HTable
{
    HNode** table = nullptr;
    size_t mask = 0;
    size_t size = 0;
};

struct HMap
{
    HTable table1;
    HTable table2;

    size_t ResizingPos = 0;    
};

struct Entry
{
    struct HNode node;
    std::string key;
    std::string value;
};

class HashTable{
public:

    HashTable();
    ~HashTable();

    //暴露给外界的插入函数
    bool hm_insert(std::string& key, std::string& val);
    bool hm_insert(HMap* hp, HNode* node);

    //暴露给外界的删除函数
    bool hm_pop(std::string& key);                                              //会直接释放资源
    HNode* hm_pop(HMap* hp, HNode* node, bool (*cmp)(HNode*, HNode*));          //不会释放资源

    //暴露给外界的查找函数
    HNode* hm_lookup(std::string& key);
    HNode* hm_lookup(HMap* hp, HNode* key, bool (*cmp)(HNode*, HNode*));

    //计算哈希值
    uint64_t str_hash(const uint8_t* data, size_t len);

    HMap* get();

private:
    //善后函数
    void hm_destroy();

    //比较节点
    bool entry_eq(HNode* lhs, HNode* rhs);

    //初始化哈希表
    void hm_init(HTable* tle, int n);

    //内部插入函数
    void h_insert(HTable* tb, HNode* node);

    HNode* hm_detach(HTable* tb, HNode** from);

    void hm_help_resizing();
    void hm_help_resizing(HMap* hp);
    void hm_start_resizing();

    //寻找节点
    HNode** h_lookup(HTable* tb, HNode* ent);
    HNode** h_lookup(HTable* tb, HNode* key, bool (*cmp)(HNode*, HNode*));

private:
    HMap* g_data;
    spinlock* lock;
};



#endif