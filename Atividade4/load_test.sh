#!/bin/bash

PORTA=0
TEMPO_SLEEP=5

for BACKLOG in {0..10}; do
    echo
    echo "Creating server with BACKLOG=$BACKLOG"
    ./server_http $PORTA $BACKLOG $TEMPO_SLEEP > /dev/null &

    for i in {1..10}; do
        echo "Connecting client $i"
        ./client_http localhost $PORTA > /dev/null &
    done
    
    sleep 3
done

killall server_http