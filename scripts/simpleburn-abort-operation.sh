#!/bin/bash

pid=$1

if [ -z $pid ]; then
	echo "usage: $0 simpleburn-operation-pid"
	echo "example: $0 12345"
	exit
fi


pslist=""
function listps () {
	pid=$1
	
	pslist="$pid $pslist"
	for ppid in `pgrep -P $pid`; do
		listps $ppid
	done
}
listps $pid

echo "kill $pslist"
kill $pslist
