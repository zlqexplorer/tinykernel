if [[ ! -d "../lib" || ! -d "../build" ]];then
    echo "dependent dir don\'t exist!"
    cwd=$(pwd)
    cwd=${cwd##*/}
    cwd=${cwd%/}
    if [[ $cwd != "command" ]];then
        echo -e "you\'d better in command dir\n"
    fi
    exit
fi

BIN="morecat"
CFLAGS="-Wall -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes -Wsystem-headers -m32"
LIB="-I ../lib -I ../lib/user -I ../lib/kernel -I ../fs -I ../device -I ../kernel -I ../thread"
OBJS="../build/stdio.o ./string.o ../build/syscall.o start.o"

DD_IN=$BIN
DD_OUT="/home/zc/bochs/hd60M.img"

nasm -f elf ./start.S -o ./start.o
ar rcs simple_crt.a $OBJS
gcc $CFLAGS $LIB -o $BIN".o" $BIN".c"
ld -m elf_i386 $BIN".o" simple_crt.a -o $BIN
SEC_CNT=$(ls -l $BIN|awk '{printf("%d",($5+511)/512)}')

if [[ -f $BIN ]];then
    dd if=./$DD_IN of=$DD_OUT bs=512 count=$SEC_CNT seek=300 conv=notrunc
fi
