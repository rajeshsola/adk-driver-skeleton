/*
 * adk-ioctl.c : Userspace code to initialize linux machine as Android Accessory
 * 		 using ioctl method supported by adk-init module
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
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<stdio.h>
#include<stdlib.h>

#include "../include/linux-adk.h"

#define MYIOC_TYPE 'k'

static const accessory_t acc_default = {
	.manufacturer = "Nexus-Computing GmbH",
	.model = "Simple Slider",
	.description = "A Simple Slider",
	.version = "0.1",
	.url = "http://www.nexus-computing.ch/SimpleAccessory.apk",
	.serial ="1337",
	//.mode = AOA_ACCESSORY_MODE|AOA_AUDIO_MODE,
	.mode=AOA_ACCESSORY_MODE,
};
int main()
{
	int MY_IOCTL,dummy=0;
	MY_IOCTL = (int)_IOW(MYIOC_TYPE, 1, int);
	printf("MY_IOCTL=0x%x\n",MY_IOCTL);

	int fd,ret;
	fd=open("/dev/adk-skel0",O_RDWR);
	if(fd<0)
	{
		perror("open");
		exit(1);
	}
	ret=ioctl(fd,MY_IOCTL,&acc_default);
	if(ret<0)
	{
		perror("ioctl");
		exit(2);
	}
	close(fd);
	return 0;
}
