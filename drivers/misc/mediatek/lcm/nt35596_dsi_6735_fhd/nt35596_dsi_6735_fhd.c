#ifndef BUILD_LK
#include <linux/string.h>
#endif
#ifdef BUILD_LK
#include <string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
	#include <platform/mt_pmic.h>
	#include <platform/mt_i2c.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	//#include <mach/mt_gpio.h>
	//#include <linux/xlog.h>
	//#include <mach/mt_pm_ldo.h>
	#include <mt-plat/mt_gpio.h>
	#include <mach/gpio_const.h>
	#include <linux/gpio.h>
#endif

#define GPIO_65132_ENP (GPIO81 | 0x80000000)
#define GPIO_65132_ENN (GPIO86 | 0x80000000)
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (1080)
#define FRAME_HEIGHT (1920)

#define LCM_ID_NT35596 (0x96)

static const unsigned int BL_MIN_LEVEL = 20;

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	  lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

/* static unsigned char lcd_id_pins_value = 0xFF; */
static const unsigned char LCD_MODULE_ID = 0x01;
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define   LCM_DSI_CMD_MODE							0
#define REGFLAG_DELAY		0xFFFC
#define REGFLAG_UDELAY	0xFFFB
#define REGFLAG_END_OF_TABLE	0xFFFD
#define REGFLAG_RESET_LOW	0xFFFE
#define REGFLAG_RESET_HIGH	0xFFFF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};
//NT35596_LGD5.0_GAMMAM2.2_20150522 start
void TC358768_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];

	//data_array[0] =(0x00001500 | (para<<24) | (cmd<<16));
	//dsi_set_cmdq(data_array, 1, 1);

      data_array[0] =(0x00022902);
      data_array[1] =(0x00000000 | (para<<8) | (cmd));
      dsi_set_cmdq(data_array, 2, 1);

}

void TC358768_DCS_write_1A_0P(unsigned char cmd)
{
	unsigned int data_array[16];

	data_array[0]=(0x00000500 | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);
}

static void init_lcm_registers(void)
{
TC358768_DCS_write_1A_1P(0xFF,0xEE); ////2 cont
//TC358768_DCS_write_1A_1P(0xFB,0x01); //
TC358768_DCS_write_1A_1P(0x18,0x40);
MDELAY(10);
TC358768_DCS_write_1A_1P(0x18,0x00);
MDELAY(20);
TC358768_DCS_write_1A_1P(0xFF,0x00);
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
    MDELAY(10);

//LCD driver initialization
TC358768_DCS_write_1A_1P(0xFF, 0x05);
//TC358768_DCS_write_1A_1P(REGFLAG_DELAY, 1, TC358768_DCS_write_1A_1P();
TC358768_DCS_write_1A_1P(0xFB, 0x01);
TC358768_DCS_write_1A_1P(0xC5, 0x31);


//CMD2 P4
TC358768_DCS_write_1A_1P(0xFF, 0x05);
//TC358768_DCS_write_1A_1P(REGFLAG_DELAY, 1, TC358768_DCS_write_1A_1P();


TC358768_DCS_write_1A_1P(0x90, 0x00);

//FP/BP
TC358768_DCS_write_1A_1P(0x93, 0x04);
TC358768_DCS_write_1A_1P(0x94, 0x04);


TC358768_DCS_write_1A_1P(0x9B, 0x0F);
//Angus new add 2013/05/08
TC358768_DCS_write_1A_1P(0xA4, 0x0F);

TC358768_DCS_write_1A_1P(0x91, 0x44);
TC358768_DCS_write_1A_1P(0x92, 0x79);

//PIN MAP
TC358768_DCS_write_1A_1P(0x00,0x0F);
TC358768_DCS_write_1A_1P(0x01,0x00);
TC358768_DCS_write_1A_1P(0x02,0x00);
TC358768_DCS_write_1A_1P(0x03,0x00);
TC358768_DCS_write_1A_1P(0x04,0x0B);
TC358768_DCS_write_1A_1P(0x05,0x0C);
TC358768_DCS_write_1A_1P(0x06,0x00);
TC358768_DCS_write_1A_1P(0x07,0x00);
TC358768_DCS_write_1A_1P(0x08,0x00);
TC358768_DCS_write_1A_1P(0x09,0x00);
TC358768_DCS_write_1A_1P(0x0A,0x03);
TC358768_DCS_write_1A_1P(0x0B,0x04);
TC358768_DCS_write_1A_1P(0x0C,0x01);
TC358768_DCS_write_1A_1P(0x0D,0x13);
TC358768_DCS_write_1A_1P(0x0E,0x15);
TC358768_DCS_write_1A_1P(0x0F,0x17);

TC358768_DCS_write_1A_1P(0x10,0x0F);
TC358768_DCS_write_1A_1P(0x11,0x00);
TC358768_DCS_write_1A_1P(0x12,0x00);
TC358768_DCS_write_1A_1P(0x13,0x00);
TC358768_DCS_write_1A_1P(0x14,0x0B);
TC358768_DCS_write_1A_1P(0x15,0x0C);
TC358768_DCS_write_1A_1P(0x16,0x00);
TC358768_DCS_write_1A_1P(0x17,0x00);
TC358768_DCS_write_1A_1P(0x18,0x00);
TC358768_DCS_write_1A_1P(0x19,0x00);
TC358768_DCS_write_1A_1P(0x1A,0x03);
TC358768_DCS_write_1A_1P(0x1B,0x04);
TC358768_DCS_write_1A_1P(0x1C,0x01);
TC358768_DCS_write_1A_1P(0x1D,0x13);
TC358768_DCS_write_1A_1P(0x1E,0x15);
TC358768_DCS_write_1A_1P(0x1F,0x17);

//STV
TC358768_DCS_write_1A_1P(0x20,0x09);
TC358768_DCS_write_1A_1P(0x21,0x01);
TC358768_DCS_write_1A_1P(0x22,0x00);
TC358768_DCS_write_1A_1P(0x23,0x00);
TC358768_DCS_write_1A_1P(0x24,0x00);
TC358768_DCS_write_1A_1P(0x25,0x6D);

//GCK1,2
TC358768_DCS_write_1A_1P(0x2F,0x02);
TC358768_DCS_write_1A_1P(0x30,0x04);
TC358768_DCS_write_1A_1P(0x31,0x49);
TC358768_DCS_write_1A_1P(0x32,0x23);
TC358768_DCS_write_1A_1P(0x33,0x01);
TC358768_DCS_write_1A_1P(0x34,0x00);
TC358768_DCS_write_1A_1P(0x35,0x69);
TC358768_DCS_write_1A_1P(0x36,0x00);
TC358768_DCS_write_1A_1P(0x37,0x2D);
TC358768_DCS_write_1A_1P(0x38,0x08);

//U2D,D2U
TC358768_DCS_write_1A_1P(0x29, 0x58);
TC358768_DCS_write_1A_1P(0x2A, 0x16);
TC358768_DCS_write_1A_1P(0x2B, 0x0A);

TC358768_DCS_write_1A_1P(0x5B, 0x00);
TC358768_DCS_write_1A_1P(0x5F, 0x75);
TC358768_DCS_write_1A_1P(0x63, 0x00);
TC358768_DCS_write_1A_1P(0x67, 0x04);
TC358768_DCS_write_1A_1P(0x6C, 0x00);

TC358768_DCS_write_1A_1P(0x5C, 0x2C);
TC358768_DCS_write_1A_1P(0x60, 0x75);
TC358768_DCS_write_1A_1P(0x64, 0x00);
TC358768_DCS_write_1A_1P(0x68, 0x04);
TC358768_DCS_write_1A_1P(0x6C, 0x10);

TC358768_DCS_write_1A_1P(0x5D, 0x00);
TC358768_DCS_write_1A_1P(0x61, 0x75);
TC358768_DCS_write_1A_1P(0x65, 0x00);
TC358768_DCS_write_1A_1P(0x69, 0x04);
TC358768_DCS_write_1A_1P(0x6C, 0x00);

TC358768_DCS_write_1A_1P(0x6C, 0x10);

TC358768_DCS_write_1A_1P(0x7A, 0x01);
//MUX dummy
TC358768_DCS_write_1A_1P(0x7B, 0x80);
//TC358768_DCS_write_1A_1P(0x7B, 0x91);
TC358768_DCS_write_1A_1P(0x7C, 0xD8);
TC358768_DCS_write_1A_1P(0x7D, 0x60);
TC358768_DCS_write_1A_1P(0x7E, 0x07);
TC358768_DCS_write_1A_1P(0x7F, 0x17);
TC358768_DCS_write_1A_1P(0x80, 0x00);
TC358768_DCS_write_1A_1P(0x81, 0x05);
TC358768_DCS_write_1A_1P(0x82, 0x02);
TC358768_DCS_write_1A_1P(0x83, 0x00);
TC358768_DCS_write_1A_1P(0x84, 0x05);
TC358768_DCS_write_1A_1P(0x85, 0x05);
TC358768_DCS_write_1A_1P(0x86, 0x1B);
TC358768_DCS_write_1A_1P(0x87, 0x1B);
TC358768_DCS_write_1A_1P(0x88, 0x1B);
TC358768_DCS_write_1A_1P(0x89, 0x1B);
TC358768_DCS_write_1A_1P(0x8A, 0x33);
TC358768_DCS_write_1A_1P(0x8B, 0x00);
TC358768_DCS_write_1A_1P(0x8C, 0x00);

TC358768_DCS_write_1A_1P(0x73, 0xD0);
TC358768_DCS_write_1A_1P(0x74, 0x0D);
TC358768_DCS_write_1A_1P(0x75, 0x03);
TC358768_DCS_write_1A_1P(0x76, 0x16);

TC358768_DCS_write_1A_1P(0x99, 0x33);
TC358768_DCS_write_1A_1P(0x98, 0x00);

//*******************************************************************
//Power Related
TC358768_DCS_write_1A_1P(0xFF, 0x01);
//TC358768_DCS_write_1A_1P(REGFLAG_DELAY, 1, TC358768_DCS_write_1A_1P();


//Normal Black Panel
TC358768_DCS_write_1A_1P(0x00, 0x01);

//PUMP VGH=2xAVDD, VGL=2AVEE
TC358768_DCS_write_1A_1P(0x05, 0x50);

//EN_PRE_REG
TC358768_DCS_write_1A_1P(0x06, 0x4A);

//Enable VGHO/VGLO clamp function, and Set VGHO=10V, VGLO=-10V
TC358768_DCS_write_1A_1P(0x14, 0xA8);
TC358768_DCS_write_1A_1P(0x07, 0xB2);

//Regulator Enable, VGHO/VGLO setting, VGHO=9V, VGLO=-9V
TC358768_DCS_write_1A_1P(0x0E, 0xB5);
TC358768_DCS_write_1A_1P(0x0F, 0xB8);

//Gamma Voltage Setting GVDDP=3.9V, GVDDN=-3.9V
TC358768_DCS_write_1A_1P(0x0B, 0x69);//55, 50,69
TC358768_DCS_write_1A_1P(0x0C, 0x69);//55, 50,69

//VCOM setting, VCOM=-0.2V
TC358768_DCS_write_1A_1P(0x11, 0x1E);//13
TC358768_DCS_write_1A_1P(0x12, 0x1E);//13

//AVDDR/AVEER Setting (+/- 4.8V)
TC358768_DCS_write_1A_1P(0x08, 0x0C);
TC358768_DCS_write_1A_1P(0x15, 0x13);
TC358768_DCS_write_1A_1P(0x16, 0x13);


//ISOP
//TC358768_DCS_write_1A_1P(0x6D, 0x22);

//Gate EQ
//TC358768_DCS_write_1A_1P(0x58, 0x82);
//TC358768_DCS_write_1A_1P(0x59, 0x02);
//TC358768_DCS_write_1A_1P(0x5A, 0x02);
//TC358768_DCS_write_1A_1P(0x5B, 0x02);
//TC358768_DCS_write_1A_1P(0x5C, 0x82);
//TC358768_DCS_write_1A_1P(0x5D, 0x82);
//TC358768_DCS_write_1A_1P(0x5E, 0x02);
//TC358768_DCS_write_1A_1P(0x5F, 0x02);

//TC358768_DCS_write_1A_1P(0x58, 0x02);
//TC358768_DCS_write_1A_1P(0x5C, 0x02);
//TC358768_DCS_write_1A_1P(0x5D, 0x02);

TC358768_DCS_write_1A_1P(0xFF, 0xE0);

//TC358768_DCS_write_1A_1P(REGFLAG_DELAY, 1, TC358768_DCS_write_1A_1P();

TC358768_DCS_write_1A_1P(0x46, 0x63);
TC358768_DCS_write_1A_1P(0xFB, 0x01);


TC358768_DCS_write_1A_1P(0x69, 0x99); // \CC\F93D\B9\E2դ\B1ؼӼĴ\E6\C6\F7 2017-6-28


//*******************************************************************
//page selection cmd start
TC358768_DCS_write_1A_1P( 0xFF, 0x01);
//page selection cmd end
//R(+) MCR cmd
TC358768_DCS_write_1A_1P( 0x75,0x00);
TC358768_DCS_write_1A_1P( 0x76,0x00);
TC358768_DCS_write_1A_1P( 0x77,0x00);
TC358768_DCS_write_1A_1P( 0x78,0x08);
TC358768_DCS_write_1A_1P( 0x79,0x00);
TC358768_DCS_write_1A_1P( 0x7A,0x28);
TC358768_DCS_write_1A_1P( 0x7B,0x00);
TC358768_DCS_write_1A_1P( 0x7C,0x3B);
TC358768_DCS_write_1A_1P( 0x7D,0x00);
TC358768_DCS_write_1A_1P( 0x7E,0x5A);
TC358768_DCS_write_1A_1P( 0x7F,0x00);
TC358768_DCS_write_1A_1P( 0x80,0x65);
TC358768_DCS_write_1A_1P( 0x81,0x00);
TC358768_DCS_write_1A_1P( 0x82,0x7A);
TC358768_DCS_write_1A_1P( 0x83,0x00);
TC358768_DCS_write_1A_1P( 0x84,0x80);
TC358768_DCS_write_1A_1P( 0x85,0x00);
TC358768_DCS_write_1A_1P( 0x86,0x90);
TC358768_DCS_write_1A_1P( 0x87,0x00);
TC358768_DCS_write_1A_1P( 0x88,0xBC);
TC358768_DCS_write_1A_1P( 0x89,0x00);
TC358768_DCS_write_1A_1P( 0x8A,0xE0);
TC358768_DCS_write_1A_1P( 0x8B,0x01);
TC358768_DCS_write_1A_1P( 0x8C,0x24);
TC358768_DCS_write_1A_1P( 0x8D,0x01);
TC358768_DCS_write_1A_1P( 0x8E,0x50);
TC358768_DCS_write_1A_1P( 0x8F,0x01);
TC358768_DCS_write_1A_1P( 0x90,0x99);
TC358768_DCS_write_1A_1P( 0x91,0x01);
TC358768_DCS_write_1A_1P( 0x92,0xD6);
TC358768_DCS_write_1A_1P( 0x93,0x01);
TC358768_DCS_write_1A_1P( 0x94,0xD8);
TC358768_DCS_write_1A_1P( 0x95,0x02);
TC358768_DCS_write_1A_1P( 0x96,0x0F);
TC358768_DCS_write_1A_1P( 0x97,0x02);
TC358768_DCS_write_1A_1P( 0x98,0x4D);
TC358768_DCS_write_1A_1P( 0x99,0x02);
TC358768_DCS_write_1A_1P( 0x9A,0x75);
TC358768_DCS_write_1A_1P( 0x9B,0x02);
TC358768_DCS_write_1A_1P( 0x9C,0xC2);
TC358768_DCS_write_1A_1P( 0x9D,0x02);
TC358768_DCS_write_1A_1P( 0x9E,0xF7);
TC358768_DCS_write_1A_1P( 0x9F,0x03);
TC358768_DCS_write_1A_1P( 0xA0,0x2D);
TC358768_DCS_write_1A_1P( 0xA2,0x03);
TC358768_DCS_write_1A_1P( 0xA3,0x3D);
TC358768_DCS_write_1A_1P( 0xA4,0x03);
TC358768_DCS_write_1A_1P( 0xA5,0x4E);
TC358768_DCS_write_1A_1P( 0xA6,0x03);
TC358768_DCS_write_1A_1P( 0xA7,0x61);
TC358768_DCS_write_1A_1P( 0xA9,0x03);
TC358768_DCS_write_1A_1P( 0xAA,0x76);
TC358768_DCS_write_1A_1P( 0xAB,0x03);
TC358768_DCS_write_1A_1P( 0xAC,0x91);
TC358768_DCS_write_1A_1P( 0xAD,0x03);
TC358768_DCS_write_1A_1P( 0xAE,0xB1);
TC358768_DCS_write_1A_1P( 0xAF,0x03);
TC358768_DCS_write_1A_1P( 0xB0,0xCA);
TC358768_DCS_write_1A_1P( 0xB1,0x03);
TC358768_DCS_write_1A_1P( 0xB2,0xCC);
//R(-) MCR cmd
TC358768_DCS_write_1A_1P( 0xB3,0x00);
TC358768_DCS_write_1A_1P( 0xB4,0x00);
TC358768_DCS_write_1A_1P( 0xB5,0x00);
TC358768_DCS_write_1A_1P( 0xB6,0x08);
TC358768_DCS_write_1A_1P( 0xB7,0x00);
TC358768_DCS_write_1A_1P( 0xB8,0x28);
TC358768_DCS_write_1A_1P( 0xB9,0x00);
TC358768_DCS_write_1A_1P( 0xBA,0x3B);
TC358768_DCS_write_1A_1P( 0xBB,0x00);
TC358768_DCS_write_1A_1P( 0xBC,0x5A);
TC358768_DCS_write_1A_1P( 0xBD,0x00);
TC358768_DCS_write_1A_1P( 0xBE,0x65);
TC358768_DCS_write_1A_1P( 0xBF,0x00);
TC358768_DCS_write_1A_1P( 0xC0,0x7A);
TC358768_DCS_write_1A_1P( 0xC1,0x00);
TC358768_DCS_write_1A_1P( 0xC2,0x80);
TC358768_DCS_write_1A_1P( 0xC3,0x00);
TC358768_DCS_write_1A_1P( 0xC4,0x90);
TC358768_DCS_write_1A_1P( 0xC5,0x00);
TC358768_DCS_write_1A_1P( 0xC6,0xBC);
TC358768_DCS_write_1A_1P( 0xC7,0x00);
TC358768_DCS_write_1A_1P( 0xC8,0xE0);
TC358768_DCS_write_1A_1P( 0xC9,0x01);
TC358768_DCS_write_1A_1P( 0xCA,0x24);
TC358768_DCS_write_1A_1P( 0xCB,0x01);
TC358768_DCS_write_1A_1P( 0xCC,0x50);
TC358768_DCS_write_1A_1P( 0xCD,0x01);
TC358768_DCS_write_1A_1P( 0xCE,0x99);
TC358768_DCS_write_1A_1P( 0xCF,0x01);
TC358768_DCS_write_1A_1P( 0xD0,0xD6);
TC358768_DCS_write_1A_1P( 0xD1,0x01);
TC358768_DCS_write_1A_1P( 0xD2,0xD8);
TC358768_DCS_write_1A_1P( 0xD3,0x02);
TC358768_DCS_write_1A_1P( 0xD4,0x0F);
TC358768_DCS_write_1A_1P( 0xD5,0x02);
TC358768_DCS_write_1A_1P( 0xD6,0x4D);
TC358768_DCS_write_1A_1P( 0xD7,0x02);
TC358768_DCS_write_1A_1P( 0xD8,0x75);
TC358768_DCS_write_1A_1P( 0xD9,0x02);
TC358768_DCS_write_1A_1P( 0xDA,0xC2);
TC358768_DCS_write_1A_1P( 0xDB,0x02);
TC358768_DCS_write_1A_1P( 0xDC,0xF7);
TC358768_DCS_write_1A_1P( 0xDD,0x03);
TC358768_DCS_write_1A_1P( 0xDE,0x2D);
TC358768_DCS_write_1A_1P( 0xDF,0x03);
TC358768_DCS_write_1A_1P( 0xE0,0x3D);
TC358768_DCS_write_1A_1P( 0xE1,0x03);
TC358768_DCS_write_1A_1P( 0xE2,0x4E);
TC358768_DCS_write_1A_1P( 0xE3,0x03);
TC358768_DCS_write_1A_1P( 0xE4,0x61);
TC358768_DCS_write_1A_1P( 0xE5,0x03);
TC358768_DCS_write_1A_1P( 0xE6,0x76);
TC358768_DCS_write_1A_1P( 0xE7,0x03);
TC358768_DCS_write_1A_1P( 0xE8,0x91);
TC358768_DCS_write_1A_1P( 0xE9,0x03);
TC358768_DCS_write_1A_1P( 0xEA,0xB1);
TC358768_DCS_write_1A_1P( 0xEB,0x03);
TC358768_DCS_write_1A_1P( 0xEC,0xCA);
TC358768_DCS_write_1A_1P( 0xED,0x03);
TC358768_DCS_write_1A_1P( 0xEE,0xCC);
//G(+) MCR cmd
TC358768_DCS_write_1A_1P( 0xEF,0x00);
TC358768_DCS_write_1A_1P( 0xF0,0x00);
TC358768_DCS_write_1A_1P( 0xF1,0x00);
TC358768_DCS_write_1A_1P( 0xF2,0x08);
TC358768_DCS_write_1A_1P( 0xF3,0x00);
TC358768_DCS_write_1A_1P( 0xF4,0x28);
TC358768_DCS_write_1A_1P( 0xF5,0x00);
TC358768_DCS_write_1A_1P( 0xF6,0x3B);
TC358768_DCS_write_1A_1P( 0xF7,0x00);
TC358768_DCS_write_1A_1P( 0xF8,0x5A);
TC358768_DCS_write_1A_1P( 0xF9,0x00);
TC358768_DCS_write_1A_1P( 0xFA,0x65);
//page selection cmd start
TC358768_DCS_write_1A_1P( 0xFF, 0x02);
//page selection cmd end
TC358768_DCS_write_1A_1P( 0x00,0x00);
TC358768_DCS_write_1A_1P( 0x01,0x7A);
TC358768_DCS_write_1A_1P( 0x02,0x00);
TC358768_DCS_write_1A_1P( 0x03,0x80);
TC358768_DCS_write_1A_1P( 0x04,0x00);
TC358768_DCS_write_1A_1P( 0x05,0x90);
TC358768_DCS_write_1A_1P( 0x06,0x00);
TC358768_DCS_write_1A_1P( 0x07,0xBC);
TC358768_DCS_write_1A_1P( 0x08,0x00);
TC358768_DCS_write_1A_1P( 0x09,0xE0);
TC358768_DCS_write_1A_1P( 0x0A,0x01);
TC358768_DCS_write_1A_1P( 0x0B,0x24);
TC358768_DCS_write_1A_1P( 0x0C,0x01);
TC358768_DCS_write_1A_1P( 0x0D,0x50);
TC358768_DCS_write_1A_1P( 0x0E,0x01);
TC358768_DCS_write_1A_1P( 0x0F,0x99);
TC358768_DCS_write_1A_1P( 0x10,0x01);
TC358768_DCS_write_1A_1P( 0x11,0xD6);
TC358768_DCS_write_1A_1P( 0x12,0x01);
TC358768_DCS_write_1A_1P( 0x13,0xD8);
TC358768_DCS_write_1A_1P( 0x14,0x02);
TC358768_DCS_write_1A_1P( 0x15,0x0F);
TC358768_DCS_write_1A_1P( 0x16,0x02);
TC358768_DCS_write_1A_1P( 0x17,0x4D);
TC358768_DCS_write_1A_1P( 0x18,0x02);
TC358768_DCS_write_1A_1P( 0x19,0x75);
TC358768_DCS_write_1A_1P( 0x1A,0x02);
TC358768_DCS_write_1A_1P( 0x1B,0xC2);
TC358768_DCS_write_1A_1P( 0x1C,0x02);
TC358768_DCS_write_1A_1P( 0x1D,0xF7);
TC358768_DCS_write_1A_1P( 0x1E,0x03);
TC358768_DCS_write_1A_1P( 0x1F,0x2D);
TC358768_DCS_write_1A_1P( 0x20,0x03);
TC358768_DCS_write_1A_1P( 0x21,0x3D);
TC358768_DCS_write_1A_1P( 0x22,0x03);
TC358768_DCS_write_1A_1P( 0x23,0x4E);
TC358768_DCS_write_1A_1P( 0x24,0x03);
TC358768_DCS_write_1A_1P( 0x25,0x61);
TC358768_DCS_write_1A_1P( 0x26,0x03);
TC358768_DCS_write_1A_1P( 0x27,0x76);
TC358768_DCS_write_1A_1P( 0x28,0x03);
TC358768_DCS_write_1A_1P( 0x29,0x91);
TC358768_DCS_write_1A_1P( 0x2A,0x03);
TC358768_DCS_write_1A_1P( 0x2B,0xB1);
TC358768_DCS_write_1A_1P( 0x2D,0x03);
TC358768_DCS_write_1A_1P( 0x2F,0xCA);
TC358768_DCS_write_1A_1P( 0x30,0x03);
TC358768_DCS_write_1A_1P( 0x31,0xCC);
//G(-) MCR cmd
TC358768_DCS_write_1A_1P( 0x32,0x00);
TC358768_DCS_write_1A_1P( 0x33,0x00);
TC358768_DCS_write_1A_1P( 0x34,0x00);
TC358768_DCS_write_1A_1P( 0x35,0x08);
TC358768_DCS_write_1A_1P( 0x36,0x00);
TC358768_DCS_write_1A_1P( 0x37,0x28);
TC358768_DCS_write_1A_1P( 0x38,0x00);
TC358768_DCS_write_1A_1P( 0x39,0x3B);
TC358768_DCS_write_1A_1P( 0x3A,0x00);
TC358768_DCS_write_1A_1P( 0x3B,0x5A);
TC358768_DCS_write_1A_1P( 0x3D,0x00);
TC358768_DCS_write_1A_1P( 0x3F,0x65);
TC358768_DCS_write_1A_1P( 0x40,0x00);
TC358768_DCS_write_1A_1P( 0x41,0x7A);
TC358768_DCS_write_1A_1P( 0x42,0x00);
TC358768_DCS_write_1A_1P( 0x43,0x80);
TC358768_DCS_write_1A_1P( 0x44,0x00);
TC358768_DCS_write_1A_1P( 0x45,0x90);
TC358768_DCS_write_1A_1P( 0x46,0x00);
TC358768_DCS_write_1A_1P( 0x47,0xBC);
TC358768_DCS_write_1A_1P( 0x48,0x00);
TC358768_DCS_write_1A_1P( 0x49,0xE0);
TC358768_DCS_write_1A_1P( 0x4A,0x01);
TC358768_DCS_write_1A_1P( 0x4B,0x24);
TC358768_DCS_write_1A_1P( 0x4C,0x01);
TC358768_DCS_write_1A_1P( 0x4D,0x50);
TC358768_DCS_write_1A_1P( 0x4E,0x01);
TC358768_DCS_write_1A_1P( 0x4F,0x99);
TC358768_DCS_write_1A_1P( 0x50,0x01);
TC358768_DCS_write_1A_1P( 0x51,0xD6);
TC358768_DCS_write_1A_1P( 0x52,0x01);
TC358768_DCS_write_1A_1P( 0x53,0xD8);
TC358768_DCS_write_1A_1P( 0x54,0x02);
TC358768_DCS_write_1A_1P( 0x55,0x0F);
TC358768_DCS_write_1A_1P( 0x56,0x02);
TC358768_DCS_write_1A_1P( 0x58,0x4D);
TC358768_DCS_write_1A_1P( 0x59,0x02);
TC358768_DCS_write_1A_1P( 0x5A,0x75);
TC358768_DCS_write_1A_1P( 0x5B,0x02);
TC358768_DCS_write_1A_1P( 0x5C,0xC2);
TC358768_DCS_write_1A_1P( 0x5D,0x02);
TC358768_DCS_write_1A_1P( 0x5E,0xF7);
TC358768_DCS_write_1A_1P( 0x5F,0x03);
TC358768_DCS_write_1A_1P( 0x60,0x2D);
TC358768_DCS_write_1A_1P( 0x61,0x03);
TC358768_DCS_write_1A_1P( 0x62,0x3D);
TC358768_DCS_write_1A_1P( 0x63,0x03);
TC358768_DCS_write_1A_1P( 0x64,0x4E);
TC358768_DCS_write_1A_1P( 0x65,0x03);
TC358768_DCS_write_1A_1P( 0x66,0x61);
TC358768_DCS_write_1A_1P( 0x67,0x03);
TC358768_DCS_write_1A_1P( 0x68,0x76);
TC358768_DCS_write_1A_1P( 0x69,0x03);
TC358768_DCS_write_1A_1P( 0x6A,0x91);
TC358768_DCS_write_1A_1P( 0x6B,0x03);
TC358768_DCS_write_1A_1P( 0x6C,0xB1);
TC358768_DCS_write_1A_1P( 0x6D,0x03);
TC358768_DCS_write_1A_1P( 0x6E,0xCA);
TC358768_DCS_write_1A_1P( 0x6F,0x03);
TC358768_DCS_write_1A_1P( 0x70,0xCC);
//B(+) MCR cmd
TC358768_DCS_write_1A_1P( 0x71,0x00);
TC358768_DCS_write_1A_1P( 0x72,0x00);
TC358768_DCS_write_1A_1P( 0x73,0x00);
TC358768_DCS_write_1A_1P( 0x74,0x08);
TC358768_DCS_write_1A_1P( 0x75,0x00);
TC358768_DCS_write_1A_1P( 0x76,0x28);
TC358768_DCS_write_1A_1P( 0x77,0x00);
TC358768_DCS_write_1A_1P( 0x78,0x3B);
TC358768_DCS_write_1A_1P( 0x79,0x00);
TC358768_DCS_write_1A_1P( 0x7A,0x5A);
TC358768_DCS_write_1A_1P( 0x7B,0x00);
TC358768_DCS_write_1A_1P( 0x7C,0x65);
TC358768_DCS_write_1A_1P( 0x7D,0x00);
TC358768_DCS_write_1A_1P( 0x7E,0x7A);
TC358768_DCS_write_1A_1P( 0x7F,0x00);
TC358768_DCS_write_1A_1P( 0x80,0x80);
TC358768_DCS_write_1A_1P( 0x81,0x00);
TC358768_DCS_write_1A_1P( 0x82,0x90);
TC358768_DCS_write_1A_1P( 0x83,0x00);
TC358768_DCS_write_1A_1P( 0x84,0xBC);
TC358768_DCS_write_1A_1P( 0x85,0x00);
TC358768_DCS_write_1A_1P( 0x86,0xE0);
TC358768_DCS_write_1A_1P( 0x87,0x01);
TC358768_DCS_write_1A_1P( 0x88,0x24);
TC358768_DCS_write_1A_1P( 0x89,0x01);
TC358768_DCS_write_1A_1P( 0x8A,0x50);
TC358768_DCS_write_1A_1P( 0x8B,0x01);
TC358768_DCS_write_1A_1P( 0x8C,0x99);
TC358768_DCS_write_1A_1P( 0x8D,0x01);
TC358768_DCS_write_1A_1P( 0x8E,0xD6);
TC358768_DCS_write_1A_1P( 0x8F,0x01);
TC358768_DCS_write_1A_1P( 0x90,0xD8);
TC358768_DCS_write_1A_1P( 0x91,0x02);
TC358768_DCS_write_1A_1P( 0x92,0x0F);
TC358768_DCS_write_1A_1P( 0x93,0x02);
TC358768_DCS_write_1A_1P( 0x94,0x4D);
TC358768_DCS_write_1A_1P( 0x95,0x02);
TC358768_DCS_write_1A_1P( 0x96,0x75);
TC358768_DCS_write_1A_1P( 0x97,0x02);
TC358768_DCS_write_1A_1P( 0x98,0xC2);
TC358768_DCS_write_1A_1P( 0x99,0x02);
TC358768_DCS_write_1A_1P( 0x9A,0xF7);
TC358768_DCS_write_1A_1P( 0x9B,0x03);
TC358768_DCS_write_1A_1P( 0x9C,0x2D);
TC358768_DCS_write_1A_1P( 0x9D,0x03);
TC358768_DCS_write_1A_1P( 0x9E,0x3D);
TC358768_DCS_write_1A_1P( 0x9F,0x03);
TC358768_DCS_write_1A_1P( 0xA0,0x4E);
TC358768_DCS_write_1A_1P( 0xA2,0x03);
TC358768_DCS_write_1A_1P( 0xA3,0x61);
TC358768_DCS_write_1A_1P( 0xA4,0x03);
TC358768_DCS_write_1A_1P( 0xA5,0x76);
TC358768_DCS_write_1A_1P( 0xA6,0x03);
TC358768_DCS_write_1A_1P( 0xA7,0x91);
TC358768_DCS_write_1A_1P( 0xA9,0x03);
TC358768_DCS_write_1A_1P( 0xAA,0xB1);
TC358768_DCS_write_1A_1P( 0xAB,0x03);
TC358768_DCS_write_1A_1P( 0xAC,0xCA);
TC358768_DCS_write_1A_1P( 0xAD,0x03);
TC358768_DCS_write_1A_1P( 0xAE,0xCC);
//B(-) MCR cmd
TC358768_DCS_write_1A_1P( 0xAF,0x00);
TC358768_DCS_write_1A_1P( 0xB0,0x00);
TC358768_DCS_write_1A_1P( 0xB1,0x00);
TC358768_DCS_write_1A_1P( 0xB2,0x08);
TC358768_DCS_write_1A_1P( 0xB3,0x00);
TC358768_DCS_write_1A_1P( 0xB4,0x28);
TC358768_DCS_write_1A_1P( 0xB5,0x00);
TC358768_DCS_write_1A_1P( 0xB6,0x3B);
TC358768_DCS_write_1A_1P( 0xB7,0x00);
TC358768_DCS_write_1A_1P( 0xB8,0x5A);
TC358768_DCS_write_1A_1P( 0xB9,0x00);
TC358768_DCS_write_1A_1P( 0xBA,0x65);
TC358768_DCS_write_1A_1P( 0xBB,0x00);
TC358768_DCS_write_1A_1P( 0xBC,0x7A);
TC358768_DCS_write_1A_1P( 0xBD,0x00);
TC358768_DCS_write_1A_1P( 0xBE,0x80);
TC358768_DCS_write_1A_1P( 0xBF,0x00);
TC358768_DCS_write_1A_1P( 0xC0,0x90);
TC358768_DCS_write_1A_1P( 0xC1,0x00);
TC358768_DCS_write_1A_1P( 0xC2,0xBC);
TC358768_DCS_write_1A_1P( 0xC3,0x00);
TC358768_DCS_write_1A_1P( 0xC4,0xE0);
TC358768_DCS_write_1A_1P( 0xC5,0x01);
TC358768_DCS_write_1A_1P( 0xC6,0x24);
TC358768_DCS_write_1A_1P( 0xC7,0x01);
TC358768_DCS_write_1A_1P( 0xC8,0x50);
TC358768_DCS_write_1A_1P( 0xC9,0x01);
TC358768_DCS_write_1A_1P( 0xCA,0x99);
TC358768_DCS_write_1A_1P( 0xCB,0x01);
TC358768_DCS_write_1A_1P( 0xCC,0xD6);
TC358768_DCS_write_1A_1P( 0xCD,0x01);
TC358768_DCS_write_1A_1P( 0xCE,0xD8);
TC358768_DCS_write_1A_1P( 0xCF,0x02);
TC358768_DCS_write_1A_1P( 0xD0,0x0F);
TC358768_DCS_write_1A_1P( 0xD1,0x02);
TC358768_DCS_write_1A_1P( 0xD2,0x4D);
TC358768_DCS_write_1A_1P( 0xD3,0x02);
TC358768_DCS_write_1A_1P( 0xD4,0x75);
TC358768_DCS_write_1A_1P( 0xD5,0x02);
TC358768_DCS_write_1A_1P( 0xD6,0xC2);
TC358768_DCS_write_1A_1P( 0xD7,0x02);
TC358768_DCS_write_1A_1P( 0xD8,0xF7);
TC358768_DCS_write_1A_1P( 0xD9,0x03);
TC358768_DCS_write_1A_1P( 0xDA,0x2D);
TC358768_DCS_write_1A_1P( 0xDB,0x03);
TC358768_DCS_write_1A_1P( 0xDC,0x3D);
TC358768_DCS_write_1A_1P( 0xDD,0x03);
TC358768_DCS_write_1A_1P( 0xDE,0x4E);
TC358768_DCS_write_1A_1P( 0xDF,0x03);
TC358768_DCS_write_1A_1P( 0xE0,0x61);
TC358768_DCS_write_1A_1P( 0xE1,0x03);
TC358768_DCS_write_1A_1P( 0xE2,0x76);
TC358768_DCS_write_1A_1P( 0xE3,0x03);
TC358768_DCS_write_1A_1P( 0xE4,0x91);
TC358768_DCS_write_1A_1P( 0xE5,0x03);
TC358768_DCS_write_1A_1P( 0xE6,0xB1);
TC358768_DCS_write_1A_1P( 0xE7,0x03);
TC358768_DCS_write_1A_1P( 0xE8,0xCA);
TC358768_DCS_write_1A_1P( 0xE9,0x03);
TC358768_DCS_write_1A_1P( 0xEA,0xCC);

//CMD1
TC358768_DCS_write_1A_1P(0xFF, 0x00);
//VBP=8,VFP=8
TC358768_DCS_write_1A_1P(0xD3, 0x08);
TC358768_DCS_write_1A_1P(0xD4, 0x08);

TC358768_DCS_write_1A_0P(0x11);
MDELAY(120);
TC358768_DCS_write_1A_0P(0x29);
MDELAY(20);
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
#endif
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch = 6;//10
	params->dsi.vertical_frontporch = 6;//6
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 10;
	params->dsi.horizontal_backporch = 60;//50
	params->dsi.horizontal_frontporch = 60;//50
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	/* params->dsi.ssc_disable                                                   = 1; */
	params->dsi.PLL_CLOCK = 450;	/* this value must be in MTK suggested table */ //460

	//params->dsi.CLK_HS_POST = 36;
	//params->dsi.clk_lp_per_line_enable = 0;
	//params->dsi.esd_check_enable = 1;
	//params->dsi.customization_esd_check_enable = 0;
	  params->dsi.lcm_esd_check_table[0].cmd = 0x53;
	  params->dsi.lcm_esd_check_table[0].count = 1;
	  params->dsi.lcm_esd_check_table[0].para_list[0] = 0x24;

}


static void lcm_init_power(void)
{
	mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);//IOVCC,1.8V
	mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
	MDELAY(10);

	mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);//AVDD,AVEE
	mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
	MDELAY(10);

}

static void lcm_suspend_power(void)
{
	mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);//AVDD,AVEE
	mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ZERO);
	MDELAY(10);

	mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);//IOVCC,1.8V
	mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ZERO);
	MDELAY(10);

}

static void lcm_resume_power(void)
{
	mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);//IOVCC,1.8V
	mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
	MDELAY(10);

	mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);//AVDD,AVEE
	mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
	MDELAY(10);

}


static void lcm_init(void)
{
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
	SET_RESET_PIN(1);
    MDELAY(20);


	//push_table(init_setting, sizeof(init_setting) / sizeof(struct LCM_setting_table), 1);// NT35596+auo
	//init_lcm_registers(); // NT35596+LG
	//push_table(lcm_initialization_setting_lg, sizeof(lcm_initialization_setting_lg) / sizeof(struct LCM_setting_table), 1);// NT35596+LG ,hct_nt35596_dsi_vdo_fhd_lg.c

	init_lcm_registers(); // NT35596+LG
}

static void lcm_suspend(void)
{
	//push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);
	/* SET_RESET_PIN(0); */
	//MDELAY(10);
	unsigned int data_array[16];

    data_array[0]=0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);

    data_array[0] = 0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);

    //SET_RESET_PIN(1);
    //MDELAY(20);
    //SET_RESET_PIN(0);
    //MDELAY(50);
}

static void lcm_resume(void)
{
	lcm_init();
}
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif
static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[2];
	unsigned int array[16];

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(100);

	array[0] = 0x00023700;	// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; 	//we only need ID

	//printf("%s,nt35596_fhd_dsi_vdo_auo.c id = 0x%08x\n", __func__, id);

	if (id == 0x96)
		return 1;
	else
		return 0;

}


/* return TRUE: need recovery */
/* return FALSE: No need recovery */
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
	char buffer[3];
	int array[4];

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x53, buffer, 1);

	if (buffer[0] != 0x24) {
		//printf("[LCM ERROR] [0x53]=0x%02x\n", buffer[0]);
		return TRUE;
	} else {
		//printf("[LCM NORMAL] [0x53]=0x%02x\n", buffer[0]);
		return FALSE;
	}
#else
	return FALSE;
#endif

}


LCM_DRIVER nt35596_fhd_dsi_vdo_auo_lcm_drv = {
	.name = "nt35596_fhd_dsi_vdo_auo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.esd_check = lcm_esd_check,
	#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
	#endif
	//.switch_mode = lcm_switch_mode,
};
