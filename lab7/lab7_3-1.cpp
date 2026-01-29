#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

class DeadlockDetector {
private:
    int numProcesses;
    std::vector<std::vector<int>> waitForGraph; // Adjacency list
    
    // DFS helper for cycle detection
    bool hasCycleDFS(int node, std::vector<bool>& visited, 
                     std::vector<bool>& recStack, 
                     std::vector<int>& cycle) {
        visited[node] = true;
        recStack[node] = true;
        cycle.push_back(node);
        
        for (int neighbor : waitForGraph[node]) {
            if (!visited[neighbor]) {
                if (hasCycleDFS(neighbor, visited, recStack, cycle)) {
                    return true;
                }
            } else if (recStack[neighbor]) {
                // Cycle found - trim cycle to actual loop
                auto it = std::find(cycle.begin(), cycle.end(), neighbor);
                cycle.erase(cycle.begin(), it);
                return true;
            }
        }
        
        recStack[node] = false;
        cycle.pop_back();
        return false;
    }
    
public:
    DeadlockDetector(int processes) : numProcesses(processes) {
        waitForGraph.resize(processes);
    }
    
    // Add edge: process1 waits for process2
    void addWaitEdge(int process1, int process2) {
        waitForGraph[process1].push_back(process2);
    }
    
    // Remove edge
    void removeWaitEdge(int process1, int process2) {
        auto& edges = waitForGraph[process1];
        edges.erase(std::remove(edges.begin(), edges.end(), process2), 
                   edges.end());
    }
    
    // Detect cycle (deadlock)
    bool detectDeadlock(std::vector<int>& deadlockedProcesses) {
        std::vector<bool> visited(numProcesses, false);
        std::vector<bool> recStack(numProcesses, false);
        
        for (int i = 0; i < numProcesses; i++) {
            if (!visited[i]) {
                std::vector<int> cycle;
                if (hasCycleDFS(i, visited, recStack, cycle)) {
                    deadlockedProcesses = cycle;
                    return true;
                }
            }
        }
        
        return false;
    }
    
    void printGraph() {
        std::cout << "\n=== Wait-For Graph ===\n";
        for (int i = 0; i < numProcesses; i++) {
            if (!waitForGraph[i].empty()) {
                std::cout << "P" << i << " waits for: ";
                for (int p : waitForGraph[i]) {
                    std::cout << "P" << p << " ";
                }
                std::cout << "\n";
            }
        }
    }
};

int main() {
    // Create detector for 5 processes
    DeadlockDetector detector(5);
    
    // Build wait-for graph
    // P0 waits for P1
    detector.addWaitEdge(0, 1);
    // P1 waits for P2
    detector.addWaitEdge(1, 2);
    // P2 waits for P3
    detector.addWaitEdge(2, 3);
    // P3 waits for P4
    detector.addWaitEdge(3, 4);
    // P4 waits for P1 (creates cycle!)
    detector.addWaitEdge(4, 1);
    
    detector.printGraph();
    
    // Detect deadlock
    std::vector<int> deadlocked;
    if (detector.detectDeadlock(deadlocked)) {
        std::cout << "\n🚨 DEADLOCK DETECTED!\n";
        std::cout << "Deadlocked processes: ";
        for (int p : deadlocked) {
            std::cout << "P" << p << " ";
        }
        std::cout << "\n";
        
        // Recovery: Kill one process
        std::cout << "\nRecovery: Terminating P" << deadlocked[0] << "\n";
        // Remove all edges from/to this process
        for (int i = 0; i < 5; i++) {
            detector.removeWaitEdge(deadlocked[0], i);
            detector.removeWaitEdge(i, deadlocked[0]);
        }
        
        // Check again
        deadlocked.clear();
        if (!detector.detectDeadlock(deadlocked)) {
            std::cout << "✓ Deadlock resolved!\n";
        }
    } else {
        std::cout << "\n✓ No deadlock detected\n";
    }
    
    return 0;
}
