#include "MarketDataFeed.hpp"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>

MarketDataFeed::~MarketDataFeed() {
    stop();
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

bool MarketDataFeed::connect(const std::string& ip, int port) {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    // Set socket options for low latency
    int rcvbuf = 64 * 1024 * 1024; // 64MB buffer
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        return false;
    }
    
    return true;
}

void MarketDataFeed::start() {
    running = true;
    feed_thread = std::thread(&MarketDataFeed::feed_loop, this);
}

void MarketDataFeed::stop() {
    running = false;
    if (feed_thread.joinable()) {
        feed_thread.join();
    }
}

void MarketDataFeed::feed_loop() {
    char buffer[2048];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (running) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        ssize_t bytes = recvfrom(socket_fd, buffer, sizeof(buffer), 0,
                                 (struct sockaddr*)&sender, &sender_len);
        
        if (bytes > 0) {
            parse_and_process(buffer, bytes);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(
                end_time - start_time).count();
            
            total_latency_ns += latency;
            messages_processed++;
        }
    }
}

void MarketDataFeed::parse_and_process(const char* buffer, size_t length) {
    // Simple message format: "A,order_id,price,quantity,side" for add
    // "C,order_id" for cancel
    // "E,order_id,quantity" for execute
    
    std::string msg(buffer, length);
    char msg_type = msg[0];
    
    size_t pos = 2; 
    
    if (msg_type == 'A') {
        uint64_t order_id = std::stoull(&msg[pos]);
        pos = msg.find(',', pos) + 1;
        uint64_t price = std::stoull(&msg[pos]);
        pos = msg.find(',', pos) + 1;
        uint32_t qty = std::stoul(&msg[pos]);
        pos = msg.find(',', pos) + 1;
        char side = msg[pos];
        
        order_book.add_order(order_id, price, qty, side);
    }
    else if (msg_type == 'C') {
        uint64_t order_id = std::stoull(&msg[pos]);
        order_book.cancel_order(order_id);
    }
    else if (msg_type == 'E') {
        uint64_t order_id = std::stoull(&msg[pos]);
        pos = msg.find(',', pos) + 1;
        uint32_t qty = std::stoul(&msg[pos]);
        order_book.execute_order(order_id, qty);
    }
}

double MarketDataFeed::get_avg_latency_ms() const {
    uint64_t count = messages_processed.load();
    if (count == 0) return 0;
    return (total_latency_ns.load() / count) / 1'000'000.0;
}