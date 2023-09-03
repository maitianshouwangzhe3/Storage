#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>


class Epoller{
private:
    Epoller(int maxEvents = 1024);
    Epoller() = default;
    ~Epoller();

public:
    static void DstroyInstance();

    static Epoller* getinstance();

    //添加监听的文件描述符
    bool AddFd(int fd, uint32_t events);

    //改变文件描述符的状态
    bool ModFd(int fd, uint32_t events);

    //删除epoll中维护的文件描述符
    bool DelFd(int fd);

    //epool_wait()的安全接口
    int wait(int timeoutMs = -1);

    //获取epool中的文件描述符
    int GetEventFd(uint32_t i);

    //获取指定描述符的事件
    int GetEvents(uint32_t i);

private:
    int epollFd_;                                          //epool文件描述符

    std::vector<struct epoll_event> events_;               //事件的容器

    static Epoller* epoller_;
};


#endif