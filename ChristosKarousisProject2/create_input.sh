#!/bin/bash

if [ "$#" -ne 2 ] 
then 
    echo "Wrong arguments"
    exit 1
fi

politicalParties=$1
numLines=$2
outputFile="inputFile"

if [ ! -e $politicalParties ] 
then 
    echo "File does not exist"
    exit 1
fi

if [ ! -r $politicalParties ] 
then 
    echo "I cant read this file !!!"
    exit 1
fi


for ((i=1; i<=$numLines; i++)); do
    number=$RANDOM
    name=$(cat /dev/urandom | tr -dc '[:alpha:]' | fold -w $((number % 10 + 3)) | head -n 1)
    surname=$(cat /dev/urandom | tr -dc '[:alpha:]' | fold -w $((number % 10 + 3)) | head -n 1) 
    party=$(shuf -n 1 "$politicalParties")
    echo "$name $surname $party" >> "$outputFile"
done