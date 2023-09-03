#include "storage.h"

bool Storage::isClose = false;

Storage::Storage(int pthrad_size, int port_size, int mode){
    thread_pool = PthreadPool::GetInstance();
    m_timer = new TimerWheel();
    lock = new spinlock();
    epoller_ = Epoller::getinstance();
    
    for(int i = 0; i < port_size; ++i){
        port_.emplace_back(8000 + i);
    }

    InitEventMode_(mode);

}

Storage::~Storage(){
    PthreadPool::DestroyInstance();
    Epoller::DstroyInstance(); 

    for(auto port : port_) {
        close(port);
    }
    port_.clear();

    for(auto fd : listenFd_) {
        close(fd);
    }
    listenFd_.clear();
    
    if(m_timer) {
        delete m_timer;
        m_timer = nullptr;
    }

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

    storage_read_thread_close();
    storage_write_thread_close();
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
}