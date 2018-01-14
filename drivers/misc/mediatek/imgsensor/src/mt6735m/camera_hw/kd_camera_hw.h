#ifndef _KD_CAMERA_HW_H_
#define _KD_CAMERA_HW_H_

#include <linux/types.h>

#if 0 //defined(TYD_PRO_M9)
//AVDD  (GPIO126| 0X80000000)
//DVDD  (GPIO125| 0X80000000)
#define CAMERA_POWER_VCAM_AVDD 
#define CAMERA_POWER_VCAM_DVDD 
#endif

#if defined(DROI_PRO_PU6)
#define CAMERA_MAIN2_GPIO_SUPPORT //Modify camera main2 by Droi DXQ  20170906
#endif

#define GPIO_UNSUPPORTED -1
#define CAMERA_CMRST_PIN            0
#define CAMERA_CMRST_PIN_M_GPIO     0

#define CAMERA_CMPDN_PIN            0
#define CAMERA_CMPDN_PIN_M_GPIO     0

/* FRONT sensor */
#define CAMERA_CMRST1_PIN           0
#define CAMERA_CMRST1_PIN_M_GPIO    0

#define CAMERA_CMPDN1_PIN           0
#define CAMERA_CMPDN1_PIN_M_GPIO    0

/* MAIN2 sensor */
#define CAMERA_CMRST2_PIN           0
#define CAMERA_CMRST2_PIN_M_GPIO    0

#define CAMERA_CMPDN2_PIN           0
#define CAMERA_CMPDN2_PIN_M_GPIO    0

#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0

#define VOL_2800 2800000
#define VOL_1800 1800000
#define VOL_1500 1500000
#define VOL_1200 1200000
#define VOL_1000 1000000


typedef enum {
	CAMPDN,
	CAMRST,
	CAM1PDN,
	CAM1RST,
	CAMLDO_AVDD,
    CAMLDO_DVDD
} CAMPowerType;


typedef enum{
	VDD_None,
	PDN,
	RST,
	VCAMA,
	VCAMD,
	VCAMIO,
	VCAMAF,
	AF_EN,
	SUB_AF_EN,
	SensorMCLK,
	VRF18_1,
#if defined(CAMERA_MAIN2_GPIO_SUPPORT)
	MIPI_SEL,
#endif
}PowerType;

typedef enum{
	Vol_Low =0,
	Vol_High=1,
    Vol_1000 = 1000000,
    Vol_1200 = 1200000,	  
    Vol_1500 = 1500000,    
    Vol_1800 = 1800000,     
    Vol_2800 = 2800000 
}Voltage;


typedef struct{
	PowerType PowerType;
	Voltage Voltage;
	u32 Delay;
}PowerInformation;


typedef struct{
	char* SensorName;
	PowerInformation PowerInfo[12];
}PowerSequence;

typedef struct{
	PowerSequence PowerSeq[50];
}PowerUp;

typedef struct{
	u32 Gpio_Pin;  
	u32 Gpio_Mode;
	Voltage Voltage;
}PowerCustInfo;

typedef struct{
	PowerCustInfo PowerCustInfo[6];
}PowerCust;

extern void ISP_MCLK1_EN(bool En);

extern bool _hwPowerDown(PowerType type);
extern bool _hwPowerOn(PowerType type, int powerVolt);

int mtkcam_gpio_set(int PinIdx, int PwrType, int Val);
int mtkcam_gpio_init(struct platform_device *pdev);

#endif
