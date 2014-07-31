#!/bin/sh

function detect() {
	device=$1 #assumes it is a valid CD / DVD device
	readcd dev=$device -fulltoc 2>/dev/null; rm -f  ~/toc.dat; rm -f toc.dat #wait for loading
	if cdrom_id $device | grep -q "ID_CDROM_MEDIA"; then
		trackscount=`cdrom_id $device | grep "ID_CDROM_MEDIA_TRACK_COUNT=" | cut -f2 -d'='`
		if cdrom_id $device | grep -q -E "ID_CDROM_MEDIA_CD_RW|ID_CDROM_MEDIA_DVD.*RW"; then
			rewritablemedia=1
		fi
		if cdrom_id $device | grep -q "ID_CDROM_MEDIA_STATE=blank"; then
			mediacontent="blank"
		else
			mediasize=`isosize $device 2>/dev/null`
			mediacontent=`blkid -o udev $device | grep "ID_FS_TYPE=" | cut -f2 -d'='`
			medialabel=`blkid -o udev $device | grep "ID_FS_LABEL=" | cut -f2 -d'='`
		fi
		if cdrom_id $device | grep -q ID_CDROM_MEDIA_DVD; then #DVD
			mediatype="dvd"
			if cdrom_id $device | grep -q "ID_CDROM_MEDIA_DVD.*R"; then
				mediacapacity=`cdrecord -atip dev=$device 2>&1 | grep "rzone size:" | tail -1 | sed 's/rzone size: \+//'`
				if [ -z "$mediacapacity" ]; then
					mediacapacity=`cdrecord -atip dev=$device 2>&1 | grep "phys size:..." | tail -1 | sed 's/phys size:... \+//'`
				fi
				let mediacapacity=mediacapacity*2048
			fi
			{ mplayer -dvd-device $device dvd://1 -identify -vo null -ao null -frames 0 2>&1 > /tmp/simpleburn-detect.$$ ;} 2>&1 >/dev/null
			if grep -q "ID_DVD_TITLES" /tmp/simpleburn-detect.$$; then
				mediacontent="video"
				trackscount=0
				for title in `cat  /tmp/simpleburn-detect.$$ | grep "TITLE_[0-9]\+_LENGTH"`; do #for each title during more than 3'
					titlenum=`echo $title | cut -d'_' -f4`
					titlelenght=`echo $title | cut -d'=' -f2 | cut -f1 -d'.'`
					let minutes=titlelenght/60
					if (( minutes > 3 )); then
						if (( $titlenum != 1 )); then
							{ mplayer -dvd-device $device dvd://$titlenum -identify -vo null -ao null -frames 0 2>&1 > /tmp/simpleburn-detect.$$; } 2>&1 >/dev/null
						fi
						if grep -q "ID_AID" /tmp/simpleburn-detect.$$ && grep -q "ID_SID" /tmp/simpleburn-detect.$$; then
							let trackscount=trackscount+1
							if [ ! -z "$mediainfos" ]; then
								mediainfos="$mediainfos\n"
								detailedinfos="$detailedinfos\n"
							fi
							mediainfos="$mediainfos$titlenum;$titlelenght"
							detailedinfos="$detailedinfos""title: $titlenum ($minutes')"
							for id in ID_AID ID_SID; do #audio & subtitles
								mediasubinfos=""
								subdetailedinfos=""
								for language in `cat /tmp/simpleburn-detect.$$ | grep "$id"`; do
									if [ ! -z "$mediasubinfos" ]; then
										mediasubinfos="$mediasubinfos,"
										subdetailedinfos="$subdetailedinfos, "
									fi
									languageid=`echo $language | cut -d'_' -f3`
									languagename=`echo $language | cut -d'=' -f2 | cut -f1 -d'.'`
									mediasubinfos="$mediasubinfos$languageid/$languagename"
									subdetailedinfos="$subdetailedinfos $languagename($languageid)"
								done
								mediainfos="$mediainfos;$mediasubinfos"
								if [ "$id" == "ID_AID" ]
								then detailedinfos="$detailedinfos\n\tlanguages: $subdetailedinfos"
								else detailedinfos="$detailedinfos\n\tsubtitles: $subdetailedinfos"
								fi
							done
							cropinfos=`mplayer -dvd-device $device dvd://$titlenum -ao null -ss 15 -frames 120 -vf cropdetect -vo null 2>&1 | grep "crop=720:" | tail -n1 | cut -f2 -d'=' | cut -f1 -d')'`
							mediainfos="$mediainfos;$cropinfos"
							detailedinfos="$detailedinfos""title: $titlenum ($minutes') - crop=$cropinfos"
						fi
					fi
				done
			fi
			rm -f /tmp/simpleburn-detect.$$
		else #CD
			if cdrom_id $device | grep -q ID_CDROM_MEDIA_CD; then
				mediatype="cd"
				if cdrom_id $device | grep -q "ID_CDROM_MEDIA_CD_R"; then
				mediacapacity=`cdrecord -atip dev=$device 2>&1 | grep "ATIP start of lead out:" | sed 's/.*: \([0-9]\+\) .*/\1/'` 
					let mediacapacity=mediacapacity*2048
				fi
				if cdrom_id $device | grep -q "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO"; then
					mediacontent="audio"
					mediasize=`cdrecord -toc dev=$device 2>&1 | grep "track:lout" | sed 's/track:lout lba: \+\([0-9]\+\) .*/\1/'`
					let mediasize=mediasize*2048
					cdda2wav -J -L1 -v titles,toc -g -N -H dev=$device out-fd=1 2>/dev/null | tr -d '\200-\377'> /tmp/simpleburn-detect.$$ 
					medialabel=`cat /tmp/simpleburn-detect.$$ | grep "^Album title:" | sed 's/^Album title: .\(.*\). from .*$/\1/'`
					n=`cat /tmp/simpleburn-detect.$$ | grep "^T..:" | wc -l`
					for (( i=1; i<=$n; i++ )); do
						line=`cat /tmp/simpleburn-detect.$$ | grep "^T..:" | sed -n $i\p`
						if [ ! -z "$mediainfos" ]; then
							mediainfos="$mediainfos\n"
							detailedinfos="$detailedinfos\n"
						fi
						tracknum=`echo $line | sed -e 's/^T0\?\([0-9]*\):.*/\1/'`
						tracktitle=`echo $line | grep "^T..:" | sed -e 's/.* title .\(.*\). from .*/\1/'`
						tracklength=`echo $line | sed 's/T..: \(.*\) title.*/\1/' | cut -f1 -d .`
						detailedinfos="$detailedinfos""track $tracknum ($tracklength): $tracktitle"
						mediainfos="$mediainfos$tracknum;$tracktitle;$tracklength"
					done
					rm -f /tmp/simpleburn-detect.$$
				fi
			else
				mediatype="unknown"
			fi
		fi
	fi
	
	let mediasize_=mediasize/1048576
	let mediacapacity_=mediacapacity/1048576
	if [ $rewritablemedia -eq 1 ]
	then rewritablemedia_="yes"
	else rewritablemedia_="no"
	fi
	detailedinfos="type: $mediatype\ncontent: $mediacontent\ncapacity: $mediacapacity_ MB\nsize: $mediasize_ MB\nrewritable: $rewritablemedia_\ntrackscount: $trackscount\nlabel: $medialabel\n$detailedinfos"
}


#tools detection
PATH=$PATH:/lib/udev:/usr/lib/udev:/sbin:/usr/sbin
for tool in cdrom_id blkid isosize cdrecord cdda2wav mplayer; do
	if ! which $tool >/dev/null 2>&1; then echo "error: '$tool' is missing"; exit 2; fi
done

#common parameters
if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
	echo "usage: $0 [CD / DVD device]"
	echo "example: $0 /dev/sr0"
	exit
fi
if [ "$1" == "-cr" ]; then opt="-cr"; shift; fi
if [ -b "$1" ]
then device=$1; shift
else device=/dev/cdrom
fi

#global vars initialization
mediatype="none" #none, cd, dvd
mediacontent="empty" #empty, blank, audio, video, iso9660, udf
mediacapacity=0 #bytes
mediasize=0
rewritablemedia=0
trackscount=0
medialabel=""
mediainfos="" #computer readable (-cr)
detailedinfos="" #human readable

#display infos if asked for
if [ `basename "'$0"` == "simpleburn-detect.sh" ]; then
	detect $device
	if [ "$opt" == "-cr" ]
	then echo "$mediatype:$mediacontent:$mediacapacity:$mediasize:$rewritablemedia:$trackscount:$medialabel"
		echo -e $mediainfos
		echo ""
	fi
	echo -e $detailedinfos
fi
