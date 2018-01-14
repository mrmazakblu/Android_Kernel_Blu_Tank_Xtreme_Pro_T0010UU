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

struct pinctrl *flashctrl = NULL;
struct pinctrl_state *flash_mode_h = NULL;
struct pinctrl_state *flash_mode_l = NULL;
struct pinctrl_state *flash_en_l = NULL;
struct pinctrl_state *flash_en_h = NULL;
#if defined(TYD_PRO_FQ7M) || defined(TYD_PRO_FQ7_PUB)
struct pinctrl_state *flash_ext_l = NULL;
struct pinctrl_state *flash_ext_h = NULL;
#endif
typedef enum{
	FLASH_MODE,
	FLASH_EN
}Flash_Type;


/*****************************************************************************
flashlight_gpio_init
*****************************************************************************/
int flashlight_gpio_init(struct platform_device *pdev)
{
	int ret = 0;
	PK_DBG("dingxueqi func==%s,line ==%d,,,%s\n",__func__,__LINE__,pdev->name);
	flashctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(flashctrl)) {
		dev_err(&pdev->dev, "Cannot find flashlight pinctrl!");
		ret = PTR_ERR(flashctrl);
	}
    //Cam0 Power/Rst Ping initialization
	flash_mode_h = pinctrl_lookup_state(flashctrl, "flash_mode_1");
	if (IS_ERR(flash_mode_h)) {
		ret = PTR_ERR(flash_mode_h);
		PK_DBG("%s : pinctrl err, flash_mode_h\n", __func__);
	}

	flash_mode_l = pinctrl_lookup_state(flashctrl, "flash_mode_0");
	if (IS_ERR(flash_mode_l)) {
		ret = PTR_ERR(flash_mode_l);
		PK_DBG("%s : pinctrl err, flash_mode_l\n", __func__);
	}


	flash_en_h = pinctrl_lookup_state(flashctrl, "flash_en_1");
	if (IS_ERR(flash_en_h)) {
		ret = PTR_ERR(flash_en_h);
		PK_DBG("%s : pinctrl err, flash_en_h\n", __func__);
	}

	flash_en_l = pinctrl_lookup_state(flashctrl, "flash_en_0");
	if (IS_ERR(flash_en_l)) {
		ret = PTR_ERR(flash_en_l);
		PK_DBG("%s : pinctrl err, flash_en_l\n", __func__);
	}
#if defined(TYD_PRO_FQ7M) || defined(TYD_PRO_FQ7_PUB)
	flash_ext_h = pinctrl_lookup_state(flashctrl, "flash_ext0_1");
	if (IS_ERR(flash_en_h)) {
		ret = PTR_ERR(flash_ext_h);
		PK_DBG("%s : pinctrl err, flash_ext_h\n", __func__);
	}

	flash_ext_l = pinctrl_lookup_state(flashctrl, "flash_ext0_0");
	if (IS_ERR(flash_en_l)) {
		ret = PTR_ERR(flash_ext_l);
		PK_DBG("%s : pinctrl err, flash_ext_l\n", __func__);
	}
#endif
	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);
	return ret;
}

extern struct mutex camera_set_gpio_mutex;
static int flashlight_gpio_set(unsigned long Flash_Type, unsigned long Val)
{
	int ret = 0;
	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);

	mutex_lock(&camera_set_gpio_mutex);//xinhai
	switch (Flash_Type) {
	case FLASH_EN:
		if (Val == 0)
			pinctrl_select_state(flashctrl, flash_en_l);
		else
			pinctrl_select_state(flashctrl, flash_en_h);
		break;
	case FLASH_MODE:
		if (Val == 0)
			pinctrl_select_state(flashctrl, flash_mode_l);
		else
			pinctrl_select_state(flashctrl, flash_mode_h);
		break;
	default:
		PK_DBG("Flash_Type(%ld) is invalid !!\n", Flash_Type);
		break;
	};
	mutex_unlock(&camera_set_gpio_mutex);//xinhai

	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);

	return ret;
}

ssize_t gpio_FL_Init(void)
{
    	PK_DBG("gpio_FL_init\n");
	flashlight_gpio_set(FLASH_MODE,0);  //set torch mode
	flashlight_gpio_set(FLASH_EN,0); //Init. to disable
   	// init_isnk_flashlight();//fake flashlight
	INIT_WORK(&workTimeOut, work_timeOutFunc); // add tyd-rjt
    	return 0;
}

ssize_t gpio_FL_Uninit(void)
{
    	PK_DBG("gpio_FL_uninit\n");
	flashlight_gpio_set(FLASH_EN,0); //Uninit. to disable
    	return 0;
}

ssize_t gpio_FL_Enable(void)
{
    	PK_DBG("gpio_FL_enable\n");
	flashlight_gpio_set(FLASH_EN,1);
    	return 0;

}

ssize_t gpio_FL_Disable(void)
{
	PK_DBG("gpio_FL_disable\n");
	flashlight_gpio_set(FLASH_EN,0);

	flashlight_gpio_set(FLASH_MODE,0);
    	return 0;
}

ssize_t gpio_FL_Dim_duty(kal_uint8 duty)
{
    	PK_DBG("gpio_FL_dim_duty\n");
    	return 0;
}

int FL_Flash_Mode(void)
{
    	PK_DBG("FL_Flash_Mode\n");
	flashlight_gpio_set(FLASH_MODE,1);
	return 0;
}

int FL_Torch_Mode(void)
{
    	PK_DBG("FL_torch_Mode\n");
	flashlight_gpio_set(FLASH_MODE,0);
	return 0;
}

/* abstract interface */
#define FL_Init 	gpio_FL_Init
#define FL_Uninit 	gpio_FL_Uninit
#define FL_Enable 	gpio_FL_Enable
#define FL_Disable 	gpio_FL_Disable
#define FL_dim_duty gpio_FL_dim_duty

/*****************************************************************************
User interface
*****************************************************************************/

static void work_timeOutFunc(struct work_struct *data)
{
    	FL_Disable();
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
	case FLASH_IOC_PRE_ON:
		FL_Torch_Mode();
		break;
    	case FLASH_IOC_SET_DUTY :
    		PK_DBG("FLASHLIGHT_SET_DUTY: %ld\n",arg);
    		//FL_dim_duty(arg);
    		if(arg == 1)  // fix to flashlight for first highlight

			FL_Flash_Mode();
		else
			FL_Torch_Mode();
		//ctl_isnk_flashlight(arg);//ctrl fake flashlight duty
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
    			FL_Enable();
			//enable_isnk_flshlight(true);//enable fake flashlight
    		}
    		else
    		{
			//enable_isnk_flshlight(false);//disable fake flashlight
    			FL_Disable();
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

int sgm3141_flashlight_open(void *pArg)
{
    int i4RetValue = 0;
    PK_DBG("sgm3141_flashlight_open start line=%d\n", __LINE__);

	if (0 == strobe_Res)
	{
	    FL_Init();
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

int sgm3141_flashlight_release(void *pArg)
{
    PK_DBG(" sgm3141_flashlight_release\n");
    if (strobe_Res)
    {
        spin_lock_irq(&g_strobeSMPLock);

        strobe_Res = 0;
        strobe_Timeus = 0;

        g_strobe_On = FALSE;

        spin_unlock_irq(&g_strobeSMPLock);

    	FL_Uninit();
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
