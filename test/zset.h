#ifndef __ZSET_H__
#define __ZSET_H__


#include "public.h"
#include "hashtable.h"
#include "avltree.h"
#include <string.h>
#include <algorithm>


struct HKey {
    HNode node;
    const char *name = NULL;
    size_t len = 0;
};

struct ZSet
{
    AVLNode* tree = nullptr;
    HMap node;
};

struct ZNode
{
    AVLNode tree;
    HNode hMap;
    double score = 0.0;
    size_t len = 0;
    char name[0];
};

struct ZsetEntry {
    struct HNode node;
    std::string key;
    std::string val;
    uint32_t type = 0;
    ZSet* zet = nullptr;
};

class OrderedList{
public:
    OrderedList();
    ~OrderedList();

    //查找节点
    ZNode* ZNode_lookup(const char* name, size_t len);

    //添加节点进列表
    bool ZNode_add(const char* key, const char* name, size_t len, double score);

    //删除节点
    bool ZNode_pop(const char* key, const char* name, size_t len);

    //找到大于或等于参数的 (score, name) 元组，然后相对于它进行偏移。
    ZNode* ZNode_query(ZSet* pZSet, double sroce, const char* name, size_t len, int64_t offset);

private:   
    //创建新节点
    ZNode* ZNode_new(const char* name, size_t len, double score);

    void zset_tree_add(ZSet* pzset, ZNode* node);

    void zset_updata(ZSet* pzset, ZNode* node, double score);

    bool zless(AVLNode* lhs, AVLNode* rhs);

    bool zless(AVLNode* lhs, double score, const char* name, size_t len);


private:
    HMap* g_data;
    AvlTree* con;
    HashTable* tab;
};


#endif