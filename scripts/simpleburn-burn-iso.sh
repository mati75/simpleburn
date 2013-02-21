#!/bin/bash

cdwriter=$1
filename=`echo $2 | sed 's/"//g'`

if [ ! -f "$filename" ] || [ ! -b "$cdwriter" ]; then
	echo "usage: $0 cdwriter somefile.iso"
	echo "example: $0 /dev/sr0 /home/me/linux.iso"
	exit
fi


if mount | grep -q "^$cdwriter "; then
	umount $cdwriter || exit 255
fi

source simpleburn-burning-suite quiet

tracksize=`ls -l "$filename" | cut -f5 -d' '`
let tracksize=tracksize/1048576
$CDRECORD -v gracetime=3 dev=$cdwriter -pad "$filename" | simpleburn-gauges cdrecord $tracksize
status=${PIPESTATUS[0]}

echo "100"
exit $status
