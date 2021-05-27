#!/bin/bash
# for filename in ../traces/*.bz2; do
#     bunzip2 -kc $filename| ./predictor --static;
#     echo $filename
# done

for filename in ../traces/*.bz2; do
    echo $filename;
    bunzip2 -kc $filename| ./predictor --tournament:10:9:10;
done

# for filename in ../traces/*.bz2; do
#     echo $filename;
#     bunzip2 -kc $filename| ./predictor --custom:10:9:10;
# done