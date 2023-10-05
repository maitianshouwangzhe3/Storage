#ifndef __STORAGE_AOF_H__
#define __STORAGE_AOF_H__

void storage_data_persistence(char* buf, int len);
//默认5秒进行一次数据落盘，第一个参数是否从aof加载数据，默认不加载
void storage_init_data_persistence(int flags = 0, int time = 5000);

void storage_aof_release();
#endif