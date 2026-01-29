#include <iostream>
#include <thread>
#include <mutex>

std::mutex resourceA, resourceB, resourceC;

void process1() {
    resourceA.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    resourceB.lock();
    // Work...
    resourceB.unlock();
    resourceA.unlock();
}

void process2() {
    resourceB.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    resourceC.lock();
    // Work...
    resourceC.unlock();
    resourceB.unlock();
}

void process3() {
    resourceC.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    resourceA.lock();
    // Work...
    resourceA.unlock();
    resourceC.unlock();
}
