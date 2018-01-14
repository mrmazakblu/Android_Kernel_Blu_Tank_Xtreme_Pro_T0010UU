#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
//#include <mach/mt_gpio.h>
#endif

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY          	0X11FE
#define REGFLAG_END_OF_TABLE  	0xFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE			0

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};

#if defined(DROI_PRO_F6)
#include "rm68200_dsi_6735_hd_f6.h"
#elif defined(DROI_PRO_Q1)
#include "rm68200_dsi_6735_hd_q1.h"
#elif defined(DROI_PRO_F5C)
#include "rm68200_dsi_6735_hd_f5c.h"
#endif

#if defined(DROI_PRO_PF5)
#include "rm68200_dsi_6735_hd_pf5.h"
#endif

#if defined(DROI_PRO_FQ5CW)
#include "rm68200_dsi_6735_hd_fq5cw.h"
#endif

#if defined(DROI_PRO_B38)
#include "rm68200_dsi_6735_hd_b38.h"
#endif

#if defined(SUPORT_ADC_CHECK)
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);

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
#if defined(DROI_PRO_F5C_WHX)
	if((val>1000)&&(val<1400)) 	//voltage=1.2V
	{
		push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}else if((val>600)&&(val<1000)) 	//voltage=0.8V
	{
		push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

	}
#else
	if((val>200)&&(val<600)) 	//voltage=0.8V
	{
		push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

	}
#endif
}
#endif

#ifndef TYD_LCD_USE_CUSTOM_RM68200_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
};
#endif

static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},

	{REGFLAG_DELAY, 50, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},

	{REGFLAG_DELAY, 100, {}},

	{0x4F, 1, {0x01}},
#if defined(DROI_PRO_F5C_LEAGOO4)
	{REGFLAG_DELAY, 50, {}},
#endif

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

//static int vcom= 0x40;
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

#if 0
		case 0x46:
			table[i].para_list[0]=vcom;
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
			vcom+=2;
			break;
#endif

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

	params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = BURST_VDO_MODE;//SYNC_EVENT_VDO_MODE;//BURST_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
#if defined(DROI_PRO_F5C_LEAGOO3) || defined(DROI_PRO_F5C_SGDZ) || defined(DROI_PRO_FQ5CW_ZGW5) || defined(DROI_PRO_F5C_ZGW_E2) || defined(DROI_PRO_PF5_LEAGOO3) || defined(DROI_PRO_B38_GB)||defined(DROI_PRO_F5C_ZGW_A1)||defined(DROI_PRO_F5C_BPZN2)
    params->dsi.LANE_NUM				= LCM_THREE_LANE;
#else
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
#endif


	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format	  = LCM_DSI_FORMAT_RGB888;

	// Video mode setting
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.word_count=FRAME_WIDTH*3;

#if defined(DROI_PRO_F6_GB)||defined(DROI_PRO_Q1_DJ) || defined(DROI_PRO_F6_G2)
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
	//params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	//params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=212;

	// Non-continuous clock
	params->dsi.noncont_clock = 1;
	params->dsi.noncont_clock_period = 1; // 1=per frame 3=per line
#elif defined(DROI_PRO_F5C_BPZN2)
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 32;
	params->dsi.horizontal_frontporch	= 32;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=245;
	params->dsi.noncont_clock=1;
	params->dsi.noncont_clock_period=1;

	// ESD
	params->dsi.esd_check_enable=1;
	params->dsi.customization_esd_check_enable=1;
	params->dsi.lcm_esd_check_table[0].cmd=0x0A;
	params->dsi.lcm_esd_check_table[0].count=1;
	params->dsi.lcm_esd_check_table[0].para_list[0]=0x9C;
#elif defined(DROI_PRO_F5C_ZGW_A1)
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 32;
	params->dsi.horizontal_frontporch	= 32;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=245;
#elif defined(DROI_PRO_F5C_ZGW_E2)
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 32;
	params->dsi.horizontal_frontporch	= 32;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=245;
#elif defined(DROI_PRO_F5C_FM)
        params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;// 50  2
	params->dsi.horizontal_backporch	= 48;
	params->dsi.horizontal_frontporch	= 48;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;
	params->dsi.PLL_CLOCK=212;

		//esd_check
	params->dsi.esd_check_enable 					= 1;
	params->dsi.customization_esd_check_enable 		= 1;
	params->dsi.lcm_esd_check_table[0].cmd 			= 0x0a;
	params->dsi.lcm_esd_check_table[0].count 		= 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;

#elif defined(DROI_PRO_F5C_FM2_E2)
    	params->dsi.vertical_sync_active	= 4;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 10; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;// 50  2
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=215;
#elif defined(DROI_PRO_F5C_LEAGOO3)
		params->dsi.ssc_disable = 1;
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 32;
	params->dsi.horizontal_frontporch	= 32;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	params->physical_width = 71;
	params->physical_height = 126;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=260;
#elif defined(DROI_PRO_F5C_SGDZ)
    params->dsi.vertical_active_line=FRAME_HEIGHT;
    params->dsi.vertical_sync_active	= 4;// 3    2
	params->dsi.vertical_backporch		= 4;// 20   1
	params->dsi.vertical_frontporch		= 10; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;// 50  2
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=240;
#elif defined(DROI_PRO_F6_PY) || defined(DROI_PRO_F6_KR)
	params->dsi.vertical_active_line=FRAME_HEIGHT;
	params->dsi.vertical_sync_active	= 4;
	params->dsi.vertical_backporch		= 36;
	params->dsi.vertical_frontporch		= 16;
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=240;
#elif defined(DROI_PRO_F6_YH)
	params->dsi.vertical_sync_active			= 4;
	params->dsi.vertical_backporch				= 16;
	params->dsi.vertical_frontporch				= 16;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 8;
	params->dsi.horizontal_backporch			= 32;
	params->dsi.horizontal_frontporch			= 32;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 195; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;
	params->physical_width = 68;
	params->physical_height = 122;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_PF5_LEAGOO3)
	params->dsi.ssc_disable = 1;
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 24;
	params->dsi.horizontal_frontporch	= 24;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=249;
#elif defined(DROI_PRO_B38_GB)
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;// 50  2
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
	//params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	//params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=247;
#elif defined(DROI_PRO_F6_YT) || defined(DROI_PRO_F6_NYX)
	params->dsi.vertical_sync_active	= 4;
	params->dsi.vertical_backporch		= 36;
	params->dsi.vertical_frontporch		= 16;
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=220;
#elif defined(DROI_PRO_F5C_LEAGOO4)
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 36;
	//params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=198;

	params->dsi.ssc_disable = 1;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
	params->dsi.noncont_clock = 1;
	params->dsi.noncont_clock_period = 1;
	//params->dsi.compatibility_for_nvk = 0;

#elif defined(DROI_PRO_Q1_HN)
	params->dsi.vertical_sync_active	= 4;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 10; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;// 50  2
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
	//params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	//params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=220;


	params->physical_width = 70;
	params->physical_height = 121;
#else
	params->dsi.vertical_sync_active	= 4;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 10; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 2;// 50  2
	params->dsi.horizontal_backporch	= 36;
	params->dsi.horizontal_frontporch	= 26;
	//params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	//params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=220;
#endif
}

static void lcm_init(void)
{
	#if defined(SUPORT_ADC_CHECK)
	int val;
	#endif
#if defined(DROI_PRO_F5C_FM)||defined(DROI_PRO_F5C_FM2_E2)||defined(DROI_PRO_B38_GB)
 SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);
#else
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(150);
#endif

#if defined(SUPORT_ADC_CHECK)
	val = 0;
	val=lcm_read_ADC_value();
#ifdef BUILD_LK
	printf("GC------------------- val:%d\n",val);
#else
	printk("GC------------------- val:%d\n",val);
#endif
	adc_lcm_push_table(val);
#else
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}

static void lcm_suspend(void)
{
	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
	lcm_init();
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x,unsigned int y,unsigned int width,unsigned int height)
{
	static int last_update_x      = -1;
	static int last_update_y      = -1;
	static int last_update_width  = -1;
	static int last_update_height = -1;
	unsigned int need_update = 1;

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

	// need update at the first time
	if(-1 == last_update_x && -1 == last_update_y && -1 == last_update_width && -1 == last_update_height)
	{
		last_update_x      = (int)x;
		last_update_y      = (int)y;
		last_update_width  = (int)width;
		last_update_height = (int)height;
	}
	// no need update if the same region as last time
	else if(last_update_x == (int)x && last_update_y == (int)y && last_update_width == (int)width && last_update_height == (int)height)
	{
		//need_update = 0;
	}
	// need update if region change
	else
	{
		last_update_x      = (int)x;
		last_update_y      = (int)y;
		last_update_width  = (int)width;
		last_update_height = (int)height;
	}

	if(need_update)
	{
		data_array[0]= 0x00053902;
		data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
		data_array[2]= (x1_LSB);
		dsi_set_cmdq(data_array, 3, 1);

		data_array[0]= 0x00053902;
		data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
		data_array[2]= (y1_LSB);
		dsi_set_cmdq(data_array, 3, 1);
	}

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif

static unsigned int lcm_compare_id(void)
{

	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id1=0;

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	array[0]=0x01FE1500;
	dsi_set_cmdq(array,1, 1);

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xde, buffer, 1);

	id_high = buffer[0];
	read_reg_v2(0xdf, buffer, 1);
	id_low = buffer[0];
	id1 = (id_high<<8) | id_low;

#if defined(BUILD_LK)
	printf("rm68200a %s id1 = 0x%04x\n", __func__, id1);
#else
	printk("rm68200a %s id1 = 0x%04x\n", __func__, id1);
#endif

	return (0x6820 == id1)?1:0;
}
#ifndef DROI_PRO_F5C_LEAGOO3
#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif
#endif

static unsigned int lcm_ata_check(unsigned char *buffer)
{
#if defined(DROI_PRO_F5C_LEAGOO3)||defined(DROI_PRO_F5C_LEAGOO4)
	return 1;
#else
#ifndef BUILD_LK
	int array[4];
	char buff[5];
	char id_high=0;
	char id_low=0;
	int id1=0;

	array[0]=0x01FE1500;
	dsi_set_cmdq(array,1, 1);

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	atomic_set(&ESDCheck_byCPU,1);
	read_reg_v2(0xde, buff, 1);
	atomic_set(&ESDCheck_byCPU,0);

	id_high = buff[0];

	atomic_set(&ESDCheck_byCPU,1);
	read_reg_v2(0xdf, buff, 1);
	atomic_set(&ESDCheck_byCPU,0);

	id_low = buff[0];
	id1 = (id_high<<8) | id_low;

	return (0x6820 == id1)?1:0;
#endif
#endif
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER rm68200_dsi_6735_hd_drv =
{
	.name		    = "rm68200_dsi_H",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
	.ata_check	= lcm_ata_check
};
