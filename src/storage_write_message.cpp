#include "storage_write_message.h"

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
    }
}

void storage_send(storage_message* q){
    size_t ret = send(q->fd, q->buf, strlen(q->buf), 0);
    if(ret < 0) {

    }
    memset(q->buf, 0, sizeof(q->buf));
}

void storage_insert_write_message(storage_message* q){
    wq.q.push(q);
    wq.size++;
}

void storage_write_thread_close(){
    isClose = true;
}