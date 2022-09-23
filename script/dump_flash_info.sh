#! /bin/bash

if [ -z "$1" ] || [ "$1" != "fat" ] && [ "$1" != "lfs" ]; then
    echo "please give me type ..."
    exit
fi

../main --type $1 --dump info

# dnl set how many unit print then next line
# dal set how many space for erase and write count info
# ../main --type $1 --dump info --dnl 12 --dal 4
