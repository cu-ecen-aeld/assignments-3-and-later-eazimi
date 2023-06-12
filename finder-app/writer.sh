#!/bin/bash

writefile=$1
writestr=$2

if [ $# -ne 2 ]
then
    exit 1
fi

mkdir -p $(dirname $writefile) && touch $writefile

if [ ! -f $writefile ]
then
    echo "file could not be created"
    exit 1
fi

echo $writestr > $writefile