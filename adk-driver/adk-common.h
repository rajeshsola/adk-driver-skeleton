/*
 * adk-common.h : common header file
 *
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
#ifndef __ADK_COMMON_H
#define __ADK_COMMON_H

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

#include "linux-adk.h"

struct usb_adk_aoa {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct kref		kref;
	struct mutex		io_mutex;		/* synchronize I/O with disconnect */
	int			vendor_id;
	int 			product_id;
	int			iface_number;
	/*------------------------------------*/
        unsigned char           *bulk_in_buffer;        /* the buffer to receive data */
        size_t                  bulk_in_size;           /* the size of the receive buffer */
        size_t                  bulk_in_filled;         /* number of bytes in the buffer */
        size_t                  bulk_in_copied;         /* already copied to user space */
        __u8                    bulk_in_endpointAddr;   /*the address of the bulk in endpoint*/
        __u8                    bulk_out_endpointAddr;  /*the address of the bulk out endpoint*/
        int                     errors;                 /* the last request tanked */
        int                     open_count;             /* count the number of openers */

	int			aoa_version;
	struct kobject		adk_kobj;
	int 			is_kobj_initialized;
};
#define to_adk_aoa_dev(d) container_of(d, struct usb_adk_aoa, kref)

int aoa_init_accessory(struct usb_adk_aoa* dev,const accessory_t* acc);
int is_in_accessory_mode(struct usb_adk_aoa* dev);
int aoa_get_version(struct usb_adk_aoa*);
int aoa_send_identity(struct usb_adk_aoa*,int,void*,int);
int aoa_start_mode(struct usb_adk_aoa*,int,int);

int adk_aoa_open(struct inode *inode, struct file *file);
int adk_aoa_release(struct inode *inode, struct file *file);


/*static void adk_aoa_delete(struct kref *kref);
static void adk_aoa_disconnect(struct usb_interface *interface);
static int adk_aoa_suspend(struct usb_interface *intf, pm_message_t message);
static int adk_aoa_resume(struct usb_interface *intf);
static int adk_aoa_pre_reset(struct usb_interface *intf);
static int adk_aoa_post_reset(struct usb_interface *intf);
static int adk_aoa_probe(struct usb_interface *interface,const struct usb_device_id *id);*/

extern struct kobj_type adk_ktype;
extern struct kobj_type aoa_ktype;

extern struct usb_class_driver adk_class;
extern struct usb_class_driver aoa_class;

#endif
