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
#include "kd_camera_typedef.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/leds.h>


#define TAG_NAME "[leds_strobe.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)

#define DEBUG_LEDS_STROBE
#ifdef DEBUG_LEDS_STROBE
#define PK_DBG PK_DBG_FUNC
#else
#define PK_DBG(a, ...)
#endif

/******************************************************************************
 * local variables
******************************************************************************/

static DEFINE_SPINLOCK(g_strobeSMPLock);	/* cotta-- SMP proection */
static u32 strobe_Res = 0;
static u32 strobe_Timeus = 0;
static BOOL g_strobe_On = 0;

static int g_timeOutTimeMs=0;
static struct work_struct workTimeOut;

static void work_timeOutFunc(struct work_struct *data);


extern void init_isnk_flashlight(void);
extern void enable_isnk_flshlight(bool b);
extern void ctl_isnk_flashlight(int duty);

/*****************************************************************************
User interface
*****************************************************************************/

static void work_timeOutFunc(struct work_struct *data)
{
    	PK_DBG("ledTimeOut_callback\n");
}

enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
    	schedule_work(&workTimeOut);
    	return HRTIMER_NORESTART;
}
static struct hrtimer g_timeOutTimer;
void timerInit(void)
{
	g_timeOutTimeMs=1000; //1s
	hrtimer_init( &g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	g_timeOutTimer.function=ledTimeOutCallback;
}

static int sgm3141_flashlight_ioctl(unsigned int cmd, unsigned long arg)
{
	int i4RetValue = 0;
	int ior_shift;
	int iow_shift;
	int iowr_shift;
	ior_shift = cmd - (_IOR(FLASHLIGHT_MAGIC,0, int));
	iow_shift = cmd - (_IOW(FLASHLIGHT_MAGIC,0, int));
	iowr_shift = cmd - (_IOWR(FLASHLIGHT_MAGIC,0, int));
	PK_DBG("sgm3141_flashlight_ioctl() line=%d ior_shift=%d, iow_shift=%d iowr_shift=%d arg=%ld\n",__LINE__, ior_shift, iow_shift, iowr_shift, arg);
    switch(cmd)
    {
	case FLASH_IOC_SET_TIME_OUT_TIME_MS:
		PK_DBG("FLASH_IOC_SET_TIME_OUT_TIME_MS: %ld\n",arg);
		g_timeOutTimeMs=arg;
		break;

    	case FLASH_IOC_SET_DUTY :
    		PK_DBG("FLASHLIGHT_SET_DUTY: %ld\n",arg);
			ctl_isnk_flashlight(arg);//ctrl fake flashlight duty
    		break;

    	case FLASH_IOC_SET_STEP:
    		PK_DBG("FLASH_IOC_SET_STEP: %ld\n",arg);
    		break;

    	case FLASH_IOC_SET_ONOFF :
    		PK_DBG("FLASHLIGHT_ONOFF: %ld\n",arg);
    		if(arg==1)
    		{
			if(g_timeOutTimeMs!=0)
	            	{
	            		ktime_t ktime;
				ktime = ktime_set( 0, g_timeOutTimeMs*1000000 );
				hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );
	            	}
				enable_isnk_flshlight(true);//enable fake flashlight
    		}
    		else
    		{
				enable_isnk_flshlight(false);//disable fake flashlight
				hrtimer_cancel( &g_timeOutTimer );
    		}
    		break;

	case FLASHLIGHTIOC_G_FLASHTYPE:
		//copy_to_user(arg,(void*)&flashtype,sizeof(int));
		break;
	case FLASHLIGHTIOC_T_STATE:
		break;
	default :
    		PK_DBG(" No such command default\n");
    		i4RetValue = -EPERM;
    		break;
    }
    return i4RetValue;
}

static int sgm3141_flashlight_open(void *pArg)
{
    int i4RetValue = 0;
    PK_DBG("sgm3141_flashlight_open start line=%d\n", __LINE__);

	if (0 == strobe_Res)
	{
	    init_isnk_flashlight();//fake flashlight
		INIT_WORK(&workTimeOut, work_timeOutFunc); // add tyd-rjt
	    timerInit();
	}
	PK_DBG("sgm3141_flashlight_open line=%d\n", __LINE__);
	spin_lock_irq(&g_strobeSMPLock);

    if(strobe_Res)
    {
        PK_DBG(" busy!\n");
        i4RetValue = -EBUSY;
    }
    else
    {
        strobe_Res += 1;
    }

    spin_unlock_irq(&g_strobeSMPLock);
    PK_DBG("sgm3141_flashlight_open end line=%d\n", __LINE__);
    return i4RetValue;
}

static int sgm3141_flashlight_release(void *pArg)
{
    PK_DBG(" sgm3141_flashlight_release\n");
    if (strobe_Res)
    {
        spin_lock_irq(&g_strobeSMPLock);

        strobe_Res = 0;
        strobe_Timeus = 0;

        g_strobe_On = FALSE;

        spin_unlock_irq(&g_strobeSMPLock);
    }
    PK_DBG("sgm3141_flashlight_release end\n");
    return 0;
}

FLASHLIGHT_FUNCTION_STRUCT	constantFlashlightFunc=
{
	sgm3141_flashlight_open,
	sgm3141_flashlight_release,
	sgm3141_flashlight_ioctl
};

MUINT32 constantFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc != NULL)
    {
        *pfFunc = &constantFlashlightFunc;
    }
    return 0;
}

/* LED flash control for high current capture mode*/
ssize_t strobe_VDIrq(void)
{
    return 0;
}

EXPORT_SYMBOL(strobe_VDIrq);
