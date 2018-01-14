/* virkey.c - ah1883 hall driver
 * version 1.0
 * Author: huangjun <huangjun@tydtech.com>
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
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
//#include <linux/earlysuspend.h>
#include <linux/suspend.h>
#include <linux/virkey.h>
#include <linux/module.h>

//#include <mach/mt_sleep.h>

//#define HDEBUG
#ifdef HDEBUG
#define __MODE__ "VIRKEY"
#define h_debug(fmt,arg...) printk(KERN_ERR"\n[ #h#j# (%s)::%s ]"fmt,__MODE__,__func__,##arg)
#else
#define h_debug(fmt,arg...)
#endif

#define DEVICE_NAME "VIRKEY"
#undef CONFIG_EARLYSUSPEND
struct virkey_data{
	struct input_dev *input_virkey;
	struct early_suspend eshandle;
};

static struct virkey_data vkd;
void virkey_register(int keycode)
{
	set_bit(keycode,vkd.input_virkey->keybit);
}
EXPORT_SYMBOL_GPL(virkey_register);
void virkey_arr_register(int *keycode,int size)
{
	int i=size;
	h_debug("X");
	while(i--){
		set_bit(keycode[i],vkd.input_virkey->keybit);
		h_debug("key=%d\n",keycode[i]);
	}
	h_debug("E");
}
EXPORT_SYMBOL_GPL(virkey_arr_register);
void virkey_report(int keycode,int value,bool sync)
{
	input_report_key(vkd.input_virkey,keycode,value);
	if(sync)
		input_sync(vkd.input_virkey);
}
EXPORT_SYMBOL_GPL(virkey_report);

#ifdef CONFIG_EARLYSUSPEND
void virkey_early_suspend(struct early_suspend *h)
{
}

void virkey_late_resume(struct early_suspend *h)
{
}
#endif
static int virkey_probe(struct platform_device *pdev)
{
	int ret;
	h_debug("E");

	vkd.input_virkey = input_allocate_device();
	if (!vkd.input_virkey) {
		printk(KERN_ERR"%s: no memory for input_dev '%s'\n",
				__func__, DEVICE_NAME);
		ret = -ENODEV;
		goto fail;
	}
	vkd.input_virkey->name = DEVICE_NAME;
	vkd.input_virkey->id.bustype = BUS_HOST;
	vkd.input_virkey->dev.parent = &pdev->dev;

	set_bit(EV_KEY, vkd.input_virkey->evbit);
//	set_bit(KEY_POWER, vkd.input_virkey->keybit);
	ret = input_register_device(vkd.input_virkey);
	if (ret) {
		input_free_device(vkd.input_virkey);
		printk(KERN_ERR"%s: cant register input '%s'\n",
				__func__, DEVICE_NAME);
		goto fail;
	}
#ifdef CONFIG_EARLYSUSPEND
	vkd.eshandle.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	vkd.eshandle.suspend = virkey_early_suspend;
	vkd.eshandle.resume = virkey_late_resume;
	register_early_suspend(&vkd.eshandle);
#endif
	h_debug("X probe OK");
	return 0;
fail:
	h_debug("X probe fail");
	return -1;
}
static int virkey_remove(struct platform_device *pdev)
{

	input_free_device(vkd.input_virkey);
	return 0;
}
static struct platform_driver virkey_driver = {
	.probe      = virkey_probe,
	.remove     = virkey_remove,
	.driver     = {
		.name  = DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};
static struct platform_device virkey_dev = {
	.name	       = DEVICE_NAME,
	.id            = -1,
};
static int __init virkey_init(void)
{
	h_debug("E");
	if(platform_driver_register(&virkey_driver)){
		printk(KERN_ERR"failed to register driver,virkey_driver\n");
		return -ENODEV;
	}
	if(platform_device_register(&virkey_dev)){
		printk(KERN_ERR"fail registor dev hall,virkey\n");
		return -ENODEV;
	}
	h_debug("X");
	return 0;
}
static void __exit virkey_exit(void)
{
	platform_driver_unregister(&virkey_driver);
	platform_device_unregister(&virkey_dev);
}
module_init(virkey_init);
module_exit(virkey_exit);

MODULE_AUTHOR("huangjun");
MODULE_DESCRIPTION("virtualkey driver");
MODULE_LICENSE("GPL2");
