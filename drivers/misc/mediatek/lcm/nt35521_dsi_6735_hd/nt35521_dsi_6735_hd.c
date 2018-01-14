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


const static unsigned char LCD_MODULE_ID = 0x09;//ID0->1;ID1->X
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

//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5C_XDNZ)||defined(DROI_PRO_FQ5C_ZG)
int lcm_flag = 0;
#endif
//end

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

#if defined(DROI_PRO_FQ5C)
#include "nt35521_dsi_6735_hd_fq5c.h"
#endif

#if defined(DROI_PRO_FQ5CW)
#include "nt35521_dsi_6735_hd_fq5cw.h"
#endif

#if defined(DROI_PRO_FQ5B)
#include "nt35521_dsi_6735_hd_fq5b.h"
#endif

#if defined(DROI_PRO_F6)
#include "nt35521_dsi_6735_hd_f6.h"
#endif

#if defined(DROI_PRO_F5C)
#include "nt35521_dsi_6735_hd_f5c.h"
#endif

#if defined(DROI_PRO_PF5T)
#include "nt35521_dsi_6735_hd_pf5t.h"
#endif

#if defined(DROI_PRO_PF5)
#include "nt35521_dsi_6735_hd_pf5.h"
#endif

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
#if defined(DROI_PRO_FQ5C_XWP)
	if((val>1000)&&(val<1400))             //voltage=1.2V
	{
	    push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000))         //voltage=0.8V
	{
	    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>200)&&(val<600))         //voltage=0.4V
	{
	    push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{

     }
#else
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
#endif
}
#endif


#ifndef TYD_LCD_USE_CUSTOM_NT35521_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
	//#NT35521 Initial code For INL(群创玻璃)5.0\EF\BC?0141228   oncell
	{0xFF, 4,{0xAA,0x55,0xA5,0x80}},
	{0x6F, 2,{0x11,0x00}},
	{0xF7, 2,{0x20,0x00}},
	{0x6F, 1,{0x06}},
	{0xF7, 1,{0xA0}},
	{0x6F, 1,{0x19}},
	{0xF7, 1,{0x12}},

	{0xF0, 5,{0x55,0xAA,0x52,0x08,0x00}},
	{0xC8, 1,{0x80}},
	{0xB1, 2,{0x6C,0x21}},
	{0xB6, 1,{0x08}},
	{0x6F, 1,{0x02}},
	{0xB8, 1,{0x08}},
	{0xBB, 2,{0x74,0x44}},
	{0xBC, 2,{0x00,0x00}},
	{0xBD, 5,{0x01,0xAC,0x10,0x10,0x01}},

	{0xF0, 5,{0x55,0xAA,0x52,0x08,0x01}},
	{0xB0, 2,{0x05,0x05}},
	{0xB1, 2,{0x05,0x05}},
	{0xBC, 2,{0xA0,0x01}},
	{0xBD, 2,{0xA0,0x01}},

	{0xCA, 1,{0x00}},
	{0xC0, 1,{0x0C}},
	{0xBE, 1,{0x5A}},
	{0xB3, 2,{0x37,0x37}},
	{0xB4, 2,{0x0F,0x0F}},
	{0xB9, 2,{0x46,0x46}},
	{0xBA, 2,{0x14,0x14}},

	{0xF0, 5,{0x55,0xAA,0x52,0x08,0x02}},
	{0xEE, 1,{0x01}},
	{0xEF, 4,{0x09,0x06,0x15,0x18}},

	{0xB0,16,{0x00,0x00,0x00,0x2A,0x00,0x4F,0x00,0x68,0x00,0x80,0x00,0xA7,0x00,0xD4,0x01,0x10}},
	{0xB1,16,{0x01,0x3E,0x01,0x83,0x01,0xBB,0x02,0x10,0x02,0x55,0x02,0x57,0x02,0x96,0x02,0xDC}},
	{0xB2,16,{0x03,0x08,0x03,0x47,0x03,0x72,0x03,0x9D,0x03,0xB5,0x03,0xD4,0x03,0xE3,0x03,0xF4}},
	{0xB3, 4,{0x03,0xFD,0x03,0xFF}},

	{0xF0, 5,{0x55,0xAA,0x52,0x08,0x06}},
	{0xB0, 2,{0x29,0x2A}},
	{0xB1, 2,{0x10,0x12}},
	{0xB2, 2,{0x14,0x16}},
	{0xB3, 2,{0x18,0x1A}},
	{0xB4, 2,{0x08,0x0A}},
	{0xB5, 2,{0x2E,0x2E}},
	{0xB6, 2,{0x2E,0x2E}},
	{0xB7, 2,{0x2E,0x2E}},
	{0xB8, 2,{0x2E,0x00}},
	{0xB9, 2,{0x2E,0x2E}},
	{0xBA, 2,{0x2E,0x2E}},
	{0xBB, 2,{0x01,0x2E}},
	{0xBC, 2,{0x2E,0x2E}},
	{0xBD, 2,{0x2E,0x2E}},
	{0xBE, 2,{0x2E,0x2E}},
	{0xBF, 2,{0x0B,0x09}},
	{0xC0, 2,{0x1B,0x19}},
	{0xC1, 2,{0x17,0x15}},
	{0xC2, 2,{0x13,0x11}},
	{0xC3, 2,{0x2A,0x29}},
	{0xE5, 2,{0x2E,0x2E}},
	{0xC4, 2,{0x29,0x2A}},
	{0xC5, 2,{0x1B,0x19}},
	{0xC6, 2,{0x17,0x15}},
	{0xC7, 2,{0x13,0x11}},
	{0xC8, 2,{0x01,0x0B}},
	{0xC9, 2,{0x2E,0x2E}},
	{0xCA, 2,{0x2E,0x2E}},
	{0xCB, 2,{0x2E,0x2E}},
	{0xCC, 2,{0x2E,0x09}},
	{0xCD, 2,{0x2E,0x2E}},
	{0xCE, 2,{0x2E,0x2E}},
	{0xCF, 2,{0x08,0x2E}},
	{0xD0, 2,{0x2E,0x2E}},
	{0xD1, 2,{0x2E,0x2E}},
	{0xD2, 2,{0x2E,0x2E}},
	{0xD3, 2,{0x0A,0x00}},
	{0xD4, 2,{0x10,0x12}},
	{0xD5, 2,{0x14,0x16}},
	{0xD6, 2,{0x18,0x1A}},
	{0xD7, 2,{0x2A,0x29}},
	{0xE6, 2,{0x2E,0x2E}},
	{0xD8, 5,{0x00,0x00,0x00,0x00,0x00}},
	{0xD9, 5,{0x00,0x00,0x00,0x00,0x00}},
	{0xE7, 1,{0x00}},

	{0xF0, 5,{0x55,0xAA,0x52,0x08,0x03}},
	{0xB0, 2,{0x00,0x00}},
	{0xB1, 2,{0x00,0x00}},
	{0xB2, 5,{0x05,0x00,0x00,0x00,0x00}},

	{0xB6, 5,{0x05,0x00,0x00,0x00,0x00}},
	{0xB7, 5,{0x05,0x00,0x00,0x00,0x00}},

	{0xBA, 5,{0x57,0x00,0x00,0x00,0x00}},
	{0xBB, 5,{0x57,0x00,0x00,0x00,0x00}},

	{0xC0, 4,{0x00,0x00,0x00,0x00}},
	{0xC1, 4,{0x00,0x00,0x00,0x00}},

	{0xC4, 1,{0x60}},
	{0xC5, 1,{0x40}},

	{0xF0, 5,{0x55,0xAA,0x52,0x08,0x05}},
	{0xBD, 5,{0x03,0x01,0x03,0x03,0x03}},
	{0xB0, 2,{0x17,0x06}},
	{0xB1, 2,{0x17,0x06}},
	{0xB2, 2,{0x17,0x06}},
	{0xB3, 2,{0x17,0x06}},
	{0xB4, 2,{0x17,0x06}},
	{0xB5, 2,{0x17,0x06}},

	{0xB8, 1,{0x00}},
	{0xB9, 1,{0x00}},
	{0xBA, 1,{0x00}},
	{0xBB, 1,{0x02}},
	{0xBC, 1,{0x00}},

	{0xC0, 1,{0x07}},

	{0xC4, 1,{0x80}},
	{0xC5, 1,{0xA4}},

	{0xC8, 2,{0x05,0x30}},
	{0xC9, 2,{0x01,0x31}},

	{0xCC, 3,{0x00,0x00,0x3C}},
	{0xCD, 3,{0x00,0x00,0x3C}},

	{0xD1, 5,{0x00,0x05,0x09,0x07,0x10}},
	{0xD2, 5,{0x00,0x05,0x0E,0x07,0x10}},

	{0xE5, 1,{0x06}},
	{0xE6, 1,{0x06}},
	{0xE7, 1,{0x06}},
	{0xE8, 1,{0x06}},
	{0xE9, 1,{0x06}},
	{0xEA, 1,{0x06}},

	{0xED, 1,{0x30}},

	{0x6F, 1,{0x11}},
	{0xF3, 1,{0x01}},

	{0x35, 1,{0x00}},
	#if 0
	{0xF0,5,{ 0x55,0xAA,0x52,0x08,0x00}},
	{0xee,4,{ 0x87,0x78,0x02,0x40}},
	{0xef,2,{ 0x07,0xff}},
	#else
	{0x11,	1,	{0x00}},
	{REGFLAG_DELAY, 120, {}},

	{0x29,	1,	{0x00}},
	{REGFLAG_DELAY, 20, {}},
	#endif
	{REGFLAG_END_OF_TABLE, 0x00, {}},
};
#endif
/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    //Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/
#if defined(DROI_PRO_FQ5C_XWP)||defined(DROI_PRO_FQ5C_ZG)
#else
static struct LCM_setting_table lcm_sleep_in_setting[] =
{
    {0x28, 0, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    // Sleep Mode On
    {0x10, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
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
    	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
#if defined(DROI_PRO_F5C_YH2)
	params->dsi.LANE_NUM				  = LCM_THREE_LANE;
#else
	params->dsi.LANE_NUM				  = LCM_FOUR_LANE;
#endif
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size=256;

	// Video mode setting		//for low temp. can't initial
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=720*3;


#if defined(DROI_PRO_FQ5C_XWP)
 params->dsi.vertical_sync_active                         = 4;// 3    2
        params->dsi.vertical_backporch                    = 38;// 20   1
        params->dsi.vertical_frontporch                   = 40; // 1  12
        params->dsi.vertical_active_line                  = FRAME_HEIGHT;

        params->dsi.horizontal_sync_active                = 10;// 50  2
        params->dsi.horizontal_backporch                  = 72;
        params->dsi.horizontal_frontporch                 = 72 ;
        params->dsi.horizontal_active_pixel               = FRAME_WIDTH;

        params->dsi.esd_check_enable = 1;
        params->dsi.customization_esd_check_enable = 1;
        params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
        params->dsi.lcm_esd_check_table[0].count        = 1;
        params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

	    params->physical_width = 62;
	    params->physical_height = 110;

        params->dsi.PLL_CLOCK = 219; // 240 230this value must be in MTK suggested table
        params->dsi.ssc_disable = 1;
        params->dsi.compatibility_for_nvk = 1;
#elif defined(DROI_PRO_F5C_NYX)
	params->dsi.vertical_sync_active				= 4;// 3    2
    	params->dsi.vertical_backporch					=16;// 20   1
    	params->dsi.vertical_frontporch					= 20; // 1  12
    	params->dsi.vertical_active_line				= FRAME_HEIGHT;

    	params->dsi.horizontal_sync_active			=4;// 50  2
    	params->dsi.horizontal_backporch				= 64 ;
    	params->dsi.horizontal_frontporch				= 64 ;
    	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK =218;
    	params->dsi.ssc_disable = 1;

	 params->dsi.cont_clock = 0;
    params->dsi.esd_check_enable = 1;
    params->dsi.noncont_clock = 1;
    params->dsi.customization_esd_check_enable = 1;
    params->dsi.clk_lp_per_line_enable =1 ;
    params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
    params->dsi.lcm_esd_check_table[0].count        = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;

#elif defined(DROI_PRO_FQ5C_ZG)
 params->dsi.vertical_sync_active                         = 4;// 3    2
        params->dsi.vertical_backporch                    = 16;// 20   1
        params->dsi.vertical_frontporch                   = 40; // 1  12
        params->dsi.vertical_active_line                  = FRAME_HEIGHT;

        params->dsi.horizontal_sync_active                = 4;// 50  2
        params->dsi.horizontal_backporch                  = 100;
        params->dsi.horizontal_frontporch                 = 100;
        params->dsi.horizontal_active_pixel               = FRAME_WIDTH;

        params->dsi.PLL_CLOCK = 238; // 240 230this value must be in MTK suggested table
        params->dsi.compatibility_for_nvk = 1;
        params->dsi.ssc_disable = 1;
   	    params->dsi.esd_check_enable = 1;
	    params->dsi.customization_esd_check_enable = 1;
	    params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	    params->dsi.lcm_esd_check_table[0].count = 1;
	    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F5C_YH2)
	params->dsi.vertical_sync_active			= 4;
    params->dsi.vertical_backporch				= 16;
    params->dsi.vertical_frontporch				= 16;
    params->dsi.vertical_active_line			= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active			= 10;
    params->dsi.horizontal_backporch			= 66;
    params->dsi.horizontal_frontporch			= 64;
    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 234; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;
    params->physical_width  = 68;
    params->physical_height = 122;

	params->dsi.noncont_clock = 1;
	params->dsi.noncont_clock_period = 1;
   	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_F6_G2)
	params->dsi.vertical_sync_active			= 4;
	params->dsi.vertical_backporch				= 16;
	params->dsi.vertical_frontporch				= 20;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 4;
	params->dsi.horizontal_backporch			= 60;
	params->dsi.horizontal_frontporch			= 60;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 220; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_PF5T_ZXHL)||defined(DROI_PRO_PF5_ZXHL)
	params->dsi.vertical_sync_active			= 4;
	params->dsi.vertical_backporch				= 40;
	params->dsi.vertical_frontporch				= 40;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 50;
	params->dsi.horizontal_backporch			= 40;
	params->dsi.horizontal_frontporch			= 40;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 207; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#elif defined(DROI_PRO_FQ5CW_XWP)	
	params->dsi.vertical_sync_active			= 4;
    params->dsi.vertical_backporch				= 16;
    params->dsi.vertical_frontporch				= 16;
    params->dsi.vertical_active_line			= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active			= 10;
    params->dsi.horizontal_backporch			= 66;
    params->dsi.horizontal_frontporch			= 64;
    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 230; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;
#else
    params->dsi.vertical_sync_active			= 4;
    params->dsi.vertical_backporch				= 16;
    params->dsi.vertical_frontporch				= 16;
    params->dsi.vertical_active_line			= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active			= 10;
    params->dsi.horizontal_backporch			= 66;
    params->dsi.horizontal_frontporch			= 64;
    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 234; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;

   	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#endif
}

static void lcm_init(void)
{
#if defined(SUPORT_ADC_CHECK)
	int val = 0;
#endif
	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(20);
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
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}

static void lcm_suspend(void)
{
#if defined(DROI_PRO_FQ5C_XWP)||defined(DROI_PRO_FQ5C_ZG)
	unsigned int data_array[16];
	data_array[0]=0x00280500;
	dsi_set_cmdq(data_array,1,1);
	MDELAY(20);
	data_array[0]=0x00100500;
	dsi_set_cmdq(data_array,1,1);
	MDELAY(120);
	data_array[0]=0x014f1500;
	dsi_set_cmdq(data_array,1,1);
	MDELAY(20);
#else
    push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);

#if defined(TYD_PRO_F1C)

#elif defined(DROI_PRO_F5C_YH)
    if((lcm_read_ADC_value()>600)&&(lcm_read_ADC_value()<1000))         //voltage=0.8V LG
    {
        ;
    }
    else
    {
		SET_RESET_PIN(0);
		MDELAY(50);
    }
#else
		SET_RESET_PIN(0);
		MDELAY(50);
#endif

#endif

//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5C_XDNZ)||defined(DROI_PRO_FQ5C_ZG)
    lcm_flag = 1;
#endif
//end

}

//static unsigned int vcom=0x70;
static void lcm_resume(void)
{
	lcm_init();
	//add for limit charger current by lcm on/off
#if defined(DROI_PRO_FQ5C_XDNZ)||defined(DROI_PRO_FQ5C_ZG)
    lcm_flag = 0;
#endif
//end
#if 0
	 vcom+=2;
    lcm_initialization_setting_0800[22].para_list[0]=vcom;
    lcm_initialization_setting_0800[23].para_list[0]=vcom;
    push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
#endif
}

static unsigned int lcm_compare_id(void)
{

	unsigned int id = 0, id2 = 0;
	unsigned char buffer[2];

	unsigned int data_array[5];

	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

//*************Enable CMD2 Page1  *******************//
	data_array[0]=0x00063902;
	data_array[1]=0x52AA55F0;
	data_array[2]=0x00000108;
	dsi_set_cmdq(data_array, 3, 1);
	MDELAY(10);

	data_array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xC5, buffer, 2);
	id = buffer[0]; //we only need ID
	id2= buffer[1]; //we test buffer 1
#ifdef BUILD_LK
	printf("NT35521 LK id=%d,id2=%d\n",id,id2);
#else
	printk("NT35521 KERNEL id=%d,id2=%d\n",id,id2);
#endif

       return (0x5521 == (id2 |id<<8))?1:0;

}
LCM_DRIVER nt35521_dsi_6735_hd_drv =
{
    .name           	= "nt35521_dsi_6737_H",
    .set_util_funcs 	= lcm_set_util_funcs,
    .get_params     	= lcm_get_params,
    .init           	= lcm_init,
    .suspend        	= lcm_suspend,
    .resume         	= lcm_resume,
    .compare_id     	= lcm_compare_id,
};

