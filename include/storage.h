#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "storage_pthread_pool.h"
#include "storage_timewheel.h"
#include "storage_spinlock.h"
#include "storage_epoller.h"
#include "public.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>


class Storage{
public:
    Storage(int pthrad_size = 8, int port_size = 1, int mode = -1);
    ~Storage();
   
    void run();

    static bool set_isClose();

private:
    //初始化sock
    bool Init_socket(int port);

    //设置非阻塞
    int SetFdNonblock(int fd);

    //连接新进来的客户端
    void DealListen_(int listenFd);

    //添加客户端信息
    void AddClient_(int fd, sockaddr_in addr);

    //设置模式
    void InitEventMode_(int trigMode);

    //运行定时器
    void run_timer();

private:
    uint32_t                listenEvent_;
    uint32_t                connEvent_;            
    static bool             isClose;
    PthreadPool*            thread_pool;
    Epoller*                epoller_;
    TimerWheel*             m_timer;
    spinlock*               lock;
    std::vector<int>        port_;
    std::unordered_set<int> listenFd_;    
};

#endif