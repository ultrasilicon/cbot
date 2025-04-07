cmake_minimum_required(VERSION 3.16)
project(cbot VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)
find_package(Boost REQUIRED COMPONENTS system thread iostreams)
find_package(OpenSSL REQUIRED)

set(SOURCES
    src/main.cpp
    src/KrakenFeed.cpp
    src/BinanceFeed.cpp
    src/ServiceRunner.cpp
    src/WebSocket.cpp
)

set(HEADERS
    include/BookTicker.hpp
    include/KrakenFeed.hpp
    include/BinanceFeed.hpp
    include/ServiceRunner.hpp
    include/WebSocket.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # Qt6::Widgets
        # Qt6::Charts
        OpenSSL::SSL
        OpenSSL::Crypto
        ${Boost_LIBRARIES}
)
