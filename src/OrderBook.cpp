#include "OrderBook.hpp"
#include <mutex>
#include <algorithm>

void OrderBook::add_order(uint64_t order_id, uint64_t price, 
                          uint32_t quantity, char side) {
    std::unique_lock<std::mutex> lock(rw_mutex);
    
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
    
    Order order{order_id, price, quantity, side, 
                static_cast<uint64_t>(nanos)};  
    orders[order_id] = order;
    
    if (side == 'B') {
        bids[price] += quantity;
    } else {
        asks[price] += quantity;
    }
    
    update_best_prices();
}

bool OrderBook::cancel_order(uint64_t order_id) {
    std::unique_lock<std::mutex> lock(rw_mutex);
    
    auto it = orders.find(order_id);
    if (it == orders.end()) return false;
    
    if (it->second.side == 'B') {
        auto level = bids.find(it->second.price);
        if (level != bids.end()) {
            level->second -= it->second.quantity;
            if (level->second == 0) {
                bids.erase(level);
            }
        }
    } else {
        auto level = asks.find(it->second.price);
        if (level != asks.end()) {
            level->second -= it->second.quantity;
            if (level->second == 0) {
                asks.erase(level);
            }
        }
    }
    
    orders.erase(it);
    update_best_prices();
    return true;
}

void OrderBook::execute_order(uint64_t order_id, uint32_t quantity) {
    std::unique_lock<std::mutex> lock(rw_mutex);
    
    auto it = orders.find(order_id);
    if (it == orders.end()) return;
    
    // Reduce quantity
    it->second.quantity -= quantity;
    
    // Update price level
    if (it->second.side == 'B') {
        auto level = bids.find(it->second.price);
        if (level != bids.end()) {
            level->second -= quantity;
            if (level->second == 0) {
                bids.erase(level);
            }
        }
    } else {
        auto level = asks.find(it->second.price);
        if (level != asks.end()) {
            level->second -= quantity;
            if (level->second == 0) {
                asks.erase(level);
            }
        }
    }
    
    // Remove if fully executed
    if (it->second.quantity == 0) {
        orders.erase(it);
    }
    
    update_best_prices();
}

void OrderBook::update_best_prices() {
    // Update best bid
    if (!bids.empty()) {
        auto best = bids.begin();
        best_bid.store(best->first, std::memory_order_release);
        best_bid_qty.store(best->second, std::memory_order_release);
    } else {
        best_bid.store(0, std::memory_order_release);
        best_bid_qty.store(0, std::memory_order_release);
    }
    
    // Update best ask
    if (!asks.empty()) {
        auto best = asks.begin();
        best_ask.store(best->first, std::memory_order_release);
        best_ask_qty.store(best->second, std::memory_order_release);
    } else {
        best_ask.store(0, std::memory_order_release);
        best_ask_qty.store(0, std::memory_order_release);
    }
}

std::vector<std::pair<uint64_t, uint32_t>> OrderBook::get_bids(size_t depth) const {
    std::unique_lock<std::mutex> lock(rw_mutex);
    std::vector<std::pair<uint64_t, uint32_t>> result;
    result.reserve(std::min(depth, bids.size()));
    
    auto it = bids.begin();
    for (size_t i = 0; i < depth && it != bids.end(); ++i, ++it) {
        result.emplace_back(it->first, it->second);
    }
    return result;
}

std::vector<std::pair<uint64_t, uint32_t>> OrderBook::get_asks(size_t depth) const {
    std::unique_lock<std::mutex> lock(rw_mutex);
    std::vector<std::pair<uint64_t, uint32_t>> result;
    result.reserve(std::min(depth, asks.size()));
    
    auto it = asks.begin();
    for (size_t i = 0; i < depth && it != asks.end(); ++i, ++it) {
        result.emplace_back(it->first, it->second);
    }
    return result;
}