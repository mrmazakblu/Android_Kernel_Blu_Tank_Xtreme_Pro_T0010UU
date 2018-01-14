#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
#endif
// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (720)
#define FRAME_HEIGHT (1280)

#define REGFLAG_DELAY 0xAB
#define REGFLAG_END_OF_TABLE 0xAA // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE 0

#define LCM_ID_S6D7AA0X62				0x0802
#define DEBUG 0

#if 0
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE; ///only for ESD test

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
struct LCM_setting_table {
unsigned char cmd;
unsigned char count;
unsigned char para_list[64];
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);

#if defined(DROI_PRO_F5C)
#include "s6d7aa0_hd_vdo_f5c.h"
#endif

#if defined(TYD_PRO_F6)
#include "s6d7aa0_hd_vdo_f6.h"
#endif

#if defined(DROI_PRO_Q1)
#include "s6d7aa0_hd_vdo_q1.h"
#endif

#ifndef TYD_LCD_USE_CUSTOM_S6D7AA0_HD
static struct LCM_setting_table lcm_initialization_setting[] = {

};
#endif





static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
     // Display off sequence
     {0x28, 1, {0x00}},
     {REGFLAG_DELAY, 20, {}},
     // Sleep Mode Ondiv1_real*div2_real
     {0x10, 1, {0x00}},
     {REGFLAG_DELAY, 120, {}},
     {REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct LCM_setting_table lcm_backlight_level_setting[] = {
    {0x51, 1, {0xFF}},
    {0x53, 1, {0x24}},//close dimming
    {0x55, 1, {0x00}},//close cabc
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
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if 0//(LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
#if defined(TYD_PRO_F6_LM)||defined(DROI_PRO_F5C_GQBN)
	params->dsi.LANE_NUM				= LCM_THREE_LANE;
#else
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;//LCM_THREE_LANE;
#endif
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
#if defined(DROI_PRO_F5C_JR)
params->dsi.vertical_sync_active				= 4;//8;
	params->dsi.vertical_backporch					= 8;//8;
	params->dsi.vertical_frontporch					= 16;//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				=16;//16;
	params->dsi.horizontal_backporch				=32;//16;
	params->dsi.horizontal_frontporch				=16;//16;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.esd_check_enable					=1;
	params->dsi.customization_esd_check_enable		=1;
	params->dsi.lcm_esd_check_table[0].cmd			=0x0a;
	params->dsi.lcm_esd_check_table[0].count		=1;
	params->dsi.lcm_esd_check_table[0].para_list[0]	=0x9c;

    // Bit rate calculation
	//1 Every lane speed
	params->dsi.PLL_CLOCK = 205;//220;
#elif defined(TYD_PRO_F6_LM)
	params->dsi.vertical_sync_active				= 4;//8;
	params->dsi.vertical_backporch					= 8;//8;
	params->dsi.vertical_frontporch					= 16;//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				=16;//16;
	params->dsi.horizontal_backporch				=16;//16;
	params->dsi.horizontal_frontporch				=16;//16;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.esd_check_enable					=1;
	params->dsi.customization_esd_check_enable		=1;
	params->dsi.lcm_esd_check_table[0].cmd			=0x0a;
	params->dsi.lcm_esd_check_table[0].count		=1;
	params->dsi.lcm_esd_check_table[0].para_list[0]	=0x9c;

    // Bit rate calculation
	//1 Every lane speed
	params->dsi.PLL_CLOCK = 256;//220;
#elif defined(DROI_PRO_F5C_PY_A1)||defined(DROI_PRO_F5C_PY_E1)
	params->dsi.vertical_sync_active				= 4;//8;
	params->dsi.vertical_backporch					= 8;//8;
	params->dsi.vertical_frontporch					= 16;//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				=16;//16;
	params->dsi.horizontal_backporch				=32;//16;
	params->dsi.horizontal_frontporch				=16;//16;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.esd_check_enable					=1;
	params->dsi.customization_esd_check_enable		=1;
	params->dsi.lcm_esd_check_table[0].cmd			=0x0a;
	params->dsi.lcm_esd_check_table[0].count		=1;
	params->dsi.lcm_esd_check_table[0].para_list[0]	=0x9c;

    // Bit rate calculation
	//1 Every lane speed
	params->dsi.PLL_CLOCK = 205;//220;

#elif defined(DROI_PRO_F5C_ZXHL_A6)||defined(DROI_PRO_F5C_NYX)
	params->dsi.vertical_sync_active				= 4;//8;
	params->dsi.vertical_backporch					= 8;//8;
	params->dsi.vertical_frontporch					= 16;//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				=16;//16;
	params->dsi.horizontal_backporch				=32;//16;
	params->dsi.horizontal_frontporch				=16;//16;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.esd_check_enable					=1;
	params->dsi.customization_esd_check_enable		=1;
	params->dsi.lcm_esd_check_table[0].cmd			=0x0a;
	params->dsi.lcm_esd_check_table[0].count		=1;
	params->dsi.lcm_esd_check_table[0].para_list[0]	=0x9c;

    // Bit rate calculation
	//1 Every lane speed
	params->dsi.PLL_CLOCK = 197;//220;

#elif defined(DROI_PRO_F5C_CFLQ)||defined(DROI_PRO_Q1_CFLQ)||defined(DROI_PRO_Q1_CF)
	params->dsi.vertical_sync_active				= 4;//8;
	params->dsi.vertical_backporch					= 8;//8;
	params->dsi.vertical_frontporch					= 16;//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				=16;//16;
	params->dsi.horizontal_backporch				=32;//16;
	params->dsi.horizontal_frontporch				=16;//16;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.esd_check_enable					=1;
	params->dsi.customization_esd_check_enable		=1;
	params->dsi.lcm_esd_check_table[0].cmd			=0x0a;
	params->dsi.lcm_esd_check_table[0].count		=1;
	params->dsi.lcm_esd_check_table[0].para_list[0]	=0x9c;

    // Bit rate calculation
	//1 Every lane speed
	params->dsi.PLL_CLOCK = 205;//220;
#else
	params->dsi.vertical_sync_active				= 4;//8;
	params->dsi.vertical_backporch					= 8;//8;
	params->dsi.vertical_frontporch					= 16;//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				=16;//16;
	params->dsi.horizontal_backporch				=32;//16;
	params->dsi.horizontal_frontporch				=16;//16;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.esd_check_enable					=1;
	params->dsi.customization_esd_check_enable		=1;
	params->dsi.lcm_esd_check_table[0].cmd			=0x0a;
	params->dsi.lcm_esd_check_table[0].count		=1;
	params->dsi.lcm_esd_check_table[0].para_list[0]	=0x9c;

    // Bit rate calculation
	//1 Every lane speed
	params->dsi.PLL_CLOCK = 205;//220;
#endif
}



static unsigned int lcm_compare_id(void)
{
 unsigned int id = 0;
unsigned int id_high=0,id_low=0;

unsigned char buffer[3];
unsigned int data_array[16];

SET_RESET_PIN(1);
MDELAY(10);
SET_RESET_PIN(0);
MDELAY(50);
SET_RESET_PIN(1);
MDELAY(50);

data_array[0]= 0x00033902;
data_array[1]= 0x005A5AF0;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x005A5AF1;
dsi_set_cmdq(data_array, 2, 1);


data_array[0] = 0x00023700;// set return byte number
dsi_set_cmdq(data_array, 1, 1);

read_reg_v2(0xF2, buffer, 2);
id_high = buffer[1];//0x08
id_low = buffer[0];//0x02

id = (id_high<<8) | id_low;

#if DEBUG
  #ifdef BUILD_LK
 //     printf("lishuwen_test nt35517 uboot %s\n", __func__);
      printf("lishuwen_test %s, id = 0x%x\n", __func__, id);//should be 0x8009
  #else
 //     printk("lishuwen_test nt35517 kernel %s\n", __func__);
      printk("lishuwen_test %s, id = 0x%x\n", __func__, id);//should be 0x8009
  #endif
#endif
 //return 1;
return (LCM_ID_S6D7AA0X62 == id) ? 1 : 0;
}

static void lcm_init(void)
{
#if defined(TYD_PRO_F6_LM)
    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(100);
#else
    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(30);
    SET_RESET_PIN(1);
    MDELAY(30);
#endif
    //lcm_init_registers();
#if DEBUG
      #ifdef BUILD_LK
      printf("lishuwen_test nt35517 uboot %s\n", __func__);
   //   printf("lishuwen_test %s, id = 0x%x 0x%x 0x%x 0x%x\n", __func__, buffer[0], buffer[1], buffer[2], buffer[3]);//should be 0x8009
  #else
      printk("lishuwen_test nt35517 kernel %s\n", __func__);
     // printk("lishuwen_test %s, id = 0x%x 0x%x 0x%x 0x%x\n", __func__, buffer[0], buffer[1], buffer[2], buffer[3]);//should be 0x8009
  #endif
#endif
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
#if DEBUG
	      #ifdef BUILD_LK
      printf("lishuwen_test nt35517 uboot %s\n", __func__);
   //   printf("lishuwen_test %s, id = 0x%x 0x%x 0x%x 0x%x\n", __func__, buffer[0], buffer[1], buffer[2], buffer[3]);//should be 0x8009
  #else
      printk("lishuwen_test nt35517 kernel %s\n", __func__);
     // printk("lishuwen_test %s, id = 0x%x 0x%x 0x%x 0x%x\n", __func__, buffer[0], buffer[1], buffer[2], buffer[3]);//should be 0x8009
  #endif
	lcm_compare_id();
#endif
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

}

static void lcm_resume(void)
{

	 lcm_init();
#if DEBUG
	      #ifdef BUILD_LK
      printf("lishuwen_test nt35517 uboot %s\n", __func__);
   //   printf("lishuwen_test %s, id = 0x%x 0x%x 0x%x 0x%x\n", __func__, buffer[0], buffer[1], buffer[2], buffer[3]);//should be 0x8009
  #else
      printk("lishuwen_test nt35517 kernel %s\n", __func__);
     // printk("lishuwen_test %s, id = 0x%x 0x%x 0x%x 0x%x\n", __func__, buffer[0], buffer[1], buffer[2], buffer[3]);//should be 0x8009
  #endif

#endif
	//lcm_initialization_setting[24].para_list[0]+=2;
    //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
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

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif

#if 0
static void lcm_setbacklight(unsigned int level)
{

    unsigned int mapped_level = 0;

    //for LGE backlight IC mapping table
    if(level > 255)
    level = 255;

    mapped_level = level * 7 / 10;//to reduce power


    // Refresh value of backlight level.
    lcm_backlight_level_setting[0].para_list[0] = mapped_level;

#ifdef BUILD_LK
    printf("uboot:otm9605a_lcm_setbacklight mapped_level = %d,level=%d\n",mapped_level,level);
#else
    printk("kernel:otm9605a_lcm_setbacklight mapped_level = %d,level=%d\n",mapped_level,level);
#endif

    push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);

}

static unsigned int lcm_esd_check(void)
{
    #ifndef BUILD_LK
    if(lcm_esd_test)
    {
        lcm_esd_test = FALSE;
        return TRUE;
    }

    if(read_reg(0x0A) == 0x9C)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    #endif
}

static unsigned int lcm_esd_recover(void)
{
    unsigned char para = 0;

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);

    push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);

    dsi_set_cmdq_V2(0x35, 1, &para, 1); ///enable TE
    MDELAY(10);

    return TRUE;
}
#endif


LCM_DRIVER 	s6d7aa0_hd_vdo_drv	 =
{
    .name			= "s6d7aa0_hd_vdo",
    .set_util_funcs = lcm_set_util_funcs,

    .get_params = lcm_get_params,
    .init = lcm_init,
    .suspend = lcm_suspend,
    .resume = lcm_resume,
    .compare_id = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    //.set_backlight = lcm_setbacklight,
    //.esd_check = lcm_esd_check,
    //.esd_recover = lcm_esd_recover,
    .update = lcm_update,
#endif
};

