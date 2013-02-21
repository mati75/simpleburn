#!/bin/bash

cdreader=$1
directory=`echo $2 | sed 's/"//g'`
audioformat=$3
tracksrange=$4

if [ ! -d "$directory" ] || [ ! -b "$cdreader" ] || [ -z "$audioformat" ]; then
	echo "usage: $0 cdreader directory wav|flac|ogg|mp3 [tracksrange]"
	echo "example: $0 /dev/sr0 /home/me/audio wav 1-10,15,17-20"
	exit
fi


source simpleburn-burning-suite quiet


FIFOFILE="/tmp/simpleburn-extract-audio$$"


if [ -z "$tracksrange" ] #define / expand tracks to extract
then lasttrack=`simpleburn-media-detection $cdreader | grep  "trackscount=" | cut -f2 -d=`
	for ((i=1; i<=lasttrack; i++)); do
		tracks="$tracks $i"
	done
else tracks=""
	while [ ! -z "$tracksrange" ]; do
		expd=`echo $tracksrange | cut -f1 -d,`
		if echo $expd | grep -q "-"; then
			firsttrack=`echo $expd | cut -f1 -d-`
			lasttrack=`echo $expd | cut -f2 -d-`
			expd=""
			for ((i=firsttrack; i<=lasttrack; i++)); do
				expd="$expd $i"
			done
			tracks="$tracks$expd"
		else tracks="$tracks $expd"
		fi
		
		if echo $tracksrange | grep -q ","
		then tracksrange=`echo $tracksrange | cut -f2- -d,`
		else tracksrange=""
		fi
	done
fi

cd "$directory"

if echo $CDDA2WAV | grep -q "cd-paranoia" || echo $CDDA2WAV | grep -q "cdparanoia"
then total=0 #retrieves tracks lengths
	i=1
	for info in `$CDDA2WAV -Q -d $cdreader 2>&1 | grep "\. " | sed 's/ \+/:/g' | cut -f3,6 -d:`; do 
		if echo " $tracks " | grep -q " $i "; then
			length=`echo $info | cut -f1 -d:`
			offset=`echo $info | cut -f2 -d:`
			lengths[$i]=$length
			offsets[$i]=$offset
			let total=total+length
		fi
		let i=i+1
	done
	
	mkfifo $FIFOFILE
	let totaldone=0
	for track in $tracks; do #extract each track
		let previouspercent=totaldone*100/total
		let trackpercent=lengths[$track]*100/total
		if (( $track < 10 ))
		then filenum="0$track"
		else filenum="$track"
		fi
		case $audioformat in
		"wav")  $CDDA2WAV -e -Z -d $cdreader $track track_$filenum.wav 2>$FIFOFILE | \
					simpleburn-gauges cdparanoia $previouspercent $trackpercent ${lengths[$track]} ${offsets[$track]} $FIFOFILE
			;;
		"flac") $CDDA2WAV -e -Z -d $cdreader $track - 2>$FIFOFILE | flac --totally-silent -f -o track_$filenum.flac - | \
					simpleburn-gauges cdparanoia $previouspercent $trackpercent ${lengths[$track]} ${offsets[$track]} $FIFOFILE
			;;
		"ogg") $CDDA2WAV -e -Z -d $cdreader $track - 2>$FIFOFILE | oggenc -b 256 -Q -o track_$filenum.ogg - | \
					simpleburn-gauges cdparanoia $previouspercent $trackpercent ${lengths[$track]} ${offsets[$track]} $FIFOFILE
			;;
		"mp3") $CDDA2WAV -e -Z -d $cdreader $track - 2>$FIFOFILE | lame -b 256 --quiet - track_$filenum.mp3 | \
					simpleburn-gauges cdparanoia $previouspercent $trackpercent ${lengths[$track]} ${offsets[$track]} $FIFOFILE
			;;
		esac
		let totaldone=totaldone+lengths[$track]
	done
	rm -f $FIFOFILE
	
else total=0 #retrieves tracks lengths
	for trackinfo in `$CDDA2WAV -N -J -v toc dev=$cdreader 2>&1 | grep ".(" | tr "," "\n" | sed /^$/d | sed 's/ //g'`; do 
		i=`echo $trackinfo | cut -f1 -d.`
		if echo " $tracks " | grep -q " $i "; then
			ms=`echo $trackinfo | cut -f2 -d'(' | cut -f1 -d.`
			minutes=`echo $ms | cut -f1 -d:`
			seconds=`echo $ms | cut -f2 -d: | sed 's/^0//'`
			let seconds=(minutes*60)+seconds
			lengths[$i]=$seconds
			let total=total+seconds
		fi
	done
	
	mkfifo $FIFOFILE
	let totaldone=0
	for track in $tracks; do #extract each track
		let previouspercent=totaldone*100/total
		let trackpercent=lengths[$track]*100/total
		if (( $track < 10 ))
		then filenum="0$track"
		else filenum="$track"
		fi
		case $audioformat in
		"wav")  $CDDA2WAV dev=$cdreader -t $track track_$filenum.wav 2>$FIFOFILE | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			;;
		"flac") $CDDA2WAV dev=$cdreader -t $track - 2>$FIFOFILE | flac --totally-silent -f -o track_$filenum.flac - | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			;;
		"ogg") $CDDA2WAV dev=$cdreader -t $track - 2>$FIFOFILE | oggenc -b 256 -Q -o track_$filenum.ogg - | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			;;
		"mp3") $CDDA2WAV dev=$cdreader -t $track - 2>$FIFOFILE | lame -b 256 --quiet - track_$filenum.mp3 | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			;;
		esac
		let totaldone=totaldone+lengths[$track]
	done
	rm -f $FIFOFILE
fi

#renaming
if ! echo $CDDA2WAV | grep -q "cd-paranoia" && ! echo $CDDA2WAV | grep -q "cdparanoia"
then $CDDA2WAV -L1 -J -v toc dev=$cdreader track
	ls *.inf | while read trackinfofile; do 
		tracknumber=` echo $trackinfofile | sed 's/^track_\([0-9]*\).inf$/\1/'`
		albumtitle=`cat $trackinfofile | grep "Albumtitle" | sed 's/.*=\t//' | sed s@/@-@g`
		tracktitle=`cat $trackinfofile | grep "Tracktitle" | sed 's/.*=\t//' | sed s@/@-@g`
		if [ -f track_$tracknumber.$audioformat ]
		then mv track_$tracknumber.$audioformat "$albumtitle"-$tracknumber-"$tracktitle".$audioformat
			mv track_$tracknumber.inf "$albumtitle"-$tracknumber-"$tracktitle".inf
		else rm -f $trackinfofile
		fi
	done
	rm -f track.cdindex track.cddb
fi

cd - >/dev/null

echo "100"
exit $status
