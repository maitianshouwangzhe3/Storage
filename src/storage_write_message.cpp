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

void storage_del_message(storage_message** q) {
    storage_free((void**)((*q)->buf));
    storage_free((void**)(q));
}

void* storage_write_message(void* arg){
    while(true){
        if(wq.size > 0){
            auto q = wq.q.front();
            wq.q.pop();
            wq.size--;
            storage_send(q);
            Epoller::getinstance()->ModFd((*q)->fd, EPOLLONESHOT | EPOLLRDHUP| EPOLLET | EPOLLIN);
            storage_del_message(q);
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

void storage_send(storage_message** q){
    if(Storage::get_fd_status((*q)->fd)) {
        int len = strlen(*((*q)->buf));
        int buflen = storage_get_memory_node_size((void**)((*q)->buf));
        size_t ret = -1;
        if (buflen - len > 1) {
            ret = send((*q)->fd, *((*q)->buf), len + 2, 0);
        }
        else {
            ret = send((*q)->fd, *(*q)->buf, len, 0);
        }
    
        if(ret < 0) {
            char err[] = "error";
            ret = send((*q)->fd, err, strlen(err), 0);
        }
        memset(*((*q)->buf), 0, buflen);
    } 
}

void storage_insert_write_message(storage_message** q){
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