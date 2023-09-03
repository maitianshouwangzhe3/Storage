#include "storage.h"

int main(){
    Storage* wb = new Storage();
    wb->run();
    if(wb){
        delete wb;
        wb = nullptr;
    }
    return 0;
}