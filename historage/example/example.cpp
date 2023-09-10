#include "../historage.h"
#include <iostream>

//g++ example.cpp -o example -I ../ -L ../ -lhistorage
int main(){
    int fd = 0;
    fd = storage_init("192.168.192.130", 8000);
    if(fd <= 0) {
        std::cout << "init fail" << std::endl;
        return -1;
    }

    char buf[256] = {0};
    char com[256] = {0};
    strncpy(com, "set key 123", 255);
    storage_exec(buf, sizeof(buf), com, strlen(com), fd);

    std::cout << com << std::endl;
    std::cout << buf << std::endl;

    memset(com, 0, sizeof(com));
    memset(buf, 0, sizeof(buf));

    strncpy(com, "get key 123", 255);
    storage_exec(buf, sizeof(buf), com, strlen(com), fd);

    std::cout << com << std::endl;
    std::cout << buf << std::endl;

    memset(com, 0, sizeof(com));
    memset(buf, 0, sizeof(buf));

    strncpy(com, "quit", 255);
    storage_exec(buf, sizeof(buf), com, strlen(com), fd);

    std::cout << com << std::endl;
    std::cout << buf << std::endl;

    memset(com, 0, sizeof(com));
    memset(buf, 0, sizeof(buf));

    storage_distory(fd);
    return 0;
}