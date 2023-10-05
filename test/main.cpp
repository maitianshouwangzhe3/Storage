#include <stdio.h>
#include <string.h>
#include <string>

int main() {
    
#if 0
    char buf[128] = "aaaaaaaaaaaaaaaa";
    
    FILE* fp = fopen("aof", "ab");
    for (int i = 0; i < 10; i++) { // 逐行写入数据  
        int ret = fwrite(buf, strlen(buf) + 1, 1, fp); // 将一行数据以二进制形式写入文件 
        printf("%d\n", ret);
    }  
#else
    int num = 0;
    char buf[256] = {0};
    FILE* fp = fopen("aof", "rb");
    int ret = fread(buf, 1, 256, fp);
    
    int index = 0;
    while(ret > 0) {
        char* token = strtok(buf, "\0");
        int offset = 0;
        printf("%s --- %d\n", token, offset);
        offset = (int)strlen(token) + 1;
        ++num;
        while(token != NULL && offset < ret) {
            token = strtok(buf + offset, "\0");
            if(offset + strlen(token) + 1 >= ret) {
                index += offset;
                break;
            }
            printf("%s --- %d\n", token, offset);

            if(token){
                
                offset += strlen(token) + 1;
            }           
            ++num;
        }

        if(ret == 256) {
            fseek(fp, index, SEEK_SET);
            ret = fread(buf, 1, 256, fp);
        }
        else {
            ++num;
            printf("%s --- %d\n", token, offset);
            break;
        }       
    }
    printf("----------------> %d\n", num);
    fclose(fp);
#endif
    return 0;
}