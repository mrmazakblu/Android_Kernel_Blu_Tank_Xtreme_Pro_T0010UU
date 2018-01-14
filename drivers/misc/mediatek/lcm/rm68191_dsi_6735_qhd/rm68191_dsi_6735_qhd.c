#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#else
	#include <linux/string.h>
	#if defined(BUILD_UBOOT)
		#include <asm/arch/mt_gpio.h>
	#else
		//#include <mach/mt_gpio.h>
	#endif
#endif
#ifdef BUILD_LK
#include <platform/mt_pmic.h>
#else
//#include <mach/mt_pm_ldo.h>
//#include <mach/upmu_common.h>
#endif

#include "lcm_drv.h"
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  			(540)
#define FRAME_HEIGHT 			(960)

#define REGFLAG_DELAY          	0XFE
#define REGFLAG_END_OF_TABLE  	0xFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE			0

#define LCM_RM68191_ID          (0x8191)
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

#define SET_RESET_PIN(v)    	(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 		(lcm_util.udelay(n))
#define MDELAY(n) 		(lcm_util.mdelay(n))



// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg					lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

#if defined(TYD_PRO_V9)
#include "rm68191_dsi_6735_qhd_v9.h"
#endif

static unsigned int lcm_compare_id(void);


#if 0//defined(SUPORT_ADC_CHECK)
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);

static int lcm_read_ADC_value()
{
	int val;           //lvl = LCM_V_LEVEL;
	int dwChannel = 12; //LCM_ADC_CHAN;

	int data[4] = {0,0,0,0};
	int data0;
	char* buf_temp;
	int res =0;
	unsigned int ret;
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
	if((val>500)&&(val<900)) 	//voltage=0.7V
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
}
#endif

#ifndef TYD_LCD_USE_CUSTOM_RM68191_QHD
static struct LCM_setting_table lcm_initialization_setting[] = {
/**************************************************
IC Name: RM68191  BOE5.0
**************************************************/
	{0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x03}},
	{0x90, 9, {0x03, 0x14, 0x01, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00}},
	{0x91, 9, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x92, 11, {0x40, 0x03, 0x04, 0x05, 0x06, 0x00, 0x38, 0x00, 0x10, 0x03, 0x04}},
	{0x94, 8, {0x00, 0x08, 0x03, 0x03, 0xCA, 0x03, 0xCB, 0x0C}},
	{0x95, 16, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x99, 2, {0x00, 0x00}},
	{0x9A, 11, {0x00, 0x0F, 0x03, 0xD4, 0x03, 0xD6, 0x00, 0x00, 0x00, 0x00, 0x50}},
	{0x9B, 6, {0x01, 0x38, 0x00, 0x00, 0x00, 0x00}},
	{0x9C, 2, {0x00, 0x00}},
	{0x9D, 8, {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00}},
	{0x9E, 2, {0x00, 0x00}},
	{0xA0, 10, {0x9F, 0x1F, 0x08, 0x1F, 0x0A, 0x1F, 0x00, 0x1F, 0x14, 0x1F}},
	{0xA1, 10, {0x15, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},
	{0xA2, 10, {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},
	{0xA4, 10, {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},
	{0xA5, 10, {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x15}},
	{0xA6, 10, {0x1F, 0x14, 0x1F, 0x01, 0x1F, 0x0B, 0x1F, 0x09, 0x1F, 0x1F}},
	{0xA7, 10, {0x1F, 0x1F, 0x0B, 0x1F, 0x09, 0x1F, 0x01, 0x1F, 0x15, 0x1F}},
	{0xA8, 10, {0x14, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},
	{0xA9, 10, {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},
	{0xAB, 10, {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}},
	{0xAC, 10, {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x14}},
	{0xAD, 10, {0x1F, 0x15, 0x1F, 0x00, 0x1F, 0x08, 0x1F, 0x0A, 0x1F, 0x1F}},
	{0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x00}},
	{0xBC, 3, {0x00, 0x00, 0x00}},
	{0xB8, 4, {0x01, 0xAF, 0x8F, 0x8F}},
	{0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x01}},
	{0xD1, 16, {0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},
	{0xD2, 16, {0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},
	{0xD3, 16, {0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},
	{0xD4, 4, {0x03, 0xE5, 0x03, 0xFF}},
	{0xD5, 16, {0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},
	{0xD6, 16, {0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},
	{0xD7, 16, {0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},
	{0xD8, 4, {0x03, 0xE5, 0x03, 0xFF}},
	{0xD9, 16, {0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},
	{0xDD, 16, {0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},
	{0xDE, 16, {0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},
	{0xDF, 4, {0x03, 0xE5, 0x03, 0xFF}},
	{0xE0, 16, {0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},
	{0xE1, 16, {0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},
	{0xE2, 16, {0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},
	{0xE3, 4, {0x03, 0xE5, 0x03, 0xFF}},
	{0xE4, 16, {0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},
	{0xE5, 16, {0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},
	{0xE6, 16, {0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},
	{0xE7, 4, {0x03, 0xE5, 0x03, 0xFF}},
	{0xE8, 16, {0x00, 0x00, 0x00, 0x0F, 0x00, 0x2A, 0x00, 0x41, 0x00, 0x54, 0x00, 0x75, 0x00, 0x92, 0x00, 0xC0}},
	{0xE9, 16, {0x00, 0xE7, 0x01, 0x25, 0x01, 0x56, 0x01, 0xA6, 0x01, 0xE6, 0x01, 0xE8, 0x02, 0x24, 0x02, 0x63}},
	{0xEA, 16, {0x02, 0x8D, 0x02, 0xC4, 0x02, 0xEC, 0x03, 0x23, 0x03, 0x48, 0x03, 0x78, 0x03, 0x97, 0x03, 0xBD}},
	{0xEB, 4, {0x03, 0xE5, 0x03, 0xFF}},
	{0xB0, 3, {0x05, 0x05, 0x05}},
	{0xB1, 3, {0x05, 0x05, 0x05}},
	{0xB3, 3, {0x10, 0x10, 0x10}},
	{0xB4, 3, {0x06, 0x06, 0x06}},
	{0xB6, 3, {0x44, 0x44, 0x44}},
	{0xB7, 3, {0x34, 0x34, 0x34}},
	{0xB8, 3, {0x34, 0x34, 0x34}},
	{0xB9, 3, {0x24, 0x24, 0x24}},
	{0xBA, 3, {0x24, 0x24, 0x24}},
	{0xBC, 3, {0x00, 0x70, 0x00}},
	{0xBD, 3, {0x00, 0x70, 0x00}},
	{0xBE, 1, {0x50}},
	{0x35, 1, {0x00}},

	{0x11, 0,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0x29, 0,{0x00}},
	{REGFLAG_DELAY, 200, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

#if 0
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 150, {}},

    // Display ON
//	{0x29, 1, {0x00}},
//	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
//	{0x28, 1, {0x00}},
//	{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#endif
static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xf0, 5, {0x55, 0xaa, 0x52, 0x08, 0x01}},
	{REGFLAG_DELAY, 10, {}},
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
            case 0xBE:
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
#if defined(TYD_PRO_W6_FM)
    params->dsi.LANE_NUM				= LCM_THREE_LANE;
#else
    params->dsi.LANE_NUM				= LCM_TWO_LANE;//LCM_THREE_LANE;
#endif

    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format	  = LCM_DSI_FORMAT_RGB888;

    // Video mode setting
    params->dsi.intermediat_buffer_num = 2;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
    params->dsi.vertical_active_line=FRAME_HEIGHT;

	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 32;
	params->dsi.horizontal_frontporch	= 32;
       //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's
#if defined(TYD_PRO_W6_FM)
	params->dsi.PLL_CLOCK=145;
#elif defined(TYD_PRO_V9_JHY3_QHD)
	params->dsi.PLL_CLOCK=240;
#else
	params->dsi.PLL_CLOCK=250;
#endif
    //params->dsi.esd_check_enable = 1;
    //params->dsi.customization_esd_check_enable = 1;
	//params->dsi.lcm_esd_check_table[0].cmd = 0xac;//0x0a;
	//params->dsi.lcm_esd_check_table[0].count = 1;
	//params->dsi.lcm_esd_check_table[0].para_list[0] = 0x00;//0x9c
}

static void lcm_init(void)
{

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(150);

#if 0//defined(SUPORT_ADC_CHECK)
	int val = 0;
	val=lcm_read_ADC_value();
#ifdef BUILD_LK
	printf("ZWS------------------- val:%d\n",val);
#else
	printk("ZWS------------------- val:%d\n",val);
#endif
	adc_lcm_push_table(val);
#else
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
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
	//MDELAY(1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	//MDELAY(1);

	data_array[0]= 0x00290508;
	dsi_set_cmdq(data_array, 1, 1);
	//MDELAY(1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
	//MDELAY(1);

}
#endif

static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500;
	dsi_set_cmdq(data_array,1,1);
	MDELAY(10);
	data_array[0]=0x00100500;
	dsi_set_cmdq(data_array,1,1);
	MDELAY(100);
}


static void lcm_resume(void)
{
/*
	unsigned int data_array[16];
	data_array[0]=0x00110500;
	dsi_set_cmdq(&data_array,1,1);
	MDELAY(100);
	data_array[0]=0x00290500;
	dsi_set_cmdq(&data_array,1,1);
	MDELAY(10);
*/
	lcm_init();
}

static unsigned int lcm_compare_id(void)
{
	unsigned int id;
	unsigned char buffer[5];
	unsigned int array[5];

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(150);

	push_table(lcm_compare_id_setting,
			sizeof(lcm_compare_id_setting) /
			sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xc5, buffer, 2);
	id = ((buffer[0] << 8) | buffer[1]);
#ifndef BUILD_LK
    printk("RM68191 read id=%x\n",id);
#endif

	return ((LCM_RM68191_ID == id)? 1 : 0);
}
#if 0
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    int array[4];
    char buffer[3]={0};

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x0A, buffer, 1);

     //printk(KERN_ERR "nt35517:read register 0x0A=%4x\n",buffer[0]);
    if(0x9c == buffer[0])
        return FALSE;
    else
        return TRUE;
#endif
}

static unsigned int lcm_esd_recover(void)
{

    lcm_init();
    return TRUE;
}
#endif

#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif

static unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
	unsigned int id;
	unsigned char buff[5];
	unsigned int array[5];

	push_table(lcm_compare_id_setting,
			sizeof(lcm_compare_id_setting) /
			sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	atomic_set(&ESDCheck_byCPU,1);
	read_reg_v2(0xc5, buff, 2);
	atomic_set(&ESDCheck_byCPU,0);
	id = ((buff[0] << 8) | buff[1]);
#ifndef BUILD_LK
    printk("RM68191 read id=%x\n",id);
#endif

	return ((LCM_RM68191_ID == id)? 1 : 0);
#endif
}


LCM_DRIVER rm68191_dsi_6735_qhd_lcm_drv =
{
	.name			= "rm68191_dsi_Q",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id   	= lcm_compare_id,
	.ata_check      = lcm_ata_check,
#if 0
    .esd_check      = lcm_esd_check,
    .esd_recover    = lcm_esd_recover,
#endif
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
};
