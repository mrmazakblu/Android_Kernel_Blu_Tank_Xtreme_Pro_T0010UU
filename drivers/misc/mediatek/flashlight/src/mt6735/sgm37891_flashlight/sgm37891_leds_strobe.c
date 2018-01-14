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


#define TAG_NAME "[sgm37891]"
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

static int Flash_Mode = 0;
static int Torch_Mode = 0;
static int Enable_count = 0;

static int g_timeOutTimeMs=0;
static struct work_struct workTimeOut;
//static DEFINE_MUTEX(main_flashlight_mutex);
extern struct mutex main_flashlight_mutex;
static void work_timeOutFunc(struct work_struct *data);

struct pinctrl *flashctrl = NULL;
struct pinctrl_state *flash_on_h = NULL;
struct pinctrl_state *flash_on_l = NULL;
struct pinctrl_state *flash_torch_l = NULL;
struct pinctrl_state *flash_torch_h = NULL;
struct pinctrl_state *flash_1w_l = NULL;
struct pinctrl_state *flash_1w_h = NULL;

typedef enum{
	FLASH_ON,
	FLASH_TORCH,
	FLASH_1W
}Flash_Type;

enum{
	Level0 = 1,
	Level1,
	Level2,
	Level3,
	Level4,
	Level5

};


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
	flash_on_h = pinctrl_lookup_state(flashctrl, "flash_en_1");
	if (IS_ERR(flash_on_h)) {
		ret = PTR_ERR(flash_on_h);
		PK_DBG("%s : pinctrl err, flash_on_h\n", __func__);
	}

	flash_on_l = pinctrl_lookup_state(flashctrl, "flash_en_0");
	if (IS_ERR(flash_on_l)) {
		ret = PTR_ERR(flash_on_l);
		PK_DBG("%s : pinctrl err, flash_on_l\n", __func__);
	}


	flash_torch_h = pinctrl_lookup_state(flashctrl, "flash_mode_1");
	if (IS_ERR(flash_torch_h)) {
		ret = PTR_ERR(flash_torch_h);
		PK_DBG("%s : pinctrl err, flash_torch_h\n", __func__);
	}

	flash_torch_l = pinctrl_lookup_state(flashctrl, "flash_mode_0");
	if (IS_ERR(flash_torch_l)) {
		ret = PTR_ERR(flash_torch_l);
		PK_DBG("%s : pinctrl err, flash_torch_l\n", __func__);
	}

	flash_1w_h = pinctrl_lookup_state(flashctrl, "flash_1w_1");
	if (IS_ERR(flash_1w_h)) {
		ret = PTR_ERR(flash_1w_h);
		PK_DBG("%s : pinctrl err, flash_1w_h\n", __func__);
	}

	flash_1w_l = pinctrl_lookup_state(flashctrl, "flash_1w_0");
	if (IS_ERR(flash_1w_l)) {
		ret = PTR_ERR(flash_1w_l);
		PK_DBG("%s : pinctrl err, flash_1w_l\n", __func__);
	}
	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);
	return ret;
}

ssize_t Flash_1w_sleep(void)
{
	pinctrl_select_state(flashctrl, flash_1w_l);
	udelay(600);

	return 0;
}

ssize_t Flash_1w_level_set(unsigned long level)
{
	pinctrl_select_state(flashctrl, flash_1w_h);
	udelay(200);
	do{
		pinctrl_select_state(flashctrl, flash_1w_l);
		udelay(30);
	    pinctrl_select_state(flashctrl, flash_1w_h);
		udelay(30);
	}while(--level);
	udelay(600);

	return 0;
}

int flashlight_gpio_set(unsigned long Flash_Type, unsigned long Val)
{
	int ret = 0;
	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);
	mutex_lock(&main_flashlight_mutex);

	switch (Flash_Type) {
	case FLASH_ON:
		if (Val == 0)
			pinctrl_select_state(flashctrl, flash_on_l);
		else
			pinctrl_select_state(flashctrl, flash_on_h);
		break;
	case FLASH_TORCH:
		if (Val == 0)
			pinctrl_select_state(flashctrl, flash_torch_l);
		else
			pinctrl_select_state(flashctrl, flash_torch_h);
		break;
	case FLASH_1W:
		if (Val == 0)
			Flash_1w_sleep();
		else
			Flash_1w_level_set(Val);
		break;
	default:
		PK_DBG("Flash_Type(%lu) is invalid !!\n", Flash_Type);
		break;
	};
	mutex_unlock(&main_flashlight_mutex);
	PK_DBG("dingxueqi func==%s,line ==%d\n",__func__,__LINE__);

	return ret;
}

ssize_t gpio_FL_Init(void)
{
    PK_DBG("gpio_FL_init\n");

	mdelay(35);
	//flashlight_gpio_set(FLASH_1W,Level5);//FLASH_1W set Level
	flashlight_gpio_set(FLASH_TORCH,0);
	flashlight_gpio_set(FLASH_ON,0);

	Flash_Mode = 0;
	Torch_Mode = 0;

	INIT_WORK(&workTimeOut, work_timeOutFunc); // add tyd-rjt

    return 0;
}

ssize_t gpio_FL_Uninit(void)
{
    PK_DBG("gpio_FL_uninit\n");

	flashlight_gpio_set(FLASH_TORCH,0);
	flashlight_gpio_set(FLASH_ON,0);
	flashlight_gpio_set(FLASH_1W,0); //FLASH_1W to disable

	Flash_Mode = 0;
	Torch_Mode = 0;

    return 0;
}

ssize_t gpio_FL_Enable(void)
{
    PK_DBG("gpio_FL_enable\n");

	if(!Flash_Mode){
		flashlight_gpio_set(FLASH_TORCH,1);
		flashlight_gpio_set(FLASH_ON,1);
		udelay(15);
		flashlight_gpio_set(FLASH_ON,0);
	}

  	Enable_count = 1;
	return 0;

}

ssize_t gpio_FL_Disable(void)
{
	PK_DBG("gpio_FL_disable\n");

	if(Enable_count){
		if(Torch_Mode){
			flashlight_gpio_set(FLASH_TORCH,0);
			flashlight_gpio_set(FLASH_ON,1);
			udelay(15);
			flashlight_gpio_set(FLASH_ON,0);
		}
		if(Flash_Mode){
			flashlight_gpio_set(FLASH_ON,0);
			Flash_Mode = 0;
		}

		Enable_count = 0;
	}
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

	flashlight_gpio_set(FLASH_ON,1);
	Flash_Mode = 1;
	Torch_Mode = 0;

	return 0;
}

int FL_Torch_Mode(void)
{
    PK_DBG("FL_torch_Mode\n");

	if(!Torch_Mode){
		flashlight_gpio_set(FLASH_1W,Level5);//FLASH_1W set Level
		flashlight_gpio_set(FLASH_ON,0);
		Torch_Mode = 1;
	}

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
#if defined(TYD_PRO_H1_GXI2) || defined(TYD_PRO_H1_GXI) || defined(TYD_PRO_H1_GXI2_OBX)
			FL_Torch_Mode();
#else
    		if(arg == 1)  // fix to flashlight for first highlight

			FL_Flash_Mode();
		else
			FL_Torch_Mode();
#endif
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
