/*
 * key_test.c
 *
 *  Created on: May 11, 2017
 *      Author: haobo
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(void)
{
	char val[4];
	int fd1 = open("/dev/key0",O_RDWR)\
			,fd2 = open("/dev/key1",O_RDWR),\
			fd3 = open("/dev/key2",O_RDWR),fd4 = open("/dev/key3",O_RDWR);
	if(!(fd1/*|fd2|fd3|fd4*/)){
		printf("error:open");
	}

	while(1){
		read(fd1,&val[0],1);
		if(val[0] != 1){
			printf("key1_pressed!\n");
		}
		read(fd2,&val[1],1);
		if(val[1] != 1){
			printf("led2_pressed!\n");
		}
		read(fd3,&val[2],1);
		if(val[2] != 1){
			printf("led3_pressed!\n");
		}
		read(fd4,&val[3],1);
		if(val[3] != 1){
			printf("led4_pressed!\n");
		}


	}
	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);
}

