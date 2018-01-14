
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
#include "../../leds/mt6735/leds_sw.h"
#include <mt-plat/mt_pwm.h>
#include <mt-plat/upmu_common.h>
#include <mach/upmu_hw.h>

#define TAG_NAME "[leds_rgb.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)

#define DEBUG_LEDS_RGB
#ifdef DEBUG_LEDS_RGB
#define PK_DBG PK_DBG_FUNC
#else
#define PK_DBG(a, ...)
#endif

static struct platform_driver led_rgb_driver;
struct pinctrl *ledrgbctrl = NULL;
struct pinctrl_state *red_light_h = NULL;
struct pinctrl_state *red_light_l = NULL;
struct pinctrl_state *green_light_l = NULL;
struct pinctrl_state *green_light_h = NULL;
struct pinctrl_state *blue_light_l = NULL;
struct pinctrl_state *blue_light_h = NULL;

typedef enum{
	LED_RGB_RED,
	LED_RGB_GREEN,
	LED_RGB_BLUE
}Led_Rgb_Type;

static struct miscdevice led_rgb_device ;
//extern int led_status;
spinlock_t spinlock;

typedef enum
{
	TYDLED_MODE_LED_RGB_POWER= 0,	//no auto
	TYDLED_MODE_LED_RGB_LIGHT_ON,
	TYDLED_MODE_LED_RGB_RED_LIGHT_ON,
	TYDLED_MODE_LED_RGB_GREEN_LIGHT_ON,
	TYDLED_MODE_LED_RGB_BLUE_LIGHT_ON,
	TYDLED_MODE_LED_RGB_ALL_LIGHT_ON,
	TYDLED_MODE_LED_RGB_FACTORY_MODE_TEST,
}TYD_LED_RGB_mode;

//static struct timer_list effect_timer;
//static int startimer_first=1;
//static int timer_exist = 0;
//int led_status = 0;


/*****************************************************************************
flashlight_gpio_init
*****************************************************************************/
int led_rgb_gpio_init(struct platform_device *pdev)
{
	//int ret = 0;
	PK_DBG("dingxueqi func==%s,line ==%d,,,%s\n",__func__,__LINE__,pdev->name);
	// ISINK0
	pmic_set_register_value(PMIC_RG_DRV_32K_CK_PDN, 0x0);	/* Disable power down */
	pmic_set_register_value(PMIC_RG_DRV_ISINK0_CK_PDN, 0);
	pmic_set_register_value(PMIC_RG_DRV_ISINK0_CK_CKSEL, 0);
	pmic_set_register_value(PMIC_ISINK_CH0_MODE, ISINK_PWM_MODE);
	pmic_set_register_value(PMIC_ISINK_CH0_STEP, ISINK_0);	/* 8mA */
	pmic_set_register_value(PMIC_ISINK_DIM0_DUTY, 31);
	pmic_set_register_value(PMIC_ISINK_DIM0_FSEL, ISINK_1KHZ);	/* 1KHz */

	// ISINK1
	pmic_set_register_value(PMIC_RG_DRV_32K_CK_PDN, 0x0);	/* Disable power down */
	pmic_set_register_value(PMIC_RG_DRV_ISINK1_CK_PDN, 0);
	pmic_set_register_value(PMIC_RG_DRV_ISINK1_CK_CKSEL, 0);
	pmic_set_register_value(PMIC_ISINK_CH1_MODE, ISINK_PWM_MODE);
	pmic_set_register_value(PMIC_ISINK_CH1_STEP, ISINK_0);	/* 8mA */
	pmic_set_register_value(PMIC_ISINK_DIM1_DUTY, 31);
	pmic_set_register_value(PMIC_ISINK_DIM1_FSEL, ISINK_1KHZ);	/* 1KHz */

	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);
	return 0;
}

static int led_rgb_gpio_set(unsigned long Led_Rgb_Type, unsigned long Val)
{
	int ret = 0;
	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);
	switch (Led_Rgb_Type) {
	case LED_RGB_RED:
		if (Val == 1) {
			//ISINK0
			pmic_set_register_value(PMIC_ISINK_CH0_EN, 1);
		} else {
			//ISINK0
			pmic_set_register_value(PMIC_ISINK_CH0_EN, 0);
		}
		break;
	case LED_RGB_GREEN:
		if (Val == 1) {
			//ISINK1
			pmic_set_register_value(PMIC_ISINK_CH1_EN, 1);
		} else {
			//ISINK1
			pmic_set_register_value(PMIC_ISINK_CH1_EN, 0);
		}
		break;
	case LED_RGB_BLUE:
		if (Val == 1) {
			//ISINK0
			pmic_set_register_value(PMIC_ISINK_CH0_EN, 1);
		} else {
			//ISINK0
			pmic_set_register_value(PMIC_ISINK_CH0_EN, 0);
		}
		break;
	default:
		PK_DBG("Flash_Type(%ld) is invalid !!\n", Led_Rgb_Type);
		break;
	};

	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);

	return ret;
}


/*
void fun_timer_start(int delay,int fun)
{
	if (!led_status)
		return;

	if(startimer_first)
	{
		startimer_first=0;
		init_timer(&effect_timer);
	}

	if(timer_exist)
	{
		mod_timer(&effect_timer,jiffies+delay*HZ/1000);
	}
	else
	{
		effect_timer.expires = jiffies + delay*HZ/1000;
		effect_timer.function = fun;
		add_timer(&effect_timer);
		timer_exist =1;
	}
}

void fun_timer_stop(void)
{
	if(!timer_exist || startimer_first)
		return;
	mod_timer(&effect_timer,jiffies + 0*HZ);
	del_timer_sync(&effect_timer);
	timer_exist = 0;
}
*/


//red light on
void red_light_on(void)
{
	led_rgb_gpio_set(LED_RGB_RED,1);
	udelay(50);
}

//red light off
void red_light_off(void)
{
	led_rgb_gpio_set(LED_RGB_RED,0);
	udelay(50);
}

//green light on
void green_light_on(void)
{
	led_rgb_gpio_set(LED_RGB_GREEN,1);
	udelay(50);
}

//green light off
void green_light_off(void)
{
	led_rgb_gpio_set(LED_RGB_GREEN,0);
	udelay(50);
}

//blue light on
void blue_light_on(void)
{
	led_rgb_gpio_set(LED_RGB_BLUE,1);
	udelay(50);
}

//blue light off
void blue_light_off(void)
{
	led_rgb_gpio_set(LED_RGB_BLUE,0);
	udelay(50);
}



//All light on
void all_light_on(void)
{
	led_rgb_gpio_set(LED_RGB_RED,1);
	udelay(50);

	led_rgb_gpio_set(LED_RGB_GREEN,1);
	udelay(50);

	led_rgb_gpio_set(LED_RGB_BLUE,1);
	udelay(50);
}

//All light off
void all_light_off(void)
{

	led_rgb_gpio_set(LED_RGB_RED,0);
	udelay(50);
	led_rgb_gpio_set(LED_RGB_GREEN,0);
	udelay(50);
	led_rgb_gpio_set(LED_RGB_BLUE,0);
	udelay(50);

}

//factory_mode_test
void factory_mode_test(void)
{
	led_rgb_gpio_set(LED_RGB_RED,1);// output RED
	mdelay(1000);
	led_rgb_gpio_set(LED_RGB_RED,0);
	udelay(50);

	led_rgb_gpio_set(LED_RGB_GREEN,1);// output green
	mdelay(1000);
	led_rgb_gpio_set(LED_RGB_GREEN,0);
	udelay(50);

    //*/ Modified begin by droi xupeng 20160311, add led control feature
	led_rgb_gpio_set(LED_RGB_BLUE,0);// output blue
	mdelay(1000);
	led_rgb_gpio_set(LED_RGB_BLUE,1);
	udelay(50);
    //*/ Modified end
}

static long led_rgb_unlocked_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	void __user *argp = (void __user *)arg;

	int mode=-1;

	if (copy_from_user(&mode, argp, sizeof(int)))
	{
		PK_DBG("copy_from_user error\n");
		return -EFAULT;
	}

	PK_DBG("led_rgb %s mode:%d\n",__func__,mode);
	PK_DBG("led_rgb %s cmd:%d\n",__func__,cmd);


	switch (mode)
	{
		case 0:
			blue_light_off();
			break;
		case TYDLED_MODE_LED_RGB_LIGHT_ON:
			blue_light_off();
			all_light_on();
			break;
		case TYDLED_MODE_LED_RGB_RED_LIGHT_ON:
			red_light_on();
			break;
		case TYDLED_MODE_LED_RGB_GREEN_LIGHT_ON:
			green_light_on();
			break;
		case TYDLED_MODE_LED_RGB_BLUE_LIGHT_ON:
			blue_light_on();
			break;
		case TYDLED_MODE_LED_RGB_ALL_LIGHT_ON:
			all_light_on();
			break;
		case TYDLED_MODE_LED_RGB_FACTORY_MODE_TEST:
			factory_mode_test();
			break;
		default:
			blue_light_off();

	}

	return 0;
}

static int led_rgb_probe(struct platform_device *dev)
{
	PK_DBG("dingxueqi %s  line==%d\n",__func__,__LINE__);
	led_rgb_gpio_init(dev);
	return 0;
}

static int led_rgb_remove(struct platform_device *dev)
{
	return 0;
}

struct of_device_id led_rgb_of_match[] = {
	{ .compatible = "mediatek,mt6735-led-rgb", },
	{},
};

static struct platform_driver led_rgb_driver = {
	.probe = led_rgb_probe,
	.remove = led_rgb_remove,
	.driver = {
			.name = "Led_rgb_Driver",
			.of_match_table = led_rgb_of_match,
		   },
};


static const struct file_operations led_rgb_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = led_rgb_unlocked_ioctl,
};


static  struct miscdevice led_rgb_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "TYD_LED",
	.fops = &led_rgb_fops,
};

static int __init led_rgb_device_init(void)
{
	int ret = 0;
	PK_DBG("dingxueqi %s\n",__func__);
	ret = platform_driver_register(&led_rgb_driver);
	if(ret)
	{
		PK_DBG("[led_rgb] platform_driver_register error %s\n",__func__);
	}
	else
	{
		PK_DBG("[led_rgb] platform_driver_register done! %s\n",__func__);
	}


	if( misc_register(&led_rgb_device))
	{
		PK_DBG("led_rgb_device can not register\n");
	}

	return 0;
}

late_initcall(led_rgb_device_init);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LED_RGB device driver");
MODULE_AUTHOR("Mediatek");
