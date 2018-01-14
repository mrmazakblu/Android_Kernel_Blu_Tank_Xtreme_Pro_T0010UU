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


#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define GPIO_65132_ENP (GPIO81 | 0x80000000)
#define GPIO_65132_ENN (GPIO86 | 0x80000000)
const static unsigned char LCD_MODULE_ID = 0x09;//ID0->1;ID1->X
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0
#define FRAME_WIDTH  (1080)
#define FRAME_HEIGHT (1920)
#define LCM_ID_OTM1191A (0x91)


#define REGFLAG_DELAY             								0xFD
#define REGFLAG_END_OF_TABLE      							0xFB   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#if defined(DROI_PRO_FQ5B_XDNZ)
 int lcm_state_flag = 0;
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

const static unsigned int BL_MIN_LEVEL =20;
static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[128];
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);

#ifndef TYD_LCD_USE_CUSTOM_OTM1191A_FHD
static struct LCM_setting_table lcm_initialization_setting[] = {
        {0x00,1,{0x00}},
	{0xFF,3,{0x19,0x11,0x01}},

	{0x00,1,{0x80}},
	{0xFF,2,{0x19,0x11}},

	{0x00,1,{0x80}},
	{0xC0,7,{0x5D,0x00,0x05,0x06,0x5D,0x04,0x00}},

	{0x00,1,{0x8A}},
	{0xC0,1,{0x04}},

	{0x00,1,{0x92}},
{0xB3,  2 ,{0x08,0x04}},

	{0x00,1,{0x8B}},
{0xC0,  1 ,{0x00}},

	{0x00,1,{0x80}},
	{0xC4,2,{0x01,0x02}},

	//ckh
	{0x00,1,{0x94}},
{0xC0,  7 ,{0x01,0x03,0x06,0x03,0x03,0x12,0x03}},

	{0x00,1,{0x80}},
	{0xC1,1,{0x0A}},

	{0x00,1,{0x81}},
	{0xC1,3,{0x30,0x02,0xB0}},

	{0x00,1,{0x91}},
	{0xC1,1,{0x0F}},

	///////////////////////////////////////////////////////////////////////////////////////
	{0x00,1,{0x82}},
{0xC0,  2 ,{0x08,0x10}},

	//vst
	{0x00,1,{0x80}},
{0xC2,  4 ,{0x81,0x01,0x35,0xAB}},

	{0x00,1,{0x84}},
{0xC2,  2 ,{0x00,0x00}},

	//ckv
{0x00, 1 , {0xB0}},
{0xC2, 14 ,{0x82,0x02,0x00,0x08,0x00,0x82,0x02,0x00,0x81,0x02,0x00,0x81,0x02,0x00}},

{0x00, 1 , {0xC0}},
{0xC2, 14 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00, 1 , {0xD0}},
{0xC2,  5 ,{0x11,0x11,0x00,0x00,0xF0}},

	//vend
{0x00, 1 , {0xE0}},
{0xC2,  6 ,{0x01,0x00,0x08,0x08,0x00,0x00}},

	//rst
{0x00, 1 , {0xF0}},
{0xC2,  5 ,{0x80,0x02,0x01,0x34,0xAB}},

	//cic
	{0x00, 1,{0x90}},
{0xC2, 11 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	//pch
	{0x00,1,{0x80}},
{0xC3,  8 ,{0x22,0x22,0x22,0x22,0x00,0x00,0x22,0x00}},

	{0x00,1,{0x90}},
{0xC3,  4 ,{0x20,0x00,0x02,0x00}},

	//pwron/pwrof/lvd
	{0x00,1,{0x90}},
{0xCB,  3 ,{0x00,0x00,0x00}},

{0x00, 1 , {0xC0}},
{0xCB, 12 ,{0x01,0x04,0x04,0xF4,0x00,0x00,0x04,0x00,0x04,0x00,0x00,0x00}},

{0x00, 1 , {0xF0}},
{0xCB,  4 ,{0xFF,0x30,0x33,0x80}},

	{0x00,1,{0x80}},
{0xCD,  1 ,{0x01}},

	//ckh_pch
{0x00, 1 , {0xF5}},
{0xC2,  5 ,{0x00,0x00,0x80,0x80,0x00}},

	{0x00,1,{0x81}},
{0xA5,  1 ,{0x00}},

	//ckh
	{0x00,1,{0x94}},
{0xC0,  7 ,{0x00,0x01,0x06,0x02,0x04,0x11,0x04}},

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	{0x00,1,{0x00}},
{0xD8,  2 ,{0x21,0x21}},

	{0x00, 1,{0x00}},
{0xD9,  2 ,{0xAC,0x00}},

	{0x00,1,{0x00}},
	{0xE0,1,{0x00}},







	//--------Down Power  Consumption-----------------
	{0x00,1,{0x90}},
	{0xC5,1,{0x45}},

	{0x00,1,{0x91}},
	{0xC5,1,{0xA0}},

	{0x00,1,{0x86}},
	{0xC3,1,{0x33}},

	{0x00,1,{0x90}},
	{0xC3,1,{0x30}},

	{0x00,1,{0x92}},
	{0xC3,1,{0x03}},

	{0x00,1,{0x83}},
	{0xC5,1,{0x1B}},

	{0x00,1,{0x84}},
	{0xC5,1,{0xA1}},

	{0x00,1,{0xA0}},
	{0xC5,1,{0x94}},

	{0x00, 1,{0xA1}},
	{0xC5,1,{0x9E}},
	//-----------------------------------------------

	{0x00,1,{0xB1}},
	{0xC5,1,{0x08}},

	{0x00,1,{0xB2}},
	{0xC5,1,{0x22}},

{0x00, 1 , {0x00}},
{0xE1, 37 ,{0x00,0x6c,0xc7,0x09,0x40,0x31,0x58,0x9b,0xce,0x55,0xbb,0xef,0x18,0x3e,0xa5,0x6a,0x92,0x06,0x29,0xaa,0x48,0x69,0x80,0x9f,0xaa,0xc4,0xc0,0xea,0x20,0xea,0x40,0x5e,0x92,0xc9,0xff,0xff,0x03}},
	{0x00,1,{0x00}},
{0xE2, 37 ,{0x00,0x6c,0xc7,0x09,0x40,0x31,0x58,0x9b,0xce,0x55,0xbb,0xef,0x18,0x3e,0xa5,0x6a,0x92,0x06,0x29,0xaa,0x48,0x69,0x80,0x9f,0xaa,0xc4,0xc0,0xea,0x20,0xea,0x40,0x5e,0x92,0xc9,0xff,0xff,0x03}},
{0x00, 1 , {0x00}},
{0xE3, 37 ,{0x00,0x6c,0xc7,0x09,0x40,0x31,0x58,0x9b,0xce,0x55,0xbb,0xef,0x18,0x3e,0xa5,0x6a,0x92,0x06,0x29,0xaa,0x48,0x69,0x80,0x9f,0xaa,0xc4,0xc0,0xea,0x20,0xea,0x40,0x5e,0x92,0xc9,0xff,0xff,0x03}},
{0x00, 1 , {0x00}},
{0xE4, 37 ,{0x00,0x6c,0xc7,0x09,0x40,0x31,0x58,0x9b,0xce,0x55,0xbb,0xef,0x18,0x3e,0xa5,0x6a,0x92,0x06,0x29,0xaa,0x48,0x69,0x80,0x9f,0xaa,0xc4,0xc0,0xea,0x20,0xea,0x40,0x5e,0x92,0xc9,0xff,0xff,0x03}},
{0x00, 1 , {0x00}},
{0xE5, 37 ,{0x00,0x6c,0xc7,0x09,0x40,0x31,0x58,0x9b,0xce,0x55,0xbb,0xef,0x18,0x3e,0xa5,0x6a,0x92,0x06,0x29,0xaa,0x48,0x69,0x80,0x9f,0xaa,0xc4,0xc0,0xea,0x20,0xea,0x40,0x5e,0x92,0xc9,0xff,0xff,0x03}},
{0x00, 1 , {0x00}},
{0xE6, 37 ,{0x00,0x6c,0xc7,0x09,0x40,0x31,0x58,0x9b,0xce,0x55,0xbb,0xef,0x18,0x3e,0xa5,0x6a,0x92,0x06,0x29,0xaa,0x48,0x69,0x80,0x9f,0xaa,0xc4,0xc0,0xea,0x20,0xea,0x40,0x5e,0x92,0xc9,0xff,0xff,0x03}},
{0x00, 1 , {0x00}},
	{0xFF,3,{0xFF,0xFF,0xFF}},


	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 50, {}},


	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 1, {}},
};
#endif

static struct LCM_setting_table lcm_sleep_in_setting[] =
{
    {0x28, 0, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    // Sleep Mode On
    {0x10, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


//static int vcom = 0x2F;
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {
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

#if (LCM_DSI_CMD_MODE)
    	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = BURST_VDO_MODE;
#endif

          params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      		= LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size=256;

		// Video mode setting		//for low temp. can't initial
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	//params->dsi.word_count=1080*3;


    params->dsi.vertical_sync_active			= 4;
    params->dsi.vertical_backporch				= 16;
    params->dsi.vertical_frontporch				= 16;
    params->dsi.vertical_active_line			= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active			= 10;
    params->dsi.horizontal_backporch			= 66;
    params->dsi.horizontal_frontporch			= 64;
    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 490; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;
}

static void lcm_init(void)
{
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
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);

	//	SET_RESET_PIN(0);
	//MDELAY(10);
	//mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);
	//mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	//mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ZERO);
	//MDELAY(10);
	//mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);
	//mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	//mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ZERO);
	//MDELAY(10);


    SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);


        #if defined(DROI_PRO_FQ5B_XDNZ)
               lcm_state_flag = 1;
        #endif

}
static void lcm_resume(void)
{
	lcm_init();
        #if defined(DROI_PRO_FQ5B_XDNZ)
                      lcm_state_flag = 0;
        #endif
}

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


    /*if(id == LCM_ID_OTM1191A)
    	return 1;
    else*/
        return 1;
}
LCM_DRIVER otm1191a_dsi_6735_fhd_lcm_drv =
{
    	.name		= "OTM1191A_DSI_FHD",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
};
