/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#ifdef BUILD_LK
#include <string.h>
#endif
#include "lcm_drv.h"



#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             								0xFC
#define REGFLAG_END_OF_TABLE      							0xFD   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
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

//static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);

#if defined(SUPORT_ADC_CHECK)
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
	else if((val>600)&&(val<1000))             //voltage=1.2V
	{
	    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{
	}
}
#endif

#if 0
//DROI_LCD_USE_CUSTOM_HX8394_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
 
    {0xB9,3,{0xFF,0x83,0x94}},
    
    {0xBA,17,{0x13,0x83,0x00,0x16,0xa6,0x11,0x08,0xff,0x0f,0x24,0x03,0x21,0x24,0x25,0x20,0x02,0x10}},
    
    {0xB1,15,{0x7C,0x00,0x24,0x09,0x01,0x11,0x11,0x36,0x3e,0x26,0x26,0x57,0x02,0x01,0xe6}},
    
    {0xB4,18,{0x00,0x00,0x00,0x05,0x06,0x41,0x42,0x02,0x41,0x42,0x43,0x47,0x19,0x58,0x60,0x08,0x85,0x10}},
    
    {0xD5,24,{0x4c,0x01,0x07,0x01,0xcd,0x23,0xef,0x45,0x67,0x89,0xab,0x11,0x00,0xdc,0x10,0xfe,0x32,0xba,0x98,0x76,0x54,0x00,0x11,0x40}},
    
    {0xB2,6,{0x0f,0xc8,0x04,0x04,0x00,0x81}},
    
    {0xB6,1,{0x2a}},
    
    {0xc7,2,{0x00,0x30}},
    
    {0xcc,1,{0x09}},
    
    {0xe0,34,{0x24,0x33,0x36,0x3F,0x3f,0x3f,0x3c,0x56,0x05,0x0c,0x0e,0x11,0x13,0x12,0x14,0x12,0x1e,0x24,0x33,0x36,0x3F,0x3f,0x3f,0x3c,0x56,0x05,0x0c,0x0e,0x11,0x13,0x12,0x14,0x12,
    0x1e}},
    
    {0xc1,127,
    {0x01,0x00,0x07,0x10,0x16,0x1B,0x25,0x2C,0x34,0x38,0x44,0x4B,0x52,0x5B,0x61,0x6A,0x71,0x79,0x82,0x89,0x91,0x9B,0xA4,0xAC,0xB5,0xBD,0xC6,0xCD,0xD6,
    0xDD,0xE4,0xEC,0xF4,0xFB,0x05,0xEA,0xC9,0xB0,0x02,0x5F,0xFD,0x73,0xC0,0x00,0x03,0x0E,0x14,0x19,0x20,0x28,0x2F,0x33,0x3D,0x43,0x4A,0x52,0x57,0x60,0x66,
    0x6E,0x75,0x7C,0x83,0x8B,0x93,0x9B,0xA3,0xAA,0xB3,0xBA,0xC2,0xC9,0xCF,0xD7,0xDE,0xE5,0x05,0xEA,0xC9,0xB1,0x02,0x5F,0xFD,0x73,0xC0,0x00,0x01,0x0F,0x16,
    0x1C,0x26,0x2E,0x36,0x3B,0x47,0x4E,0x56,0x5F,0x65,0x6F,0x76,0x7F,0x88,0x90,0x99,0xA2,0xAC,0xB4,0xBD,0xC5,0xCD,0xD5,0xDE,0xE5,0xEB,0xF4,0xFA,0xFF,0x05,
    0xEA,0xC9,0xB1,0x02,0x5F,0xFD,0x73,0xC0}},
    
    {0x53,1,{0x24}},
    
    {0x55,1,{0x00}},
    
    {0xbf,2,{0x06,0x10}},
    
    {0x11,1,{0x00}},
    {REGFLAG_DELAY, 120, {}},
    {0x29,1,{0x00}},
    {REGFLAG_DELAY, 100, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    //Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


//static int vcom = 0x2F;
#if 0
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
			
#if 0	
      	    case 0xBE:
		    table[i].para_list[0]=vcom;
		    dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
            vcom+=3;
            break;			
#endif

            default:
				
		dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
                break;
        }
    }
}
#endif
#endif

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

    params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;
        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
    params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
        #endif
#if defined(DROI_PRO_F5C62W_SGDZ_3)||defined(DROI_PRO_F5C_SGDZ3)

		params->dsi.ssc_disable = 1;
params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable 	= 1;
		params->dsi.lcm_esd_check_table[0].cmd          = 0x09;
		params->dsi.lcm_esd_check_table[0].count        = 3;
		params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
		params->dsi.lcm_esd_check_table[0].para_list[1] = 0x73;
		params->dsi.lcm_esd_check_table[0].para_list[2] = 0x04;//04

		params->dsi.lcm_esd_check_table[1].cmd          = 0xd9;
		params->dsi.lcm_esd_check_table[1].count        = 1;
		params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;
		
		params->dsi.lcm_esd_check_table[2].cmd          = 0x45;
		params->dsi.lcm_esd_check_table[2].count        = 2;
		params->dsi.lcm_esd_check_table[2].para_list[0] = 0x05;
		params->dsi.lcm_esd_check_table[2].para_list[1] = 0x10;
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

    params->dsi.packet_size=256;
		// Video mode setting		
    params->dsi.intermediat_buffer_num = 2;
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577

	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 16;
	params->dsi.vertical_frontporch					= 11; 
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 60;
	params->dsi.horizontal_backporch				= 60;
	params->dsi.horizontal_frontporch				= 60;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8; 

		// Bit rate calculation
    params->dsi.PLL_CLOCK=208;
		//1 Every lane speed
}


#ifndef DROI_LCD_USE_CUSTOM_HX8394A_HD
static void init_lcm_registers(void){
unsigned int data_array[16];	
	//JD9361+CPT5.5 IPS DA TONG 20160802
#if defined(DROI_PRO_F5C62W_SGDZ_3)||defined(DROI_PRO_F5C_SGDZ3)

data_array[0] = 0x00043902;
data_array[1] = 0x9483FFB9;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x008373BA;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x11116AB1;
data_array[2] = 0xF1110413;
data_array[3] = 0x23942881;
data_array[4] = 0x58D2C080;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x000C3902;
data_array[1] = 0x106400B2;
data_array[2] = 0x081C1207;
data_array[3] = 0x004D1C08;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);

data_array[0] = 0x000D3902;
data_array[1] = 0x03FF00B4;
data_array[2] = 0x035A035A;
data_array[3] = 0x016A015A;
data_array[4] = 0x0000006A;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x00023902;
data_array[1] = 0x000007BC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0x010E41BF;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00023902;
data_array[1] = 0x000011D2;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00213902;
data_array[1] = 0x000F00D3;
data_array[2] = 0x00081A40;
data_array[3] = 0x00081032;
data_array[4] = 0x0F155408;
data_array[5] = 0x12020405;
data_array[6] = 0x33070510;
data_array[7] = 0x370C0C34;
data_array[8] = 0x11170707;
data_array[9] = 0x00000008;
dsi_set_cmdq(data_array, 10, 1);
MDELAY(1);

data_array[0] = 0x002D3902;
data_array[1] = 0x181919D5;
data_array[2] = 0x1A1B1B18;
data_array[3] = 0x0605041A;
data_array[4] = 0x02010007;
data_array[5] = 0x18212003;
data_array[6] = 0x18232218;
data_array[7] = 0x18181818;
data_array[8] = 0x18181818;
data_array[9] = 0x18181818;
data_array[10] = 0x18181818;
data_array[11] = 0x18181818;
data_array[12] = 0x00000018;
dsi_set_cmdq(data_array, 13, 1);
MDELAY(1);

data_array[0] = 0x002D3902;
data_array[1] = 0x191818D6;
data_array[2] = 0x1A1B1B19;
data_array[3] = 0x0102031A;
data_array[4] = 0x05060700;
data_array[5] = 0x18222304;
data_array[6] = 0x18202118;
data_array[7] = 0x18181818;
data_array[8] = 0x18181818;
data_array[9] = 0x18181818;
data_array[10] = 0x18181818;
data_array[11] = 0x18181818;
data_array[12] = 0x00000018;
dsi_set_cmdq(data_array, 13, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00AAAAB6;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x002B3902;
data_array[1] = 0x020000E0;
data_array[2] = 0x0D3F332D;
data_array[3] = 0x0B090632;
data_array[4] = 0x12100D16;
data_array[5] = 0x10061310;
data_array[6] = 0x00001612;
data_array[7] = 0x3F332D02;
data_array[8] = 0x0906320D;
data_array[9] = 0x100D160B;
data_array[10] = 0x06131012;
data_array[11] = 0x00161210;
dsi_set_cmdq(data_array, 12, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001430C0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x00C000C7;
data_array[2] = 0x000000C0;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00023902;
data_array[1] = 0x000005CC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00023902;
data_array[1] = 0x000088DF;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00110500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(150);

data_array[0] = 0x00290500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(20);

#else	
	
data_array[0]=0x00110500;
dsi_set_cmdq(data_array,1, 1);
MDELAY(200);

data_array[0]=0x00043902;
data_array[1]=0x9483FFB9;
dsi_set_cmdq(data_array,2, 1);

data_array[0]=0x00123902;
data_array[1]=0x008313BA;
data_array[2]=0x0811A616;
data_array[3]=0x03240FFF;
data_array[4]=0x20252421;
data_array[5]=0x00001002;
dsi_set_cmdq(data_array,6, 1);

data_array[0]=0x00103902;
data_array[1]=0x24007CB1;//24
data_array[2]=0x11110109;//01
data_array[3]=0x26263E36;
data_array[4]=0xE6010257;
dsi_set_cmdq(data_array,5, 1);

data_array[0]=0x00133902;
data_array[1]=0x000000B4;
data_array[2]=0x42410605;
data_array[3]=0x43424102;
data_array[4]=0x60581947;
data_array[5]=0x00108508;
dsi_set_cmdq(data_array,6, 1);

data_array[0]=0x00193902;
data_array[1]=0x07014CD5;
data_array[2]=0xEF23CD01;
data_array[3]=0xAB896745;
data_array[4]=0x10DC0011;
data_array[5]=0x98BA32FE;
data_array[6]=0x11005476;
data_array[7]=0x00000040;
dsi_set_cmdq(data_array,8, 1);
MDELAY(5);

data_array[0]=0x00073902;
data_array[1]=0x04C80FB2;
data_array[2]=0x00810004;
dsi_set_cmdq(data_array,3, 1);

data_array[0]=0x00023902;
data_array[1]=0x00002aB6;//2a
dsi_set_cmdq(data_array,2, 1);

data_array[0]=0x00033902;
data_array[1]=0x800004C6;
dsi_set_cmdq(data_array,2, 1);


data_array[0]=0x00033902;
data_array[1]=0x003000C7;
dsi_set_cmdq(data_array,2, 1);

data_array[0]=0x00023902;
data_array[1]=0x000009CC;
dsi_set_cmdq(data_array,2, 1);

data_array[0]=0x00233902;
data_array[1]=0x363324E0;
data_array[2]=0x3C3F3F3F;
data_array[3]=0x0E0C0556;
data_array[4]=0x14121311;
data_array[5]=0x33241E12;
data_array[6]=0x3F3F3F36;
data_array[7]=0x0C05563C;
data_array[8]=0x1213110E;
data_array[9]=0x001E1214;
dsi_set_cmdq(data_array,10, 1);
MDELAY(5);
data_array[0]=0x00033902;
data_array[1]=0x001006BF;
dsi_set_cmdq(data_array,2, 1);
data_array[0]=0x00290500;
dsi_set_cmdq(data_array,1, 1);
MDELAY(50);
#endif
}
#endif


static void lcm_init(void)
{
#if defined(SUPORT_ADC_CHECK)
	int val = 0;
#endif
	SET_RESET_PIN(1);
    MDELAY(10);
	SET_RESET_PIN(0);
    MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(120); 

#if defined(SUPORT_ADC_CHECK)
	val=lcm_read_ADC_value();
#ifdef BUILD_LK
	printf("TYD------------------- val:%d\n",val);
#else
	printk("TYD------------------- val:%d\n",val);
#endif
	adc_lcm_push_table(val);
#else
	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    init_lcm_registers();

#endif	
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	
	data_array[0] = 0x00280500;//			
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(50);
	
	data_array[0] = 0x00100500;   
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
	
	SET_RESET_PIN(1);	
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(30);
	SET_RESET_PIN(1);
	MDELAY(120);     

}

//static unsigned int vcom=0x70;
static void lcm_resume(void)
{
	lcm_init();
#if 0
	 vcom+=2;
    lcm_initialization_setting_0800[22].para_list[0]=vcom;
    lcm_initialization_setting_0800[23].para_list[0]=vcom;
    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1); 
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

	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];
    unsigned char buffer1[2],buffer2[2],buffer3[2];

	SET_RESET_PIN(1);
    MDELAY(10);
	SET_RESET_PIN(0);
    MDELAY(50);
	SET_RESET_PIN(1);
    MDELAY(120);//Must over 6 ms
	array[0]=0x00043902;
	array[1]=0x9483FFB9;// page enable
	dsi_set_cmdq(array, 2, 1);
	MDELAY(10);

	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; 
	read_reg_v2(0xda, buffer1, 1);
	read_reg_v2(0xdb, buffer2, 1);
	read_reg_v2(0xdc, buffer3, 1);
#if defined(BUILD_LK)
	printf("%s, id = 0x%08x\n", __func__, id);
#else
printk("%s, da = 0x%08x\n", __func__, buffer1[0]);
printk("%s, db = 0x%08x\n", __func__, buffer2[0]);
printk("%s, dc = 0x%08x\n", __func__, buffer3[0]);
#endif	

	return (0x94 == id)?1:0;

}

LCM_DRIVER hx8394a_dsi_6735_hd_drv = 
{
    .name			= "hx8394_hd",
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
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};
