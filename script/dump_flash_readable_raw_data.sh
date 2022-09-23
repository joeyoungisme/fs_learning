#! /bin/bash

if [ -z "$1" ] || [ "$1" != "fat" ] && [ "$1" != "lfs" ]; then
    echo "please give me type ..."
    exit
fi

../main --type $1 --dump rraw

# ../main --type $1 --dump rraw --dnl 16
