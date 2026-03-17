cmake_minimum_required(VERSION 3.10)
project(HFT_OrderBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native -flto")

# For profiling (optional)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories(include)

add_executable(orderbook_demo 
    src/main.cpp 
    src/OrderBook.cpp 
    src/MarketDataFeed.cpp
)

target_link_libraries(orderbook_demo pthread)