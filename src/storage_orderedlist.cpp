#include "storage_orderedlist.h"


static bool zless(avl_node* lhs, double score, const char* name, size_t len){
    zset_node* zl = container_of(lhs, zset_node, tree);
    if(!zl){
        return false;
    }

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

static bool zless(avl_node* lhs, avl_node* rhs){
    zset_node* zr = container_of(rhs, zset_node, tree);
    return zless(lhs, zr->score, zr->name, zr->len);
}

static void zset_tree_add(orderedlist_zset* pzset, zset_node* node){
    if(pzset->tree == nullptr)
    {
        pzset->tree = &node->tree;
        return;
    }

    avl_node* cur = pzset->tree;
    while(true)
    {
        avl_node** from = zless(&node->tree, cur) ? &cur->left : &cur->right;
        if(*from == nullptr)
        {
            *from = &node->tree;
            node->tree.parent = cur;
            pzset->tree = avl_fix(&node->tree);
            break;
        }
        cur = *from;
    }
}

static void zset_updata(orderedlist_zset* pzset, zset_node* node, double score){
    if(node->score == score)
    {
        return;
    }

    pzset->tree = avl_del(&node->tree);
    node->score = score;
    avl_init(&node->tree);
    zset_tree_add(pzset, node);
}

static bool hcmp(hashtable_node* node, hashtable_node* key){
     if(node->code != key->code)
    {
        return false;
    }

    zset_node* nodeTemp = container_of(node, zset_node, node);
    orderedlist_key* hkey = container_of(key, orderedlist_key, node);
    if(nodeTemp->len != hkey->len)
    {
        return false;
    }

    return 0 == memcmp(nodeTemp->name, hkey->name, nodeTemp->len);
}

zset_node* create_orderedlist_node(const char* name, size_t len, double score){
    zset_node* node = (zset_node*)malloc(sizeof(zset_node) + len);
    if(node == nullptr)
    {
        return nullptr;
    }

    avl_init(&node->tree);
    node->node.next = nullptr;
    node->node.code = str_hash((uint8_t*)name, len);
    node->score = score;
    node->len = len;
    memcpy(&node->name[0], name, len);
    return node;
}

zset_node* orderedlist_find(orderedlist_zset* pzset, const char* name, size_t len){
    if(pzset->tree == nullptr)
    {
        return nullptr;
    }

    orderedlist_key key;
    key.node.code = str_hash((uint8_t*)name, len);
    key.name = name;
    key.len = len;

    hashtable_node* found = hashtable_find(&pzset->node, &key.node, &hcmp);
    if(found == nullptr)
    {
        return nullptr;
    }

    return container_of(found, zset_node, node);
}

bool orderedlist_add(orderedlist_zset* pzset, const char* name, size_t len, double score){
    zset_node* node = orderedlist_find(pzset, name, len);

    //存在就更新，不存在就创建
    if(node != nullptr)
    {
        zset_updata(pzset, node, score);
        return true;
    }
    else
    {
        node = create_orderedlist_node(name, len, score);
        hashtable_add(&pzset->node, &node->node);
        zset_tree_add(pzset, node);
        return true;
    }
    return false;
}

bool orderedlist_pop(orderedlist_zset* pzset, const char* name, size_t len){
    if(pzset->tree == nullptr)
    {
        return false;
    }

    orderedlist_key key;
    key.node.code = str_hash((const uint8_t*)name, len);
    key.name = name;
    key.len = len;

    hashtable_node* fount = hashtable_pop(&pzset->node, &key.node, &hcmp);
    if(fount == nullptr)
    {
        return false;
    }

    //偏移找到树上的节点
    zset_node* node = container_of(fount, zset_node, node);
    if(nullptr == avl_del(&node->tree))
    {
        return false;
    }
    return true;
}

zset_node* orderedlist_query(orderedlist_zset* pzset, double sroce, const char* name, size_t len, int64_t offset){
    avl_node* fount = nullptr;
    avl_node* cur = pzset->tree;

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
        fount = avl_offset(fount, offset);
    }

    return fount != nullptr ? container_of(fount, zset_node, tree) : nullptr;
}
