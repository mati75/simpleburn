#!/bin/bash

CDRECORD=`which cdrskin 2>/dev/null`
CDDA2WAV=`which cdparanoia 2>/dev/null || which cd-paranoia 2>/dev/null`
MKISOFS=`which xorriso 2>/dev/null`
CDRDAO=`which cdrdao 2>/dev/null`

if (($# == 0)); then
	echo "CDRECORD=$CDRECORD"
	echo "CDDA2WAV=$CDDA2WAV"
	echo "MKISOFS=$MKISOFS"
	echo "CDRDAO=$CDRDAO"
fi