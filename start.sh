#!/bin/bash


if [ ! -d "./build/" ];then
    mkdir ./build
    echo "Directory build created successfully !"
fi

if [ ! -d "./bin/" ];then
    mkdir ./bin
    echo "Directory bin created successfully !"
fi

if [ -n "$(ls -A ./bin)" ];then
    rm ./bin/*
fi

if [ -n "$(ls -A ./build)" ];then
    rm ./build/*
fi

cd build

cmake ..

make

cd ../

if [ -n "$(ls -A ./bin)" ];then
    echo "compile successfully! The executable file is in the bin directory."
    else
    echo "build executable failed !"
fi

