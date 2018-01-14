/* ah1883.c - ah1883 hall driver
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
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/wakelock.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/virkey.h>
#include <linux/types.h>


#define USE_DELAY_WORK

struct ah1883_data_struct{
	struct input_dev *input_hall;
	struct delayed_work dwork;
	spinlock_t lock;
	bool suspended;
	bool enable;
#ifdef STOP_WAKE_UP_CLOSE_STATUS
	char irq_count;
#endif
	char hall_status;
	struct class *cls;
};
enum{
	HALL_CLOSE,
	HALL_FAR,
};

struct pinctrl *hallctrl = NULL;
struct pinctrl_state *hall_eint = NULL;

static struct ah1883_data_struct ah1883data;
static int hall_setup_eint_trigger_rising_falling(void);

static irqreturn_t ah1883_irq(int irq, void *desc);
struct device_node *hall_irq_node;
static	int irq_num_high=-1;

extern int gpiod_get_value(const struct gpio_desc *desc);
extern struct gpio_desc *gpio_to_desc(unsigned gpio);
extern int gpiod_request(struct gpio_desc *desc, const char *label);
struct gpio_desc *gpio_hall=NULL;


static void ah1883_irq_work(struct work_struct *work)
{

	 ah1883data.hall_status = gpiod_get_value(gpio_hall);
	 //printk(KERN_ERR"dingxueqihall   ah1883data.hall_status= %d\n",ah1883data.hall_status);

	if(ah1883data.hall_status == HALL_CLOSE)
	{
		virkey_report(KEY_RIGHTALT,1,0);
		virkey_report(KEY_RIGHTALT,0,1);
	}
	else if(ah1883data.hall_status == HALL_FAR)
	{
		virkey_report(KEY_LEFTALT,1,0);
		virkey_report(KEY_LEFTALT,0,1);
	}

#ifdef STOP_WAKE_UP_CLOSE_STATUS
	if(ah1883data.irq_count < 3)
		ah1883data.irq_count++;
#endif
};

static irqreturn_t ah1883_irq(int irq, void *desc)
{

	//printk(KERN_ERR"dingxueqihall   ah1883data.enable= %d\n",ah1883data.enable);
	if(ah1883data.enable){
#ifdef USE_DELAY_WORK
		schedule_delayed_work(&ah1883data.dwork,HZ/1000);
#else
		ah1883_irq_work(NULL);
#endif
	}
	return IRQ_HANDLED;
}

/*****************************************************************************
hall_gpio_init
*****************************************************************************/
static int hall_gpio_eint_init(struct platform_device *pdev)
{
	int ret = 0;
	//printk("dingxueqi func==%s,line ==%d,,,%s\n",__func__,__LINE__,pdev->name);
	hallctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(hallctrl)) {
		dev_err(&pdev->dev, "Cannot find hall pinctrl!");
		ret = PTR_ERR(hallctrl);
	}
        //hall pins initialization
	hall_eint = pinctrl_lookup_state(hallctrl, "hall_pins_as_eint");
	if (IS_ERR(hall_eint)) {
		ret = PTR_ERR(hall_eint);
		printk("%s : pinctrl err,hall\n", __func__);
	}

	//printk("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);
	return ret;
}

static int hall_setup_eint_trigger_rising_falling(void)
{
	u32 ints[2] = {0, 0};
/* eint request */
	hall_irq_node= of_find_compatible_node(NULL,NULL, "mediatek,hall");
	if (hall_irq_node) {

		of_property_read_u32_array(hall_irq_node, "debounce", ints, ARRAY_SIZE(ints));
		gpio_set_debounce(ints[0], ints[1]);
		pinctrl_select_state(hallctrl,hall_eint);
		printk("ints[0] = %d, ints[1] = %d!!\n", ints[0], ints[1]);

		irq_num_high = irq_of_parse_and_map(hall_irq_node, 0);
		printk("hall_eint_num = %d\n", irq_num_high);
		if (!irq_num_high) {
			printk("irq_of_parse_and_map fail!!\n");
			return -EINVAL;
		}
		if (request_irq(irq_num_high, ah1883_irq, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "HALL_1-eint", NULL)) {
			printk("IRQ LINE NOT AVAILABLE!!\n");
			return -EINVAL;
		}
		enable_irq(irq_num_high);
	} else {
		printk("null irq node!!\n");
		return -EINVAL;
	}

    return 0;
}

static int ah1883_add_sysfs_interfaces(struct device *dev,
	struct device_attribute *a, int size)
{
	int i;
	for (i = 0; i < size; i++)
		if (device_create_file(dev, a + i))
			goto undo;
	return 0;
undo:
	for (; i >= 0 ; i--)
		device_remove_file(dev, a + i);
	printk(KERN_ERR"%s: failed to create sysfs interface\n", __func__);
	return -ENODEV;
}

static ssize_t show_hall_status(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", ah1883data.hall_status ?"FAR":"CLOSE");
}

static ssize_t show_enable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", ah1883data.enable ?"enable":"disable");
}
static ssize_t store_enable(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	bool value;
	if(strtobool(buf,&value))
		return -EINVAL;
	spin_lock(&ah1883data.lock);
	ah1883data.enable = value;
	spin_unlock(&ah1883data.lock);
	printk("ah1883 enable with %d",value);
	return size;
}
static struct device_attribute ah1883_attrs[] = {
	__ATTR(hall_status, 0664, show_hall_status,NULL),
	__ATTR(enable, 0664, show_enable,store_enable),
};
ssize_t cls_hall_state_show(struct class *class, struct class_attribute *attr,
			char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ah1883data.hall_status);
}
static CLASS_ATTR(state,0664,cls_hall_state_show,NULL);

static int get_hall_dts_func(void)
{
	int hall_gpio_number[]={0};
	int ret=-1,get_hall_gpio_num=-1;
	struct device_node *node = NULL;

	printk("Device Tree get hall info!\n");

	node = of_find_compatible_node(NULL, NULL, "mediatek,ah1883");
	if (node)
	{
		ret = of_property_read_u32_array(node , "hall_gpio_num", hall_gpio_number, ARRAY_SIZE(hall_gpio_number));
		if (ret == 0)
		{
			get_hall_gpio_num=hall_gpio_number[0];
		}

	}
	else
	{
		printk("Device Tree: can not find accel node!. Go to use old cust info\n");
		return -1;
	}

	return get_hall_gpio_num;
}

static int ah1883_probe(struct platform_device *pdev)
{
	int ret,value,gpio_num;
	gpio_num = get_hall_dts_func();
	if (-1==gpio_num)
	{
		return -1;
	}
	//printk("dingxueqihall fucn=%s,gpio_num=%d\n",__func__,gpio_num);
	hall_gpio_eint_init(pdev);
	virkey_register(KEY_LEFTALT);
	virkey_register(KEY_RIGHTALT);
	gpio_hall = gpio_to_desc(gpio_num);
	value=gpiod_request(gpio_hall,"hall");

#ifdef USE_DELAY_WORK
	INIT_DELAYED_WORK(&ah1883data.dwork, ah1883_irq_work);
#endif
#ifdef CONFIG_EARLYSUSPEND
	ah1883data.eshandle.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ah1883data.eshandle.suspend = ah1883_early_suspend;
	ah1883data.eshandle.resume = ah1883_late_resume;
	register_early_suspend(&ah1883data.eshandle);
	ah1883data.suspended = false;
#endif
	spin_lock_init(&ah1883data.lock);
	ah1883data.enable = true;
	ah1883data.hall_status = gpiod_get_value(gpio_hall);

	ret= ah1883_add_sysfs_interfaces(&pdev->dev,
			ah1883_attrs, ARRAY_SIZE(ah1883_attrs));
	if(ret)
		printk(KERN_ERR"fail to register sys interface\n");

	ah1883data.cls = class_create(THIS_MODULE,"hall");
	ret=class_create_file(ah1883data.cls,&class_attr_state);
	if(ret)
		goto fail_r_cls;

	hall_setup_eint_trigger_rising_falling();
	printk("X probe OK");
	return 0;
fail_r_cls:
	class_destroy(ah1883data.cls);
//fail:
	printk("X probe fail");
	return -1;
}
static int ah1883_remove(struct platform_device *pdev)
{

	return 0;
}
struct of_device_id hall_of_match[] = {
	{ .compatible = "mediatek,hall",},
	{},
};

static struct platform_driver ah1883_driver = {
	.probe      = ah1883_probe,
	.remove     = ah1883_remove,
	.driver     = {
		.name  = "ah1883",
		.of_match_table = hall_of_match,
		.owner = THIS_MODULE,
	}
};

static int __init ah1883_init(void)
{
	printk("E");
	if(platform_driver_register(&ah1883_driver)){
		printk(KERN_ERR"failed to register driver,ah1883_alsps_driver\n");
		return -ENODEV;
	}

	return 0;
}
static void __exit ah1883_exit(void)
{
	class_destroy(ah1883data.cls);
	platform_driver_unregister(&ah1883_driver);
}
module_init(ah1883_init);
module_exit(ah1883_exit);

MODULE_AUTHOR("DROI DRIVER");
MODULE_DESCRIPTION("hall sensor driver");
MODULE_LICENSE("GPL");
