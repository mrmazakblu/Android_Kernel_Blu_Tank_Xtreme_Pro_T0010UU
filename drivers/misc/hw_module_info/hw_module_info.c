/* Copyright Statement:
 *
 * This software/firmware and related documentation ("tydtech Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to tydtech Inc. and/or its licensors.
 * Without the prior written permission of tydtech inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of tydtech Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* tydtech Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("TYDTECH SOFTWARE")
 * RECEIVED FROM TYDTECH AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. TYDTECH EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES TYDTECH PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE TYDTECH SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN TYDTECH SOFTWARE. TYDTECH SHALL ALSO NOT BE RESPONSIBLE FOR ANY TYDTECH
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND TYDTECH'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE TYDTECH SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT TYDTECH'S OPTION, TO REVISE OR REPLACE THE TYDTECH SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * TYDTECH FOR SUCH TYDTECH SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("tydtech Software")
 * have been modified by tydtech Inc. All revisions are subject to any receiver's
 * applicable license agreements with tydtech Inc.
 */

/*
 * drivers/hw_module_info/hw_module_info.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * hw_module_info driver
 *
 */

/******************************************************************************
 * file name: hw_module_info.h
 * author: liguo
 * email: lxm542006@163.com
 * copying to A92. tyd tianyaping 2012.4.12
 * Copyright 2010 tydtech Co.,Ltd.
 ******************************************************************************/

#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include "../mediatek/include/mt-plat/mt_gpio.h"
#include <linux/hw_module_info.h>

#include <linux/mutex.h>
/****************************************************************************
 * Variable Settings
 ***************************************************************************/
#if defined(DROI_PRO_Q1)
#define CAM_MAINCAMERA_MIPI_AB		(12 | 0x80000000)
#endif

#if defined(DROI_PRO_K8)
#define CAM_MAINCAMERA_MIPI_AB		(59 | 0x80000000)
#endif
/****************************************************************************
 * MACROS
 ***************************************************************************/
#define HWM_DEBUG_ENABLE

#ifdef HWM_DEBUG_ENABLE
#define HWM_DEBUG(format, args...)  printk(KERN_INFO "hwm_info:" format,##args)
#else
#define HWM_DEBUG(format, args...)  do { } while(0)
#endif

#define HWM_INFO_ATTR(_name) \
static struct kobj_attribute _name##_attr = {   \
        .attr   = {                             \
                .name = __stringify(_name),     \
                .mode = 0644,                   \
        },                                      \
        .show   = _name##_show,                 \
        .store  = _name##_store,                \
 }

/****************************************************************************
 * structures
 ***************************************************************************/
static hw_module hw_modules[HW_MODULE_TYPE_MAX] = {
	{HW_MODULE_TYPE_CTP,            "[ Touch Panel ]"},
	{HW_MODULE_TYPE_LCM,            "[ LCM ]"},
	{HW_MODULE_TYPE_ALSPS,          "[ ALS/PS ]"},
	{HW_MODULE_TYPE_GS,             "[ G-sensor ]"},
	{HW_MODULE_TYPE_MS,             "[ M-sensor ]"},
	{HW_MODULE_TYPE_FP,             "[ Fingerprint ]"},
	{HW_MODULE_TYPE_GY,             "[ Gyroscope ]"},
	{HW_MODULE_TYPE_MAIN_CAMERA,    "[ Main Camera ]"},
	{HW_MODULE_TYPE_SUB_CAMERA,     "[ Sub Camera ]"},
	{HW_MODULE_TYPE_MAIN_2_CAMERA,  "[ Main2 Camera ]"},
	{HW_MODULE_TYPE_LEN,            "[ Len ]"},
	{HW_MODULE_TYPE_FLASHLIGHT,     "[ Flashlight ]"},
	{HW_MODULE_TYPE_BATTERY,     	 "[ Battery ]"},
	{HW_MODULE_TYPE_MAX,            "[ Unkown ]"}
};

/****************************************************************************
 * function prototypes
 ***************************************************************************/
int hw_module_info_add(hw_module_info *info);
int hw_module_info_del(hw_module_info *info);
int get_hw_module_name(hw_module_type hwm_type, char **out);

/****************************************************************************
 * global variables
 ***************************************************************************/
struct kobject *hwm_info_kobj;
#if defined(DROI_PRO_Q1)||defined(DROI_PRO_K8)|| defined(DROI_PRO_PU6)|| defined(DROI_PRO_PU6T)
struct kobject *mipi_switch_kobj;
#endif
static int hw_module_count = 0;
static DEFINE_MUTEX(hw_module_lock);
static LIST_HEAD(hw_module_infos);

// default is HW_MODULE_TYPE_ALL.
static hw_module_type getType = HW_MODULE_TYPE_ALL;
/****************************************************************************
 * functions
 ***************************************************************************/
int get_hw_module_name(hw_module_type hwm_type, char **out)
{
	int i = 0;
//	char *sRet = NULL;

	if((hwm_type > HW_MODULE_TYPE_MAX) || (hwm_type < HW_MODULE_TYPE_MIN)){
		*out = "Unkown";
		return 0;
	}

	HWM_DEBUG("get_hw_module_name: hwm_type = %d\n", hwm_type);
	for(i = 0; i < HW_MODULE_TYPE_MAX; i++){
		if(hwm_type == hw_modules[i].type){
			*out = hw_modules[i].name;
			HWM_DEBUG("hw_modules[i].name is %s\n", hw_modules[i].name);
			return 1;
		}
	}

	return 0;
}

int hw_module_info_add(hw_module_info *info)
{
	struct list_head *pos;
	hw_module_info *hwm;

	//INIT_LIST_HEAD(&info->link);

	mutex_lock(&hw_module_lock);
	list_for_each(pos, &hw_module_infos) {
		hwm = list_entry(pos, hw_module_info, link);
		// ensure the note be registered only once, or else it will be a endless loop.
		if(hwm == info){
			printk("error: the info(%s) has already add in\n",info->name);
			goto err;
		}
		// found the right pos to be place accross the priority.
		if (hwm->priority > info->priority)
			break;
	}
	INIT_LIST_HEAD(&info->link);
	list_add_tail(&info->link, pos);
	hw_module_count++;
	mutex_unlock(&hw_module_lock);

	HWM_DEBUG("hw_module_info_add called\n");
	return 0;

err:
	mutex_unlock(&hw_module_lock);
	return -1;
}
EXPORT_SYMBOL(hw_module_info_add);

int hw_module_info_del(hw_module_info *info)
{
	mutex_lock(&hw_module_lock);
	list_del(&info->link);
	hw_module_count--;
	mutex_unlock(&hw_module_lock);
	return 0;
}
EXPORT_SYMBOL(hw_module_info_del);

static ssize_t hw_module_info_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char *sOut = buf;
	char *sHwm_name = "Unkown";
	hw_module_info *pos;

        HWM_DEBUG("enter show\n");

	mutex_lock(&hw_module_lock);
	list_for_each_entry(pos, &hw_module_infos, link) {
		HWM_DEBUG("pos->type = %d\n", pos->type);
		if ((pos->type < HW_MODULE_TYPE_MAX) && (pos->type > HW_MODULE_TYPE_MIN)){
			// if not HW_MODULE_TYPE_ALL, and not the type the user want to get,we just continue to attach next item
			if((HW_MODULE_TYPE_ALL != getType) && (pos->type != getType))
				continue;
			if(get_hw_module_name(pos->type, &sHwm_name)){
					if (!strncmp(sHwm_name,"[ Battery ]",11)){

						sOut += sprintf(sOut, "%s:\n", sHwm_name);
						sOut += sprintf(sOut, "    Battery-Type    : %s\n", pos->name);
						sOut += sprintf(sOut, "    High-Voltage-Support: %s\n", pos->vendor);
						sOut += sprintf(sOut, "    Bat-25-Capacity: %dmAh\n", pos->id);
						if(strcmp("", pos->more))
							sOut += sprintf(sOut, "    Bat-25-H-Current-Cap  : %s\n", pos->more);
						sOut += sprintf(sOut, "\n");
						HWM_DEBUG("module name is %s: chip: %s, vendor: %s, id:0x %x,\n\t\t more: %s\n", sHwm_name, pos->name, pos->vendor, pos->id, pos->more);
					}else{
						sOut += sprintf(sOut, "%s:\n", sHwm_name);
						sOut += sprintf(sOut, "    chip  : %s\n", pos->name);
						sOut += sprintf(sOut, "    vendor: %s\n", pos->vendor);
						sOut += sprintf(sOut, "    id    : 0x%x\n", pos->id);
						if(strcmp("", pos->more))
							sOut += sprintf(sOut, "    more  : %s\n", pos->more);
						sOut += sprintf(sOut, "\n");
						HWM_DEBUG("module name is %s: chip: %s, vendor: %s, id:0x %x,\n\t\t more: %s\n", sHwm_name, pos->name, pos->vendor, pos->id, pos->more);
					}
			}
			// if we get the especial type, we done and just break.
			if(HW_MODULE_TYPE_ALL != getType)
				break;
	    }
	}
	// recover to default type
	getType = HW_MODULE_TYPE_ALL;

	mutex_unlock(&hw_module_lock);

	if (sOut != buf)
		// convert the last space to a newline
		*(sOut-1) = '\n';

	return (sOut - buf);
}

static ssize_t hw_module_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t size)
{
    #define LEN_INTEGRTER_TO_STRING    (11)

	int temp, retValue = 0;
    hw_module_type type = getType;

    HWM_DEBUG("enter store, size is %d\n", (int)size);

    if((NULL == buf) || (size > LEN_INTEGRTER_TO_STRING)) {
        HWM_DEBUG("parameter error\n");
        return -1;
    }

	if (sscanf(buf, "%d", &temp) == 1) {
        type = (hw_module_type)temp;
        HWM_DEBUG("parameter type is %d\n", type);
	    if((type > HW_MODULE_TYPE_MAX) || (type < HW_MODULE_TYPE_MIN)) {
            HWM_DEBUG("type %d is invalid\n", type);
		    return -1;
        }
	} else {
        HWM_DEBUG("parameter error\n");
        return -1;
    }

	mutex_lock(&hw_module_lock);
	// store the type
	getType = type;
        HWM_DEBUG("save getType to %d\n", getType);
	mutex_unlock(&hw_module_lock);

	return retValue;
}

HWM_INFO_ATTR(hw_module_info);


static struct attribute * g[] = {
	&hw_module_info_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};
#if defined(DROI_PRO_Q1)||defined(DROI_PRO_K8)
bool mipi_switch_enable=0;
static ssize_t mipi_switch_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{

	if(strtobool(buf, &mipi_switch_enable))
		return -EINVAL;

	printk("weixinghai test %d\n",mipi_switch_enable);

	mt_set_gpio_mode(CAM_MAINCAMERA_MIPI_AB, 0);
	mt_set_gpio_dir(CAM_MAINCAMERA_MIPI_AB, GPIO_DIR_OUT);
	mt_set_gpio_out(CAM_MAINCAMERA_MIPI_AB, mipi_switch_enable);


	return count;
}
static struct kobj_attribute mipi_switch_attr = {
    .attr = {
        .name = "mipi_switch",
        .mode = S_IRWXUGO,
    },

    .store = &mipi_switch_store,
};

static struct attribute *properties_attrs[] = {
    &mipi_switch_attr.attr,
    NULL
};
static struct attribute_group properties_attr_group = {
    .attrs = properties_attrs,
};
#elif defined(DROI_PRO_PU6)|| defined(DROI_PRO_PU6T)
bool droi_is_sub2 = 0;
static ssize_t mipi_switch_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
	if(strtobool(buf, &droi_is_sub2))
		return -EINVAL;

	return count;
}
static struct kobj_attribute mipi_switch_attr = {
    .attr = {
        .name = "droi_is_sub2",
        .mode = S_IRWXUGO,
    },

    .store = &mipi_switch_store,
};

static struct attribute *properties_attrs[] = {
    &mipi_switch_attr.attr,
    NULL
};
static struct attribute_group properties_attr_group = {
    .attrs = properties_attrs,
};
#endif

static int __init hw_module_info_init(void)
{
	int ret=0;
	hwm_info_kobj = kobject_create_and_add("hwm_info", NULL);
	if (!hwm_info_kobj)
		return -ENOMEM;
	ret = sysfs_create_group(hwm_info_kobj, &attr_group);
#if defined(DROI_PRO_Q1)||defined(DROI_PRO_K8)|| defined(DROI_PRO_PU6)|| defined(DROI_PRO_PU6T)
	mipi_switch_kobj = kobject_create_and_add("mipi_switch", NULL);
	if (!mipi_switch_kobj)
		return -ENOMEM;
	return sysfs_create_group(mipi_switch_kobj, &properties_attr_group);
#else
	return ret;
#endif
}

//static int __init hw_module_info_exit(void)
static void __init hw_module_info_exit(void)
{
	#define NO_USE 0
	if(NO_USE){
hw_module_info_show( (struct kobject *) 0x100, (struct kobj_attribute *) 0x100, (char *) 0x100 );
hw_module_info_store((struct kobject *) 0x100, (struct kobj_attribute *) 0x100, (char *) 0x100, (size_t ) 0x100);
}
//	return 0;
}


module_init(hw_module_info_init);
module_exit(hw_module_info_exit);

MODULE_AUTHOR("tydtech Inc.");
MODULE_DESCRIPTION("hardware module information driver for tydtech");
MODULE_LICENSE("GPL");
MODULE_ALIAS("hw_module_info");

