#include "storage_timewheel.h"

timer_node_t::timer_node_t(){
    next = nullptr;
    expiration_time = 0;
    callback = nullptr;
    state = false;
    arg = nullptr;
    id = 0;
}

timer_node_t::~timer_node_t(){
    next = nullptr;
    expiration_time = 0;
    callback = nullptr;
    state = false;
    arg = nullptr;
}

link_list_t::link_list_t(){
    head = new timer_node();
    tail = head;
}

link_list_t::~link_list_t(){
    timer_node* cur = head;
    while(cur != nullptr)
    {
        timer_node* tmp = cur;
        cur = cur->next;
        delete tmp;
    }
    head = nullptr;
    tail = nullptr;
}

TimerWheel::TimerWheel(){
    m_near.resize(TIME_NEAR);

    for(int i = 0; i < TIME_NEAR; ++i){
        m_near[i] = new link_list();
    }

    for(int i = 0; i < 4; ++i){
        std::vector<link_list*> tmp;
        for(int j = 0; j < TIME_SECOND; ++j){
            tmp.emplace_back(new link_list());
        }
        m_second.emplace_back(tmp);
    }

    lock = new spinlock();
    time = 0;
    state = true;
    current_time = get_time();
}

TimerWheel::~TimerWheel(){
    m_near.clear();
    for(int i = 0; i < 4; ++i){
        m_second[i].clear();
    }
    hashmap.clear();
}

unsigned int TimerWheel::add(int timer, std::function<void(void*)> callback, void* arg){
    if(timer <= 0){
        callback(arg);
        return 0;
    }
    timer_node* node = new timer_node();
    if(node == nullptr){
        return -1;
    }
    lock->spinlock_lock();
    node->expiration_time = timer + time;
    node->callback = callback;
    node->arg = arg;
    node->id = get_rag();
    if(!add_node(node)){
        return -1;
    }
    node->state = true;
    hashmap.insert(std::pair<unsigned int, timer_node*>(node->id, node));
    lock->spinlock_unlock();
    return node->id;  
}

void TimerWheel::add(timer_node* node){
    if(node->expiration_time <= 0){
        node->callback(node->arg);
        return ;
    }

    lock->spinlock_lock();
    add_node(node);
    lock->spinlock_unlock();
}

bool TimerWheel::del(unsigned int id){
    if(hashmap.find(id) != hashmap.end()){
        lock->spinlock_lock();
        if(hashmap[id] != nullptr)
        {
            hashmap[id]->state = false;
        }
        lock->spinlock_unlock();
        return true;
    }
    return false;
}

void TimerWheel::del(timer_node* node){
    if(node != nullptr){
        node->state = false;
    }
}

void TimerWheel::run(){
    uint64_t now_time = get_time();
    if(now_time != current_time){
        uint64_t diff = now_time - current_time;
        current_time = now_time;
        for(int i = 0; i < diff; ++i){
            timer_update();
        }
    }
}

bool TimerWheel::add_node(timer_node* node){
    uint32_t current_time = time;
    uint32_t mse = node->expiration_time - time;

    if(mse < TIME_NEAR){
        link(m_near[node->expiration_time & TIME_NEAR_MASK], node);
    }
    else if(mse < TIME_ONE){
        link(m_second[0][(node->expiration_time >> TIME_NEAR_SHIFT) & TIME_SECOND_MASK], node);
    }
    else if(mse < TIME_TWO){
        link(m_second[1][(node->expiration_time >> (TIME_NEAR_SHIFT + TIME_SECOND_SHIFT)) & TIME_SECOND_MASK], node);
    }
    else if(mse < TIME_TRE){
        link(m_second[2][(node->expiration_time >> (TIME_NEAR_SHIFT + 2 * TIME_SECOND_SHIFT)) & TIME_SECOND_MASK], node);
    }
    else{
        link(m_second[3][(node->expiration_time >> (TIME_NEAR_SHIFT + 3 * TIME_SECOND_SHIFT)) & TIME_SECOND_MASK], node);
    }
    return true;
}

void TimerWheel::link(link_list* m_near, timer_node* node){
    m_near->tail->next = node;
    m_near->tail = node;
    node->next = nullptr;
}

uint64_t TimerWheel::get_time(){
    uint64_t time = 0;
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    time = static_cast<uint64_t>(t.tv_sec * 100);
    time += t.tv_nsec / 10000000;
    return time;
}

void TimerWheel::timer_update(){
    lock->spinlock_lock();
    timer_execute();
    timer_shift();
    timer_execute();
    lock->spinlock_unlock();
}

void TimerWheel::timer_execute(){
    int index = time & TIME_NEAR_MASK;
    while(m_near[index]->head->next){
        timer_node* node = link_clear(m_near[index]);
        lock->spinlock_unlock();
        dispatch_list(node);
        lock->spinlock_lock();
    }
}

void TimerWheel::timer_shift(){
    int mask = TIME_NEAR;
    uint32_t ct = ++time;
    if(ct == 0){
        move_list(3, 0);
    }
    else{
        uint32_t t = ct >> TIME_NEAR_SHIFT;
        int index = 0;

        while((ct & (mask - 1)) == 0){
            int idx = t & TIME_SECOND_MASK;
            if(idx != 0){
                move_list(index, idx);
                break;
            }

            mask <<= TIME_SECOND_SHIFT;
            t >>= TIME_SECOND_SHIFT;
            ++index;
        }
    }
}

timer_node* TimerWheel::link_clear(link_list* list){
    timer_node* ret = list->head->next;
    list->head->next = nullptr;
    list->tail = list->head;
    return ret;
}

void TimerWheel::dispatch_list(timer_node* node){
    if(node == nullptr){
        return;
    }
    
    do{
        timer_node* cur = node;
        node = node->next;
        if(cur->state && cur->callback != nullptr){
            cur->callback(cur->arg);
            delete cur;
            cur = nullptr;
        }
        else if(cur != nullptr){
            delete cur;
            cur = nullptr;
        }
    }while(node != nullptr);
}

bool TimerWheel::get_state(){
    return state;
}

void TimerWheel::set_state(bool state){
    lock->spinlock_lock();
    this->state = state;
    lock->spinlock_unlock();
}

void TimerWheel::move_list(int level, int index){
    timer_node* cur = link_clear(m_second[level][index]);
    while(cur != nullptr){
        timer_node* tmp = cur->next;
        add_node(cur);
        cur = tmp;
    }
}

unsigned int TimerWheel::get_rag(){
    std::mt19937 rng(std::random_device{}());
    unsigned int id = rng();
    while(hashmap.find(id) != hashmap.end()){
        id = rng();
    }
    return id;
}