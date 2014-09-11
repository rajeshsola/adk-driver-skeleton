/*
 * backlight-demo.c : Example code to change brightness of linux as adk when slider changes in 
 *  		      SimpleAccessory app,which works with adk-aoa-skeleton module.
 * Value of /sys/class/baclight/acpi_video/brightness is adjusted to change the backlight 
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
 * with this program; if not,see <http://www.gnu.org/licenses/>
 */

#include<unistd.h>
#include<signal.h>
#include<fcntl.h>

#include<stdio.h>
#include<stdlib.h>

int stop=0;
unsigned int maxval=0;
double part;
void setvalue(int,unsigned int);
void handler(int signo)
{
	stop=1;
	kill(0,SIGKILL);
	printf("stop reading aoa dump\n");
}
int main(int argc,char* argv[])
{
	int fd,cfd,k,i;
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
	unsigned char buf[2];
	unsigned char maxbuf[20];
	unsigned int maxval;
	double part;
	//cfd=open("/sys/class/backlight/intel_backlight/max_brightness",O_RDONLY);
	cfd=open("/sys/class/backlight/acpi_video0/max_brightness",O_RDONLY);
	read(cfd,maxbuf,20);
	maxval=strtoul(maxbuf,NULL,10);
	part=maxval/256.0;
	printf("sysfs::maxval=%lu\n",maxval);
	close(cfd);
	//cfd=open("/sys/class/backlight/intel_video0/brightness",O_WRONLY);
	cfd=open("/sys/class/backlight/acpi_video0/brightness",O_WRONLY);
	if(cfd<0) {
		perror("backlight open");
		exit(1);
	}
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
		//printf("received:%d bytes\n",k);
		//printf("%u\t",(unsigned int)buf[0]);
		setvalue(cfd,buf[0]);
		printf("\n");
	}
	close(cfd);
	close(fd);
	return 0;
}
void setvalue(int cfd,unsigned int level)
{
	char cmdbuf[64];
	int buflen;
	level++;	//1 to 256, instead of 0 to 255
	//printf("level=%u,cmd=%d\n",level,(int)(part*level));
	buflen=sprintf(cmdbuf,"%d\n",(int)(part*level));
	puts(cmdbuf);
	write(cfd,cmdbuf,buflen);
	//sprintf(cmdbuf,"amixer sset 'Master' %d\n",level);
	//system(cmdbuf);	//change volume thru alsa mixer
}
