#ifndef __MAIN_H__
#define __MAIN_H__


#include "hashtable.h"
#include "avltree.h"
#include "timewheel.h"
#include "zset.h"
#include <iostream>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <math.h>



struct Entry2 {
    struct HNode node;
    std::string key;
    std::string val;
    uint32_t type = 0;
    ZSet *zset = NULL;
};



enum {
    T_STR = 0,
    T_ZSET = 1,
};



void do_set(std::vector<std::string>& cmd, char* buf, TimerWheel* tw);
void do_del(std::vector<std::string>& cmd, char* buf);
void do_get(std::vector<std::string>& cmd, char* buf);
void get_cmd(std::vector<std::string>& cmd, std::string& str_cmd);


void do_zadd(std::vector<std::string>& cmd, char* buf, TimerWheel* tw);
void do_zdel(std::vector<std::string>& cmd, char* buf);
void do_zget(std::vector<std::string>& cmd, char* buf);

void do_zset(std::vector<std::string>& cmd, char* buf, TimerWheel* tw);
void do_troy(std::vector<std::string>& cmd, char* buf);
void do_query(std::vector<std::string>& cmd, char* buf);

#endif