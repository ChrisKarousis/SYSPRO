#!/bin/bash

if [ "$#" -ne 1 ]
then
    echo "Wrong arguments"
    exit 1
fi

inputFile="inputFile"
tallyResultsFile="$1"

if [ ! -e $inputFile ] 
then 
    echo "File does not exist"
    exit 1
fi

if [ ! -r $inputFile ] 
then 
    echo "I cant read this file"
    exit 1
fi

declare -A partyVotes

while read -r line
do
    party=$(echo "$line" | awk '{print $3}')
    if [[ -z "${partyVotes[$party]}" ]]
    then
        partyVotes[$party]=1
    fi
done < "$inputFile"

for party in "${!partyVotes[@]}"
do
    echo "$party ${partyVotes[$party]}"
done > "$tallyResultsFile"