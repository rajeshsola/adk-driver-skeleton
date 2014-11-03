#ifndef __ADK_SKELETON_H
#define __ADK_SKELETON_H

#define MYIOC_TYPE 'k'

#include "adk-common.h"

static long adk_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/*int aoa_init_accessory(struct usb_adk_aoa* dev,const accessory_t* acc);
int is_in_accessory_mode(struct usb_adk_aoa* dev);
int aoa_get_version(struct usb_adk_aoa*);
int aoa_send_identity(struct usb_adk_aoa*,int,void*,int);
int aoa_start_mode(struct usb_adk_aoa*,int,int);*/

#endif
