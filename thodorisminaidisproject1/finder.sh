#!/bin/bash
for args in $*;
do
    counter=0
    for dir in out_directory/*.out;
    do
        while read line
        do
            list=($line)
            if [[ ${list[0]} == *$args ]];
            then
                counter=$((counter+${list[1]}))
            fi
        done < $dir
    done
    echo $args $counter
done
