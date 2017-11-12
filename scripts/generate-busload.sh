#!/bin/bash

killall -9 cangen candump canplayer  2> /dev/null

DURACAO=10000

##########
# ## 1% 
# for CYCLE in 10 ; do ( 
#    cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
# ); done

## 62% busload
# for CYCLE in {10..999..1} ; do ( 
#    cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
# ); done

##########
# ## 11% 
# for CYCLE in 1 ; do ( 
#    cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
# ); done

##########
# ## 86% busload
# for CYCLE in {2..999..1} ; do ( 
#    cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
# ); done

##########
## 99% busload
for CYCLE in {1..999..1} ; do ( 
   cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
); done

sleep 3 && (candump -t z vcan0 -l &)
#sleep 3 && (candump -t z vcan0 > candump.log &)

sleep 7 && killall cangen && pkill candump

mv candump-*.log candump.log

wc -l candump*.log
ls -sh candump*.log

killall -9 cangen candump canplayer  2> /dev/null






## 81% busload
# for CYCLE in {10..37..1} ; do ( 
#    for REPLICA in {1..2}; do (
#       cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
#    ); done
# ); done
# for CYCLE in {38..999..1} ; do ( 
#    cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
# ); done


## 99% busload
# for CYCLE in {10..37..1} ; do ( 
#    for REPLICA in {1..3}; do (
#       cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
#    ); done
# ); done
# for CYCLE in {38..999..1} ; do ( 
#    cangen -g $CYCLE -n $[DURACAO / CYCLE] -L 8 -D i vcan0 &
# ); done