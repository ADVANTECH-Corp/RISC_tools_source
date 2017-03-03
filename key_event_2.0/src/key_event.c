#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <signal.h>

#define TEST_PASS 0
#define TEST_FAIL -1
char board[20];

int getBoardName(char* ret_string)
{
	FILE *pp;
	char tmp[20];

	if ((pp = popen("cat /proc/board", "r")) == NULL)
	{
		printf("\n [getBoardName] popen() error!\n");
		return TEST_FAIL;
	}
	if (fgets(tmp, 12, pp))
	{
		pclose(pp);
		strncpy(ret_string, tmp, strlen(tmp));
		return TEST_PASS;
	} else {
		pclose(pp);
		return TEST_FAIL;
	}
}

int getLinuxBase()
{
	FILE *pp;
	char tmp[80];

	if ((pp = popen("cat /etc/issue |grep Distro", "r")) == NULL)
	{
		printf("\n [getLinuxBase] popen() error!\n");
		return TEST_FAIL;
	}
	if (fgets(tmp, 80, pp)) //Yocto
	{
		pclose(pp);
		return TEST_PASS;
	} 
	else //Ltib 
	{
		pclose(pp);
		return TEST_FAIL;
	}
}

int getEventName(char* ret_string)
{
	int event_count=0, event_id=0, i;
	char event_name_cmd[64], event_name[64], event_cmd[28];
	FILE *pp;

	if(strstr(board, "ROM-3420") != NULL)
		event_count = 3;
	else
		event_count = 2;

	for(i=0; i<event_count; i++)
	{
		sprintf(event_name_cmd, "cat /sys/class/input/event%d/device/name", i);

		if ((pp = popen(event_name_cmd, "r")) == NULL) {
			printf("\n [getEventName] popen() event_name_cmd error!\n");
			continue;
		} else {
			if (fgets(event_name, 10, pp))
			{
				pclose(pp);
				
				if(!strncmp(event_name, "gpio-keys" , 9))
				{
					sprintf(event_cmd, "/dev/input/event%d", i);
					event_cmd[strlen(event_cmd)] = '\0';
					strncpy(ret_string, event_cmd, strlen(event_cmd));

					return TEST_PASS;
				}
			} else {
				//printf("\n [getEventName]Get event_name error!\n");
				pclose(pp);
				continue;
			}
		}
	}

	return TEST_FAIL;
}

int main(void)
{
	int key_state, fd, ret, type, code;
	struct input_event buf;
	char cmd[64];
	int start_count=0, suspend_flag=0, poweroff_flag=0;
	char event_cmd[28];

	if(getBoardName(board) == TEST_FAIL)
	{
		printf("\n Get board failed!\n");
		return -1;
	}

	memset(event_cmd,'\0', sizeof(event_cmd));	
	ret = getEventName(event_cmd);

	if(ret == TEST_FAIL)
	{
		printf("\n Get event command failed!\n");
		return -1;
	}

	fd = open(event_cmd, O_RDONLY);

	if (fd < 0) {
		printf("[%s] Open gpio-keys failed.\n", board);
		return -1;
	} else
		printf("[%s] Open gpio-keys success.\n", board);

	while(1) {
		ret = read(fd,&buf,sizeof(struct input_event));

		if(ret <= 0) {
			printf("read fail!\n");
			return -1;
		}

		type = buf.type;
		code = buf.code;
		key_state = buf.value;

		//printf("\n*** [key_event] type:%d; code:%d, key_state:%d\n", type,code,key_state);

		switch(code) {
			case KEY_SUSPEND:
				if(!suspend_flag) { /* sleep mode */
					if(key_state) {
 						start_count=1;
					} else {
						if(start_count) {
							printf("\n-------------------------  suspend  ----------------------\n");
							suspend_flag = 1;
							sprintf(cmd, "echo mem > /sys/power/state");
							system(cmd);
							start_count=0;
						}	
					}
				} else { /* wake up mode */
					if(!key_state)
						suspend_flag = 0;
				}
				break;
			case KEY_POWER:
				if(!poweroff_flag) {
					if(!key_state) {
						printf("\n-------------------------  poweroff  ----------------------\n");
						poweroff_flag = 1;
						sprintf(cmd, "poweroff");
						system(cmd);
					}
				}
				break;
			default:
				code = 0;
				break;
		} /* switch(code) */
	} /* while(1) */

	close(fd);
	return 0;
}

