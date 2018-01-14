#ifdef BUILD_LK
#include <string.h>
#include <mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <linux/string.h>
#endif

#include "lcm_drv.h"

#if defined(BUILD_LK)
#define LCM_PRINT printf
#elif defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif

// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0
#define FRAME_WIDTH (720)
#define FRAME_HEIGHT (1280)
#define REGFLAG_DELAY          		(0XFE)
#define REGFLAG_END_OF_TABLE      	(0xFFF)	// END OF REGISTERS MARKER


#define   LCM_ID_OTM1287A    0x1287

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// ---------------------------------------------------------------------------
// Local Variables
// ---------------------------------------------------------------------------

//extern void set_HS_read();
//extern void restore_HS_read();

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


#if defined(SUPORT_ADC_CHECK)
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

	if((val>1000)&&(val<1400)) 	    //voltage=1.2V
	{
	    push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000)) 	    //voltage=0.8V
	{
	    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>200)&&(val<600)) 	    //voltage=0.4V
	{
	    push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

	}

}
#endif

#ifndef TYD_LCD_USE_CUSTOM_OTM1287A_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
//OTM1287A_CMI5.0  HongXian    20150908
    {0x00,1,{0x00}},
	{0xff,3,{0x12,0x87,0x01}},	//EXTC=1
	{0x00,1,{0x80}},	        //Orise mode enable
	{0xff,2,{0x12,0x87}},

	{0x00,1,{0x92}},
	{0xff,2,{0x20,0x02}},		//MIPI 30 4 Lane  20 3lane

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
	{REGFLAG_DELAY,50,{}},
	{0x2c, 1, {0x00}},
	{REGFLAG_DELAY,20,{}},
	//{0xfff, 0x00, {}}
};
#endif

#if 0
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
        {0x28, 1, {0x00}},
        {REGFLAG_DELAY, 20, {}},
    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
{0x51, 1, {0xFF}},
{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
//static int vcom = 0x6A;
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
      	    		case 0xD9:
		    	table[i].para_list[0]=vcom;
		    	dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
            		vcom+=1;
            		break;
#endif
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
    //params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

    #if (LCM_DSI_CMD_MODE)
    params->dsi.mode   = CMD_MODE;
    #else
    params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
    #endif

    // DSI
    /* Command mode setting */
    //1 Three lane or Four lane
    params->dsi.LANE_NUM				= LCM_THREE_LANE;

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

	params->dsi.vertical_sync_active				= 4; //  4  0x3;// 3    2
	params->dsi.vertical_backporch					= 16;// 15 0x8;// 20   1
	params->dsi.vertical_frontporch					= 15; //16 0x8; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;//10 50  2
	params->dsi.horizontal_backporch				= 64;//42
	params->dsi.horizontal_frontporch				= 64; //44
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=300;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0xac;//0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x00;//0x9c;
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
	MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);//Must over 6 ms,SPEC request

#if defined(SUPORT_ADC_CHECK)
	int val = 0;
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
	//unsigned int data_array[16];

	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(30);
	SET_RESET_PIN(1);
	MDELAY(120);

}

static void lcm_resume(void)
{
	lcm_init();
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
/*
static unsigned int lcm_esd_check(void)
{
	int array[4];
	char buffer[3]={0};

#if defined(BUILD_LK)
    printf("lcm_esd_check  come in\n");
#else
    printk("lcm_esd_check  come in\n");
#endif

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x0A, buffer, 1);
	if(0x9c == buffer[0])
	return FALSE;
	else if(0x02 == buffer[0])
	{
		array[0] = 0x00013700;
		dsi_set_cmdq(array, 1, 1);
		read_reg_v2(0x0A, buffer, 1);
		if(0x9c == buffer[0])
		return FALSE;
	}
	else if(0x02 == buffer[0])
	{
	// printk(KERN_ERR "otm1287a:read register 0x0A=%4x\n",buffer[0]);
	return TRUE;
	}

	return FALSE;
}
*/

#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif

unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
	unsigned int id=0, id1 = 0;
	//unsigned int ret = 0;
	//unsigned int data_array[3];
	//unsigned char read_buf[4];

	unsigned int array[2];
	unsigned char buff[5];

//	printk("G_TEST 1");

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
LCM_DRIVER otm1287a_dsi_6580_hd_drv =
{
    	.name		= "otm1287a_dsi_6580_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if 0
	.esd_check	= lcm_esd_check,
	.esd_recover	= lcm_esd_recover,
#endif
   .ata_check      = lcm_ata_check,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};
