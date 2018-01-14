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

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1440)

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

//static kal_bool IsFirstBoot = KAL_TRUE;
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
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[120];
};


#if defined(DROI_PRO_PU6T)
#include "jd9369_dsi_6735_hd_pu6t.h"
#endif

#ifndef TYD_LCD_USE_CUSTOM_JD9365_HD
static struct LCM_setting_table lcm_initialization_setting[] = {

{0x11, 0, {0x00}},
{REGFLAG_DELAY,120,{}},

{0x29, 0, {0x00}},
{REGFLAG_DELAY,5,{}},

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

	    //params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	    //params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
		params->dsi.ssc_disable = 1;

	#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
	#else
	    params->dsi.mode   = SYNC_PULSE_VDO_MODE;  //; BURST_VDO_MODE
	#endif
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	    // Video mode setting
	    params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	    params->dsi.vertical_active_line=FRAME_HEIGHT;
	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 12;//20;//50
	params->dsi.vertical_frontporch					= 20;//20;//20
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 20;//42;
	params->dsi.horizontal_frontporch				= 90;//42;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=232;


	params->dsi.noncont_clock=1;
	params->dsi.noncont_clock_period=1;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x1C;


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
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(80);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
        MDELAY(1);
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	SET_RESET_PIN(1);
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
#define LCM_ID_JD9369		(0x9369)

  unsigned int array[4];
  unsigned short device_id;
  unsigned char buffer[2];

  SET_RESET_PIN(1);
  MDELAY(10);

  SET_RESET_PIN(0);
  MDELAY(10);

  SET_RESET_PIN(1);
  MDELAY(100);

	array[0]=0x00043902;
	array[1]=0xFC6993DF;// page enable
	dsi_set_cmdq(array, 2, 1);
	MDELAY(10);


    //*************Enable CMD2 Page1  *******************//

  array[0] = 0x00033700;// read id return two byte,version and id
  dsi_set_cmdq(array, 1, 1);
    MDELAY(10);

  read_reg_v2(0x04, buffer, 2);
  device_id = buffer[0]<<8|buffer[1];




  return (((LCM_ID_JD9369 == device_id) || 0x8555 == device_id) ? 1 : 0);
}


LCM_DRIVER jd9369_dsi_6735_hd_drv =
{
    .name			= "JD9369_dsi_6735_hd",
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

