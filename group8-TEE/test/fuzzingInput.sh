#!/bin/bash    
    
    ta_secret -e test1.txt 1234 Alfonso
    ta_secret -e test1000.txt 1234 Alfonso   # files doesn't exist 
    ta_secret -e 1234 Alfonso                # it doesn't have input file

    # generate more random keys

    ta_secret -d ab Alfonso                            # short key
    ta_secret -d abcdddddddddddddddddddddddddddddddddddddddddddddddddddd Alfonso # long key
    ta_secret -d !@#$%^   Alfonso                  # special characters
    ta_secret -d 1234abcd Alfonso                      # alphanumeric key
    ta_secret -d  Alfonso                              # empty key
    ta_secret -d 0000 Alfonso                          # repetitive characters

    # generate more random passwords
    ta_secret -d 1234 A                          # short password
    ta_secret -d 1234 $(printf 'A%.0s' {1..100}) # long password
    ta_secret -d 1234 '!@#$%^&*()_+'            # special characters
    ta_secret -d 1234 abc123                    # alphanumeric password

    # generate random number in access log function. ta_secret -a <entry range begin> <entry range end> <pwd>

    ta_secret -a -1 10 Alfonso          # negative begin range
    ta_secret -a 10 -1 Alfonso          # negative end range
    ta_secret -a 10 5 Alfonso           # begin range greater than end range
    ta_secret -a 0 2147483647 Alfonso   # maximum integer value
    ta_secret -a abc def Alfonso        # non-integer input
    ta_secret -a  Alfonso              # missing range values
    ta_secret -a 5 5 Alfonso            # begin range equal to end range
