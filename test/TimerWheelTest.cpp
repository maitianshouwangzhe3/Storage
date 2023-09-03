#include "TimerWheel.h"
#include <iostream>

int main()
{
    std::cout << "begin" << std::endl;
    TimerWheel* tw = new TimerWheel();
    tw->add(300, [&](){
        std::cout << "-----------------------300ms---------------------" << std::endl;
    });

    tw->add(3000, [&](){
        std::cout << "-----------------------3000ms---------------------" << std::endl;
    });

    tw->add(30000, [&](){
        std::cout << "-----------------------30000ms---------------------" << std::endl;
    });

    tw->run();
    delete tw;
    return 0;
}