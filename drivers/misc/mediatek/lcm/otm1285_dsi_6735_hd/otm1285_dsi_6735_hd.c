#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (720)
#define FRAME_HEIGHT (1280)
#define REGFLAG_DELAY          	0XFE
#define REGFLAG_END_OF_TABLE  	0xFC   // END OF REGISTERS MARKER

#define OTM1285_LCM_ID 									     (0x40)//(0x9806)

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


struct LCM_setting_table {
unsigned char cmd;
unsigned char count;
unsigned char para_list[64];
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


#ifndef TYD_LCD_USE_CUSTOM_OTM1285_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
    //OTM1285_BOE5.0"init
    {0x00,1,{0x00}},
    {0xff,3,{0x12,0x85,0x01}},//EXTC=1
    {0x00,1,{0x80}},//Orise,mode,enable
    {0xff,2,{0x12,0x85}},

    {0x00,1,{0x90}},
    {0xB3,2,{0x0C,0x10}},//4LANE

    {0x00,1,{0x80}},//TCON,Setting,{RTN,
    {0xc0,2,{0x00,0x7F}},

    {0x00,1,{0x82}},//TCON,Setting,{VFP,VBP,
    {0xc0,3,{0x00,0x0c,0x08}},

    {0x00,1,{0x90}},//,Oscillator,Divided,mclk/pclk=6+1=7,{defaul=8+1=9,
    {0xc1,1,{0x55}},

    {0x00,1,{0xB3}},
    {0xc0,2,{0x33,0x40}},//

    {0x00,1,{0x80}},//,Oscillator,idle/norm/pwron/vdo,64.61MHz
    {0xc1,4,{0x1D,0x1D,0x1D,0x1D}},

    {0x00,1,{0x90}},
    {0xc2,10,{0x86,0x20,0x00,0x0F,0x00,0x87,0x20,0x00,0x0F,0x00}},//CKV1/2,shift,switch,width,chop_t1,shift_t2

    {0x00,1,{0xec}},//duty,block,block,width?
    {0xc2,1,{0x00}},

    {0x00,1,{0x80}},//,LTPS,STV,Setting
    {0xc2,4,{0x82,0x01,0x08,0x08}},

    {0x00,1,{0xa0}},
    {0xc0,7,{0x00,0x0f,0x00,0x00,0x0a,0x1A,0x00}},//CKH

    {0x00,1,{0xfb}},//PCH_SET
    {0xc2,1,{0x80}},

    {0x00,1,{0x80}},
    {0xcc,12,{0x03,0x04,0x01,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}},//Step1:SIGIN_SEL,U2D

    {0x00,1,{0xb0}},
    {0xcc,12,{0x04,0x03,0x01,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}},//Step1:{add,SIGIN_SEL,D2U

    {0x00,1,{0x80}},
    {0xcd,15,{0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x15,0x0B,0x0b,0x12,0x13,0x14,0x0b,0x0b,0x0b}},//Step1:SIGIN_SEL

    {0x00,1,{0xc0}},
    {0xcb,15,{0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},//try,Step2:ENMODE,{address,1-27在各個state狀態,

    {0x00,1,{0xd0}},
    {0xcb,15,{0x00,0x00,0x00,0x0A,0x05,0x00,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00}},//Step2:ENMODE,{address,1-27在各個state狀態,

    {0x00,1,{0xe0}},
    {0xcc,4,{0x00,0x00,0x00,0x00}},//Step3:Hi-Z,mask,{若reg_hiz,=,0，則維持STEP2的設定,

    {0x00,1,{0xd0}},
    {0xcd,15,{0x01,0x02,0x04,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}},//Step4:決定address1-27,mapping,到,R_CGOUT1-15

    {0x00,1,{0xe0}},
    {0xcd,12,{0x10,0x11,0x12,0x13,0x03,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b}},//Step4:決定address1-27,mapping,到,R_CGOUT16-27

    {0x00,1,{0xa0}},
    {0xcd,15,{0x01,0x02,0x04,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}},//{add,Step4:決定address1-27,mapping,到,L_CGOUT1-15

    {0x00,1,{0xb0}},
    {0xcd,12,{0x10,0x11,0x12,0x13,0x03,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b}},//{add,Step4:決定address1-27,mapping,到,L_CGOUT16-27

//    ----power,setting----------------------------------------------------------------------------------------------

    {0x00,1,{0x90}},//,power,control,setting,for,normal,mode}},,{set,AVDD,VGL,VGH,AVDD,NAVDD,
    {0xC5,6,{0xB2,0xD6,0xA0,0x0F,0xA0,0x14}},//,AVDD,clamp,at,6V,AVDD=2VCI}},,VGH=4xVCI,VGHS=,8.1V}},VGL_DM=3xVCI,VGLS=,-8V

    {0x00,1,{0x96}},
    {0xc5,4,{0xa0,0x0d,0xaa,0x0c}},
 //   -------------------------------------------------------------------------------------------------------------
    {0x00,1,{0xc2}},
    {0xc5,1,{0x08}},

    {0x00,1,{0x80}},
    {0xc4,1,{0x00}},

    {0x00,1,{0xA8}},
    {0xc4,1,{0x11}},
  //  ----------------------,tunig,default,---------------------------------------------------
    {0x00,1,{0xc1}},
    {0xc5,1,{0x33}},//VDD_18/LVDSVDD=1.6V

    {0x00,1,{0xa3}},
    {0xc5,1,{0x0f}},

    {0x00,1,{0xa5}},
    {0xc5,5,{0x0f,0xa0,0x0d,0xaa,0x0c}},

    {0x00,1,{0x93}},
    {0xc5,1,{0x10}},
    {0x00,1,{0x95}},
    {0xc5,5,{0x11,0xa0,0x0f,0xa0,0x0f}},

    {0x00,1,{0x90}},
    {0xf5,14,{0x03,0x15,0x09,0x15,0x07,0x15,0x0c,0x15,0x0a,0x15,0x09,0x15,0x0a,0x15}},

    {0x00,1,{0xa0}},
    {0xf5,14,{0x12,0x11,0x03,0x15,0x09,0x15,0x11,0x15,0x08,0x15,0x07,0x15,0x09,0x15}},

    {0x00,1,{0xc0}},
    {0xf5,14,{0x0e,0x15,0x0e,0x15,0x00,0x15,0x00,0x15,0x0e,0x15,0x14,0x11,0x00,0x15}},

    {0x00,1,{0xd0}},
    {0xf5,15,{0x07,0x15,0x0a,0x15,0x10,0x11,0x00,0x10,0x90,0x90,0x90,0x02,0x90,0x00,0x00}},

    {0x00,1,{0x00}},//,set,GVDD/NGVDD=+/-4.1V{measure=+/-,
    {0xD8,2,{0x26,0x26}},

    {0x00,1,{0x00}},//
    {0xD9,1,{0x80}},//

    //White,gamma

    {0x00,1,{0x00}},//R+
    {0xE1,24,{0x08,0x1A,0x24,0x2E,0x39,0x3F,0x4B,0x5E,0x67,0x7B,0x88,0x92,0x66,0x5F,0x5A,0x4B,0x38,0x27,0x1B,0x15,0x0E,0x05,0x03,0x00}},
    {0x00,1,{0x00}},//R-
    {0xE2,24,{0x08,0x1A,0x24,0x2E,0x39,0x3F,0x4B,0x5E,0x67,0x7B,0x88,0x92,0x66,0x5F,0x5A,0x4B,0x38,0x27,0x1B,0x15,0x0E,0x05,0x03,0x00}},
    {0x00,1,{0x00}},//G+
    {0xE3,24,{0x08,0x1A,0x24,0x2E,0x39,0x3F,0x4B,0x5E,0x67,0x7B,0x88,0x92,0x66,0x5F,0x5A,0x4B,0x38,0x27,0x1B,0x15,0x0E,0x05,0x03,0x00}},
    {0x00,1,{0x00}},//G-
    {0xE4,24,{0x08,0x1A,0x24,0x2E,0x39,0x3F,0x4B,0x5E,0x67,0x7B,0x88,0x92,0x66,0x5F,0x5A,0x4B,0x38,0x27,0x1B,0x15,0x0E,0x05,0x03,0x00}},
    {0x00,1,{0x00}},//B+
    {0xE5,24,{0x08,0x1A,0x24,0x2E,0x39,0x3F,0x4B,0x5E,0x67,0x7B,0x88,0x92,0x66,0x5F,0x5A,0x4B,0x38,0x27,0x1B,0x15,0x0E,0x05,0x03,0x00}},
    {0x00,1,{0x00}},//B-
    {0xE6,24,{0x08,0x1A,0x24,0x2E,0x39,0x3F,0x4B,0x5E,0x67,0x7B,0x88,0x92,0x66,0x5F,0x5A,0x4B,0x38,0x27,0x1B,0x15,0x0E,0x05,0x03,0x00}},

    //---------------------------------------------------------------------------------------------------------------------------

    {0x00,1,{0x00}},
    {0xff,3,{0x00,0x00,0x00}},

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

		params->dsi.mode   = BURST_VDO_MODE;

		// DSI
		params->dsi.LANE_NUM                = LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.

		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		params->dsi.packet_size=256;

	    // Video mode setting
	    params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		// Video mode setting
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;

		params->dsi.vertical_sync_active				= 2;//2//
		params->dsi.vertical_backporch					= 10;//8
		params->dsi.vertical_frontporch					= 10;//8
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 2;//8
		params->dsi.horizontal_backporch				= 80;//88
		params->dsi.horizontal_frontporch				= 80;//88
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	//params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's
		params->dsi.ssc_disable = 1;
		params->dsi.ssc_range = 0;

        //	params->dsi.HS_PRPR=6;
	//    params->dsi.LPX=8;
		//params->dsi.HS_PRPR=5;
		//params->dsi.HS_TRAIL=13;
		params->dsi.PLL_CLOCK=215; //250 //248
    // Bit rate calculation
	//	//1 Every lane speed
		//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		//params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4
		//params->dsi.fbk_div =8;     // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	18

}

static void lcm_init(void)
{
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
	push_table(lcm_sleep_mode_in_setting, sizeof(lcm_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
	#ifndef BUILD_LK
	printk("OTM1285 lcm_resume 20140825 \n");
	#endif
	//lcm_init();
    push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

static unsigned int lcm_compare_id(void)
{
	    int array[4];
        char buffer[5];
        int id0=0;
        int id=0;
        //Do reset here
        SET_RESET_PIN(1);
        MDELAY(10);
        SET_RESET_PIN(0);
        MDELAY(10);
        SET_RESET_PIN(1);
        MDELAY(120);

        array[0]=0x00023700;//0x00023700;
        dsi_set_cmdq(array, 1, 1);
        read_reg_v2(0xda, buffer,1); //0xda
        id0 = buffer[0]; ///////////////////////0x98
        id = id0;

		#if defined(BUILD_UBOOT) || defined(BUILD_LK)
			printf("OTM1285: %s, id0 = 0x%x \n", __func__, id0);
		#else
			printk("OTM1285: id0= 0x%x\n",id0);
		#endif
		return (OTM1285_LCM_ID == id)?1:0;
}

#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif

// ---------------------------------------------------------------------------
// Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1285_dsi_6735_hd_drv =
{
    	.name		= "OTM1285_DSI_HD",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
};

