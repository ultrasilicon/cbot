cmake_minimum_required(VERSION 3.16)
project(cbot VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/deps")

# Copy public/index.html into the build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/public/index.html   # source
    ${CMAKE_BINARY_DIR}/index.html          # destination in build/
    COPYONLY
)

find_package(Boost REQUIRED COMPONENTS system thread iostreams)
find_package(OpenSSL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG master
)
FetchContent_MakeAvailable(httplib)


set(SOURCES
    src/main.cpp
    src/Cbot.cpp
    src/ChartServer.cpp
    src/KrakenFeed.cpp
    src/BinanceFeed.cpp
    src/CoinbaseFeed.cpp
    src/WebSocket.cpp
)

set(HEADERS
    include/Cbot.hpp
    include/Algorithm.hpp
    include/BookTicker.hpp
    include/KrakenFeed.hpp
    include/BinanceFeed.hpp
    include/CoinbaseFeed.hpp
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
        httplib::httplib
)
