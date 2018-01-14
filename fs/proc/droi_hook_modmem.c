/* Copyright Statement:
 *
 * This software/firmware and related documentation ("Droi Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * droi_hook_modmem.c -  driver
 *
 * Author: zhanggenjian <zhanggenjian@droi.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/statfs.h>
#include <linux/types.h>
#include <linux/math64.h>

#include "droi_hook_modmem.h"


static struct droiram dram;
static unsigned long ram_total_default;

static ssize_t total_ram_hook_store(struct class *class, struct class_attribute *attr,
        const char *buf, size_t count)
{
    uint64_t value;
    ssize_t ret = -EINVAL;
    if ( kstrtoull(buf,10,&value)) {
        return -EINVAL;
    }
    //printk("fake, total_ram_hook_store, value = %llu\n", value);
    mutex_lock(&dram.lock);
    dram.fake_ram = value;
    mutex_unlock(&dram.lock);
    ret = count;
    return ret;
}
static CLASS_ATTR(droiram_total,0220,NULL,total_ram_hook_store);

static ssize_t used_ram_hook_store(struct class *class, struct class_attribute *attr,
        const char *buf, size_t count)
{
    uint64_t value;
    ssize_t ret = -EINVAL;
    if ( kstrtoull(buf,10,&value)) {
        return -EINVAL;
    }
    mutex_lock(&dram.lock);
    dram.used_ram_plus = value;
    mutex_unlock(&dram.lock);
    ret = count;
    return ret;
}
static CLASS_ATTR(droiram_used,0220,NULL,used_ram_hook_store);

void droi_hook_ram(struct modram *modram)
{
	mutex_lock(&dram.lock);
    if (dram.fake_ram == 0 && ram_total_default != 0) {
        dram.fake_ram = ram_total_default;
    }
    //printk("fake, dram.fake_ram = %lu, ram total_default = %lu\n", dram.fake_ram, ram_total_default);
    modram->fake_ram = dram.fake_ram;
    modram->used_ram_plus = dram.used_ram_plus;
    mutex_unlock(&dram.lock);
}
EXPORT_SYMBOL_GPL(droi_hook_ram);

static int __init droi_hook_init(void)
{
    struct class *memclass;
    int ret;
    mutex_init(&dram.lock);

    memclass = class_create(THIS_MODULE,"droi_meminfo");//
    if(memclass){
        ret = class_create_file(memclass,&class_attr_droiram_total);
        ret = class_create_file(memclass,&class_attr_droiram_used);
    }

    ret = kstrtoul(CONFIG_DROI_RAM,10,&ram_total_default);
    //printk("fake, default value ram: %lu", ram_total_default);

    dram.used_ram_plus = 0;

    return 0;
}

late_initcall(droi_hook_init);

MODULE_AUTHOR("ZHANGGENJIAN");
MODULE_DESCRIPTION("droi hook meminfo call");
MODULE_LICENSE("GPL");
