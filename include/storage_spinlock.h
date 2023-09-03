#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

class spinlock
{
private:
    int lock;
public:
    spinlock();
    ~spinlock();
    void spinlock_lock();
    bool spinlock_trylock();
    void spinlock_unlock();
};


#endif