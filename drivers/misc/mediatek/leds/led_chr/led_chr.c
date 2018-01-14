
/*****************************************************************************
* Include
*****************************************************************************/
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

#define TAG_NAME "[leds_chr.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)

#define DEBUG_LEDS_RGB
#ifdef DEBUG_LEDS_RGB
#define PK_DBG PK_DBG_FUNC
#else
#define PK_DBG(a, ...)
#endif

static struct platform_driver led_chr_driver;
struct pinctrl *ledchrctrl = NULL;
struct pinctrl_state *red_light_h = NULL;
struct pinctrl_state *red_light_l = NULL;

//static struct miscdevice led_rgb_device ;
//extern int led_status;
spinlock_t spinlock;
/*
typedef enum
{
	TYDLED_MODE_LED_RGB_POWER= 0,	//no auto
	TYDLED_MODE_LED_RGB_RED_LIGHT_ON,
	TYDLED_MODE_LED_RGB_FACTORY_MODE_TEST,
}TYD_LED_RGB_mode;
*/
//static struct timer_list effect_timer;
//static int startimer_first=1;
//static int timer_exist = 0;
//int led_status = 0;


/*****************************************************************************
flashlight_gpio_init
*****************************************************************************/
int led_rgb_gpio_init(struct platform_device *pdev)
{
	int ret = 0;
	PK_DBG("G_TEST func==%s,line ==%d,,,%s\n",__func__,__LINE__,pdev->name);
	ledchrctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(ledchrctrl)) {
		dev_err(&pdev->dev, "Cannot find flashlight pinctrl!");
		ret = PTR_ERR(ledchrctrl);
	}
        //led red green blue initialization
	red_light_h = pinctrl_lookup_state(ledchrctrl, "led_chr_0");
	if (IS_ERR(red_light_h)) {
		ret = PTR_ERR(red_light_h);
		PK_DBG("%s : pinctrl err, light_h\n", __func__);
	}

	red_light_l = pinctrl_lookup_state(ledchrctrl, "led_chr_1");
	if (IS_ERR(red_light_l)) {
		ret = PTR_ERR(red_light_l);
		PK_DBG("%s : pinctrl err, light_l\n", __func__);
	}

	PK_DBG("G_TEST func==%s,line ==%d\n",__func__,__LINE__);
	return ret;
}

static int led_gpio_set(unsigned long Val)
{
	int ret = 0;
	PK_DBG("G_TEST func==%s,line ==%d\n",__func__,__LINE__);
	if (Val == 1)
		pinctrl_select_state(ledchrctrl, red_light_h);
	else
		pinctrl_select_state(ledchrctrl, red_light_l);
	PK_DBG("G_TEST func==%s,line ==%d\n",__func__,__LINE__);

	return ret;
}

//light on
void led_light_on(void)
{
	led_gpio_set(1);
	udelay(50);
}

//light off
void led_light_off(void)
{
	led_gpio_set(0);
	udelay(50);
}
/*
//factory_mode_test
void factory_mode_test(void)
{
	led_gpio_set(1);
	mdelay(1000);
	led_gpio_set(0);
	udelay(50);
	led_gpio_set(1);
}

static long led_chr_unlocked_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	void __user *argp = (void __user *)arg;

	int mode=-1;

	if (copy_from_user(&mode, argp, sizeof(int)))
	{
		PK_DBG("copy_from_user error\n");
		return -EFAULT;
	}

	PK_DBG("led_chr %s mode:%d\n",__func__,mode);
	PK_DBG("led_chr %s cmd:%d\n",__func__,cmd);


	switch (mode)
	{
		case TYDLED_MODE_LED_RGB_RED_LIGHT_ON:
			led_light_on();
			break;
		case TYDLED_MODE_LED_RGB_FACTORY_MODE_TEST:
			factory_mode_test();
			break;
		default:
			break;

	}

	return 0;
}
*/
static int led_chr_probe(struct platform_device *dev)
{
	PK_DBG("G_TEST %s  line==%d\n",__func__,__LINE__);
	led_rgb_gpio_init(dev);
	return 0;
}

static int led_chr_remove(struct platform_device *dev)
{
	return 0;
}

struct of_device_id led_chr_of_match[] = {
	{ .compatible = "mediatek,mt6580-led-rgb", },
	{},
};

static struct platform_driver led_chr_driver = {
	.probe = led_chr_probe,
	.remove = led_chr_remove,
	.driver = {
			.name = "Led_Chr_Driver",
			.of_match_table = led_chr_of_match,
		   },
};

/*
static const struct file_operations led_chr_fops = {
	.owner = THIS_MODULE,
//	.unlocked_ioctl = led_chr_unlocked_ioctl,
};


static  struct miscdevice led_chr_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "LED_CHR",
	.fops = &led_chr_fops,
};
*/
static int __init led_chr_device_init(void)
{
	int ret = 0;
	PK_DBG("G_TEST %s\n",__func__);
	ret = platform_driver_register(&led_chr_driver);
	if(ret)
	{
		PK_DBG("[led_chr] platform_driver_register error %s\n",__func__);
	}
	else
	{
		PK_DBG("[led_chr] platform_driver_register done! %s\n",__func__);
	}

/*
	if( misc_register(&led_chr_device))
	{
		PK_DBG("led_chr_device can not register\n");
	}
*/
	return 0;
}

late_initcall(led_chr_device_init);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LED_CHR device driver");
MODULE_AUTHOR("Mediatek");
