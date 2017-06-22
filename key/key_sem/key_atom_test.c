/*
 * key_atom_test.c
 *
 *  Created on: 2017Äê6ÔÂ22ÈÕ
 *      Author: haobo
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	int fd[4],i;

	fd[0] = open("/dev/key0");
	fd[1] = open("/dev/key1");
	fd[2] = open("/dev/key2");
	fd[3] = open("/dev/key3");

	for (i = 0;i<4;i++){
		if(fd[i]<0){
			printf("OPEN: failed!\n");
			return -1;
		}
	}
	printf("get to sleep!\n");
	sleep(5);
	printf("sleep end!!\n ");
	for (i = 0;i<4;i++)
		close(fd[i]);



}
