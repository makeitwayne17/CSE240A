#!/bin/bash
# for filename in ../traces/*.bz2; do
#     bunzip2 -kc $filename| ./predictor --static;
#     echo $filename
# done

echo -e "\n\nTournament:\n\n"
for filename in ../traces/*.bz2; do
    echo $filename;
    bunzip2 -kc $filename| ./predictor --tournament:9:10:10;
done

# echo -e "\n\nCustom:\n\n"
# for filename in ../traces/*.bz2; do
#     echo $filename;
#     bunzip2 -kc $filename| ./predictor --custom:9:10:10;
# done

# echo -e '\n\nGSHARE:\n\n'
# for filename in ../traces/*.bz2; do
#     echo $filename;
#     bunzip2 -kc $filename| ./predictor --gshare:10;
# done
