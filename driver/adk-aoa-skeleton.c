/*
 * adk-aoa-skeleton.c : kernel module to initialize linux machine as android accessory 
 * 			and also talk to accessory interface using bulk transfer
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

#define MYIOC_TYPE 'k'

int my_device_vid=0x04e8;
int my_device_pid=0x6865;
int init_on_probe=0; //initialize device within probe only if this value is true
		     //otherwise one need to go for ioctl later
module_param(my_device_vid,int,S_IRUGO);
module_param(my_device_pid,int,S_IRUGO);
module_param(init_on_probe,int,S_IRUGO);

/* table of devices that work with this driver */
static struct usb_device_id adk_aoa_table[] = {
	{ USB_DEVICE(MY_DEV_VENDOR_ID,  MY_DEV_PRODUCT_ID) },	//replaced by module params
	{ USB_DEVICE(CUSTOM_VENDOR_ID,  CUSTOM_PRODUCT_ID) },
        { USB_DEVICE(AOA_ACCESSORY_VID, AOA_ACCESSORY_PID) },
        { USB_DEVICE(AOA_ACCESSORY_VID, AOA_ACCESSORY_ADB_PID) },
        { USB_DEVICE(AOA_ACCESSORY_VID, AOA_AUDIO_PID) },
        { USB_DEVICE(AOA_ACCESSORY_VID, AOA_AUDIO_ADB_PID) },
        { USB_DEVICE(AOA_ACCESSORY_VID, AOA_ACCESSORY_AUDIO_PID) },
        { USB_DEVICE(AOA_ACCESSORY_VID, AOA_ACCESSORY_AUDIO_ADB_PID) },
	{ } /*Terminating entry*/
};
MODULE_DEVICE_TABLE(usb, adk_aoa_table);

/* Get a minor range for your devices from the usb maintainer */
#define USB_SKEL_MINOR_BASE	192
#define MAX_TRANSFER            (PAGE_SIZE - 512)

struct usb_adk_aoa {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct kref		kref;
	struct mutex		io_mutex;		/* synchronize I/O with disconnect */
	int			vendor_id;
	int 			product_id;
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
};
#define to_adk_aoa_dev(d) container_of(d, struct usb_adk_aoa, kref)
struct kset 		*adk_kset;
static struct usb_driver adk_aoa_driver;

static int aoa_init_accessory(struct usb_adk_aoa* dev,const accessory_t* acc);
static int is_in_accessory_mode(int vid,int pid);
static int aoa_get_version(struct usb_adk_aoa*);
static int aoa_send_identity(struct usb_adk_aoa*,int,void*,int);
static int aoa_start_mode(struct usb_adk_aoa*,int,int);

static ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	struct usb_adk_aoa* pdev=container_of(kobj,struct usb_adk_aoa,adk_kobj);
	int ver=aoa_get_version(pdev);
	printk("ADKSKEL::device supports AOA version %d.0\n",ver);
        return sprintf(buf, "%d.0\n", ver);
}
static ssize_t identity_store(struct kobject *kobj, struct kobj_attribute *attr,
		                         const char *buf, size_t count)
{
	struct usb_adk_aoa* pdev=container_of(kobj,struct usb_adk_aoa,adk_kobj);
	char ibuf[4],*ps;
	ibuf[0]=buf[0];ibuf[1]=buf[1];ibuf[2]=buf[2];ibuf[3]='\0';
	ps=strchr(buf,'=');
	if(ps==NULL)
	{
		printk("ADKSKEL::string not in name=value format\n");
		return count;
	}
	ps++;			//skip the character '='
	ps[strlen(ps)-1]='\0';  //strip off new line character
	//TODO:think of logic to use delimiter separated strings
	//for eg:- "Google, Inc.:DemoKit:description:2.0:url:serial"
	printk("ADKSKEL::sending identity:%s,val=%s\n",ibuf,ps);
	if(strcasecmp(ibuf,"man")==0)		//manufacturer
		aoa_send_identity(pdev,AOA_STRING_MAN_ID,ps,strlen(ps)+1);
	else if(strcasecmp(ibuf,"mod")==0)	//model
		aoa_send_identity(pdev,AOA_STRING_MOD_ID,ps,strlen(ps) + 1);
	else if(strcasecmp(ibuf,"ver")==0)	//version
		aoa_send_identity(pdev,AOA_STRING_VER_ID,ps,strlen(ps) + 1);
	else if(strcasecmp(ibuf,"des")==0)	//description
		aoa_send_identity(pdev,AOA_STRING_DSC_ID,ps,strlen(ps) + 1);
	else if(strcasecmp(ibuf,"url")==0)	//url
		aoa_send_identity(pdev,AOA_STRING_URL_ID,ps,strlen(ps) + 1);
	else if(strcasecmp(ibuf,"ser")==0)	//serial no.
		aoa_send_identity(pdev,AOA_STRING_SER_ID,ps,strlen(ps) + 1);
	else
		printk("ADKSKEL::unknown identity string\n");
        return count;
}
static ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr,
		                         const char *buf, size_t count)
{
	struct usb_adk_aoa* pdev=container_of(kobj,struct usb_adk_aoa,adk_kobj);
	if(strncmp(buf,"aud",3)==0)
	{
		printk("ADKSKEL::starting device in audio mode\n");
		aoa_start_mode(pdev,AOA_AUDIO_SUPPORT,1);
	}
	else if(strncmp(buf,"acc",3)==0) {
		printk("ADKSKEL::starting device in accessory mode\n");
		aoa_start_mode(pdev,AOA_START_ACCESSORY,1);
	}
	else
		printk("ADKSKEL::sysfs::unknown start mode\n");
        return count;
}
static ssize_t adk_attr_show(struct kobject *kobj,struct attribute *attr,char *buf)
{
	struct kobj_attribute* pobj=container_of(attr,struct kobj_attribute,attr);
	printk("attr show::name=%s\n",attr->name);
	return pobj->show(kobj,pobj,buf);
}
static ssize_t adk_attr_store(struct kobject *kobj,struct attribute *attr,
                               const char *buf, size_t len)
{
	struct kobj_attribute* pobj=container_of(attr,struct kobj_attribute,attr);
	printk("attr show::name=%s\n",attr->name);
	return pobj->store(kobj,pobj,buf,len);
}
static void adk_sysfs_release(struct kobject *kobj)
{
	//TODO
}
static struct kobj_attribute version_attribute =
        __ATTR(version, 0666, version_show,NULL);
static struct kobj_attribute identity_attribute =
        __ATTR(identity, 0666, NULL, identity_store);
static struct kobj_attribute mode_attribute =
        __ATTR(start, 0666, NULL, mode_store);

static struct attribute *adk_default_attrs[] = {
        &version_attribute.attr,
        &identity_attribute.attr,
        &mode_attribute.attr,
        NULL,   /* need to NULL terminate the list of attributes */
};
static const struct sysfs_ops adk_sysfs_ops = {
        .show = adk_attr_show,
        .store = adk_attr_store,
};
static struct kobj_type adk_ktype = {
        .sysfs_ops = &adk_sysfs_ops,
        .release = adk_sysfs_release,
        .default_attrs = adk_default_attrs,
};
static void adk_aoa_delete(struct kref *kref)
{
	struct usb_adk_aoa *dev = to_adk_aoa_dev(kref);
	usb_put_dev(dev->udev);
	if(dev->bulk_in_buffer)
		        kfree(dev->bulk_in_buffer);
	if(!is_in_accessory_mode(dev->vendor_id,dev->product_id))
		kobject_put(&dev->adk_kobj);
	kfree(dev);
}

static int adk_aoa_open(struct inode *inode, struct file *file)
{
	struct usb_adk_aoa *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;
	subminor = iminor(inode);
	interface = usb_find_interface(&adk_aoa_driver, subminor);
	if (!interface) {
		printk("%s - error, can't find device for minor %d", __func__, subminor);
		retval = -ENODEV;
		goto exit;
	}
	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		goto exit;
	}
	kref_get(&dev->kref);
	mutex_lock(&dev->io_mutex);
	if (!dev->open_count++) {
		retval = usb_autopm_get_interface(interface);
		if (retval) {
			dev->open_count--;
			mutex_unlock(&dev->io_mutex);
			kref_put(&dev->kref, adk_aoa_delete);
			goto exit;
		}
	}
	file->private_data = dev;
	mutex_unlock(&dev->io_mutex);
	printk("ADKSKEL/AOASKEL::device opened successfully,ready for ioctl/bulk transfer\n");
exit:
	return retval;
}

static int adk_aoa_release(struct inode *inode, struct file *file)
{
	struct usb_adk_aoa *dev;
        dev = (struct usb_adk_aoa *)file->private_data;
        if (dev == NULL)
        	return -ENODEV;
	mutex_lock(&dev->io_mutex);
	if (!--dev->open_count && dev->interface)
		usb_autopm_put_interface(dev->interface);
	mutex_unlock(&dev->io_mutex);
        kref_put(&dev->kref, adk_aoa_delete);
	printk("ADKSKEL/AOASKEL::device closed successfully\n");
	return 0;
}

static ssize_t aoa_read(struct file *file, char *buffer, size_t count,
			 loff_t *ppos)
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

static ssize_t aoa_write(struct file *file, const char *user_buffer,
			  size_t count, loff_t *ppos)
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
static long adk_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct usb_adk_aoa *dev;
	int size, retval, direction;
	void __user *ioargp = (void __user *)arg;
	accessory_t *pacc;
	dev=(struct usb_adk_aoa*) file->private_data;
	if(is_in_accessory_mode(dev->vendor_id,dev->product_id))
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
static struct usb_class_driver adk_class = {
	.name =		"adk-skel%d",
	.fops =		&adk_fops,
	.minor_base =	USB_SKEL_MINOR_BASE,
};
static const struct file_operations aoa_fops = {
	.owner 		=	THIS_MODULE,
	.open 		=	adk_aoa_open,
	.release 	=	adk_aoa_release,
	.read 		=	aoa_read,
	.write 		=	aoa_write,
};
static struct usb_class_driver aoa_class = {
	.name =		"aoa-skel%d",
	.fops =		&aoa_fops,
	.minor_base =	USB_SKEL_MINOR_BASE,
};
static void adk_aoa_disconnect(struct usb_interface *interface)
{
	struct usb_adk_aoa *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &adk_class);

	/* prevent more I/O from starting */
	mutex_lock(&dev->io_mutex);
	dev->interface = NULL;
	mutex_unlock(&dev->io_mutex);

	/* decrement our usage count */
	kref_put(&dev->kref, adk_aoa_delete);

	dev_info(&interface->dev, "ADKSKEL::USB Skeleton #%d now disconnected\n", minor);
}
static int adk_aoa_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct usb_adk_aoa *dev = usb_get_intfdata(intf);

	if (!dev)
		return 0;
	//adk_draw_down(dev);
	return 0;
}

static int adk_aoa_resume(struct usb_interface *intf)
{
	return 0;
}

static int adk_aoa_pre_reset(struct usb_interface *intf)
{
	struct usb_adk_aoa *dev = usb_get_intfdata(intf);

	mutex_lock(&dev->io_mutex);
	//adk_draw_down(dev);

	return 0;
}

static int adk_aoa_post_reset(struct usb_interface *intf)
{
	struct usb_adk_aoa *dev = usb_get_intfdata(intf);

	/* we are sure no URBs are active - no locking needed */
	mutex_unlock(&dev->io_mutex);

	return 0;
}
static int is_in_accessory_mode(int vid,int pid)
{
	if(vid==AOA_ACCESSORY_VID && (pid==AOA_ACCESSORY_PID || pid==AOA_ACCESSORY_ADB_PID
	  || pid==AOA_AUDIO_PID || pid==AOA_AUDIO_ADB_PID || pid==AOA_ACCESSORY_AUDIO_PID
	  || pid==AOA_ACCESSORY_AUDIO_ADB_PID))
		return 1;
	else
		return 0;
}
static int aoa_get_version(struct usb_adk_aoa* dev)
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
static int aoa_send_identity(struct usb_adk_aoa* dev,int ident_id,void* data,int size)
{
	int ret,timeout=0,value=0,index=ident_id,request=AOA_SEND_IDENT;

	ret=usb_control_msg(dev->udev,usb_sndctrlpipe(dev->udev,0),
			request,USB_TYPE_VENDOR | USB_DIR_OUT,
			value,index,data,size,timeout);
	return ret;
}
static int aoa_start_mode(struct usb_adk_aoa* dev,int aoa_mode,int value)
{
	int ret,timeout=0,index=0;
	ret=usb_control_msg(dev->udev,usb_sndctrlpipe(dev->udev,0),
                  aoa_mode,USB_TYPE_VENDOR | USB_DIR_OUT,value,index,NULL,0,timeout);
	if(ret<0) printk("ADKSKEL::error in starting acessory/audio\n");
	return ret;
}
static int aoa_init_accessory(struct usb_adk_aoa* dev,const accessory_t* acc)
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
static int adk_aoa_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{
	struct usb_adk_aoa *dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
        size_t buffer_size;
	int i;

	int retval = -ENOMEM;
	/* allocate memory for our device state and initialize it */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		printk("Out of memory");
		goto error;
	}
	kref_init(&dev->kref);
	mutex_init(&dev->io_mutex);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;
	dev->vendor_id=id->idVendor;
	dev->product_id=id->idProduct;

	iface_desc = interface->cur_altsetting;
	printk("ADKSKEL::probe,iface num=%d\n",iface_desc->desc.bInterfaceNumber);
	printk("ADKSKEL::num end points:%d\n",iface_desc->desc.bNumEndpoints);

	if(is_in_accessory_mode(dev->vendor_id,dev->product_id))
		printk("Device already in aoa mode\n");
	else {
		if(init_on_probe)
			aoa_init_accessory(dev,&acc_default);
	}
	//else go for user space code with ioctl with custom attributes

	if(is_in_accessory_mode(dev->vendor_id,dev->product_id) && 
		iface_desc->desc.bInterfaceNumber==AOA_ACCESSORY_INTERFACE)
	{
		 printk("AOASKEL:devid=%04x:%04x\n",id->idVendor,id->idProduct);
                 for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
			endpoint = &iface_desc->endpoint[i].desc;
			pr_info("AOASKEL::ep addr:%x\n",endpoint->bEndpointAddress);
			if (!dev->bulk_in_endpointAddr && usb_endpoint_is_bulk_in(endpoint)) {
                        /* we found a bulk in endpoint */
	                     buffer_size = usb_endpoint_maxp(endpoint);
			     dev->bulk_in_size = buffer_size;
			     dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			     dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			     if (!dev->bulk_in_buffer) {
				     printk("Could not allocate bulk_in_buffer");
				     goto error;
			     }
			}
			if (!dev->bulk_out_endpointAddr && usb_endpoint_is_bulk_out(endpoint)) {
                        /* we found a bulk out endpoint */
                                dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
                        }
		}
	}
	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);
	
	/* we can register the device now, as it is ready */
	/* we are not registering other interfaces like audio,adb */
	if(is_in_accessory_mode(dev->vendor_id,dev->product_id) && 
		iface_desc->desc.bInterfaceNumber!=AOA_ACCESSORY_INTERFACE)
	{
		printk("Device doesn't support bulk interface for read/write\n");
		return 0;
	}
	if(!is_in_accessory_mode(dev->vendor_id,dev->product_id)) {
		retval = usb_register_dev(interface, &adk_class);
		dev->adk_kobj.kset=adk_kset;
		retval=kobject_init_and_add(&dev->adk_kobj,&adk_ktype,NULL,"%s","aoa_init");
		kobject_uevent(&dev->adk_kobj, KOBJ_ADD);
	}
	else {
		retval = usb_register_dev(interface, &aoa_class);
		/*retval=kobject_init_and_add(&dev->adk_kobj,&adk_ktype,NULL,"%s","aoa_reinit");
		if (retval)
		        kobject_put(&dev->adk_kobj);
		kobject_uevent(&dev->adk_kobj, KOBJ_ADD);*/
	}
	if (retval) {
		/* something prevented us from registering this driver */
		printk("ADKSKEL::Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}
	/* let the user know what node this device is now attached to */
	dev_info(&interface->dev,"ADKSkeleton::device now attached to ADKSkel-%d\n",
		 						interface->minor);
	return 0;
error:
	if (dev)
		/* this frees allocated memory */
		kref_put(&dev->kref, adk_aoa_delete);
	return retval;
}


static struct usb_driver adk_aoa_driver = {
	.name =		"adk_skeleton",
	.probe =	adk_aoa_probe,
	.disconnect =	adk_aoa_disconnect,
	.suspend =	adk_aoa_suspend,
	.resume =	adk_aoa_resume,
	.pre_reset =	adk_aoa_pre_reset,
	.post_reset =	adk_aoa_post_reset,
	.id_table =	adk_aoa_table,
	.supports_autosuspend = 1,
};

static int __init usb_adk_aoa_init(void)
{
	int result;
	adk_kset = kset_create_and_add("adk_linux", NULL, kernel_kobj);
	if (!adk_kset)
		return -ENOMEM;

	printk("ADKSKEL::my device vid=%x,pid=%x\n",my_device_vid,my_device_pid);
	//printk("ADKSKEL::custom vid=%x,pid=%x\n",
	//	adk_aoa_table[1].idVendor,adk_aoa_table[1].idProduct);
	adk_aoa_table[0].idVendor=my_device_vid;
	adk_aoa_table[0].idProduct=my_device_pid;
	result = usb_register(&adk_aoa_driver);
	if (result)
		printk("usb_register failed. Error number %d", result);

	return result;
}

static void __exit usb_adk_aoa_exit(void)
{
	kset_unregister(adk_kset);
	/* deregister this driver with the USB subsystem */
	usb_deregister(&adk_aoa_driver);
}

module_init(usb_adk_aoa_init);
module_exit(usb_adk_aoa_exit);

MODULE_AUTHOR("Rajesh Sola");
MODULE_DESCRIPTION("Skeleton code for adk/aoa host driver");
MODULE_LICENSE("GPL");
