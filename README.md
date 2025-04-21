# cbot

A CEX analytics bot in C++ for fun

## Dependencies

- Boost libraries
- OpenSSL
- `cpp-httplib`

## Build Dependency

### macOS
Install dependencies using Homebrew:
```sh
brew install boost openssl gnuplot cmake
```

### Debian/Ubuntu

Install dependencies using APT:

```sh
sudo apt install -y libboost-all-dev libssl-dev gnuplot nlohmann-json3-dev build-essential cmake git
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

```sh
docker build . -t cbot
docker run -it -p 18080:18080 cbot
```

