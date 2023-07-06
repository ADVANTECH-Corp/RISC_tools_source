#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include <linux/fs.h>
#include "ecsd_func.h"


static void help()
{
	printf("example:\n");
	printf("sandisk\n");
	printf("sandisk /dev/mmcblk*\n");
	printf("sandisk /dev/mmcblk* 0-511\n");
	printf("sandisk /dev/mmcblk* 0-511 1-512\n");
}

int main(int argc, char *argv[])
{
		int ret;
		int flag=0;
		int start_number;
		int size_number;
		char cmp_dev[50];
		char dev[50]="/dev/mmcblk2";
	 if(argc>0&&argc<5){
		 switch(argc){
			case 1 :
			{   
				start_number=187;				
				size_number=4;			
				break;
			}
			case 2 :
			{
				strcpy(dev,argv[1]);
				start_number=511;
				size_number=512;
				break;
			}
			case 3 :
			{
				strcpy(dev,argv[1]);
				start_number=atoi(argv[2]);
				if(start_number>=0&&start_number<512){
					size_number=1;
				}
				else{
					printf("argument error\n");
					help();
					goto err;
				}
				break;
			}
			case 4 :
			{
				strcpy(dev,argv[1]);
				start_number=atoi(argv[2]);
				size_number=atoi(argv[3]);
				if(start_number>=0&&start_number<512&&size_number>0&&size_number<start_number+2){
				}
				else{
					printf("argument error\n");
					help();
					goto err;
				}
				break;
			}	
		 }
		 printf("----------Health-Report-----------\n");
		 loop:
		 ret=read_healthreport_all_buff(dev,&start_number,&size_number);
				if(ret){
					printf("read error,can not find device.ret=%d\n",ret);
					exit(1);
		        }
		        flag=flag+1;
		        if(argc==1&&flag<2)
		        {
		        start_number=39;
		        size_number=4;
		        goto loop;
		        }
	 }
	 else {
		 printf("argument error\n");
		 help();
	 }
	err:
	exit(1);
	return 0;
}
