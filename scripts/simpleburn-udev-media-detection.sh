#!/bin/bash

device=$1
if [ -z $device ]; then
	echo "usage: $0 device"
	echo "example: $0 /dev/sr0"
	exit
fi


source simpleburn-burning-suite quiet

for loc in /lib/udev /usr/lib/udev /lib64/udev /usr/lib64/udev; do
	if [ -f $loc/cdrom_id ]; then
		CDROM_ID=$loc/cdrom_id
		break
	fi
done
if [ -z "$CDROM_ID" ]; then
	echo "missing udev/cdrom_id"
	exit
fi

if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA"
then 
	if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA_DVD"
	then mediatype="dvd"
	else if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA_CD"
		then mediatype="cd"
		else exit 1
		fi
	fi
	
	if df | grep -q "$device "
	then mediacontent=`mount | grep "$device " | cut -f5 -d' '`
		mediasize=`simpleburn-get-datasize $device`
	else if mount $device 2>/dev/null
		then mediacontent=`mount | grep "$device " | cut -f5 -d' '`
			mediasize=`simpleburn-get-datasize $device`
			umount $device 2>/dev/null
		else mediacontent="blank"
			mediasize=0
		fi
	fi
	
	if [ "$mediatype" == "cd" ]
	then
		if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA_CD_RW"
		then rewritablemedia=1
		else rewritablemedia=0
		fi
		
		if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO"
		then mediacontent="audio" #overrides "data" if it is an hybrid CD
			mediasize=`$CDRECORD -toc dev=$device 2>&1 | grep "track:lout" | sed 's/track:lout lba: \+\([0-9]\+\) .*/\1/'`
			let mediasize=mediasize*2
		fi
		
		if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA_CD_R"
		then mediacapacity=`$CDRECORD -atip dev=$device 2>&1 | grep "ATIP start of lead out:" | sed 's/.*: \([0-9]\+\) .*/\1/'` 
			let mediacapacity=mediacapacity*2
		else mediacapacity=0
		fi
	else #DVD
		if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA_DVD.*RW"
		then rewritablemedia=1
		else rewritablemedia=0
		fi
		
		if $CDROM_ID $device | grep -q "ID_CDROM_MEDIA_DVD.*R"
		then mediacapacity=`$CDRECORD -atip dev=$device 2>&1 | grep "rzone size:" | tail -1 | sed 's/rzone size: \+//'` #doesn't work with wodim or cdrskin => mediacapacity = 0 (unknown)
			let mediacapacity=mediacapacity*2
		else mediacapacity=0
		fi
	fi
	
	trackscount=`$CDROM_ID $device | grep "ID_CDROM_MEDIA_TRACK_COUNT=" | cut -f2 -d=`
	
	if [ "$2" == "oneline" ]
	then echo "$mediatype:$mediacontent:$mediacapacity:$mediasize:$rewritablemedia:$trackscount"
	else	echo "mediatype=$mediatype"
		echo "mediacontent=$mediacontent" #KiB
		echo "mediacapacity=$mediacapacity"
		echo "mediasize=$mediasize"
		echo "rewritablemedia=$rewritablemedia"
		echo "trackscount=$trackscount"
	fi
	exit 0
else	exit 1
fi
