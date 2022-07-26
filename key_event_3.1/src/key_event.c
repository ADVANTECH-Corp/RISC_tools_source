#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define TRUE		1
#define FALSE		0
#define	GPIO_NAME	"gpio-keys"
#define	SNVS_NAME	"snvs-powerkey"
#define MCU		"MCU"
#define SNVS	"SNVS"
#define TIME_COUNT_DEFAULT 1

int time_count = TIME_COUNT_DEFAULT;
static int snvs_pwrkey_pressed=-1;

enum distro {
	UBUNTU_DISTRO = 1,
	YOCTO_DISTRO,
};

int getEventName(char* ret_string)
{
	int i = 0;
	char event_name[64], event_path[28], cmd[64] , buf[16];
	FILE *fp;
	
	while (1) {
		sprintf(cmd, "cat /sys/class/input/input%d/name", i);
		fp = popen(cmd , "r");
		fgets(event_name, sizeof(event_name), fp);
		//printf("event%d name = %s\n",i,event_name);
		if((strstr(event_name , GPIO_NAME ) != NULL) || 
			(strstr(event_name , SNVS_NAME ) != NULL)) {
			sprintf(event_path, "/dev/input/event%d", i);
			event_path[strlen(event_path)] = '\0';
			strncpy(ret_string, event_path, strlen(event_path));
			pclose(fp);
			return TRUE;
		} else {
			i++;
			memset(buf ,'\0', sizeof(buf));	
			sprintf(cmd, "test -d /sys/class/input/input%d && echo Exist",i);
			fp = popen(cmd , "r");
			fgets(buf, sizeof(buf), fp);
			if ( strstr(buf , "Exist" ) == NULL ) {
				pclose(fp);
				return FALSE;
			}
			
		}
		
	}
}

int getLinuxBase(int* distro)
{
	FILE *pp;
	char os_distro[80];

	pp = popen("cat /etc/issue", "r");
	fgets(os_distro, sizeof(os_distro), pp);

	if ( strstr(os_distro , "Ubuntu") != NULL) {
		*distro = UBUNTU_DISTRO;
		printf("getLinuxBase: Ubuntu\n");
	}
	else { //Yocto
		*distro = YOCTO_DISTRO;
                printf("getLinuxBase: Yocto\n");
	}

	pclose(pp);
	return TRUE;
}

void SuspendProcess(int os_distro)
{
	char cmd[64];
	printf("\n------------suspend-------------\n");
	switch (os_distro) {
		case UBUNTU_DISTRO:
			sprintf(cmd, "systemctl suspend");
			break;
		case YOCTO_DISTRO:
			sprintf(cmd, "echo mem > /sys/power/state");
			break;
	}
	//printf("%s\n", cmd);
	system(cmd);
}
void PowerOffProcess()
{
	char cmd[64];
	printf("\n------------poweroff------------\n");
	sprintf(cmd, "poweroff");
	system(cmd);
}
int Check_Label(char *label)
{
	FILE* fp;
	char cmd[64],buf[64];
	int i=0;

	memset(buf ,'\0', sizeof(buf));
	if(strstr(label , MCU ) != NULL) {
		sprintf(cmd, "cat /sys/firmware/devicetree/base/gpio-keys/*/label | grep %s -c -i", label);
		fp = popen(cmd , "r");
		fgets(buf, sizeof(buf), fp);
		pclose(fp);
	} else if(strstr(label , SNVS ) != NULL) {
		printf("Check_Label SNVS\n");
		while (1) {
			sprintf(cmd, "cat /sys/class/input/input%d/name", i);
			fp = popen(cmd , "r");
			fgets(buf, sizeof(buf), fp);
			pclose(fp);
			if(strstr(buf, SNVS_NAME ) != NULL) {
				break;
			} else {
				i++;
				memset(buf ,'\0', sizeof(buf));	
				sprintf(cmd, "test -d /sys/class/input/input%d && echo Exist",i);
				fp = popen(cmd , "r");
				fgets(buf, sizeof(buf), fp);
				pclose(fp);
				if ( strstr(buf, "Exist" ) == NULL ) {
					memset(buf, '\0', sizeof(buf)); 
					break;
				}
			}
		}
	}

	if (atoi(buf))
		return TRUE;
	else
		return FALSE;
}

int Check_CPU()
{
        FILE* fp;
        char cmd[64],buf[64];
        sprintf(cmd, "cat /proc/cpuinfo | grep processor -c");
        fp = popen(cmd , "r");
        memset(buf ,'\0', sizeof(buf));
        fgets(buf, sizeof(buf), fp);
        pclose(fp);
        if (atoi(buf) < 3)
                return TRUE;
        else
		return FALSE;
}

static void * handle_time_count(void *arg)
{
	while(1)
	{
		time_count++;

		if( time_count == 2147483646 ) /* Avoid Overflow */
		{
			time_count = TIME_COUNT_DEFAULT;
		}

		usleep(1000000);
	}
	return NULL;
}

static void * handle_power_timeout(void *arg)
{
	struct timeval *start_time,end_time;
	unsigned long long	interval_time;

	start_time = arg;
	while(1)
	{
		usleep(50000);
		gettimeofday(&end_time,NULL);
		interval_time = (end_time.tv_sec*1000000+end_time.tv_usec) - (start_time->tv_sec*1000000+start_time->tv_usec);
		if(snvs_pwrkey_pressed && (interval_time > 1000000))
		{
			PowerOffProcess();
			break;
		} else if(!snvs_pwrkey_pressed)
			break;
	}

	pthread_exit(0);
	return NULL;
}

int main(int argc, char **argv)
{
	int fd, ret ,suspend_flag=0;
	struct input_event inputevent;
	char event_cmd[28];
	int os_distro;
	struct timeval start_time,end_time;
	unsigned long long  interval_time;
	pthread_t time_count_thread = NULL;
	pthread_t timeout_thread = NULL;

	// Get OS Distro
	if(!getLinuxBase(&os_distro)) {
		printf("%s: Not support for this OS distro.\n");
		return FALSE;
	}

	// Get Event ID
	memset(event_cmd,'\0', sizeof(event_cmd));	

	if(!getEventName(event_cmd)) {
		printf("%s: Can't get %s or %s event\n", argv[0], GPIO_NAME, SNVS_NAME);
		return FALSE;
	}

	fd = open(event_cmd, O_RDONLY);
	if (fd < 0) {
		printf("%s: Open gpio-keys failed.\n", argv[0]);
		return FALSE;
	} else
		printf("%s: Open gpio-keys success.\n", argv[0]);

	if(pthread_create(&time_count_thread, NULL, handle_time_count, NULL)) {
		printf("Create time_count thread failed.\n");
	}

	while(1) {
		ret = read(fd,&inputevent,sizeof(struct input_event));

		if(ret <= 0) {
			printf("read fail!\n");
			return FALSE;
		}
		switch(inputevent.type){
			case EV_PWR:
				switch(inputevent.code) {
					case KEY_POWER:
						if(inputevent.value)
							PowerOffProcess();
					break;

					case KEY_SUSPEND:
						if(!suspend_flag && time_count >= TIME_COUNT_DEFAULT) { /* sleep mode */
							if(!inputevent.value) {
								suspend_flag = 1;
								SuspendProcess(os_distro);
							}
						} else  /* wake up mode */
							if(inputevent.value)
							{
								suspend_flag = 0;
								time_count = 0;
							}
					break;

					case KEY_SLEEP:
						if(!suspend_flag && time_count >= TIME_COUNT_DEFAULT) { /* sleep mode */
							if(!inputevent.value) {
								suspend_flag = 1;
								SuspendProcess(os_distro);
							}
						} else  /* wake up mode */
							if(!inputevent.value)
							{
								suspend_flag = 0;
								time_count = 0;
							}
					break;
				}
				break;
			case EV_KEY:
				switch(inputevent.code) {
					case KEY_SUSPEND:
						if(!suspend_flag) { /* sleep mode */
							if(!inputevent.value) {
								SuspendProcess(os_distro);
								suspend_flag = 1;
							}
						} else  /* wake up mode */
							if(!inputevent.value)
								suspend_flag = 0;
					break;
					case KEY_POWER:
						if (Check_Label(MCU)) {
							if(!suspend_flag) { /* sleep mode */
								if(inputevent.value) gettimeofday(&start_time,NULL);
								else {
									gettimeofday(&end_time,NULL);
									interval_time = (end_time.tv_sec*1000000+end_time.tv_usec) - (start_time.tv_sec*1000000+start_time.tv_usec);
									//printf("start time = %d.%d\n",start_time.tv_sec ,start_time.tv_usec);
									//printf("end   time = %d.%d\n",end_time.tv_sec, end_time.tv_usec);
									printf("Time interval is %llu usecs\n",interval_time);
									if ( interval_time < 1000000) {
										if(Check_CPU()){ /*if cpu core > 2 , then we should enabel susepnd flag*/
											printf("\n------suspend flag enable------\n");
											suspend_flag = 1;
										}
										SuspendProcess(os_distro);
									} else
										PowerOffProcess();
								}
							} else {
								if(!inputevent.value) {
									printf("\n------suspend flag disable------\n");
									suspend_flag = 0;
								}
							}
						} else if (Check_Label(SNVS)) {
							if(inputevent.value) {
								gettimeofday(&start_time,NULL);
								snvs_pwrkey_pressed = 1;
								if(pthread_create(&timeout_thread, NULL, handle_power_timeout, &start_time))
									printf("Create handle_power_timeout thread failed.\n");
							} else {
								snvs_pwrkey_pressed = 0;
							}
						} else {
							if(!suspend_flag) { /* sleep mode */
								if(!inputevent.value) {
									SuspendProcess(os_distro);
									suspend_flag = 1;
								}
							} else  /* wake up mode */
								if(!inputevent.value)
									suspend_flag = 0;
						}
					break;
				}
				break;
		}
	} /* while(1) */
	close(fd);
	return 0;
}

