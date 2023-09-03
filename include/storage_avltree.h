#ifndef __STORAGE_AVLTREE_H__
#define __STORAGE_AVLTREE_H__

#include "public.h"
#include <set>
#include <stdint.h>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>

//添加节点
bool avl_add(Container* c, uint32_t key, const char* val);

 //分离节点，并返回最新的根节点
avl_node* avl_del(avl_node* node);

//初始化节点
void avl_init(avl_node* node);

//获取对应偏移量的节点
avl_node* avl_offset(avl_node* node, int64_t offset);

avl_node* avl_fix(avl_node* root);

bool avl_delete(Container* c, uint32_t key);

#endif