#!/bin/bash

INFOFILE="/tmp/ripdvd-detection.tmp.$$"
SUMARYFILE="/tmp/ripdvd-detection.$$"
MINTITLELENGTH=900 #seconds

dvdDevice=$1
if [ ! -b "$dvdDevice" ]; then 
	echo "usage: $0 DVD_device"
	echo "example: $0 /dev/dvd"
	exit 1
fi

#get DVD informations
{ mplayer -dvd-device $dvdDevice dvd://1 -identify -vo null -ao null -frames 0 2>&1 > $INFOFILE; } 2>&1 >/dev/null

#for each title during more than $MINTITLELENGTH'
for title in `cat $INFOFILE | grep "TITLE_[0-9]\+_LENGTH"`; do
	titleNum=`echo $title | cut -d'_' -f4`
	titleLenght=`echo $title | cut -d'=' -f2 | cut -f1 -d'.'`
	let titleLenghtInMinutes=titleLenght/60
	if (( $titleLenght > $MINTITLELENGTH )); then
		if (( $titleNum != 1 )); then
			{ mplayer -dvd-device $dvdDevice dvd://$titleNum -identify -vo null -ao null -frames 0 2>&1 > $INFOFILE; } 2>&1 >/dev/null
		fi
		
		languages=""
		for language in `cat $INFOFILE | grep "ID_AID"`; do
			languageId=`echo $language | cut -d'_' -f3`
			languageName=`echo $language | cut -d'=' -f2 | cut -f1 -d'.'`
			#~ if cat $INFOFILE | grep "stereo" | grep -q $languageId; then #AC3 stereo only
			languages="$languages$languageName-$languageId "
			#~ fi
		done
		subtitles=""
		for subtitle in `cat $INFOFILE | grep "ID_SID"`; do
			subtitleId=`echo $subtitle | cut -d'_' -f3`
			subtitleName=`echo $subtitle | cut -d'=' -f2`
			subtitles="$subtitles$subtitleName-$subtitleId "
		done
		if [ "$2" == "oneline" ]
			then echo "$titleNum:$titleLenghtInMinutes:$languages:$subtitles" | sed 's/ :/:/' | sed 's/ $//' >> $SUMARYFILE
			else echo -e "Title: $titleNum\nLenght: $titleLenghtInMinutes'\nLanguages: $languages\nSubtitles: $subtitles\n" >> $SUMARYFILE
		fi
	fi
done

rm -f $INFOFILE
if [ -f "$SUMARYFILE" ]
then cat $SUMARYFILE
	rm -f $SUMARYFILE
else exit 1
fi


