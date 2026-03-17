#pragma once
#include <map>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <string>
#include <atomic>
#include <chrono>   
#include <iostream>    


struct Order {
    uint64_t order_id;
    uint64_t price;
    uint32_t quantity;
    char side; // 'B' for buy, 'S' for sell
    uint64_t timestamp;
};

class OrderBook {
private:
    // Using std::map for price levels (ordered for best bid/ask)
    std::map<uint64_t, uint32_t, std::greater<uint64_t>> bids; // Buy orders (highest first)
    std::map<uint64_t, uint32_t> asks; // Sell orders (lowest first)
    
    // Fast lookup for order cancellation
    std::unordered_map<uint64_t, Order> orders;
    
    // Cache best bid and ask for ultra-fast access
    std::atomic<uint64_t> best_bid{0};
    std::atomic<uint64_t> best_ask{0};
    std::atomic<uint64_t> best_bid_qty{0};
    std::atomic<uint64_t> best_ask_qty{0};
    
    // For thread safety in production
    mutable std::mutex rw_mutex;

public:
    OrderBook() = default;
    
    // Core operations
    void add_order(uint64_t order_id, uint64_t price, uint32_t quantity, char side);
    bool cancel_order(uint64_t order_id);
    void execute_order(uint64_t order_id, uint32_t quantity);
    
    // Fast inline getters for best prices
    inline uint64_t get_best_bid() const { return best_bid.load(std::memory_order_acquire); }
    inline uint64_t get_best_ask() const { return best_ask.load(std::memory_order_acquire); }
    inline uint64_t get_spread() const { 
        uint64_t ask = best_ask.load(std::memory_order_acquire);
        uint64_t bid = best_bid.load(std::memory_order_acquire);
        return (ask > 0 && bid > 0) ? (ask - bid) : 0;
    }
    
    // Get market depth
    std::vector<std::pair<uint64_t, uint32_t>> get_bids(size_t depth = 5) const;
std::vector<std::pair<uint64_t, uint32_t>> get_asks(size_t depth = 5) const;
    
private:
    void update_best_prices();
};