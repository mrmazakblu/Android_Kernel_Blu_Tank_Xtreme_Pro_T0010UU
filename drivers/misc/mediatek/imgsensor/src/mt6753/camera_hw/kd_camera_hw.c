#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include "kd_camera_hw.h"
/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, args...)    pr_debug(PFX  fmt, ##args)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG(fmt, args...) pr_debug(PFX  fmt, ##args)
#define PK_ERR(fmt, arg...)         pr_err(fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...)  pr_debug(PFX  fmt, ##args);
#else
#define PK_DBG(a, ...)
#define PK_ERR(a, ...)
#define PK_XLOG_INFO(fmt, args...)
#endif

#define IDX_PS_ON   2
#define IDX_PS_OFF  3

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4

u32 pinSetIdx = 0;//default main sensor
u32 pinSet[3][8] = {
	//for main sensor
	{
		CAMERA_CMRST_PIN,
		CAMERA_CMRST_PIN_M_GPIO,   /* mode */
		GPIO_OUT_ONE,              /* ON state */
		GPIO_OUT_ZERO,             /* OFF state */
		CAMERA_CMPDN_PIN,
		CAMERA_CMPDN_PIN_M_GPIO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,
	},
	//for sub sensor
	{
		CAMERA_CMRST1_PIN,
		CAMERA_CMRST1_PIN_M_GPIO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,
		CAMERA_CMPDN1_PIN,
		CAMERA_CMPDN1_PIN_M_GPIO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,
	},
};

PowerCust PowerCustList={
	{
		{GPIO_UNSUPPORTED,Vol_Low},   //for VCAMA;
		{GPIO_UNSUPPORTED,Vol_Low},   //for VCAMD;
		{GPIO_UNSUPPORTED,Vol_Low},   //for VCAMIO;
		{GPIO_UNSUPPORTED,Vol_Low},   //for VCAMAF;
		{GPIO_UNSUPPORTED,Vol_Low},   //for AFEN;
	}
};

PowerUp PowerOnList={
	{
//SUB DVDD MAX 1.5V
		{SENSOR_DRVNAME_GC5005MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 5},
				{VCAMA,  Vol_2800, 5},
				{VCAMD,  Vol_1200, 5},
				{VCAMAF, Vol_2800, 5},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_High, 5},
				{PDN,   Vol_Low,  5},
				{RST,   Vol_Low,  5},
				{RST,   Vol_High, 5}
			},
		},
		{SENSOR_DRVNAME_IMX214_MIPI_RAW,
		{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1000, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_IMX135_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
		   {SENSOR_DRVNAME_IMX190_MIPI_RAW ,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_GC2755_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_High, 10},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{RST,   Vol_High, 10}
			},
		},
	   	{SENSOR_DRVNAME_IMX219_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_HI841_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
		//add new sensor before this line
		{NULL,},
	}
};

/* GPIO Pin control*/
struct platform_device *cam_plt_dev = NULL;
struct pinctrl *camctrl = NULL;
struct pinctrl_state *cam0_pnd_h = NULL;
struct pinctrl_state *cam0_pnd_l = NULL;
struct pinctrl_state *cam0_rst_h = NULL;
struct pinctrl_state *cam0_rst_l = NULL;
struct pinctrl_state *cam1_pnd_h = NULL;
struct pinctrl_state *cam1_pnd_l = NULL;
struct pinctrl_state *cam1_rst_h = NULL;
struct pinctrl_state *cam1_rst_l = NULL;
struct pinctrl_state *lens_af_en_l = NULL;
struct pinctrl_state *lens_af_en_h = NULL;

#if defined (CAMERA_POWER_VCAM_AVDD)
struct pinctrl_state *cam_ldo_avdd_h = NULL;
struct pinctrl_state *cam_ldo_avdd_l = NULL;
#endif
#if defined (CAMERA_POWER_VCAM_DVDD)
struct pinctrl_state *cam_ldo_dvdd_h = NULL;
struct pinctrl_state *cam_ldo_dvdd_l = NULL;
#endif

int mtkcam_gpio_init(struct platform_device *pdev)
{
	int ret = 0;

	camctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(camctrl)) {
		dev_err(&pdev->dev, "Cannot find camera pinctrl!");
		ret = PTR_ERR(camctrl);
	}
    /*Cam0 Power/Rst Ping initialization*/
	cam0_pnd_h = pinctrl_lookup_state(camctrl, "cam0_pnd1");
	if (IS_ERR(cam0_pnd_h)) {
		ret = PTR_ERR(cam0_pnd_h);
		pr_debug("%s : pinctrl err, cam0_pnd_h\n", __func__);
	}

	cam0_pnd_l = pinctrl_lookup_state(camctrl, "cam0_pnd0");
	if (IS_ERR(cam0_pnd_l)) {
		ret = PTR_ERR(cam0_pnd_l);
		pr_debug("%s : pinctrl err, cam0_pnd_l\n", __func__);
	}


	cam0_rst_h = pinctrl_lookup_state(camctrl, "cam0_rst1");
	if (IS_ERR(cam0_rst_h)) {
		ret = PTR_ERR(cam0_rst_h);
		pr_debug("%s : pinctrl err, cam0_rst_h\n", __func__);
	}

	cam0_rst_l = pinctrl_lookup_state(camctrl, "cam0_rst0");
	if (IS_ERR(cam0_rst_l)) {
		ret = PTR_ERR(cam0_rst_l);
		pr_debug("%s : pinctrl err, cam0_rst_l\n", __func__);
	}

    /*Cam1 Power/Rst Ping initialization*/
	cam1_pnd_h = pinctrl_lookup_state(camctrl, "cam1_pnd1");
	if (IS_ERR(cam1_pnd_h)) {
		ret = PTR_ERR(cam1_pnd_h);
		pr_debug("%s : pinctrl err, cam1_pnd_h\n", __func__);
	}

	cam1_pnd_l = pinctrl_lookup_state(camctrl, "cam1_pnd0");
	if (IS_ERR(cam1_pnd_l )) {
		ret = PTR_ERR(cam1_pnd_l );
		pr_debug("%s : pinctrl err, cam1_pnd_l\n", __func__);
	}


	cam1_rst_h = pinctrl_lookup_state(camctrl, "cam1_rst1");
	if (IS_ERR(cam1_rst_h)) {
		ret = PTR_ERR(cam1_rst_h);
		pr_debug("%s : pinctrl err, cam1_rst_h\n", __func__);
	}


	cam1_rst_l = pinctrl_lookup_state(camctrl, "cam1_rst0");
	if (IS_ERR(cam1_rst_l)) {
		ret = PTR_ERR(cam1_rst_l);
		pr_debug("%s : pinctrl err, cam1_rst_l\n", __func__);
	}
	/*externel LDO enable */
#if defined (CAMERA_POWER_VCAM_AVDD)
	cam_ldo_avdd_h = pinctrl_lookup_state(camctrl, "cam_ldo_avdd_1");
	if (IS_ERR(cam_ldo_avdd_h)) {
		ret = PTR_ERR(cam_ldo_avdd_h);
		pr_debug("%s : pinctrl err, cam_ldo_avdd_h\n", __func__);
	}
	cam_ldo_avdd_l = pinctrl_lookup_state(camctrl, "cam_ldo_avdd_0");
	if (IS_ERR(cam_ldo_avdd_l)) {
		ret = PTR_ERR(cam_ldo_avdd_l);
		pr_debug("%s : pinctrl err, cam_ldo_avdd_l\n", __func__);
	}
#endif
#if defined (CAMERA_POWER_VCAM_DVDD)
	cam_ldo_dvdd_h = pinctrl_lookup_state(camctrl, "cam_ldo_dvdd_1");
	if (IS_ERR(cam_ldo_dvdd_h)) {
		ret = PTR_ERR(cam_ldo_dvdd_h);
		pr_debug("%s : pinctrl err, cam_ldo_dvdd_h\n", __func__);
	}
	cam_ldo_dvdd_l = pinctrl_lookup_state(camctrl, "cam_ldo_dvdd_0");
	if (IS_ERR(cam_ldo_dvdd_l)) {
		ret = PTR_ERR(cam_ldo_dvdd_l);
		pr_debug("%s : pinctrl err, cam_ldo_dvdd_l\n", __func__);
	}
#endif
	/*enable main lens AF_EN*/
	lens_af_en_h = pinctrl_lookup_state(camctrl, "lens_af_en_1");
	if (IS_ERR(lens_af_en_h)) {
		ret = PTR_ERR(lens_af_en_h);
		pr_debug("%s : pinctrl err, lens_af_en_h\n", __func__);
	}


	lens_af_en_l = pinctrl_lookup_state(camctrl, "lens_af_en_0");
	if (IS_ERR(lens_af_en_l)) {
		ret = PTR_ERR(lens_af_en_l);
		pr_debug("%s : pinctrl err, lens_af_en_l\n", __func__);
	}
	return ret;
}
DEFINE_MUTEX(camera_set_gpio_mutex);
int mtkcam_gpio_set(int PinIdx, int PwrType, int Val)
{
	int ret = 0;
	mutex_lock(&camera_set_gpio_mutex);
	switch (PwrType) {
	case CAMRST:
		if (PinIdx == 0) {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam0_rst_l);
			else
				pinctrl_select_state(camctrl, cam0_rst_h);
		} else {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam1_rst_l);
			else
				pinctrl_select_state(camctrl, cam1_rst_h);
		}
		break;
	case CAMPDN:
		if (PinIdx == 0) {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam0_pnd_l);
			else
				pinctrl_select_state(camctrl, cam0_pnd_h);
		} else {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam1_pnd_l);
			else
				pinctrl_select_state(camctrl, cam1_pnd_h);
		}

		break;
#if defined (CAMERA_POWER_VCAM_AVDD)
	case CAMLDO_AVDD:
		if (Val == 0)
			pinctrl_select_state(camctrl, cam_ldo_avdd_l);
		else
			pinctrl_select_state(camctrl, cam_ldo_avdd_h);
		break;
#endif
#if defined (CAMERA_POWER_VCAM_DVDD)
	case CAMLDO_DVDD:
		if (Val == 0)
			pinctrl_select_state(camctrl, cam_ldo_dvdd_l);
		else
			pinctrl_select_state(camctrl, cam_ldo_dvdd_h);
		break;
#endif
	case AF_EN:
		if (Val == 0)
			pinctrl_select_state(camctrl, lens_af_en_l);
		else
			pinctrl_select_state(camctrl, lens_af_en_h);
		break;
	default:
		PK_DBG("PwrType(%d) is invalid !!\n", PwrType);
		break;
	};
	mutex_unlock(&camera_set_gpio_mutex);
	PK_DBG("PinIdx(%d) PwrType(%d) val(%d)\n", PinIdx, PwrType, Val);

	return ret;
}


BOOL hwpoweron(PowerInformation pwInfo, char* mode_name)
{
	if(pwInfo.PowerType == VCAMA)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s  pwInfo.PowerType == VCAMA\n",__func__);
		if(PowerCustList.PowerCustInfo[0].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			#if defined(CAMERA_POWER_VCAM_AVDD)
				if(pinSetIdx == 1)
				{
					printk("G_TEST camera sub AVDD power on\n");
					if(mtkcam_gpio_set(pinSetIdx, CAMLDO_AVDD, 1))
		   	 		{
		        			PK_DBG("[CAMERA SENSOR CAMLDO_AVDD] LINE=%d Fail to enable CAMLDO_AVDD\n",__LINE__);
		    		}
        		}
			#endif
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
       	 		{
            		PK_DBG("[CAMERA SENSOR] Fail to enable VCAMA power\n");
            		return FALSE;
        		}
		}
	}
	else if(pwInfo.PowerType == VCAMD)
	{
        PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMD\n",__func__);
		if(PowerCustList.PowerCustInfo[1].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			//PK_DBG("[CAMERA SENSOR] Main camera VAM_D power on");
			#if defined(CAMERA_POWER_VCAM_DVDD)
			if(pinSetIdx == 1)
			{
				printk("G_TEST camera sub DVDD power on\n");
				if(mtkcam_gpio_set(pinSetIdx, CAMLDO_DVDD, 1))
		   	 	{
		       		PK_DBG("[CAMERA SENSOR CAMLDO_DVDD] LINE=%d Fail to enable CAMLDO_DVDD\n",__LINE__);
		    	}
			}
			#endif
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
	       	 	{
	            		PK_DBG("[CAMERA SENSOR] LINE=%d Fail to enable VCAMD power\n",__LINE__);
	            		return FALSE;
	        	}
		}

	}
	else if(pwInfo.PowerType == VCAMIO)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMIO\n",__func__);
		if(PowerCustList.PowerCustInfo[2].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
       	 		{
            			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to enable VCAMIO power\n",__LINE__);
            			return FALSE;
        		}
		}

	}
	else if(pwInfo.PowerType == VCAMAF)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMAF\n",__func__);
		if(PowerCustList.PowerCustInfo[3].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
       	 		{
            			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to enable VCAMAF power\n",__LINE__);
            			return FALSE;
        		}
		}

	}
	else if(pwInfo.PowerType == AF_EN)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == AF_EN\n",__func__);
		if(PowerCustList.PowerCustInfo[4].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(mtkcam_gpio_set(pinSetIdx, AF_EN, 1))
       	 		{
            			PK_DBG("[CAMERA SENSOR AF_EN] LINE=%d Fail to enable AF_EN\n",__LINE__);
        		}
		}

	}
	else if(pwInfo.PowerType==PDN)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==PDN\n",__func__);
		if(pwInfo.Voltage == Vol_High)
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set PDN !! \n",__LINE__);}
		}
		else
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set PDN !! \n",__LINE__);}
		}
	}
	else if(pwInfo.PowerType==RST)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==RST \n",__func__);
		if(pinSetIdx==0)
		{
		    if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
			if(pwInfo.Voltage == Vol_High)
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST !! \n",__LINE__);}
			}
			else
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST !! \n",__LINE__);}
			}
		}
		else if(pinSetIdx==1)
		{
	        if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)\n",__LINE__);}
			if(pwInfo.Voltage == Vol_High)
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST!! \n",__LINE__);}
			}
			else
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST!! \n",__LINE__);}
			}
		}


	}
	else if(pwInfo.PowerType==SensorMCLK)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==SensorMCLK \n",__func__);
		if(pinSetIdx==0)
		{
			PK_DBG("Sensor MCLK1 On");
			ISP_MCLK1_EN(TRUE);
		}
		else if(pinSetIdx==1)
		{
			PK_DBG("Sensor MCLK2 On");
			ISP_MCLK2_EN(TRUE);
		}
	}
	else{}
	if(pwInfo.Delay>0)
		mdelay(pwInfo.Delay);
	return TRUE;
}



BOOL hwpowerdown(PowerInformation pwInfo, char* mode_name)
{
	if(pwInfo.PowerType == VCAMA)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMA\n",__func__);
		if(PowerCustList.PowerCustInfo[0].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			#if defined(CAMERA_POWER_VCAM_AVDD)
			if(pinSetIdx == 1)
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMLDO_AVDD, 0))
       	 		{
            		PK_DBG("[CAMERA SENSOR CAMLDO_AVDD] LINE=%d Fail to powerdown CAMLDO_AVDD\n",__LINE__);
        		}
        	}
			#endif
			if(TRUE != _hwPowerDown(pwInfo.PowerType))
       	 		{
            			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to powerdown VCAMA\n",__LINE__);
            			return FALSE;
        		}
		}
	}
	else if(pwInfo.PowerType == VCAMD)
	{
        PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMD\n",__func__);
		if(PowerCustList.PowerCustInfo[1].Gpio_Pin == GPIO_UNSUPPORTED)
		{
		#if defined(CAMERA_POWER_VCAM_DVDD)
			if(pinSetIdx == 1)
			{

				if(mtkcam_gpio_set(pinSetIdx, CAMLDO_DVDD, 0))
		   	 	{
		        	PK_DBG("[CAMERA SENSOR CAMLDO_DVDD] LINE=%d Fail to powerdown CAMLDO_DVDD\n",__LINE__);
		    	}
			}
		#elif defined(TYD_PRO_F1)
			if(pinSetIdx == 1)
			{
				PK_DBG("[CAMERA SENSOR] Sub camera VCAM_D power down \n");
				if(TRUE != _hwPowerDown(pwInfo.Voltage))
   	 			{
    				PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
    				return FALSE;
    			}
			}
    	#endif
			if(TRUE != _hwPowerDown(pwInfo.PowerType))
       	 		{
        			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to powerdown VCAMD\n",__LINE__);
        			return FALSE;
        		}
		}
	}
	else if(pwInfo.PowerType == VCAMIO)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMIO\n",__func__);
		if(PowerCustList.PowerCustInfo[2].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(TRUE != _hwPowerDown(pwInfo.PowerType))
       	 		{
            			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to powerdown VCAMIO\n",__LINE__);
            			return FALSE;
        		}
		}
	}
	else if(pwInfo.PowerType == VCAMAF)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMAF\n",__func__);
		if(PowerCustList.PowerCustInfo[3].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(TRUE != _hwPowerDown(pwInfo.PowerType))
       	 		{
            			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to powerdown VCAMAF\n",__LINE__);
            			return FALSE;
        		}
		}
	}
	else if(pwInfo.PowerType == AF_EN)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == AF_EN\n",__func__);
		if(PowerCustList.PowerCustInfo[4].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(mtkcam_gpio_set(pinSetIdx, AF_EN, 0))
       	 		{
            			PK_DBG("[CAMERA SENSOR AF_EN] LINE=%d Fail to enable AF_EN\n",__LINE__);
        		}
		}

	}
	else if(pwInfo.PowerType==PDN)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==PDN\n",__func__);
		if(pwInfo.Voltage == Vol_High)
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (PDN)!! \n",__LINE__);}
			msleep(1);
		}
		else
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (PDN)!! \n",__LINE__);}
			msleep(1);
		}
	}
	else if(pwInfo.PowerType==RST)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==RST\n",__func__);
		if(pinSetIdx==0)
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)\n",__LINE__);}
			if(pwInfo.Voltage == Vol_High)
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)!! \n",__LINE__);}
			}
			else
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)!! \n",__LINE__);}
			}
		}
		else if(pinSetIdx==1)
		{

			if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)\n",__LINE__);}
			if(pwInfo.Voltage == Vol_High)
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)!! \n",__LINE__);}
			}
			else
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)!! \n",__LINE__);}
			}
		}
	}
	else if(pwInfo.PowerType==SensorMCLK)
	{
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==SensorMCLK \n",__func__);
		if(pinSetIdx==0)
		{
			ISP_MCLK1_EN(FALSE);
		}
		else if(pinSetIdx==1)
		{
			ISP_MCLK2_EN(FALSE);
		}
	}
	else{}
	return TRUE;
}


int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, bool On, char *mode_name)
{

	int pwListIdx=0;
	int pwIdx=0;
	static int count=0;
    	BOOL sensorInPowerList = KAL_FALSE;

   	if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx)
	{
		pinSetIdx = 0;
	}
    	else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx)
	{
        	pinSetIdx = 1;
   	}
    	else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx)
	{
        	pinSetIdx = 2;
    	}
   	printk(KERN_ERR"#h#j#,%d times %d name=%s %d modemname=%s\n",count++,SensorIdx,currSensorName,On,mode_name);

	/* power ON */
	if (On)
	{
		PK_DBG("kdCISModulePowerOn -on:currSensorName=%s\n",currSensorName);
		PK_DBG("kdCISModulePowerOn -on:pinSetIdx=%d\n",pinSetIdx);

		for(pwListIdx=0 ; pwListIdx<24; pwListIdx++)
		{
			if(currSensorName && (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,currSensorName)))
			{
				PK_DBG("kdCISModulePowerOn get in--- \n");
				PK_DBG("sensorIdx:%d \n",SensorIdx);

                		sensorInPowerList = KAL_TRUE;

				for(pwIdx=0;pwIdx<10;pwIdx++)
				{
					if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType != VDD_None)
					{
						if(hwpoweron(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx],mode_name)==FALSE)
						goto _kdCISModulePowerOn_exit_;
					}
					else
					{
						PK_DBG("pwIdx=%d \n",pwIdx);
						break;
					}
				}
				break;
			}
			else if(PowerOnList.PowerSeq[pwListIdx].SensorName == NULL)
			{
				break;
			}
			else
			{

			}
		}


		// Temp solution: default power on/off sequence
        	if(KAL_FALSE == sensorInPowerList)
        	{
            		PK_DBG("Default power on sequence");

            		if(pinSetIdx == 0 )
			{
                		ISP_MCLK1_EN(1);
            		}
            		else if (pinSetIdx == 1)
			{
                		ISP_MCLK2_EN(1);
            		}

            		//First Power Pin low and Reset Pin Low
            		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
			{
                		if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            		}

            		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
			{
                		if(0 == pinSetIdx)
				{
                    			if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
                		}
                		else
				{
                   			if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
                		}
            		}
			//VCAM_IO
            		if(TRUE != _hwPowerOn(VCAMIO, VOL_1800))
            		{
				PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO) \n");
                		goto _kdCISModulePowerOn_exit_;
            		}

            		//VCAM_A
            		if(TRUE != _hwPowerOn(VCAMA, VOL_2800))
            		{
                		PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A)\n");
                		goto _kdCISModulePowerOn_exit_;
            		}
	    		//VCAMD
            		if(TRUE != _hwPowerOn(VCAMD, VOL_1500))
            		{
                 		PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D)\n");
                 		goto _kdCISModulePowerOn_exit_;
            		}

             		//AF_VCC
            		if(TRUE != _hwPowerOn(VCAMAF, VOL_2800))
            		{
               			PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF)\n");
                		goto _kdCISModulePowerOn_exit_;
            		}

			if(mtkcam_gpio_set(pinSetIdx, AF_EN, 1))
			{
               			PK_DBG("[CAMERA SENSOR AF_EN] Fail to enable AF_EN\n");
			}

            		mdelay(5);

            		//enable active sensor
            		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
	    		{
				if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            		}

            		mdelay(1);

            		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
	    		{
            			if(0 == pinSetIdx)
				{
					if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
				}
                		else
				{
					if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
                		}
            		}
		}

	}
	else
	{ /* power OFF */
		for(pwListIdx=0 ; pwListIdx<24; pwListIdx++)
		{
			if(currSensorName && (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,currSensorName)))
			{
				PK_DBG("kdCISModulePowerOn get in--- \n");
				PK_DBG("sensorIdx:%d \n",SensorIdx);

                		sensorInPowerList = KAL_TRUE;

				for(pwIdx=9;pwIdx>=0;pwIdx--)
				{
					if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType != VDD_None)
					{
						if(hwpowerdown(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx],mode_name)==FALSE)
						goto _kdCISModulePowerOn_exit_;
						if(pwIdx>0)
						{
							if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx-1].Delay > 0)
							mdelay(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx-1].Delay);
						}
					}
					else
					{
						PK_DBG("pwIdx=%d \n",pwIdx);
					}
				}
			}
			else if(PowerOnList.PowerSeq[pwListIdx].SensorName == NULL)
			{
				break;
			}
			else
			{

			}
		}

        	// Temp solution: default power on/off sequence
        	if(KAL_FALSE == sensorInPowerList)
        	{
		    	PK_DBG("Default power off sequence");

		    	if(pinSetIdx == 0 )
			{
		        	ISP_MCLK1_EN(0);
		    	}
		    	else if (pinSetIdx == 1)
			{
		        	ISP_MCLK2_EN(0);
		    	}

		    	//Set Power Pin low and Reset Pin Low
		    	if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
		   	{
		        	if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
		    	}

		    	if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
			{
		        	if(0 == pinSetIdx)
				{
		            		if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
		        	}
		        	else
				{

		            		if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
		        	}
		    	}

			//VCAM_D
		    	if(TRUE != _hwPowerDown(VCAMD))
		    	{
		         	PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D)\n");
		         	goto _kdCISModulePowerOn_exit_;
		    	}

		    	//VCAM_A
		    	if(TRUE != _hwPowerDown(VCAMA))
			{
		        	PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A)\n");
		        	//return -EIO;
		        	goto _kdCISModulePowerOn_exit_;
		    	}

		    	//VCAM_IO
		    	if(TRUE != _hwPowerDown(VCAMIO))
			{
		        	PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO)\n");
		        	//return -EIO;
		        	goto _kdCISModulePowerOn_exit_;
		    	}

		    	//AF_VCC
		    	if(TRUE != _hwPowerDown(VCAMAF))
		    	{
		       		PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF)\n");
		        	//return -EIO;
		        	goto _kdCISModulePowerOn_exit_;
		    	}
			if(mtkcam_gpio_set(pinSetIdx, AF_EN, 0))
			{
               			PK_DBG("[CAMERA SENSOR AF_EN] Fail to enable AF_EN\n");
			}
		}
    	}

	return 0;

	_kdCISModulePowerOn_exit_:
	return -EIO;

}

EXPORT_SYMBOL(kdCISModulePowerOn);

/* !-- */
/*  */


