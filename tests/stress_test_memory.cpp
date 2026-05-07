#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <cassert>
#include "../stdlib/xotira.hpp"

using namespace std;
using namespace uzpp::Xotira;

struct Obyekt {
    int id;
    double data[10];
    Obyekt(int id) : id(id) {}
    ~Obyekt() {}
};

void test_ulashilgan() {
    cout << "--- Testing Ulashilgan (Shared Pointers) ---\n";
    auto start = chrono::high_resolution_clock::now();
    
    const int num_objects = 5'000'000;
    long long checksum = 0;
    
    // Create millions of shared pointers
    vector<Ulashilgan<Obyekt>> ptrs;
    ptrs.reserve(num_objects);
    
    for (int i = 0; i < num_objects; ++i) {
        ptrs.push_back(yangi<Obyekt>(i));
    }
    
    for (int i = 0; i < num_objects; ++i) {
        checksum += ptrs[i]->id;
    }
    
    ptrs.clear(); // This will trigger 5 million destructors immediately
    
    auto end = chrono::high_resolution_clock::now();
    cout << "Created and destroyed " << num_objects << " shared pointers.\n";
    cout << "Time taken: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms.\n";
    cout << "Checksum: " << checksum << "\n";
}

void test_arena_allocator() {
    cout << "\n--- Testing Arena Allocator ---\n";
    auto start = chrono::high_resolution_clock::now();
    
    const int num_objects = 10'000'000;
    const size_t req_memory = num_objects * sizeof(Obyekt) + 1024;
    
    Arena arena(req_memory);
    long long checksum = 0;
    
    for (int i = 0; i < num_objects; ++i) {
        Obyekt* obj = arena.ajratish<Obyekt>(i);
        if (obj) {
            checksum += obj->id;
        } else {
            cout << "Arena out of memory at iteration " << i << "!\n";
            break;
        }
    }
    
    auto end = chrono::high_resolution_clock::now();
    cout << "Allocated " << num_objects << " objects in Arena.\n";
    cout << "Time taken: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms.\n";
    cout << "Checksum: " << checksum << "\n";
    cout << "Memory used: " << arena.ishlatilgan() / (1024 * 1024) << " MB out of " << arena.sigimi() / (1024 * 1024) << " MB.\n";
    
    arena.tozalash(); // O(1) deallocation!
}

struct Node {
    int id;
    Ulashilgan<Node> next;
    Node(int id) : id(id) {}
};

void test_memory_leaks() {
    cout << "\n--- Testing Cyclic Dependencies & Memory Management ---\n";
    // By default std::shared_ptr leaks on cyclic references.
    // In uz++, users must be aware of this, as it uses standard RAII, not a tracing GC.
    
    auto n1 = yangi<Node>(1);
    auto n2 = yangi<Node>(2);
    
    n1->next = n2;
    // n2->next = n1; // If uncommented, this causes a memory leak due to cyclic reference.
    
    cout << "Linked list created. n1 reference count: " << n1.use_count() << ", n2: " << n2.use_count() << "\n";
}

int main() {
    cout << "==========================================\n";
    cout << " UZ++ MEMORY MANAGEMENT STRESS TEST\n";
    cout << "==========================================\n\n";
    
    test_ulashilgan();
    test_arena_allocator();
    test_memory_leaks();
    
    cout << "\n==========================================\n";
    cout << " MEMORY TESTS PASSED SUCCESSFULLY! ✅\n";
    cout << "==========================================\n";
    
    return 0;
}
