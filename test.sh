#!/bin/bash

count="0"
while [ $count -le 15 ]
do
    fname="./test_files/test$count"
    ./tests $count &> $fname.out
    d=$(diff $fname.out $fname.exp)
    if [ "$d" ]
    then
        echo test $count failed
        echo $d
    else
        echo test $count passed
        suc=`expr $suc + 1`
    fi
    rm $fname.out
    count=`expr $count + 1`
done
