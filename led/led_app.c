/*
 * led_app.c
 *
 *  Created on: May 10, 2017
 *      Author: haobo
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
int main(int argc,char * argv[])
{
	int fd = open("/dev/led",O_RDWR);
	if(!fd){
		printf("error£º¡¡open");
	}
	if(argc != 2){
		printf("usage ./app on/off");
		return -1;
	}
	printf("write %s count %d bytes \n",argv[1],strlen(argv[1]));
	write(fd,argv[1],strlen(argv[1]));

	close(fd);

}

