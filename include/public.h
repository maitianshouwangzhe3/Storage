#ifndef __PUBLIC_H__
#define __PUBLIC_H__
#include <stdint.h>
#include <string>
#include <unordered_map>
//#include <queue>

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

struct avl_node {
    uint32_t depth;
    uint32_t cnt;
    avl_node* left;
    avl_node* right;
    avl_node* parent;
};

struct Data_int {
    avl_node node;
    uint32_t val;
    std::string key;
};

struct Container {
    avl_node* root;
};

struct hashtable_node {
    hashtable_node* next;
    uint64_t    code;
};

struct table {
    hashtable_node** table;
    size_t mask;
    size_t size;
};

struct hashtable_map {
    table table1;
    table table2;

    size_t ResizingPos;    
};

struct entry_hashtable {
    struct hashtable_node node;
    std::string key;
    std::string value;
};

struct orderedlist_key {
    hashtable_node node;
    const char *name;
    size_t len;
};

struct orderedlist_zset {
    avl_node* tree;
    hashtable_map node;
};

struct zset_node {
    avl_node tree;
    hashtable_node node;
    double score;
    size_t len;
    char name[0];
};

struct entry_zset {
    hashtable_node node;
    std::string key;
    std::string val;
    uint32_t type;
    orderedlist_zset* pzset;
};

struct list_key_value {
    char* key;
    char* value;
    list_key_value* next;
};

struct storage_message{
    uint32_t fd;
    type_ cmd;
    char* key;
    list_key_value* list_kv;
    char* limit1;
    char* limit2;
    uint32_t time;
    char** buf;

    storage_message();
    ~storage_message();
};





#endif