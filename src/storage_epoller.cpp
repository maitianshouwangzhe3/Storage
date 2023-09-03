#include "storage_epoller.h"
#include <iostream>

Epoller* Epoller::epoller_ = nullptr;

Epoller* Epoller::getinstance(){
    if(epoller_ == nullptr){
        epoller_ = new Epoller(1024);
        return epoller_;
    }
    return epoller_;
}

void Epoller::DstroyInstance(){
    if(epoller_){
        delete epoller_;
        epoller_ = nullptr;
    }
}

Epoller::Epoller(int maxEvents) : epollFd_(epoll_create(512)), events_(maxEvents){
    assert(epollFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller(){
    close(epollFd_);
}

bool Epoller::AddFd(int fd, uint32_t events){
    if(fd < 0){
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t events){
    if(fd < 0){
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd , &ev);
}


bool Epoller::DelFd(int fd){
    if(fd < 0){
        return false;
    }
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::wait(int timeoutMs){
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

int Epoller::GetEventFd(uint32_t i){
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

int Epoller::GetEvents(uint32_t i){
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}