#ifndef __AVLTREE_H__
#define __AVLTREE_H__


#include "public.h"
#include "spinlock.h"
#include <set>
#include <stdint.h>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>


struct AVLNode
{
    uint32_t depth = 0;
    uint32_t cnt   = 0;
    AVLNode* left  = nullptr;
    AVLNode* right = nullptr;
    AVLNode* parent = nullptr;
};

struct Data_int_s
{
    AVLNode node;
    uint32_t val = 0;
    std::string key;
};

struct Container_s
{
    AVLNode* root = nullptr;
};


class AvlTree{
public:
    AvlTree();
    ~AvlTree();

    //添加节点
    bool avl_add(std::string& key, uint32_t val);

    //删除节点
    bool avl_delete(uint32_t val);
    AVLNode* avl_find(AVLNode* node);  

    //查找节点，过程中会监测平衡，并调整
    AVLNode* avl_fix(AVLNode* root);
    Container_s* get();

    AVLNode* avl_offset_t(AVLNode* node, int64_t offset);

    void avl_init(AVLNode* node);
    
private:
    //下列方法，法如其名
    void avl_update(AVLNode* node);
    
    int avl_depth(AVLNode* node);
    int avl_cnt(AVLNode* node);
    int max_node(uint32_t lhs, uint32_t rhs);
    AVLNode* avl_find_node_del(AVLNode* node);
    

    AVLNode* avl_offset(AVLNode* node, int64_t offset);
    AVLNode* avl_fix_right(AVLNode* root);
    AVLNode* avl_fix_left(AVLNode* root);
    AVLNode* rot_right(AVLNode* node);
    AVLNode* rot_left(AVLNode* node);

private:
    Container_s* con;
    spinlock* lock;
};



#endif