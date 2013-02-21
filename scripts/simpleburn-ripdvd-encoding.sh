#!/bin/bash

#90' =  690 MB
if mencoder -oac help | grep -q "mp3lame"
then	AUDIORATE=160
	HQAUDIORATE=256
	OAC_STRING="-oac mp3lame -lameopts mode=2:cbr:br"
else	AUDIORATE=224
	HQAUDIORATE=320
	OAC_STRING="-oac lavc -lavcopts acodec=mp2:abitrate"
fi

VIDEORATE=896
HQVIDEORATE=1856

INFOFILE="/tmp/ripdvd-detection.tmp.$$"

dvdDevice=$1
titleNum=$2
languageId=$3
subtitleId=$4
quality=$5
outputDirectory=$6

if [ ! -d $outputDirectory ] || [ ! -b "$dvdDevice" ] || (( $# != 6 )); then 
	echo "usage: $0 DVD_device title_num language_id subtitle_id|nosub normal|hq|dvd output_directory"
	echo "example: $0 /dev/dvd 1 128 nosub normal /path/to/output"
	exit 1
fi

function mencoderprogress () {
	total=$1
	cumul=$2
	lockfile=$3
	let totalRateInBytesPS=(audioRate+videoRate)*1000/8
	let totalSizeinBytes=titleLenght*totalRateInBytesPS
	#~ lastpercent=$cumul
	
	echo $cumul
	
	while [ -f $lockfile ]; do
		currentSizeInBytes=`ls -l $dvdTitle.$fileFormat | cut -f5 -d' '`
		let percent=(currentSizeInBytes*total/totalSizeinBytes)+cumul
		#~ if (( $percent != $lastpercent )); then
			echo $percent
			#~ lastpercent=$percent
		#~ fi
		sleep 1
	done
}


{ mplayer -dvd-device $dvdDevice dvd://$titleNum -identify -vo null -ao null -frames 0 2>&1 > $INFOFILE; } 2>&1 >/dev/null
dvdTitle=`cat $INFOFILE | grep "ID_DVD_VOLUME_ID" | cut -f2 -d'=' | tr A-Z a-z`
titleLenght=`cat $INFOFILE | grep "ID_LENGTH" | cut -f2 -d'=' | cut -f1 -d'.'`


{ mplayer -dvd-device $dvdDevice dvd://$titleNum -vo null -ao null -vf cropdetect -frames 250 2>&1 > $INFOFILE; } 2>&1 >/dev/null
cropInfo=`cat $INFOFILE | grep "crop=" | tail -n1 | cut -f2 -d'=' | cut -f1 -d')'`
rm -f $INFOFILE


cd "$outputDirectory"
if [ "$quality" == "dvd" ]
then videoRate=$dvdVideoRate
	audioRate=$dvdAudioRate
	fileFormat="vob"
	{ mplayer -dvd-device $dvdDevice dvd://$titleNum -dumpstream -dumpfile $dvdTitle.vob; } 2>&1
	status=$?
else	fileFormat="avi"
	if [ "$quality" == "hq" ]
	then videoRate=$HQVIDEORATE
		audioRate=$HQAUDIORATE
		scale="scale=720:-3"
	else videoRate=$VIDEORATE
		audioRate=$AUDIORATE
		scale="scale=540:-3"
	fi

	if [ "$subtitleId" == "nosub" ]
	then subtitles="-slang none"
	else subtitles="-vobsubout $dvdTitle-sid $subtitleId"
	fi


	rm -f divx2pass.log
	touch /tmp/simpleburn-ripdvd-encoding.$$-1
	{ sleep 5; mencoderprogress 49 0 /tmp/simpleburn-ripdvd-encoding.$$-1; } & #50->49 (AVI container)
	{ mencoder -dvd-device $dvdDevice dvd://$titleNum -aid $languageId \
		$OAC_STRING=$audioRate -slang none \
		-ovc lavc -lavcopts vcodec=mpeg4:vbitrate=$videoRate:v4mv:mbd=2:trell:turbo:autoaspect:vpass=1 \
		-vf crop=$cropInfo,$scale,hqdn3d=2:1:2 \
		-o $dvdTitle.avi; } 2>&1
	rm -f /tmp/simpleburn-ripdvd-encoding.$$-1
	
	touch /tmp/simpleburn-ripdvd-encoding.$$-2
	{ sleep 5; mencoderprogress 49 50 /tmp/simpleburn-ripdvd-encoding.$$-2; } & #50% (first pass) has been done
	{ mencoder -dvd-device $dvdDevice dvd://$titleNum -aid $languageId \
		$OAC_STRING=$audioRate $subtitles \
		-ovc lavc -lavcopts vcodec=mpeg4:vbitrate=$videoRate:v4mv:mbd=2:trell:autoaspect:vpass=2 \
		-vf crop=$cropInfo,$scale,hqdn3d=2:1:2 \
		-o $dvdTitle.avi; } 2>&1
	status=$?
	rm -f /tmp/simpleburn-ripdvd-encoding.$$-2

	echo "100"

	rm -f divx2pass.log
fi
cd -

exit $status
