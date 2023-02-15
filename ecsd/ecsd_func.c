#include <stdio.h>
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
#include <linux/fs.h> /* for BLKGETSIZE */
#include <stdbool.h>
#include "mmc.h"
#include "ecsd_func.h"



#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif




static int read_extcsd(int fd, __u8 *ext_csd)
{
	int ret = 0;
	struct mmc_ioc_cmd idata;
	memset(&idata, 0, sizeof(idata));
	memset(ext_csd, 0, sizeof(__u8) * 512);
	idata.write_flag = 0;
	idata.opcode = MMC_SEND_EXT_CSD;
	idata.arg = 0;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blksz = 512;
	idata.blocks = 1;
	mmc_ioc_cmd_set_data(idata, ext_csd);

	ret = ioctl(fd, MMC_IOC_CMD, &idata);
	if (ret)
		perror("ioctl");

	return ret;
}


//argument4
int read_ext_csd_all_buff(char* device,int* s,int* j)
{
	__u8 ext_csd[512];
	int fd, ret;
	__u8 buff[512];
	if(!device){
		ret = E_NULL;
		goto ERR;
	}
	fd = open(device, O_RDWR);
	if (fd < 0) {
		perror("open");
		ret = E_OPEN;
		goto ERR;
	}
	ret = read_extcsd(fd, ext_csd);
	if (ret) {
		fprintf(stderr, "could not read EXT_CSD from %s\n", device);
		ret = E_IOCTL;
		goto ERR1;
	}
	buff[0] = ext_csd[*s];
	if(*j>=16){ 
        	printf("[%04d-%04d]:  ",*s,*s-15);
	}
	else 	printf("[%04d-%04d]:  ",*s,*s-*j+1);
	for(int i=0;i<*j;i++)
	{
			buff[i]=ext_csd[*s];
			printf("%02x ",buff[i]);
		   if((i+1)%4==0){
			 printf("\t");
			}
		   if(((i+1)%16==0)&&(*j-i>=16)){
				   printf("\n[%04d-%04d]:  ",*s-1,*s-16);
			}
		   if(*j%16==*j-i-1&&*j-i-1>0){
				  printf("\n[%04d-%04d]:  ",*s-1,*s-*j%16);
			} 
			*s=*s-1;          
	}
	printf("\n");	
	return ret;
ERR1:
	close(fd);
ERR:
	return ret;
}
