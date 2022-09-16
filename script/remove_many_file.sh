#! /bin/zsh

if [ -z "$1" ] || [ "$1" != "fat" ] && [ "$1" != "lfs" ]; then
    echo "please give me type ..."
    exit
fi

remove_1() {
    WRITE_COUNT=10
    # SYNC_TIMES=$(($WRITE_COUNT / 3))
    SYNC_TIMES=$(($WRITE_COUNT / 3))
    ../main --type $1 --file  WW00.txt --rm
    ../main --type $1 --file  WW01.txt --rm
    ../main --type $1 --file  WW02.txt --rm
    ../main --type $1 --file  WW03.txt --rm
    ../main --type $1 --file  WW04.txt --rm
    ../main --type $1 --file  WW05.txt --rm
    ../main --type $1 --file  WW06.txt --rm
    ../main --type $1 --file  WW07.txt --rm
    ../main --type $1 --file  WW08.txt --rm
    ../main --type $1 --file  WW09.txt --rm
    ../main --type $1 --file  WW10.txt --rm
    ../main --type $1 --file  WW11.txt --rm
    ../main --type $1 --file  WW12.txt --rm
    ../main --type $1 --file  WW13.txt --rm
    ../main --type $1 --file  WW14.txt --rm
    ../main --type $1 --file  WW15.txt --rm
    ../main --type $1 --file  WW16.txt --rm
    ../main --type $1 --file  WW17.txt --rm
    ../main --type $1 --file  WW18.txt --rm
    ../main --type $1 --file  WW19.txt --rm
    ../main --type $1 --file  WW20.txt --rm
    ../main --type $1 --file  WW21.txt --rm
    ../main --type $1 --file  WW22.txt --rm
    ../main --type $1 --file  WW23.txt --rm
    ../main --type $1 --file  WW24.txt --rm
    ../main --type $1 --file  WW25.txt --rm
    ../main --type $1 --file  WW26.txt --rm
    ../main --type $1 --file  WW27.txt --rm
    ../main --type $1 --file  WW28.txt --rm
    ../main --type $1 --file  WW29.txt --rm
    ../main --type $1 --file  WW30.txt --rm
    ../main --type $1 --file  WW31.txt --rm
    ../main --type $1 --file  WW32.txt --rm
    ../main --type $1 --file  WW33.txt --rm
    ../main --type $1 --file  WW34.txt --rm
    ../main --type $1 --file  WW35.txt --rm
    ../main --type $1 --file  WW36.txt --rm
    ../main --type $1 --file  WW37.txt --rm
    ../main --type $1 --file  WW38.txt --rm
    ../main --type $1 --file  WW39.txt --rm
}

remove_1 $1
../main --type $1 --list /
