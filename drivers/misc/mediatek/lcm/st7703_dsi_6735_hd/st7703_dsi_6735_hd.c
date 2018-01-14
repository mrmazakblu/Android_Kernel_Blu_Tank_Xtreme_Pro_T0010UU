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


#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#else
	#include <linux/string.h>
	#if defined(BUILD_UBOOT)
		#include <asm/arch/mt_gpio.h>
	#else
//		#include <mach/mt_gpio.h>
	#endif
#endif


#include "lcm_drv.h"

#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
	#include <platform/mt_pmic.h>
	#include <platform/mt_i2c.h> 
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	//#include <mach/mt_gpio.h>
	//#include <linux/xlog.h>
	//#include <mach/mt_pm_ldo.h>
	#include <mt-plat/mt_gpio.h>
	#include <mach/gpio_const.h>
	#include <linux/gpio.h>
#endif
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#define FRAME_WIDTH  										(640)
#define FRAME_HEIGHT 										(1280)
#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#endif
#define FL11281_MODULE_ID (0x8)

#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#define LCM_DSI_CMD_MODE									0
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------


//#define ST7703_MODULE_ID 0x8312

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

#if defined(DROI_PRO_PU6)||defined(DROI_PRO_PU6T)
#define GPIO_LDO28_EN (GPIO125 | 0x80000000)
#define GPIO_LDO18_EN (GPIO126 | 0x80000000)
//#define GPIO_LCM_RST (GPIO146 | 0x80000000)
#endif
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define changecmd(a,b,c,d) ((d<<24)|(c<<16)|(b<<8)|a)

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update);

#if defined(DROI_PRO_F6)
#include "st7703_dsi_6735_hd_f6.h"
#endif

#if defined(DROI_PRO_F5C)
#include "st7703_dsi_6735_hd_f5c.h"
#endif

#if defined(DROI_PRO_PU6)
#include "st7703_dsi_6735_hd_pu6.h"
#endif

#ifndef DROI_LCD_USE_CUSTOM_ST7703_HD
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xB9,3,{0xF1,0x12,0x83}},

{0xBA,27,{0x33,0x81,0x05,0xF9,0x0E,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x25,0x00,0x91,0x0A,0x00,0x00,0x02,0x4F,0xD1,0x00,0x00,0x37}},

{0xB8,1,{0x25}},

{0xBF,3,{0x02,0x11,0x00}},

{0xB3,10,{0x0C,0x10,0x0A,0x50,0x03,0xFF,0x00,0x00,0x00,0x00}},

{0xC0,9,{0x73,0x73,0x50,0x50,0x00,0x00,0x08,0x70,0x00}},

{0xBC,1,{0x46}},

{0xCC,1,{0x0B}},

{0xB4,1,{0x80}},

{0xB2,3,{0xC8,0x12,0x30}},

{0xE3,14,{0x07,0x07,0x0B,0x0B,0x03,0x0B,0x00,0x00,0x00,0x00,0xFF,0x80,0xC0,0x10}},

{0xC1,12,{0x25,0x00,0x1E,0x1E,0x77,0xE1,0xFF,0xFF,0xCC,0xCC,0x77,0x77}},


{0xB5,2,{0x0A,0x0A}},

{0xB6,2,{0x7F,0x7F}},

{0xE9,63,{0xC2,0x10,0x08,0x00,0x00,0x41,0xF8,0x12,0x31,0x23,0x37,0x86,0x11,0xC8,0x37,0x2A,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x88,0x20,0x46,0x02,0x88,0x88,0x88,0x88,0x88,0x88,0xFF,0x88,0x31,0x57,0x13,0x88,0x88,0x88,0x88,0x88,0x88,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0xEA,61,{0x00,0x1A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x8F,0x13,0x31,0x75,0x88,0x88,0x88,0x88,0x88,0x88,0xF8,0x8F,0x02,0x20,0x64,0x88,0x88,0x88,0x88,0x88,0x88,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0xE0,34,{0x03,0x19,0x1D,0x2E,0x32,0x38,0x4A,0x3E,0x07,0x0C,0x0F,0x12,0x14,0x12,0x13,0x0F,0x16,0x03,0x19,0x1D,0x2E,0x32,0x38,0x4A,0x3E,0x07,0x0C,0x0F,0x12,0x14,0x12,0x13,0x0F,0x16}},

{0x11,1,{0X00}}, ////Sleep Out
{REGFLAG_DELAY,250,{}},

{0x29,1,{0X00}}, ///Display On
{REGFLAG_DELAY,50,{}},
};
#endif
//static int vcom = 0x18;
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {
#if 0
      	    	case 0xB6:
		    		table[i].para_list[0]=vcom;
		    		table[i].para_list[1]=vcom;
		    		dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
            		vcom+=2;
            		break;
#endif

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

    params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
    //params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;


	#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
	#else
	    params->dsi.mode   = SYNC_PULSE_VDO_MODE;  //; BURST_VDO_MODE
	#endif
	    // DSI
	    /* Command mode setting */

#if defined(DROI_PRO_F6_BPZN3)
	params->dsi.LANE_NUM				= LCM_THREE_LANE;
#else
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
#endif

	    //The following defined the fomat for data coming from LCD engine.
	    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	    params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	    params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	    params->dsi.data_format.format	  = LCM_DSI_FORMAT_RGB888;

        // Highly depends on LCD driver capability.
		params->dsi.packet_size=256;
	    // Video mode setting
	    params->dsi.intermediat_buffer_num = 0;

	    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
#if defined(DROI_PRO_FQ5CW_LTX)
	params->dsi.vertical_sync_active				= 3;
	params->dsi.vertical_backporch					= 10;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;
	params->dsi.horizontal_sync_active				= 4;
	params->dsi.horizontal_backporch				= 45;
	params->dsi.horizontal_frontporch				= 45;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK= 238;//208
#elif defined(DROI_PRO_F6_BPZN3)
	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 17;
	params->dsi.vertical_frontporch					= 18;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;
	params->dsi.horizontal_sync_active				= 10;//20
	params->dsi.horizontal_backporch				= 40;//30
	params->dsi.horizontal_frontporch				= 40;//30
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK= 240;//208
#elif defined(DROI_PRO_F5C_SGDZ)
	params->dsi.vertical_sync_active				= 3;
	params->dsi.vertical_backporch					= 10;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;
	params->dsi.horizontal_sync_active				= 4;
	params->dsi.horizontal_backporch				= 45;
	params->dsi.horizontal_frontporch				= 45;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK= 208;//208
#elif defined(DROI_PRO_PU6_OLK)
	
	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 17;
	params->dsi.vertical_frontporch					= 18;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 50;
	params->dsi.horizontal_backporch				= 110;
	params->dsi.horizontal_frontporch				= 110;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK= 228;//208

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0xaf;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0xfd;

	params->dsi.lcm_esd_check_table[1].cmd = 0x09;
	params->dsi.lcm_esd_check_table[1].count = 4;
	params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;
	params->dsi.lcm_esd_check_table[1].para_list[1] = 0x73;
	params->dsi.lcm_esd_check_table[1].para_list[2] = 0x04;
	params->dsi.lcm_esd_check_table[1].para_list[3] = 0x1c;

	params->dsi.lcm_esd_check_table[2].cmd = 0x68;
	params->dsi.lcm_esd_check_table[2].count = 1;
	params->dsi.lcm_esd_check_table[2].para_list[0] = 0xc0;
#else
	params->dsi.vertical_sync_active				= 3;
	params->dsi.vertical_backporch					= 10;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;
	params->dsi.horizontal_sync_active				= 4;
	params->dsi.horizontal_backporch				= 45;
	params->dsi.horizontal_frontporch				= 45;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.PLL_CLOCK= 240;//208
#endif
	params->dsi.noncont_clock	= 1;
	params->dsi.noncont_clock_period	= 1;
    	//params->dsi.esd_check_enable = 1;
    	//params->dsi.customization_esd_check_enable = 1;
	//params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
	//params->dsi.lcm_esd_check_table[0].count = 1;
	//params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;

	params->dsi.ssc_disable = 1;
}

static void lcm_init(void)
{
//  unsigned int data_array[16];
  SET_RESET_PIN(1);
  MDELAY(10);
  SET_RESET_PIN(0);
  MDELAY(10);
  SET_RESET_PIN(1);
  MDELAY(120);

  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];
	data_array[0] = 0x00280500;
	dsi_set_cmdq(data_array, 1,1);
	MDELAY(10);

	data_array[0] = 0x00100500;
	dsi_set_cmdq(data_array, 1,1);
	MDELAY(120);

	SET_RESET_PIN(0);
	MDELAY(20);
}

static unsigned int lcm_compare_id(void);
static void lcm_resume(void)
{
	lcm_init();
}

static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[4];
	unsigned int data_array[16];

	SET_RESET_PIN(1);	//NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	MDELAY(10);
	data_array[0] = 0x00013700;
	dsi_set_cmdq(data_array, 1, 1);
	read_reg_v2(0xD0, buffer, 1);
	id = buffer[0];

	#if defined(BUILD_LK)
		printf("st7703_hd_dsi %s id = 0x%04x\n", __func__, id);
	#else
		printk("st7703_hd_dsi %s id = 0x%04x\n", __func__, id);
	#endif
	return (0x0D == id)?1:0;


}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER st7703_hd720_dsi_vdo_drv =
{
	.name			= "st7703_dsi_mt6735_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
//    .esd_check   = lcm_esd_check,
//    .esd_recover   = lcm_esd_recover,
};

