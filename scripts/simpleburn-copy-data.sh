#!/bin/bash

cdreader=$1
cdwriter=$2

if [ ! -b "$cdwriter" ] || [ ! -b "$cdreader" ] || [ $cdwriter == $cdreader ]; then
	echo "usage: $0 cdreader cdwriter"
	echo "example: $0 /dev/sr0 /dev/sr1"
	exit
fi


if mount | grep -q "^$cdwriter "; then
	umount $cdwriter || exit 255
fi

source simpleburn-burning-suite quiet

tracksize=`simpleburn-media-detection $cdreader | grep "mediasize" | cut -f2 -d=`
let tracksize=tracksize/1024
$CDRECORD -v gracetime=3 dev=$cdwriter -isosize $cdreader | simpleburn-gauges cdrecord $tracksize
status=${PIPESTATUS[0]}

echo "100"
exit $status
