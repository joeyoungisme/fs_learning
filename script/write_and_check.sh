#! /bin/bash

if [ -z "$1" ] || [ "$1" != "fat" ] && [ "$1" != "lfs" ]; then
    echo "please give me type ..."
    exit
fi

FILE_AMOUNT=10

gen_test_file() {
    FILE_SIZE=$(expr 512 \* 1024)
    echo "FILE SIZE $FILE_SIZE"

    mkdir -p ./temp

    echo "[TEST SCRIPT] Gen Test File WRC$1.txt"
    rm -f ./temp/WRC$1.txt

    dd if=/dev/urandom of=./temp/WRC$1.txt bs=1 count=$FILE_SIZE
    
#   for ((wline=0;wline<=$FILE_LINE_AMOUNT;wline++));
#   do
#       w=$(expr $1 + $wline)
#       echo "DATA[$w$w$w$w$w$w$w$w$w$w$w$w$w$w$w$w]" >> ./temp/WRC$1.txt
#   done

}
write_to_fs() {

    file_name="WRC$2.txt"
    echo "[TEST SCRIPT] [WRITE] [$file_name]"

    ../main --type $1 --file $file_name --put ./temp/$file_name

#   while read line;
#   do
#       echo "[TEST SCRIPT] [WRITE] $line"
#       ../main --type $1 --file $file_name --w "$line"
#   done < "./temp/$file_name"
}
get_from_fs() {

    mkdir -p get
    file_name="WRC$2.txt"
    echo "[TEST SCRIPT] [GET] [$file_name]"
    ../main --type $1 --file $file_name --get ./get/$file_name
}
check_file() {

    file_name="WRC$1.txt"
    get_file_name="./get/$file_name"
    cmp_file_name="./temp/$file_name"
    diff $get_file_name $cmp_file_name
    if [ $? -ne 0 ]; then
        echo "FAILED"
    else
        echo "PASS"
    fi
}

rm $10.config
# create temp file
for ((idx=0;idx<$FILE_AMOUNT;++idx));
do

    gen_test_file $idx
    write_to_fs $1 $idx
    get_from_fs $1 $idx
    RESULT=$(check_file $idx)
    if [ "$RESULT" != "PASS" ]; then
        echo "please check ./get/ and ./temp/ \"WRC$idx.txt\" file."
        echo "[TEST SCRIPT] FILE $idx FAILED."
        break
    else
        echo "[TEST SCRIPT] FILE $idx PASS."
    fi

done

