#!/bin/bash

export PORTA=0
export TEMPO_SLEEP=3
export BACKLOG=10

echo
./server_http $PORTA $BACKLOG $TEMPO_SLEEP &

seq 10 | xargs -P 10 -I {} bash -c "./client_http localhost $PORTA > /dev/null &"

# for i in {1..10}; do
#     # echo "Connecting client $i"
#     ./client_http localhost $PORTA > /dev/null &
#     sleep 0.05
# done

# sleep 5
killall server_http
