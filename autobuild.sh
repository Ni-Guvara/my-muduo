#!/bin/bash

if [ ! -d "build" ];then
mkdir build
fi

cd build
rm -rf `pwd`/*
cmake ..
make