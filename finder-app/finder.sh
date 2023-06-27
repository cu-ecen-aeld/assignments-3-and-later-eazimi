#!/bin/sh

filesdir=$1
searchstr=$2

if [ $# -ne 2 ]
then
    exit 1
fi

if [ ! -d $filesdir ]
then
    echo "$filesdir does not represent a directory on the filesystem"
    exit 1
fi

files=$(find $filesdir -type f)
x=$(ls $files | wc -l)

for f in $files
do
    tmp=$(grep -c $searchstr $f)
    y=$((y + tmp))
done

echo  "The number of files are $x and the number of matching lines are $y"
