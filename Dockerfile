FROM ubuntu:20.04

# set non-interactive mode for apt-get
ENV DEBIAN_FRONTEND=noninteractive

# update and install build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libboost-all-dev \
    libssl-dev \
    nlohmann-json3-dev \
    cmake \
    git

WORKDIR /opt/app
COPY . /opt/app

RUN mkdir build && cd build && cmake .. && make -j$(nproc)

WORKDIR /opt/app/build
CMD ["./cbot"]
