#!/bin/bash
file=swaptions_truncated_1
rm run_lite.txt
rm valid_lite.txt
echo "Making...."
make clean; make

echo "Getting validation set..."
./simulate_cache_ref 32000 4 64 16 0 $file > valid_lite.txt

#echo "Getting calculated set..."
#./dsm 32000 32000 4 64 16 0 $file > run_lite.txt


echo "done!"