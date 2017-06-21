#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
int main(int argc, char **argv)
{
	int fd[4],i;
	unsigned char key_val;
	int ret;

	struct pollfd fds[4];

	fd[0] = open("/dev/key0", O_RDWR);
	fd[1] = open("/dev/key1", O_RDWR);
	fd[2] = open("/dev/key2", O_RDWR);
	fd[3] = open("/dev/key3", O_RDWR);

	for(i = 0;i<4;i++){
		if (fd[i] < 0){
			printf("can't open!\n");
		}
		fds[i].fd     = fd[i];
		fds[i].events = POLLIN;
	}
	while (1)
	{
		ret = poll(fds, 4, 500);
		if (ret == 0)
		{
			printf("time out\n");
		}
		else{

			for(i = 0;i<4;i++){
				if((fds[i].revents&POLLIN) == POLLIN){
					read(fd[i], &key_val, 1);
					printf("key_val = 0x%x\n", key_val);
				}
			}
		}
	}

	return 0;
}
