/*
 * adk-sysfs.h : sysfs operations
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
#ifndef __ADK_SYSFS_H__
#define __ADK_SYSFS_H__

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

#include "adk-common.h"

ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf);
ssize_t identity_store(struct kobject *kobj, struct kobj_attribute *attr,
		                         const char *buf, size_t count);
ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr,
		                         const char *buf, size_t count);
ssize_t adk_attr_show(struct kobject *kobj,struct attribute *attr,char *buf);
ssize_t adk_attr_store(struct kobject *kobj,struct attribute *attr,
                               const char *buf, size_t len);
void adk_sysfs_release(struct kobject *kobj);
#endif
