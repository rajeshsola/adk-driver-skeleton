/*
 * aoa-read-dump.c : Example code to read raw data from android device which can 
 * 		     work based on adk-aoa-skeleton or aoa-skeleton module.
 *
 * Copyright (C) 2014 - Rajesh Sola <rajeshsola@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not,see <http://www.gnu.org/licenses/ >
 */

#include<unistd.h>
#include<signal.h>
#include<fcntl.h>

#include<stdio.h>
#include<stdlib.h>

int stop=0;
void handler(int signo)
{
	stop=1;
	printf("stop reading aoa dump\n");
}
int main(int argc,char* argv[])
{
	int fd,k,i;
	char *devfile;
	if(argc>1)
		devfile=argv[1];
	else
		devfile="/dev/aoa-skel0";
		fd=open(devfile,O_RDWR);
	if(fd<0)
	{
		perror("open");
		exit(0);
	}
	printf("opening %s\n",devfile);
	signal(SIGINT,handler);
	char buf[64];
	while(!stop)
	{
		k=read(fd,buf,64);
		printf("k=%d\n",k);
		if(k<0)
		{
			perror("read");
			exit(2);
		}
		if(k==0) { printf("no data from aoa\n"); }
		printf("received:%d bytes\n",k);
		for(i=0;i<k;i++)
			printf("%4x\t",(int)buf[i]);
		printf("\n");
	}
	close(fd);
	return 0;
}
