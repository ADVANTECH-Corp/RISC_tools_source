#!/bin/bash
ls /dev/mmc* > emmc_life.log 2>&1
getpath=`grep "boot" emmc_life.log |awk 'NR==1{print $1}'`
device=`echo ${getpath:0:12}`

echo "eMMC node: $device"
ecsd $device 269 3 > emmc_life.log 2>&1
DEVICE_LIFE_TIME_EST_TYP_B=`cat emmc_life.log|awk '{print $2}'`
DEVICE_LIFE_TIME_EST_TYP_A=`cat emmc_life.log|awk '{print $3}'`
PRE_EOL_INFO=`cat emmc_life.log|awk '{print $4}'`
trueout=`grep "[-]" emmc_life.log`

rm emmc_life.log
sync

if [[ "$trueout" != "" ]];then
    DEVICE_LIFE_TIME=$((0x$DEVICE_LIFE_TIME_EST_TYP_A))
    if [ $DEVICE_LIFE_TIME -lt $((0x$DEVICE_LIFE_TIME_EST_TYP_B)) ] ;then
	DEVICE_LIFE_TIME=$((0x$DEVICE_LIFE_TIME_EST_TYP_B))
    fi

case $DEVICE_LIFE_TIME in 
"01")
       	echo "0%-10% device life time used" 
       	;;
"02")
	echo "10%-20% device life time used"
	;;
"03")
	echo "20%-30% device life time used" 
	;;
"04")
	echo "30%-40% device life time used" 
	;;
"05")
	echo "40%-50% device life time used"
	;;
"06")
	echo "50%-60% device life time used"
	;;
"07")
	echo "60%-70% device life time used"
	;;
"08")
	echo "70%-80% device life time used"
	;;
"09")
	echo "80%-90% device life time used"
	;;
"10")
	echo "90%-100% device life time used"
	;;
"11")
	echo "Exceeded its maximum estimated device life time"
	;;
*)
	echo "Reserved"
	;;
esac


case $PRE_EOL_INFO in
"01")
	echo "eMMC status: Normal" 
	exit
	;;
"02")
	echo "eMMC status:eMMC Waring Consumed 80% of reserved block" 
	exit
	;;
"03")
	echo "eMMC status:Urgent" 
	exit
	;;
*)
	echo "Reserved" 
	exit
	;;
esac

else
	echo "read error"
fi

