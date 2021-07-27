#!/bin/bash

protocol=0
cachesize=32000
assoc=4
blocksize=64
procs=16



file=swaptions_truncated_1_1_50
rm run_lite.txt
rm valid_lite.txt
echo "Making...."
make clean; make

echo "|-------------------------|"
echo "| Sim Params              |"
echo "|-------------------------|"
echo "| protocol=====> [$protocol]"
echo "| cachesize====> [$cachesize]"
echo "| association==> [$assoc]"
echo "| blocksize====> [$blocksize]"
echo "| processors===> [$procs]"
echo "|-------------------------|"

if [[ $1 -eq 1 ]]
then
echo "Getting validation set..."
./simulate_cache_ref $cachesize $assoc $blocksize $procs $protocol $file > valid_lite.txt
fi
echo "Getting calculated set..."
./dsm $cachesize $assoc $blocksize $procs $protocol $file $file > run_lite.txt


echo "done!"