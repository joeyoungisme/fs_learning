#! /bin/bash

if [ -z "$1" ] || [ "$1" != "fat" ] && [ "$1" != "lfs" ]; then
    echo "please give me type ..."
    exit
fi

CYCLE=5
RING=10

gen_test_file() {

    FILE_SIZE=$(expr 256 \* 1024)

    mkdir -p ./temp

    rm -f ./temp/$1

    dd if=/dev/urandom of=./temp/$1 bs=1 count=$FILE_SIZE
    
}
write_to_fs() {

    ../main --type $1 --file $2 --put ./temp/$2
}
get_from_fs() {

    mkdir -p get
    ../main --type $1 --file $2 --get ./get/$2
}

rm_fs_file() {
    ../main --type $1 --file $2 --rm
}

BASE_NAME="RRPGC"

# create temp file

for ((cyc=0;cyc<$CYCLE;++cyc));
do

    for ((idx=0;idx<$RING;++idx));
    do

        FILE_NAME="$BASE_NAME""$idx"".txt"

        TITLE="[CYCLE $cyc] [RING $FILE_NAME]"

        echo $TITLE Rm File.
        rm_fs_file $1 $FILE_NAME

        echo $TITLE Gen File.
        gen_test_file $FILE_NAME

        echo $TITLE Put File.
        write_to_fs $1 $FILE_NAME

        echo $TITLE Get File.
        get_from_fs $1 $FILE_NAME

        echo $TITLE Check File.
        diff "./get/$FILE_NAME" "./temp/$FILE_NAME"

        if [ $? -ne 0 ]; then
            echo "please check ./get/ and ./temp/ \"$FILE_NAME\" file."
            echo "[TEST SCRIPT] FILE $FILE_NAME FAILED."
            exit
        fi

    done

done

