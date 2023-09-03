#!/bin/bash

if [ -d "./bin/" ];then
    if [ -n "$(ls -A ./bin)" ];then
        rm -r ./bin/*
        echo "Clean up directory bin successfully !"
        else
        echo "bin directory is empty !"
    fi
    else
    echo "bin directory does not exist !";
fi

if [ -d "./build/" ];then
    if [ -n "$(ls -A ./build)" ];then
        rm -r ./build/*
        echo "Clean up directory build successfully !"
        else
        echo "build directory is empty"
    fi
    else
    echo "build directory does not exist !"
fi
