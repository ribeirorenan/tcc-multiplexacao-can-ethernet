#!/bin/bash

if [ -z $1 ]; then
	echo "run.sh <DIR>"
	exit -1
else
	CODE_DIR="$@"
fi

cd ${CODE_DIR}

echo "compiling..."
make

echo "simulating..."
killall -9 canplayer
canplayer -I ../candump.log &

echo "running..."
./bridge-a.out > ./time-dif.txt || killall -9 canplayer

echo "done!"
wc -l ./time-dif.txt
