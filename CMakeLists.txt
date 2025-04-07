cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME cbot)
project(${PROJECT_NAME} VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/WebSocket.cpp
    src/KrakenFeed.cpp
    src/BinanceFeed.cpp
    src/ServiceRunner.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        ${Boost_LIBRARIES}
)
