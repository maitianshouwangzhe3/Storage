#include "storage_avltree.h"

#include <set>
#include <stdint.h>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>

static int max_node(uint32_t lhs, uint32_t rhs){
    return lhs < rhs ? rhs : lhs;
}

static int avl_cnt(avl_node* node){
    return node ? node->cnt : 0;
}

static int avl_depth(avl_node* node){
    return node ? node->depth : 0;
}

static void avl_update(avl_node* node){
    node->depth = 1 + max_node(avl_depth(node->left), avl_depth(node->right));
    node->cnt   = 1 + avl_cnt(node->left) + avl_cnt(node->right);
}
    
static avl_node* rot_right(avl_node* node){
    avl_node* new_node = node->left;
    if(nullptr != new_node->right)
    {
        new_node->right->parent = node;
    }

    node->left = new_node->right;
    new_node->right = node;
    new_node->parent = node->parent;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);

    return new_node;
}

static avl_node* rot_left(avl_node* node){
    avl_node* new_node = node->right;
    if(nullptr != new_node->left)
    {
        new_node->left->parent = node;
    }

    node->right = new_node->left;
    new_node->left = node;
    new_node->parent = node->parent;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);

    return new_node;
}

static avl_node* avl_fix_right(avl_node* root){
    if(avl_depth(root->right->right) < avl_depth(root->right->left))
    {
        root->right = rot_right(root->right);
    }
    return rot_left(root);
}

static avl_node* avl_fix_left(avl_node* root){
    if(avl_depth(root->left->left) < avl_depth(root->left->right))
    {
        root->left = rot_left(root->left);
    }
    return rot_right(root);
}

avl_node* avl_fix(avl_node* root){
    while(true)
    {
        avl_update(root);
        uint32_t lh = avl_depth(root->left);
        uint32_t rh = avl_depth(root->right);

        avl_node** from = nullptr;
        if(nullptr != root->parent)
        {
            from = (root->parent->left == root) ? &root->parent->left : &root->parent->right;
        }

        if(lh == rh + 2)
        {
            root = avl_fix_left(root);
        }
        if(rh == lh + 2)
        {
            root =  avl_fix_right(root);
        }

        if(nullptr == from)
        {
            return root;
        }

        *from = root;
        root = root->parent;
    }
}

bool avl_add(Container* c, uint32_t key, const char* val){
    Data_int* data = new Data_int();
    if(!data){
        return false;
    }

    //初始化节点
    avl_init(&data->node);
    data->key = val;
    data->val = key;

    //树为空直接当做头节点
    if(!c->root){
        c->root = &data->node;
        return true;
    }

    //不为空开始向下查找位置并插入
    avl_node* cur = c->root;
    while(true)
    {
        avl_node** from = (key < container_of(cur, Data_int, node)->val) ? &cur->left :&cur->right;
        if(!*from)
        {
            *from = &data->node;
            data->node.parent = cur;
            c->root = avl_fix(&data->node);
            return true;
        }
        cur = *from;
    }

    return false;
}

avl_node* avl_del(avl_node* node){
    if(nullptr == node->right)
    {
        avl_node* parent = node->parent;
        if(nullptr != node->left)
        {
            node->left->parent = parent;
        }

        if(nullptr != parent)
        {
            (parent->left == node ? parent->left : parent->right) = node->left;
            return avl_fix(parent);
        }
        else
        {
            return node->left;
        }
    }
    else
    {
        avl_node* victim = node->right;
        while(nullptr != victim->left)
        {
            victim = victim->left;
        }

        avl_node* root = avl_del(victim);

        *victim = *node;
        if(nullptr != victim->left)
        {
            victim->left->parent = victim;
        }
        if(nullptr != victim->right)
        {
            victim->right->parent = victim;
        }

        avl_node* parent = node->parent;
        if(nullptr != parent)
        {
            (parent->left == node ? parent->left : parent->right) = victim;
            return root;
        }
        else
        {
            return victim;
        }
    }
}

void avl_init(avl_node* node){
    node->depth = 1;
    node->cnt = 1;
    node->left = nullptr;
    node->right = nullptr;
    node->parent = nullptr;
}

avl_node* avl_offset(avl_node* node, int64_t offset){
    //起点
    int pos = 0;
    while(pos != offset)
    {
        if(pos < offset && pos + avl_cnt(node->right) >= offset)
        {
            //元组在右子树
            node = node->right;
            pos += avl_cnt(node->right) + 1;
        }
        else if(pos > offset && pos - avl_cnt(node->left) <= offset)
        {
            //元组在左子树
            node = node->left;
            pos -= avl_cnt(node->left) + 1;
        }
        else
        {
            //向上找父节点
            avl_node* parent = node->parent;
            if(parent == nullptr)
            {
                return nullptr;
            }

            //将之前+（-）的cnt值+（-）回来
            if(parent->right == node)
            {
                pos -= avl_cnt(node->left) + 1;
            }
            else
            {
                pos += avl_cnt(node->right) + 1;
            }

            node = parent;
        }
    }
    return node;
}

bool avl_delete(Container* c, uint32_t key){
    avl_node* cur = c->root;
    while(cur != nullptr)
    {
        uint32_t now_val = container_of(cur, Data_int, node)->val;
        if(key == now_val)
        {
            break;
        }

        cur = key < now_val ? cur->left : cur->right;
    }

    if(cur == nullptr)
    {
        return false;
    }

    c->root = avl_del(cur);
    delete container_of(cur, Data_int, node);
    return true;
}