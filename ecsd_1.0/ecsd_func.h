#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <linux/fs.h> /* for BLKGETSIZE */
#include <stdbool.h>

/*
 *The return value of the rpmb function API from kernel
 *0x0000 (0x0080) Operation OK
 *0x0001 (0x0081) General failure
 *0x0002 (0x0082) Authentication failure (MAC comparison not matching, MAC calculation failure)
 *0x0003 (0x0083) Counter failure (counters not matching in comparison, counter incrementing failure)
 *0x0004 (0x0084) Address failure (address out of range, wrong address alignment)
 *0x0005 (0x0085) Write failure (data/counter/result write failure)
 *0x0006 (0x0086) Read failure (data/counter/result read failure)
 *0x0007    Authentication Key not yet programmed

 The values in parenthesis are valid in case the Write Counter has expired i.e. reached its max value
 */
#define E_NULL 8		//NULL point error
#define E_OPEN 9		//device open error
#define E_NOMEM 10		//malloc memory error
#define E_IOCTL 11		//ioctl error
#define E_CSD_VERSION 12	//can not read emmc version
#define E_LENGTH 13		//wrong srting length, 32b for key, 256b for read/write

/* gp return ERROR */
#define E_DRYRUN 14
#define E_PARTITION 15
#define E_ATTR	16
#define E_ALREADY_PARTITIONED 17
#define E_PARTITION_SIZE 18
#define E_OTP_SET_PARTITION 19
int read_ext_csd_all_buff(char* device,int* s,int* j);
