#include "OrderBook.hpp"
#include "MarketDataFeed.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <iomanip>

void print_order_book(const OrderBook& book) {
    std::cout << "\n=== Order Book ===" << std::endl;
    std::cout << "Best Bid: " << book.get_best_bid() 
              << " | Best Ask: " << book.get_best_ask()
              << " | Spread: " << book.get_spread() << std::endl;
    
    auto bids = book.get_bids(3);
    auto asks = book.get_asks(3);
    
    std::cout << "\nBids (Price : Qty): ";
    for (const auto& [price, qty] : bids) {
        std::cout << price << ":" << qty << " ";
    }
    
    std::cout << "\nAsks (Price : Qty): ";
    for (const auto& [price, qty] : asks) {
        std::cout << price << ":" << qty << " ";
    }
    std::cout << std::endl;
}

void benchmark_order_book() {
    OrderBook book;
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simulate high-frequency updates
    const int NUM_OPERATIONS = 1'000'000;
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        // Alternating operations to simulate real market
        if (i % 3 == 0) {
            book.add_order(i, 100 + (i % 10), 100, i % 2 ? 'B' : 'S');
        } else if (i % 3 == 1) {
            book.cancel_order(i - 1);
        } else {
            book.execute_order(i - 2, 50);
        }
        
        // Access best prices frequently (simulating trading decisions)
        volatile uint64_t bid = book.get_best_bid();
        volatile uint64_t ask = book.get_best_ask();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\n=== Performance Benchmark ===" << std::endl;
    std::cout << "Operations: " << NUM_OPERATIONS << std::endl;
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << (NUM_OPERATIONS * 1000.0 / duration.count()) 
              << " ops/sec" << std::endl;
}

int main() {
    std::cout << "HFT Order Book System Demo" << std::endl;
    std::cout << "===========================" << std::endl;
    
    OrderBook book;
    
    // Demo basic functionality
    std::cout << "\n--- Adding Orders ---" << std::endl;
    book.add_order(1, 100, 1000, 'B');  // Buy 1000 @ 100
    book.add_order(2, 101, 500, 'B');   // Buy 500 @ 101
    book.add_order(3, 102, 200, 'S');   // Sell 200 @ 102
    book.add_order(4, 103, 800, 'S');   // Sell 800 @ 103
    
    print_order_book(book);
    
    std::cout << "\n--- Executing Order ---" << std::endl;
    book.execute_order(3, 150);  // Execute 150 from order 3
    
    print_order_book(book);
    
    std::cout << "\n--- Cancelling Order ---" << std::endl;
    book.cancel_order(1); 
    
    print_order_book(book);
    
    // Run benchmark
    benchmark_order_book();
    
    return 0;
}