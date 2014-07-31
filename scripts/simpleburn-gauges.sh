#!/bin/sh

function mencodergauge () {
	totalsize=$1 #bytes
	pass=$2
	lockfile=$3
	destination=$4
	
	if (( $pass == 0 ))
	then cumul=0
		passpercent=99 #1% for container
	else passpercent=49
		if (( $pass == 1 ))
		then cumul=0
		else cumul=50
		fi
	fi
	echo $cumul	
	while [ -f $lockfile ]; do
		currentsize=`ls -l "$destination" | cut -f5 -d' '`
		let percent=(currentsize*passpercent/totalsize)+cumul
		echo $percent
		sleep 1
	done
}


function ddgauge() {
	totalsize=$1 #bytes
	lockfile=$2
	shift; shift; destination=$*
	
	while [ -f $lockfile ]; do
		filesize=`ls -l "$destination" | cut -f5 -d' '`
		let filesize=filesize/1024
		let percent=(filesize*100)/totalsize
		let percent=percent%100
		echo $percent
		sleep 1
	done
}


case $1 in
	"mencoder") shift
		mencodergauge $*
		;;
	"dd") shift
		ddgauge $*
		;;
	*) echo "usage: intended for SimpleBurn scripts"
	exit 1
esac
