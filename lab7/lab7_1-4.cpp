#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

mutex resourceA, resourceB, resourceC;

// Global lock order: A → B → C

void process1() {
    lock_guard<mutex> lockA(resourceA);
    lock_guard<mutex> lockB(resourceB);

    cout << "Process 1: Using Resource A and B" << endl;
    this_thread::sleep_for(chrono::milliseconds(100));
}

void process2() {
    lock_guard<mutex> lockA(resourceA);
    lock_guard<mutex> lockB(resourceB);
    lock_guard<mutex> lockC(resourceC);

    cout << "Process 2: Using Resource A, B and C" << endl;
    this_thread::sleep_for(chrono::milliseconds(100));
}

void process3() {
    lock_guard<mutex> lockA(resourceA);
    lock_guard<mutex> lockC(resourceC);

    cout << "Process 3: Using Resource A and C" << endl;
    this_thread::sleep_for(chrono::milliseconds(100));
}

int main() {
    thread t1(process1);
    thread t2(process2);
    thread t3(process3);

    t1.join();
    t2.join();
    t3.join();

    cout << "All processes completed successfully (No Deadlock)" << endl;
    return 0;
}
