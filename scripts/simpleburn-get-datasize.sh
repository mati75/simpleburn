#!/bin/bash

device=$1
if [ -z $device ]; then
	echo "usage: $0 device"
	echo "example: $0 /dev/sr0"
	exit
fi

if df | grep -q "$device "
then datasize=`df | grep "$device " | sed 's/ \+/ /g' | cut -f3 -d' '`
else if mount $device 2>/dev/null
	then datasize=`df | grep "$device " | sed 's/ \+/ /g' | cut -f3 -d' '`
		umount $device 2>/dev/null
	fi
fi
echo $datasize
