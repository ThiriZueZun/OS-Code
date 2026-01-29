#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <memory>

class BankAccount {
private:
    double balance;
    std::mutex mtx;
    int accountId;

public:
    BankAccount(int id, double initial)
        : accountId(id), balance(initial) {}

    // Deadlock-safe transfer using lock ordering
    static bool transfer(BankAccount& from, BankAccount& to, double amount) {
        if (&from == &to) return false;

        BankAccount* first = (&from < &to) ? &from : &to;
        BankAccount* second = (&from < &to) ? &to : &from;

        std::lock(first->mtx, second->mtx);
        std::lock_guard<std::mutex> lock1(first->mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(second->mtx, std::adopt_lock);

        if (from.balance >= amount) {
            from.balance -= amount;
            to.balance += amount;

            std::cout << "Transfer $" << amount
                      << " from Account " << from.accountId
                      << " to Account " << to.accountId << std::endl;
            return true;
        }
        return false;
    }

    double getBalance() const {
        return balance;
    }

    int getId() const {
        return accountId;
    }
};

int main() {
    const int NUM_ACCOUNTS = 5;
    const int NUM_THREADS = 4;
    const int TRANSFERS_PER_THREAD = 5;

    // Create accounts safely
    std::vector<std::unique_ptr<BankAccount>> accounts;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts.push_back(std::make_unique<BankAccount>(i, 1000.0));
    }

    // Random generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> accDist(0, NUM_ACCOUNTS - 1);
    std::uniform_real_distribution<> amountDist(1.0, 200.0);

    // Worker function
    auto worker = [&]() {
        for (int i = 0; i < TRANSFERS_PER_THREAD; i++) {
            int from = accDist(gen);
            int to = accDist(gen);
            double amount = amountDist(gen);

            BankAccount::transfer(
                *accounts[from],
                *accounts[to],
                amount
            );

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    // Launch threads
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nFinal balances:\n";
    for (const auto& acc : accounts) {
        std::cout << "Account " << acc->getId()
                  << ": $" << acc->getBalance() << std::endl;
    }

    return 0;
}
