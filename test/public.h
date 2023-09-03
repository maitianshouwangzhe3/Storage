#ifndef __PUBLIC_H__
#define __PUBLIC_H__
#include <stdint.h>
#include <string>

#define null "null"
#define OK "OK"
#define Fail "Fail"
#define PANG "pang"
#define ERR "error"
#define EXPECT_INT "expect_int"
#define EXPECT_FP_NUMBER "expect fp number"

extern "C"{
#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type, member) );})
}

enum {
    T_STR = 0,
    T_ZSET = 1,
};

enum type_{
    ERROR = -1,
    PING,
    SET,
    GET,
    DEL,
    ZADD,
    ZDEL,
    ZGET,
    ZSET,
    ZTROY,
    QUERY,
    QUIT,
    SYNTAX_ERROR
};

struct avl_node
{
    uint32_t depth = 0;
    uint32_t cnt   = 0;
    avl_node* left  = nullptr;
    avl_node* right = nullptr;
    avl_node* parent = nullptr;
};

struct Data_int
{
    avl_node node;
    uint32_t val = 0;
    std::string key;
};

struct Container
{
    avl_node* root = nullptr;
};

struct hashtable_node
{
    hashtable_node* next = nullptr;
    uint64_t    code = 0;
};

struct table
{
    hashtable_node** table = nullptr;
    size_t mask = 0;
    size_t size = 0;
};

struct hashtable_map
{
    table table1;
    table table2;

    size_t ResizingPos = 0;    
};

struct entry_hashtable
{
    struct hashtable_node node;
    std::string key;
    std::string value;
};

struct orderedlist_key {
    hashtable_node node;
    const char *name = nullptr;
    size_t len = 0;
};

struct orderedlist_zset
{
    avl_node* tree = nullptr;
    hashtable_map node;
};

struct zset_node
{
    avl_node tree;
    hashtable_node node;
    double score = 0.0;
    size_t len = 0;
    char name[0];
};

struct entry_zset {
    hashtable_node node;
    std::string key;
    std::string val;
    uint32_t type = 0;
    orderedlist_zset* pzset = nullptr;
};

#endif