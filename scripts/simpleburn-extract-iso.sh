#!/bin/bash

function ddgauge() {
	total=$1
	shift; filename=$*
	ppid=$$
	
	sleep 1
	#~ lastpercent=0
	while pgrep -P $ppid dd >/dev/null; do
		filesize=`ls -l "$filename" | cut -f5 -d' '`
		let filesize=filesize/1048576
		let percent=(filesize*100)/total
		let percent=percent%100
		#~ if (( $percent != $lastpercent )); then
			echo $percent
			#~ lastpercent=$percent
		#~ fi
		sleep 1
	done
}


cdreader=$1
filename=`echo $2 | sed 's/"//g'`
directory=`dirname "$filename"`
mkdir -p "$directory"

if [ ! -b "$cdreader" ]; then
	echo "usage: $0 cdreader somefile.iso"
	echo "example: $0 /dev/sr0 /home/me/linux.iso"
	exit
fi

tracksize=`simpleburn-media-detection $cdreader | grep "mediasize" | cut -f2 -d=`
let blockscount=tracksize/2
let tracksize=tracksize/1024

rm -f "$filename"
dd if=$cdreader bs=2048 count=$blockscount of="$filename" | ddgauge $tracksize $filename
status=${PIPESTATUS[0]}

echo "100"
exit $status