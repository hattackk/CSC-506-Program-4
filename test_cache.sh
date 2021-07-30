#!/bin/bash

OUTN=0
swaptions_truncated='/afs/eos.ncsu.edu/lockers/workspace/csc/CSC506-1/trace/swaptions_truncated'
for protocol in 0 1
do
 for cachesize in 32000 64000 128000 256000 512000
 do
  
  echo "./simulate_cache_ref $cachesize 4 64 16 $protocol $swaptions_truncated" > ref.$OUTN
  ./simulate_cache_ref $cachesize 4 64 16 $protocol $swaptions_truncated >> ref.$OUTN
  
  OUTN=$(($OUTN+1))
 done
 for blocksize in 64 128 256
 do
  
  echo "./simulate_cache_ref 1000000 4 $blocksize 16 $protocol $swaptions_truncated" > ref.$OUTN
  ./simulate_cache_ref 256000 4 $blocksize 16 $protocol $swaptions_truncated >> ref.$OUTN
  
  OUTN=$(($OUTN+1))
 done
done
