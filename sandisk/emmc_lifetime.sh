#!/bin/bash
ls /dev/mmc* > /mnt/data/emmc_life.log 2>&1
GETPATH=`grep "boot" /mnt/data/emmc_life.log |awk 'NR==1{print $1}'`
DEVICE=`echo ${GETPATH:0:12}`

ecsd $DEVICE 187 4 > /mnt/data/emmc_life.log 2>&1
ecsd $DEVICE 39 4 >> /mnt/data/emmc_life.log 2>&1
DEVICE_HEALTH_USED=`cat /mnt/data/emmc_life.log|awk 'NR==2{print $5}'`
DEVICE_BAD_BLK=`cat /mnt/data/emmc_life.log|awk 'NR==4{print $5}'`
OUTPUT=`grep "[-]" /mnt/data/emmc_life.log`

rm /mnt/data/emmc_life.log
sync

if [[ x$OUTPUT != x ]];then
	echo "Percentage of eMMC used:$((0x$DEVICE_HEALTH_USED))%"
	echo "Bad block num:$((0x$DEVICE_BAD_BLK))"
else
	echo "Read error!"
fi
exit

