/*
 * aoa-skeleton.c : kernel module to initialize linux machine as android accessory 
 * 			and also talk to accessory interface using bulk transfer
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
#include "aoa-skeleton.h"

/* Get a minor range for your devices from the usb maintainer */
#define AOA_SKEL_MINOR_BASE	192
#define MAX_TRANSFER            (PAGE_SIZE - 512)
/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
const struct file_operations aoa_fops = {
	.owner 		=	THIS_MODULE,
	.open 		=	adk_aoa_open,
	.release 	=	adk_aoa_release,
	.read 		=	aoa_read,
	.write 		=	aoa_write,
};
struct usb_class_driver aoa_class = {
	.name =		"aoa-skel%d",
	.fops =		&aoa_fops,
	.minor_base =	AOA_SKEL_MINOR_BASE,
};

static ssize_t aoa_read(struct file *file, char *buffer, size_t count,loff_t *ppos)
{
	struct usb_adk_aoa *dev;
	int ret,nactual,i;
	printk("AOA::Read request of %d bytes,off=%d\n",count,(int)*ppos);
        dev = file->private_data;
	ret = mutex_lock_interruptible(&dev->io_mutex);
	if (ret < 0)
		goto exit;
	if (!dev->interface) {          /* disconnect() was called */
		ret = -ENODEV;
		goto exit;
	}
        ret=usb_bulk_msg(dev->udev,usb_rcvbulkpipe(dev->udev,dev->bulk_in_endpointAddr),
	dev->bulk_in_buffer,min(dev->bulk_in_size,count),&nactual,0);
	if(ret<0) {
		printk("aoa::read-usb_bulk_msg:in failed\n");
		goto exit;
	}
        printk("AOA::read--dump\n");
	for(i=0;i<nactual;i++)
		printk("%4x ",dev->bulk_in_buffer[i]);
	printk("\n");
	ret=copy_to_user(buffer,dev->bulk_in_buffer,nactual);
	if(ret)
		ret = -EFAULT;
	else 
		ret = nactual;
exit:
	mutex_unlock(&dev->io_mutex);
	return ret;
}

static ssize_t aoa_write(struct file *file, const char *user_buffer,size_t count, loff_t *ppos)
{
	struct usb_adk_aoa *dev;
	int ret,nactual;
	char* buf;
	size_t writesize = min(count, (size_t)MAX_TRANSFER);
	printk("AOA::Write request of %d bytes,off=%d\n",count,(int)*ppos);
	dev = file->private_data;
	buf=kmalloc(writesize,GFP_KERNEL);
	if(buf==NULL) {
		ret=-ENOMEM;
		goto error;
	}
	if (copy_from_user(buf, user_buffer, writesize)) {
		ret = -EFAULT;
		goto error;
	}
	ret=usb_bulk_msg(dev->udev,usb_sndbulkpipe(dev->udev,dev->bulk_out_endpointAddr),
						              buf,writesize,&nactual,0);
	if(ret<0) {
		printk("AOA::write-usb_bulk_msg:out failed\n");
		goto error;
 	}
error:
        return ret;
}

