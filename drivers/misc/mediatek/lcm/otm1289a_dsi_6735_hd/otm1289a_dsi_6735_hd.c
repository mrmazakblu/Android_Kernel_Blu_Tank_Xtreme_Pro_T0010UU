#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/upmu_hw.h>
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#else
#include <mt-plat/upmu_common.h>
#include <mach/upmu_hw.h>
#include <mach/gpio_const.h>
#include <mt-plat/mt_gpio.h>
#endif

// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (720)
#define FRAME_HEIGHT (1280)
#define REGFLAG_DELAY          	0XFE
#define REGFLAG_END_OF_TABLE  	0xFC   // END OF REGISTERS MARKER


#define   LCM_ID_OTM1289A    0x1289
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


#if defined(TYD_PRO_V9_FAC_INCELL)
#define LCM_LDO_POWER_PIN	(GPIO123 | 0x80000000)
#define GPIO_LCM_RST            (GPIO146 | 0x80000000)
#endif
#if defined(DROI_PRO_FQ5CW_XDNZ)||defined(DROI_PRO_FQ5C_ZG)||defined(DROI_PRO_FQ5C_XDNZ)
int lcm_state_flag = 0;
#endif

struct LCM_setting_table {
unsigned char cmd;
unsigned char count;
unsigned char para_list[64];
};

#if defined(DROI_PRO_FQ5C)
#include "otm1289a_dsi_6735_hd_fq5c.h"
#endif

#if defined(DROI_PRO_FQ5CW)
#include "otm1289a_dsi_6735_hd_fq5cw.h"
#endif

#if defined(DROI_PRO_Q1)
#include "otm1289a_dsi_6735_hd_q1.h"
#endif

#ifndef DROI_LCD_USE_CUSTOM_OTM1289A_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x00,1,{0x00}},		//EXTC=1
	{0xff,3,{0x12,0x89,0x01}},
	{0x00,1,{0x80}},		//CMD2 enable
	{0xff,2,{0x12,0x89}},
	{0x00,1,{0x90}},         	//MIPI 4:0xb0, 3:0xa0, 2:0x90
	{0xff,1,{0xb0}},

	//-------------------- panel setting --------------------------------//
	{0x00,1,{0x80}},             //TCON Setting
	{0xc0,8,{0x4a,0x00,0x10,0x10,0x96,0x01,0x68,0x40}},

	{0x00,1,{0x90}},             //Panel Timing Setting
	{0xc0,3,{0x3b,0x01,0x09}},

	{0x00,1,{0x8c}},             //column inversion
	{0xc0,1,{0x00}},
	{0x00,1,{0x80}},             //frame rate:60Hz
	{0xc1,1,{0x33}},

	//-------------------- power setting --------------------------------//
	{0x00,1,{0x85}},             //VGH=8x, VGL=-5x, VGH=18V, VGL=-10V
	{0xc5,4,{0x09,0x0A,0x39,0x10}},

	{0x00,1,{0x00}},             //GVDD=5.00V, NGVDD=-5.00V
	{0xd8,2,{0x2b,0x2b}},

	{0x00,1,{0x00}},             //VCOM=-1.0V
	{0xd9,2,{0x00,0x43}},

	{0x00,1,{0x84}},             //chopper
	{0xC4,1,{0x02}},
	{0x00,1,{0x93}},             //pump option
	{0xC4,1,{0x04}},
	{0x00,1,{0x96}},  		//VCL regulator
	{0xF5,1,{0xE7}},
	{0x00,1,{0xA0}},    		//pump3 off
	{0xF5,1,{0x4A}},
	{0x00,1,{0x8a}},             //blank frame
	{0xc0,1,{0x11}},
	{0x00,1,{0x83}},             //vcom active
	{0xF5,1,{0x81}},

	//-------------------- for Power IC ----------------------------------//
	{0x00,1,{0x90}},             //2xVPNL, x1.5=01, x2=05, x3=09
	{0xc4,2,{0x96,0x05}},

	//-------------------- panel timing state control --------------------//
	{0x00,1,{0x80}},             //panel timing state control
	{0xcb,15,{0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x00,0x14,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x90}},             //panel timing state control
	{0xcb,7,{0x00,0x00,0x00,0x00,0x14,0x00,0x00}},

	//-------------------- panel pad mapping control ---------------------//
	{0x00,1,{0x80}},             //panel timing state control
	{0xcc,14,{0x19,0x1a,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x06,0x08,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x90}},             //panel timing state control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x19,0x1a,0x09,0x0b,0x0d,0x0f,0x11,0x13}},

	{0x00,1,{0xa0}},             //panel timing state control
	{0xcc,13,{0x05,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00}},

	{0x00,1,{0xb0}},             //panel timing state control
	{0xcc,14,{0x19,0x1a,0x13,0x11,0x0f,0x0d,0x0b,0x09,0x01,0x07,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xc0}},             //panel timing state control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x19,0x1a,0x14,0x12,0x10,0x0e,0x0c,0x0a}},

	{0x00,1,{0xd0}},             //panel timing state control
	{0xcc,13,{0x02,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00}},

	//-------------------- panel timing setting --------------------------//
	{0x00,1,{0x80}},             //panel VST setting
	{0xce,6,{0x89,0x05,0x00,0x88,0x00,0x00}},

	{0x00,1,{0x90}},             //panel VEND setting
	{0xce,9,{0x54,0xfc,0x00,0x04,0xfd,0x05,0x00,0x05,0x01}},

	{0x00,1,{0xa0}},             //panel CLKA setting
	{0xce,15,{0x50,0x85,0x09,0x00,0x00,0x00,0x84,0x0a,0x00,0x83,0x0b,0x00,0x82,0x0c,0x00}},

	{0x00,1,{0xb0}},             //panel CLKB setting
	{0xce,15,{0x50,0x81,0x0d,0x00,0x00,0x00,0x80,0x0e,0x00,0x00,0x0f,0x00,0x01,0x10,0x00}},

	{0x00,1,{0xc0}},             //panel CLKC setting
	{0xce,15,{0x50,0x02,0x11,0x00,0x00,0x00,0x03,0x12,0x00,0x04,0x13,0x00,0x05,0x14,0x00}},

	{0x00,1,{0xd0}},             //panel CLKD setting
	{0xce,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xf0}},             //panel ECLK setting
	{0xce,6,{0x39,0x20,0x01,0x01,0x20,0x00}},

	//-------------------- gamma -----------------------------------------//
	{0x00,1,{0x00}},///////////////////////////////////////////////////
	{0xE1,16,{0x18,0x37,0x40,0x53,0x5E,0x79,0x74,0x88,0x70,0x5F,0x6B,0x54,0x42,0x2E,0x21,0x0E}},

	{0x00,1,{0x00}},
	{0xE2,16,{0x18,0x37,0x40,0x53,0x5E,0x79,0x74,0x88,0x70,0x5F,0x6B,0x54,0x42,0x2E,0x21,0x0E}},


	{0x00,1,{0x00}},             //CMD2 disable
	{0xff,3,{0xff,0xff,0xff}},
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

#if defined(DROI_PRO_Q1_JR)
	params->dsi.vertical_sync_active				= 4; //  4  0x3;// 3    2
	params->dsi.vertical_backporch					= 12;// 15 0x8;// 20   1
	params->dsi.vertical_frontporch					= 15; //16 0x8; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 20;//10 50  2
	params->dsi.horizontal_backporch				= 70;//42
	params->dsi.horizontal_frontporch				= 70; //44
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=210;
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
#endif

}

static void lcm_init(void)
{
#if defined(TYD_PRO_V9_FAC_INCELL)
    #ifdef BUILD_LK
	pmic_set_register_value(PMIC_RG_VGP1_EN,6);
    #else
    if(TRUE != hwPowerOn(MT6328_POWER_LDO_VGP1,VOL_2800,"LCM"))
    {
	    printk("incell vgp1 poweron failed\n");

    }
    #endif
	mt_set_gpio_mode(LCM_LDO_POWER_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(LCM_LDO_POWER_PIN, GPIO_DIR_OUT);

	//mt_set_gpio_mode(GPIO_LCM_RST, GPIO_LCM_RST_M_GPIO);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);

	mt_set_gpio_out(LCM_LDO_POWER_PIN, GPIO_OUT_ONE);

	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(15);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(6);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(40);
#else
    SET_RESET_PIN(1);
	MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);//Must over 6 ms,SPEC request
#endif
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{
#if defined(TYD_PRO_V9_FAC_INCELL)
    #ifdef BUILD_LK
	pmic_set_register_value(PMIC_RG_VGP1_EN,0);
     #else
    if(TRUE != hwPowerDown(MT6328_POWER_LDO_VGP1,"LCM"))
    {
	    printk("incell vgp1 poweron failed\n");
    }
    #endif
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(2);

	mt_set_gpio_out(LCM_LDO_POWER_PIN, GPIO_OUT_ZERO);
#elif defined(TYD_PRO_X1_LJ)||defined(TYD_PRO_X1_DAQ)
	unsigned int data_array[16];
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	SET_RESET_PIN(0);
#else
	//unsigned int data_array[16];


	SET_RESET_PIN(0);
	MDELAY(130);

#endif
//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5CW_XDNZ)||defined(DROI_PRO_FQ5C_ZG)||defined(DROI_PRO_FQ5C_XDNZ)
    lcm_state_flag = 1;
#endif
//end
}

static void lcm_resume(void)
{
	lcm_init();
//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5CW_XDNZ)||defined(DROI_PRO_FQ5C_ZG)||defined(DROI_PRO_FQ5C_XDNZ)
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
		printf("%s, LK otm1289a debug: otm1289a id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel otm1289a horse debug: otm1289a id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_OTM1289A)
    	return 1;
    else
        return 0;
}

#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif

static unsigned int lcm_ata_check(unsigned char *buffer)
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
		printf("%s, LK otm1289a debug: otm1289a id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel otm1289a horse debug: otm1289a id = 0x%08x\n", __func__, id);
    #endif
    if(id == LCM_ID_OTM1289A)
    	return 1;
    else
        return 0;
#endif
}
// ---------------------------------------------------------------------------
// Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1289a_dsi_6735_hd_drv =
{
    	.name		= "OTM1289A_DSI_HD",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
   	.ata_check      = lcm_ata_check,
};

