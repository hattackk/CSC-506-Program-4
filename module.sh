#!bin/sh


if [ -f result.txt ]
then
	rm -f result.txt
fi
if [ -f ref_result.txt ]
then
	rm -f ref_result.txt
fi
if [ -f difference.txt ]
then
	rm -f difference.txt
fi

	./simulate_cache_ref $1 $2 $3 $4 $5 $6 $7> ref_result.txt
	./dsm $1 $2 $3 $4 $5 $6 $7> result.txt
	diff result.txt ref_result.txt > difference.txt


cat difference.txt


