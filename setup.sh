#!/bin/bash
# Must be executed after the development image is started to install secp256k1.
# On completion, run 'export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH'
# in the image's shell
# See README.md for details about development environment setup

cd secp256k1
./autogen.sh
./configure
make
make install
