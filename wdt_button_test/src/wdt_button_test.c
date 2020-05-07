#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>
#include <pthread.h>

#include "test_utils.h"

#define TRUE		1
#define FALSE		0
#define	GPIO_NAME	"gpio-keys"
#define WDT_OUT		352

int gTimeout;
pthread_mutex_t timeoutMutex;

void help_info(void);

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
		if(strstr(event_name , GPIO_NAME ) != NULL) {
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

static void * timeout_handle(void *arg)
{
	char cmd[64];

	while(1)
	{
		pthread_mutex_lock(&timeoutMutex);
		if(gTimeout > 0)
		{
			gTimeout--;
			//printf("gTimeout = %d\n", gTimeout);
		}
		else
		{
			//Set WDT Out
			sprintf(cmd, "echo %d > /sys/class/gpio/export", WDT_OUT);
			system(cmd);
			sprintf(cmd, "echo out > /sys/class/gpio/gpio1/direction");
			system(cmd);
			sprintf(cmd, "echo 1 > /sys/class/gpio/gpio1/value");
			system(cmd);
			printf("\n------------start reboot------------\n");
			sprintf(cmd, "reboot");
			system(cmd);
		}
		pthread_mutex_unlock(&timeoutMutex);

		sleep(1);
	}

	return NULL;
}

int main(int argc, char * const argv[])
{
	int fd, timeout, ret;
	char event_cmd[28];
	struct input_event inputevent;
	pthread_t timeoutThread = NULL;

	if (argc < 2) {
		help_info();
		return 1;
	}
	timeout = atoi(argv[1]);
	printf("Starting SW_WDT (timeout: %d)\n", timeout);

	gTimeout = timeout;

	// Get Event ID
	memset(event_cmd,'\0', sizeof(event_cmd));	

	if(!getEventName(event_cmd)) {
		printf("%s: Can't get %s event\n", argv[0], GPIO_NAME);
		exit(1);
	}

	fd = open(event_cmd, O_RDONLY);
	if (fd < 0) {
		printf("%s: Open gpio-keys failed.\n", argv[0]);
		exit(1);
	} else
		printf("%s: Open gpio-keys success.\n", argv[0]);

	pthread_mutex_init(&timeoutMutex, NULL);

	if(pthread_create(&timeoutThread, NULL, timeout_handle, NULL))
	{
		printf("create timeout thread fail.\n");
	}
	
	while (1) {
		ret = read(fd, &inputevent, sizeof(struct input_event));
		if(ret <= 0) {
			printf("read fail!\n");
			exit(1);
		}

		switch(inputevent.type){
			case EV_KEY:
				switch(inputevent.code) {
					case KEY_PROG1: /* SW WDT triger*/
						if(!inputevent.value) {
							pthread_mutex_lock(&timeoutMutex);
							if(gTimeout > 0)
							{
								printf("WDT KEEPALIVE\n");
								gTimeout = timeout;
							}
							pthread_mutex_unlock(&timeoutMutex);
						}
					break;
				}
			break;
		}
	}

	print_result(argv);

	close(fd);
	return 0;
}

void help_info(void)
{
	printf("Usage: wdt_driver_test <timeout>\n");
	printf("    timeout: value in seconds to cause wdt timeout/reset\n");
}
