/*
 * adk-sysfs.c : sysfs operations
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

#include "adk-sysfs.h"

struct kobj_attribute version_attribute =
        __ATTR(version, 0666, version_show,NULL);
struct kobj_attribute identity_attribute =
        __ATTR(identity, 0666, NULL, identity_store);
struct kobj_attribute mode_attribute =
        __ATTR(start, 0666, NULL, mode_store);
struct kobj_attribute bulk_attribute =
        __ATTR(bulk, 0666, NULL, NULL);

struct attribute *adk_default_attrs[] = {
        &version_attribute.attr,
        &identity_attribute.attr,
        &mode_attribute.attr,
        NULL,   /* need to NULL terminate the list of attributes */
};
struct attribute *aoa_default_attrs[] = {
        &version_attribute.attr,
	&bulk_attribute.attr,
        NULL,   /* need to NULL terminate the list of attributes */
};
const struct sysfs_ops adk_sysfs_ops = {
        .show = adk_attr_show,
        .store = adk_attr_store,
};
struct kobj_type adk_ktype = {
        .sysfs_ops = &adk_sysfs_ops,
        .release = adk_sysfs_release,
        .default_attrs = adk_default_attrs,
};
struct kobj_type aoa_ktype = {
        .sysfs_ops = &adk_sysfs_ops,
        .release = adk_sysfs_release,
        .default_attrs = aoa_default_attrs,
};

ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	struct usb_adk_aoa* pdev=container_of(kobj,struct usb_adk_aoa,adk_kobj);
	int ver=aoa_get_version(pdev);
	printk("ADKSKEL::device supports AOA version %d.0\n",ver);
        return sprintf(buf, "%d.0\n", ver);
}
ssize_t identity_store(struct kobject *kobj, struct kobj_attribute *attr,
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
ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr,
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
ssize_t adk_attr_show(struct kobject *kobj,struct attribute *attr,char *buf)
{
	struct kobj_attribute* pobj=container_of(attr,struct kobj_attribute,attr);
	printk("attr show::name=%s\n",attr->name);
	return pobj->show(kobj,pobj,buf);
}
ssize_t adk_attr_store(struct kobject *kobj,struct attribute *attr,
                               const char *buf, size_t len)
{
	struct kobj_attribute* pobj=container_of(attr,struct kobj_attribute,attr);
	printk("attr show::name=%s\n",attr->name);
	return pobj->store(kobj,pobj,buf,len);
}
void adk_sysfs_release(struct kobject *kobj)
{
	//TODO
}

