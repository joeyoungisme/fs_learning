#! /bin/bash

if [ -z "$1" ] || [ "$1" != "fat" ] && [ "$1" != "lfs" ]; then
    echo "please give me type ..."
    exit
fi

CYCLE=5

gen_test_file() {

    FILE_SIZE=$(expr 512 \* 1024)

    mkdir -p ./temp

    echo "[TEST SCRIPT] Gen Test File \"$1\""
    rm -f ./temp/$1

    dd if=/dev/urandom of=./temp/$1 bs=1 count=$FILE_SIZE
    
}
write_to_fs() {

    echo "[TEST SCRIPT] [WRITE] [$2]"

    ../main --type $1 --file $2 --put ./temp/$2
}
get_from_fs() {

    mkdir -p get
    echo "[TEST SCRIPT] [GET] [$2]"
    ../main --type $1 --file $2 --get ./get/$2
}

rm_fs_file() {
    ../main --type $1 --file $2 --rm
}

gen_test_file "PGCR.txt"

# create temp file
for ((idx=0;idx<$CYCLE;++idx));
do

    write_to_fs $1 "PGCR.txt"
    ../main --type $1 --list /

    get_from_fs $1 "PGCR.txt"

    diff "./get/PGCR.txt" "./temp/PGCR.txt"

    if [ $? -ne 0 ]; then
        echo "please check ./get/ and ./temp/ \"PGCR.txt\" file."
        echo "[TEST SCRIPT] FILE $idx FAILED."
        break
    fi

    rm_fs_file $1 "PGCR.txt"
    ../main --type $1 --list /

done

