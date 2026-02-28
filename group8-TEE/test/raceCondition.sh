#!/bin/bash

echo " TEST 1 : Check base line behaviour commands are exceuted in serial order"
rm log.txt
rm log2.txt
rm log3.txt
ta_secret -e test1.txt 12 Alfonso   >> log.txt
ta_secret -e test2.txt 11 Alfonso   >> log.txt
ta_secret -e test3.txt 14 Alfonso   >> log.txt
ta_secret -e test1.txt 67 Alfonso   >> log.txt
ta_secret -e test2.txt 14 Alfonso   >> log.txt
ta_secret -e test3.txt 43 Alfonso   >> log.txt
ta_secret -e test1.txt 14 Alfonso   >> log.txt
ta_secret -e test2.txt 98 Alfonso   >> log.txt
ta_secret -e test3.txt 22 Alfonso   >> log.txt
rm log.txt
ta_secret -a 0 9 Alfonso            >> log.txt

# if statement to check if 3 decoded lines are present
if [ $(grep -c "Encrypting" log.txt) -eq 9 ]; then
    echo "Result: All encryption commands are register in the log."
else
    echo "Result: Some decryption commands are not register."
fi

## ------------------------------------------------------------------###
echo " TEST 2: Check Concurrency stress tests with commands in parallel"


ta_secret -d 14 Alfonso             >> log2.txt & 
ta_secret -d 14 Alfonso             >> log2.txt &   
ta_secret -d 14 Alfonso             >> log2.txt &
ta_secret -d 14 Alfonso             >> log2.txt & 
ta_secret -d 14 Alfonso             >> log2.txt &   
ta_secret -d 14 Alfonso             >> log2.txt &
ta_secret -d 14 Alfonso             >> log2.txt &
ta_secret -d 14 Alfonso             >> log2.txt & 
ta_secret -d 14 Alfonso             >> log2.txt &   
wait

rm log2.txt
ta_secret -a 0 9 Alfonso            >> log2.txt

registeredDecryptions=$(grep -c "Decrypting" log2.txt)

#if statement to check if 6 decoded lines are present
if [ "$registeredDecryptions" -eq 9 ]; then
    echo "Result: All decryption commands registered in the log successfully."
else
    echo "Result: Some decryption commands are not registered - Race condition occurred"
    expectedDecryptions=9
    echo "Result: Total registered decryption commands: $registeredDecryptions of $expectedDecryptions"
fi

echo " Fisnish test "