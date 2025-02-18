#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <signal.h>

#define TRUE		1
#define FALSE		0
#define	EVENT_NAME	"pmic_pwrkey"

int getEventName(char* ret_string)
{
	int i = 0;
	char event_name[64] = {0}, event_path[28] = {0}, cmd[64] = {0}, buf[16] = {0};
	FILE *fp = NULL;
	
	while (1) {
		sprintf(cmd, "cat /sys/class/input/input%d/name", i);
		fp = popen(cmd , "r");
		fgets(event_name, sizeof(event_name), fp);
		//printf("event%d name = %s\n", i, event_name);
		if(strstr(event_name , EVENT_NAME ) != NULL) {
			sprintf(event_path, "/dev/input/event%d", i);
			event_path[strlen(event_path)] = '\0';
			strncpy(ret_string, event_path, strlen(event_path));
			pclose(fp);
			return TRUE;
		} else {
			i++;
			memset(buf, '\0', sizeof(buf));	
			sprintf(cmd, "test -d /sys/class/input/input%d && echo Exist", i);
			fp = popen(cmd, "r");
			fgets(buf, sizeof(buf), fp);
			if ( strstr(buf, "Exist") == NULL ) {
				pclose(fp);
				return FALSE;
			}
			
		}
		
	}
}

void PowerOffProcess()
{
	char cmd[64] = {0};
	printf("\n------------poweroff------------\n");
	sprintf(cmd, "poweroff");
	system(cmd);
}

int main(int argc, char **argv)
{
	int fd = 0, ret = 0;
	struct input_event inputevent = {0};
	char event_cmd[28] = {0};

	// Get Event ID
	if(!getEventName(event_cmd)) {
		printf("%s: Can't get %s event\n", argv[0], EVENT_NAME);
		return FALSE;
	}

	fd = open(event_cmd, O_RDONLY);
	if (fd < 0) {
		printf("%s: Open %s failed.\n", argv[0], EVENT_NAME);
		return FALSE;
	} else
		printf("%s: Open %s success.\n", argv[0], EVENT_NAME);

	while(1) {
		ret = read(fd, &inputevent, sizeof(struct input_event));

		if(ret <= 0) {
			printf("read fail!\n");
			return FALSE;
		}
		switch(inputevent.type){
			case EV_KEY:
				switch(inputevent.code) {
					case KEY_POWER:
						PowerOffProcess();
					break;
				}
			break;
		}
	} /* while(1) */

	close(fd);
	return 0;
}

