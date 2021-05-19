#! /bin/bash

cd ../src/build/ && cmake .. && make && make install

if [ $? -ne 0 ]; then
  echo -e "\ntanshipper: install failed\n"
fi
