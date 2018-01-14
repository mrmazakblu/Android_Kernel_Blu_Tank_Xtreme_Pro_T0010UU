/*
 *
 * HYN CST TouchScreen driver.
 *
 * Copyright (c) 2010-2015, Focaltech Ltd. All rights reserved.
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

#ifndef __LINUX_HYNXXXX_H__
#define __LINUX_HYNXXXX_H__
 /*******************************************************************************
*
* File Name: hyn_cst3_core.h
*
*    Author: QI.WU
*
*   Created: 2015-01-29
*
*  Abstract:
*
* Reference:
*
*******************************************************************************/

/*******************************************************************************
* 1.Included header files
*******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/rtpm_prio.h>
#include <linux/proc_fs.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <asm/uaccess.h>
//#include "mt_boot_common.h"
#include "tpd.h"

#include <linux/hrtimer.h>
#include <linux/string.h>
#include <linux/vmalloc.h>


#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>

#include <linux/version.h>
#include <linux/types.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/mount.h>
#include <linux/unistd.h>
#include <linux/netdevice.h>
#include <../fs/proc/internal.h>

#include <linux/wakelock.h>

#define HYN_GESTURE

#endif
