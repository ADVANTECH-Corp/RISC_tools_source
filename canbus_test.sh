#!/bin/sh
DEV=$1
D_OUT=out1
DATA="12345678   \[6\]  12 34 12 34 12 34"


if [ "`ifconfig | grep "$DEV"`" == "" ] ; then
	ip link set "$DEV" up type can bitrate 125000
fi


# this is a trap for ctrl + c
trap ctrl_c INT

function ctrl_c()
{
        killall candump
	rm "$D_OUT"
        echo "CTRL+C received, exit"
        exit
}


cansend "$DEV" 12345678#123412341234
while true
do
	if [ "`ps | grep candump`" == "" ] ; then
		candump "$DEV" > "$D_OUT" &
	fi
	
        if [ "`grep "$DATA" "$D_OUT"`" != "" ] ; then
		killall candump
		echo "$DEV receive ok, Send data"
		usleep 100000
		cansend "$DEV" 12345678#123412341234
        fi
done

