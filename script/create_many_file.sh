#! /bin/zsh

if [ -z "$1" ] || [ "$1" != "fat" ] && [ "$1" != "lfs" ]; then
    echo "please give me type ..."
    exit
fi

create_1() {
    WRITE_COUNT=3000
    # SYNC_TIMES=$(($WRITE_COUNT / 3))
    SYNC_TIMES=$(($WRITE_COUNT / 3))
    ../main --type $1 --file  WW00.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW00.txt : 0000000000000000000000000000000000000000000000000000000000000000000000 '
    ../main --type $1 --file  WW01.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW01.txt : 0101010101010101010101010101010101010101010101010101010101010101010101 '
    ../main --type $1 --file  WW02.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW02.txt : 0202020202020202020202020202020202020202020202020202020202020202020202 '
    ../main --type $1 --file  WW03.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW03.txt : 0303030303030303030303030303030303030303030303030303030303030303030303 '
    ../main --type $1 --file  WW04.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW04.txt : 0404040404040404040404040404040404040404040404040404040404040404040404 '
    ../main --type $1 --file  WW05.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW05.txt : 0505050505050505050505050505050505050505050505050505050505050505050505 '
    ../main --type $1 --file  WW06.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW06.txt : 0606060606060606060606060606060606060606060606060606060606060606060606 '
    ../main --type $1 --file  WW07.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW07.txt : 0707070707070707070707070707070707070707070707070707070707070707070707 '
    ../main --type $1 --file  WW08.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW08.txt : 0808080808080808080808080808080808080808080808080808080808080808080808 '
    ../main --type $1 --file  WW09.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW09.txt : 0909090909090909090909090909090909090909090909090909090909090909090909 '
    ../main --type $1 --file  WW10.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW10.txt : 1010101010101010101010101010101010101010101010101010101010101010101010 '
    ../main --type $1 --file  WW11.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW11.txt : 1111111111111111111111111111111111111111111111111111111111111111111111 '
    ../main --type $1 --file  WW12.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW12.txt : 1212121212121212121212121212121212121212121212121212121212121212121212 '
    ../main --type $1 --file  WW13.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW13.txt : 1313131313131313131313131313131313131313131313131313131313131313131313 '
    ../main --type $1 --file  WW14.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW14.txt : 1414141414141414141414141414141414141414141414141414141414141414141414 '
    ../main --type $1 --file  WW15.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW15.txt : 1515151515151515151515151515151515151515151515151515151515151515151515 '
    ../main --type $1 --file  WW16.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW16.txt : 1616161616161616161616161616161616161616161616161616161616161616161616 '
    ../main --type $1 --file  WW17.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW17.txt : 1717171717171717171717171717171717171717171717171717171717171717171717 '
    ../main --type $1 --file  WW18.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW18.txt : 1818181818181818181818181818181818181818181818181818181818181818181818 '
    ../main --type $1 --file  WW19.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW19.txt : 1919191919191919191919191919191919191919191919191919191919191919191919 '
    ../main --type $1 --file  WW20.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW20.txt : 2020202020202020202020202020202020202020202020202020202020202020202020 '
    ../main --type $1 --file  WW21.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW21.txt : 2121212121212121212121212121212121212121212121212121212121212121212121 '
    ../main --type $1 --file  WW22.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW22.txt : 2222222222222222222222222222222222222222222222222222222222222222222222 '
    ../main --type $1 --file  WW23.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW23.txt : 2323232323232323232323232323232323232323232323232323232323232323232323 '
    ../main --type $1 --file  WW24.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW24.txt : 2424242424242424242424242424242424242424242424242424242424242424242424 '
    ../main --type $1 --file  WW25.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW25.txt : 2525252525252525252525252525252525252525252525252525252525252525252525 '
    ../main --type $1 --file  WW26.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW26.txt : 2626262626262626262626262626262626262626262626262626262626262626262626 '
    ../main --type $1 --file  WW27.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW27.txt : 2727272727272727272727272727272727272727272727272727272727272727272727 '
    ../main --type $1 --file  WW28.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW28.txt : 2828282828282828282828282828282828282828282828282828282828282828282828 '
    ../main --type $1 --file  WW29.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW29.txt : 2929292929292929292929292929292929292929292929292929292929292929292929 '
    ../main --type $1 --file  WW30.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW30.txt : 3030303030303030303030303030303030303030303030303030303030303030303030 '
    ../main --type $1 --file  WW31.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW31.txt : 3131313131313131313131313131313131313131313131313131313131313131313131 '
    ../main --type $1 --file  WW32.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW32.txt : 3232323232323232323232323232323232323232323232323232323232323232323232 '
    ../main --type $1 --file  WW33.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW33.txt : 3333333333333333333333333333333333333333333333333333333333333333333333 '
    ../main --type $1 --file  WW34.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW34.txt : 3434343434343434343434343434343434343434343434343434343434343434343434 '
    ../main --type $1 --file  WW35.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW35.txt : 3535353535353535353535353535353535353535353535353535353535353535353535 '
    ../main --type $1 --file  WW36.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW36.txt : 3636363636363636363636363636363636363636363636363636363636363636363636 '
    ../main --type $1 --file  WW37.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW37.txt : 3737373737373737373737373737373737373737373737373737373737373737373737 '
    ../main --type $1 --file  WW38.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW38.txt : 3838383838383838383838383838383838383838383838383838383838383838383838 '
    ../main --type $1 --file  WW39.txt --sync $SYNC_TIMES --wcnt $WRITE_COUNT --w 'WW39.txt : 3939393939393939393939393939393939393939393939393939393939393939393939 '
}

create_1 $1
../main --type $1 --list /
