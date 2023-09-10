#ifndef __HISTORAGE_H__
#define __HISTORAGE_H__
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

int storage_init(const char* ip, int port);
void storage_distory(int sock);
void storage_exec(const char* buf, int buflen, const char* com, int comlen ,int fd);

#endif