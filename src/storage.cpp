#include "storage.h"
#include "storage_avltree.h"
#include "storage_hashtable.h"
#include "storage_orderedlist.h"
#include "storage_resource.h"
#include "storage_grammar_parser.h"
#include "storage_write_message.h"
#include "storage_read_message.h"
#include "storage_memory_pool.h"
#include "storage_aof.h"

#include <fcntl.h>       // fcntl(
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <functional>
#include <stdio.h>
#include <math.h>
#include <memory>
#include <mutex>

static pthread_mutex_t mtx;
bool Storage::isClose = false;
std::set<int> Storage::mfd_status;

Storage::Storage(int pthrad_size, int port_size, int mode){
    thread_pool = PthreadPool::GetInstance();
    m_timer = TimerWheel::get_instance();
    lock = new spinlock();
    epoller_ = Epoller::getinstance();
    
    for(int i = 0; i < port_size; ++i){
        port_.emplace_back(8000 + i);
    }

    InitEventMode_(mode);
    storage_memory_init();
    pthread_mutex_init(&mtx, NULL);
}

Storage::~Storage(){
    PthreadPool::DestroyInstance();
    Epoller::DstroyInstance(); 
    TimerWheel::dstory_instance();
    storage_memory_dstory();

    m_timer = nullptr;

    for(auto port : port_) {
        close(port);
    }
    port_.clear();

    for(auto fd : listenFd_) {
        close(fd);
    }
    listenFd_.clear();

    if(lock){
        delete lock;
        lock = nullptr;
    }
}

bool Storage::Init_socket(int port){
    int ret;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct linger optLinger = {0};

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0){
        close(listenFd);
        return false;
    }

    listenFd_.insert(listenFd);

    ret = setsockopt(listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0){
        close(listenFd);
        return false;
    }

    int optval = 1;
    ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR,(const void*)&optval, sizeof(int));
    if( ret < 0){
        close(listenFd);
        return false;
    }

    ret = bind(listenFd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0 ){
        close(listenFd);
        return false;
    }

    ret = listen(listenFd, 1024);
    if(ret < 0){
        close(listenFd);
        return false;
    }

    ret = epoller_->AddFd(listenFd, listenEvent_ | EPOLLIN);
    if(ret < 0){
        close(listenFd);       
    }

    SetFdNonblock(listenFd);
    
    return true;
}

void Storage::run(){
    int size = port_.size();
    for(int i = 0; i < size; ++i){
        Init_socket(port_[i]);
    }

    storage_rq_init();
    storage_wq_init();
    //storage_init_data_persistence();
    thread_pool->push(std::bind(&Storage::run_timer, this));

    while(!isClose){
        int eventCnt = epoller_->wait();

        for(int i = 0; i < eventCnt; ++i){
            int fd = epoller_->GetEventFd(i);
            uint32_t event = epoller_->GetEvents(i);

            if(listenFd_.find(fd) != listenFd_.end()){
                thread_pool->push(std::bind(&Storage::DealListen_, this, fd));
            }
            else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                thread_pool->push(std::bind(&storage_message_destruction, fd));
            }
            else if(event & EPOLLIN){
                thread_pool->push(std::bind(&storage_grammar_parser, fd));
            }
        }
    }

    m_timer->set_state(false);
    storage_read_thread_close();
    storage_write_thread_close();
    //storage_aof_release();
}

int Storage::SetFdNonblock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void Storage::DealListen_(int listenFd){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do{
        int fd = accept(listenFd, (struct sockaddr*)&addr, &len);
        if(fd < 0){
            return;
        }
        AddClient_(fd, addr);
        set_fd_status(fd);
    }while(listenEvent_ & EPOLLET);   
}

void Storage::AddClient_(int fd, sockaddr_in addr){
    assert(fd > 0);
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
}

void Storage::InitEventMode_(int trigMode){
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch(trigMode){
        case 0:
        break;
        case 1:
        connEvent_ |= EPOLLET;
        break;
        case 2:
        listenEvent_ |= EPOLLET;
        break;
        case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
        default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
}

void Storage::run_timer(){
    while(m_timer->get_state()){
        m_timer->run();
    }
}

bool Storage::set_isClose(){
    isClose = true;
    return isClose;
}

bool Storage::get_fd_status(int fd) {
    bool status = false;
    pthread_mutex_lock(&mtx);
    if(mfd_status.count(fd) > 0) {
        status = true;
    }
    pthread_mutex_unlock(&mtx);
    return status;
}

bool Storage::delete_fd_status(int fd) {
    pthread_mutex_lock(&mtx);
    if(mfd_status.count(fd) > 0) {
        mfd_status.erase(fd);
    }
    pthread_mutex_unlock(&mtx);
    return true;
}

bool Storage::set_fd_status(int fd) {
    pthread_mutex_lock(&mtx);
    if(mfd_status.count(fd) == 0) {
        mfd_status.insert(fd);
    }
    pthread_mutex_unlock(&mtx);
    return true;
}