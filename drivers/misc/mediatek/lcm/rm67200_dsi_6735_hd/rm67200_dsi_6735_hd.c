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
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <string.h>
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>
#else
//#include <mach/mt_gpio.h>
//#include <linux/xlog.h>
//#include <mach/mt_pm_ldo.h>
#include <mt-plat/mt_gpio.h>
#include <mach/gpio_const.h>
#include <linux/gpio.h>
#endif

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#ifndef BUILD_LK
extern atomic_t ESDCheck_byCPU;
#endif

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
#include "rm67200_dsi_6735_hd_f6.h"
#endif
#if defined(DROI_PRO_F5C)
#include "rm67200_dsi_6735_hd_f5c.h"
#endif

#if defined(DROI_PRO_Q1)
#include "rm67200_dsi_6735_hd_q1.h"
#endif


#if defined(SUPORT_ADC_CHECK)
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static int lcm_read_ADC_value(void)
{
	int val;           //lvl = LCM_V_LEVEL;
	int dwChannel = 12; //LCM_ADC_CHAN;

	int data[4] = {0,0,0,0};
	int data0;
//	char* buf_temp;
	int res =0;
//	unsigned int ret;
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
#if defined(DROI_PRO_Q1_WT)
	if((val>0200)&&(val<0600))             //voltage=1.2V
	{
		push_table(lcm_initialization_setting_0400, sizeof(lcm_initialization_setting_0400) / sizeof(struct LCM_setting_table), 1);
	}
	else if((val>600)&&(val<1000))             //voltage=1.2V
	{
		push_table(lcm_initialization_setting_0800, sizeof(lcm_initialization_setting_0800) / sizeof(struct LCM_setting_table), 1);
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

#ifndef DROI_LCD_USE_CUSTOM_RM67200_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x11, 0,{0x00}},

	{REGFLAG_DELAY,120,{}},

	// Display ON
	{0x29, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
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
#if defined(DROI_PRO_F5C_YH2) || defined(DROI_PRO_F6_YH)||defined(DROI_PRO_Q1_HS)
	params->dsi.LANE_NUM				= LCM_THREE_LANE;
#else
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
#endif
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      		= LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size=256;

	// Video mode setting		//for low temp. can't initial
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=720*3;
#if defined(DROI_PRO_F5C_YH2)
	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 24;
	params->dsi.horizontal_frontporch	= 24;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=260;
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;
#elif defined(DROI_PRO_F6_YH)
	params->dsi.vertical_sync_active			= 2;
	params->dsi.vertical_backporch				= 14;
	params->dsi.vertical_frontporch				= 16;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 8;
	params->dsi.horizontal_backporch			= 24;
	params->dsi.horizontal_frontporch			= 24;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 260; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;
#elif defined(DROI_PRO_Q1_HS)
	params->dsi.vertical_sync_active			= 2;
	params->dsi.vertical_backporch				= 14;
	params->dsi.vertical_frontporch				= 16;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 8;
	params->dsi.horizontal_backporch			= 24;
	params->dsi.horizontal_frontporch			= 24;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 260; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;

	params->physical_width = 70;
	params->physical_height = 121;	
#elif	defined(DROI_PRO_Q1_DT)

	params->dsi.vertical_sync_active	= 2;// 3    2
	params->dsi.vertical_backporch		= 14;// 20   1
	params->dsi.vertical_frontporch		= 16; // 1  12
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 8;// 50  2
	params->dsi.horizontal_backporch	= 24;
	params->dsi.horizontal_frontporch	= 24;
  //params->dsi.horizontal_blanking_pixel	= 60;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

//    params->dsi.compatibility_for_nvk = 0;

	params->dsi.PLL_CLOCK=245;
#elif	defined(DROI_PRO_Q1_HN)
	params->dsi.vertical_sync_active			= 4;
	params->dsi.vertical_backporch				= 16;
	params->dsi.vertical_frontporch				= 16;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 10;
	params->dsi.horizontal_backporch			= 66;
	params->dsi.horizontal_frontporch			= 64;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 208; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;

	params->physical_width = 70;
	params->physical_height = 121;
#elif defined(DROI_PRO_Q1_WT)
	params->dsi.vertical_sync_active			= 2;
	params->dsi.vertical_backporch				= 14;
	params->dsi.vertical_frontporch				= 16;
	params->dsi.vertical_active_line			= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active			= 8;
	params->dsi.horizontal_backporch			= 32;
	params->dsi.horizontal_frontporch			= 32;
	params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 198; //this value must be in MTK suggested table
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
	params->dsi.PLL_CLOCK = 208; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;
#endif
}

static void lcm_init(void)
{
#if defined(SUPORT_ADC_CHECK)
	int val = 0;
#endif
#ifdef BUILD_LK
	printf("DROI ------------------- %d start\n", __func__);
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
	printf("Droi------------------- val:%d\n",val);
#else
	printk("Droi------------------- val:%d\n",val);
#endif
	adc_lcm_push_table(val);
#else
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif

#ifdef BUILD_LK
	printf("DROI ------------------- %d end\n", __func__);
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

static struct LCM_setting_table lcm_compare_id_setting[] = {
	{0xFE, 1,{0x20}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static unsigned int lcm_compare_id(void)
{
	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id1=0;
	int id2=0;

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(200);

	push_table(lcm_compare_id_setting,
			sizeof(lcm_compare_id_setting) /
			sizeof(struct LCM_setting_table), 1);

	/*array[0]=0x02FE1500;
	dsi_set_cmdq(array,1, 1);*/

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xed, buffer, 1);

	id_high = buffer[0];
	read_reg_v2(0xee, buffer, 1);
	id_low = buffer[0];
	id1 = (id_high<<8) | id_low;

#if defined(BUILD_LK)
	printf("rm67200a %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#else
	printk("rm67200a %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#endif

	return (0x7200 == id1)?1:0;
}

static unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
	int array[4];
	char buff[5];
	char id_high=0;
	char id_low=0;
	int id1=0;
	int id2=0;

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(200);

	push_table(lcm_compare_id_setting,
			sizeof(lcm_compare_id_setting) /
			sizeof(struct LCM_setting_table), 1);

	/*array[0]=0x02FE1500;
	dsi_set_cmdq(array,1, 1);*/

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	atomic_set(&ESDCheck_byCPU,1);
	read_reg_v2(0xed, buff, 1);
	atomic_set(&ESDCheck_byCPU,0);

	id_high = buff[0];
	atomic_set(&ESDCheck_byCPU,1);
	read_reg_v2(0xee, buff, 1);
	atomic_set(&ESDCheck_byCPU,0);
	id_low = buff[0];
	id1 = (id_high<<8) | id_low;

#if defined(BUILD_LK)
	printf("rm67200a %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#else
	printk("rm67200a %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#endif

	return (0x7200 == id1)?1:0;
#endif
}

LCM_DRIVER rm67200_dsi_6735_hd_drv =
{
	.name		    	= "rm67200_dsi_H",
	.set_util_funcs 	= lcm_set_util_funcs,
	.get_params     	= lcm_get_params,
	.init           	= lcm_init,
	.suspend        	= lcm_suspend,
	.resume         	= lcm_resume,
	.compare_id     	= lcm_compare_id,
	.ata_check      	= lcm_ata_check,
};

