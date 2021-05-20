#!/bin/bash
# for filename in ../traces/*.bz2; do
#     bunzip2 -kc $filename| ./predictor --static;
#     echo $filename
# done


for filename in ../traces/*.bz2; do
    echo $filename;
    bunzip2 -kc $filename| ./predictor --gshare:13;
done