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
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (1080)
#define FRAME_HEIGHT (1920)

#define LCM_ID_NT35532 (0x80)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define dsi_lcm_set_gpio_out(pin, out)										lcm_util.set_gpio_out(pin, out)
#define dsi_lcm_set_gpio_mode(pin, mode)									lcm_util.set_gpio_mode(pin, mode)
#define dsi_lcm_set_gpio_dir(pin, dir)										lcm_util.set_gpio_dir(pin, dir)
#define dsi_lcm_set_gpio_pull_enable(pin, en)								lcm_util.set_gpio_pull_enable)(pin, en)

#define   LCM_DSI_CMD_MODE							0

static bool lcm_is_init = false;

void NT35532_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];
	//unsigned char buffer;

#if 0//ndef BUILD_LK

	do {
		data_array[0] =(0x00001500 | (para<<24) | (cmd<<16));
		dsi_set_cmdq(data_array, 1, 1);

		if (cmd == 0xFF)
			break;

		read_reg_v2(cmd, &buffer, 1);

		if(buffer != para)
			printk("%s, data_array = 0x%08x, (cmd, para, back) = (0x%02x, 0x%02x, 0x%02x)\n", __func__, data_array[0], cmd, para, buffer);

		MDELAY(1);

	} while (buffer != para);

#else

	data_array[0] =(0x00001500 | (para<<24) | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

	//MDELAY(1);

#endif

}

/*static void NT35532_DCS_write_1A_0P(unsigned char cmd)
{
	unsigned int data_array[16];

	data_array[0]=(0x00000500 | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

}*/

#if defined(DROI_PRO_FQ5B)
#include "nt35532_dsi_6735_fhd_fq5b.h"
#endif

unsigned int data_array[16]={0};
#define NT35532_DCS_write_1A_0P(cmd)							data_array[0]=(0x00000500 | (cmd<<16)); \
																dsi_set_cmdq(data_array, 1, 1);

#if defined(SUPORT_ADC_CHECK)
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static int lcm_read_ADC_value(void)
{
	int val;           //lvl = LCM_V_LEVEL;
	int dwChannel = 12; //LCM_ADC_CHAN;

	int data[4] = {0,0,0,0};
	int data0;
	//char* buf_temp;
	int res =0;
	//unsigned int ret;
	int num1_10;
	int num1_1;
	int num2_10;
	int num2_1;

    //read and calculate ADC value
	res = IMM_GetOneChannelValue(dwChannel,data,NULL);
	num1_10=data[0]/10;
	num1_1=data[0]%10;
	if(data[1]<10)
	{
	    num2_10=(data[1] * 10) / 10;
	    num2_1=(data[1] * 10) % 10;
	    data0=num1_10*1000+num1_1*100+num2_10*10+num2_1;
	    val=data0*100;
	}
	else
	{
	    num2_10=data[1] / 10;
	    num2_1=data[1] % 10;
	    data0=num1_10*1000+num1_1*100+num2_10*10+num2_1;
	    val=data0*10;
	}
    return val;
}

static void adc_lcm_registers(int val)
{
#if defined(DROI_PRO_FQ5B_ZGW)||defined(DROI_PRO_FQ5B_ZGW2)
	if(val == 0)   //<200
	{
        init_lcm_registers_0000();      //voltage= 0V
	}
	else
	{
        init_lcm_registers_1200();      //voltage=1.2V
	}

    //printk("cuizhaojun:%d\n",val);
#endif
}
#endif

#ifndef DROI_LCD_USE_CUSTOM_NT35532_FHD
static void init_lcm_registers(void)
{

NT35532_DCS_write_1A_1P(0xFF,0x01);



NT35532_DCS_write_1A_1P(0xFB,0x01);

NT35532_DCS_write_1A_1P(0x6E,0x80);

NT35532_DCS_write_1A_1P(0x00,0x01);
NT35532_DCS_write_1A_1P(0x01,0x55);
NT35532_DCS_write_1A_1P(0x02,0x59);
NT35532_DCS_write_1A_1P(0x04,0x0C);
NT35532_DCS_write_1A_1P(0x05,0x3B);
NT35532_DCS_write_1A_1P(0x06,0x6B);
NT35532_DCS_write_1A_1P(0x07,0xC2);
NT35532_DCS_write_1A_1P(0x0D,0x89);
NT35532_DCS_write_1A_1P(0x0E,0x89);
NT35532_DCS_write_1A_1P(0x0F,0x60);
NT35532_DCS_write_1A_1P(0x10,0x03);
NT35532_DCS_write_1A_1P(0x11,0x66);
NT35532_DCS_write_1A_1P(0x12,0x5B);
NT35532_DCS_write_1A_1P(0x13,0x7C);
NT35532_DCS_write_1A_1P(0x14,0x7C);
NT35532_DCS_write_1A_1P(0x15,0x60);
NT35532_DCS_write_1A_1P(0x16,0x10);
NT35532_DCS_write_1A_1P(0x17,0x10);

NT35532_DCS_write_1A_1P(0x1C,0xA3);




NT35532_DCS_write_1A_1P(0x75,0x00);
NT35532_DCS_write_1A_1P(0x76,0x56);
NT35532_DCS_write_1A_1P(0x77,0x00);
NT35532_DCS_write_1A_1P(0x78,0x62);
NT35532_DCS_write_1A_1P(0x79,0x00);
NT35532_DCS_write_1A_1P(0x7A,0x6D);
NT35532_DCS_write_1A_1P(0x7B,0x00);
NT35532_DCS_write_1A_1P(0x7C,0x89);
NT35532_DCS_write_1A_1P(0x7D,0x00);
NT35532_DCS_write_1A_1P(0x7E,0xA0);
NT35532_DCS_write_1A_1P(0x7F,0x00);
NT35532_DCS_write_1A_1P(0x80,0xB4);
NT35532_DCS_write_1A_1P(0x81,0x00);
NT35532_DCS_write_1A_1P(0x82,0xC7);
NT35532_DCS_write_1A_1P(0x83,0x00);
NT35532_DCS_write_1A_1P(0x84,0xD6);
NT35532_DCS_write_1A_1P(0x85,0x00);
NT35532_DCS_write_1A_1P(0x86,0xE4);
NT35532_DCS_write_1A_1P(0x87,0x01);
NT35532_DCS_write_1A_1P(0x88,0x16);
NT35532_DCS_write_1A_1P(0x89,0x01);
NT35532_DCS_write_1A_1P(0x8A,0x3B);
NT35532_DCS_write_1A_1P(0x8B,0x01);
NT35532_DCS_write_1A_1P(0x8C,0x78);
NT35532_DCS_write_1A_1P(0x8D,0x01);
NT35532_DCS_write_1A_1P(0x8E,0xA8);
NT35532_DCS_write_1A_1P(0x8F,0x01);
NT35532_DCS_write_1A_1P(0x90,0xF3);
NT35532_DCS_write_1A_1P(0x91,0x02);
NT35532_DCS_write_1A_1P(0x92,0x30);
NT35532_DCS_write_1A_1P(0x93,0x02);
NT35532_DCS_write_1A_1P(0x94,0x37);
NT35532_DCS_write_1A_1P(0x95,0x02);
NT35532_DCS_write_1A_1P(0x96,0x6C);
NT35532_DCS_write_1A_1P(0x97,0x02);
NT35532_DCS_write_1A_1P(0x98,0xA3);
NT35532_DCS_write_1A_1P(0x99,0x02);
NT35532_DCS_write_1A_1P(0x9A,0xC5);
NT35532_DCS_write_1A_1P(0x9B,0x02);
NT35532_DCS_write_1A_1P(0x9C,0xF6);
NT35532_DCS_write_1A_1P(0x9D,0x03);
NT35532_DCS_write_1A_1P(0x9E,0x15);
NT35532_DCS_write_1A_1P(0x9F,0x03);
NT35532_DCS_write_1A_1P(0xA0,0x3B);
NT35532_DCS_write_1A_1P(0xA2,0x03);
NT35532_DCS_write_1A_1P(0xA3,0x4C);
NT35532_DCS_write_1A_1P(0xA4,0x03);
NT35532_DCS_write_1A_1P(0xA5,0x57);
NT35532_DCS_write_1A_1P(0xA6,0x03);
NT35532_DCS_write_1A_1P(0xA7,0x68);
NT35532_DCS_write_1A_1P(0xA9,0x03);
NT35532_DCS_write_1A_1P(0xAA,0x79);
NT35532_DCS_write_1A_1P(0xAB,0x03);
NT35532_DCS_write_1A_1P(0xAC,0x8D);
NT35532_DCS_write_1A_1P(0xAD,0x03);
NT35532_DCS_write_1A_1P(0xAE,0xA3);
NT35532_DCS_write_1A_1P(0xAF,0x03);
NT35532_DCS_write_1A_1P(0xB0,0xBB);
NT35532_DCS_write_1A_1P(0xB1,0x03);
NT35532_DCS_write_1A_1P(0xB2,0xBF);



NT35532_DCS_write_1A_1P(0xB3,0x00);
NT35532_DCS_write_1A_1P(0xB4,0x08);
NT35532_DCS_write_1A_1P(0xB5,0x00);
NT35532_DCS_write_1A_1P(0xB6,0x36);
NT35532_DCS_write_1A_1P(0xB7,0x00);
NT35532_DCS_write_1A_1P(0xB8,0x7B);
NT35532_DCS_write_1A_1P(0xB9,0x00);
NT35532_DCS_write_1A_1P(0xBA,0x96);
NT35532_DCS_write_1A_1P(0xBB,0x00);
NT35532_DCS_write_1A_1P(0xBC,0xAD);
NT35532_DCS_write_1A_1P(0xBD,0x00);
NT35532_DCS_write_1A_1P(0xBE,0xC1);
NT35532_DCS_write_1A_1P(0xBF,0x00);
NT35532_DCS_write_1A_1P(0xC0,0xD4);
NT35532_DCS_write_1A_1P(0xC1,0x00);
NT35532_DCS_write_1A_1P(0xC2,0xE2);
NT35532_DCS_write_1A_1P(0xC3,0x00);
NT35532_DCS_write_1A_1P(0xC4,0xF1);
NT35532_DCS_write_1A_1P(0xC5,0x01);
NT35532_DCS_write_1A_1P(0xC6,0x23);
NT35532_DCS_write_1A_1P(0xC7,0x01);
NT35532_DCS_write_1A_1P(0xC8,0x48);
NT35532_DCS_write_1A_1P(0xC9,0x01);
NT35532_DCS_write_1A_1P(0xCA,0x85);
NT35532_DCS_write_1A_1P(0xCB,0x01);
NT35532_DCS_write_1A_1P(0xCC,0xB5);
NT35532_DCS_write_1A_1P(0xCD,0x02);
NT35532_DCS_write_1A_1P(0xCE,0x00);
NT35532_DCS_write_1A_1P(0xCF,0x02);
NT35532_DCS_write_1A_1P(0xD0,0x3D);
NT35532_DCS_write_1A_1P(0xD1,0x02);
NT35532_DCS_write_1A_1P(0xD2,0x37);
NT35532_DCS_write_1A_1P(0xD3,0x02);
NT35532_DCS_write_1A_1P(0xD4,0x70);
NT35532_DCS_write_1A_1P(0xD5,0x02);
NT35532_DCS_write_1A_1P(0xD6,0xAF);
NT35532_DCS_write_1A_1P(0xD7,0x02);
NT35532_DCS_write_1A_1P(0xD8,0xD7);
NT35532_DCS_write_1A_1P(0xD9,0x03);
NT35532_DCS_write_1A_1P(0xDA,0x0B);
NT35532_DCS_write_1A_1P(0xDB,0x03);
NT35532_DCS_write_1A_1P(0xDC,0x32);
NT35532_DCS_write_1A_1P(0xDD,0x03);
NT35532_DCS_write_1A_1P(0xDE,0x5D);
NT35532_DCS_write_1A_1P(0xDF,0x03);
NT35532_DCS_write_1A_1P(0xE0,0x69);
NT35532_DCS_write_1A_1P(0xE1,0x03);
NT35532_DCS_write_1A_1P(0xE2,0x79);
NT35532_DCS_write_1A_1P(0xE3,0x03);
NT35532_DCS_write_1A_1P(0xE4,0x8A);
NT35532_DCS_write_1A_1P(0xE5,0x03);
NT35532_DCS_write_1A_1P(0xE6,0x9B);
NT35532_DCS_write_1A_1P(0xE7,0x03);
NT35532_DCS_write_1A_1P(0xE8,0xAF);
NT35532_DCS_write_1A_1P(0xE9,0x03);
NT35532_DCS_write_1A_1P(0xEA,0xC5);
NT35532_DCS_write_1A_1P(0xEB,0x03);
NT35532_DCS_write_1A_1P(0xEC,0xDD);
NT35532_DCS_write_1A_1P(0xED,0x03);
NT35532_DCS_write_1A_1P(0xEE,0xE1);



NT35532_DCS_write_1A_1P(0xEF,0x00);
NT35532_DCS_write_1A_1P(0xF0,0x56);
NT35532_DCS_write_1A_1P(0xF1,0x00);
NT35532_DCS_write_1A_1P(0xF2,0x62);
NT35532_DCS_write_1A_1P(0xF3,0x00);
NT35532_DCS_write_1A_1P(0xF4,0x6D);
NT35532_DCS_write_1A_1P(0xF5,0x00);
NT35532_DCS_write_1A_1P(0xF6,0x89);
NT35532_DCS_write_1A_1P(0xF7,0x00);
NT35532_DCS_write_1A_1P(0xF8,0xA0);
NT35532_DCS_write_1A_1P(0xF9,0x00);
NT35532_DCS_write_1A_1P(0xFA,0xB4);



NT35532_DCS_write_1A_1P(0xFF,0x02);



NT35532_DCS_write_1A_1P(0xFB,0x01);



NT35532_DCS_write_1A_1P(0x00,0x00);
NT35532_DCS_write_1A_1P(0x01,0xC7);
NT35532_DCS_write_1A_1P(0x02,0x00);
NT35532_DCS_write_1A_1P(0x03,0xD6);
NT35532_DCS_write_1A_1P(0x04,0x00);
NT35532_DCS_write_1A_1P(0x05,0xE4);
NT35532_DCS_write_1A_1P(0x06,0x01);
NT35532_DCS_write_1A_1P(0x07,0x16);
NT35532_DCS_write_1A_1P(0x08,0x01);
NT35532_DCS_write_1A_1P(0x09,0x3B);
NT35532_DCS_write_1A_1P(0x0A,0x01);
NT35532_DCS_write_1A_1P(0x0B,0x78);
NT35532_DCS_write_1A_1P(0x0C,0x01);
NT35532_DCS_write_1A_1P(0x0D,0xA8);
NT35532_DCS_write_1A_1P(0x0E,0x01);
NT35532_DCS_write_1A_1P(0x0F,0xF3);
NT35532_DCS_write_1A_1P(0x10,0x02);
NT35532_DCS_write_1A_1P(0x11,0x30);
NT35532_DCS_write_1A_1P(0x12,0x02);
NT35532_DCS_write_1A_1P(0x13,0x37);
NT35532_DCS_write_1A_1P(0x14,0x02);
NT35532_DCS_write_1A_1P(0x15,0x6C);
NT35532_DCS_write_1A_1P(0x16,0x02);
NT35532_DCS_write_1A_1P(0x17,0xA3);
NT35532_DCS_write_1A_1P(0x18,0x02);
NT35532_DCS_write_1A_1P(0x19,0xC5);
NT35532_DCS_write_1A_1P(0x1A,0x02);
NT35532_DCS_write_1A_1P(0x1B,0xF6);
NT35532_DCS_write_1A_1P(0x1C,0x03);
NT35532_DCS_write_1A_1P(0x1D,0x15);
NT35532_DCS_write_1A_1P(0x1E,0x03);
NT35532_DCS_write_1A_1P(0x1F,0x3B);
NT35532_DCS_write_1A_1P(0x20,0x03);
NT35532_DCS_write_1A_1P(0x21,0x4C);
NT35532_DCS_write_1A_1P(0x22,0x03);
NT35532_DCS_write_1A_1P(0x23,0x57);
NT35532_DCS_write_1A_1P(0x24,0x03);
NT35532_DCS_write_1A_1P(0x25,0x68);
NT35532_DCS_write_1A_1P(0x26,0x03);
NT35532_DCS_write_1A_1P(0x27,0x79);
NT35532_DCS_write_1A_1P(0x28,0x03);
NT35532_DCS_write_1A_1P(0x29,0x8D);
NT35532_DCS_write_1A_1P(0x2A,0x03);
NT35532_DCS_write_1A_1P(0x2B,0xA3);
NT35532_DCS_write_1A_1P(0x2D,0x03);
NT35532_DCS_write_1A_1P(0x2F,0xBB);
NT35532_DCS_write_1A_1P(0x30,0x03);
NT35532_DCS_write_1A_1P(0x31,0xBF);



NT35532_DCS_write_1A_1P(0x32,0x00);
NT35532_DCS_write_1A_1P(0x33,0x08);
NT35532_DCS_write_1A_1P(0x34,0x00);
NT35532_DCS_write_1A_1P(0x35,0x36);
NT35532_DCS_write_1A_1P(0x36,0x00);
NT35532_DCS_write_1A_1P(0x37,0x7B);
NT35532_DCS_write_1A_1P(0x38,0x00);
NT35532_DCS_write_1A_1P(0x39,0x96);
NT35532_DCS_write_1A_1P(0x3A,0x00);
NT35532_DCS_write_1A_1P(0x3B,0xAD);
NT35532_DCS_write_1A_1P(0x3D,0x00);
NT35532_DCS_write_1A_1P(0x3F,0xC1);
NT35532_DCS_write_1A_1P(0x40,0x00);
NT35532_DCS_write_1A_1P(0x41,0xD4);
NT35532_DCS_write_1A_1P(0x42,0x00);
NT35532_DCS_write_1A_1P(0x43,0xE2);
NT35532_DCS_write_1A_1P(0x44,0x00);
NT35532_DCS_write_1A_1P(0x45,0xF1);
NT35532_DCS_write_1A_1P(0x46,0x01);
NT35532_DCS_write_1A_1P(0x47,0x23);
NT35532_DCS_write_1A_1P(0x48,0x01);
NT35532_DCS_write_1A_1P(0x49,0x48);
NT35532_DCS_write_1A_1P(0x4A,0x01);
NT35532_DCS_write_1A_1P(0x4B,0x85);
NT35532_DCS_write_1A_1P(0x4C,0x01);
NT35532_DCS_write_1A_1P(0x4D,0xB5);
NT35532_DCS_write_1A_1P(0x4E,0x02);
NT35532_DCS_write_1A_1P(0x4F,0x00);
NT35532_DCS_write_1A_1P(0x50,0x02);
NT35532_DCS_write_1A_1P(0x51,0x3D);
NT35532_DCS_write_1A_1P(0x52,0x02);
NT35532_DCS_write_1A_1P(0x53,0x37);
NT35532_DCS_write_1A_1P(0x54,0x02);
NT35532_DCS_write_1A_1P(0x55,0x70);
NT35532_DCS_write_1A_1P(0x56,0x02);
NT35532_DCS_write_1A_1P(0x58,0xAF);
NT35532_DCS_write_1A_1P(0x59,0x02);
NT35532_DCS_write_1A_1P(0x5A,0xD7);
NT35532_DCS_write_1A_1P(0x5B,0x03);
NT35532_DCS_write_1A_1P(0x5C,0x0B);
NT35532_DCS_write_1A_1P(0x5D,0x03);
NT35532_DCS_write_1A_1P(0x5E,0x32);
NT35532_DCS_write_1A_1P(0x5F,0x03);
NT35532_DCS_write_1A_1P(0x60,0x5D);
NT35532_DCS_write_1A_1P(0x61,0x03);
NT35532_DCS_write_1A_1P(0x62,0x69);
NT35532_DCS_write_1A_1P(0x63,0x03);
NT35532_DCS_write_1A_1P(0x64,0x79);
NT35532_DCS_write_1A_1P(0x65,0x03);
NT35532_DCS_write_1A_1P(0x66,0x8A);
NT35532_DCS_write_1A_1P(0x67,0x03);
NT35532_DCS_write_1A_1P(0x68,0x9B);
NT35532_DCS_write_1A_1P(0x69,0x03);
NT35532_DCS_write_1A_1P(0x6A,0xAF);
NT35532_DCS_write_1A_1P(0x6B,0x03);
NT35532_DCS_write_1A_1P(0x6C,0xC5);
NT35532_DCS_write_1A_1P(0x6D,0x03);
NT35532_DCS_write_1A_1P(0x6E,0xDD);
NT35532_DCS_write_1A_1P(0x6F,0x03);
NT35532_DCS_write_1A_1P(0x70,0xE1);



NT35532_DCS_write_1A_1P(0x71,0x00);
NT35532_DCS_write_1A_1P(0x72,0x56);
NT35532_DCS_write_1A_1P(0x73,0x00);
NT35532_DCS_write_1A_1P(0x74,0x62);
NT35532_DCS_write_1A_1P(0x75,0x00);
NT35532_DCS_write_1A_1P(0x76,0x6D);
NT35532_DCS_write_1A_1P(0x77,0x00);
NT35532_DCS_write_1A_1P(0x78,0x89);
NT35532_DCS_write_1A_1P(0x79,0x00);
NT35532_DCS_write_1A_1P(0x7A,0xA0);
NT35532_DCS_write_1A_1P(0x7B,0x00);
NT35532_DCS_write_1A_1P(0x7C,0xB4);
NT35532_DCS_write_1A_1P(0x7D,0x00);
NT35532_DCS_write_1A_1P(0x7E,0xC7);
NT35532_DCS_write_1A_1P(0x7F,0x00);
NT35532_DCS_write_1A_1P(0x80,0xD6);
NT35532_DCS_write_1A_1P(0x81,0x00);
NT35532_DCS_write_1A_1P(0x82,0xE4);
NT35532_DCS_write_1A_1P(0x83,0x01);
NT35532_DCS_write_1A_1P(0x84,0x16);
NT35532_DCS_write_1A_1P(0x85,0x01);
NT35532_DCS_write_1A_1P(0x86,0x3B);
NT35532_DCS_write_1A_1P(0x87,0x01);
NT35532_DCS_write_1A_1P(0x88,0x78);
NT35532_DCS_write_1A_1P(0x89,0x01);
NT35532_DCS_write_1A_1P(0x8A,0xA8);
NT35532_DCS_write_1A_1P(0x8B,0x01);
NT35532_DCS_write_1A_1P(0x8C,0xF3);
NT35532_DCS_write_1A_1P(0x8D,0x02);
NT35532_DCS_write_1A_1P(0x8E,0x30);
NT35532_DCS_write_1A_1P(0x8F,0x02);
NT35532_DCS_write_1A_1P(0x90,0x37);
NT35532_DCS_write_1A_1P(0x91,0x02);
NT35532_DCS_write_1A_1P(0x92,0x6C);
NT35532_DCS_write_1A_1P(0x93,0x02);
NT35532_DCS_write_1A_1P(0x94,0xA3);
NT35532_DCS_write_1A_1P(0x95,0x02);
NT35532_DCS_write_1A_1P(0x96,0xC5);
NT35532_DCS_write_1A_1P(0x97,0x02);
NT35532_DCS_write_1A_1P(0x98,0xF6);
NT35532_DCS_write_1A_1P(0x99,0x03);
NT35532_DCS_write_1A_1P(0x9A,0x15);
NT35532_DCS_write_1A_1P(0x9B,0x03);
NT35532_DCS_write_1A_1P(0x9C,0x3B);
NT35532_DCS_write_1A_1P(0x9D,0x03);
NT35532_DCS_write_1A_1P(0x9E,0x4C);
NT35532_DCS_write_1A_1P(0x9F,0x03);
NT35532_DCS_write_1A_1P(0xA0,0x57);
NT35532_DCS_write_1A_1P(0xA2,0x03);
NT35532_DCS_write_1A_1P(0xA3,0x68);
NT35532_DCS_write_1A_1P(0xA4,0x03);
NT35532_DCS_write_1A_1P(0xA5,0x79);
NT35532_DCS_write_1A_1P(0xA6,0x03);
NT35532_DCS_write_1A_1P(0xA7,0x8D);
NT35532_DCS_write_1A_1P(0xA9,0x03);
NT35532_DCS_write_1A_1P(0xAA,0xA3);
NT35532_DCS_write_1A_1P(0xAB,0x03);
NT35532_DCS_write_1A_1P(0xAC,0xBB);
NT35532_DCS_write_1A_1P(0xAD,0x03);
NT35532_DCS_write_1A_1P(0xAE,0xBF);



NT35532_DCS_write_1A_1P(0xAF,0x00);
NT35532_DCS_write_1A_1P(0xB0,0x08);
NT35532_DCS_write_1A_1P(0xB1,0x00);
NT35532_DCS_write_1A_1P(0xB2,0x36);
NT35532_DCS_write_1A_1P(0xB3,0x00);
NT35532_DCS_write_1A_1P(0xB4,0x7B);
NT35532_DCS_write_1A_1P(0xB5,0x00);
NT35532_DCS_write_1A_1P(0xB6,0x96);
NT35532_DCS_write_1A_1P(0xB7,0x00);
NT35532_DCS_write_1A_1P(0xB8,0xAD);
NT35532_DCS_write_1A_1P(0xB9,0x00);
NT35532_DCS_write_1A_1P(0xBA,0xC1);
NT35532_DCS_write_1A_1P(0xBB,0x00);
NT35532_DCS_write_1A_1P(0xBC,0xD4);
NT35532_DCS_write_1A_1P(0xBD,0x00);
NT35532_DCS_write_1A_1P(0xBE,0xE2);
NT35532_DCS_write_1A_1P(0xBF,0x00);
NT35532_DCS_write_1A_1P(0xC0,0xF1);
NT35532_DCS_write_1A_1P(0xC1,0x01);
NT35532_DCS_write_1A_1P(0xC2,0x23);
NT35532_DCS_write_1A_1P(0xC3,0x01);
NT35532_DCS_write_1A_1P(0xC4,0x48);
NT35532_DCS_write_1A_1P(0xC5,0x01);
NT35532_DCS_write_1A_1P(0xC6,0x85);
NT35532_DCS_write_1A_1P(0xC7,0x01);
NT35532_DCS_write_1A_1P(0xC8,0xB5);
NT35532_DCS_write_1A_1P(0xC9,0x02);
NT35532_DCS_write_1A_1P(0xCA,0x00);
NT35532_DCS_write_1A_1P(0xCB,0x02);
NT35532_DCS_write_1A_1P(0xCC,0x3D);
NT35532_DCS_write_1A_1P(0xCD,0x02);
NT35532_DCS_write_1A_1P(0xCE,0x37);
NT35532_DCS_write_1A_1P(0xCF,0x02);
NT35532_DCS_write_1A_1P(0xD0,0x70);
NT35532_DCS_write_1A_1P(0xD1,0x02);
NT35532_DCS_write_1A_1P(0xD2,0xAF);
NT35532_DCS_write_1A_1P(0xD3,0x02);
NT35532_DCS_write_1A_1P(0xD4,0xD7);
NT35532_DCS_write_1A_1P(0xD5,0x03);
NT35532_DCS_write_1A_1P(0xD6,0x0B);
NT35532_DCS_write_1A_1P(0xD7,0x03);
NT35532_DCS_write_1A_1P(0xD8,0x32);
NT35532_DCS_write_1A_1P(0xD9,0x03);
NT35532_DCS_write_1A_1P(0xDA,0x5D);
NT35532_DCS_write_1A_1P(0xDB,0x03);
NT35532_DCS_write_1A_1P(0xDC,0x69);
NT35532_DCS_write_1A_1P(0xDD,0x03);
NT35532_DCS_write_1A_1P(0xDE,0x79);
NT35532_DCS_write_1A_1P(0xDF,0x03);
NT35532_DCS_write_1A_1P(0xE0,0x8A);
NT35532_DCS_write_1A_1P(0xE1,0x03);
NT35532_DCS_write_1A_1P(0xE2,0x9B);
NT35532_DCS_write_1A_1P(0xE3,0x03);
NT35532_DCS_write_1A_1P(0xE4,0xAF);
NT35532_DCS_write_1A_1P(0xE5,0x03);
NT35532_DCS_write_1A_1P(0xE6,0xC5);
NT35532_DCS_write_1A_1P(0xE7,0x03);
NT35532_DCS_write_1A_1P(0xE8,0xDD);
NT35532_DCS_write_1A_1P(0xE9,0x03);
NT35532_DCS_write_1A_1P(0xEA,0xE1);



NT35532_DCS_write_1A_1P(0xFF,0x05);



NT35532_DCS_write_1A_1P(0xFB,0x01);

NT35532_DCS_write_1A_1P(0x00,0x40);
NT35532_DCS_write_1A_1P(0x01,0x40);
NT35532_DCS_write_1A_1P(0x02,0x40);
NT35532_DCS_write_1A_1P(0x03,0x06);
NT35532_DCS_write_1A_1P(0x04,0x40);
NT35532_DCS_write_1A_1P(0x05,0x16);
NT35532_DCS_write_1A_1P(0x06,0x18);
NT35532_DCS_write_1A_1P(0x07,0x1A);
NT35532_DCS_write_1A_1P(0x08,0x1C);
NT35532_DCS_write_1A_1P(0x09,0x1E);
NT35532_DCS_write_1A_1P(0x0A,0x20);
NT35532_DCS_write_1A_1P(0x0B,0x40);
NT35532_DCS_write_1A_1P(0x0C,0x40);
NT35532_DCS_write_1A_1P(0x0D,0x26);
NT35532_DCS_write_1A_1P(0x0E,0x28);
NT35532_DCS_write_1A_1P(0x0F,0x08);
NT35532_DCS_write_1A_1P(0x10,0x38);
NT35532_DCS_write_1A_1P(0x11,0x38);
NT35532_DCS_write_1A_1P(0x12,0x38);
NT35532_DCS_write_1A_1P(0x13,0x0E);
NT35532_DCS_write_1A_1P(0x14,0x40);
NT35532_DCS_write_1A_1P(0x15,0x40);
NT35532_DCS_write_1A_1P(0x16,0x40);
NT35532_DCS_write_1A_1P(0x17,0x07);
NT35532_DCS_write_1A_1P(0x18,0x40);
NT35532_DCS_write_1A_1P(0x19,0x17);
NT35532_DCS_write_1A_1P(0x1A,0x19);
NT35532_DCS_write_1A_1P(0x1B,0x1B);
NT35532_DCS_write_1A_1P(0x1C,0x1D);
NT35532_DCS_write_1A_1P(0x1D,0x1F);
NT35532_DCS_write_1A_1P(0x1E,0x21);
NT35532_DCS_write_1A_1P(0x1F,0x40);
NT35532_DCS_write_1A_1P(0x20,0x40);
NT35532_DCS_write_1A_1P(0x21,0x26);
NT35532_DCS_write_1A_1P(0x22,0x28);
NT35532_DCS_write_1A_1P(0x23,0x09);
NT35532_DCS_write_1A_1P(0x24,0x38);
NT35532_DCS_write_1A_1P(0x25,0x38);
NT35532_DCS_write_1A_1P(0x26,0x38);
NT35532_DCS_write_1A_1P(0x27,0x0F);
NT35532_DCS_write_1A_1P(0x28,0x40);
NT35532_DCS_write_1A_1P(0x29,0x40);
NT35532_DCS_write_1A_1P(0x2A,0x40);
NT35532_DCS_write_1A_1P(0x2B,0x09);
NT35532_DCS_write_1A_1P(0x2D,0x40);
NT35532_DCS_write_1A_1P(0x2F,0x19);
NT35532_DCS_write_1A_1P(0x30,0x17);
NT35532_DCS_write_1A_1P(0x31,0x21);
NT35532_DCS_write_1A_1P(0x32,0x1F);
NT35532_DCS_write_1A_1P(0x33,0x1D);
NT35532_DCS_write_1A_1P(0x34,0x1B);
NT35532_DCS_write_1A_1P(0x35,0x40);
NT35532_DCS_write_1A_1P(0x36,0x40);
NT35532_DCS_write_1A_1P(0x37,0x26);
NT35532_DCS_write_1A_1P(0x38,0x28);
NT35532_DCS_write_1A_1P(0x39,0x07);
NT35532_DCS_write_1A_1P(0x3A,0x38);
NT35532_DCS_write_1A_1P(0x3B,0x38);
NT35532_DCS_write_1A_1P(0x3D,0x38);
NT35532_DCS_write_1A_1P(0x3F,0x0F);
NT35532_DCS_write_1A_1P(0x40,0x40);
NT35532_DCS_write_1A_1P(0x41,0x40);
NT35532_DCS_write_1A_1P(0x42,0x40);
NT35532_DCS_write_1A_1P(0x43,0x08);
NT35532_DCS_write_1A_1P(0x44,0x40);
NT35532_DCS_write_1A_1P(0x45,0x18);
NT35532_DCS_write_1A_1P(0x46,0x16);
NT35532_DCS_write_1A_1P(0x47,0x20);
NT35532_DCS_write_1A_1P(0x48,0x1E);
NT35532_DCS_write_1A_1P(0x49,0x1C);
NT35532_DCS_write_1A_1P(0x4A,0x1A);
NT35532_DCS_write_1A_1P(0x4B,0x40);
NT35532_DCS_write_1A_1P(0x4C,0x40);
NT35532_DCS_write_1A_1P(0x4D,0x26);
NT35532_DCS_write_1A_1P(0x4E,0x28);
NT35532_DCS_write_1A_1P(0x4F,0x06);
NT35532_DCS_write_1A_1P(0x50,0x38);
NT35532_DCS_write_1A_1P(0x51,0x38);
NT35532_DCS_write_1A_1P(0x52,0x38);
NT35532_DCS_write_1A_1P(0x53,0x0E);

NT35532_DCS_write_1A_1P(0x54,0x07);
NT35532_DCS_write_1A_1P(0x55,0x19);
NT35532_DCS_write_1A_1P(0x59,0x24);
NT35532_DCS_write_1A_1P(0x5B,0x69);
NT35532_DCS_write_1A_1P(0x5C,0x12);
NT35532_DCS_write_1A_1P(0x5D,0x01);
NT35532_DCS_write_1A_1P(0x5E,0x22);
NT35532_DCS_write_1A_1P(0x62,0x21);
NT35532_DCS_write_1A_1P(0x63,0x69);
NT35532_DCS_write_1A_1P(0x64,0x12);
NT35532_DCS_write_1A_1P(0x66,0x57);
NT35532_DCS_write_1A_1P(0x67,0x11);
NT35532_DCS_write_1A_1P(0x68,0x69);
NT35532_DCS_write_1A_1P(0x69,0x12);
NT35532_DCS_write_1A_1P(0x6A,0x05);
NT35532_DCS_write_1A_1P(0x6B,0x29);
NT35532_DCS_write_1A_1P(0x6C,0x08);
NT35532_DCS_write_1A_1P(0x6D,0x18);
NT35532_DCS_write_1A_1P(0x6F,0x3C);
NT35532_DCS_write_1A_1P(0x70,0x03);
NT35532_DCS_write_1A_1P(0x72,0x22);
NT35532_DCS_write_1A_1P(0x73,0x22);
NT35532_DCS_write_1A_1P(0x7D,0x01);

NT35532_DCS_write_1A_1P(0x7E,0x00);
NT35532_DCS_write_1A_1P(0x7F,0x00);
NT35532_DCS_write_1A_1P(0x80,0x00);
NT35532_DCS_write_1A_1P(0x81,0x00);
NT35532_DCS_write_1A_1P(0x85,0x3F);
NT35532_DCS_write_1A_1P(0x86,0x3F);

NT35532_DCS_write_1A_1P(0x9F,0x0F);

NT35532_DCS_write_1A_1P(0xB7,0x11);
NT35532_DCS_write_1A_1P(0xBD,0xA6);
NT35532_DCS_write_1A_1P(0xBE,0x08);
NT35532_DCS_write_1A_1P(0xBF,0x12);
NT35532_DCS_write_1A_1P(0xC8,0x00);
NT35532_DCS_write_1A_1P(0xC9,0x00);
NT35532_DCS_write_1A_1P(0xCA,0x00);
NT35532_DCS_write_1A_1P(0xCB,0x00);
NT35532_DCS_write_1A_1P(0xCC,0x09);
NT35532_DCS_write_1A_1P(0xCF,0x88);
NT35532_DCS_write_1A_1P(0xD0,0x00);
NT35532_DCS_write_1A_1P(0xD1,0x00);
NT35532_DCS_write_1A_1P(0xD2,0x00);
NT35532_DCS_write_1A_1P(0xD3,0x00);
NT35532_DCS_write_1A_1P(0xD4,0x40);
NT35532_DCS_write_1A_1P(0xD5,0x11);

NT35532_DCS_write_1A_1P(0x91,0x10);
NT35532_DCS_write_1A_1P(0x92,0x10);
NT35532_DCS_write_1A_1P(0x97,0x00);
NT35532_DCS_write_1A_1P(0x98,0x00);
NT35532_DCS_write_1A_1P(0x99,0x00);

NT35532_DCS_write_1A_1P(0xD6,0x89);
NT35532_DCS_write_1A_1P(0xD7,0x31);
NT35532_DCS_write_1A_1P(0xD8,0x7E);

NT35532_DCS_write_1A_1P(0xA2,0xB0);



NT35532_DCS_write_1A_1P(0xFF,0x00);

NT35532_DCS_write_1A_1P(0xD3,0x24);
NT35532_DCS_write_1A_1P(0xD4,0x24);
NT35532_DCS_write_1A_1P(0xD5,0x04);
NT35532_DCS_write_1A_1P(0xD6,0x04);
NT35532_DCS_write_1A_1P(0x35,0x00);
NT35532_DCS_write_1A_1P(0x11,0x00);
MDELAY(300);
NT35532_DCS_write_1A_1P(0x29,0x00);
MDELAY(300);
}
#endif
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{

		memset(params, 0, sizeof(LCM_PARAMS));

		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE;
        #endif

		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
  		params->dsi.word_count=720*3;
#if defined(DROI_PRO_FQ5B_TW2)
	params->dsi.vertical_sync_active	= 2;
	params->dsi.vertical_backporch		= 8;
	params->dsi.vertical_frontporch		= 14;
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 10;
	params->dsi.horizontal_backporch	= 100;
	params->dsi.horizontal_frontporch	= 100;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	// Bit rate calculation
	params->dsi.PLL_CLOCK=450;
#elif defined(DROI_PRO_FQ5B_ZGW4)
        params->dsi.vertical_sync_active	= 2;
	params->dsi.vertical_backporch		= 4;
	params->dsi.vertical_frontporch		= 4;
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 10;
	params->dsi.horizontal_backporch	= 130;
	params->dsi.horizontal_frontporch	= 130;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	// Bit rate calculation
	params->dsi.PLL_CLOCK=470;
#else
        params->dsi.vertical_sync_active	= 4;
	params->dsi.vertical_backporch		= 12;
	params->dsi.vertical_frontporch		= 16;
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;
	params->dsi.horizontal_backporch	= 60;
	params->dsi.horizontal_frontporch	= 72;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	// Bit rate calculation
	params->dsi.PLL_CLOCK=430;
#endif
        params->dsi.ssc_disable = 1;

   	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

}

static void lcm_init(void)
{
#if defined(SUPORT_ADC_CHECK)
	int val = 0;
#endif
	lcm_is_init = true;

		MDELAY(10);
        mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
		MDELAY(10);
	mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
		MDELAY(10);

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);

	SET_RESET_PIN(1);
	MDELAY(20);
#if defined(SUPORT_ADC_CHECK)
	val=lcm_read_ADC_value();
    adc_lcm_registers(val);
#else
	init_lcm_registers();
#endif
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	//unsigned char buffer[2];

#if 0//ndef BUILD_LK
	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0xFE, buffer, 1);
	printk("%s, kernel nt35532 horse debug: nt35532 id = 0x%08x\n", __func__, buffer[0]);
#endif

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);

	MDELAY(120);
	SET_RESET_PIN(0);
	MDELAY(20);

	MDELAY(10);
	mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ZERO);
	MDELAY(10);

	lcm_is_init = false;
}

static void lcm_resume(void)
{
	//unsigned int data_array[16];
	//unsigned char buffer[2];
	if(!lcm_is_init)
		lcm_init();

#if 0//ndef BUILD_LK
	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0xFE, buffer, 1);
	printk("%s, kernel nt35532 horse debug: nt35532 id = 0x%08x\n", __func__, buffer[0]);
#endif

	//TC358768_DCS_write_1A_0P(0x11); // Sleep Out
	//MDELAY(150);

	//TC358768_DCS_write_1A_0P(0x29); // Display On

}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);

	SET_RESET_PIN(1);
	MDELAY(20);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID
    #ifdef BUILD_LK
		printf("***********%s, LK nt35532 debug: nt35532 id = 0x%08x\n", __func__, id);
    #else
		printk("*****%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_NT35532)
    	return 1;
    else
        return 0;


}


LCM_DRIVER nt35532_dsi_6735_fhd_lcm_drv =
{
    .name			= "nt35532_dsi_fhd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
