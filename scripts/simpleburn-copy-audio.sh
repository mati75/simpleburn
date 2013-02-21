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

tempdir="/tmp/simpleburn$$"

mkdir $tempdir
cd $tempdir

#~ if echo $CDDA2WAV | grep -q "cd-paranoia" || echo $CDDA2WAV | grep -q "cdparanoia"
if echo $CDRDAO | grep -q "cdrdao"
then #cdrskin can't do audio CD copying yet, using cdrdao instead
	#~ trackslengths==`$CDDA2WAV -d $cdreader -Q  2>&1 | sed -n '/^ \+[0-9]\+\./p' | sed 's/ \+/ /g' | cut -f3 -d' '`
	#~ $CDDA2WAV -p -Z -q -B -d $cdreader 1- - | cdrskin -v gracetime=3 dev=$cdwriter -tao -audio - >/dev/null #| simpleburn-gauges cdrecord $trackslengths
	cdrdao copy --fast-toc --on-the-fly --source-device $cdreader --device $cdwriter 2>&1 | simpleburn-gauges cdrdao
	status=${PIPESTATUS[0]}
else
	$CDDA2WAV dev=$cdreader -vtoc -L1 -info-only
	trackslengths=""
	for infofile in *.inf; do 
		tracklength=`sed -n /Tracklength/p $infofile | sed 's/\t//' | cut -f2 -d= | cut -f1 -d,`
		let tracklength=tracklength/512
		trackslengths="$trackslengths$tracklength "
	done

	$CDDA2WAV dev=$cdreader -no-infofile -B -Oraw - 2>/dev/null | $CDRECORD -v gracetime=3 dev=$cdwriter -dao -audio -useinfo -text *.inf | simpleburn-gauges cdrecord $trackslengths
	status=${PIPESTATUS[0]}
fi

cd - >/dev/null
rm -rf $tempdir

echo "100"
exit $status
