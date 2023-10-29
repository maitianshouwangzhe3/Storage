
#include <stdio.h>
#include <memory>
#include <memory.h>



#if 1
#include "storage_memory_pool.h"

int main() {

    storage_memory_init();

    char** arr = (char**)storage_malloc(128);

    memset(*arr, 0, sizeof(*arr));
    strcpy(*arr, "ssssssssssssssssssssss");

    printf("-->%s\n", *arr);

    storage_free((void**)arr);

    storage_memory_dstory();
    

    return 0;
}
#else

extern "C"{
#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type, member) );})
}

struct test {
    int id;
    void** data;
};

void tttt(void* node) {
    void** ptr = &node;
    test* tmp = container_of(&ptr, test, data);  

    printf("--->%d\n", tmp->id);
}

int main() {
    test* s = new test();
    s->id = 22;

    void* p = malloc(128); 
    s->data = &p; 

    void*** ptr = &s->data;
    //printf("s->data = %p\n", s->data);
    //printf("ptr = %p\n", ptr);

    //printf("&s->data = %p\n", &s->data);
    //printf("&ptr = %p\n", &ptr);

    test* tmp = container_of(ptr, test, data);  

    printf("--->%d\n", tmp->id);

    //tttt(*s->data);

    
    return 0;
}

#endif