#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/upmu_common.h>
	#include <platform/upmu_hw.h>
#else
	#include <mt-plat/upmu_common.h>
	#include <mach/upmu_hw.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)

#define REGFLAG_DELAY             							0XFC
#define REGFLAG_END_OF_TABLE      							0xFD   // END OF REGISTERS MARKER

#define LCM_ID (0x7701)

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

#if defined(SUPORT_ADC_CHECK)
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);
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
}
#endif

#ifndef DROI_LCD_USE_CUSTOM_ST7701_FWVGA
static struct LCM_setting_table lcm_initialization_setting[] = {
    /* ST7701 Initial Code For IVO4.98TN(C050SWY7-0)                              */
    {0x11,  0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
  {0xFF,5,{0x77,0x01,0x00,0x00,0x10}},



{0xC0,2,{0xE9,0x03}},


{0xC1,2,{0x0D,0x02}},


{0xC2,2,{0x31,0x06}},
//----------------------------------Gamma Cluster Setting----------------------------------------//


{0xB0,16,{0x00,0x07,0x93,0x13,0x19,0x0B,0x0B,0x09,0x08,0x1F,0x08,0x15,0x11,0x0F,0x18,0x17}},


{0xB1,16,{0x00,0x07,0x92,0x12,0x15,0x09,0x08,0x09,0x09,0x1F,0x07,0x15,0x11,0x15,0x18,0x17}},
//------------------------------------End Gamma Setting-------------------------------------------//
//---------------------------------End Display Control setting-------------------------------------//
//--------------------------------------Bank0 Setting End------------------------------------------//
//----------------------------------------Bank1 Setting------------------------------------------------//
//----------------------------- Power Control Registers Initial -----------------------------------//


{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},


{0xB0,1,{0x4D}},

//----------------------------------------Vcom Setting------------------------------------------------//

{0xB1,1,{0x47}},

//--------------------------------------End Vcom Setting--------------------------------------------//

{0xB2,1,{0x07}},


{0xB3,1,{0x80}},


{0xB5,1,{0x47}},


{0xB7,1,{0x8A}},


{0xB8,1,{0x20}},


{0xC1,1,{0x78}},


{0xC2,1,{0x78}},


{0xD0,1,{0x88}},
//------------------------------End Power Control Registers Initial ----------------------------//
   {REGFLAG_DELAY, 100, {}},
//------------------------------------------GIP Setting-------------------------------------------------//


{0xE0,3,{0x00,0x00,0x02}},


{0xE1,10,{0x02,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x40,0x40}},


{0xE2,13,{0x33,0x33,0x34,0x34,0x62,0x00,0x63,0x00,0x61,0x00,0x64,0x00,0x00}},


{0xE3,4,{0x00,0x00,0x33,0x33}},


{0xE4,2,{0x44,0x44}},


{0xE5,16,{0x04,0x6B,0xA0,0xA0,0x06,0x6B,0xA0,0xA0,0x08,0x6B,0xA0,0xA0,0x0A,0x6B,0xA0,0xA0}},


{0xE6,4,{0x00,0x00,0x33,0x33}},


{0xE7,2,{0x44,0x44}},


{0xE8,16,{0x03,0x6B,0xA0,0xA0,0x05,0x6B,0xA0,0xA0,0x07,0x6B,0xA0,0xA0,0x09,0x6B,0xA0,0xA0}},


{0xEB,7,{0x02,0x00,0x39,0x39,0x88,0x33,0x10}},


{0xEC,2,{0x02,0x00}},


{0xED,16,{0xFF,0x04,0x56,0x7F,0x89,0xF2,0xFF,0x3F,0xF3,0xFF,0x2F,0x98,0xF7,0x65,0x40,0xFF}},
//------------------------------------------End GIP Setting--------------------------------------------//
//--------------------------- Power Control Registers Initial End--------------------------------//
//---------------------------------------Bank1 Setting-------------------------------------------------//



{0xFF,5,{0x77,0x01,0x00,0x00,0x00}},
    {0x29,  0, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {} }
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
#endif

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Sleep Out
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},

	// Display ON
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_read_id[] = {
	{0x01, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x11, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0xFF, 5,{0x77,0x01,0x00,0x00,0x11}},
	{0xD1, 1, {0x11}},
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
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;  //LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

	params->dsi.mode   = SYNC_PULSE_VDO_MODE; //BURST_VDO_MODE;//

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size=256;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 8;//1010
	params->dsi.vertical_backporch				    = 20;//10 30
	params->dsi.vertical_frontporch				    = 16;//30 30
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			    = 10;//4 60
	params->dsi.horizontal_backporch				= 80;//32 80
	params->dsi.horizontal_frontporch				= 80;//32 80
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK                           = 219;//240; //this value must be in MTK suggested table
	params->dsi.ssc_disable					        = 1;
}

static void lcm_init(void)
{
#if defined(SUPORT_ADC_CHECK)
	int val = 0;
#endif

	MDELAY(200);
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

#if defined(SUPORT_ADC_CHECK)
	val=lcm_read_ADC_value();
    #ifdef BUILD_LK
	    printf("DROI------------------- val:%d\n",val);
    #else
	    printk("DROI------------------- val:%d\n",val);
    #endif
	adc_lcm_push_table(val);
#else
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}


static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	lcm_init();
    //lcm_initialization_setting[14].para_list[0] +=1;
}
static unsigned int lcm_compare_id(void)
{
	int array[4];
	char buffer[5];
	int id=0;

	SET_RESET_PIN(1);
    MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(200);

	push_table(lcm_read_id, sizeof(lcm_read_id) / sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xA1, buffer, 2);

	id = buffer[0]<<8|buffer[1];//we only need ID

	#ifdef BUILD_LK
		printf("st7701 uboot %s,buffer=%x,%x,%x,%x\n", __func__,buffer[0],buffer[1],buffer[2],id);
		printf("%s id = 0x%08x \n", __func__, id);
	#else
		printk("st7701 kernel %s \n", __func__);
		printk("%s id = 0x%08x \n", __func__, id);
	#endif

	return (id == LCM_ID)||(id == 0xFFFF) ? 1 : 0;

}


LCM_DRIVER st7701_dsi_6735_fwvga_drv =
{
    .name			= "ST7701_DSI_6735_FWVGA",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

