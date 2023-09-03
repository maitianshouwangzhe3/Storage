#include "storage_spinlock.h"

spinlock::spinlock() : lock(0){
    
}

spinlock::~spinlock(){

}

void spinlock::spinlock_lock(){
    while (__sync_lock_test_and_set(&lock, 1)) {

    }
}

bool spinlock::spinlock_trylock(){
    return __sync_lock_test_and_set(&lock, 1) == 0;
}

void spinlock::spinlock_unlock(){
    __sync_lock_release(&lock);
}