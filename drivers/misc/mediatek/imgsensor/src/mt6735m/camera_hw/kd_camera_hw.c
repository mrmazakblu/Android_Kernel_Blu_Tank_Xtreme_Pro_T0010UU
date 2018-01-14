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

DEFINE_MUTEX(main_flashlight_mutex);
EXPORT_SYMBOL_GPL(main_flashlight_mutex);

u32 pinSetIdx = 0;//default main sensor
//Modify camera main2 by Droi DXQ start 20170210
u32 pinSet[3][8] = {
//Modify camera main2 by Droi DXQ end 20170210
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
//Modify camera main2 by Droi DXQ start 20170906
	{
		CAMERA_CMRST2_PIN,
		CAMERA_CMRST2_PIN_M_GPIO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,
		CAMERA_CMPDN2_PIN,
		CAMERA_CMPDN2_PIN_M_GPIO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,
	},
//Modify camera main2 by Droi DXQ end 20170906
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
		{SENSOR_DRVNAME_GC8024MIPI_RAW,
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
#if defined(OV9760_MIPI_RAW)
		{SENSOR_DRVNAME_OV9760_MIPI_RAW,
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
#endif
		{SENSOR_DRVNAME_HM8040_MIPI_RAW,
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
		{SENSOR_DRVNAME_HM5040_MIPI_RAW,
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
		{SENSOR_DRVNAME_BF2206MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_VD6955_MIPI_RAW,
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
		{SENSOR_DRVNAME_GC5025_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 5},
				{VCAMA,  Vol_2800, 5},
				{VCAMD,  Vol_1200, 5},
				{VCAMAF, Vol_2800, 5},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  5},
				{PDN,   Vol_High, 5},
				{RST,   Vol_Low,  5},
				{RST,   Vol_High, 5}
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
#if defined(GC2755_MIPI_RAW)
		{SENSOR_DRVNAME_GC2755_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
#endif
#if defined(GC2755_1LANE_MIPI_RAW)
		{SENSOR_DRVNAME_GC2755_1LANE_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
#endif
#if defined(GC0310_MIPI_YUV)
		{SENSOR_DRVNAME_GC0310_MIPI_YUV,
		  {
		   {PDN,    Vol_Low, 10},
		   {VCAMIO, Vol_1800, 10},
		   {VCAMA,  Vol_2800, 10},
		   {SensorMCLK,Vol_High, 5},
		   {PDN,    Vol_High, 10},
		   {PDN,    Vol_Low, 10},
		   },
	  },
#endif
	    {SENSOR_DRVNAME_SP2509_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				//{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High,  10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low, 10},
				{RST,    Vol_High, 10}
			},
		},	
		{SENSOR_DRVNAME_SP2609_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				//{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High,  10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low, 10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_SP5506_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMA,  Vol_2800, 10},
				{VCAMIO, Vol_1800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,    Vol_Low, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_High, 10}

			},
		},
		{SENSOR_DRVNAME_SP5506_SUB_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMA,  Vol_2800, 10},
				{VCAMIO, Vol_1800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,    Vol_Low, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_High, 10}

			},
		},
		{SENSOR_DRVNAME_SP5407_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMA,  Vol_2800, 10},
				{VCAMIO, Vol_1800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,    Vol_Low, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_High, 10}

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
		{SENSOR_DRVNAME_MN34172_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_MN045_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_High, 10}
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
	  	{SENSOR_DRVNAME_IMX237_MIPI_RAW,
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
		{SENSOR_DRVNAME_GC2365MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_GC2355_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_GC2375MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
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
		{ SENSOR_DRVNAME_GC5005MIPI_SUB_RAW,
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
	   	{SENSOR_DRVNAME_GC5024_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 5},
				{VCAMA,  Vol_2800, 5},
				{VCAMD,  Vol_1500, 5},  // 1.5
				{VCAMAF, Vol_2800, 5},
                {AF_EN,  Vol_2800, 15},
				{PDN,   Vol_High, 5},
				{PDN,   Vol_Low,  5},
				{RST,   Vol_Low,  5},
				{RST,   Vol_High, 5}
			},
		},
		{SENSOR_DRVNAME_S5K3L2_MIPI_RAW,
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
		{SENSOR_DRVNAME_S5K3M2_MIPI_RAW,
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
		{SENSOR_DRVNAME_S5K3H7_MIPI_RAW,
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
 		{SENSOR_DRVNAME_S5K3H7YX_MIPI_RAW,
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
	   	{SENSOR_DRVNAME_S5K3L8_MIPI_RAW,
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
	   	{SENSOR_DRVNAME_S5K4H8_MIPI_RAW,
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
#if defined(S5K3H2_MIPI_RAW)
		{SENSOR_DRVNAME_S5K3H2_MIPI_RAW,
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
#endif
#if defined(S5K3H2_MIPI_RAW_CXT)
		{SENSOR_DRVNAME_S5K3H2_MIPI_RAW_CXT,
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
#endif
		{SENSOR_DRVNAME_GC2385MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{PDN,    Vol_Low, 10},
				{PDN,    Vol_High,  10}
			},
		},

//SUB DVDD MAX 1.5V

		{SENSOR_DRVNAME_GC2155MIPI_RAW_YUV,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
			//	{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_SP8407_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_OV13850_MIPI_RAW,
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
		{SENSOR_DRVNAME_OV5675_MIPI_RAW,
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
#if defined(OV5693_MIPI_RAW)
		{SENSOR_DRVNAME_OV5693_MIPI_RAW,
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
#endif
		{SENSOR_DRVNAME_OV5670_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				//{VCAMAF, Vol_2800, 15},
				//{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
	   	{SENSOR_DRVNAME_IMX214_MIPI_RAW,
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
		{SENSOR_DRVNAME_OV5648_MIPI_RAW,
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
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_IMX149_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{AF_EN,  Vol_2800, 15},
				{PDN,    Vol_Low,  10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_High, 10}
			},
		},
		{SENSOR_DRVNAME_OV8858_MIPI_RAW,
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
		{SENSOR_DRVNAME_OV8856_MIPI_RAW,
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
   		{SENSOR_DRVNAME_HI545_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1200, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_Low,  10},
				{PDN,    Vol_High, 10},
				{RST,    Vol_Low,  10},
				{RST,    Vol_High, 10}
			},
		},
        {SENSOR_DRVNAME_HI351_MIPI_YUV,
         {
             {SensorMCLK,Vol_High, 0},
             {VCAMIO, Vol_1800, 10},
			 {VCAMA,  Vol_2800, 10},
			 {VCAMD,  Vol_1200, 10},
             {PDN,    Vol_Low, 10},
             {PDN,    Vol_High,  10},
             {RST,    Vol_Low,  10},
             {RST,    Vol_High, 10}
         },
        },
		{SENSOR_DRVNAME_GC8034MIPI_RAW,
			{
                {SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 5},
				{VCAMA,  Vol_2800, 5},
				{VCAMD,  Vol_1200, 5},
				{VCAMAF, Vol_2800, 5},
				{AF_EN,  Vol_2800, 15},
				{PDN,   Vol_Low, 5},
				{PDN,   Vol_High,  5},
				{RST,   Vol_Low,  5},
				{RST,   Vol_High, 5}
			},
		},
		{SENSOR_DRVNAME_IMX258_MIPI_RAW,
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
	   	{SENSOR_DRVNAME_AR0543_MIPI_RAW,
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
#if defined(SP5509_MIPI_RAW)
		{SENSOR_DRVNAME_SP5509_MIPI_RAW,
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
#endif
#if defined(SP250A_MIPI_RAW)
		{SENSOR_DRVNAME_SP250A_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{VCAMIO, Vol_1800, 10},
				{VCAMA,  Vol_2800, 10},
				{VCAMD,  Vol_1500, 10},
				{VCAMAF, Vol_2800, 15},
				{PDN,    Vol_High,  10},
				{RST,    Vol_Low,  10},
				{PDN,    Vol_Low, 10},
				{RST,    Vol_High, 10}
			},
		},
#endif
#if defined(OV8865_MIPI_RAW)
		{SENSOR_DRVNAME_OV8865_MIPI_RAW,
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
#endif
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
#if defined(CAMERA_SUB_LENS_EN)
struct pinctrl_state *lens_sub_af_en_l = NULL;
struct pinctrl_state *lens_sub_af_en_h = NULL;
#endif

#if defined (CAMERA_POWER_VCAM_AVDD)
struct pinctrl_state *cam_ldo_avdd_h = NULL;
struct pinctrl_state *cam_ldo_avdd_l = NULL;
#endif
#if defined (CAMERA_POWER_VCAM_DVDD)
struct pinctrl_state *cam_ldo_dvdd_h = NULL;
struct pinctrl_state *cam_ldo_dvdd_l = NULL;
#endif
//Modify camera main2 by Droi DXQ start 20170906
#if defined (CAMERA_MAIN2_GPIO_SUPPORT)
struct pinctrl_state *cam2_pnd_h = NULL;
struct pinctrl_state *cam2_pnd_l = NULL;
struct pinctrl_state *cam2_rst_h = NULL;
struct pinctrl_state *cam2_rst_l = NULL;
struct pinctrl_state *cam3_pnd_h = NULL;
struct pinctrl_state *cam3_pnd_l = NULL;
struct pinctrl_state *cam3_rst_h = NULL;
struct pinctrl_state *cam3_rst_l = NULL;
struct pinctrl_state *mipi1_sel_h = NULL;
struct pinctrl_state *mipi1_sel_l = NULL;
struct pinctrl_state *mipi2_sel_h = NULL;
struct pinctrl_state *mipi2_sel_l = NULL;
#endif
//Modify camera main2 by Droi DXQ end 20170906

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
#if defined(CAMERA_SUB_LENS_EN)
	/*enable main lens AF_EN*/
	lens_sub_af_en_h = pinctrl_lookup_state(camctrl, "lens_sub_af_en_1");
	if (IS_ERR(lens_sub_af_en_h)) {
		ret = PTR_ERR(lens_sub_af_en_h);
		pr_debug("%s : pinctrl err, lens_sub_af_en_h\n", __func__);
	}


	lens_sub_af_en_l = pinctrl_lookup_state(camctrl, "lens_sub_af_en_0");
	if (IS_ERR(lens_sub_af_en_l)) {
		ret = PTR_ERR(lens_sub_af_en_l);
		pr_debug("%s : pinctrl err, lens_sub_af_en_l\n", __func__);
	}
#endif
//Modify camera main2 by Droi DXQ start 20170906
#if defined(CAMERA_MAIN2_GPIO_SUPPORT)
	/*Cam2 Power/Rst Ping initialization */
	cam2_pnd_h = pinctrl_lookup_state(camctrl, "cam2_pnd1");
	if (IS_ERR(cam2_pnd_h)) {
		ret = PTR_ERR(cam2_pnd_h);
		pr_debug("%s : pinctrl err, cam2_pnd_h\n", __func__);
	}

	cam2_pnd_l = pinctrl_lookup_state(camctrl, "cam2_pnd0");
	if (IS_ERR(cam2_pnd_l )) {
		ret = PTR_ERR(cam2_pnd_l );
		pr_debug("%s : pinctrl err, cam2_pnd_l\n", __func__);
	}

	cam2_rst_h = pinctrl_lookup_state(camctrl, "cam2_rst1");
	if (IS_ERR(cam2_rst_h)) {
		ret = PTR_ERR(cam2_rst_h);
		pr_debug("%s : pinctrl err, cam2_rst_h\n", __func__);
	}

	cam2_rst_l = pinctrl_lookup_state(camctrl, "cam2_rst0");
	if (IS_ERR(cam2_rst_l)) {
		ret = PTR_ERR(cam2_rst_l);
		pr_debug("%s : pinctrl err, cam2_rst_l\n", __func__);
	}

	/*Sub Cam2 Power/Rst Ping initialization */
	cam3_pnd_h = pinctrl_lookup_state(camctrl, "cam3_pnd1");
	if (IS_ERR(cam3_pnd_h)) {
		ret = PTR_ERR(cam3_pnd_h);
		pr_debug("%s : pinctrl err, cam3_pnd_h\n", __func__);
	}

	cam3_pnd_l = pinctrl_lookup_state(camctrl, "cam3_pnd0");
	if (IS_ERR(cam3_pnd_l )) {
		ret = PTR_ERR(cam3_pnd_l );
		pr_debug("%s : pinctrl err, cam3_pnd_l\n", __func__);
	}

	cam3_rst_h = pinctrl_lookup_state(camctrl, "cam3_rst1");
	if (IS_ERR(cam3_rst_h)) {
		ret = PTR_ERR(cam3_rst_h);
		pr_debug("%s : pinctrl err, cam3_rst_h\n", __func__);
	}

	cam3_rst_l = pinctrl_lookup_state(camctrl, "cam3_rst0");
	if (IS_ERR(cam3_rst_l)) {
		ret = PTR_ERR(cam3_rst_l);
		pr_debug("%s : pinctrl err, cam3_rst_l\n", __func__);
	}

	mipi1_sel_h = pinctrl_lookup_state(camctrl, "mipi1_sel1");
	if (IS_ERR(mipi1_sel_h)) {
		ret = PTR_ERR(mipi1_sel_h);
		pr_debug("%s : pinctrl err, mipi1_sel_h\n", __func__);
	}

	mipi1_sel_l = pinctrl_lookup_state(camctrl, "mipi1_sel0");
	if (IS_ERR(mipi1_sel_l)) {
		ret = PTR_ERR(mipi1_sel_l);
		pr_debug("%s : pinctrl err, mipi1_sel_l\n", __func__);
	}
	mipi2_sel_h = pinctrl_lookup_state(camctrl, "mipi2_sel1");
	if (IS_ERR(mipi2_sel_h)) {
		ret = PTR_ERR(mipi2_sel_h);
		pr_debug("%s : pinctrl err, mipi2_sel_h\n", __func__);
	}

	mipi2_sel_l = pinctrl_lookup_state(camctrl, "mipi2_sel0");
	if (IS_ERR(mipi2_sel_l)) {
		ret = PTR_ERR(mipi2_sel_l);
		pr_debug("%s : pinctrl err, mipi2_sel_l\n", __func__);
	}
#endif
//Modify camera main2 by Droi DXQ end 20170906
	return ret;
}
//Modify camera main2 by Droi DXQ start 20170906
#if defined(CAMERA_MAIN2_GPIO_SUPPORT)
extern bool droi_is_sub2;
#endif
//Modify camera main2 by Droi DXQ end 20170906
DEFINE_MUTEX(camera_set_gpio_mutex);//xinghai
EXPORT_SYMBOL_GPL(camera_set_gpio_mutex);
int mtkcam_gpio_set(int PinIdx, int PwrType, int Val)
{
	int ret = 0;

	mutex_lock(&camera_set_gpio_mutex);//xinghai
	switch (PwrType) {
	case CAMRST:
		if (PinIdx == 0) {
			if (Val == 0){
					pinctrl_select_state(camctrl, cam0_rst_l);
				} else {
				#if defined(DROI_PRO_PU6)
					pinctrl_select_state(camctrl, cam1_rst_l);
				#endif
					pinctrl_select_state(camctrl, cam0_rst_h);
				}
//Modify camera main2 by Droi DXQ start 20170906
#if defined(CAMERA_MAIN2_GPIO_SUPPORT)
		} else if (PinIdx == 2) {
			if (!droi_is_sub2) {
				if (IS_ERR(cam2_rst_h) || IS_ERR(cam2_rst_l)) {
					break;
				}
				if (Val == 0) {
					pinctrl_select_state(camctrl, cam2_rst_l);
				} else {
					pinctrl_select_state(camctrl, cam2_rst_h);
				}
			} else {
				if (IS_ERR(cam3_rst_h) || IS_ERR(cam3_rst_l)) {
					break;
				}
				if (Val == 0) {
					pinctrl_select_state(camctrl, cam3_rst_l);
				} else {
					pinctrl_select_state(camctrl, cam3_rst_h);
				}
			}
#endif
//Modify camera main2 by Droi DXQ end 20170906
		} else {
			if (Val == 0){
				pinctrl_select_state(camctrl, cam1_rst_l);
			} else {
			#if defined(DROI_PRO_PU6)
				pinctrl_select_state(camctrl, cam0_rst_l);
			#endif
				pinctrl_select_state(camctrl, cam1_rst_h);
			}
		}
		break;
	case CAMPDN:
		if (PinIdx == 0) {
			if (Val == 0){
				pinctrl_select_state(camctrl, cam0_pnd_l);
			} else {
			#if defined(DROI_PRO_PU6)
				pinctrl_select_state(camctrl, cam1_pnd_l);
			#endif
				pinctrl_select_state(camctrl, cam0_pnd_h);
			}
//Modify camera main2 by Droi DXQ start 20170906
#if defined(CAMERA_MAIN2_GPIO_SUPPORT)
		} else if (PinIdx == 2) {
			if (!droi_is_sub2) {
				if (IS_ERR(cam2_pnd_h) || IS_ERR(cam2_pnd_l)) {
					break;
				}
				if (Val == 0) {
					pinctrl_select_state(camctrl, cam2_pnd_l);
				} else {
					pinctrl_select_state(camctrl, cam2_pnd_h);
				}
			} else {
				if (IS_ERR(cam3_pnd_h) || IS_ERR(cam3_pnd_l)) {
					break;
				}
				if (Val == 0) {
					pinctrl_select_state(camctrl, cam3_pnd_l);
				} else {
					pinctrl_select_state(camctrl, cam3_pnd_h);
				}
			}
#endif
//Modify camera main2 by Droi DXQ end 20170906
		} else {
			if (Val == 0){
				pinctrl_select_state(camctrl, cam1_pnd_l);
			} else {
			#if defined(DROI_PRO_PU6)
				pinctrl_select_state(camctrl, cam0_pnd_l);
				#if defined(GC8024_MIPI_RAW) && defined(GC2385_MIPI_RAW)
					pinctrl_select_state(camctrl, cam0_pnd_h);
				#endif
			#endif
				pinctrl_select_state(camctrl, cam1_pnd_h);
			}
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
		if (Val == 0){
			if (IS_ERR(lens_af_en_l)) {
				break;
				}
			pinctrl_select_state(camctrl, lens_af_en_l);
			}
		else{
			if (IS_ERR(lens_af_en_h)) {
				break;
				}
			pinctrl_select_state(camctrl, lens_af_en_h);
		break;
		}
#if defined(CAMERA_SUB_LENS_EN)
	case SUB_AF_EN:
		if (Val == 0)
			pinctrl_select_state(camctrl, lens_sub_af_en_l);
		else
			pinctrl_select_state(camctrl, lens_sub_af_en_h);
		break;
#endif
//Modify camera main2 by Droi DXQ start 20170906
#if defined(CAMERA_MAIN2_GPIO_SUPPORT)
	case MIPI_SEL:
		if (IS_ERR(mipi1_sel_h) || IS_ERR(mipi1_sel_l)) {
			printk("PWU6 mipi1_sel error\n");
			break;
		}
		if (IS_ERR(mipi2_sel_h) || IS_ERR(mipi2_sel_l)) {
			printk("PWU6 mipi2_sel error\n");
			break;
		}
		if (PinIdx == 0) {
			pinctrl_select_state(camctrl, mipi1_sel_l);
		} else if (PinIdx == 1) {
			pinctrl_select_state(camctrl, mipi1_sel_h);
		} else if (PinIdx == 2) {
			if (!droi_is_sub2) {
				pinctrl_select_state(camctrl, mipi2_sel_l);
			} else {
				pinctrl_select_state(camctrl, mipi2_sel_h);
			}
		}
		break;
#endif
//Modify camera main2 by Droi DXQ end 20170906
	default:
		PK_DBG("PwrType(%d) is invalid !!\n", PwrType);
		break;
	};
	mutex_unlock(&camera_set_gpio_mutex);//xinghai

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
				if(mtkcam_gpio_set(pinSetIdx, CAMLDO_AVDD, 1))
       	 		{
            			PK_DBG("[CAMERA SENSOR CAMLDO_AVDD] LINE=%d Fail to enable CAMLDO_AVDD\n",__LINE__);
        		}
			#endif
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
       	 		{
            			PK_DBG("[CAMERA SENSOR] Fail to enable VCAMA power\n");
            			return FALSE;
        		}
		}
	} else if(pwInfo.PowerType == VCAMD) {
		PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMD\n",__func__);
		if(PowerCustList.PowerCustInfo[1].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			//PK_DBG("[CAMERA SENSOR] Main camera VAM_D power on");
#if defined(DROI_PRO_F2C)
			if(pinSetIdx == 1)
			{
				PK_DBG("[CAMERA SENSOR] Sub camera VCAM_D power on\n");
//				if(TRUE != hwPowerOn(SUB_CAMERA_POWER_VCAM_D,pwInfo.Voltage,mode_name))
				if(TRUE != _hwPowerOn(VRF18_1,pwInfo.Voltage))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
					return FALSE;
				}
			}else if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage)){
				PK_DBG("[CAMERA SENSOR] LINE=%d Fail to enable VCAMD power\n",__LINE__);
				return FALSE;
			}
#else
			#if defined(CAMERA_POWER_VCAM_DVDD)
			if(mtkcam_gpio_set(pinSetIdx, CAMLDO_DVDD, 1))
			{
				PK_DBG("[CAMERA SENSOR CAMLDO_DVDD] LINE=%d Fail to enable CAMLDO_DVDD\n",__LINE__);
			}
			#endif
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
			{
				PK_DBG("[CAMERA SENSOR] LINE=%d Fail to enable VCAMD power\n",__LINE__);
				return FALSE;
			}
#endif
		}

	} else if(pwInfo.PowerType == VCAMIO) {
		PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMIO\n",__func__);
		if(PowerCustList.PowerCustInfo[2].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
       	 		{
            			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to enable VCAMIO power\n",__LINE__);
            			return FALSE;
        		}
		}

	} else if(pwInfo.PowerType == VCAMAF) {
		PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == VCAMAF\n",__func__);
		if(PowerCustList.PowerCustInfo[3].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(TRUE != _hwPowerOn(pwInfo.PowerType,pwInfo.Voltage))
       	 		{
            			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to enable VCAMAF power\n",__LINE__);
            			return FALSE;
        		}
		}

	} else if(pwInfo.PowerType == AF_EN) {

		if(pinSetIdx == 0)
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
		#if defined(CAMERA_SUB_LENS_EN)
		else if(pinSetIdx == 1)
		{
	        printk("[CAMERA SENSOR] func==%s   pwInfo.PowerType == SUB_AF_EN\n",__func__);
			if(PowerCustList.PowerCustInfo[4].Gpio_Pin == GPIO_UNSUPPORTED)
			{
				if(mtkcam_gpio_set(pinSetIdx, SUB_AF_EN, 1))
	       	 		{
	            			PK_DBG("[CAMERA SENSOR AF_EN] LINE=%d Fail to enable SUB_AF_EN\n",__LINE__);
	        		}
			}
		}
		#endif
		else
		{

		}
	} else if(pwInfo.PowerType==PDN) {
		PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==PDN\n",__func__);
		if(pwInfo.Voltage == Vol_High)
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set PDN !! \n",__LINE__);}
		}
		else
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set PDN !! \n",__LINE__);}
		}
	} else if(pwInfo.PowerType==RST) {
		PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==RST \n",__func__);
		if(pinSetIdx==0)
		{
			if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
			if(pwInfo.Voltage == Vol_High) {
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST !! \n",__LINE__);}
			}
			else
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST !! \n",__LINE__);}
			}
		} else if(pinSetIdx==1) {
			if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d set gpio failed!! (CMRST)\n",__LINE__);}
			if(pwInfo.Voltage == Vol_High)
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST!! \n",__LINE__);}
			}
			else
			{
				if(mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] LINE=%d Fail to set CMRST!! \n",__LINE__);}
			}
		} else if(pinSetIdx==2) {
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

	} else if(pwInfo.PowerType==SensorMCLK) {
            	PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType==SensorMCLK \n",__func__);

			ISP_MCLK1_EN(TRUE);

	} else {

	}
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
				if(mtkcam_gpio_set(pinSetIdx, CAMLDO_AVDD, 0))
       	 		{
            			PK_DBG("[CAMERA SENSOR CAMLDO_AVDD] LINE=%d Fail to powerdown CAMLDO_AVDD\n",__LINE__);
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
#if defined(DROI_PRO_F2C)
		if(PowerCustList.PowerCustInfo[1].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(pinSetIdx==1)
			{
				PK_DBG("[CAMERA SENSOR] Sub camera VCAM_D power down \n");
//				if(TRUE != hwPowerDown(SUB_CAMERA_POWER_VCAM_D,mode_name))
				if(TRUE != _hwPowerDown(VRF18_1))
   	 			{
    				PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
    				return FALSE;
    			}
			}else if(TRUE != _hwPowerDown(pwInfo.PowerType))
       	 		{
        			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to powerdown VCAMD\n",__LINE__);
        			return FALSE;
        		}

		}
#else
		#if defined(CAMERA_POWER_VCAM_DVDD)
				if(mtkcam_gpio_set(pinSetIdx, CAMLDO_DVDD, 0))
       	 		{
            			PK_DBG("[CAMERA SENSOR CAMLDO_DVDD] LINE=%d Fail to powerdown CAMLDO_DVDD\n",__LINE__);
        		}
		#endif
		if(PowerCustList.PowerCustInfo[1].Gpio_Pin == GPIO_UNSUPPORTED)
		{
			if(TRUE != _hwPowerDown(pwInfo.PowerType))
       	 		{
        			PK_DBG("[CAMERA SENSOR] LINE=%d Fail to powerdown VCAMD\n",__LINE__);
        			return FALSE;
        		}

		}
#endif
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

		if(pinSetIdx == 0)
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
		#if defined(CAMERA_SUB_LENS_EN)
		else if(pinSetIdx == 1)
		{
	        PK_DBG("[CAMERA SENSOR] func==%s   pwInfo.PowerType == SUB_AF_EN\n",__func__);
			if(PowerCustList.PowerCustInfo[4].Gpio_Pin == GPIO_UNSUPPORTED)
			{
				if(mtkcam_gpio_set(pinSetIdx, SUB_AF_EN, 0))
	       	 		{
	            			PK_DBG("[CAMERA SENSOR AF_EN] LINE=%d Fail to enable SUB_AF_EN\n",__LINE__);
	        		}
			}
		}
		#endif
		else
		{

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
		} else if(pinSetIdx==2)
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

			ISP_MCLK1_EN(FALSE);

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
//Modify camera main2 by Droi DXQ start 20170906
#if defined(CAMERA_MAIN2_GPIO_SUPPORT)
		mtkcam_gpio_set(pinSetIdx, MIPI_SEL, 0);
	if (currSensorName && (strcmp(currSensorName, "imx219mipiraw") == 0)) {
		if ((pinSetIdx == 1)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "gc8024mipiraw") == 0)) {
		if ((pinSetIdx == 1)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "gc0310mipiyuv") == 0)) {
		if ((pinSetIdx == 0)||(pinSetIdx == 1))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "gc2385mipiraw") == 0)) {
		if ((pinSetIdx == 0)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
#if defined(DROI_PRO_PU6_JF2)||defined(DROI_PRO_PU6_JF)
	#if defined(DROI_PRO_PU6_JF_K16)
	//do noting
	#else
	if (currSensorName && (strcmp(currSensorName, "gc5025mipiraw") == 0)) {
		if ((pinSetIdx == 0)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
	#endif
#endif
#if defined(DROI_PRO_PU6_OLK)
	if (currSensorName && (strcmp(currSensorName, "ov9760mipiraw") == 0)) {
		if ((pinSetIdx == 0)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "ov5693mipi") == 0)) {
		if ((pinSetIdx == 1)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "sp2509mipiraw") == 0)) {
		if ((pinSetIdx == 0)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "imx149mipiraw") == 0)) {
		if ((pinSetIdx == 1)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "s5k3h7yxmipiraw") == 0)) {
		if ((pinSetIdx == 1)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
#endif
#if defined(DROI_PRO_PU6_FAC)
	if (currSensorName && (strcmp(currSensorName, "gc2755_1lane_mipiraw") == 0)) {
		if ((pinSetIdx == 0)||(pinSetIdx == 1))
			goto _kdCISModulePowerOn_exit_;
	}
	if (currSensorName && (strcmp(currSensorName, "gc2755mipiraw") == 0)) {
		if ((pinSetIdx == 0)||(pinSetIdx == 2))
			goto _kdCISModulePowerOn_exit_;
	}
#endif
#endif
//Modify camera main2 by Droi DXQ end 20170906
#if defined(DROI_PRO_F5C)
    if (((0==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_GC5005MIPI_SUB_RAW))) || ((1==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_GC5005MIPI_RAW))))
	{
		if (On)
			_hwPowerOn(VCAMIO, Vol_1800);
		else
			_hwPowerDown(VCAMIO);
		printk(KERN_ERR "#h#j#, GC5005 same sensor id\n");
		return -EIO;
	}
    if (((0==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_SP5506_SUB_MIPI_RAW))) || ((1==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_SP5506_MIPI_RAW))))
	{
		return 0xffffffff;
	}
    if (((0==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_GC5005MIPI_SUB_RAW))) || ((1==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_GC5005MIPI_RAW))))
    {
		return 0xffffffff;
	}
#endif
#if defined(TYD_PRO_F6_LM)
    if (((0==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_IMX190S_MIPI_RAW))) || ((1==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_IMX190_MIPI_RAW))))
	{
		if (On)
			_hwPowerOn(VCAMIO, Vol_1800);
		else
			_hwPowerDown(VCAMIO);
		printk(KERN_ERR "#h#j#, IMX190 same sensor id\n");
		return -EIO;
	}

    if (((0==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_S5K4E2GX_MIPI_RAW_SUB))) || ((1==pinSetIdx)&&(0==strcmp(currSensorName,SENSOR_DRVNAME_S5K4E2GX_MIPI_RAW))))
	{
		if (On)
			_hwPowerOn(VCAMIO, Vol_1800);
		else
			_hwPowerDown(VCAMIO);
		printk(KERN_ERR "#h#j#, S5K4E2GX same sensor id\n");
		return -EIO;
	}
#endif

	/* power ON */
	if (On)
	{

		PK_DBG("kdCISModulePowerOn -on:currSensorName=%s\n",currSensorName);
		PK_DBG("kdCISModulePowerOn -on:pinSetIdx=%d\n",pinSetIdx);

		for(pwListIdx=0 ; pwListIdx<50; pwListIdx++)
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

                		ISP_MCLK1_EN(1);


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
		for(pwListIdx=0 ; pwListIdx<50; pwListIdx++)
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


		        	ISP_MCLK1_EN(0);


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


