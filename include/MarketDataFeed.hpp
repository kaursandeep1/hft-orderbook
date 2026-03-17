#pragma once
#include "OrderBook.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>

class MarketDataFeed {
private:
    OrderBook& order_book;
    int socket_fd;
    struct sockaddr_in addr;
    std::atomic<bool> running{false};
    std::thread feed_thread;
    
    // Performance monitoring
    std::atomic<uint64_t> messages_processed{0};
    std::atomic<uint64_t> total_latency_ns{0};

public:
    MarketDataFeed(OrderBook& book) : order_book(book) {}
    ~MarketDataFeed();
    
    bool connect(const std::string& ip, int port);
    void start();
    void stop();
    
    // Stats for profiling
    double get_avg_latency_ms() const;
    uint64_t get_message_count() const { return messages_processed.load(); }
    
private:
    void feed_loop();
    void parse_and_process(const char* buffer, size_t length);
};