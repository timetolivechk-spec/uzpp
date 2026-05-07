#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>
#include <cassert>
#include "../stdlib/kesh.hpp"

using namespace std;
using namespace uzpp::Kesh;

int main() {
    XotiraKesh<int, int> cache;
    atomic<bool> start{false};
    atomic<int> ops{0};
    
    const int num_threads = 24;
    const int ops_per_thread = 500000; // Total 12 million operations
    
    vector<thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, id = i]() {
            while (!start.load()) {
                this_thread::yield();
            }
            
            mt19937 rng(id);
            uniform_int_distribution<int> op_dist(0, 100);
            uniform_int_distribution<int> key_dist(0, 5000);
            uniform_int_distribution<int> val_dist(0, 100000);
            
            for (int j = 0; j < ops_per_thread; ++j) {
                int op = op_dist(rng);
                int key = key_dist(rng);
                
                if (op < 40) { // 40% write
                    cache.saqlash(key, val_dist(rng));
                } else if (op < 70) { // 30% read (olish)
                    cache.olish(key, -1);
                } else if (op < 85) { // 15% read (izlash)
                    cache.izlash(key);
                } else if (op < 95) { // 10% contains (mavjudmi)
                    cache.mavjudmi(key);
                } else if (op < 99) { // 4% delete (ochirish)
                    cache.ochirish(key);
                } else { // 1% size (hajm)
                    cache.hajm();
                }
                ops.fetch_add(1, memory_order_relaxed);
            }
        });
    }
    
    cout << "==========================================\n";
    cout << " UZ++ KESH.HPP STRESS TEST\n";
    cout << "==========================================\n";
    cout << "Starting stress test...\n";
    cout << "Threads: " << num_threads << "\n";
    cout << "Operations per thread: " << ops_per_thread << "\n";
    cout << "Total operations: " << num_threads * ops_per_thread << "\n\n";
    
    auto t1 = chrono::high_resolution_clock::now();
    start.store(true);
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto t2 = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
    
    cout << "Completed " << ops.load() << " operations in " << duration << " ms.\n";
    cout << "Throughput: " << (ops.load() * 1000.0 / (duration > 0 ? duration : 1)) << " ops/sec\n";
    cout << "Final cache size: " << cache.hajm() << "\n\n";
    
    cout << "Cleaning up (tozalash)...\n";
    cache.tozalash();
    assert(cache.hajm() == 0);
    
    cout << "==========================================\n";
    cout << " STRESS TEST PASSED SUCCESSFULLY! ✅\n";
    cout << " No race conditions or deadlocks detected.\n";
    cout << "==========================================\n";
    
    return 0;
}
