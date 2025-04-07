# cbot

A CEX analytics bot in C++ for fun


## Prerequisites

- A C++ compiler that supports C++17
- [CMake](https://cmake.org)
- Boost libraries
- OpenSSL

## Build Instructions

Open a terminal and run the following commands:

```sh
mkdir build && cd build
cmake ..
make -j$(nproc)
./cbot
```
