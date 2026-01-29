#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1, mutex2;

// This code WILL create a deadlock!
void thread1() {
    mutex1.lock();
    std::cout << "Thread 1: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Thread 1: Waiting for mutex2...\n";
    mutex2.lock();  // Will wait forever if thread2 has mutex2
    
    std::cout << "Thread 1: Locked mutex2\n";
    
    mutex2.unlock();
    mutex1.unlock();
}

void thread2() {
    mutex2.lock();
    std::cout << "Thread 2: Locked mutex2\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Thread 2: Waiting for mutex1...\n";
    mutex1.lock();  // Will wait forever if thread1 has mutex1
    
    std::cout << "Thread 2: Locked mutex1\n";
    
    mutex1.unlock();
    mutex2.unlock();
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    
    t1.join();
    t2.join();
    
    return 0;
}
