#!/bin/sh
# 守护进程
# while test "1" = "1"
# do
#     sleep 1
#     value=$(ps aux | grep TABA.BIN | grep -v grep | wc -l)
#     if [ $value -ne 1 ]
#     then
#         killall TABA.BIN
#         /app/app/TABA.BIN leo &
#     fi
# done



while test "1" = "1"
do
    sleep 1
    value=$(ps aux | grep TABA.BIN | grep -v grep | wc -l)
    if [ $value -eq 0 ]
    then
        killall TABA.BIN
        /app/app/TABA.BIN leo &
        sleep 2
    fi
done


