#ifndef __HISTORAGE_H__
#define __HISTORAGE_H__


int storage_init(const char* ip, int port);
void storage_distory(int sock);
void storage_exec(const char* buf, int buflen, const char* com, int comlen ,int fd);

#endif