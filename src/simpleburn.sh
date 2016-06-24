#!/bin/sh

WRITESPEED=0
PATH=$PATH:/lib/udev:/usr/lib/udev

function printusage() {
	echo "----- Extract: -----"
	echo "usage: $0 [device] e-iso /path/file"
	echo "example: $0 e-iso linux.iso"
	echo ""
	echo "usage: $0 [device] e-audio /path/dir [wav|flac|ogg|mp3] [tracks]"
	echo "example: $0 e-audio cd-backup/ flac 1 2 5"
	echo ""
	echo "usage: $0 [device] e-video /path/file title lang [subtitles]"
	echo "example: $0 e-video dvd-backup.avi 1 fr"
	echo ""
	echo "----- Burn: -----"
	echo "usage: $0 [device] b-blank [(fast|all)]"
	echo "example: $0 /dev/sr0 b-blank"
	echo ""
	echo "usage: $0 [device] b-iso /path/file.iso"
	echo "example: $0 /dev/cdrom b-iso linux.iso"
	echo ""
	echo "usage: $0 [device] b-data /path/dir"
	echo "example: $0 b-data compilation/"
	echo ""
	echo "usage: $0 [device] b-audio /path/dir"
	echo "example: $0 b-audio compilation/"
	echo ""
	exit 1
}


function blankmedia() {
	device=$1
	speed=$2
	
	if [ -z $speed ]; then speed="fast"; fi
	cdrecord -eject gracetime=3 dev=$device blank=$speed
	status=$?
	exit $status
}


function extractiso() {
	device=$1
	destination=$2

	mediasize=`isosize $device 2>/dev/null`
	let blockscount=mediasize/2048
	rm -f "$destination"
	dd if=$device bs=2048 count=$blockscount of="$destination"
	status=$?
	exit $status
}


function extractaudio() {
	device=$1
	destination=$2
	format=$3
	shift; shift; shift; tracks=$*
	
	cwd=`pwd`
	cd "$destination"
	cdda2wav dev=$device -L1 -J -v title out-fd=1
	rm -f audio_*.inf audio.cdindex
	disc=`cat audio.cddb | sed -n /DTITLE/p | cut -f2 -d= | tr / - | iconv -c -f ISO-8859-1`
	if [ ! -z "$disc" ]; then
		mkdir -p "$disc"
		mv audio.cddb "$disc"/
		cd "$disc"
	fi
	
	if [ -z "$tracks" ]; then
		trackscount=`cat audio.cddb | sed -n /TTITLE/p | wc -l`
		for (( i=1; i<=trackscount; i++ )); do tracks="$tracks$i "; done
	fi
	
	for track in $tracks; do
		title="$track-"`cat audio.cddb | sed -n  -e /^TTITLE/p | sed -n "$track"p | cut -f2 -d= | iconv -c -f ISO-8859-1`
		if [ "$title" == "$track-" ]; then title="$track"; fi
		if (( $track < 10 )); then title="0$title"; fi
		case $format in
		"wav")	cdda2wav dev=$device -t $track "$title.wav"
				status=$?
			;;
		"flac") cdda2wav dev=$device -t $track - 2>/dev/null | flac -f -o "$title.flac" -
				status=${PIPESTATUS[0]}
			;;
		"ogg")	cdda2wav dev=$device -t $track - 2>/dev/null | oggenc -b 256 -o "$title.ogg" -
				status=${PIPESTATUS[0]}
			;;
		"mp3")	cdda2wav dev=$device -t $track - | lame -b 256 - "$title.mp3"
				status=${PIPESTATUS[0]}
			;;
		esac
	done
	
	rm -f audio.cddb
	cd $cwd
	exit $status
}


function extractvideo() {
	device=$1
	destination=$2
	title=$3
	audio=$4
	subtitles=$5
	
	if mencoder -oac help | grep -q "mp3lame"
	then oac="-oac mp3lame -lameopts mode=2:cbr:br=256"; ext="mp3"
	else oac="-oac lavc -lavcopts acodec=mp2:abitrate=256"; ext="mp2"
	fi

	aspect=`mplayer -dvd-device $device dvd://$title -ao null -frames 1 -vo null 2>&1 | sed -n '/^VO: /p' | head -n1 | sed 's/.*=> \([0-9]\+x[0-9]\+\) .*$/\1/'`
	echo "aspect=$aspect"
	width=`echo $aspect | cut -f1 -d'x'`
	cropinfos=`mplayer -dvd-device $device dvd://$title -ao null -ss 60 -frames 120 -vf cropdetect -vo null 2>&1 | grep "crop=" | tail -n1 | cut -f2 -d'=' | cut -f1 -d')'`
	height=`echo $cropinfos | cut -f2 -d:`
	if [ "`echo $cropinfos | cut -f3- -d:`" != "0:0" ]; then
		filterscmd="-vf crop=$cropinfos"
		echo "crop=$cropinfos"
	fi
	ratio=`bc <<< "scale=2; (($width/$height))"`
	echo "width=$width, height=$height, ration=$ratio"
	
	if [ -z "$subtitles" ]
	then subtitlescmd="-vobsubout nosubs"
	else subtitlescmd="-vobsubout $destination -slang $subtitles"
	fi
	
	cd "`dirname \"$destination\"`"
	destination=`basename "$destination"`
	rm -f divx2pass.log
	status=1
	mencoder -dvd-device $device dvd://$title -nosound $subtitlescmd $filterscmd \
		-ovc xvid -xvidencopts pass=1:chroma_opt:vhq=4:max_bframes=1:quant_type=mpeg:aspect=$ratio \
		-o /dev/null &&
	mencoder -dvd-device $device dvd://$title -nosound $subtitlescmd $filterscmd \
		-ovc xvid -xvidencopts pass=1:chroma_opt:vhq=4:max_bframes=1:quant_type=mpeg:aspect=$ratio \
		-o "$destination-nosound.avi" &&
	mencoder -dvd-device $device dvd://$title -ovc frameno -alang $audio $oac -o "$destination-audio.avi" &&
	mplayer -dumpaudio "$destination-audio.avi" -dumpfile "$destination.$ext" &&
	mencoder -oac copy -ovc copy -o "$destination.avi" -audiofile "$destination.$ext" "$destination-nosound.avi" &&
	status=0
	if [ "$status" == "0" ]; then
		rm -f "$destination-audio.avi"
		rm -f "$destination-nosound.avi"
		rm -f divx2pass.log
	fi
	if [ -z "$subtitles" ]; then
		rm -f nosubs.{idx,sub}
	fi
	cd - >/dev/null
	exit $status
}


function burniso() {
	device=$1
	source=$2
	
	tracksize=`ls -l "$source" | cut -f5 -d' '`
	let tracksize=tracksize/1048576 #MB
	if cdrom_id $device | grep -q MEDIA_CD_RW && ! cdrom_id $device | grep -q ID_CDROM_MEDIA_STATE=blank; then
		cdrecord gracetime=3 dev=$device blank=fast
	fi
	cdrecord -v -eject speed=$WRITESPEED gracetime=3 dev=$device -pad "$source"
	status=$?
	exit $status
}


function burnaudio() {
	device=$1
	source=$2
	
	normalize=0
	ls "$source"/*.flac 2>/dev/null | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then normalize=1
			flac --decode -o "$audiofile.wav" "$audiofile"
		fi
	done
	ls "$source"/*.mp3 2>/dev/null | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then normalize=1
			mpg123 --stereo --rate 44100 --wav "$audiofile.wav" "$audiofile"
		fi
	done
	ls "$source"/*.ogg 2>/dev/null | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then normalize=1
			oggdec -o "$audiofile.wav" "$audiofile"
		fi
	done
	if [ "$normalize" == "1" ]; then normalize "$source"/*.wav; fi

	cd "$source"
	exit
	if cdrom_id $device | grep -q MEDIA_CD_RW && ! cdrom_id $device | grep -q ID_CDROM_MEDIA_STATE=blank; then
		cdrecord gracetime=3 dev=$device blank=fast
	fi
	cdrecord -v -eject speed=$WRITESPEED gracetime=3 dev=$device -audio -pad -useinfo -text *.wav
	status=$?
	cd - >/dev/null
	exit $status
}


function burndata() {
	device=$1
	source=$2
	
	label=`basename "$source" | sed 's/ /_/g' | cut -c1-32`
	if [ -f "$source/VIDEO_TS/VIDEO_TS.IFO" ];
	then udfopt="-dvd-video"
	else udfopt="-udf"
	fi
	
	if cdrom_id $device | grep -q MEDIA_CD_RW && ! cdrom_id $device | grep -q ID_CDROM_MEDIA_STATE=blank; then
		cdrecord gracetime=3 dev=$device blank=fast
	fi
	tsize=`mkisofs -J -r -N -d -hide-rr-moved $udfopt -print-size "$source"`
	mkisofs -J -r -N -d -hide-rr-moved $udfopt -V "$label" "$source" | cdrecord -v -eject speed=$WRITESPEED gracetime=3 dev=$device driveropts=burnfree tsize=$tsize\s -data -pad -
	status=${PIPESTATUS[0]}&&${PIPESTATUS[1]}
	exit $status
}


function abort() {
	pid=$1
	pslist=""
	function listps () {
		pid=$1
		
		pslist="$pid $pslist"
		for ppid in `pgrep -P $pid`; do
			listps $ppid
		done
	}
	listps $pid
	echo "kill $pslist"
	kill $pslist
}


#common parameters
if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then printusage; fi
if [ -b "$1" ]
then device=$1; shift
else device=/dev/cdrom
fi
action=$1; shift

case $action in
	"b-blank")
		speed=$1
		if [ "$speed" != "" ] && [ "$speed" != "all" ] && [ "$speed" != "fast" ]
		then printusage
		else blankmedia $device $speed
		fi
		;;
	"e-audio")
		destination=$1
		format=$2
		shift; shift; tracks=$*
		if [ -z "$format" ]; then format="wav"; fi
		if [ -z "$destination" ] || [ ! -d "$destination" ] || ( [ "$format" != "wav" ] && [ "$format" != "mp3" ] && [ "$format" != "flac" ] && [ "$format" != "ogg" ] )
		then printusage
		else extractaudio $device "$destination" $format $tracks
		fi
		;;
	"e-video")
		destination=$1
		title=$2; audio=$3; subtitles=$4
		if [ -z "$destination" ] || [ ! -d `dirname "$destination"` ] || [ -z "$title" ] || [ -z "$audio" ]
		then printusage
		else extractvideo $device "$destination" $title $audio $subtitles
		fi
		;;
	"e-iso")
		destination=$1
		if [ -z "$destination" ] || [ ! -d `dirname "$destination"` ]
		then printusage
		else extractiso $device "$destination"
		fi
		;;
	"b-audio")
		source="$1"
		if [ -z "$source" ] || [ ! -d "$source" ]
		then printusage
		else burnaudio $device "$source"
		fi
		;;
	"b-data")
		source="$1"
		if [ -z "$source" ] || [ ! -d "$source" ]
		then printusage
		else burndata $device "$source"
		fi
		;;
	"b-iso")
		source="$1"
		if [ -z "$source" ] || [ ! -f "$source" ]
		then printusage
		else burniso $device $source
		fi
		;;
	"abort")
		pid=$1
		abort $pid
		;;
	*) printusage
esac
