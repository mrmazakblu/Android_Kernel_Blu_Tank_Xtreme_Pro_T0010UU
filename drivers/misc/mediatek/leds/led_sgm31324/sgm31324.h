#ifndef __GRADUAL_LED_DRV_H__
#define __GRADUAL_LED_DRV_H__

// SGM31324 api ++
#define SGM31324_SDB_PIN	8 	//SDB pin GPIO define


#define LED_RGB_NUM1         0x000100
#define LED_RGB_NUM2 		 0x000200
#define LED_RGB_NUM3         0x000400
#define LED_RGB_NUM4         0x000800
#define LED_RGB_NUM5 		 0x001000
#define LED_RGB_NUM6         0x002000
#define LED_RGB_NUM7         0x004000
#define LED_RGB_NUM8 		 0x008000
#define LED_RGB_NUM9         0x010000
#define LED_RGB_NUM10        0x020000
#define LED_RGB_NUM11 		 0x040000
#define LED_RGB_NUM12        0x080000
#define LED_RGB_NUM13        0x100000
#define LED_RGB_NUM14 		 0x200000
#define LED_RGB_NUM15 		   0x400000
#define LED_RGB_NUM16 		   0x800000
#define LED_RGB_NUM17 		  0x1000000





#define SGM31324_DEV_NAME			"SGM31324"
#define SGM31324_I2C_NAME    		SGM31324_DEV_NAME
#define SGM31324_I2C_ADDR    		(0x30)
#define REG_BRIGHTNESS_START    0x24
#define REG_BRIGHTNESS_END      0xB3

#define REG_LED_ENABLE_START    0x00
#define REG_LED_ENABLE_END      0x11
#define RGB_EXT_NUM_MAX         17
//
enum LEG_RGB_EXT_TYPE
{
	RGB_EXT_R=0,
	RGB_EXT_G,
	RGB_EXT_B,
	RGB_EXT_MAX
};
//
typedef struct
{
	unsigned char addr;
	unsigned char value;
}SGM31324_REG_STRUCT;
#endif
