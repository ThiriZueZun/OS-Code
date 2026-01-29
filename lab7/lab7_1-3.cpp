#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1, mutex2;

void safe_thread1() {
    // std::lock locks multiple mutexes without deadlock
    std::lock(mutex1, mutex2);
    
    // Adopt the locks into lock_guards for RAII
    std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);
    
    std::cout << "Thread 1: Locked both mutexes safely\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Thread 1: Doing work\n";
    
    // Locks automatically released when lock_guards go out of scope
}

void safe_thread2() {
    // Can lock in any order with std::lock
    std::lock(mutex2, mutex1);
    
    std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);
    
    std::cout << "Thread 2: Locked both mutexes safely\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Thread 2: Doing work\n";
}

int main() {
    std::thread t1(safe_thread1);
    std::thread t2(safe_thread2);
    
    t1.join();
    t2.join();
    
    std::cout << "No deadlock occurred!\n";
    
    return 0;
}
