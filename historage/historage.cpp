#include "historage.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

int storage_init(const char* ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(port);
    addr.sin_addr.s_addr = inet_addr(ip);  
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        return -1;
    }
    return fd;
}

void storage_distory(int sock)
{
    close(sock);
}

void storage_exec(const char* buf, int buflen, const char* com, int comlen ,int fd){
    if(fd < 0 || comlen <= 0){
        return;
    }

    int ret = send(fd, com, comlen, 0);
    if(ret <= 0){
        return;
    }

    ret = recv(fd, (void*)buf, buflen, 0);
    if(ret <= 0){
        strcpy((char*)buf, "error");
    }
}