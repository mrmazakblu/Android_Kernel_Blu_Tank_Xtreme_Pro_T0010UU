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
//  Local Constants
// ---------------------------------------------------------------------------
#if defined(DROI_PRO_PU6T)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1440)
#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#endif

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif

#ifndef FALSE
    #define   FALSE    0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))
#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#define GPIO_LDO28_EN (GPIO125 | 0x80000000)
#define GPIO_LDO18_EN (GPIO126 | 0x80000000)
#define GPIO_LCM_RST (GPIO146 | 0x80000000)
#endif
// ---------------------------------------------------------------------------
//  Local Functions
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

#if defined(TYD_PRO_W6)
#include "jd9365_dsi_6735_hd_w6.h"
#endif

#if defined(DROI_PRO_F5C)
#include "jd9365_dsi_6735_hd_f5c.h"
#endif

#if defined(TYD_PRO_WF5)
#include "jd9365_dsi_6735_hd_w5.h"
#endif

#if defined(DROI_PRO_FQ5CW)
#include "jd9365_dsi_6735_hd_fq5cw.h"
#endif

#if defined(DROI_PRO_Q1)
#include "jd9365_dsi_6735_hd_q1.h"
#endif

#if defined(DROI_PRO_F6)
#include "jd9365_dsi_6735_hd_f6.h"
#endif

#if defined(DROI_PRO_PU6)
#include "jd9365_dsi_6735_hd_pu6.h"
#endif

#if defined(DROI_PRO_PU6T)
#include "jd9365_dsi_6735_hd_pu6t.h"
#endif
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);


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

#if defined(TYD_PRO_E5C_TYI3)||defined(TYD_PRO_E5C_TYI4)
	if((val>600)&&(val<1000)) 	    //voltage=0.8V  HaoShiTong
	{
	    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

	}
#elif defined(DROI_PRO_F5C_GB)
	if((val>1000))            //voltage=1.2V
	{
	    push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>200)&&(val<600))      //voltage=0.4V
	{
	    push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

	}
#elif defined(DROI_PRO_F5C_LEAGOO2)
	if((val>1000))            //voltage=1.2V
	{
	    push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>200)&&(val<600))      //voltage=0.4V
	{
	    push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

	}
#elif defined(TYD_PRO_E6C_TYI2)
{
	    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
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

#ifndef TYD_LCD_USE_CUSTOM_JD9365_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
//JD9365 AUO5.0
{0xE1,1,{0x93}},
{0xE2,1,{0x65}},
{0xE3,1,{0xF8}},

{0x70,1,{0x10}},
{0x71,1,{0x13}},
{0x72,1,{0x06}},

{0x80,1,{0x03}},// 4lanes

{0xE0,1,{0x01}},

{0x00,1,{0x00}},
{0x01,1,{0x90}},
{0x03,1,{0x00}},
{0x04,1,{0x9E}},

{0x0C,1,{0x64}},
{0x17,1,{0x00}},
{0x18,1,{0xC7}},
{0x19,1,{0x01}},
{0x1A,1,{0x00}},
{0x1B,1,{0xC7}},
{0x1C,1,{0x01}},

//Set Gate Power
{0x1F,1,{0x3F}},
{0x20,1,{0x2E}},
{0x21,1,{0x2E}},
{0x22,1,{0x4E}},

//SET RGBCYC
{0x37,1,{0x29}},

//SET RGBCYC
{0x38,1,{0x05}},
{0x39,1,{0x08}},
{0x3A,1,{0x12}},
{0x3D,1,{0xFF}},
{0x3E,1,{0xFF}},
{0x3F,1,{0x7F}},

//Set TCON
{0x40,1,{0x04}},
{0x41,1,{0xA0}},
{0x43,1,{0x0F}},
{0x44,1,{0x0D}},
{0x45,1,{0x28}},


//--- power voltage  ----//
{0x55,1,{0x01}},
{0x56,1,{0x01}},
{0x57,1,{0x69}},
{0x58,1,{0x0A}},
{0x59,1,{0x0A}},
{0x5A,1,{0x29}},
{0x5B,1,{0x1A}},

//--- Gamma  ----//     2.5
{0x5D,1,{0x73}},//0x73
{0x5E,1,{0x59}},//0x57
{0x5F,1,{0x49}},//0x47
{0x60,1,{0x3D}},//0x3A
{0x61,1,{0x3A}},//0x36
{0x62,1,{0x2C}},//0x26
{0x63,1,{0x31}},//0x2A
{0x64,1,{0x1B}},//0x14
{0x65,1,{0x32}},//0x2A
{0x66,1,{0x2F}},//0x27
{0x67,1,{0x2D}},//0x25
{0x68,1,{0x49}},//0x3F
{0x69,1,{0x34}},//0x2C
{0x6A,1,{0x3B}},//0x33
{0x6B,1,{0x30}},//0x28
{0x6C,1,{0x2C}},//0x28
{0x6D,1,{0x20}},//0x1E
{0x6E,1,{0x11}},//0x11
{0x6F,1,{0x02}},//0x02
{0x70,1,{0x73}},//0x73
{0x71,1,{0x59}},//0x57
{0x72,1,{0x49}},//0x47
{0x73,1,{0x3D}},//0x3A
{0x74,1,{0x3A}},//0x36
{0x75,1,{0x2C}},//0x26
{0x76,1,{0x31}},//0x2A
{0x77,1,{0x1B}},//0x14
{0x78,1,{0x32}},//0x2A
{0x79,1,{0x2F}},//0x27
{0x7A,1,{0x2D}},//0x25
{0x7B,1,{0x49}},//0x3F
{0x7C,1,{0x34}},//0x2C
{0x7D,1,{0x3B}},//0x33
{0x7E,1,{0x30}},//0x28
{0x7F,1,{0x2C}},//0x28
{0x80,1,{0x20}},//0x1E
{0x81,1,{0x11}},//0x11
{0x82,1,{0x02}},//0x02

{0xE0,1,{0x02}},

{0x00,1,{0x1F}},
{0x01,1,{0x1F}},
{0x02,1,{0x1F}},
{0x03,1,{0x1F}},
{0x04,1,{0x1F}},
{0x05,1,{0x1F}},
{0x06,1,{0x1F}},
{0x07,1,{0x11}},
{0x08,1,{0x13}},
{0x09,1,{0x1F}},
{0x0A,1,{0x0D}},
{0x0B,1,{0x1A}},
{0x0C,1,{0x0F}},
{0x0D,1,{0x1C}},
{0x0E,1,{0x09}},
{0x0F,1,{0x05}},
{0x10,1,{0x0B}},
{0x11,1,{0x07}},
{0x12,1,{0x01}},
{0x13,1,{0x03}},
{0x14,1,{0x1F}},
{0x15,1,{0x1F}},

{0x16,1,{0x1F}},
{0x17,1,{0x1F}},
{0x18,1,{0x1F}},
{0x19,1,{0x1F}},
{0x1A,1,{0x1F}},
{0x1B,1,{0x1F}},
{0x1C,1,{0x1F}},
{0x1D,1,{0x10}},
{0x1E,1,{0x12}},
{0x1F,1,{0x1F}},
{0x20,1,{0x0C}},
{0x21,1,{0x19}},
{0x22,1,{0x0E}},
{0x23,1,{0x1B}},
{0x24,1,{0x08}},
{0x25,1,{0x04}},
{0x26,1,{0x0A}},
{0x27,1,{0x06}},
{0x28,1,{0x00}},
{0x29,1,{0x02}},
{0x2A,1,{0x1F}},
{0x2B,1,{0x1F}},


//GIP_L_GS Pin mapping
{0x2C,1,{0x1F}},
{0x2D,1,{0x1F}},
{0x2E,1,{0x1F}},
{0x2F,1,{0x1F}},
{0x30,1,{0x1F}},
{0x31,1,{0x1F}},
{0x32,1,{0x1F}},
{0x33,1,{0x02}},
{0x34,1,{0x00}},
{0x35,1,{0x1F}},
{0x36,1,{0x0E}},
{0x37,1,{0x1B}},
{0x38,1,{0x0C}},
{0x39,1,{0x19}},
{0x3A,1,{0x06}},
{0x3B,1,{0x0A}},
{0x3C,1,{0x04}},
{0x3D,1,{0x08}},
{0x3E,1,{0x12}},
{0x3F,1,{0x10}},
{0x40,1,{0x1F}},
{0x41,1,{0x1F}},

//GIP_R_GS Pin mapping
{0x42,1,{0x1F}},
{0x43,1,{0x1F}},
{0x44,1,{0x1F}},
{0x45,1,{0x1F}},
{0x46,1,{0x1F}},
{0x47,1,{0x1F}},
{0x48,1,{0x1F}},
{0x49,1,{0x03}},
{0x4A,1,{0x01}},
{0x4B,1,{0x1F}},
{0x4C,1,{0x0F}},
{0x4D,1,{0x1C}},
{0x4E,1,{0x0D}},
{0x4F,1,{0x1A}},
{0x50,1,{0x07}},
{0x51,1,{0x0B}},
{0x52,1,{0x05}},
{0x53,1,{0x09}},
{0x54,1,{0x13}},
{0x55,1,{0x11}},
{0x56,1,{0x1F}},
{0x57,1,{0x1F}},

//GIP Timing
{0x58,1,{0x40}},
{0x59,1,{0x00}},
{0x5A,1,{0x00}},
{0x5B,1,{0x30}},
{0x5C,1,{0x09}},
{0x5D,1,{0x30}},
{0x5E,1,{0x01}},
{0x5F,1,{0x02}},
{0x60,1,{0x30}},
{0x61,1,{0x01}},
{0x62,1,{0x02}},
{0x63,1,{0x1A}},
{0x64,1,{0x60}},
{0x65,1,{0x75}},
{0x66,1,{0x0B}},
{0x67,1,{0x73}},
{0x68,1,{0x09}},
{0x69,1,{0x1A}},
{0x6A,1,{0x66}},
{0x6B,1,{0x00}},
{0x6C,1,{0x00}},
{0x6D,1,{0x04}},
{0x6E,1,{0x04}},
{0x6F,1,{0x8F}},
{0x70,1,{0x74}},
{0x71,1,{0x09}},
{0x72,1,{0x0F}},
{0x73,1,{0x1A}},
{0x74,1,{0x00}},
{0x75,1,{0xBC}},
{0x76,1,{0x00}},
{0x77,1,{0x05}},
{0x78,1,{0x2E}},
{0x79,1,{0x00}},
{0x7A,1,{0x00}},
{0x7B,1,{0x00}},
{0x7C,1,{0x00}},
{0x7D,1,{0x03}},
{0x7E,1,{0x7B}},

//Page4
{0xE0,1,{0x04}},
{0x2B,1,{0x2B}},
{0x2E,1,{0x44}},
{0x2D,1,{0x03}},//lansel

//Page0
{0xE0,1,{0x00}},
{0xE6,1,{0x02}},
{0xE7,1,{0x02}},

{0x35, 1, {0x00}},
{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {}},
// Display ON
{0x29, 1, {0x00}},
{REGFLAG_DELAY, 5, {}},

{REGFLAG_END_OF_TABLE, 0x00, {}},
};
#endif

/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    	{REGFLAG_DELAY, 200, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
#if defined(DROI_PRO_F5C_LEAGOO2)
{REGFLAG_DELAY, 5, {}},
#endif
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},

    	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


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
	    //params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	    //params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
		params->dsi.ssc_disable = 1;
#if defined(DROI_PRO_F5C_NW) || defined(DROI_PRO_F5C_ZGW2)
     	params->dsi.esd_check_enable = 1;
  	    params->dsi.customization_esd_check_enable = 1;
  	    params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	    params->dsi.lcm_esd_check_table[0].count = 1;
	    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F5C_LEAGOO2)
	params->dsi.noncont_clock=1;
	params->dsi.noncont_clock_period=1;
    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;
    params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
    params->dsi.lcm_esd_check_table[0].count = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;

#endif

	#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
	#else
	    params->dsi.mode   = SYNC_PULSE_VDO_MODE;  //; BURST_VDO_MODE
	#endif
	    // DSI
	    /* Command mode setting */
		#if defined(DROI_PRO_F5C_NW) || defined(DROI_PRO_F5C_SGDZ) || defined(DROI_PRO_F5C_BPZN) || defined(DROI_PRO_F5C_BPZN5) || defined(DROI_PRO_Q1_LG)||defined(DROI_PRO_F5C_ZGW_A1)||defined(DROI_PRO_F5C_BPZN2)
        params->dsi.LANE_NUM				= LCM_THREE_LANE;//LCM_TWO_LANE;//LCM_THREE_LANE;
        #else
        params->dsi.LANE_NUM				= LCM_FOUR_LANE;//LCM_TWO_LANE;//LCM_THREE_LANE;
	#endif

	    //The following defined the fomat for data coming from LCD engine.
	    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	    params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	    params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	    params->dsi.data_format.format	  = LCM_DSI_FORMAT_RGB888;

	    // Video mode setting
	    params->dsi.intermediat_buffer_num = 0;

	    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	    params->dsi.vertical_active_line=FRAME_HEIGHT;
#if defined(DROI_PRO_F5C_ZGW2) || defined(DROI_PRO_F5C_ZGW2_A1_HD)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 6;
	    params->dsi.vertical_frontporch				= 16;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 24;
	    params->dsi.horizontal_backporch				= 80;
	    params->dsi.horizontal_frontporch				= 80;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#elif defined(DROI_PRO_F5C_BPZN2)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 9;
	    params->dsi.vertical_frontporch				= 30;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 20;
	    params->dsi.horizontal_backporch				= 50;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 267;
#elif defined(DROI_PRO_F5C_GZ)
	params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 11;
	    params->dsi.vertical_frontporch				= 8;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 30;
	    params->dsi.horizontal_backporch				= 65;
	    params->dsi.horizontal_frontporch				= 30;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

#elif defined(DROI_PRO_F5C_BPZN) || defined(DROI_PRO_F5C_BPZN5)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 9;
	    params->dsi.vertical_frontporch				= 30;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 20;
	    params->dsi.horizontal_backporch				= 50;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 270;
#elif defined(DROI_PRO_F5C_ZGW_A1)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 12;
	    params->dsi.vertical_frontporch				= 18;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 30;
	    params->dsi.horizontal_backporch				= 40;
	    params->dsi.horizontal_frontporch				= 60;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
            params->dsi.PLL_CLOCK = 260;
#elif defined(DROI_PRO_F5C_NW)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 12;
	    params->dsi.vertical_frontporch				= 24;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 40;
	    params->dsi.horizontal_backporch				= 40;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#elif defined(DROI_PRO_F5C_SGDZ)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 17;
	    params->dsi.vertical_frontporch				= 16;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 30;
	    params->dsi.horizontal_backporch				= 30;
	    params->dsi.horizontal_frontporch				= 30;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 200;
#elif defined(TYD_PRO_FQ5C_ZG)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 12;
	    params->dsi.vertical_frontporch				= 30;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 50;
	    params->dsi.horizontal_backporch				= 50;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

            params->dsi.PLL_CLOCK = 230;
#elif defined(DROI_PRO_F6_JF)
		params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch					= 10;
	    params->dsi.vertical_frontporch					= 30;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 24;
	    params->dsi.horizontal_backporch				= 48;
	    params->dsi.horizontal_frontporch				= 40;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#elif defined(DROI_PRO_Q1_LG)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 12;
	    params->dsi.vertical_frontporch				= 30;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 24;
	    params->dsi.horizontal_backporch				= 64;
	    params->dsi.horizontal_frontporch				= 56;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#elif defined(DROI_PRO_F5C_LEAGOO2)

	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 12;
	    params->dsi.vertical_frontporch				= 24;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 40;
	    params->dsi.horizontal_backporch				= 40;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#elif defined(DROI_PRO_PU6_JF)|| defined(DROI_PRO_PU6T_GB)||defined(DROI_PRO_PU6_JF2)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 6;
	    params->dsi.vertical_frontporch				= 16;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 50;
	    params->dsi.horizontal_backporch				= 50;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#elif defined(DROI_PRO_PU6T_JF)
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 12;
	    params->dsi.vertical_frontporch				= 20;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 50;
	    params->dsi.horizontal_backporch				= 50;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#else
	    params->dsi.vertical_sync_active				= 4;
	    params->dsi.vertical_backporch				= 12;
	    params->dsi.vertical_frontporch				= 24;
	    params->dsi.vertical_active_line				= FRAME_HEIGHT;

	    params->dsi.horizontal_sync_active				= 40;
	    params->dsi.horizontal_backporch				= 40;
	    params->dsi.horizontal_frontporch				= 50;
	    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
#endif
#if defined(DROI_PRO_F5C_GB)
        params->dsi.PLL_CLOCK = 200;
#elif defined(DROI_PRO_F6_JF)
        params->dsi.PLL_CLOCK = 198;
#elif defined(DROI_PRO_Q1_LG)
        params->dsi.PLL_CLOCK = 275;
#elif defined(DROI_PRO_F5C_LEAGOO2)
        params->dsi.PLL_CLOCK = 200;
#elif defined(DROI_PRO_PU6_JF)|| defined(DROI_PRO_PU6T_GB)||defined(DROI_PRO_PU6_JF2)
        params->dsi.PLL_CLOCK = 212;
#elif defined(DROI_PRO_PU6T_JF)
		params->dsi.PLL_CLOCK = 231;
#else
        params->dsi.PLL_CLOCK = 220;
#endif

}

static void lcm_init(void)
{
#if defined(SUPORT_ADC_CHECK)
	int val = 0;
#endif
#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
	mt_set_gpio_mode(GPIO_LDO28_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO28_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO28_EN, GPIO_OUT_ONE);
	mt_set_gpio_mode(GPIO_LDO18_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO18_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO18_EN, GPIO_OUT_ONE);
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
#endif
#if defined(DROI_PRO_F6_JF)
    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
#else
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(30);
    SET_RESET_PIN(1);
    MDELAY(120);
#endif
#if defined(SUPORT_ADC_CHECK)
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

#if defined(TYD_PRO_E5C_TYI3)||defined(TYD_PRO_E5C_TYI4)
	MDELAY(1);
#else

#endif
        MDELAY(1);
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	SET_RESET_PIN(1);
#if defined(DROI_PRO_F5C_GZ)
    MDELAY(10);
#endif
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(50);
#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
	MDELAY(120);
	mt_set_gpio_mode(GPIO_LDO28_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO28_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO28_EN, GPIO_OUT_ZERO);
	mt_set_gpio_mode(GPIO_LDO18_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO18_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO18_EN, GPIO_OUT_ZERO);
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_mode(GPIO_LDO18_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LDO18_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LDO18_EN, GPIO_OUT_ZERO);
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
#endif
}

static void lcm_resume(void)
{
	lcm_init();
}

static unsigned int lcm_compare_id(void)
{
    int array[4];
    char buffer[5];
    char id_high=0;
    char id_low=0;
    int id=0;

    SET_RESET_PIN(1);
    MDELAY(10);
	  SET_RESET_PIN(0);
    MDELAY(20);
	  SET_RESET_PIN(1);
    MDELAY(50);

    array[0] = 0x00023700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0x04,buffer, 2);

    id_high = buffer[0];
    id_low = buffer[1];
    id = (id_high<<8)|id_low;

    #ifdef BUILD_LK
    printf("luke: JD9365 %s %d, id = 0x%08x\n", __func__,__LINE__, id);
    #else

    printk("luke: JD9365 %s %d, id = 0x%08x\n", __func__,__LINE__, id);
    #endif

 /*
       if (LCM_ID_OTM1283A == id) {
#if COMPARE_BY_ADC
		int data[4] = {0,0,0,0};
		int res = 0;
		int rawdata = 0;
		int lcm_vol = 0;

		res = IMM_GetOneChannelValue(AUXADC_LCM_VOLTAGE_CHANNEL, data, &rawdata);
		if(res < 0) {
		#ifdef BUILD_LK
			printf("[adc_uboot  OTM1283A ruix]: get data error\n");
		#endif
			return 0;
		}

		lcm_vol = data[0] * 1000 + data[1] * 10;
//				printf("lcm_vol = : %d\n",lcm_vol);

		if(lcm_vol >= MIN_VOLTAGE && lcm_vol <= MAX_VOLTAGE) {
			return 1;
		}else{
			return 0;

		}
#endif
		return 1;
       	}else{
		return 0;
       } */

       return (id == 0x9365) ?1:0;
}


LCM_DRIVER jd9365_dsi_6735_hd_drv =
{
    .name			= "JD9365_dsi_6735_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif

};

