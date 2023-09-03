#include "zset.h"

//比较函数内部使用
static bool hcmp(HNode* node, HNode* key)
{
    if(node->code != key->code)
    {
        return false;
    }

    ZNode* nodeTemp = container_of(node, ZNode, hMap);
    HKey* hkey = container_of(key, HKey, node);
    if(nodeTemp->len != hkey->len)
    {
        return false;
    }

    return 0 == memcmp(nodeTemp->name, hkey->name, nodeTemp->len);
}

OrderedList::OrderedList(){
    g_data = new HMap();
    con = new AvlTree();
    tab = new HashTable();
}

OrderedList::~OrderedList(){
    if(g_data != nullptr){
        delete g_data;
    }

    if(con != nullptr){
        delete con;
    }

    if(tab != nullptr){
        delete tab;
    }
}

ZNode* OrderedList::ZNode_lookup(const char* name, size_t len){
    HKey key;
    key.node.code = tab->str_hash((uint8_t*)name, len);
    key.name = name;
    key.len = len;

    HNode* found = tab->hm_lookup(g_data, &key.node, &hcmp);
    if(found == nullptr)
    {
        return nullptr;
    }

    return container_of(found, ZNode, hMap);
}

bool OrderedList::ZNode_add(const char* key, const char* name, size_t len, double score){
    ZsetEntry ent;
    ent.key = key;
    ent.node.code = tab->str_hash((const uint8_t*)key, strlen(key));
    HNode* node = tab->hm_lookup(g_data, &ent.node, hcmp);

    //为空就创建节点，并插入哈希表，有就获取，且type不等于T_ZSET返回false
    ZsetEntry* res = nullptr;
    if(!node){
        res = new ZsetEntry();
        res->key.swap(ent.key);
        res->node.code = ent.node.code;
        res->type = T_ZSET;
        res->zet = new ZSet();
        tab->hm_insert(g_data, &res->node);
    }
    else{
        res = container_of(node, ZsetEntry, node);
        if(res->type != T_ZSET){
            return false;
        }
    }

    //把节点上树,且存在就更新，不存在就创建
    ZNode* znode = ZNode_lookup(name, len);
    if(znode){
        zset_updata(res->zet, znode, score);
        return true;
    }
    else{
        znode = ZNode_new(name, len, score);
        tab->hm_insert(&res->zet->node, &znode->hMap);
        zset_tree_add(res->zet, znode);
        return true;
    }
    return false;
}

bool OrderedList::ZNode_pop(const char* key, const char* name, size_t len){
    ZsetEntry ent;
    ent.key = key;
    ent.node.code = tab->str_hash((const uint8_t*)key, strlen(key));
    HNode* node = tab->hm_lookup(g_data, &ent.node, hcmp);

    ZsetEntry* znode = container_of(node, ZsetEntry, node);
    if(!znode){
        return false;
    }

    HKey k;
    k.node.code = tab->str_hash((const uint8_t*)name, len);
    k.name = name;
    k.len = len;

    HNode* fount = tab->hm_pop(g_data, &k.node, hcmp);
    if(fount == nullptr)
    {
        return false;
    }

    //偏移找到树上的节点
    ZNode* nodes = container_of(fount, ZNode, hMap);
    AVLNode* av = con->avl_find(&nodes->tree);
    if(nullptr == av)
    {
        return false;
    }

    delete znode;    
    return true;
}

//找到大于或等于参数的 (score, name) 元组，然后相对于它进行偏移。
ZNode* OrderedList::ZNode_query(ZSet* pZSet, double sroce, const char* name, size_t len, int64_t offset){
    AVLNode* fount = nullptr;
    AVLNode* cur = pZSet->tree;
    //cur = tab->hm_lookup(g_data, nullptr, hcmp);

    while(cur != nullptr)
    {
        if(zless(cur, sroce, name, len))
        {
            cur = cur->right;
        }
        else
        {
            fount = cur;
            cur = cur->left;
        }
    }

    if(fount != nullptr)
    {
        fount = con->avl_offset_t(fount, offset);
    }

    return fount != nullptr ? container_of(fount, ZNode, tree) : nullptr;
}

void OrderedList::zset_updata(ZSet* pzset, ZNode* node, double score){
    if(node->score == score)
    {
        return;
    }
    pzset->tree = con->avl_find(&node->tree);
    node->score = score;
    con->avl_init(&node->tree);
    zset_tree_add(pzset, node);
}

ZNode* OrderedList::ZNode_new(const char* name, size_t len, double score){
    //ZNode* node = (ZNode*)malloc(sizeof(ZNode) + len);
    ZNode* node = new ZNode();
    if(node == nullptr)
    {
        return nullptr;
    }

    con->avl_init(&node->tree);
    node->hMap.next = nullptr;
    node->hMap.code = tab->str_hash((uint8_t*)name, len);
    node->score = score;
    node->len = len;
    memcpy(&node->name[0], name, len);
    return node;
}

void OrderedList::zset_tree_add(ZSet* pzset, ZNode* node){
    if(pzset->tree == nullptr)
    {
        pzset->tree = &node->tree;
        return;
    }

    AVLNode* cur = pzset->tree;
    while(true)
    {
        AVLNode** from = zless(&node->tree, cur) ? &cur->left : &cur->right;
        if(*from == nullptr)
        {
            *from = &node->tree;
            node->tree.parent = cur;
            pzset->tree = con->avl_fix(&node->tree);
            break;
        }
        cur = *from;
    }
}

bool OrderedList::zless(AVLNode* lhs, AVLNode* rhs){
    ZNode* zr = container_of(rhs, ZNode, tree);
    return zless(lhs, zr->score, zr->name, zr->len);
}

bool OrderedList::zless(AVLNode* lhs, double score, const char* name, size_t len){
    ZNode* zl = container_of(lhs, ZNode, tree);
    if(zl->score != score)
    {
        return zl->score < score;
    }

    int rv = memcmp(zl->name, name, std::min(zl->len, len));
    if(rv != 0)
    {
        return rv < 0;
    }

    return zl->len < len;
}