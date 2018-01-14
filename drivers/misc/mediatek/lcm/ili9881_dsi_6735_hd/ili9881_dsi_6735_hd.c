#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#else
	#include <linux/string.h>
	#if defined(BUILD_UBOOT)
		#include <asm/arch/mt_gpio.h>
	#else
//		#include <mach/mt_gpio.h>
	#endif
#endif
#include "lcm_drv.h"

#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
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
#endif
// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------

#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#define FRAME_WIDTH  										(640)
#define FRAME_HEIGHT 										(1280)

#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#endif

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE									0

#define ILI9881C_ID	0x9881

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// ---------------------------------------------------------------------------
// Local Variables
// ---------------------------------------------------------------------------


static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#define GPIO_LDO28_EN (GPIO125 | 0x80000000)
#define GPIO_LDO18_EN (GPIO126 | 0x80000000)
#define GPIO_LCM_RST (GPIO146 | 0x80000000)
#endif

// ---------------------------------------------------------------------------
// Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);
#if defined(DROI_PRO_F5C_JF2)||defined(DROI_PRO_F5C_DT)||defined(DROI_PRO_F5C_LEAGOO4)||defined(DROI_PRO_F5C_SGDZ3)||defined(DROI_PRO_F5C_HN)
static void init_lcm_registers(void);
#endif

#if defined(DROI_PRO_FQ5CW)
#include "lcd_ili9881_dsi_6735_hd_fq5cw.h"
#endif
#if defined(DROI_PRO_F5C)
#include "lcd_ili9881_dsi_6735_hd_f5c.h"
#endif
#if defined(DROI_PRO_F6)
#include "lcd_ili9881_dsi_6735_hd_f6.h"
#endif
#if defined(DROI_PRO_Q1)
#include "lcd_ili9881_dsi_6735_hd_q1.h"
#endif
#if defined(DROI_PRO_PF5)
#include "lcd_ili9881_dsi_6735_hd_pf5.h"
#endif

#if defined(DROI_PRO_PU6)
#include "lcd_ili9881_dsi_6735_hd_pu6.h"
#endif

#if defined(DROI_PRO_F2C)
#include "lcd_ili9881_dsi_6735_hd_f2c.h"
#endif


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
static void adc_lcm_push_table(int val)
{
   #if defined(DROI_PRO_F5C_LEAGOO)
	if((val>1000)&&(val<1400))             //voltage=1.2V
	{
	    push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000))         //voltage=0.8V
	{
	    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

	}
    #elif defined(DROI_PRO_Q1_BPZN)
	if((val>1000)&&(val<1400))             //voltage=1.2V
	{
	  push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000))         //voltage=0.8V
	{
	    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{
	}
#elif defined(DROI_PRO_F6_HW)
	if((val>600)&&(val<1000))             //voltage=0.8V
	{
		push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>200)&&(val<600))         //voltage=0.4V
	{
		push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{
	}
#elif defined(DROI_PRO_PF5_LEAGOO4)
	if((val>600)&&(val<1000))             //voltage=0.8V HSD_T20
	{
		push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>200)&&(val<600))         //voltage=0.4V HSD_T00
	{
		push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{
	}
#elif defined(DROI_PRO_F6_BPZN2)
	if((val>1000)&&(val<1400))
	{             //voltage=1.2V
		push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000))
	{         //voltage=0.8V
		push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{
	}
 #else
	if((val>500)&&(val<900)) 	    //voltage=0.7V
	{
	    push_table(lcm_initialization_setting_0700, sizeof(lcm_initialization_setting_0700) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>1200)&&(val<1600)) //voltage=1.4V
	{
	    push_table(lcm_initialization_setting_1400, sizeof(lcm_initialization_setting_1400) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>1900)&&(val<2300)) //voltage=2.1V
	{
	    push_table(lcm_initialization_setting_2100, sizeof(lcm_initialization_setting_2100) / sizeof(struct LCM_setting_table), 1);
	}
	else   				//voltage=0V
	{
	    push_table(lcm_initialization_setting_0000, sizeof(lcm_initialization_setting_0000) / sizeof(struct LCM_setting_table), 1);
	}
#endif
}
#endif

#if defined(DROI_PRO_F5C_JF2)||defined(DROI_PRO_F5C_DT)||defined(DROI_PRO_F5C_LEAGOO4)||defined(DROI_PRO_F5C_SGDZ3)||defined(DROI_PRO_F5C_HN)
#else
#ifndef TYD_LCD_USE_CUSTOM_ILI9881C_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
	//ILI9881C_AUO5.5_keyuda
	//CCMON
	{0xFF,3,{0x98,0x81,0x03}},
	{0x01,1,{0x08}},
	{0x02,1,{0x00}},
	{0x03,1,{0x73}},
	{0x04,1,{0x73}},
	{0x05,1,{0x14}},
	{0x06,1,{0x06}},
	{0x07,1,{0x02}},
	{0x08,1,{0x05}},
	{0x09,1,{0x14}},
	{0x0a,1,{0x14}},
	{0x0b,1,{0x00}},
	{0x0c,1,{0x14}},
	{0x0d,1,{0x14}},
	{0x0e,1,{0x00}},
	{0x0f,1,{0x0C}},
	{0x10,1,{0x0C}},
	{0x11,1,{0x0C}},
	{0x12,1,{0x0C}},
	{0x13,1,{0x14}},
	{0x14,1,{0x0c}},
	{0x15,1,{0x00}},
	{0x16,1,{0x00}},
	{0x17,1,{0x00}},
	{0x18,1,{0x00}},
	{0x19,1,{0x00}},
	{0x1a,1,{0x00}},
	{0x1b,1,{0x00}},
	{0x1c,1,{0x00}},
	{0x1d,1,{0x00}},
	{0x1e,1,{0xc8}},
	{0x1f,1,{0x80}},
	{0x20,1,{0x02}},
	{0x21,1,{0x00}},
	{0x22,1,{0x02}},
	{0x23,1,{0x00}},
	{0x24,1,{0x00}},
	{0x25,1,{0x00}},
	{0x26,1,{0x00}},
	{0x27,1,{0x00}},
	{0x28,1,{0xfb}},
	{0x29,1,{0x43}},
	{0x2a,1,{0x00}},
	{0x2b,1,{0x00}},
	{0x2c,1,{0x07}},
	{0x2d,1,{0x07}},
	{0x2e,1,{0xff}},
	{0x2f,1,{0xff}},
	{0x30,1,{0x11}},
	{0x31,1,{0x00}},
	{0x32,1,{0x00}},
	{0x33,1,{0x00}},
	{0x34,1,{0x84}},
	{0x35,1,{0x80}},
	{0x36,1,{0x07}},
	{0x37,1,{0x00}},
	{0x38,1,{0x00}},
	{0x39,1,{0x00}},
	{0x3a,1,{0x00}},
	{0x3b,1,{0x00}},
	{0x3c,1,{0x00}},
	{0x3d,1,{0x00}},
	{0x3e,1,{0x00}},
	{0x3f,1,{0x00}},
	{0x40,1,{0x00}},
	{0x41,1,{0x88}},
	{0x42,1,{0x00}},
	{0x43,1,{0x80}},
	{0x44,1,{0x08}},
	{0x50,1,{0x01}},
	{0x51,1,{0x23}},
	{0x52,1,{0x45}},
	{0x53,1,{0x67}},
	{0x54,1,{0x89}},
	{0x55,1,{0xab}},
	{0x56,1,{0x01}},
	{0x57,1,{0x23}},
	{0x58,1,{0x45}},
	{0x59,1,{0x67}},
	{0x5a,1,{0x89}},
	{0x5b,1,{0xab}},
	{0x5c,1,{0xcd}},
	{0x5d,1,{0xef}},
	{0x5e,1,{0x10}},
	{0x5f,1,{0x02}},
	{0x60,1,{0x08}},
	{0x61,1,{0x09}},
	{0x62,1,{0x10}},
	{0x63,1,{0x12}},
	{0x64,1,{0x11}},
	{0x65,1,{0x13}},
	{0x66,1,{0x0c}},
	{0x67,1,{0x02}},
	{0x68,1,{0x02}},
	{0x69,1,{0x02}},
	{0x6a,1,{0x02}},
	{0x6b,1,{0x02}},
	{0x6c,1,{0x0e}},
	{0x6d,1,{0x0d}},
	{0x6e,1,{0x0f}},
	{0x6f,1,{0x02}},
	{0x70,1,{0x02}},
	{0x71,1,{0x06}},
	{0x72,1,{0x07}},
	{0x73,1,{0x02}},
	{0x74,1,{0x02}},
	{0x75,1,{0x02}},
	{0x76,1,{0x07}},
	{0x77,1,{0x06}},
	{0x78,1,{0x11}},
	{0x79,1,{0x13}},
	{0x7a,1,{0x10}},
	{0x7b,1,{0x12}},
	{0x7c,1,{0x0f}},
	{0x7d,1,{0x02}},
	{0x7e,1,{0x02}},
	{0x7f,1,{0x02}},
	{0x80,1,{0x02}},
	{0x81,1,{0x02}},
	{0x82,1,{0x0d}},
	{0x83,1,{0x0e}},
	{0x84,1,{0x0c}},
	{0x85,1,{0x02}},
	{0x86,1,{0x02}},
	{0x87,1,{0x09}},
	{0x88,1,{0x08}},
	{0x89,1,{0x02}},
	{0x8A,1,{0x02}},
	{0xFF,3,{0x98,0x81,0x04}},
	{0x6C,1,{0x15}},
	{0x6E,1,{0x2B}},                //2B
	{0x6F,1,{0x33}},  		//35
	{0x3A,1,{0x24}},
	{0x8D,1,{0x14}},                //VGL clamp -11V=1A  -12V=1F
	{0x87,1,{0xBA}},	               //ESD
	{0x26,1,{0x76}},
	{0xB2,1,{0xD1}},

	{0xFF,3,{0x98,0x81,0x01}},
	{0x22,1,{0x3A}},               //  0A
	{0x31,1,{0x00}},        //column inversion
	{0x53,1,{0x60}},		//VCOM1
	{0x55,1,{0x8F}},		//VCOM2
	{0x50,1,{0x96}},		//VREG1OUT=5V
	{0x51,1,{0x96}},		//VREG2OUT=-5V
	{0x60,1,{0x14}},		//SDT  14
	{0xA0,1,{0x08}},		//VP255	Gamma P
	{0xA1,1,{0x14}},	//VP251
	{0xA2,1,{0x1f}},		//VP247
	{0xA3,1,{0x0f}},		//VP243
	{0xA4,1,{0x11}},               //VP239
	{0xA5,1,{0x24}},               //VP231
	{0xA6,1,{0x16}},               //VP219
	{0xA7,1,{0x1a}},               //VP203
	{0xA8,1,{0x75}},               //VP175
	{0xA9,1,{0x1C}},               //VP144
	{0xAA,1,{0x28}},               //VP111
	{0xAB,1,{0x74}},               //VP80
	{0xAC,1,{0x1a}},               //VP52
	{0xAD,1,{0x18}},               //VP36
	{0xAE,1,{0x4c}},               //VP24
	{0xAF,1,{0x23}},               //VP16
	{0xB0,1,{0x27}},               //VP12
	{0xB1,1,{0x5a}},               //VP8
	{0xB2,1,{0x6a}},               //VP4
	{0xB3,1,{0x39}},               //VP0
	{0xC0,1,{0x08}},		//VN255 GAMMA N
	{0xC1,1,{0x13}},               //VN251
	{0xC2,1,{0x1e}},               //VN247
	{0xC3,1,{0x0f}},               //VN243
	{0xC4,1,{0x10}},               //VN239
	{0xC5,1,{0x23}},               //VN231
	{0xC6,1,{0x18}},               //VN219
	{0xC7,1,{0x1a}},               //VN203
	{0xC8,1,{0x76}},              //VN175
	{0xC9,1,{0x1C}},               //VN144
	{0xCA,1,{0x28}},               //VN111
	{0xCB,1,{0x74}},               //VN80
	{0xCC,1,{0x1a}},               //VN52
	{0xCD,1,{0x1a}},               //VN36
	{0xCE,1,{0x4D}},               //VN24
	{0xCF,1,{0x23}},               //VN16
	{0xD0,1,{0x28}},               //VN12
	{0xD1,1,{0x5b}},               //VN8
	{0xD2,1,{0x69}},               //VN4
	{0xD3,1,{0x39}},               //VN0
	{0xFF,3,{0x98,0x81,0x00}},

	{0x11,0,{0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x29,0,{0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
#endif

#if defined(DROI_PRO_F5C_JBT2) || defined(DROI_PRO_F6_JF)||defined(DROI_PRO_F5C_ZXHL_A6)||defined(DROI_PRO_PU6_OLK)
#else
static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
#if defined(DROI_PRO_F5C_LEAGOO) || defined(DROI_PRO_F6_BPZN3)
    {0xFF,3,{0x98,0x81,0x01}},
    {0x53,1,{0x10}},
    {0xB3,1,{0x3F}},
    {0xD3,1,{0x3F}},

    {0xFF,3,{0x98,0x81,0x04}},
    {0x2D,1,{0x02}},
    {0x2F,1,{0x01}},
    {REGFLAG_DELAY, 100, {}},
    {0x2F,1,{0x00}},

    {0xFF,3,{0x98,0x81,0x00}},
    {0x2F,1,{0x00}},
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
#elif defined(DROI_PRO_F5C_DT)||defined(DROI_PRO_F5C_HN)
// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0xff, 3, {0x98,0x81,0x01}},
	{0x58, 1, {0x01}},

	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
#else
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
#endif
};
#endif


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
			MDELAY(table[i].count);
			break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
	 break;
       	}
    }

}


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
	    params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
		params->dsi.ssc_disable = 1;

	#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
	#else
	    params->dsi.mode   = SYNC_PULSE_VDO_MODE;  //; BURST_VDO_MODE
	#endif
	    // DSI
	    /* Command mode setting */
	#if defined(DROI_PRO_F5C_TW_XDB)||defined(DROI_PRO_Q1_BPZN) || defined(DROI_PRO_F6_BPZN3) || defined(DROI_PRO_F2C)||defined(DROI_PRO_Q1_ZGW) || defined(DROI_PRO_F6_BPZN2)

	    params->dsi.LANE_NUM = LCM_THREE_LANE;
	#else
	    params->dsi.LANE_NUM = LCM_FOUR_LANE;
	#endif
	    //The following defined the fomat for data coming from LCD engine.
	    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	    params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	    params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	    params->dsi.data_format.format	  = LCM_DSI_FORMAT_RGB888;

        // Highly depends on LCD driver capability.
		params->dsi.packet_size=256;
	    // Video mode setting
	    params->dsi.intermediat_buffer_num = 0;

	    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	    params->dsi.vertical_active_line=FRAME_HEIGHT;

       // Bit rate calculation
	   #if defined(DROI_PRO_F5C_JF2)||defined(DROI_PRO_F5C_DT)||defined(DROI_PRO_F5C_LEAGOO4)||defined(DROI_PRO_F5C_SGDZ3)||defined(DROI_PRO_PF5_LEAGOO4)||defined(DROI_PRO_F5C_HN)
	   #else
		params->dsi.pll_div1=0;//1;//30;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1;			// div2=0~15: fout=fvo/(2*div2)
		params->dsi.fbk_div=23;//19;//25;  17
		#endif


#if defined(DROI_PRO_F5C_SBYH)
	    params->dsi.vertical_sync_active				= 6;//4; //  4  0x3;// 3    2
	    params->dsi.vertical_backporch				= 15;// 0x8;// 20   1
	    params->dsi.vertical_frontporch				= 16; //16 0x8; // 1  12
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 8;//10 50  2
	    params->dsi.horizontal_backporch				= 48;//50;//42
	    params->dsi.horizontal_frontporch				= 52;//50; //44
	    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    params->dsi.PLL_CLOCK=220;//194;
#elif defined(DROI_PRO_F5C_HW)
	     params->dsi.vertical_sync_active				= 8;
	    params->dsi.vertical_backporch				= 24;
	    params->dsi.vertical_frontporch				= 16;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 40;
	    params->dsi.horizontal_backporch				= 100;
	    params->dsi.horizontal_frontporch				= 70;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
	    params->dsi.HS_TRAIL=20;
	       params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;
    params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
    params->dsi.lcm_esd_check_table[0].count = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
            params->dsi.PLL_CLOCK = 208;
#elif defined(DROI_PRO_F5C_LEAGOO)
	    params->dsi.vertical_sync_active				= 8;
	    params->dsi.vertical_backporch				    = 24;
	    params->dsi.vertical_frontporch				    = 16;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 20;
	    params->dsi.horizontal_backporch				= 60;
	    params->dsi.horizontal_frontporch				= 60;
	    params->dsi.horizontal_active_pixel			    = FRAME_WIDTH;
	    params->dsi.HS_TRAIL = 20;
		params->dsi.PLL_CLOCK = 260;

        params->dsi.esd_check_enable = 1;
        params->dsi.customization_esd_check_enable = 1;
        params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
        params->dsi.lcm_esd_check_table[0].count        = 1;
        params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_Q1_HN)
	    params->dsi.vertical_sync_active				= 10;
	    params->dsi.vertical_backporch				= 20;
	    params->dsi.vertical_frontporch				= 10;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 10;
	    params->dsi.horizontal_backporch				= 80;
	    params->dsi.horizontal_frontporch				= 80;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 229;

	params->physical_width = 70;
	params->physical_height = 121;
#elif defined(TYD_PRO_FQ5C_GS2)
	   params->dsi.vertical_sync_active				= 8;
params->dsi.vertical_backporch					= 20;
params->dsi.vertical_frontporch					= 10;
params->dsi.vertical_active_line				= FRAME_HEIGHT;
params->dsi.horizontal_sync_active				= 40;
params->dsi.horizontal_backporch				= 100;
params->dsi.horizontal_frontporch				= 70;
params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
  params->dsi.HS_TRAIL = 20;
  params->dsi.PLL_CLOCK = 208;
  params->dsi.esd_check_enable = 1;
  params->dsi.customization_esd_check_enable = 1;
  params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
  params->dsi.lcm_esd_check_table[0].count        = 1;
  params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F5C_JBT2)
            params->dsi.vertical_sync_active				= 10;
	    params->dsi.vertical_backporch				= 20;
	    params->dsi.vertical_frontporch				= 20;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 40;
	    params->dsi.horizontal_backporch				= 100;
	    params->dsi.horizontal_frontporch				= 100;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 230;
#elif defined(DROI_PRO_F5C_JF2)||defined(DROI_PRO_F5C_DT)||defined(DROI_PRO_F5C_LEAGOO4)||defined(DROI_PRO_F5C_SGDZ3)||defined(DROI_PRO_PF5_LEAGOO4)||defined(DROI_PRO_F5C_HN)
            	    params->dsi.vertical_sync_active				= 16;//16
	    params->dsi.vertical_backporch				= 20;//20  18
	    params->dsi.vertical_frontporch				= 20;//20
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 20;
	    params->dsi.horizontal_backporch				= 80;
	    params->dsi.horizontal_frontporch				= 80;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
	  	params->dsi.HS_TRAIL = 20;
        params->dsi.PLL_CLOCK = 230;
		params->physical_width = 62;
		params->physical_height = 111;
#elif defined(DROI_PRO_Q1_ZGW)
	params->dsi.vertical_sync_active				= 8;
	params->dsi.vertical_backporch					= 20;//12;
	params->dsi.vertical_frontporch					= 12;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 60;//12;
	params->dsi.horizontal_backporch				= 60;//50;//70
	params->dsi.horizontal_frontporch				= 60;//50;//70
//	params->dsi.horizontal_blanking_pixel		       = 60;
	params->dsi.horizontal_active_pixel		       = FRAME_WIDTH;
	// Bit rate calculation

	params->dsi.PLL_CLOCK=285;
#elif defined(DROI_PRO_FQ5CW_TW)
		params->dsi.vertical_sync_active				= 8;
		params->dsi.vertical_backporch					= 24;
		params->dsi.vertical_frontporch					= 16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 40;
		params->dsi.horizontal_backporch				= 100;
		params->dsi.horizontal_frontporch				= 70;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.HS_TRAIL = 20;
		params->dsi.PLL_CLOCK = 212;

  		params->dsi.esd_check_enable = 1;
 	 	params->dsi.customization_esd_check_enable = 1;
  		params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
  		params->dsi.lcm_esd_check_table[0].count        = 1;
  		params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

#elif defined(DROI_PRO_F6_GB) || defined(DROI_PRO_F6_G2)
		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 8;
		params->dsi.vertical_frontporch					= 16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 40;
		params->dsi.horizontal_backporch				= 100;
		params->dsi.horizontal_frontporch				= 60;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		//params->dsi.HS_TRAIL = 20;
		params->dsi.PLL_CLOCK = 212;

  		params->dsi.esd_check_enable = 1;
 	 	params->dsi.customization_esd_check_enable = 1;
  		params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
  		params->dsi.lcm_esd_check_table[0].count        = 1;
  		params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

#elif defined(DROI_PRO_F5C_JF)
       params->dsi.vertical_sync_active               = 6;//4; //  4  0x3;// 3    2
       params->dsi.vertical_backporch              = 16;// 0x8;// 20   1
       params->dsi.vertical_frontporch             = 18; //16 0x8; // 1  12
       params->dsi.vertical_active_line                = FRAME_HEIGHT;

       params->dsi.horizontal_sync_active              = 20;//10 50  2
       params->dsi.horizontal_backporch                = 30;//50;//42
       params->dsi.horizontal_frontporch               = 30;//50; //44
	   params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
       params->dsi.PLL_CLOCK = 250;

#elif defined(DROI_PRO_Q1_BPZN)
	    params->dsi.vertical_sync_active				= 10;
	    params->dsi.vertical_backporch				= 20;
	    params->dsi.vertical_frontporch				= 10;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 10;
	    params->dsi.horizontal_backporch				= 150;
	    params->dsi.horizontal_frontporch				= 80;
	    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    params->dsi.PLL_CLOCK = 304;
#elif defined(DROI_PRO_F6_JF)
	params->dsi.vertical_sync_active		= 8;
	params->dsi.vertical_backporch			= 24;
	params->dsi.vertical_frontporch			= 16;
	params->dsi.vertical_active_line		= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active		= 20;
	params->dsi.horizontal_backporch		= 100;
	params->dsi.horizontal_frontporch		= 100;
	params->dsi.horizontal_active_pixel		= FRAME_WIDTH;

	params->dsi.HS_TRAIL = 120;
	params->dsi.PLL_CLOCK = 230;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F5C_ZXHL_A6)
		params->dsi.vertical_sync_active				= 10;
	    params->dsi.vertical_backporch				= 24;
	    params->dsi.vertical_frontporch				= 20;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 20;
	    params->dsi.horizontal_backporch				= 140;
	    params->dsi.horizontal_frontporch				= 120;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 230;
			params->dsi.HS_TRAIL = 120;
#elif defined(DROI_PRO_F6_ZR)
	params->dsi.vertical_sync_active		= 8;
	params->dsi.vertical_backporch			= 24;
	params->dsi.vertical_frontporch			= 16;
	params->dsi.vertical_active_line		= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active		= 40;
	params->dsi.horizontal_backporch		= 100;
	params->dsi.horizontal_frontporch		= 70;
	params->dsi.horizontal_active_pixel		= FRAME_WIDTH;

	params->dsi.HS_TRAIL = 20;
	params->dsi.PLL_CLOCK = 212;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F6_ZR_M81H)
	params->dsi.vertical_sync_active		= 10;
	params->dsi.vertical_backporch			= 20;
	params->dsi.vertical_frontporch			= 10;
	params->dsi.vertical_active_line		= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active		= 40;
	params->dsi.horizontal_backporch		= 120;
	params->dsi.horizontal_frontporch		= 80;
	params->dsi.horizontal_active_pixel		= FRAME_WIDTH;
	params->dsi.HS_TRAIL                            = 20;
	params->dsi.PLL_CLOCK = 220;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F6_HW)
	params->dsi.vertical_sync_active				= 8;
	params->dsi.vertical_backporch					= 24;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 40;
	params->dsi.horizontal_backporch				= 100;
	params->dsi.horizontal_frontporch				= 70;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.HS_TRAIL = 20;
	params->dsi.PLL_CLOCK = 235;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F6_BPZN3)
	params->dsi.vertical_sync_active				= 8;
	params->dsi.vertical_backporch					= 20;
	params->dsi.vertical_frontporch					= 20;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 100;
	params->dsi.horizontal_frontporch				= 100;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 290;
	params->dsi.HS_TRAIL = 20;
#elif defined(DROI_PRO_PU6_OLK)
	params->dsi.vertical_sync_active				= 10;
	params->dsi.vertical_backporch					= 16;
	params->dsi.vertical_frontporch					= 14;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 50;
	params->dsi.horizontal_frontporch				= 50;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 225;
	params->dsi.HS_TRAIL = 20;
#elif defined(DROI_PRO_F2C)
	 params->dsi.vertical_sync_active				 = 10;//4
	 params->dsi.vertical_backporch 				 = 20;//16
	 params->dsi.vertical_frontporch				 = 20;//20
	 params->dsi.vertical_active_line				 = FRAME_HEIGHT;

	 params->dsi.horizontal_sync_active 			 = 50;//10
	 params->dsi.horizontal_backporch				 = 100;//70;50
	 params->dsi.horizontal_frontporch				 = 80;//80;60
	// params->dsi.horizontal_blanking_pixel			 = 60;//100;80
	 params->dsi.horizontal_active_pixel			 = FRAME_WIDTH;
	 params->dsi.HS_TRAIL                             = 12;
	// Bit rate calculation
	//params->dsi.LPX=8;

	params->dsi.PLL_CLOCK=300;
	params->dsi.ssc_disable = 1;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F6_ZR_M82)
	params->dsi.vertical_sync_active		= 8;
	params->dsi.vertical_backporch			= 24;
	params->dsi.vertical_frontporch			= 16;
	params->dsi.vertical_active_line		= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active		= 40;
	params->dsi.horizontal_backporch		= 100;
	params->dsi.horizontal_frontporch		= 70;
	params->dsi.horizontal_active_pixel		= FRAME_WIDTH;

	params->dsi.HS_TRAIL = 20;
	params->dsi.PLL_CLOCK = 212;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F6_BPZN2)
	params->dsi.vertical_sync_active			= 10;
	params->dsi.vertical_backporch				= 20;
	params->dsi.vertical_frontporch				= 10;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 10;
	params->dsi.horizontal_backporch			= 150;
	params->dsi.horizontal_frontporch			= 80;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 250;
#elif defined(DROI_PRO_FQ5CW_NJX)
	    params->dsi.vertical_sync_active				= 8;
	    params->dsi.vertical_backporch				= 24;
	    params->dsi.vertical_frontporch				= 16;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 40;
	    params->dsi.horizontal_backporch				= 100;
	    params->dsi.horizontal_frontporch				= 70;
	    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

        params->dsi.HS_TRAIL = 20;
	    params->dsi.PLL_CLOCK = 287;
#else
	    params->dsi.vertical_sync_active				= 10;
	    params->dsi.vertical_backporch				= 20;
	    params->dsi.vertical_frontporch				= 10;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 40;
	    params->dsi.horizontal_backporch				= 120;
	    params->dsi.horizontal_frontporch				= 80;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 208;
#endif
}

static void lcm_init(void)
{
#if defined(SUPORT_ADC_CHECK)
	int val = 0;
#endif
#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#if defined(DROI_PRO_PU6_JF_K20)
	mt_set_gpio_mode(GPIO_LDO28_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO28_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO28_EN, GPIO_OUT_ONE);
	mt_set_gpio_mode(GPIO_LDO18_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO18_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO18_EN, GPIO_OUT_ONE);

	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(20);
#else
	mt_set_gpio_mode(GPIO_LDO28_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO28_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO28_EN, GPIO_OUT_ONE);
	mt_set_gpio_mode(GPIO_LDO18_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO18_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO18_EN, GPIO_OUT_ONE);
	//mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	//mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	//mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
  MDELAY(20);
#endif
#endif
#if defined(DROI_PRO_F5C_HW)
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
#elif defined(DROI_PRO_F5C_LEAGOO)
    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(100);
#elif defined(DROI_PRO_FQ5CW_TW)
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
#elif defined(DROI_PRO_F6_GB) || defined(DROI_PRO_F6_G2)
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(100);
#elif defined(DROI_PRO_PU6_JF_K20)
	//NULL
#else
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(30);
    SET_RESET_PIN(1);
    MDELAY(120);
#endif
#if defined(DROI_PRO_F5C_JF2)||defined(DROI_PRO_F5C_DT)||defined(DROI_PRO_F5C_LEAGOO4)||defined(DROI_PRO_F5C_SGDZ3)||defined(DROI_PRO_F5C_HN)
	init_lcm_registers();
#elif defined(SUPORT_ADC_CHECK)
	val=lcm_read_ADC_value();
#ifdef BUILD_LK
	printf("TYD------------------- val:%d\n",val);
#else
	printk("TYD------------------- val:%d\n",val);
#endif
	adc_lcm_push_table(val);
#else
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}

static void lcm_suspend(void)
{

#if defined(DROI_PRO_F5C_JBT2)
	 unsigned int data_array[16];
	 data_array[0]=0x00280500;
	 dsi_set_cmdq(data_array, 1, 1);
	 MDELAY(20);
	 data_array[0] = 0x00100500;
	 dsi_set_cmdq(data_array, 1, 1);
	 MDELAY(120);
	 SET_RESET_PIN(0);
	 MDELAY(50);
#elif defined(DROI_PRO_F6_JF)||defined(DROI_PRO_F5C_ZXHL_A6)||defined(DROI_PRO_PU6_OLK)
	unsigned int data_array[16];
	data_array[0]=0x00042902;
	data_array[1]=0x018198ff;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00022902;
	data_array[1]=0x00001053;
	dsi_set_cmdq(data_array, 2, 1);


	data_array[0]=0x00022902;
	data_array[1]=0x00003fb3;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00022902;
	data_array[1]=0x00003fd3;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00042902;
	data_array[1]=0x048198ff;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00022902;
	data_array[1]=0x0000022d;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00022902;
	data_array[1]=0x0000012f;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(120);

	data_array[0]=0x00022902;
	data_array[1]=0x0000002f;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00042902;
	data_array[1]=0x008198ff;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00022902;
	data_array[1]=0x0000002f;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00280500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00100500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
#else
    MDELAY(1);
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#if defined(DROI_PRO_F5C_LEAGOO)||defined(DROI_PRO_F5C_DT)||defined(DROI_PRO_F5C_HN)
#elif defined(DROI_PRO_FQ5CW_TW)
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
#elif defined(DROI_PRO_PU6_JF_K20)
	//NULL
#else
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(120);
#endif
#endif
#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
	MDELAY(120);
	mt_set_gpio_mode(GPIO_LDO28_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO28_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO28_EN, GPIO_OUT_ZERO);
	mt_set_gpio_mode(GPIO_LDO18_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO18_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO18_EN, GPIO_OUT_ZERO);
	//mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	//mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	//mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(10);
#if defined(DROI_PRO_PU6_JF_K20)
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
#else
	mt_set_gpio_mode(GPIO_LDO18_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO18_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO18_EN, GPIO_OUT_ZERO);
	//mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	//mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	//mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
#endif
#endif
}


static void lcm_resume(void)
{

	lcm_init();
}

static unsigned int lcm_compare_id(void)
{
	int array[4];
	char buffer[3];
	char id_high=0;
	char id_low=0;
	int id=0;

	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(100);

	//{0x39, 0xFF, 5, { 0xFF,0x98,0x06,0x04,0x01}}, // Change to Page 1 CMD
	array[0] = 0x00043902;
	array[1] = 0x018198FF;
	dsi_set_cmdq(array, 2, 1);

	array[0] = 0x00013700; //return byte number
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x00, &buffer[0], 1);  //0x98

	array[0] = 0x00013700; //return byte number
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x01, &buffer[1], 1);  //0x81

	//id = (buffer[0]<<8) | buffer[1];

        id_high = buffer[0];
        id_low = buffer[1];
        id = (id_high<<8)|id_low;


   #ifdef BUILD_LK

		printf("zbuffer=%d %d %d \n", buffer[0],buffer[1], buffer[2]);
		printf("id =0x%x", id);
#else
		printk("zbuffer=%d %d %d \n", buffer[0],buffer[1], buffer[2]);
		printk("id =0x%x", id);
   #endif
	return (ILI9881C_ID == id)?1:0;


}

#if defined(DROI_PRO_F5C_JF2)||defined(DROI_PRO_F5C_DT)
static void ILI9881_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];

    data_array[0] = (0x00022902);
	data_array[1] = (0x00000000 | (para << 8) | (cmd));
	dsi_set_cmdq(data_array, 2, 1);

}

static void ILI9881_DCS_write_1A_0P(unsigned char cmd)
{
unsigned int data_array[16];

	data_array[0]=(0x00000500 | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

}


static void init_lcm_registers(void)
{

	unsigned int data_array[16];

        data_array[0] = (0x00042902);
	data_array[1] = (0x038198FF);
	dsi_set_cmdq(data_array, 2, 1);

ILI9881_DCS_write_1A_1P(0x01,0x00);
ILI9881_DCS_write_1A_1P(0x02,0x00);
ILI9881_DCS_write_1A_1P(0x03,0x73);
ILI9881_DCS_write_1A_1P(0x04,0x00);
ILI9881_DCS_write_1A_1P(0x05,0x00);
ILI9881_DCS_write_1A_1P(0x06,0x0A);
ILI9881_DCS_write_1A_1P(0x07,0x00);
ILI9881_DCS_write_1A_1P(0x08,0x00);
ILI9881_DCS_write_1A_1P(0x09,0x01);
ILI9881_DCS_write_1A_1P(0x0a,0x00);
ILI9881_DCS_write_1A_1P(0x0b,0x00);
ILI9881_DCS_write_1A_1P(0x0c,0x01);
ILI9881_DCS_write_1A_1P(0x0d,0x00);
ILI9881_DCS_write_1A_1P(0x0e,0x00);
ILI9881_DCS_write_1A_1P(0x0f,0x1D);
ILI9881_DCS_write_1A_1P(0x10,0x1D);
ILI9881_DCS_write_1A_1P(0x11,0x00);
ILI9881_DCS_write_1A_1P(0x12,0x00);
ILI9881_DCS_write_1A_1P(0x13,0x00);
ILI9881_DCS_write_1A_1P(0x14,0x00);
ILI9881_DCS_write_1A_1P(0x15,0x00);
ILI9881_DCS_write_1A_1P(0x16,0x00);
ILI9881_DCS_write_1A_1P(0x17,0x00);
ILI9881_DCS_write_1A_1P(0x18,0x00);
ILI9881_DCS_write_1A_1P(0x19,0x00);
ILI9881_DCS_write_1A_1P(0x1a,0x00);
ILI9881_DCS_write_1A_1P(0x1b,0x00);
ILI9881_DCS_write_1A_1P(0x1c,0x00);
ILI9881_DCS_write_1A_1P(0x1d,0x00);
ILI9881_DCS_write_1A_1P(0x1e,0x40);
ILI9881_DCS_write_1A_1P(0x1f,0x80);
ILI9881_DCS_write_1A_1P(0x20,0x06);
ILI9881_DCS_write_1A_1P(0x21,0x02);
ILI9881_DCS_write_1A_1P(0x22,0x00);
ILI9881_DCS_write_1A_1P(0x23,0x00);
ILI9881_DCS_write_1A_1P(0x24,0x00);
ILI9881_DCS_write_1A_1P(0x25,0x00);
ILI9881_DCS_write_1A_1P(0x26,0x00);
ILI9881_DCS_write_1A_1P(0x27,0x00);
ILI9881_DCS_write_1A_1P(0x28,0x33);
ILI9881_DCS_write_1A_1P(0x29,0x03);
ILI9881_DCS_write_1A_1P(0x2a,0x00);
ILI9881_DCS_write_1A_1P(0x2b,0x00);
ILI9881_DCS_write_1A_1P(0x2c,0x00);
ILI9881_DCS_write_1A_1P(0x2d,0x00);
ILI9881_DCS_write_1A_1P(0x2e,0x00);
ILI9881_DCS_write_1A_1P(0x2f,0x00);
ILI9881_DCS_write_1A_1P(0x30,0x00);
ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x32,0x00);
ILI9881_DCS_write_1A_1P(0x33,0x00);
ILI9881_DCS_write_1A_1P(0x34,0x04);
ILI9881_DCS_write_1A_1P(0x35,0x00);
ILI9881_DCS_write_1A_1P(0x36,0x00);
ILI9881_DCS_write_1A_1P(0x37,0x00);
ILI9881_DCS_write_1A_1P(0x38,0x3C);
ILI9881_DCS_write_1A_1P(0x39,0x00);
ILI9881_DCS_write_1A_1P(0x3a,0x40);
ILI9881_DCS_write_1A_1P(0x3b,0x40);
ILI9881_DCS_write_1A_1P(0x3c,0x00);
ILI9881_DCS_write_1A_1P(0x3d,0x00);
ILI9881_DCS_write_1A_1P(0x3e,0x00);
ILI9881_DCS_write_1A_1P(0x3f,0x00);
ILI9881_DCS_write_1A_1P(0x40,0x00);
ILI9881_DCS_write_1A_1P(0x41,0x00);
ILI9881_DCS_write_1A_1P(0x42,0x00);
ILI9881_DCS_write_1A_1P(0x43,0x00);
ILI9881_DCS_write_1A_1P(0x44,0x00);
ILI9881_DCS_write_1A_1P(0x50,0x01);
ILI9881_DCS_write_1A_1P(0x51,0x23);
ILI9881_DCS_write_1A_1P(0x52,0x45);
ILI9881_DCS_write_1A_1P(0x53,0x67);
ILI9881_DCS_write_1A_1P(0x54,0x89);
ILI9881_DCS_write_1A_1P(0x55,0xab);
ILI9881_DCS_write_1A_1P(0x56,0x01);
ILI9881_DCS_write_1A_1P(0x57,0x23);
ILI9881_DCS_write_1A_1P(0x58,0x45);
ILI9881_DCS_write_1A_1P(0x59,0x67);
ILI9881_DCS_write_1A_1P(0x5a,0x89);
ILI9881_DCS_write_1A_1P(0x5b,0xab);
ILI9881_DCS_write_1A_1P(0x5c,0xcd);
ILI9881_DCS_write_1A_1P(0x5d,0xef);

//GIP_3
ILI9881_DCS_write_1A_1P(0x5e,0x11);
ILI9881_DCS_write_1A_1P(0x5f,0x01);
ILI9881_DCS_write_1A_1P(0x60,0x00);
ILI9881_DCS_write_1A_1P(0x61,0x15);
ILI9881_DCS_write_1A_1P(0x62,0x14);
ILI9881_DCS_write_1A_1P(0x63,0x0E);
ILI9881_DCS_write_1A_1P(0x64,0x0F);
ILI9881_DCS_write_1A_1P(0x65,0x0C);
ILI9881_DCS_write_1A_1P(0x66,0x0D);
ILI9881_DCS_write_1A_1P(0x67,0x06);
ILI9881_DCS_write_1A_1P(0x68,0x02);
ILI9881_DCS_write_1A_1P(0x69,0x07);
ILI9881_DCS_write_1A_1P(0x6a,0x02);
ILI9881_DCS_write_1A_1P(0x6b,0x02);
ILI9881_DCS_write_1A_1P(0x6c,0x02);
ILI9881_DCS_write_1A_1P(0x6d,0x02);
ILI9881_DCS_write_1A_1P(0x6e,0x02);
ILI9881_DCS_write_1A_1P(0x6f,0x02);
ILI9881_DCS_write_1A_1P(0x70,0x02);
ILI9881_DCS_write_1A_1P(0x71,0x02);
ILI9881_DCS_write_1A_1P(0x72,0x02);
ILI9881_DCS_write_1A_1P(0x73,0x02);
ILI9881_DCS_write_1A_1P(0x74,0x02);
ILI9881_DCS_write_1A_1P(0x75,0x01);
ILI9881_DCS_write_1A_1P(0x76,0x00);
ILI9881_DCS_write_1A_1P(0x77,0x14);
ILI9881_DCS_write_1A_1P(0x78,0x15);
ILI9881_DCS_write_1A_1P(0x79,0x0E);
ILI9881_DCS_write_1A_1P(0x7a,0x0F);
ILI9881_DCS_write_1A_1P(0x7b,0x0C);
ILI9881_DCS_write_1A_1P(0x7c,0x0D);
ILI9881_DCS_write_1A_1P(0x7d,0x06);
ILI9881_DCS_write_1A_1P(0x7e,0x02);
ILI9881_DCS_write_1A_1P(0x7f,0x07);
ILI9881_DCS_write_1A_1P(0x80,0x02);
ILI9881_DCS_write_1A_1P(0x81,0x02);
ILI9881_DCS_write_1A_1P(0x82,0x02);
ILI9881_DCS_write_1A_1P(0x83,0x02);
ILI9881_DCS_write_1A_1P(0x84,0x02);
ILI9881_DCS_write_1A_1P(0x85,0x02);
ILI9881_DCS_write_1A_1P(0x86,0x02);
ILI9881_DCS_write_1A_1P(0x87,0x02);
ILI9881_DCS_write_1A_1P(0x88,0x02);
ILI9881_DCS_write_1A_1P(0x89,0x02);
ILI9881_DCS_write_1A_1P(0x8A,0x02);

        data_array[0] = (0x00042902);
	data_array[1] = (0x048198FF);
	dsi_set_cmdq(data_array, 2, 1);


ILI9881_DCS_write_1A_1P(0x6C,0x15);
ILI9881_DCS_write_1A_1P(0x6E,0x2A);
ILI9881_DCS_write_1A_1P(0x6F,0x35);
ILI9881_DCS_write_1A_1P(0x3A,0x94);
ILI9881_DCS_write_1A_1P(0x8D,0x14);
ILI9881_DCS_write_1A_1P(0x87,0xBA);
ILI9881_DCS_write_1A_1P(0x26,0x76);
ILI9881_DCS_write_1A_1P(0xB2,0xD1);
ILI9881_DCS_write_1A_1P(0xB5,0x06);



        data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);
ILI9881_DCS_write_1A_1P(0x22,0x09);
ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x53,0x96);
ILI9881_DCS_write_1A_1P(0x55,0x9A);
ILI9881_DCS_write_1A_1P(0x50,0xB7);
ILI9881_DCS_write_1A_1P(0x51,0xB7);
ILI9881_DCS_write_1A_1P(0x60,0x22);
ILI9881_DCS_write_1A_1P(0x61,0x00);
ILI9881_DCS_write_1A_1P(0x62,0x19);
ILI9881_DCS_write_1A_1P(0x63,0x10);
ILI9881_DCS_write_1A_1P(0xA0,0x08);
ILI9881_DCS_write_1A_1P(0xA1,0x1A);
ILI9881_DCS_write_1A_1P(0xA2,0x27);
ILI9881_DCS_write_1A_1P(0xA3,0x15);
ILI9881_DCS_write_1A_1P(0xA4,0x17);
ILI9881_DCS_write_1A_1P(0xA5,0x2A);
ILI9881_DCS_write_1A_1P(0xA6,0x1E);
ILI9881_DCS_write_1A_1P(0xA7,0x1F);
ILI9881_DCS_write_1A_1P(0xA8,0x8B);
ILI9881_DCS_write_1A_1P(0xA9,0x1B);
ILI9881_DCS_write_1A_1P(0xAA,0x27);
ILI9881_DCS_write_1A_1P(0xAB,0x78);
ILI9881_DCS_write_1A_1P(0xAC,0x18);
ILI9881_DCS_write_1A_1P(0xAD,0x18);
ILI9881_DCS_write_1A_1P(0xAE,0x4C);
ILI9881_DCS_write_1A_1P(0xAF,0x21);
ILI9881_DCS_write_1A_1P(0xB0,0x27);
ILI9881_DCS_write_1A_1P(0xB1,0x54);
ILI9881_DCS_write_1A_1P(0xB2,0x67);
ILI9881_DCS_write_1A_1P(0xB3,0x39);
ILI9881_DCS_write_1A_1P(0xC0,0x08);
ILI9881_DCS_write_1A_1P(0xC1,0x1A);
ILI9881_DCS_write_1A_1P(0xC2,0x27);
ILI9881_DCS_write_1A_1P(0xC3,0x15);
ILI9881_DCS_write_1A_1P(0xC4,0x17);
ILI9881_DCS_write_1A_1P(0xC5,0x2A);
ILI9881_DCS_write_1A_1P(0xC6,0x1E);
ILI9881_DCS_write_1A_1P(0xC7,0x1F);
ILI9881_DCS_write_1A_1P(0xC8,0x8B);
ILI9881_DCS_write_1A_1P(0xC9,0x1B);
ILI9881_DCS_write_1A_1P(0xCA,0x27);
ILI9881_DCS_write_1A_1P(0xCB,0x78);
ILI9881_DCS_write_1A_1P(0xCC,0x18);
ILI9881_DCS_write_1A_1P(0xCD,0x18);
ILI9881_DCS_write_1A_1P(0xCE,0x4C);
ILI9881_DCS_write_1A_1P(0xCF,0x21);
ILI9881_DCS_write_1A_1P(0xD0,0x27);
ILI9881_DCS_write_1A_1P(0xD1,0x54);
ILI9881_DCS_write_1A_1P(0xD2,0x67);
ILI9881_DCS_write_1A_1P(0xD3,0x39);


data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);
ILI9881_DCS_write_1A_1P(0x60,0x22);

        data_array[0] = (0x00042902);
	data_array[1] = (0x008198FF);
	dsi_set_cmdq(data_array, 2, 1);
#if defined(DROI_PRO_F5C_DT)
ILI9881_DCS_write_1A_1P(0x36,0x03);
#endif
        ILI9881_DCS_write_1A_0P(0x11);
        MDELAY(120);

        ILI9881_DCS_write_1A_0P(0x29);
        MDELAY(60);

}
#endif
#if defined(DROI_PRO_F5C_HN)
static void ILI9881_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];

    data_array[0] = (0x00022902);
	data_array[1] = (0x00000000 | (para << 8) | (cmd));
	dsi_set_cmdq(data_array, 2, 1);

}

static void ILI9881_DCS_write_1A_0P(unsigned char cmd)
{
unsigned int data_array[16];

	data_array[0]=(0x00000500 | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

}


static void init_lcm_registers(void)
{

	unsigned int data_array[16];

        data_array[0] = (0x00042902);
	data_array[1] = (0x038198FF);
	dsi_set_cmdq(data_array, 2, 1);

ILI9881_DCS_write_1A_1P(0x01,0x00);
ILI9881_DCS_write_1A_1P(0x02,0x00);
ILI9881_DCS_write_1A_1P(0x03,0x73);
ILI9881_DCS_write_1A_1P(0x04,0x00);
ILI9881_DCS_write_1A_1P(0x05,0x00);
ILI9881_DCS_write_1A_1P(0x06,0x0A);
ILI9881_DCS_write_1A_1P(0x07,0x00);
ILI9881_DCS_write_1A_1P(0x08,0x00);
ILI9881_DCS_write_1A_1P(0x09,0x01);
ILI9881_DCS_write_1A_1P(0x0a,0x00);
ILI9881_DCS_write_1A_1P(0x0b,0x00);
ILI9881_DCS_write_1A_1P(0x0c,0x01);
ILI9881_DCS_write_1A_1P(0x0d,0x00);
ILI9881_DCS_write_1A_1P(0x0e,0x00);
ILI9881_DCS_write_1A_1P(0x0f,0x1D);
ILI9881_DCS_write_1A_1P(0x10,0x1D);
ILI9881_DCS_write_1A_1P(0x11,0x00);
ILI9881_DCS_write_1A_1P(0x12,0x00);
ILI9881_DCS_write_1A_1P(0x13,0x00);
ILI9881_DCS_write_1A_1P(0x14,0x00);
ILI9881_DCS_write_1A_1P(0x15,0x00);
ILI9881_DCS_write_1A_1P(0x16,0x00);
ILI9881_DCS_write_1A_1P(0x17,0x00);
ILI9881_DCS_write_1A_1P(0x18,0x00);
ILI9881_DCS_write_1A_1P(0x19,0x00);
ILI9881_DCS_write_1A_1P(0x1a,0x00);
ILI9881_DCS_write_1A_1P(0x1b,0x00);
ILI9881_DCS_write_1A_1P(0x1c,0x00);
ILI9881_DCS_write_1A_1P(0x1d,0x00);
ILI9881_DCS_write_1A_1P(0x1e,0x40);
ILI9881_DCS_write_1A_1P(0x1f,0x80);
ILI9881_DCS_write_1A_1P(0x20,0x06);
ILI9881_DCS_write_1A_1P(0x21,0x02);
ILI9881_DCS_write_1A_1P(0x22,0x00);
ILI9881_DCS_write_1A_1P(0x23,0x00);
ILI9881_DCS_write_1A_1P(0x24,0x00);
ILI9881_DCS_write_1A_1P(0x25,0x00);
ILI9881_DCS_write_1A_1P(0x26,0x00);
ILI9881_DCS_write_1A_1P(0x27,0x00);
ILI9881_DCS_write_1A_1P(0x28,0x33);
ILI9881_DCS_write_1A_1P(0x29,0x03);
ILI9881_DCS_write_1A_1P(0x2a,0x00);
ILI9881_DCS_write_1A_1P(0x2b,0x00);
ILI9881_DCS_write_1A_1P(0x2c,0x00);
ILI9881_DCS_write_1A_1P(0x2d,0x00);
ILI9881_DCS_write_1A_1P(0x2e,0x00);
ILI9881_DCS_write_1A_1P(0x2f,0x00);
ILI9881_DCS_write_1A_1P(0x30,0x00);
ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x32,0x00);
ILI9881_DCS_write_1A_1P(0x33,0x00);
ILI9881_DCS_write_1A_1P(0x34,0x04);
ILI9881_DCS_write_1A_1P(0x35,0x00);
ILI9881_DCS_write_1A_1P(0x36,0x00);
ILI9881_DCS_write_1A_1P(0x37,0x00);
ILI9881_DCS_write_1A_1P(0x38,0x3C);
ILI9881_DCS_write_1A_1P(0x39,0x00);
ILI9881_DCS_write_1A_1P(0x3a,0x40);
ILI9881_DCS_write_1A_1P(0x3b,0x40);
ILI9881_DCS_write_1A_1P(0x3c,0x00);
ILI9881_DCS_write_1A_1P(0x3d,0x00);
ILI9881_DCS_write_1A_1P(0x3e,0x00);
ILI9881_DCS_write_1A_1P(0x3f,0x00);
ILI9881_DCS_write_1A_1P(0x40,0x00);
ILI9881_DCS_write_1A_1P(0x41,0x00);
ILI9881_DCS_write_1A_1P(0x42,0x00);
ILI9881_DCS_write_1A_1P(0x43,0x00);
ILI9881_DCS_write_1A_1P(0x44,0x00);
ILI9881_DCS_write_1A_1P(0x50,0x01);
ILI9881_DCS_write_1A_1P(0x51,0x23);
ILI9881_DCS_write_1A_1P(0x52,0x45);
ILI9881_DCS_write_1A_1P(0x53,0x67);
ILI9881_DCS_write_1A_1P(0x54,0x89);
ILI9881_DCS_write_1A_1P(0x55,0xab);
ILI9881_DCS_write_1A_1P(0x56,0x01);
ILI9881_DCS_write_1A_1P(0x57,0x23);
ILI9881_DCS_write_1A_1P(0x58,0x45);
ILI9881_DCS_write_1A_1P(0x59,0x67);
ILI9881_DCS_write_1A_1P(0x5a,0x89);
ILI9881_DCS_write_1A_1P(0x5b,0xab);
ILI9881_DCS_write_1A_1P(0x5c,0xcd);
ILI9881_DCS_write_1A_1P(0x5d,0xef);

//GIP_3
ILI9881_DCS_write_1A_1P(0x5e,0x11);
ILI9881_DCS_write_1A_1P(0x5f,0x01);
ILI9881_DCS_write_1A_1P(0x60,0x00);
ILI9881_DCS_write_1A_1P(0x61,0x15);
ILI9881_DCS_write_1A_1P(0x62,0x14);
ILI9881_DCS_write_1A_1P(0x63,0x0E);
ILI9881_DCS_write_1A_1P(0x64,0x0F);
ILI9881_DCS_write_1A_1P(0x65,0x0C);
ILI9881_DCS_write_1A_1P(0x66,0x0D);
ILI9881_DCS_write_1A_1P(0x67,0x06);
ILI9881_DCS_write_1A_1P(0x68,0x02);
ILI9881_DCS_write_1A_1P(0x69,0x07);
ILI9881_DCS_write_1A_1P(0x6a,0x02);
ILI9881_DCS_write_1A_1P(0x6b,0x02);
ILI9881_DCS_write_1A_1P(0x6c,0x02);
ILI9881_DCS_write_1A_1P(0x6d,0x02);
ILI9881_DCS_write_1A_1P(0x6e,0x02);
ILI9881_DCS_write_1A_1P(0x6f,0x02);
ILI9881_DCS_write_1A_1P(0x70,0x02);
ILI9881_DCS_write_1A_1P(0x71,0x02);
ILI9881_DCS_write_1A_1P(0x72,0x02);
ILI9881_DCS_write_1A_1P(0x73,0x02);
ILI9881_DCS_write_1A_1P(0x74,0x02);
ILI9881_DCS_write_1A_1P(0x75,0x01);
ILI9881_DCS_write_1A_1P(0x76,0x00);
ILI9881_DCS_write_1A_1P(0x77,0x14);
ILI9881_DCS_write_1A_1P(0x78,0x15);
ILI9881_DCS_write_1A_1P(0x79,0x0E);
ILI9881_DCS_write_1A_1P(0x7a,0x0F);
ILI9881_DCS_write_1A_1P(0x7b,0x0C);
ILI9881_DCS_write_1A_1P(0x7c,0x0D);
ILI9881_DCS_write_1A_1P(0x7d,0x06);
ILI9881_DCS_write_1A_1P(0x7e,0x02);
ILI9881_DCS_write_1A_1P(0x7f,0x07);
ILI9881_DCS_write_1A_1P(0x80,0x02);
ILI9881_DCS_write_1A_1P(0x81,0x02);
ILI9881_DCS_write_1A_1P(0x82,0x02);
ILI9881_DCS_write_1A_1P(0x83,0x02);
ILI9881_DCS_write_1A_1P(0x84,0x02);
ILI9881_DCS_write_1A_1P(0x85,0x02);
ILI9881_DCS_write_1A_1P(0x86,0x02);
ILI9881_DCS_write_1A_1P(0x87,0x02);
ILI9881_DCS_write_1A_1P(0x88,0x02);
ILI9881_DCS_write_1A_1P(0x89,0x02);
ILI9881_DCS_write_1A_1P(0x8A,0x02);

        data_array[0] = (0x00042902);
	data_array[1] = (0x048198FF);
	dsi_set_cmdq(data_array, 2, 1);


ILI9881_DCS_write_1A_1P(0x6C,0x15);
ILI9881_DCS_write_1A_1P(0x6E,0x2A);
ILI9881_DCS_write_1A_1P(0x6F,0x35);
ILI9881_DCS_write_1A_1P(0x3A,0x94);
ILI9881_DCS_write_1A_1P(0x8D,0x14);
ILI9881_DCS_write_1A_1P(0x87,0xBA);
ILI9881_DCS_write_1A_1P(0x26,0x76);
ILI9881_DCS_write_1A_1P(0xB2,0xD1);
ILI9881_DCS_write_1A_1P(0xB5,0x06);



        data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);
ILI9881_DCS_write_1A_1P(0x22,0x0A);
ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x53,0x9E);
ILI9881_DCS_write_1A_1P(0x55,0x9A);
ILI9881_DCS_write_1A_1P(0x50,0xB7);
ILI9881_DCS_write_1A_1P(0x51,0xB7);
ILI9881_DCS_write_1A_1P(0x60,0x22);
ILI9881_DCS_write_1A_1P(0x61,0x00);
ILI9881_DCS_write_1A_1P(0x62,0x19);
ILI9881_DCS_write_1A_1P(0x63,0x10);
ILI9881_DCS_write_1A_1P(0xA0,0x08);
ILI9881_DCS_write_1A_1P(0xA1,0x1A);
ILI9881_DCS_write_1A_1P(0xA2,0x27);
ILI9881_DCS_write_1A_1P(0xA3,0x15);
ILI9881_DCS_write_1A_1P(0xA4,0x17);
ILI9881_DCS_write_1A_1P(0xA5,0x2A);
ILI9881_DCS_write_1A_1P(0xA6,0x1E);
ILI9881_DCS_write_1A_1P(0xA7,0x1F);
ILI9881_DCS_write_1A_1P(0xA8,0x8B);
ILI9881_DCS_write_1A_1P(0xA9,0x1B);
ILI9881_DCS_write_1A_1P(0xAA,0x27);
ILI9881_DCS_write_1A_1P(0xAB,0x78);
ILI9881_DCS_write_1A_1P(0xAC,0x18);
ILI9881_DCS_write_1A_1P(0xAD,0x18);
ILI9881_DCS_write_1A_1P(0xAE,0x4C);
ILI9881_DCS_write_1A_1P(0xAF,0x21);
ILI9881_DCS_write_1A_1P(0xB0,0x27);
ILI9881_DCS_write_1A_1P(0xB1,0x54);
ILI9881_DCS_write_1A_1P(0xB2,0x67);
ILI9881_DCS_write_1A_1P(0xB3,0x39);
ILI9881_DCS_write_1A_1P(0xC0,0x08);
ILI9881_DCS_write_1A_1P(0xC1,0x1A);
ILI9881_DCS_write_1A_1P(0xC2,0x27);
ILI9881_DCS_write_1A_1P(0xC3,0x15);
ILI9881_DCS_write_1A_1P(0xC4,0x17);
ILI9881_DCS_write_1A_1P(0xC5,0x2A);
ILI9881_DCS_write_1A_1P(0xC6,0x1E);
ILI9881_DCS_write_1A_1P(0xC7,0x1F);
ILI9881_DCS_write_1A_1P(0xC8,0x8B);
ILI9881_DCS_write_1A_1P(0xC9,0x1B);
ILI9881_DCS_write_1A_1P(0xCA,0x27);
ILI9881_DCS_write_1A_1P(0xCB,0x78);
ILI9881_DCS_write_1A_1P(0xCC,0x18);
ILI9881_DCS_write_1A_1P(0xCD,0x18);
ILI9881_DCS_write_1A_1P(0xCE,0x4C);
ILI9881_DCS_write_1A_1P(0xCF,0x21);
ILI9881_DCS_write_1A_1P(0xD0,0x27);
ILI9881_DCS_write_1A_1P(0xD1,0x54);
ILI9881_DCS_write_1A_1P(0xD2,0x67);
ILI9881_DCS_write_1A_1P(0xD3,0x39);


data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);
ILI9881_DCS_write_1A_1P(0x60,0x22);

        data_array[0] = (0x00042902);
	data_array[1] = (0x008198FF);
	dsi_set_cmdq(data_array, 2, 1);
        ILI9881_DCS_write_1A_0P(0x11);
        MDELAY(120);

        ILI9881_DCS_write_1A_0P(0x29);
        MDELAY(60);

}
#endif

#if defined(DROI_PRO_F5C_LEAGOO4)
static void ILI9881_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];

    data_array[0] = (0x00022902);
	data_array[1] = (0x00000000 | (para << 8) | (cmd));
	dsi_set_cmdq(data_array, 2, 1);

}

static void ILI9881_DCS_write_1A_0P(unsigned char cmd)
{
unsigned int data_array[16];

	data_array[0]=(0x00000500 | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

}


static void init_lcm_registers(void)
{

	unsigned int data_array[16];

        data_array[0] = (0x00042902);
	data_array[1] = (0x038198FF);
	dsi_set_cmdq(data_array, 2, 1);

ILI9881_DCS_write_1A_1P(0x01,0x00);
ILI9881_DCS_write_1A_1P(0x02,0x00);
ILI9881_DCS_write_1A_1P(0x03,0x73);
ILI9881_DCS_write_1A_1P(0x04,0x00);
ILI9881_DCS_write_1A_1P(0x05,0x00);
ILI9881_DCS_write_1A_1P(0x06,0x0A);
ILI9881_DCS_write_1A_1P(0x07,0x00);
ILI9881_DCS_write_1A_1P(0x08,0x00);
ILI9881_DCS_write_1A_1P(0x09,0x01);
ILI9881_DCS_write_1A_1P(0x0a,0x00);
ILI9881_DCS_write_1A_1P(0x0b,0x00);
ILI9881_DCS_write_1A_1P(0x0c,0x01);
ILI9881_DCS_write_1A_1P(0x0d,0x00);
ILI9881_DCS_write_1A_1P(0x0e,0x00);
ILI9881_DCS_write_1A_1P(0x0f,0x1D);
ILI9881_DCS_write_1A_1P(0x10,0x1D);
ILI9881_DCS_write_1A_1P(0x11,0x00);
ILI9881_DCS_write_1A_1P(0x12,0x00);
ILI9881_DCS_write_1A_1P(0x13,0x00);
ILI9881_DCS_write_1A_1P(0x14,0x00);
ILI9881_DCS_write_1A_1P(0x15,0x00);
ILI9881_DCS_write_1A_1P(0x16,0x00);
ILI9881_DCS_write_1A_1P(0x17,0x00);
ILI9881_DCS_write_1A_1P(0x18,0x00);
ILI9881_DCS_write_1A_1P(0x19,0x00);
ILI9881_DCS_write_1A_1P(0x1a,0x00);
ILI9881_DCS_write_1A_1P(0x1b,0x00);
ILI9881_DCS_write_1A_1P(0x1c,0x00);
ILI9881_DCS_write_1A_1P(0x1d,0x00);
ILI9881_DCS_write_1A_1P(0x1e,0x40);
ILI9881_DCS_write_1A_1P(0x1f,0x80);
ILI9881_DCS_write_1A_1P(0x20,0x06);
ILI9881_DCS_write_1A_1P(0x21,0x02);
ILI9881_DCS_write_1A_1P(0x22,0x00);
ILI9881_DCS_write_1A_1P(0x23,0x00);
ILI9881_DCS_write_1A_1P(0x24,0x00);
ILI9881_DCS_write_1A_1P(0x25,0x00);
ILI9881_DCS_write_1A_1P(0x26,0x00);
ILI9881_DCS_write_1A_1P(0x27,0x00);
ILI9881_DCS_write_1A_1P(0x28,0x33);
ILI9881_DCS_write_1A_1P(0x29,0x03);
ILI9881_DCS_write_1A_1P(0x2a,0x00);
ILI9881_DCS_write_1A_1P(0x2b,0x00);
ILI9881_DCS_write_1A_1P(0x2c,0x00);
ILI9881_DCS_write_1A_1P(0x2d,0x00);
ILI9881_DCS_write_1A_1P(0x2e,0x00);
ILI9881_DCS_write_1A_1P(0x2f,0x00);
ILI9881_DCS_write_1A_1P(0x30,0x00);
ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x32,0x00);
ILI9881_DCS_write_1A_1P(0x33,0x00);
ILI9881_DCS_write_1A_1P(0x34,0x04);
ILI9881_DCS_write_1A_1P(0x35,0x00);
ILI9881_DCS_write_1A_1P(0x36,0x00);
ILI9881_DCS_write_1A_1P(0x37,0x00);
ILI9881_DCS_write_1A_1P(0x38,0x3C);
ILI9881_DCS_write_1A_1P(0x39,0x00);
ILI9881_DCS_write_1A_1P(0x3a,0x40);
ILI9881_DCS_write_1A_1P(0x3b,0x40);
ILI9881_DCS_write_1A_1P(0x3c,0x00);
ILI9881_DCS_write_1A_1P(0x3d,0x00);
ILI9881_DCS_write_1A_1P(0x3e,0x00);
ILI9881_DCS_write_1A_1P(0x3f,0x00);
ILI9881_DCS_write_1A_1P(0x40,0x00);
ILI9881_DCS_write_1A_1P(0x41,0x00);
ILI9881_DCS_write_1A_1P(0x42,0x00);
ILI9881_DCS_write_1A_1P(0x43,0x00);
ILI9881_DCS_write_1A_1P(0x44,0x00);
ILI9881_DCS_write_1A_1P(0x50,0x01);
ILI9881_DCS_write_1A_1P(0x51,0x23);
ILI9881_DCS_write_1A_1P(0x52,0x45);
ILI9881_DCS_write_1A_1P(0x53,0x67);
ILI9881_DCS_write_1A_1P(0x54,0x89);
ILI9881_DCS_write_1A_1P(0x55,0xab);
ILI9881_DCS_write_1A_1P(0x56,0x01);
ILI9881_DCS_write_1A_1P(0x57,0x23);
ILI9881_DCS_write_1A_1P(0x58,0x45);
ILI9881_DCS_write_1A_1P(0x59,0x67);
ILI9881_DCS_write_1A_1P(0x5a,0x89);
ILI9881_DCS_write_1A_1P(0x5b,0xab);
ILI9881_DCS_write_1A_1P(0x5c,0xcd);
ILI9881_DCS_write_1A_1P(0x5d,0xef);

//GIP_3
ILI9881_DCS_write_1A_1P(0x5e,0x11);
ILI9881_DCS_write_1A_1P(0x5f,0x01);
ILI9881_DCS_write_1A_1P(0x60,0x00);
ILI9881_DCS_write_1A_1P(0x61,0x15);
ILI9881_DCS_write_1A_1P(0x62,0x14);
ILI9881_DCS_write_1A_1P(0x63,0x0E);
ILI9881_DCS_write_1A_1P(0x64,0x0F);
ILI9881_DCS_write_1A_1P(0x65,0x0C);
ILI9881_DCS_write_1A_1P(0x66,0x0D);
ILI9881_DCS_write_1A_1P(0x67,0x06);
ILI9881_DCS_write_1A_1P(0x68,0x02);
ILI9881_DCS_write_1A_1P(0x69,0x07);
ILI9881_DCS_write_1A_1P(0x6a,0x02);
ILI9881_DCS_write_1A_1P(0x6b,0x02);
ILI9881_DCS_write_1A_1P(0x6c,0x02);
ILI9881_DCS_write_1A_1P(0x6d,0x02);
ILI9881_DCS_write_1A_1P(0x6e,0x02);
ILI9881_DCS_write_1A_1P(0x6f,0x02);
ILI9881_DCS_write_1A_1P(0x70,0x02);
ILI9881_DCS_write_1A_1P(0x71,0x02);
ILI9881_DCS_write_1A_1P(0x72,0x02);
ILI9881_DCS_write_1A_1P(0x73,0x02);
ILI9881_DCS_write_1A_1P(0x74,0x02);
ILI9881_DCS_write_1A_1P(0x75,0x01);
ILI9881_DCS_write_1A_1P(0x76,0x00);
ILI9881_DCS_write_1A_1P(0x77,0x14);
ILI9881_DCS_write_1A_1P(0x78,0x15);
ILI9881_DCS_write_1A_1P(0x79,0x0E);
ILI9881_DCS_write_1A_1P(0x7a,0x0F);
ILI9881_DCS_write_1A_1P(0x7b,0x0C);
ILI9881_DCS_write_1A_1P(0x7c,0x0D);
ILI9881_DCS_write_1A_1P(0x7d,0x06);
ILI9881_DCS_write_1A_1P(0x7e,0x02);
ILI9881_DCS_write_1A_1P(0x7f,0x07);
ILI9881_DCS_write_1A_1P(0x80,0x02);
ILI9881_DCS_write_1A_1P(0x81,0x02);
ILI9881_DCS_write_1A_1P(0x82,0x02);
ILI9881_DCS_write_1A_1P(0x83,0x02);
ILI9881_DCS_write_1A_1P(0x84,0x02);
ILI9881_DCS_write_1A_1P(0x85,0x02);
ILI9881_DCS_write_1A_1P(0x86,0x02);
ILI9881_DCS_write_1A_1P(0x87,0x02);
ILI9881_DCS_write_1A_1P(0x88,0x02);
ILI9881_DCS_write_1A_1P(0x89,0x02);
ILI9881_DCS_write_1A_1P(0x8A,0x02);

        data_array[0] = (0x00042902);
	data_array[1] = (0x048198FF);
	dsi_set_cmdq(data_array, 2, 1);


ILI9881_DCS_write_1A_1P(0x6C,0x15);
ILI9881_DCS_write_1A_1P(0x6E,0x2A);
ILI9881_DCS_write_1A_1P(0x6F,0x35);
ILI9881_DCS_write_1A_1P(0x3A,0x94);
ILI9881_DCS_write_1A_1P(0x8D,0x14);
ILI9881_DCS_write_1A_1P(0x87,0xBA);
ILI9881_DCS_write_1A_1P(0x26,0x76);
ILI9881_DCS_write_1A_1P(0xB2,0xD1);
ILI9881_DCS_write_1A_1P(0xB5,0x06);



        data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);

#if defined(DROI_PRO_F5C62_LEAGOO4)
ILI9881_DCS_write_1A_1P(0x22,0x0a);
#else
ILI9881_DCS_write_1A_1P(0x22,0x09);
#endif

ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x53,0x96);
ILI9881_DCS_write_1A_1P(0x55,0xA2);
ILI9881_DCS_write_1A_1P(0x50,0xB7);
ILI9881_DCS_write_1A_1P(0x51,0xB7);
ILI9881_DCS_write_1A_1P(0x60,0x22);
ILI9881_DCS_write_1A_1P(0x61,0x00);
ILI9881_DCS_write_1A_1P(0x62,0x19);
ILI9881_DCS_write_1A_1P(0x63,0x10);
ILI9881_DCS_write_1A_1P(0xA0,0x08);
ILI9881_DCS_write_1A_1P(0xA1,0x1A);
ILI9881_DCS_write_1A_1P(0xA2,0x27);
ILI9881_DCS_write_1A_1P(0xA3,0x15);
ILI9881_DCS_write_1A_1P(0xA4,0x17);
ILI9881_DCS_write_1A_1P(0xA5,0x2A);
ILI9881_DCS_write_1A_1P(0xA6,0x1E);
ILI9881_DCS_write_1A_1P(0xA7,0x1F);
ILI9881_DCS_write_1A_1P(0xA8,0x8B);
ILI9881_DCS_write_1A_1P(0xA9,0x1B);
ILI9881_DCS_write_1A_1P(0xAA,0x27);
ILI9881_DCS_write_1A_1P(0xAB,0x78);
ILI9881_DCS_write_1A_1P(0xAC,0x18);
ILI9881_DCS_write_1A_1P(0xAD,0x18);
ILI9881_DCS_write_1A_1P(0xAE,0x4C);
ILI9881_DCS_write_1A_1P(0xAF,0x21);
ILI9881_DCS_write_1A_1P(0xB0,0x27);
ILI9881_DCS_write_1A_1P(0xB1,0x54);
ILI9881_DCS_write_1A_1P(0xB2,0x67);
ILI9881_DCS_write_1A_1P(0xB3,0x39);
ILI9881_DCS_write_1A_1P(0xC0,0x08);
ILI9881_DCS_write_1A_1P(0xC1,0x1A);
ILI9881_DCS_write_1A_1P(0xC2,0x27);
ILI9881_DCS_write_1A_1P(0xC3,0x15);
ILI9881_DCS_write_1A_1P(0xC4,0x17);
ILI9881_DCS_write_1A_1P(0xC5,0x2A);
ILI9881_DCS_write_1A_1P(0xC6,0x1E);
ILI9881_DCS_write_1A_1P(0xC7,0x1F);
ILI9881_DCS_write_1A_1P(0xC8,0x8B);
ILI9881_DCS_write_1A_1P(0xC9,0x1B);
ILI9881_DCS_write_1A_1P(0xCA,0x27);
ILI9881_DCS_write_1A_1P(0xCB,0x78);
ILI9881_DCS_write_1A_1P(0xCC,0x18);
ILI9881_DCS_write_1A_1P(0xCD,0x18);
ILI9881_DCS_write_1A_1P(0xCE,0x4C);
ILI9881_DCS_write_1A_1P(0xCF,0x21);
ILI9881_DCS_write_1A_1P(0xD0,0x27);
ILI9881_DCS_write_1A_1P(0xD1,0x54);
ILI9881_DCS_write_1A_1P(0xD2,0x67);
ILI9881_DCS_write_1A_1P(0xD3,0x39);


data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);
ILI9881_DCS_write_1A_1P(0x60,0x22);

        data_array[0] = (0x00042902);
	data_array[1] = (0x008198FF);
	dsi_set_cmdq(data_array, 2, 1);

//ILI9881_DCS_write_1A_1P(0x36,0x03);

        ILI9881_DCS_write_1A_0P(0x11);
        MDELAY(120);

        ILI9881_DCS_write_1A_0P(0x29);
        MDELAY(60);

}
#endif
#if defined(DROI_PRO_F5C_SGDZ3)

static void ILI9881_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];

    data_array[0] = (0x00022902);
	data_array[1] = (0x00000000 | (para << 8) | (cmd));
	dsi_set_cmdq(data_array, 2, 1);

}

static void ILI9881_DCS_write_1A_0P(unsigned char cmd)
{
unsigned int data_array[16];

	data_array[0]=(0x00000500 | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

}

static void init_lcm_registers(void)
{

	unsigned int data_array[16];

        data_array[0] = (0x00042902);
	data_array[1] = (0x038198FF);
	dsi_set_cmdq(data_array, 2, 1);

ILI9881_DCS_write_1A_1P(0x01,0x00);
ILI9881_DCS_write_1A_1P(0x02,0x00);
ILI9881_DCS_write_1A_1P(0x03,0x73);
ILI9881_DCS_write_1A_1P(0x04,0x00);
ILI9881_DCS_write_1A_1P(0x05,0x00);
ILI9881_DCS_write_1A_1P(0x06,0x0A);
ILI9881_DCS_write_1A_1P(0x07,0x00);
ILI9881_DCS_write_1A_1P(0x08,0x00);
ILI9881_DCS_write_1A_1P(0x09,0x01);
ILI9881_DCS_write_1A_1P(0x0a,0x00);
ILI9881_DCS_write_1A_1P(0x0b,0x00);
ILI9881_DCS_write_1A_1P(0x0c,0x01);
ILI9881_DCS_write_1A_1P(0x0d,0x00);
ILI9881_DCS_write_1A_1P(0x0e,0x00);
ILI9881_DCS_write_1A_1P(0x0f,0x1D);
ILI9881_DCS_write_1A_1P(0x10,0x1D);
ILI9881_DCS_write_1A_1P(0x11,0x00);
ILI9881_DCS_write_1A_1P(0x12,0x00);
ILI9881_DCS_write_1A_1P(0x13,0x00);
ILI9881_DCS_write_1A_1P(0x14,0x00);
ILI9881_DCS_write_1A_1P(0x15,0x00);
ILI9881_DCS_write_1A_1P(0x16,0x00);
ILI9881_DCS_write_1A_1P(0x17,0x00);
ILI9881_DCS_write_1A_1P(0x18,0x00);
ILI9881_DCS_write_1A_1P(0x19,0x00);
ILI9881_DCS_write_1A_1P(0x1a,0x00);
ILI9881_DCS_write_1A_1P(0x1b,0x00);
ILI9881_DCS_write_1A_1P(0x1c,0x00);
ILI9881_DCS_write_1A_1P(0x1d,0x00);
ILI9881_DCS_write_1A_1P(0x1e,0x40);
ILI9881_DCS_write_1A_1P(0x1f,0x80);
ILI9881_DCS_write_1A_1P(0x20,0x06);
ILI9881_DCS_write_1A_1P(0x21,0x02);
ILI9881_DCS_write_1A_1P(0x22,0x00);
ILI9881_DCS_write_1A_1P(0x23,0x00);
ILI9881_DCS_write_1A_1P(0x24,0x00);
ILI9881_DCS_write_1A_1P(0x25,0x00);
ILI9881_DCS_write_1A_1P(0x26,0x00);
ILI9881_DCS_write_1A_1P(0x27,0x00);
ILI9881_DCS_write_1A_1P(0x28,0x33);
ILI9881_DCS_write_1A_1P(0x29,0x03);
ILI9881_DCS_write_1A_1P(0x2a,0x00);
ILI9881_DCS_write_1A_1P(0x2b,0x00);
ILI9881_DCS_write_1A_1P(0x2c,0x00);
ILI9881_DCS_write_1A_1P(0x2d,0x00);
ILI9881_DCS_write_1A_1P(0x2e,0x00);
ILI9881_DCS_write_1A_1P(0x2f,0x00);
ILI9881_DCS_write_1A_1P(0x30,0x00);
ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x32,0x00);
ILI9881_DCS_write_1A_1P(0x33,0x00);
ILI9881_DCS_write_1A_1P(0x34,0x04);
ILI9881_DCS_write_1A_1P(0x35,0x00);
ILI9881_DCS_write_1A_1P(0x36,0x00);
ILI9881_DCS_write_1A_1P(0x37,0x00);
ILI9881_DCS_write_1A_1P(0x38,0x3C);
ILI9881_DCS_write_1A_1P(0x39,0x00);
ILI9881_DCS_write_1A_1P(0x3a,0x40);
ILI9881_DCS_write_1A_1P(0x3b,0x40);
ILI9881_DCS_write_1A_1P(0x3c,0x00);
ILI9881_DCS_write_1A_1P(0x3d,0x00);
ILI9881_DCS_write_1A_1P(0x3e,0x00);
ILI9881_DCS_write_1A_1P(0x3f,0x00);
ILI9881_DCS_write_1A_1P(0x40,0x00);
ILI9881_DCS_write_1A_1P(0x41,0x00);
ILI9881_DCS_write_1A_1P(0x42,0x00);
ILI9881_DCS_write_1A_1P(0x43,0x00);
ILI9881_DCS_write_1A_1P(0x44,0x00);
ILI9881_DCS_write_1A_1P(0x50,0x01);
ILI9881_DCS_write_1A_1P(0x51,0x23);
ILI9881_DCS_write_1A_1P(0x52,0x45);
ILI9881_DCS_write_1A_1P(0x53,0x67);
ILI9881_DCS_write_1A_1P(0x54,0x89);
ILI9881_DCS_write_1A_1P(0x55,0xab);
ILI9881_DCS_write_1A_1P(0x56,0x01);
ILI9881_DCS_write_1A_1P(0x57,0x23);
ILI9881_DCS_write_1A_1P(0x58,0x45);
ILI9881_DCS_write_1A_1P(0x59,0x67);
ILI9881_DCS_write_1A_1P(0x5a,0x89);
ILI9881_DCS_write_1A_1P(0x5b,0xab);
ILI9881_DCS_write_1A_1P(0x5c,0xcd);
ILI9881_DCS_write_1A_1P(0x5d,0xef);

//GIP_3
ILI9881_DCS_write_1A_1P(0x5e,0x11);
ILI9881_DCS_write_1A_1P(0x5f,0x01);
ILI9881_DCS_write_1A_1P(0x60,0x00);
ILI9881_DCS_write_1A_1P(0x61,0x15);
ILI9881_DCS_write_1A_1P(0x62,0x14);
ILI9881_DCS_write_1A_1P(0x63,0x0E);
ILI9881_DCS_write_1A_1P(0x64,0x0F);
ILI9881_DCS_write_1A_1P(0x65,0x0C);
ILI9881_DCS_write_1A_1P(0x66,0x0D);
ILI9881_DCS_write_1A_1P(0x67,0x06);
ILI9881_DCS_write_1A_1P(0x68,0x02);
ILI9881_DCS_write_1A_1P(0x69,0x07);
ILI9881_DCS_write_1A_1P(0x6a,0x02);
ILI9881_DCS_write_1A_1P(0x6b,0x02);
ILI9881_DCS_write_1A_1P(0x6c,0x02);
ILI9881_DCS_write_1A_1P(0x6d,0x02);
ILI9881_DCS_write_1A_1P(0x6e,0x02);
ILI9881_DCS_write_1A_1P(0x6f,0x02);
ILI9881_DCS_write_1A_1P(0x70,0x02);
ILI9881_DCS_write_1A_1P(0x71,0x02);
ILI9881_DCS_write_1A_1P(0x72,0x02);
ILI9881_DCS_write_1A_1P(0x73,0x02);
ILI9881_DCS_write_1A_1P(0x74,0x02);
ILI9881_DCS_write_1A_1P(0x75,0x01);
ILI9881_DCS_write_1A_1P(0x76,0x00);
ILI9881_DCS_write_1A_1P(0x77,0x14);
ILI9881_DCS_write_1A_1P(0x78,0x15);
ILI9881_DCS_write_1A_1P(0x79,0x0E);
ILI9881_DCS_write_1A_1P(0x7a,0x0F);
ILI9881_DCS_write_1A_1P(0x7b,0x0C);
ILI9881_DCS_write_1A_1P(0x7c,0x0D);
ILI9881_DCS_write_1A_1P(0x7d,0x06);
ILI9881_DCS_write_1A_1P(0x7e,0x02);
ILI9881_DCS_write_1A_1P(0x7f,0x07);
ILI9881_DCS_write_1A_1P(0x80,0x02);
ILI9881_DCS_write_1A_1P(0x81,0x02);
ILI9881_DCS_write_1A_1P(0x82,0x02);
ILI9881_DCS_write_1A_1P(0x83,0x02);
ILI9881_DCS_write_1A_1P(0x84,0x02);
ILI9881_DCS_write_1A_1P(0x85,0x02);
ILI9881_DCS_write_1A_1P(0x86,0x02);
ILI9881_DCS_write_1A_1P(0x87,0x02);
ILI9881_DCS_write_1A_1P(0x88,0x02);
ILI9881_DCS_write_1A_1P(0x89,0x02);
ILI9881_DCS_write_1A_1P(0x8A,0x02);

        data_array[0] = (0x00042902);
	data_array[1] = (0x048198FF);
	dsi_set_cmdq(data_array, 2, 1);


ILI9881_DCS_write_1A_1P(0x6C,0x15);
ILI9881_DCS_write_1A_1P(0x6E,0x2A);
ILI9881_DCS_write_1A_1P(0x6F,0x35);
ILI9881_DCS_write_1A_1P(0x3A,0x94);
ILI9881_DCS_write_1A_1P(0x8D,0x14);
ILI9881_DCS_write_1A_1P(0x87,0xBA);
ILI9881_DCS_write_1A_1P(0x88,0x0B);
ILI9881_DCS_write_1A_1P(0x26,0x76);
ILI9881_DCS_write_1A_1P(0xB2,0xD1);
ILI9881_DCS_write_1A_1P(0xB5,0x06);



        data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);
ILI9881_DCS_write_1A_1P(0x22,0x09);
ILI9881_DCS_write_1A_1P(0x31,0x00);
ILI9881_DCS_write_1A_1P(0x53,0x9a);
ILI9881_DCS_write_1A_1P(0x55,0x9a);
ILI9881_DCS_write_1A_1P(0x50,0xB7);
ILI9881_DCS_write_1A_1P(0x51,0xB7);
ILI9881_DCS_write_1A_1P(0x60,0x22);
ILI9881_DCS_write_1A_1P(0x61,0x00);
ILI9881_DCS_write_1A_1P(0x62,0x19);
ILI9881_DCS_write_1A_1P(0x63,0x10);
ILI9881_DCS_write_1A_1P(0xA0,0x08);
ILI9881_DCS_write_1A_1P(0xA1,0x1A);
ILI9881_DCS_write_1A_1P(0xA2,0x27);
ILI9881_DCS_write_1A_1P(0xA3,0x15);
ILI9881_DCS_write_1A_1P(0xA4,0x17);
ILI9881_DCS_write_1A_1P(0xA5,0x2A);
ILI9881_DCS_write_1A_1P(0xA6,0x1E);
ILI9881_DCS_write_1A_1P(0xA7,0x1F);
ILI9881_DCS_write_1A_1P(0xA8,0x8B);
ILI9881_DCS_write_1A_1P(0xA9,0x1B);
ILI9881_DCS_write_1A_1P(0xAA,0x27);
ILI9881_DCS_write_1A_1P(0xAB,0x78);
ILI9881_DCS_write_1A_1P(0xAC,0x18);
ILI9881_DCS_write_1A_1P(0xAD,0x18);
ILI9881_DCS_write_1A_1P(0xAE,0x4C);
ILI9881_DCS_write_1A_1P(0xAF,0x21);
ILI9881_DCS_write_1A_1P(0xB0,0x27);
ILI9881_DCS_write_1A_1P(0xB1,0x54);
ILI9881_DCS_write_1A_1P(0xB2,0x67);
ILI9881_DCS_write_1A_1P(0xB3,0x39);
ILI9881_DCS_write_1A_1P(0xC0,0x08);
ILI9881_DCS_write_1A_1P(0xC1,0x1A);
ILI9881_DCS_write_1A_1P(0xC2,0x27);
ILI9881_DCS_write_1A_1P(0xC3,0x15);
ILI9881_DCS_write_1A_1P(0xC4,0x17);
ILI9881_DCS_write_1A_1P(0xC5,0x2A);
ILI9881_DCS_write_1A_1P(0xC6,0x1E);
ILI9881_DCS_write_1A_1P(0xC7,0x1F);
ILI9881_DCS_write_1A_1P(0xC8,0x8B);
ILI9881_DCS_write_1A_1P(0xC9,0x1B);
ILI9881_DCS_write_1A_1P(0xCA,0x27);
ILI9881_DCS_write_1A_1P(0xCB,0x78);
ILI9881_DCS_write_1A_1P(0xCC,0x18);
ILI9881_DCS_write_1A_1P(0xCD,0x18);
ILI9881_DCS_write_1A_1P(0xCE,0x4C);
ILI9881_DCS_write_1A_1P(0xCF,0x21);
ILI9881_DCS_write_1A_1P(0xD0,0x27);
ILI9881_DCS_write_1A_1P(0xD1,0x54);
ILI9881_DCS_write_1A_1P(0xD2,0x67);
ILI9881_DCS_write_1A_1P(0xD3,0x39);


data_array[0] = (0x00042902);
	data_array[1] = (0x018198FF);
	dsi_set_cmdq(data_array, 2, 1);
ILI9881_DCS_write_1A_1P(0x60,0x22);

        data_array[0] = (0x00042902);
	data_array[1] = (0x008198FF);
	dsi_set_cmdq(data_array, 2, 1);
//ILI9881_DCS_write_1A_1P(0x36,0x03);

        ILI9881_DCS_write_1A_0P(0x11);
        MDELAY(120);

        ILI9881_DCS_write_1A_0P(0x29);
        MDELAY(60);

}
#endif

LCM_DRIVER ili9881_dsi_6735_hd_drv =
{
    	.name		= "ili9881_6735_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
	//.ata_check		= lcm_ata_check,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
	.update		= lcm_update,
#endif
};
