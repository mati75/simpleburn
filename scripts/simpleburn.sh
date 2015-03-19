#!/bin/sh

function printusage() {
	echo "error: invalid parameters"
	echo ""
	echo "usage: $0 [CD/DVD device] blank [(fast|all)]"
	echo "usage: $0 [CD/DVD device] extract /path/to/iso_file"
	echo -e "usage: $0 [CD/DVD device] extract /path/to/audio_dir\n\t[format(wav|flac|ogg|mp3)\n\t\t[tracks(1,2,4-6,...)]\n\t]"
	echo -e "usage: $0 [CD/DVD device] extract /path/to/video_file title\n\t[quality(high|normal)\n\t\t[language(country-code/aid)\n\t\t\t[subtitles(-1|country-code/sid)\n\t\t\t\t[cropinfos(W:H:X:Y)]\n\t\t\t]\n\t\t]\n\t]"
	echo "usage: $0 [CD/DVD device] burn /path/to/iso_file"
	echo -e "usage: $0 [CD/DVD device] burn /path/to/audio_dir"
	echo "usage: $0 [CD/DVD device] burn /path/to/data_dir"
	echo "usage: $0 [CD/DVD device (source)] copy [CD/DVD device (copy)]"
	echo ""
	echo "example: $0 /dev/sr0 blank"
	exit 1
}


function extractiso() {
	device=$1
	destination=$2

	let blockscount=mediasize/2048
	let tracksize=mediasize/1024
	LOCKFILE="/tmp/simpleburn-extract.$$"
	rm -f "$destination"
	touch $LOCKFILE
	if [ "$opt" == "-cr" ]; then { sleep 5; simpleburn-gauges.sh dd $tracksize $LOCKFILE "$destination"; } & fi
	dd if=$device bs=2048 count=$blockscount of="$destination"
	status=$?
	rm -f $LOCKFILE
	exit $status
}


function expandtracks() {
	tracksrange=$1
	tracks=""
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
		else
			tracks="$tracks $expd"
		fi
		if echo $tracksrange | grep -q ","
		then tracksrange=`echo $tracksrange | cut -f2- -d,`
		else tracksrange=""
		fi
	done
	echo $tracks
}


function extractaudio() {
	device=$1
	destination=$2
	format=$3
	shift; shift; shift; tracks=$*
	
	total=0; for track in $tracks; do
		tracklength=`echo -e $mediainfos | grep "^$track;" | cut -f3 -d';'`
		minutes=`echo $tracklength | cut -f1 -d:`
		seconds=`echo $tracklength | cut -f2 -d: | sed 's/^0//'`
		let seconds=minutes*60+seconds
		lengths[$track]=seconds
		let total=total+seconds
	done
	cd "$destination"
	FIFOFILE="/tmp/simpleburn-extract.$$"; mkfifo $FIFOFILE
	let totaldone=0
	for track in $tracks; do
		let previouspercent=totaldone*100/total
		let trackpercent=lengths[$track]*100/total
		title="$track-"`echo -e $mediainfos | grep "^$track;" | cut -f2 -d';' | sed 's@/@-@g'`
		if (( $track < 10 )); then title="0$title"; fi
		case $format in
		"wav")  if [ "$opt" == "-cr" ]; then
				cdda2wav dev=$device -t $track "$title.wav" 2>$FIFOFILE | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			else
				cdda2wav dev=$device -t $track "$title.wav"
				status=$?
			fi
			;;
		"flac")  if [ "$opt" == "-cr" ]; then
				cdda2wav dev=$device -t $track - 2>$FIFOFILE | flac --totally-silent -f -o "$title.flac" - | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			else
				cdda2wav dev=$device -t $track - 2>/dev/null | flac -f -o "$title.flac" -
				status=${PIPESTATUS[0]}
			fi
			;;
		"ogg")  if [ "$opt" == "-cr" ]; then
				cdda2wav dev=$device -t $track - 2>$FIFOFILE | oggenc -b 256 -Q -o "$title.ogg" - | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			else
				cdda2wav dev=$device -t $track - 2>/dev/null | oggenc -b 256 -o "$title.ogg" -
				status=${PIPESTATUS[0]}
			fi
			;;
		"mp3")  if [ "$opt" == "-cr" ]; then
				cdda2wav dev=$device -t $track - 2>$FIFOFILE | lame -b 256 --quiet - "$title.mp3" | \
					simpleburn-gauges cdda2wav $previouspercent $trackpercent $FIFOFILE
				status=${PIPESTATUS[0]}
			else
				cdda2wav dev=$device -t $track - | lame -b 256 - "$title.mp3"
				status=${PIPESTATUS[0]}
			fi
			;;
		esac
		let totaldone=totaldone+lengths[$track]
	done
	rm -f $FIFOFILE
	cd - >/dev/null
	exit $status
}


function extractvideo() {
	device=$1
	title=$2
	destination=$3
	quality=$4
	audio=$5
	subtitles=$6
	cropinfos=$7
	
	if mencoder -oac help | grep -q "mp3lame"
	then AUDIORATE=160; HQAUDIORATE=256
		OAC_STRING="-oac mp3lame -lameopts mode=2:cbr:br"
	else	AUDIORATE=224; HQAUDIORATE=320
		OAC_STRING="-oac lavc -lavcopts acodec=mp2:abitrate"
	fi
	VIDEORATE=896; HQVIDEORATE=1856
	
	titleinfos=`echo -e $mediainfos | grep "^$title;"`
	length=`echo $titleinfos | cut -f2 -d';'`
	LOCKFILE="/tmp/simpleburn-extract.$$"
	cd "`dirname \"$destination\"`"
	destination=`basename "$destination"`
	if [ "$opt" == "-cr" ]; then
			msglevel="-msglevel all=-1"
	fi
	aid=`echo $titleinfos | cut -f3 -d';' | sed 's/,/\n/g' | grep "$audio" | cut -f1 -d'/' | head -n1`
	if [ "$subtitles" == "-1" ]
	then subtitlescmd="-vobsubout nosubs"
	else sid=`echo $titleinfos | cut -f4 -d';' | sed 's/,/\n/g' | grep "$subtitles" | cut -f1 -d'/'  | head -n1`
		subtitlescmd="-vobsubout $destination -sid $sid"
	fi
	if [ "$quality" == "normal" ]; then
		audiorate=$AUDIORATE
		videorate=$VIDEORATE
	else
		audiorate=$HQAUDIORATE
		videorate=$HQVIDEORATE
	fi
	let totalrate=(audiorate+videorate)*1000/8 #kbits -> bytes
	let totalsize=length*totalrate
	if [ ! -z "$cropinfos" ]; then
		cropinfos="-vf crop=$cropinfos"
	fi
	rm -f divx2pass.log
	touch $LOCKFILE-1
	if [ "$opt" == "-cr" ]; then { sleep 5; simpleburn-gauges.sh mencoder $totalsize 1 $LOCKFILE-1 "$destination"; } & fi
	
	{ mencoder $msglevel -dvd-device $device dvd://$title -aid $aid $OAC_STRING=$audiorate \
		-ovc lavc -lavcopts vcodec=mpeg4:vbitrate=$videorate:v4mv:mbd=2:trell:turbo:autoaspect:vpass=1 $cropinfos \
		-o "$destination"; } 1>&2 2>/dev/null
	rm -f $LOCKFILE-1
	
	touch $LOCKFILE-2
	if [ "$opt" == "-cr" ]; then { sleep 5; simpleburn-gauges.sh mencoder $totalsize 2 $LOCKFILE-2 "$destination"; } & fi
	{ mencoder $msglevel -dvd-device $device dvd://$title -aid $aid $OAC_STRING=$audiorate $subtitlescmd \
		-ovc lavc -lavcopts vcodec=mpeg4:vbitrate=$videorate:v4mv:mbd=2:trell:autoaspect:vpass=2 $cropinfos \
		-o "$destination"; } 1>&2 2>/dev/null
	status=$?
	rm -f $LOCKFILE-2
	rm -f divx2pass.log
	if [ "$subtitles" == "-1" ]; then
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
	if [ "$opt" == "-cr" ]; then
		cdrecord -v -eject gracetime=3 dev=$device -pad "$source" | simpleburn-gauges cdrecord $tracksize
		status=${PIPESTATUS[0]}
	else
		cdrecord -v -eject gracetime=3 dev=$device -pad "$source"
		status=$?
	fi
	exit $status
}


function burnaudio() {
	device=$1
	source=$2
	
	ls "$source"/*.flac 2>/dev/null | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then
			flac --decode --channels=2 --sample-rate=44100 "$audiofile" "$audiofile.wav"
		fi
	done
	ls "$source"/*.mp3 2>/dev/null | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then
			mpg123 --stereo --rate 44100 --wav "$audiofile.wav" "$audiofile"
		fi
	done
	ls "$source"/*.ogg 2>/dev/null | while read audiofile; do
		if [ ! -f "$audiofile.wav" ]; then
			oggdec -o "$audiofile.wav" "$audiofile"
		fi
	done
	#~ normalize "$source"/*.wav

	audiosizes=""
	totalsize=0
	for audiosize in `ls -l "$source"/*.wav | cut -f5 -d' '`; do
		let audiosize=audiosize/1048576
		audiosizes="$audiosizes$audiosize "
		let totalsize=totalsize+audiosize
	done
	let totalsize=totalsize*913046 #1024x1024x2048/2352
	if (( $totalsize > $mediacapacity )); then #final check due to conversion
		echo "error: not enough space on CD/DVD media"
		exit 3
	fi
	
	cd "$source"
	#notice: options -useinfo -text are ignored by cdrskin
	cdrecord -eject -v speed=0 gracetime=3 dev=$device -dao -audio -pad -useinfo -text *.wav | simpleburn-gauges cdrecord $audiosizes
	status=${PIPESTATUS[0]}
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
	tsize=`mkisofs -J -r -N -d -hide-rr-moved $udfopt -print-size "$source"`
	let tracksize=tsize/512 #blocks => MB
	if [ "$opt" == "-cr" ]; then
		mkisofs -J -r -N -d -hide-rr-moved $udfopt -V "$label" "$source" | cdrecord -v -eject gracetime=3 dev=$device driveropts=burnfree tsize=$tsize\s -data -pad - | simpleburn-gauges cdrecord $tracksize
		status=${PIPESTATUS[0]}&&${PIPESTATUS[1]}
	else
		mkisofs -J -r -N -d -hide-rr-moved $udfopt -V "$label" "$source" | cdrecord -v -eject gracetime=3 dev=$device driveropts=burnfree tsize=$tsize\s -data -pad -
		status=${PIPESTATUS[0]}&&${PIPESTATUS[1]}
	fi
	exit $status
}




#tools detection
PATH=$PATH:/lib/udev:/usr/lib/udev:/sbin:/usr/sbin:`pwd`
for tool in cdrom_id blkid isosize cdrecord cdda2wav mplayer flac mpg123 oggdec mencoder oggenc lame; do
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
action=$1; shift

source simpleburn-detect.sh $device

case $action in
	"blank")
		speed=$1
		if [ -z $speed ]; then speed="fast"; fi
		if [ "$speed" != "all" ] && [ "$speed" != "fast" ]; then
			printusage
		fi
		detect $device
		if [ $rewritablemedia != 1 ]; then 
			echo "error: no rewritable media"
			exit 3
		fi
		#all ok
		cdrecord -eject gracetime=3 dev=$device blank=$speed
		exit $?
		;;
	"extract")
		destination=$1; shift
		detect $device
		if [ "$mediatype" == "none" ] || [ "$mediacontent" == "blank" ] || [ "$mediacontent" == "udf" ]; then
			echo "error: can't extract from media ($mediatype/$mediacontent)"
			exit 3
		fi
		case $mediacontent in
			"video")
				title=$1; quality=$2; audio=$3; subtitles=$4; cropinfos=$5
				if [ -z "$quality" ]; then quality="normal"; fi
				if [ -z "$audio" ]; then if [ "$LANG" == "C" ]; then audio="en"; else audio=`echo $LANG | cut -c1-2`; fi; fi
				if [ -z "$subtitles" ]; then subtitles="-1"; fi
				if [ -z "$cropinfo" ]; then cropinfos=`mplayer -dvd-device $device dvd://$title -ao null -ss 15 -frames 120 -vf cropdetect -vo null 2>&1 | grep "crop=720:" | tail -n1 | cut -f2 -d'=' | cut -f1 -d')'`; fi
				if [ -z "$destination" ] || [ ! -d `dirname "$destination"` ] || [ -z "$title" ] || ( [ "$quality" != "normal" ] && [ "$quality" != "high" ] ); then
					printusage
				fi
				titleinfos=`echo -e $mediainfos | grep "^$title;"`
				if [ -z "$titleinfos" ]; then
					echo "error: title '$title' not available"
					exit 3
				fi
				if ! echo $titleinfos | cut -f3 -d';' | sed 's/,/\n/g' | grep -q "$audio"; then
					echo "error: audio language '$audio' not available for this title"
					exit 3
				fi
				if [ "$subtitles" != "-1" ] && ! echo $titleinfos | cut -f4 -d';' | sed 's/,/\n/g' | grep -q "$subtitles"; then
					echo "error: subtitles language '$subtitles' not available for this title"
					exit 3
				fi
				#all ok
				extractvideo $device $title "$destination" $quality $audio $subtitles $cropinfos
				;;
			"audio")
				format=$1; tracks=$2
				if [ -z "$format" ]; then format="wav"; fi
				if [ -z "$tracks" ]; then tracks="1-$trackscount"; fi
				if [ -z "$destination" ] || [ ! -d "$destination" ] || ( [ "$format" != "wav" ] && [ "$format" != "mp3" ] && [ "$format" != "flac" ] && [ "$format" != "ogg" ] ); then
					printusage
				fi
				#all ok
				tracks=`expandtracks $tracks`
				for track in $tracks; do
					if (( $track < 1 )) || (( $track > $trackscount )); then echo "error: track '$track' not available"; exit 3; fi
				done
				extractaudio $device "$destination" $format $tracks
				;;
			"iso9660")
				if [ -z "$destination" ] || [ ! -d `dirname "$destination"` ]
				then printusage
				else extractiso $device "$destination"
				fi
				;;
		esac
		;;
	"burn")
		source="$1"
		detect $device
		if [ -z "$source" ] || ( [ ! -f "$source" ] && [ ! -d "$source" ] ); then
			printusage
		fi
		if [ "$mediacontent" != "blank" ] && (( rewritablemedia == 0 )); then
			echo "error: can't burn on media ($mediatype/$mediacontent)"
			exit 3
		fi
		if mount | grep -q "^$device "; then
			if ! udisksctl unmount -b $device || umount $device; then
				echo "error: media can't be unmounted"
				exit 3
			fi
		fi
		if [ -f "$source" ]; then
			datasize=`ls -l "$source" | cut -f5 -d' '`
		else
			datasize=`du -bs "$source" | sed 's/\t.*//'`
			if [ "$mediatype" != "dvd" ] && (( `ls -l "$source"/*.wav 2>/dev/null | wc -l` != 0 )); then
				let datasize=datasize*2048/2352
			fi
		fi
		if (( $datasize > $mediacapacity )); then
			echo "error: not enough space on CD/DVD media"
			exit 3
		fi
		#all ok
		if [ "$mediacontent" != "blank" ] && [ "$opt" != "-cr" ]; then
			echo -n "warning: media is not empty ($mediatype/$mediacontent); continue (y/N)? "
			read response
			if [ "$response" != "y" ]; then exit; fi
		fi
		if [ "$mediacontent" != "blank" ] && [ "$mediatype" == "cd" ]; then
			cdrecord gracetime=3 dev=$device blank=fast
		fi
		if [ -f "$source" ]; then
			burniso $device $source
		else
			if [ "$mediatype" == "dvd" ] || (( `ls -l "$source"/*.wav 2>/dev/null | wc -l` == 0 ))
			then burndata $device "$source"
			else burnaudio $device "$source"
			fi
		fi
		;;
	"copy")
		echo "sorry: not implemented yet"
		#umount / empty target
		#data: cdrecord -v gracetime=3 dev=$cdwriter -isosize $cdreader | simpleburn-gauges cdrecord $mediasize
		#audio: cdrdao copy --fast-toc --on-the-fly --source-device $cdreader --device $cdwriter 2>&1 | simpleburn-gauges cdrdao
		#audio: cdda2wav dev=$cdreader -no-infofile -B -Oraw - 2>/dev/null | cdrecord -v gracetime=3 dev=$cdwriter -dao -audio -useinfo -text *.inf | simpleburn-gauges cdrecord $trackslengths
		
		#status=${PIPESTATUS[0]}
		;;
	"abort")
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
		;;
	*) printusage
esac
