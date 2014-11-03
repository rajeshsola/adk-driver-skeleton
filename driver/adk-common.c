/*
 * adk-skeleton.c : kernel module to initialize linux machine as android accessory 
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

#include "linux-adk.h"
#include "adk-common.h"
//#include "adk-init.h"

int my_device_vid=0x22b8;//0x04e8;
int my_device_pid=0x2e82;//0x6865;
int init_on_probe=0; //initialize device within probe only if this value is true
		     //otherwise one need to go for ioctl later
module_param(my_device_vid,int,S_IRUGO);
module_param(my_device_pid,int,S_IRUGO);
module_param(init_on_probe,int,S_IRUGO);

struct kset 		*adk_kset;
static struct usb_driver adk_aoa_driver;

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

static void adk_aoa_delete(struct kref *kref)
{
	struct usb_adk_aoa *dev = to_adk_aoa_dev(kref);
	usb_put_dev(dev->udev);
	if(dev->bulk_in_buffer)
		        kfree(dev->bulk_in_buffer);
	if(dev->is_kobj_initialized)
	kobject_put(&dev->adk_kobj);
	kfree(dev);
}

int adk_aoa_open(struct inode *inode, struct file *file)
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

int adk_aoa_release(struct inode *inode, struct file *file)
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
	dev->iface_number=iface_desc->desc.bInterfaceNumber;

	printk("ADKSKEL::probe,iface num=%d\n",iface_desc->desc.bInterfaceNumber);
	printk("ADKSKEL::num end points:%d\n",iface_desc->desc.bNumEndpoints);

	if(is_in_accessory_mode(dev))
		printk("Device already in aoa mode\n");
	else {
		if(init_on_probe)
			aoa_init_accessory(dev,&acc_default);
	}
	//else go for user space code with ioctl with custom attributes

	if(is_in_accessory_mode(dev) && dev->iface_number==AOA_ACCESSORY_INTERFACE)
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
	if(is_in_accessory_mode(dev) && dev->iface_number!=AOA_ACCESSORY_INTERFACE)
	{
		printk("Device doesn't support bulk interface for read/write\n");
		return 0;
	}
	if(!is_in_accessory_mode(dev)) {
		retval = usb_register_dev(interface, &adk_class);
		dev->adk_kobj.kset=adk_kset;
		retval=kobject_init_and_add(&dev->adk_kobj,&adk_ktype,NULL,"%s","aoa_init");
		if (retval)
			kobject_put(&dev->adk_kobj);
		else
			dev->is_kobj_initialized=1;
		kobject_uevent(&dev->adk_kobj, KOBJ_ADD);
		dev_info(&interface->dev,"ADKSkeleton::device now attached to ADKSkel-%d\n",
		 						interface->minor);
	}
	else {
		retval = usb_register_dev(interface, &aoa_class);
		dev->adk_kobj.kset=adk_kset;
		retval=kobject_init_and_add(&dev->adk_kobj,&aoa_ktype,NULL,"%s","aoa_reinit");
		if (retval) {
		        kobject_put(&dev->adk_kobj);
			printk("ADKSKEL::failed to add reinit kobject\n");
		}
		else
			dev->is_kobj_initialized=1;
		printk("AOASKEL::aoa_reinit added successfully\n");
		kobject_uevent(&dev->adk_kobj, KOBJ_ADD);
		dev_info(&interface->dev,"AOASkeleton::device now attached to AOASkel-%d\n",
								interface->minor);
	}
	if (retval) {
		/* something prevented us from registering this driver */
		printk("ADKSKEL::Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}
	/* let the user know what node this device is now attached to */
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
