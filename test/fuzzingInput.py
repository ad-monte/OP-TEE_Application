import sys
import time
import string
import statistics

import subprocess

TA_APP = "ta_secret -a "  # Command to test the token
TA_PWD = " Alfonso"
output_file = "/mnt/host/optee_examples/project/test/fuzzing.txt" #Change the name of the folder according to your project name


# Global child process
child = None


def fuzzingInput():

    
    # Genrate random input files. ta_secret -e <inputfile> <key> <password>

    result = subprocess.Popen("ta_secret -e test1.txt 1234" + TA_PWD)
    result = subprocess.Popen("ta_secret -e test1000.txt 1234" + TA_PWD)   # files doesn't exist 
    result = subprocess.Popen("ta_secret -e 1234" + TA_PWD)                # it doesn't have input file

    # generate more random keys

    subprocess.Popen("ta_secret -d ab" + TA_PWD)                            # short key
    subprocess.Popen("ta_secret -d abcdddddddddddddddddddddddddddddddddddddddddddddddddddd" + TA_PWD) # long key
    subprocess.Popen("ta_secret -d !@#$%^&*()_+" + TA_PWD)                  # special characters
    subprocess.Popen("ta_secret -d 1234abcd" + TA_PWD)                      # alphanumeric key
    subprocess.Popen("ta_secret -d " + TA_PWD)                              # empty key
    subprocess.Popen("ta_secret -d 0000" + TA_PWD)                          # repetitive characters

    # generate more random passwords
    subprocess.Popen("ta_secret -d 1234 " + "A" )                          # short password
    subprocess.Popen("ta_secret -d 1234 " + "A"*100 )                      # long password
    subprocess.Popen("ta_secret -d 1234 " + "!@#$%^&*()_+")                # special characters
    subprocess.Popen("ta_secret -d 1234 " + "abc123")                     # alphanumeric password

    # generate random number in access log function. ta_secret -a <entry range begin> <entry range end> <pwd>

    subprocess.Popen("ta_secret -a -1 10" + TA_PWD)          # negative begin range
    subprocess.Popen("ta_secret -a 10 -1" + TA_PWD)          # negative end range
    subprocess.Popen("ta_secret -a 10 5" + TA_PWD)           # begin range greater than end range
    subprocess.Popen("ta_secret -a 0 2147483647" + TA_PWD)   # maximum integer value
    subprocess.Popen("ta_secret -a abc def" + TA_PWD)        # non-integer input
    subprocess.Popen("ta_secret -a  " + TA_PWD)              # missing range values
    subprocess.Popen("ta_secret -a 5 5" + TA_PWD)            # begin range equal to end range




def main():
    global child

    print("Will start to try to fuzzing randomized input\n")
    fuzzingInput()
    print("Hacked")

if __name__ == '__main__':
    main()
