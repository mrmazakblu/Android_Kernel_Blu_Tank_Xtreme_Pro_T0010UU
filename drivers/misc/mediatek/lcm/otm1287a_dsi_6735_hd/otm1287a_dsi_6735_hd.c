#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#if defined(TYD_PRO_F6_FAC)
#include <platform/mt_gpio.h>
#include <string.h>
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>

#define GPIO_65132_ENP (GPIO102 | 0x80000000)
#endif
#endif

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

#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#define GPIO_LDO28_EN (GPIO125 | 0x80000000)
#define GPIO_LDO18_EN (GPIO126 | 0x80000000)
#define GPIO_LCM_RST (GPIO146 | 0x80000000)
#endif
// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (720)
#define FRAME_HEIGHT (1280)
#define REGFLAG_DELAY          	0XFE
#define REGFLAG_END_OF_TABLE  	0xFC   // END OF REGISTERS MARKER


#define   LCM_ID_OTM1287A    0x1287
#define   LCM_DSI_CMD_MODE	0
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// ---------------------------------------------------------------------------
// Local Variables
// ---------------------------------------------------------------------------

//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5C_XDNZ)
int lcm_state_flag = 0;
#endif
//end

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
// Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


struct LCM_setting_table {
unsigned char cmd;
unsigned char count;
unsigned char para_list[64];
};

#if defined(DROI_PRO_FQ5C)
#include "otm1287a_dsi_6735_hd_fq5c.h"
#endif

#ifndef DROI_LCD_USE_CUSTOM_OTM1287A_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
//fac
	{0x00,1,{0x00}},
	{0xff,3,{0x12,0x87,0x01}},	//EXTC=1
	{0x00,1,{0x80}},	        //Orise mode enable
	{0xff,2,{0x12,0x87}},

	{0x00,1,{0x92}},
	{0xff,2,{0x30,0x02}},		//MIPI 30 4 Lane  20 3lane

//------------------- panel setting --------------------//
	{0x00,1,{0x80}},            //TCON Setting
	{0xc0,9,{0x00,0x64,0x00,0x10,0x10,0x00,0x64,0x10,0x10}},

	{0x00,1,{0x90}},            //Panel Timing Setting
	{0xc0,6,{0x00,0x4b,0x00,0x01,0x00,0x04}},

	{0x00,1,{0xb3}},            //Interval Scan Frame: 0 frame, column inversion
	{0xc0,2,{0x00,0x55}},

	{0x00,1,{0x81}},            //frame rate:60Hz
	{0xc1,1,{0x55}},

//------------------- power setting --------------------//
	{0x00,1,{0xa0}},            //dcdc setting
	{0xc4,14,{0x05,0x10,0x04,0x02,0x05,0x15,0x11,0x05,0x10,0x07,0x02,0x05,0x15,0x11}},

	{0x00,1,{0xb0}},            //clamp voltage setting
	{0xc4,2,{0x00,0x00}},

	{0x00,1,{0x91}},            //VGH=18V, VGL=-10V, pump ratio:VGH=8x, VGL=-5x
	{0xc5,2,{0xa6,0xd2}},

	{0x00,1,{0x00}},            //GVDD=5.008V, NGVDD=-5.008V
	{0xd8,2,{0xc7,0xc7}},

	{0x00,1,{0x00}},            //VCOM=-0.9V
	{0xd9,1,{0x71}},

	{0x00,1,{0xb3}},            //VDD_18V=1.7V, LVDSVDD=1.6V
	{0xc5,1,{0x84}},

	{0x00,1,{0xbb}},            //LVD voltage level setting
	{0xc5,1,{0x8a}},

	{0x00,1,{0x82}},            //chopper
	{0xc4,1,{0x0a}},

	{0x00,1,{0xc6}},		//debounce
	{0xB0,1,{0x03}},

//------------------- control setting --------------------//
	{0x00,1,{0x00}},            //ID1
	{0xd0,1,{0x40}},

	{0x00,1,{0x00}},            //ID2, ID3
	{0xd1,2,{0x00,0x00}},

//------------------- power on setting --------------------//
	{0x00,1,{0xb2}},            //VGLO1
	{0xf5,2,{0x00,0x00}},

	{0x00,1,{0xb6}},            //VGLO2
	{0xf5,2,{0x00,0x00}},

	{0x00,1,{0x94}},            //VCL pump dis
	{0xf5,2,{0x00,0x00}},

	{0x00,1,{0xd2}},            //VCL reg. en
	{0xf5,2,{0x06,0x15}},

	{0x00,1,{0xb4}},            //VGLO1/2 Pull low setting
	{0xc5,1,{0xcc}},	       //d[7] vglo1 d[6] vglo2 => 0: pull vss, 1: pull vgl

//------------------- for Power IC ---------------------------------
	{0x00,1,{0x90}},            //Mode-3
	{0xf5,4,{0x02,0x11,0x02,0x15}},

	{0x00,1,{0x90}},            //2xVPNL, 1.5*=00, 2*=50, 3*=a0
	{0xc5,1,{0x50}},

	{0x00,1,{0x94}},            //Frequency
	{0xc5,1,{0x66}},

//------------------- panel timing state control --------------------//
	{0x00,1,{0x80}},            //panel timing state control
	{0xcb,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x90}},            //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xa0}},            //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xb0}},            //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xc0}},            //panel timing state control
	{0xcb,15,{0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x05,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xd0}},            //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05}},

	{0x00,1,{0xe0}},            //panel timing state control
	{0xcb,14,{0x05,0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x00}},

	{0x00,1,{0xf0}},            //panel timing state control
	{0xcb,11,{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},

//------------------- panel pad mapping control --------------------//
	{0x00,1,{0x80}},            //panel pad mapping control
	{0xcc,15,{0x29,0x2a,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x06,0x00,0x08,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x90}},            //panel pad mapping control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x29,0x2a,0x09,0x0b,0x0d,0x0f,0x11,0x13}},

	{0x00,1,{0xa0}},            //panel pad mapping control
	{0xcc,14,{0x05,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00}},

  	{0x00,1,{0xb0}},            //panel pad mapping control
	{0xcc,15,{0x29,0x2a,0x13,0x11,0x0f,0x0d,0x0b,0x09,0x01,0x00,0x07,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xc0}},            //panel pad mapping control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x29,0x2a,0x14,0x12,0x10,0x0e,0x0c,0x0a}},

	{0x00,1,{0xd0}},            //panel pad mapping control
	{0xcc,14,{0x02,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00}},

//------------------- panel timing setting --------------------//
	{0x00,1,{0x80}},            //panel VST setting
	{0xce,12,{0x89,0x05,0x10,0x88,0x05,0x10,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x90}},            //panel VEND setting
	{0xce,14,{0x54,0xfc,0x10,0x54,0xfd,0x10,0x55,0x00,0x10,0x55,0x01,0x10,0x00,0x00}},

	{0x00,1,{0xa0}},            //panel CLKA1/2 setting
	{0xce,14,{0x58,0x07,0x04,0xfc,0x00,0x10,0x00,0x58,0x06,0x04,0xfd,0x00,0x10,0x00}},

	{0x00,1,{0xb0}},            //panel CLKA3/4 setting
	{0xce,14,{0x58,0x05,0x04,0xfe,0x00,0x10,0x00,0x58,0x04,0x04,0xff,0x00,0x10,0x00}},

	{0x00,1,{0xc0}},            //panel CLKb1/2 setting
	{0xce,14,{0x58,0x03,0x05,0x00,0x00,0x10,0x00,0x58,0x02,0x05,0x01,0x00,0x10,0x00}},

	{0x00,1,{0xd0}},            //panel CLKb3/4 setting
	{0xce,14,{0x58,0x01,0x05,0x02,0x00,0x10,0x00,0x58,0x00,0x05,0x03,0x00,0x10,0x00}},

	{0x00,1,{0x80}},            //panel CLKc1/2 setting
	{0xcf,14,{0x50,0x00,0x05,0x04,0x00,0x10,0x00,0x50,0x01,0x05,0x05,0x00,0x10,0x00}},

	{0x00,1,{0x90}},            //panel CLKc3/4 setting
	{0xcf,14,{0x50,0x02,0x05,0x06,0x00,0x10,0x00,0x50,0x03,0x05,0x07,0x00,0x10,0x00}},

	{0x00,1,{0xa0}},            //panel CLKd1/2 setting
	{0xcf,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xb0}},            //panel CLKd3/4 setting
	{0xcf,14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xc0}},            //panel ECLK setting
	{0xcf,11,{0x39,0x39,0x20,0x20,0x00,0x00,0x01,0x01,0x20,0x00,0x00}},

	{0x00,1,{0xb5}},
	{0xc5,6,{0x0b,0x95,0xff,0x0b,0x95,0xff}},

//------------------- Gamma --------------------//
	{0x00,1,{0x00}},			//G2.2+                                            ///////////////
	{0xE1,20,{0x10,0x2A,0x38,0x44,0x54,0x61,0x62,0x8A,0x78,0x8F,0x75,0x62,0x75,0x52,0x50,0x43,0x35,0x26,0x1F,0x08}},

	{0x00,1,{0x00}},		        //G2.2-
	{0xE2,20,{0x10,0x2A,0x38,0x44,0x54,0x61,0x62,0x8A,0x78,0x8F,0x75,0x62,0x75,0x52,0x50,0x43,0x35,0x26,0x1F,0x08}},


	{0x00,1,{0x00}},            //Orise mode disable
	{0xff,3,{0xff,0xff,0xff}},

{0x36,1,{0x00}},

{0x35,1,{0x00}},//TE

	{0x11, 1, {0x00}},
	{REGFLAG_DELAY,120,{}},
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY,20,{}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};
#endif


#if 0
static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence

        {0x00,1,{0x00}},
	{0xff,3,{0x12,0x87,0x01}},	//EXTC=1
	{0x00,1,{0x80}},	        //Orise mode enable
	{0xff,2,{0x12,0x87}},


        {0x00,1,{0xb3}},
	{0xc0,2,{0x00,0x11}},


	{0x00,1,{0x00}},            //Orise mode disable
	{0xff,3,{0xff,0xff,0xff}},


        {0x22, 1, {0x00}},

        {0x28, 1, {0x00}},
        {REGFLAG_DELAY, 20, {}},
    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
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
		}
	}
}

// ---------------------------------------------------------------------------
// LCM Driver Implementations
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
    params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;
    //params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

    #if (LCM_DSI_CMD_MODE)
    params->dsi.mode   = CMD_MODE;
    #else
    params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
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
    params->dsi.intermediat_buffer_num = 2;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
    //params->dsi.vertical_active_line=FRAME_HEIGHT;

#if defined(DROI_PRO_FQ5C_XDNZ)
	params->dsi.vertical_sync_active				= 4; //  4  0x3;// 3    2
	params->dsi.vertical_backporch					= 16;// 15 0x8;// 20   1
	params->dsi.vertical_frontporch					= 20; //16 0x8; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;//10 50  2
	params->dsi.horizontal_backporch				= 85;//42    //75
	params->dsi.horizontal_frontporch				= 85; //44
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK=200;
    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0xac;//0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x00;//0x9c;
    params->dsi.ssc_disable = 1;
#else
	params->dsi.vertical_sync_active				= 4; //  4  0x3;// 3    2
	params->dsi.vertical_backporch					= 16;// 15 0x8;// 20   1
	params->dsi.vertical_frontporch					= 15; //16 0x8; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;//10 50  2
	params->dsi.horizontal_backporch				= 64;//42
	params->dsi.horizontal_frontporch				= 64; //44
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=210;

    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0xac;//0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x00;//0x9c;
#endif

}

static void lcm_init(void)
{
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
    SET_RESET_PIN(1);
	MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);//Must over 6 ms,SPEC request

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{
	SET_RESET_PIN(0);
	MDELAY(130);
//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5C_XDNZ)
    lcm_state_flag = 1;
#endif
//end
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
//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5C_XDNZ)
    lcm_state_flag = 0;
#endif
//end
}

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0, id1 = 0;
	unsigned char buffer[5];
	unsigned int array[16];


	MDELAY(60);
	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(50);


	array[0] = 0x00053700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xA1, buffer, 5);
	id = buffer[2]; //we only need ID
	id1 = buffer[3];
	id = (id << 8) | id1;
    #ifdef BUILD_LK
		printf("%s, LK otm1287a debug: otm1287a id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel otm1287a horse debug: otm1287a id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_OTM1287A)
    	return 1;
    else
        return 0;
}

#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif

unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
	unsigned int id=0, id1 = 0;

	unsigned int array[2];
	unsigned char buff[5];


	printk("G_TEST 1");

	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	array[0] = 0x00053700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	atomic_set(&ESDCheck_byCPU,1);
	read_reg_v2(0xA1, buff, 5);
	atomic_set(&ESDCheck_byCPU,0);
	id = buff[2]; //we only need ID
	id1 = buff[3];
	id = (id << 8) | id1;
    #ifdef BUILD_LK
		printf("%s, LK otm1287a debug: otm1287a id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel otm1287a horse debug: otm1287a id = 0x%08x\n", __func__, id);
    #endif
    if(id == LCM_ID_OTM1287A)
    	return 1;
    else
        return 0;
#endif
}
// ---------------------------------------------------------------------------
// Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1287a_dsi_6735_hd_drv =
{
    	.name		= "OTM1287A_DSI_HD",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
   	.ata_check      = lcm_ata_check,
};

