#!/bin/bash

functiones(){
    echo $1
}

for arg in "$@"
do
    functiones $arg
done
