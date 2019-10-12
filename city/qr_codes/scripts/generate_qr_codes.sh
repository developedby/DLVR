#!/bin/bash

for i in $(seq 1 $1)
do
    qrencode -o $i.png $i
done
