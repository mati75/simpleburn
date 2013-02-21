#!/bin/bash

cdwriter=$1
directory=`echo $2 | sed 's/"//g'`

if [ ! -d "$directory" ] || [ ! -b "$cdwriter" ]; then
	echo "usage: $0 cdwriter directory"
	echo "example: $0 /dev/sr0 /home/me/data"
	exit
fi


if mount | grep -q "^$cdwriter "; then
	umount $cdwriter || exit 255
fi

source simpleburn-burning-suite quiet

label=`basename "$directory"`
if echo $MKISOFS | grep -q "xorriso"
then umount $cdwriter
	xorriso -dev $cdwriter -joliet on -volid "$label" -map "$directory" / -pkt_output on | simpleburn-gauges xorriso
	status=${PIPESTATUS[0]}
else tsize=`$MKISOFS -J -r -N -d -hide-rr-moved -print-size "$directory"`
	let tracksize=tsize/512 #blocks => MB
	$MKISOFS -J -r -N -d -hide-rr-moved -V "$label" "$directory" | $CDRECORD -v gracetime=3 dev=$cdwriter driveropts=burnfree tsize=$tsize\s -data -pad - | simpleburn-gauges cdrecord $tracksize
	status=${PIPESTATUS[0]}&&${PIPESTATUS[1]}
fi

echo "100"
exit $status