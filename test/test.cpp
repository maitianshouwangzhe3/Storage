#include <stdio.h>
#include <memory.h>
#include <memory>

struct list {
    int a;
    list* next;
};

struct test {
    int fd;
    list* cur;
    char** p;
};

enum type_ {
    aaa = 0,
    bbb,
    ccc,
    ddd,
    eee,
};

struct __attribute__((packed))storage_message{
    uint32_t fd;
    type_ cmd;
    uint32_t time;
    char* key;
    list* list_kv;
    char* limit1;
    char* limit2;   
    char** buf;

    storage_message();
    ~storage_message();
};

int main() {
#if 0
    char* ptr = (char*)malloc(256);
    char** tmp = &ptr;
    printf("ptr = %d\n", sizeof(*ptr));
    printf("tmp = %d\n", sizeof(*tmp));
    free(ptr);
#endif
    list* t = new list();
    t->a = 10;
    char* arr = (char*)malloc(128);
    memset(arr, '\0', 128);
    sprintf(arr, "hello world!");
    test* ptr = (test*)malloc(128);
    ptr->fd = 5;
    ptr->cur = t;
    ptr->p = &arr;

    printf("((test*)ptr)->fd = %d ((test*)ptr)->cur->a = %d ((test*)ptr)->p = %s\n", ptr->fd,  ptr->cur->a, *(ptr->p));
    printf("storage_message size %ld\n", sizeof(storage_message));
    printf("type_ size %ld\n", sizeof(type_));
    delete t;
    free(arr);
    free((void*)ptr);
    char ss[] = "sssss\r\n";
    printf("ss = %ld", strlen(ss));
    return 0;
}