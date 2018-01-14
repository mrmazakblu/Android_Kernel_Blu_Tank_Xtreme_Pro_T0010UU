/* camflashlight.c - camflashlight hall driver
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

/*****************************************************************************
* Include
*****************************************************************************/
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/kthread.h>

#define TAG_NAME "[message_flashlight.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)

#define DEBUG_LEDS_RGB
#ifdef 	DEBUG_LEDS_RGB
#define PK_DBG PK_DBG_FUNC
#else
#define PK_DBG(a, ...)
#endif

static bool	message_en=0;

static struct task_struct *tsk=NULL;
#include <mt-plat/battery_common.h>
extern kal_bool bat_is_charger_exist(void);
extern PMU_ChargerStruct BMT_status;
extern int sgm3141_flashlight_open(void *pArg);
extern ssize_t gpio_FL_Disable(void);
extern ssize_t gpio_FL_Enable(void);
extern int FL_Torch_Mode(void);
extern int sgm3141_flashlight_release(void *pArg);
int flashlight_open=0;
static int message_flashlight_thread(void *data)
{

	if((bat_is_charger_exist()==1)||(BMT_status.UI_SOC2>15))
	{

		do{
			if(message_en == true){
				if(flashlight_open!=2){
					flashlight_open=sgm3141_flashlight_open(NULL);
					if(flashlight_open<0){
						mdelay(1000);
						continue;
					}
					else{
						flashlight_open=2;
						FL_Torch_Mode();
					}
				}
				else{
					gpio_FL_Enable();
					mdelay(1000);
					gpio_FL_Disable();
					mdelay(1000);
				}

			}
			else
				mdelay(1000);
		}while(!kthread_should_stop());
    }
	else
	{
		do{
			mdelay(1000);
		}while(!kthread_should_stop());
	}
	return 0;
}

static ssize_t message_mode_show(struct class *class, struct class_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", message_en == true ? "enable" : "disable");
}

static ssize_t message_mode_store(struct class *class, struct class_attribute *attr,const char *buf, size_t count)
{
	bool value;
	if(strtobool(buf, &value))
		return -EINVAL;

	printk("test %d\n",value);

	if(value)
		{
			message_en = true;
			tsk = kthread_run(message_flashlight_thread, NULL, "message_flashlight_thread");
		}
	else
		{
			message_en = false;
			if(tsk!=NULL){
				kthread_stop(tsk);
				if(flashlight_open==2){
					sgm3141_flashlight_release(NULL);
					flashlight_open=0;
				}
				tsk=NULL;
			}
		}

	printk(KERN_ERR"test message_en%d\n",message_en);
	return count;
}

static struct class_attribute message_attr[]={

    __ATTR(camflashlighten, 0664, message_mode_show, message_mode_store),

};


static int message_flashlight_init(void)
{
	struct class *cls;
	int retval;
	cls = class_create(THIS_MODULE,"camflashlight");
	if(cls){
		retval=class_create_file(cls,message_attr);
		if(retval)
		PK_DBG("message_flashlight_attr can not register\n");
	}
	return 0;
}
static void message_flashlight_exit(void)
{
 	PK_DBG("message_flashlight_exit\n");
}

module_init(message_flashlight_init);
module_exit(message_flashlight_exit);

MODULE_AUTHOR("weixinghai");
MODULE_DESCRIPTION("message Led Driver");
MODULE_LICENSE("GPL");
