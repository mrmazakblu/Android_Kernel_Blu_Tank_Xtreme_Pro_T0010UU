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

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);

#if defined(DROI_PRO_F6)
#include "nt35523_dsi_6735_hd_f6.h"
#endif

#if defined(DROI_PRO_PF5)
#include "nt35523_dsi_6735_hd_pf5.h"
#endif

#if defined(DROI_PRO_F5C)
#include "nt35523_dsi_6735_hd_f5c.h"
#endif

#if defined(DROI_PRO_FQ5B)
#include "nt35523_dsi_6735_hd_fq5b.h"
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
#if defined(DROI_PRO_PF5_A7)||defined(DROI_PRO_F5C_A7)
	if((val>200)&&(val<600))		    //voltage=0.4V hongli
	{
		push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000))		//voltage=0.8V zhanjie
	{
		push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{
	}
#else
	if((val>1000)&&(val<1400))		//voltage=1.2V
	{
		push_table(lcm_initialization_setting_1200, sizeof(lcm_initialization_setting_1200) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000))		//voltage=0.8V
	{
		push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
	}
	else if ((val>200)&&(val<600))		//voltage=0.4V
	{
		push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else
	{
	}
#endif
}
#endif

#ifndef DROI_LCD_USE_CUSTOM_NT35523_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x11,	1,	{0x00}},
	{REGFLAG_DELAY, 120, {}},

	{0x29,	1,	{0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}},
};
#endif

static struct LCM_setting_table lcm_sleep_in_setting[] =
{
    {0x28, 0, {0x00}},
    {REGFLAG_DELAY, 20, {}},
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
	params->dsi.LANE_NUM				  = LCM_FOUR_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size=256;

	// Video mode setting		//for low temp. can't initial
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=720*3;

#if defined(DROI_PRO_PF5_A7)||defined(DROI_PRO_F5C_A7)
	params->dsi.vertical_sync_active			= 4;
	params->dsi.vertical_backporch				= 16;
	params->dsi.vertical_frontporch				= 16;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 10;
	params->dsi.horizontal_backporch			= 66;
	params->dsi.horizontal_frontporch			= 64;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 212; //this value must be in MTK suggested table
#elif defined(DROI_PRO_F5C_NYX)
	params->dsi.vertical_sync_active				= 4;// 3    2
    	params->dsi.vertical_backporch					=20;// 20   1
    	params->dsi.vertical_frontporch					= 20; // 1  12
    	params->dsi.vertical_active_line				= FRAME_HEIGHT;

    	params->dsi.horizontal_sync_active			=10;// 50  2
    	params->dsi.horizontal_backporch				= 64 ;
    	params->dsi.horizontal_frontporch				= 64 ;
    	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK =205;
    	params->dsi.ssc_disable = 1;

	 params->dsi.cont_clock = 0;
    params->dsi.esd_check_enable = 1;
    params->dsi.noncont_clock = 1;
    params->dsi.customization_esd_check_enable = 1;
    params->dsi.clk_lp_per_line_enable =1 ;
    params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
    params->dsi.lcm_esd_check_table[0].count        = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;

#elif defined(DROI_PRO_F6_ZR_M82)
	params->dsi.vertical_sync_active			= 10;
	params->dsi.vertical_backporch				= 20;
	params->dsi.vertical_frontporch				= 20;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 10;
	params->dsi.horizontal_backporch			= 70;//100;
	params->dsi.horizontal_frontporch			= 70;//100;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 208;//280; //this value must be in MTK suggested table
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
#endif
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
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
#if 0
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

#if defined(DROI_PRO_PF5_A7)||defined(DROI_PRO_F5C_A7)
#else
	SET_RESET_PIN(0);
	MDELAY(50);
#endif
#endif
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
	printf("NT35523 LK id=%d,id2=%d\n",id,id2);
#else
	printk("NT35523 KERNEL id=%d,id2=%d\n",id,id2);
#endif

       return (0x5523 == (id2 |id<<8))?1:0;

}

LCM_DRIVER nt35523_dsi_6735_hd_drv =
{
    .name           	= "nt35523_dsi_6737_H",
    .set_util_funcs 	= lcm_set_util_funcs,
    .get_params     	= lcm_get_params,
    .init           	= lcm_init,
    .suspend        	= lcm_suspend,
    .resume         	= lcm_resume,
    .compare_id     	= lcm_compare_id,
};

