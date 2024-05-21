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
DEVICE_LIFE_TIME=$((0x$DEVICE_LIFE_TIME_EST_TYP_A))
if [ $DEVICE_LIFE_TIME -lt $((0x$DEVICE_LIFE_TIME_EST_TYP_B)) ] ;then
	DEVICE_LIFE_TIME=$((0x$DEVICE_LIFE_TIME_EST_TYP_B))
fi

TIMES=3
LIFE=$[ ($DEVICE_LIFE_TIME - 1) * 10 ]

print(){
while [[ TIMES -lt 4 ]]
        do
        echo "eMMC: More than $LIFE% of the life  be used!" |tee -a /var/log/emmc_life.log
       /bin/sleep 8
        ((TIMES++))
        done
}

if [[ "$trueout" != "" ]];then
case $DEVICE_LIFE_TIME in 
"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8")
	print
	;;
"9"|"10")
	TIMES=1
	print
	echo "[eMMC life Warning!!]"
	;;
"11")
	TIMES=1
	print
        echo "The eMMc life Exceeds estimates"
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
	echo "eMMC Read error!"
	exit
fi

