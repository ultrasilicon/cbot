# cbot

A CEX analytics bot in C++ for fun


## Dependencies

- Boost libraries
- OpenSSL

## Build Instructions

### Local build

Open a terminal and run the following commands:

```sh
mkdir build && cd build
cmake ..
make -j$(nproc)

# start the bot with:
./cbot
```

### docker

```sh
docker build . -t cbot
docker run -it cbot
```

