#!/bin/bash

cdwriter=$1
directory=`echo $2 | sed 's/"//g'`

if [ ! -d "$directory" ] || [ ! -b "$cdwriter" ]; then
	echo "usage: $0 cdwriter directory"
	echo "example: $0 /dev/sr0 /home/me/audio"
	exit
fi


if mount | grep -q "^$cdwriter "; then
	umount $cdwriter || exit 255
fi

source simpleburn-burning-suite quiet

#FLAC, MP3 and OGG conversion
if which flac >/dev/null 2>&1; then
	ls "$directory"/*.flac | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then
			#~ flac --decode "$audiofile" "$audiofile.wav"
			flac --decode --channels=2 --sample-rate=44100 "$audiofile" "$audiofile.wav"
		fi
	done
fi

if which mpg123 >/dev/null 2>&1; then
	ls "$directory"/*.mp3 | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then
			#~ mpg123 --stereo --wav "$audiofile.wav" "$audiofile"
			mpg123 --stereo --rate 44100 --wav "$audiofile.wav" "$audiofile"
		fi
	done
fi

if which oggdec >/dev/null 2>&1; then
	ls "$directory"/*.ogg | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then
			oggdec -o "$audiofile.wav" "$audiofile"
		fi
	done
fi

NORMALIZE=`which normalize 2>/dev/null || which normalize-audio 2>/dev/null`
if [ ! -z  "$NORMALIZE" ]; then
	$NORMALIZE "$directory"/*.wav
fi

audiosizes=""
totalsize=0
for audiosize in `ls -l "$directory"/*.wav | cut -f5 -d' '`; do
	let audiosize=audiosize/1048576
	audiosizes="$audiosizes$audiosize "
	let totalsize=totalsize+audiosize
done
let totalsize=totalsize*2048/2352

#recheck if  CD media has enough space (new WAV files can be added by OGG/MP3 conversion)
mediacapacity=`simpleburn-media-detection $cdwriter | grep  "mediacapacity=" | cut -f2 -d=`
let mediacapacity=mediacapacity/1024

if (( $totalsize <= $mediacapacity ))
then cd "$directory"
	#notice: options -useinfo -text are ignored by cdrskin
	$CDRECORD -v speed=0 gracetime=3 dev=$cdwriter -dao -audio -pad -useinfo -text "$directory"/*.wav | simpleburn-gauges cdrecord $audiosizes
	status=${PIPESTATUS[0]}
	cd - >/dev/null
fi

echo "100"
exit $status
