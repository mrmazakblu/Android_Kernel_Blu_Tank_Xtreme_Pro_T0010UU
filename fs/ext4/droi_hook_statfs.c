/* Copyright Statement:
 *
 * This software/firmware and related documentation ("Droi Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * droi_hook_statfs.c -  driver
 *
 * Author: huangjun <huangjun@droi.com>
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

#ifdef CONFIG_MTK_ENCRYPTION_DEFAULT_ON
#define DM_MAJOR     253
static int dm_major = DM_MAJOR;
#endif

struct hesb {
	struct mutex 	lock;
	uint64_t he_blocks;
	uint64_t bytes_temp;
	uint64_t he_bfree;
	uint64_t bfree_temp;
	int he_bsize;
	char s_id[32];
};

static struct hesb dsb;
static uint64_t rom_total_default = 0;

static ssize_t sb_hook_blocks_store(struct class *class, struct class_attribute *attr,
		const char *buf, size_t count)
{
	ssize_t	ret = -EINVAL;
	if (kstrtoull(buf,10,&dsb.bytes_temp)) {
		return -EINVAL;
	}
    ret = count;
	return ret;
}
static CLASS_ATTR(sba,0220,NULL,sb_hook_blocks_store);

static ssize_t sb_hook_free_blocks_store(struct class *class, struct class_attribute *attr,
		const char *buf, size_t count)
{
	ssize_t	ret = -EINVAL;
	if (kstrtoull(buf,10,&dsb.bfree_temp)) {
		return -EINVAL;
	}
	ret = count;
	return ret;
}
static CLASS_ATTR(sbf,0220,NULL,sb_hook_free_blocks_store);

void droi_hook_blocks(struct dentry *dentry,struct kstatfs *buf)
{
    struct super_block *sb = dentry->d_sb;
    uint64_t byte2block;
    uint64_t k;

#ifdef CONFIG_MTK_ENCRYPTION_DEFAULT_ON
    #if defined(DROI_PRO_F5C_SGDZ3)
        int devno = MKDEV(dm_major, 1);
    #else
        int devno = MKDEV(dm_major, 0);
    #endif
    //printk("%s, sb->s_dev = %d, devno = %d\n", __func__, sb->s_dev, devno);
    if(sb->s_dev == devno){
#else
    #if defined(DROI_PRO_Q1_BPZN)
        strncpy(dsb.s_id,"mmcblk0p22",32);
    #else
        strncpy(dsb.s_id,"mmcblk0p21",32);
    #endif
    //printk("%s, sb->s_id = %s, dsb.s_id = %d\n", __func__, sb->s_id, dsb.s_id);
	if(!strcmp(sb->s_id,dsb.s_id)){
#endif

		mutex_lock(&dsb.lock);
		if(!dsb.he_bsize)
			dsb.he_bsize = buf->f_bsize;
        if (dsb.bytes_temp == 0 && rom_total_default != 0) {
            dsb.bytes_temp = rom_total_default;
        }
        dsb.he_blocks = div_u64(dsb.bytes_temp,dsb.he_bsize);
        //printk("fake, dsb.he_blocks = %llu, buf->f_blocks = %llu\n", dsb.he_blocks, buf->f_blocks);

        byte2block = div_u64(dsb.bfree_temp,dsb.he_bsize);
        if(byte2block <= dsb.he_blocks){
	    	dsb.he_bfree = byte2block;
        }

        if (dsb.he_blocks > buf->f_blocks) {
			k = div_u64(dsb.he_blocks*100,buf->f_blocks);
            buf->f_bfree = div_u64((buf->f_bfree * k),100);

            buf->f_blocks = dsb.he_blocks;
        }
		mutex_unlock(&dsb.lock);
	}
}
EXPORT_SYMBOL_GPL(droi_hook_blocks);

static int __init droi_hook_init(void)
{
	struct class *sbclass;
    int ret;
	mutex_init(&dsb.lock);

	sbclass = class_create(THIS_MODULE,"droi_sb");//
	if(sbclass){
		ret = class_create_file(sbclass,&class_attr_sba);
		ret = class_create_file(sbclass,&class_attr_sbf);
	}
//test
//	dsb.he_blocks +=0x100000;
//	dsb.he_bfree +=0x100000;
//
    ret = kstrtoull(CONFIG_DROI_ROM,10,&rom_total_default);
    //printk("fake, default value ram: %llu", rom_total_default);
    return 0;
}

late_initcall(droi_hook_init);

MODULE_AUTHOR("HUANGJUN");
MODULE_DESCRIPTION("droi hook ext4 statfs call");
MODULE_LICENSE("GPL");
