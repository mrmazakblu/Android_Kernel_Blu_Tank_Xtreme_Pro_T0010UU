/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
//#include "kd_camera_hw.h"
//#include <cust_gpio_usage.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
//#include <linux/xlog.h>
#include <linux/version.h>
#include "leds_sw.h"
#include <mt-plat/mt_pwm.h>
#include <mt-plat/upmu_common.h>
#include <mach/upmu_hw.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
#include <linux/mutex.h>
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif
#endif

/******************************************************************************
 * Debug configuration
******************************************************************************/

#define TAG_NAME "[sub_strobe.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)

#define DEBUG_LEDS_STROBE
#ifdef DEBUG_LEDS_STROBE
#define PK_DBG PK_DBG_FUNC
#else
#define PK_DBG(a, ...)
#endif

#if defined(DROI_PRO_K8)
	#include <mach/gpio_const.h>
	#include <mt-plat/mt_gpio.h>
	#define SUB_FLASHLIGHT_USE_GPIO
	#define GPIO_SUB_FLASHLIGHT  (GPIO95 | 0x80000000)
#endif

static u32 strobe_Timeus = 0;
static DEFINE_SPINLOCK(g_strobeSMPLock); /* cotta-- SMP proection */
static u32 strobe_Res = 0;
static struct hrtimer g_timeOutTimer;
static int g_timeOutTimeMs=0;
static int g_duty=-1;
static struct work_struct workTimeOut;

static enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
    schedule_work(&workTimeOut);
    return HRTIMER_NORESTART;
}

static void timerInit(void)
{
    g_timeOutTimeMs=1000; //1s
    hrtimer_init( &g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
    g_timeOutTimer.function=ledTimeOutCallback;

}
static int FL_Init(void)
{
	printk("flashlight init\n");
    /*Init. to disable*/

#if defined(SUB_FLASHLIGHT_USE_GPIO)
	mt_set_gpio_mode(GPIO_SUB_FLASHLIGHT,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_SUB_FLASHLIGHT,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_SUB_FLASHLIGHT,GPIO_OUT_ZERO);
#else
	// ISINK0
	pmic_set_register_value(PMIC_RG_DRV_32K_CK_PDN, 0x0);	/* Disable power down */
	pmic_set_register_value(PMIC_RG_DRV_ISINK0_CK_PDN, 0);
	pmic_set_register_value(PMIC_RG_DRV_ISINK0_CK_CKSEL, 0);
	pmic_set_register_value(PMIC_ISINK_CH0_MODE, ISINK_PWM_MODE);
	pmic_set_register_value(PMIC_ISINK_CH0_STEP, ISINK_5);	/* 16mA */
	pmic_set_register_value(PMIC_ISINK_DIM0_DUTY, 31);
	pmic_set_register_value(PMIC_ISINK_DIM0_FSEL, ISINK_1KHZ);	/* 1KHz */

	// ISINK1

	pmic_set_register_value(PMIC_RG_DRV_32K_CK_PDN, 0x0);	/* Disable power down */
	pmic_set_register_value(PMIC_RG_DRV_ISINK1_CK_PDN, 0);
	pmic_set_register_value(PMIC_RG_DRV_ISINK1_CK_CKSEL, 0);
	pmic_set_register_value(PMIC_ISINK_CH1_MODE, ISINK_PWM_MODE);
	pmic_set_register_value(PMIC_ISINK_CH1_STEP, ISINK_5);	/* 16mA */
	pmic_set_register_value(PMIC_ISINK_DIM1_DUTY, 31);
	pmic_set_register_value(PMIC_ISINK_DIM1_FSEL, ISINK_1KHZ);	/* 1KHz */
#endif
    return 0;
}

static int FL_Disable(void)
{
#if defined(SUB_FLASHLIGHT_USE_GPIO)
	mt_set_gpio_mode(GPIO_SUB_FLASHLIGHT,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_SUB_FLASHLIGHT,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_SUB_FLASHLIGHT,GPIO_OUT_ZERO);
#else
	//ISINK0
	pmic_set_register_value(PMIC_ISINK_CH0_EN,0);
	//ISINK1
	pmic_set_register_value(PMIC_ISINK_CH1_EN,0);
#endif

    PK_DBG(" FL_Disable line=%d\n",__LINE__);
    return 0;
}

static int FL_Enable(void)
{
	printk("flashlight enable\n");
#if defined(SUB_FLASHLIGHT_USE_GPIO)
	mt_set_gpio_mode(GPIO_SUB_FLASHLIGHT,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_SUB_FLASHLIGHT,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_SUB_FLASHLIGHT,GPIO_OUT_ONE);
#else
    if (g_duty==0)
    {
        //ISINK0
        pmic_set_register_value(PMIC_RG_ISINK0_DOUBLE_EN,0x1);
	    pmic_set_register_value(PMIC_ISINK_CH0_EN,1);
        //ISINK1
        pmic_set_register_value(PMIC_RG_ISINK1_DOUBLE_EN,0x1);
	    pmic_set_register_value(PMIC_ISINK_CH1_EN,1);
    }
    else
    {
        //ISINK0
	    pmic_set_register_value(PMIC_RG_ISINK0_DOUBLE_EN,0x1);
	    pmic_set_register_value(PMIC_ISINK_CH0_EN,1);
	    //ISINK1
	    pmic_set_register_value(PMIC_RG_ISINK1_DOUBLE_EN,0x1);
	    pmic_set_register_value(PMIC_ISINK_CH1_EN,1);
    }
#endif

    return 0;
}

static int FL_dim_duty(MUINT32 duty)
{
    g_duty=duty;
    PK_DBG(" FL_dim_duty line=%d\n",__LINE__);
    return 0;
}

static int FL_Uninit(void)
{
    FL_Disable();
    return 0;
}

static int sub_strobe_ioctl(unsigned int cmd, unsigned long arg)
{

    int i4RetValue = 0;
    int ior_shift;
    int iow_shift;
    int iowr_shift;
	int iFlashType = (int)FLASHLIGHT_NONE;
    ior_shift = cmd - (_IOR(FLASHLIGHT_MAGIC,0, int));
    iow_shift = cmd - (_IOW(FLASHLIGHT_MAGIC,0, int));
    iowr_shift = cmd - (_IOWR(FLASHLIGHT_MAGIC,0, int));
    PK_DBG("constant_flashlight_ioctl() line=%d ior_shift=%d, iow_shift=%d iowr_shift=%d arg=%ld\n",__LINE__, ior_shift, iow_shift, iowr_shift, arg);
    switch (cmd)
    {

    case FLASH_IOC_SET_TIME_OUT_TIME_MS:
        PK_DBG("FLASH_IOC_SET_TIME_OUT_TIME_MS: %ld\n",arg);
        g_timeOutTimeMs=arg;
        break;


    case FLASH_IOC_SET_DUTY :
        PK_DBG("FLASHLIGHT_DUTY: %ld\n",arg);
        FL_dim_duty(arg);
        break;


    case FLASH_IOC_SET_STEP:
        PK_DBG("FLASH_IOC_SET_STEP: %ld\n",arg);

        break;

    case FLASH_IOC_SET_ONOFF :
        PK_DBG("FLASHLIGHT_ONOFF: %ld\n",arg);
		printk("songqi:flashlight open\n");
		//FL_Enable();
        if (arg==1)
        {
            if (g_timeOutTimeMs!=0)
            {
                ktime_t ktime;
                ktime = ktime_set( 0, g_timeOutTimeMs*1000000 );
                hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );
            }
            FL_Enable();
        }
        else
        {
			//FL_Enable();
            FL_Disable();
            hrtimer_cancel( &g_timeOutTimer );
        }
        break;
		case FLASHLIGHTIOC_G_FLASHTYPE:
			spin_lock(&g_strobeSMPLock); /* cotta-- SMP proection */
			iFlashType = FLASHLIGHT_LED_CONSTANT;
			spin_unlock(&g_strobeSMPLock);
			if(copy_to_user((void __user *) (unsigned long)arg , (void*)&iFlashType , _IOC_SIZE(cmd)))
			{
			PK_DBG(" ioctl copy to user failed\n");
			return -EFAULT;
			}
		break;
    default :
        PK_DBG(" No such command \n");
        i4RetValue = -EPERM;
        break;
    }
    return i4RetValue;

}

static int sub_strobe_open(void *pArg)
{
	int i4RetValue = 0;
	printk("songqi:flashlight open\n");

    PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

    if (0 == strobe_Res)
    {
        FL_Init();
        timerInit();
    }
    PK_DBG("constant_flashlight_open line=%d\n", __LINE__);
    spin_lock_irq(&g_strobeSMPLock);


    if (strobe_Res)
    {
        PK_DBG(" busy!\n");
        i4RetValue = -EBUSY;
    }
    else
    {
        strobe_Res += 1;
    }


    spin_unlock_irq(&g_strobeSMPLock);
    PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

    return i4RetValue;

}

static int sub_strobe_release(void *pArg)
{

  PK_DBG(" constant_flashlight_release\n");

    if (strobe_Res)
    {
        spin_lock_irq(&g_strobeSMPLock);

        strobe_Res = 0;
        strobe_Timeus = 0;

        /* LED On Status */
       // g_strobe_On = FALSE;

        spin_unlock_irq(&g_strobeSMPLock);

        FL_Uninit();
    }

    PK_DBG(" Done\n");

    return 0;

}

FLASHLIGHT_FUNCTION_STRUCT	subStrobeFunc=
{
	sub_strobe_open,
	sub_strobe_release,
	sub_strobe_ioctl
};


MUINT32 subStrobeInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc != NULL)
    {
        *pfFunc = &subStrobeFunc;
    }
    return 0;
}





