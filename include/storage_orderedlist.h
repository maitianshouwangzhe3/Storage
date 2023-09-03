#ifndef __STORAGE_ORDEREDLIST_H__
#define __STORAGE_ORDEREDLIST_H__

#include "public.h"
#include "storage_hashtable.h"
#include "storage_avltree.h"
#include <string.h>
#include <string>
#include <algorithm>

//创建新节点
zset_node* create_orderedlist_node(const char* name, size_t len, double score);

//查找节点
zset_node* orderedlist_find(orderedlist_zset* pzset, const char* name, size_t len);

//添加节点进列表
bool orderedlist_add(orderedlist_zset* pzset, const char* name, size_t len, double score);

//删除节点
bool orderedlist_pop(orderedlist_zset* pzset, const char* name, size_t len);

//找到大于或等于参数的 (score, name) 元组，然后相对于它进行偏移。
zset_node* orderedlist_query(orderedlist_zset* pzset, double sroce, const char* name, size_t len, int64_t offset);

#endif