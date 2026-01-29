#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1, mutex2;

// Solution: Always lock mutexes in the same order
void thread1_fixed() {
    // Lock in order: mutex1 first, then mutex2
    mutex1.lock();
    std::cout << "Thread 1: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    mutex2.lock();
    std::cout << "Thread 1: Locked mutex2\n";
    
    // Perform operations
    std::cout << "Thread 1: Critical section\n";
    
    // Unlock in reverse order
    mutex2.unlock();
    mutex1.unlock();
    std::cout << "Thread 1: Released all locks\n";
}

void thread2_fixed() {
    // Lock in SAME order: mutex1 first, then mutex2
    mutex1.lock();
    std::cout << "Thread 2: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    mutex2.lock();
    std::cout << "Thread 2: Locked mutex2\n";
    
    // Perform operations
    std::cout << "Thread 2: Critical section\n";
    
    // Unlock in reverse order
    mutex2.unlock();
    mutex1.unlock();
    std::cout << "Thread 2: Released all locks\n";
}

int main() {
    std::thread t1(thread1_fixed);
    std::thread t2(thread2_fixed);
    
    t1.join();
    t2.join();
    
    std::cout << "Program completed successfully!\n";
    
    return 0;
}
