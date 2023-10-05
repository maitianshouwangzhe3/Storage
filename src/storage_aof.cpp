#include "storage_aof.h"
#include "storage_timewheel.h"
#include "storage_spinlock.h"

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <memory.h>

struct storage_message_log {
    char* buf;
    int   len;

    storage_message_log() {
        buf = (char*)malloc(sizeof(char) * 1024);
        memset(buf, '0', sizeof(buf));
        len = 0;
    }

    ~storage_message_log() {
        free(buf);
        buf = nullptr;
        len = 0;
    }
};

struct data_vec {
    std::vector<storage_message_log*> data1;
    std::vector<storage_message_log*> data2;
    int flags = 0;
    int time = 0;
};

static data_vec* data_v = new data_vec();
static spinlock* lock = new spinlock();

static void refresh_data_to_disk(std::vector<storage_message_log*> data) {
    if(data.size() <= 0) {
        return;
    }

    FILE* fp = fopen("aof", "ab");
    if(!fp) {
        return;
    }

    for(int i = 0; i < data.size(); ++i) {
        if(data[i]) {
            fwrite(data[i]->buf, data[i]->len, sizeof(char), fp);
        }       
    }

    fclose(fp);
}

static void release_resources(std::vector<storage_message_log*> data) {
    if(data.size() <= 0) {
        return;
    }

    for(int i = 0; i < data.size(); ++i) {
        delete data[i];
        data[i] = nullptr;
    }

    data.resize(4);
}

static void storage_data_persistence_callback(void * arg) {
    data_vec* data = (data_vec*)arg;
    lock->spinlock_lock();
    if(!data->flags) {
        data->flags = 1;
        refresh_data_to_disk(data->data1);
        release_resources(data->data1);
    }
    else {
        data->flags = 0;
        refresh_data_to_disk(data->data2);
        release_resources(data->data2);
    }

    TimerWheel::get_instance()->add(data_v->time, storage_data_persistence_callback, (void*)data_v);
    lock->spinlock_unlock();
}

static void restore_data() {

}

void storage_data_persistence(char* buf, int len) {
    storage_message_log* data = new storage_message_log();
    memcpy(data->buf, buf, len);
    data->len = len;
    
    if(!data_v->flags) {
        data_v->data1.emplace_back(data);
    }
    else {
        data_v->data2.emplace_back(data);
    }
}

void storage_init_data_persistence(int flags, int time) {

    if(flags) {
        restore_data();
    }

    data_v->time = time;
    TimerWheel::get_instance()->add(time, storage_data_persistence_callback, (void*)data_v);
}

void storage_aof_release() {
    release_resources(data_v->data1);
    release_resources(data_v->data2);
    data_v->data1.resize(0);
    data_v->data2.resize(0);
    
    if(lock) {
        delete lock;
        lock = nullptr;
    }
}