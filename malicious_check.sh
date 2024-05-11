#!/bin/bash

# l w m
lines=(`sudo wc -l $1 | awk '{print $1}'`)
words=(`sudo wc -w $1 | awk '{print $1}'`)
chars=(`sudo wc -m $1 | awk '{print $1}'`)

# echo "Numar de linii:" $lines
# echo "Numar de cuvinte:" $words
# echo "Numar de caractere:" $chars

if test $lines -le 1 && test $words -gt 1000 && test $chars -gt 2000
then
    # corrupted, dangerous, risk, attack, malware, malicious
    keys=(`sudo grep -Eo '\b(corrupted|dangerous|risk|attack|malware|malicious)\b' $1 | wc -l`)
    # echo "Numar de cuvinte cheie periculoase:" $keys
    
    # ascii char -> 0-127 (00-7F)
    non_ascii=(`sudo grep -Po "[^\x00-\x7F]" $1 | wc -l`)
    # echo "Numar de caractere non-ascii:" $non_ascii

    # verfic daca exista cuvinte cheie sau caractere non-ascii
    if test $keys -ne 0 || test $non_ascii -ne 0
    then
        # echo $1 | awk -F'/' '{print $NF}'
	exit 1
    fi
fi

# echo "SAFE"
exit 0
