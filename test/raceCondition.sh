#!/bin/bash

echo "Check base line behaviour commands are exceuted in serial order"
rm log.txt
rm log2.txt
rm log3.txt
ta_secret -e test1.txt 14 Alfonso   >> log.txt
ta_secret -d 14 Alfonso             >> log.txt
ta_secret -e test2.txt 14 Alfonso   >> log.txt
ta_secret -d 14 Alfonso             >> log.txt
ta_secret -e test3.txt 14 Alfonso   >> log.txt
ta_secret -d 14 Alfonso             >> log.txt
ta_secret -e test1.txt 14 Alfonso   >> log.txt
ta_secret -d 14 Alfonso             >> log.txt
ta_secret -e test2.txt 14 Alfonso   >> log.txt

echo "Encryption and Decryption 9 commands executed in serial order completed"
# if statement to check if 3 decoded lines are present
if [ $(grep -c "Decoded" log.txt) -eq 4 ]; then
    echo "All decryption commands completed successfully."
else
    echo "Some decryption commands failed."
fi

ta_secret -a 0 9 Alfonso >> checkBaseline.txt

## ------------------------------------------------------------------###
echo " ---- Check Concurrency stress tests with commands in parallel ----- "

ta_secret -e test1.txt 14 Alfonso   >> log2.txt &
ta_secret -d 14 Alfonso             >> log2.txt &
ta_secret -e test2.txt 14 Alfonso   >> log2.txt &   
ta_secret -d 14 Alfonso             >> log2.txt &   
ta_secret -e test3.txt 14 Alfonso   >> log2.txt & 
ta_secret -d 14 Alfonso             >> log2.txt &
ta_secret -e test1.txt 14 Alfonso   >> log2.txt &
ta_secret -d 14 Alfonso             >> log2.txt &
ta_secret -e test2.txt 14 Alfonso   >> log2.txt &   
ta_secret -d 14 Alfonso             >> log2.txt &   
ta_secret -e test3.txt 14 Alfonso   >> log2.txt &
ta_secret -d 14 Alfonso             >> log2.txt &

echo "Encryption and Decryption 3 commands executed in parallel completed"
wait

#if statement to check if 6 decoded lines are present
if [ $(grep -c "Decoded" log2.txt) -eq 6 ]; then
    echo "All decryption commands completed successfully."
else
    echo "Some decryption commands failed."
fi

echo "exploit vulnerability and exceute ta_secret -a multiple time in parallel"

ta_secret -a 0 9 Alfonso >> log3.txt &
ta_secret -a 0 5 Alfonso >> log3.txt &
ta_secret -a 0 9 Alfonso >> log3.txt &
ta_secret -a 0 4 Alfonso >> log3.txt &
ta_secret -a 0 9 Alfonso >> log3.txt &
ta_secret -a 0 2 Alfonso >> log3.txt &
ta_secret -a 0 4 Alfonso >> log3.txt &
ta_secret -a 0 9 Alfonso >> log3.txt &
ta_secret -a 0 2 Alfonso >> log3.txt &

wait

if [ $(grep -c "valid" log3.txt) -eq 9 ]; then
    echo "All log commands completed successfully #9."
else
    echo "Some log commands failed"
fi

echo "finish parallel processing" 
ta_secret -a 0 9 Alfonso  >> checkConcurrency.txt

echo "Check log"