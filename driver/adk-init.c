/*
 * adk-skeleton.c : 
 * Skeleton code for ADK Host driver based on drivers/usb/usb-skeleton.c of 
 * kernel source v2.6.3 or higher by Greg Kroah-Hartman (greg@kroah.com) and
 * https://github.com/gibsson/linux-adk by Gary Bisson <bisson.gary@gmail.com>
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
 * with this program; if not, see <http://www.gnu.org/licenses/>
 */
#include "adk-init.h"

/* Get a minor range for your devices from the usb maintainer */
#define ADK_SKEL_MINOR_BASE	192

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static const struct file_operations adk_fops = {
	.owner 		=	THIS_MODULE,
	.open 		=	adk_aoa_open,
	.release 	=	adk_aoa_release,
	.unlocked_ioctl =	adk_ioctl,
};
struct usb_class_driver adk_class = {
	.name =		"adk-skel%d",
	.fops =		&adk_fops,
	.minor_base =	ADK_SKEL_MINOR_BASE,
};

static long adk_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct usb_adk_aoa *dev;
	int size, retval, direction;
	void __user *ioargp = (void __user *)arg;
	accessory_t *pacc;
	dev=(struct usb_adk_aoa*) file->private_data;
	if(is_in_accessory_mode(dev))
	{
		printk("Device already in accessory mode,ioctl not applicable\n");
		return -EINVAL;
	}
	if (_IOC_TYPE (cmd) != MYIOC_TYPE) {
	        printk (KERN_INFO "ADKSKEL::got invalid case, CMD=%d\n", cmd);
	        return -EINVAL;
    	}

	direction = _IOC_DIR (cmd);
    	size = _IOC_SIZE (cmd);
	pacc = kzalloc(sizeof(accessory_t),GFP_KERNEL);
	retval=copy_from_user(pacc,ioargp,sizeof(accessory_t));
	if(retval<0) return retval;
	printk("ADKSKEL::acc-->%s,%s,%s,%s,%s,%s\n",pacc->manufacturer,pacc->model,
			pacc->description,pacc->version,pacc->url,pacc->serial);
	switch (direction) {
		case _IOC_WRITE: 
			printk("ADKSKEL::Trying to initialize phone in accessory mode\n");
			aoa_init_accessory(dev,pacc);
			break;
		default		:printk("ADKSKEL::invalid icotl operation\n");
	}
	kfree(pacc);
	return 0;
}

int is_in_accessory_mode(struct usb_adk_aoa* pdev)
{
	int vid=pdev->vendor_id,pid=pdev->product_id;
	if(vid==AOA_ACCESSORY_VID && (pid==AOA_ACCESSORY_PID || pid==AOA_ACCESSORY_ADB_PID
	  || pid==AOA_AUDIO_PID || pid==AOA_AUDIO_ADB_PID || pid==AOA_ACCESSORY_AUDIO_PID
	  || pid==AOA_ACCESSORY_AUDIO_ADB_PID))
		return 1;
	else
		return 0;
}
int aoa_get_version(struct usb_adk_aoa* dev)
{
	int ret,version,timeout=0,value=0,index=0;
	char ver_buf[2];//two byte data to support versions > 255 in future
	ret=usb_control_msg(dev->udev,usb_rcvctrlpipe(dev->udev, 0),
			AOA_GET_PROTOCOL,USB_TYPE_VENDOR | USB_DIR_IN,
			value,index,ver_buf,sizeof(ver_buf),timeout);
	if(ret<0) {
		printk("ADKSKEL::error in getting protocol\n");
		return -1;
	}
	version = ver_buf[1] << 8 | ver_buf[0];	
	return version;
}
int aoa_send_identity(struct usb_adk_aoa* dev,int ident_id,void* data,int size)
{
	int ret,timeout=0,value=0,index=ident_id,request=AOA_SEND_IDENT;

	ret=usb_control_msg(dev->udev,usb_sndctrlpipe(dev->udev,0),
			request,USB_TYPE_VENDOR | USB_DIR_OUT,
			value,index,data,size,timeout);
	return ret;
}
int aoa_start_mode(struct usb_adk_aoa* dev,int aoa_mode,int value)
{
	int ret,timeout=0,index=0;
	ret=usb_control_msg(dev->udev,usb_sndctrlpipe(dev->udev,0),
                  aoa_mode,USB_TYPE_VENDOR | USB_DIR_OUT,value,index,NULL,0,timeout);
	if(ret<0) printk("ADKSKEL::error in starting acessory/audio\n");
	return ret;
}
int aoa_init_accessory(struct usb_adk_aoa* dev,const accessory_t* acc)
{
	int ret,ver;
	//get protocol(51)
	ver=aoa_get_version(dev);
	if(ver<0) goto err;
	printk("ADKSKEL::Device supports version no.%d.0!\n",ver);
	dev->aoa_version=ver;
	//acc->aoa_version=ver;

	//send identity-Manufacturer(52)
	ret=aoa_send_identity(dev,AOA_STRING_MAN_ID,
			acc->manufacturer,strlen(acc->manufacturer) + 1);
	if(ret<0) goto err;

	//send identity-ModelName(52)
	ret=aoa_send_identity(dev,AOA_STRING_MOD_ID,acc->model,strlen(acc->model) + 1);
	if(ret<0) goto err;

	//send identity-Description(52)
	ret=aoa_send_identity(dev,AOA_STRING_DSC_ID,
			acc->description,strlen(acc->description) + 1);
	if(ret<0) goto err;

	//send identity-version(52)
	ret=aoa_send_identity(dev,AOA_STRING_VER_ID,acc->version,strlen(acc->version) + 1);
	if(ret<0) goto err;

	//send identity-url(52)
	ret=aoa_send_identity(dev,AOA_STRING_URL_ID,acc->url,strlen(acc->url) + 1);
	if(ret<0) goto err;

	//send identity-serial no.(52)
	ret=aoa_send_identity(dev,AOA_STRING_SER_ID,acc->serial,strlen(acc->serial) + 1);
	if(ret<0) goto err;

	if(dev->aoa_version >=2 && (acc->mode & AOA_AUDIO_MODE)) {
		printk("ADKSKEL::requesting audio support from the device\n");
		ret=aoa_start_mode(dev,AOA_AUDIO_SUPPORT,1); //request audio support
		if(ret<0) printk("ADKSKEL::failed to get audio support\n");
	}

	printk("ADKSKEL::starting device in accessory mode\n");
	ret=aoa_start_mode(dev,AOA_START_ACCESSORY,1); // start in accessory mode
	if(ret<0) goto err;
	printk("ADKSKEL::Accessory got initialized successfully\n");

	return 0;
err:
	printk("ADKSKEL::Accessory initialization got failed\n");
	return -EINVAL;
}

