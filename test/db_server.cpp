/*
 * db服务端
 * 2023 4 11
 */
#include "../include/hashtable.h"
#include "../include/main.h"
#include "../include/ZSet.h"
#include "../include/timewheel.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include <pthread.h>





static int do_ReadAndWrite(int conn, TimerWheel* tw)
{
    char buf[1024] = {};
    ssize_t ret = read(conn, buf, sizeof(buf));
    if(0 > ret)
    {
        std::cout << "read() size < 0" << std::endl;
        return 0;
    }

    std::string str_cmd = buf;
    std::vector<std::string> cmd(10, "");
    get_cmd(cmd, str_cmd);

    std::cout << "client : " << buf << std::endl;

    if("ping" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "pang");
    }
    else if ("get" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        if (cmd[1].empty())
        {
            std::cout << "[get] [key]" << std::endl;
            strcpy(buf, "[get] [key]");
        }
        else
        {
            do_get(cmd, buf);
        }
    }
    else if ("set" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        if (cmd[1].empty() || cmd[2].empty())
        {
            std::cout << "[set] [key] [value]" << std::endl;
            strcpy(buf, "[set] [key] [value]");
        }
        else
        {
            do_set(cmd, buf, tw);
        }
    }
    else if ("del" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        if (cmd[1].empty())
        {
            std::cout << "[del] [key]" << std::endl;
            strcpy(buf, "[del] [key]");
        }
        else
        {
            do_del(cmd, buf);
        }
    }
    else if("zadd" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        if (cmd[1].empty() || cmd[2].empty())
        {
            std::cout << "[zadd] [key] [val]" << std::endl;
            strcpy(buf, "[zadd] [key] [val]");
        }
        else
        {
            do_zadd(cmd, buf, tw);
        }
    }
    else if("zdel" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
       
        do_zdel(cmd, buf);
        
    }
    else if("zget" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        do_zget(cmd, buf);
    }
    else if("zset" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        do_zset(cmd, buf, tw);
    }
    else if("ztroy" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        do_troy(cmd, buf);
    }
    else if("query" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        do_query(cmd, buf);
    }
    else if ("quit" == cmd[0])
    {
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "Bey");
        write(conn, buf, strlen(buf));
        close(conn);
        return 0;
    }
    else if("server_quit" == cmd[0]){
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "Bey");
        write(conn, buf, strlen(buf));
        close(conn);
        return 1;
    }
    else
    {
        std::cout << "[get or set or del] [value]" << std::endl;
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "[get or set or del] [value]");
    }

    write(conn, buf, strlen(buf));
    return 0;
    //tw->run();
}

void* timer(void* arg){
    TimerWheel* tw = (TimerWheel*)arg;
    while(tw->get_state()){
        tw->run();
        usleep(2500);
    }
}

int main()
{
    printf("start\n");
    std::cout << "start" << std::endl;
    TimerWheel* tw = new TimerWheel();
    pthread_t pid;
    int ret = pthread_create(&pid, nullptr, timer, (void*)tw);
    if(ret != 0){
        std::cout << "pthread_create() err" << std::endl;
        return 0;
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(0 > fd)
    {
        std::cout << "socket() err" << std::endl;
        return 0;
    }

    //设置端口释放后不用等两分钟，马上能用
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(8888);
    addr.sin_addr.s_addr = ntohs(0);        //通配符0.0.0.0
    ret = bind(fd, (const sockaddr*)&addr, sizeof(addr));
    if(0 != ret)
    {
        std::cout << "bind() err" << std::endl;
        return 0;
    }

    ret = listen(fd, SOMAXCONN);
    if(0 != ret)
    {
        std::cout << "listen() err" << std::endl;
        return 0;
    }

    struct epoll_event ev;
    struct epoll_event event[10];

    int epfd = epoll_create(256);
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

    //printf("111111111111111\n");

    bool btrue = true;
    struct sockaddr_in clientaddr = {0};
    while(btrue)
    {
        int nfd = epoll_wait(epfd, event, 10, -1);
        if(nfd == -1)
        {
            continue;
        }
        for(int i = 0; i < nfd; ++i)
        {
            if(event[i].data.fd == fd)
            {

                int clientfd = sizeof(clientaddr);
                int connfd = accept(fd, (struct sockaddr*)&clientaddr, (socklen_t*)&clientfd);
                if(-1 == connfd)
                {
                    continue;
                }

                ev.data.fd = connfd;
                ev.events = EPOLLIN | EPOLLET;

                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);

                //printf("ADD\n");
            }
            else if(event[i].events & EPOLLIN)
            {
                //printf("NO\n");
                ret = do_ReadAndWrite(event[i].data.fd, tw);
                if(ret != 0){
                    btrue = false;
                    break;
                }
            }

        }
    }

    tw->set_state(false);
    pthread_join(pid, nullptr);
    close(fd);
    close(epfd);
    //close(conn);

    return 0;
}