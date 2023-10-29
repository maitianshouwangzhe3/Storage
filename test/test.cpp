#include <stdio.h>
#include <memory.h>
#include <memory>


int main() {
    char* ptr = (char*)malloc(256);
    char** tmp = &ptr;
    printf("ptr = %d\n", sizeof(*ptr));
    printf("tmp = %d\n", sizeof(*tmp));
    free(ptr);
    return 0;
}