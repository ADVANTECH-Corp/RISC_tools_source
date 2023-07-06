#!/bin/bash
ls /dev/mmc* > emmc_life.log 2>&1
GETPATH=`grep "boot" emmc_life.log |awk 'NR==1{print $1}'`
DEVICE=`echo ${GETPATH:0:12}`

ecsd $DEVICE 187 4 > emmc_life.log 2>&1
ecsd $DEVICE 39 4 >> emmc_life.log 2>&1
DEVICE_HEALTH_USED=`cat emmc_life.log|awk 'NR==2{print $5}'`
DEVICE_BAD_BLK=`cat emmc_life.log|awk 'NR==4{print $5}'`
OUTPUT=`grep "[-]" emmc_life.log`

rm emmc_life.log
sync

if [[ x$OUTPUT != x ]];then
	echo "Life loss:$((0x$DEVICE_HEALTH_USED))%"
	echo "Bad block:$((0x$DEVICE_BAD_BLK))"
else
	echo "Read error!"
fi
exit

