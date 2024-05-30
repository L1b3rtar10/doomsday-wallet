# Docker is only used for development purposes
FROM gcc:latest

# Install any additional dependencies needed for debugging (e.g., gdb)
RUN apt-get update && apt-get install -y gdb

# Set the working directory inside the container
WORKDIR /doomsdaywallet

# Set a directory as a volume
VOLUME /doomsdaywallet
