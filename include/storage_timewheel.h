#ifndef __TIMEWHEEL_H__
#define __TIMEWHEEL_H__

#include "storage_spinlock.h"
#include <stdint.h>
#include <functional>
#include <vector>
#include <time.h>
#include <random>
#include <unordered_map>

//这里的宏定义用来确定元素在时间轮的位置 
//实际转动的时间轮（只有最低一级才会一直转动，相当于秒针）
//2的8次方 256个槽位 后面四级64个槽位
#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_SECOND_SHIFT 6
#define TIME_SECOND (1 << TIME_SECOND_SHIFT)
#define TIME_NEAR_MASK ((TIME_NEAR) - 1)
#define TIME_SECOND_MASK ((TIME_SECOND) - 1)

#define TIME_ONE 16383
#define TIME_TWO 1048575
#define TIME_TRE 67108863
#define TIME_FOU 4294967295

//定时器节点
typedef struct timer_node_t{
    timer_node_t* next;                           //用于连接下个节点的指针
    uint32_t expiration_time;                   //过期时间
    std::function<void(void*)> callback;        //回调函数指针，到期执行次函数
    bool state;                                 //标记此节点状态
    void* arg;                                  //预留，可用来传递参数
    unsigned int id;                            //随机数
    int thread_id;
    
    timer_node_t();
    ~timer_node_t();
} timer_node;

//链表，用来挂载到每个槽位上,让每个槽位能挂载多个任务
typedef struct link_list_t{
    timer_node* head;
    timer_node* tail;

    link_list_t();
    ~link_list_t();
} link_list;


class TimerWheel{
public:
    static TimerWheel* get_instance();
    static void dstory_instance();

    unsigned int add(int timer, std::function<void(void*)> callback, void* arg = nullptr);
    bool del(unsigned int id);
    void run();
    
    bool get_state();
    void set_state(bool state);

    void add(timer_node* node);
    void del(timer_node* node);

private:
    TimerWheel();
    ~TimerWheel();

    bool add_node(timer_node* node);
    void link(link_list* m_near, timer_node* node);
    uint64_t get_time();
    void timer_update();
    void timer_execute();
    void timer_shift();
    timer_node* link_clear(link_list* list);
    void dispatch_list(timer_node* node);
    void move_list(int level, int index);
    unsigned int get_rag();
    

private:
    spinlock* lock;                                          //自旋锁      
    std::vector<link_list*> m_near;                          //实际轮询的一级
    std::vector<std::vector<link_list*>> m_second;           //挂载离执行时间较久的任务
    uint32_t time;
    uint32_t current_time;
    bool state;
    std::unordered_map<unsigned int, timer_node*> hashmap;

    static TimerWheel* timer;
};

#endif