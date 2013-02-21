#!/bin/bash

CDRECORD=`which cdrecord 2>/dev/null || which wodim 2>/dev/null`
CDDA2WAV=`which cdda2wav 2>/dev/null || which icedax 2>/dev/null`
MKISOFS=`which mkisofs 2>/dev/null || which genisoimage 2>/dev/null`

if (($# == 0)); then
	echo "CDRECORD=$CDRECORD"
	echo "CDDA2WAV=$CDDA2WAV"
	echo "MKISOFS=$MKISOFS"
fi
