/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifdef BUILD_LK
#else
#include <linux/string.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/upmu_common.h>
	#include <platform/upmu_hw.h>
	#include <platform/mt_gpio.h>
	#include <platform/mt_i2c.h>
	#include <platform/mt_pmic.h>
	#include <string.h>
#else
	#include <mt-plat/upmu_common.h>
	#include <mach/upmu_sw.h>
	#include <mach/upmu_hw.h>

	#include <mt-plat/mt_gpio.h>
#endif

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                          (720)
#define FRAME_HEIGHT                                         (1280)

#define REGFLAG_DELAY                                         0xFC
#define REGFLAG_END_OF_TABLE                           0xFD   // END OF REGISTERS MARKER

#define LCM_ID_HX8394D                                      0x8394
#define LCM_DSI_CMD_MODE                                    0

#if 0
//added for lcm detect ,read adc voltage
#define AUXADC_LCM_VOLTAGE_CHANNEL     12
#define MIN_VOLTAGE (600) //600mV
#define MAX_VOLTAGE (1000) //1000mV
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
#endif

//#define HX839D_INIT_METHOD_ONE
#define HX839D_INIT_METHOD_TWO

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
	lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) \
	lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
	lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
	lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

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
    params->dbi.te_mode                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;

    params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;

    // DSI
    /* Command mode setting */
    //Three lane or Four lane
    params->dsi.LANE_NUM                = LCM_FOUR_LANE; //LCM_THREE_LANE
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;

    // Video mode setting
    params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    params->dsi.word_count=720*3;


    params->dsi.vertical_sync_active                  = 4;// 3    2
    params->dsi.vertical_backporch                     = 12;// 20   1
    params->dsi.vertical_frontporch                    = 15; // 1  12
    params->dsi.vertical_active_line                   = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active              = 16;// 50  2
    params->dsi.horizontal_backporch                = 67;  //72
    params->dsi.horizontal_frontporch               = 69 ;  //72
    params->dsi.horizontal_active_pixel             = FRAME_WIDTH;

    params->dsi.esd_check_enable = 0;
    params->dsi.customization_esd_check_enable = 0;
    params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
    params->dsi.lcm_esd_check_table[0].count        = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x1C;

    //params->dsi.LPX=8;

    // Bit rate calculation
    //Every lane speed
    //params->dsi.pll_select=1;
    params->dsi.PLL_CLOCK = 195; //210 239 229 240 230this value must be in MTK suggested table
    params->dsi.ssc_disable = 1;
    params->dsi.compatibility_for_nvk = 1;
}

#ifdef HX839D_INIT_METHOD_ONE
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

static struct LCM_setting_table lcm_initialization_setting[] = {
    {0xB9,3,{0xFF, 0x83, 0x94}},

    {0xBA,2,{0x33,0x83}},

    {0xB1,15,{0x6C,0x0A,0x0A,0x24,0x04,0x11,0xF1,0x80,0xE9,0xd5,0x23,0x80,0xc0,0xd2,0x58}},

    {0xB2,11,{0x00,0x64,0x05,0x07,0x22,0x1C,0x08,0x08,0x1C,0x4D,0x00}},

    {0xB4,12,{0x00,0xFF,0x03,0x5A,0x03,0x5A,0x03,0x5A,0x01,0x70,0x01,0x70}},

    {0xBC,1,{0x07}},

    {0xBF,3,{0x41,0x0E,0x01}},

    {0xE0,42,{0x20,0x25,0x14,0x2A,0x2C,0x3F,0x1F,0x3E,0x07,0x0B,0x0D,0x18,0x0E,0x11,0x14,0x13,0x13,0x08,0x14,0x14,0x18,0x00,0x25,0x14,0x2B,0x2E,0x3F,0x1F,0x3C,0x07,0x0B,0x0D,0x18,0x0E,0x11,0x13,0x11,0x13,0x08,0x12,0x14,0x18}},

    {0xD3,30,{0x00,0x06,0x00,0x01,0x01,0x10,0x00,0x32,0x10,0x00,0x00,0x00,0x32,0x15,0x04,0x35,0x04,0x32,0x15,0x14,0x05,0x14,0x37,0x33,0x00,0x00,0x37,0x00,0x07,0x37}},

    {0xD5,44,{0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x25,0x24,0x27,0x26,0x18,0x18,0x47,0x46,0x43,0x42,0x41,0x40,0x45,0x44,0x05,0x04,0x01,0x00,0x07,0x06,0x03,0x02,0x21,0x20,0x23,0x22,0x18,0x18,0x18,0x18}},

    {0xD6,44,{0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x22,0x23,0x20,0x21,0x18,0x18,0x00,0x01,0x04,0x05,0x06,0x07,0x02,0x03,0x02,0x03,0x06,0x07,0x00,0x01,0x04,0x05,0x06,0x07,0x04,0x05,0x18,0x18,0x18,0x18}},

    {0xCC,  1 ,{0x0B}},

    {0xB6,2,{0x3c,0x3c}},

    {0xC0,2,{0x30,0x14}},

    {0xC7,4,{0x00,0xC0,0x40,0xC0}},

    {0x36,  1 ,{0x00}},

    {0x21,  1 ,{0x00}},

    {0x11, 0,{}},
    {REGFLAG_DELAY, 150, {}},

    {0x29, 0,{}},
    {REGFLAG_DELAY, 30, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

#ifdef HX839D_INIT_METHOD_TWO
static void init_lcm_registers(void)
{
    unsigned int data_array[16];

    data_array[0]=0x00043902;
    data_array[1]=0x9483FFB9;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]=0x00033902;
    data_array[1]=0x008333BA;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]=0x000B3902;
    data_array[1]=0x0A0A6CB1;
    data_array[2]=0xF1110424;
    data_array[3]=0x00D5E980;
    //data_array[4]=0x58D2C080;
    dsi_set_cmdq(data_array, 4, 1);
    MDELAY(10);

    data_array[0]=0x000C3902;
    data_array[1]=0x056400B2;
    data_array[2]=0x081C2207;
    data_array[3]=0x004D1C08;
    dsi_set_cmdq(data_array, 4, 1);
    MDELAY(1);

    data_array[0]=0x000D3902;
    data_array[1]=0x03FF00B4;
    data_array[2]=0x035A035A;
    data_array[3]=0x0170015A;
    data_array[4]=0x00000070;
    dsi_set_cmdq(data_array, 5, 1);
    MDELAY(1);

    data_array[0]=0x00023902;
    data_array[1]=0x000007BC;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]=0x00043902;
    data_array[1]=0x010E41BF;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);
 /*
    data_array[0]=0x002B3902;
    data_array[1]=0x272620E0;
    data_array[2]=0x30383534;
    data_array[3]=0x0D0E0749;
    data_array[4]=0x15120F17;
    data_array[5]=0x11071513;
    data_array[6]=0x1C101713;
    data_array[7]=0x3A353425;
    data_array[8]=0x09084930;
    data_array[9]=0x120F170C;
    data_array[10]=0x07151315;
    data_array[11]=0x00181311;
    dsi_set_cmdq(data_array, 12, 1);
    MDELAY(1);
    */
    data_array[0]=0x002B3902;
    data_array[1]=0x212320E0;
    data_array[2]=0x1A3F312B;
    data_array[3]=0x0A070537;
    data_array[4]=0x110E0C15;
    data_array[5]=0x13081110;
    data_array[6]=0x23201914;
    data_array[7]=0x3F312B21;
    data_array[8]=0x0705371A;
    data_array[9]=0x0E0C150A;
    data_array[10]=0x08111011;
    data_array[11]=0x00191413;
    dsi_set_cmdq(data_array, 12, 1);
    MDELAY(1);


    data_array[0]=0x001F3902;
    data_array[1]=0x000600D3;
    data_array[2]=0x00100101;
    data_array[3]=0x00001032;
    data_array[4]=0x04153200;
    data_array[5]=0x15320435;
    data_array[6]=0x37140514;
    data_array[7]=0x37000033;
    data_array[8]=0x00370700;
    dsi_set_cmdq(data_array, 9, 1);
    MDELAY(1);

    data_array[0]=0x002D3902;
    data_array[1]=0x181818D5;
    data_array[2]=0x18181818;
    data_array[3]=0x18181818;
    data_array[4]=0x25181818;
    data_array[5]=0x18262724;
    data_array[6]=0x43464718;
    data_array[7]=0x45404142;
    data_array[8]=0x01040544;
    data_array[9]=0x03060700;
    data_array[10]=0x23202102;
    data_array[11]=0x18181822;
    data_array[12]=0x00000018;
    dsi_set_cmdq(data_array, 13, 1);
    MDELAY(1);

    data_array[0]=0x002D3902;
    data_array[1]=0x181818D6;
    data_array[2]=0x18181818;
    data_array[3]=0x18181818;
    data_array[4]=0x22181818;
    data_array[5]=0x18212023;
    data_array[6]=0x04010018;
    data_array[7]=0x02070605;
    data_array[8]=0x06030203;
    data_array[9]=0x04010007;
    data_array[10]=0x04070605;
    data_array[11]=0x18181805;
    data_array[12]=0x00000018;
    dsi_set_cmdq(data_array, 13, 1);
    MDELAY(1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000BCC;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]=0x00033902;
    data_array[1]=0x003535B6;  // shui bo wen  //larger or smaller
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]=0x00033902;
    data_array[1]=0x001430C0;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]=0x00053902;
    data_array[1]=0x40C000C7;
    data_array[2]=0x000000C0;
    dsi_set_cmdq(data_array, 3, 1);
    MDELAY(10);

    data_array[0]=0x00023902;
    data_array[1]=0x00000036;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000021;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    data_array[0]= 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(200);

    data_array[0]= 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(10);
}
#endif

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);

    #ifdef HX839D_INIT_METHOD_ONE
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    #endif

    #ifdef HX839D_INIT_METHOD_TWO
    init_lcm_registers();
    #endif
}


static void lcm_suspend(void)
{
    unsigned int data_array[16];



    data_array[0]=0x00280500; // Display Off
    dsi_set_cmdq(data_array, 1, 1);

    data_array[0] = 0x00100500; // Sleep In
    dsi_set_cmdq(data_array, 1, 1);

     SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(20);

    SET_RESET_PIN(1);
    MDELAY(120);
}


static void lcm_resume(void)
{
    lcm_init();
}

#if 0
//added for lcm detect ,read adc voltage
static unsigned int lcm_compare_id_auxadc(void)
{
    int data[4] = {0,0,0,0};
    int res = 0;
    int rawdata = 0;
    int lcm_vol = 0;

    res = IMM_GetOneChannelValue(AUXADC_LCM_VOLTAGE_CHANNEL,data,&rawdata);
    if(res < 0)
    {
        #ifdef BUILD_LK
        printf("cgs hx8394d_hd720_dsi_vdo_yizhiming: get adc data error\n");
        #endif

        return 0;
    }

    lcm_vol = data[0]*1000+data[1]*10;

    #ifdef BUILD_LK
    printf("cgs hx8394d_hd720_dsi_vdo_yizhiming:lcm_vol=%d\n",lcm_vol);
    #endif

    if (lcm_vol >= MIN_VOLTAGE && lcm_vol <= MAX_VOLTAGE)
    {
        #ifdef BUILD_LK
        printf("cgs hx8394d_hd720_dsi_vdo_yizhiming return 1.\n");
        #endif
        return 1;
    }
    #ifdef BUILD_LK
    printf("cgs hx8394d_hd720_dsi_vdo_yizhiming return 0.\n");
    #endif
    return 0;

}
#endif

static unsigned int lcm_compare_id(void)
{
    //unsigned int id=0;
    unsigned char buffer[3];
    unsigned int array[16];
    unsigned int data_array[16];
    //unsigned int lcd_auxadc_id = 0;

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);

    SET_RESET_PIN(1);
    MDELAY(80);

    #if 0
    lcd_auxadc_id = lcm_compare_id_auxadc();
    #ifdef BUILD_LK
    printf("cgs lk hx8394d_hd720_dsi_vdo_yizhiming lcd_auxadc_id=%d\n",lcd_auxadc_id);
    #else
    printk("cgs kernel hx8394d_hd720_dsi_vdo_yizhiming lcd_auxadc_id=%d\n",lcd_auxadc_id);
    #endif
    #endif

    data_array[0] = 0x00043902;
    data_array[1] = 0x9483ffb9;
    dsi_set_cmdq(data_array, 2, 1);

    array[0] = 0x00033700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);

    read_reg_v2(0x04, buffer, 3);//buffer0-3   --> 0x83940d

    #ifdef BUILD_LK
    printf("cgs lk hx8394d_hd720_dsi_vdo_yizhimingbuffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n",buffer[0],buffer[1],buffer[2]);
    #else
    printk("cgs kernel hx8394d_hd720_dsi_vdo_yizhimingbuffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n",buffer[0],buffer[1],buffer[2]);
    #endif

    if(buffer[0]==0x83 && buffer[1]==0x94 /*&& lcd_auxadc_id == 1*/)
    {
        #ifdef BUILD_LK
        printf("%s,lk hx8394d_hd720_dsi_vdo_yizhiming,read lcd id success,return 1\n", __func__);
        #else
        printk("%s,kernel hx8394d_hd720_dsi_vdo_yizhiming,read lcd id success,return 1\n", __func__);
        #endif
        return 1;
    }
    else
    {
        #ifdef BUILD_LK
        printf("%s,lk hx8394d_hd720_dsi_vdo_yizhiming id,read lcd id fail,return 0\n", __func__);
        #else
        printk("%s,kernel hx8394d_hd720_dsi_vdo_yizhiming,read lcd id fail,return 0\n", __func__);
        #endif
        return 0;
    }
}


#if 0
static int err_count = 0;
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    unsigned char buffer[8] = {0};
    unsigned int array[4];
    int i =0;

    array[0] = 0x00013700;
    dsi_set_cmdq(array, 1,1);
    read_reg_v2(0x0A, buffer,8);

    printk( "nt35521_JDI lcm_esd_check: buffer[0] = %d,buffer[1] = %d,buffer[2] = %d,buffer[3] = %d,buffer[4] = %d,buffer[5] = %d,buffer[6] = %d,buffer[7] = %d\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);

    if((buffer[0] != 0x9C))/*LCD work status error,need re-initalize*/
    {
        printk( "nt35521_JDI lcm_esd_check buffer[0] = %d\n",buffer[0]);
        return TRUE;
    }
    else
    {
        if(buffer[3] != 0x02) //error data type is 0x02
        {
             //return FALSE;
        err_count = 0;
        }
        else
        {
             //if(((buffer[4] != 0) && (buffer[4] != 0x40)) ||  (buffer[5] != 0x80))
        if( (buffer[4] == 0x40) || (buffer[5] == 0x80))
             {
                  err_count = 0;
             }
             else
             {
                  err_count++;
             }
             if(err_count >=2 )
             {
                 err_count = 0;
                 printk( "nt35521_JDI lcm_esd_check buffer[4] = %d , buffer[5] = %d\n",buffer[4],buffer[5]);
                 return TRUE;
             }
        }
        return FALSE;
    }
#endif

}
static unsigned int lcm_esd_recover(void)
{
    lcm_init();

    return TRUE;
}
#endif

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8394d_dsi_6735_hd_drv =
{
    .name            = "hx8394d_dsi_6735_hd",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id    = lcm_compare_id,
    #if 0//defined(LCM_DSI_CMD_MODE)
    //    .set_backlight    = lcm_setbacklight,
    //.set_pwm        = lcm_setpwm,
    //.get_pwm        = lcm_getpwm,
    .update         = lcm_update
    #endif
};


