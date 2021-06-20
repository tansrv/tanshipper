#! /bin/bash

pacman -S gcc make cmake openssl && \
chmod +x install.sh              && \
./install.sh
