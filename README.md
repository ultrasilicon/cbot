# cbot

A CEX analytics bot in C++ for fun

## Dependencies

- Boost libraries
- OpenSSL
- GnuPlot

## Build Dependency

### macOS
Install dependencies using Homebrew:
```sh
brew install boost openssl gnuplot cmake
```

### Debian/Ubuntu

Install dependencies using APT:

```sh
sudo apt install -y libboost-all-dev libssl-dev gnuplot nlohmann-json3-dev build-essential cmake
```

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
> Note: docker container has no GUI support (requires display device redirection support)
```sh
docker build . -t cbot
docker run -it cbot
```

