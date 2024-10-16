# Docker is only used for development purposes
FROM gcc:latest

# Install any additional dependencies needed for debugging (e.g., gdb)
RUN apt-get update && \
    apt-get install -y gdb && \
    apt-get install -y libssl-dev && \
    apt-get install -y libcurl4-openssl-dev

# Set the working directory inside the container
WORKDIR /doomsdaywallet
COPY setup.sh .

RUN chmod +x setup.sh && \
    cd / && \
    wget https://archives.boost.io/release/1.82.0/source/boost_1_82_0.tar.gz && \
    tar -xf boost_1_82_0.tar.gz && \
    cd /boost_1_82_0/ && \
    ./bootstrap.sh && \
    ./b2 install

# Set a directory as a volume
VOLUME /doomsdaywallet
