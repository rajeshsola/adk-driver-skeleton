/*
 * Linux ADK - linux-adk.h
 *
 * Copyright (C) 2013 - Gary Bisson <bisson.gary@gmail.com>
 *
 * Modified according to the requirements of kernel space driver
 * by Rajesh Sola<rajeshsola@gmail.com>
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

#ifndef _LINUX_ADK_H_
#define _LINUX_ADK_H_
 
/* Android Open Accessory protocol defines */
enum aoa_control_codes {
	AOA_GET_PROTOCOL	=51,
	AOA_SEND_IDENT		=52,
	AOA_START_ACCESSORY	=53,
	AOA_REGISTER_HID	=54,
	AOA_UNREGISTER_HID	=55,
	AOA_SET_HID_REPORT_DESC	=56,
	AOA_SEND_HID_EVENT	=57,
	AOA_AUDIO_SUPPORT	=58,
};

/* String IDs */
enum aoa_identity_string_ids {
	AOA_STRING_MAN_ID=0,
	AOA_STRING_MOD_ID=1,
	AOA_STRING_DSC_ID=2,
	AOA_STRING_VER_ID=3,
	AOA_STRING_URL_ID=4,
	AOA_STRING_SER_ID=5,
};

/* Product IDs / Vendor IDs */
#define AOA_ACCESSORY_VID		0x18D1	/* Google */
#define AOA_ACCESSORY_PID		0x2D00	/* accessory */
#define AOA_ACCESSORY_ADB_PID		0x2D01	/* accessory + adb */
#define AOA_AUDIO_PID			0x2D02	/* audio */
#define AOA_AUDIO_ADB_PID		0x2D03	/* audio + adb */
#define AOA_ACCESSORY_AUDIO_PID		0x2D04	/* accessory + audio */
#define AOA_ACCESSORY_AUDIO_ADB_PID	0x2D05	/* accessory + audio + adb */

#define AOA_ACCESSORY_INTERFACE		0x00

enum _aoa_support_mode_t
{
	AOA_ACCESSORY_MODE=1,
	AOA_AUDIO_MODE=2,
	AOA_HID_MODE=4
};
typedef enum _aoa_support_mode_t aoa_support_mode_t;

/* Structures */
typedef struct _accessory_t {
	unsigned int  aoa_version;
	char *manufacturer;
	char *model;
	char *description;
	char *version;
	char *url;
	char *serial;
	aoa_support_mode_t mode;
}accessory_t;

static const accessory_t acc_default = {
	.manufacturer = "Nexus-Computing GmbH",
	.model = "Simple Slider",
	.description = "A Simple Slider",
	.version = "0.1",
	.url = "http://www.nexus-computing.ch/SimpleAccessory.apk",
	.serial ="1337",
	.mode=AOA_ACCESSORY_MODE|AOA_AUDIO_MODE,
};

#define CUSTOM_VENDOR_ID	0x22b8
#define CUSTOM_PRODUCT_ID	0x2e83

#define MY_DEV_VENDOR_ID	0x0000
#define MY_DEV_PRODUCT_ID	0x0000

#endif /* _LINUX_ADK_H_ */
