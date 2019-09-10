#!/bin/sh
DEV=$1
BITRATE=$2
DELAY_TIME=$3
D_OUT=out_$1
COUNT=0
DATA="12345678   \[6\]  12 34 12 34 12 34"
if [ "$1" == "" ] || [ "$2" == "" ] || [ "$3" == "" ]; then
	echo "Usage:"
	echo "./canbus_test device bitrate delay"
	echo "eg."
	echo "./canbus_test can0 125000 1"
	exit 1
fi

ifconfig $DEV down

ip link set "$DEV" up type can bitrate $BITRATE


# this is a trap for ctrl + c
trap ctrl_c INT

function ctrl_c()
{
        killall candump
#	rm "$D_OUT"
        echo "CTRL+C received, exit"
        exit
}


cansend "$DEV" 12345678#123412341234
while true
do
	if [ "`ps | grep candump`" == "" ] ; then
		candump "$DEV" > "$D_OUT" &
	fi
	sync
	
        if [ "`grep "$DATA" "$D_OUT"`" != "" ] ; then
		killall candump
		COUNT=$((COUNT + 1))
		echo "[$COUNT] : $DEV receive ok"
		sleep $DELAY_TIME
		cansend "$DEV" 12345678#123412341234
		echo "[$COUNT] : $DEV Send data"
        fi
done

