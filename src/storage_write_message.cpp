#include "storage_write_message.h"
#include "storage.h"
#include "storage_epoller.h"
#include "storage_memory_pool.h"
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <iostream>


static bool isClose = false;
static write_queue wq;
pthread_t pid;

void storage_wq_init(){
    pthread_create(&pid, nullptr, storage_write_message, nullptr);
    pthread_detach(pid);
}

void* storage_write_message(void* arg){
    while(true){
        if(wq.size > 0){
            auto q = wq.q.front();
            wq.q.pop();
            wq.size--;
            storage_send(q);
            Epoller::getinstance()->ModFd(q->fd, EPOLLONESHOT | EPOLLRDHUP| EPOLLET | EPOLLIN);
        }
        else if(isClose){
            break;
        }
        else {
            usleep(0);
        }
    }
    return nullptr;
}

void storage_send(storage_message* q){
    size_t ret = send(q->fd, *q->buf, strlen(*q->buf), 0);
    if(ret < 0) {

    }
    memset(*q->buf, 0, storage_get_memory_node_size((void**)q->buf));
}

void storage_insert_write_message(storage_message* q){
    wq.q.push(q);
    wq.size++;
}

void storage_write_thread_close(){
    while(true){
        if(wq.size <= 0){
            isClose = true;
            return;
        }
    }
}