#!/bin/bash

blanktype=$1
cdwriter=$2

if [ ! -b "$cdwriter" ]; then
	echo "usage: $0 all|fast cdwriter"
	echo "example: $0 fast /dev/sr0"
	exit
fi


if mount | grep -q "^$cdwriter "; then
	umount $cdwriter || exit 255
fi

source simpleburn-burning-suite quiet

$CDRECORD gracetime=3 blank=$blanktype dev=$cdwriter
status=$?

exit $status
