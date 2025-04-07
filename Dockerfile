FROM ubuntu:20.04

# Set non-interactive mode for apt-get
ENV DEBIAN_FRONTEND=noninteractive

# Update and install build tools, Boost, SSL libraries, and nlohmann-json4-dev
RUN apt-get update && apt-get install -y \
    build-essential \
    libboost-all-dev \
    libssl-dev \
    nlohmann-json3-dev \
    cmake

# Create and set the working directory
WORKDIR /opt/app

# Copy the source code into the container
COPY . /opt/app

# Compile the C++ program. Adjust "main.cpp" to your source file name if needed.
RUN mkdir build && cd build && cmake .. && make -j$(nproc)

# Run the executable
CMD ["./build/cbot"]
