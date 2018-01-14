/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __MT65XX_LCM_LIST_H__
#define __MT65XX_LCM_LIST_H__

#include <lcm_drv.h>
/*FWVGA*/
extern LCM_DRIVER st7701_dsi_6735_fwvga_drv;
/*HD*/
extern LCM_DRIVER rm68191_dsi_6735_qhd_lcm_drv;
extern LCM_DRIVER otm1287a_dsi_6580_hd_drv;
extern LCM_DRIVER otm1287a_dsi_6735_hd_drv;
extern LCM_DRIVER otm1289a_dsi_6735_hd_drv;
extern LCM_DRIVER otm1285a_hd720_dsi_vdo_drv;
extern LCM_DRIVER ili9881_dsi_6735_hd_drv;
extern LCM_DRIVER fl11281_hd720_dsi_vdo_drv;
extern LCM_DRIVER jd9369_dsi_6735_hd_drv;
extern LCM_DRIVER jd9365_dsi_6735_hd_drv;
extern LCM_DRIVER rm68200_dsi_6735_hd_drv;
extern LCM_DRIVER nt35523_dsi_6735_hd_drv;
extern LCM_DRIVER s6d7aa0_hd_vdo_drv;
extern LCM_DRIVER hx8394a_dsi_6735_hd_drv;
extern LCM_DRIVER hx8394d_dsi_6735_hd_drv;
extern LCM_DRIVER ili9881_dsi_6735_hd_drv;
extern LCM_DRIVER nt35521_dsi_6735_hd_drv;
extern LCM_DRIVER jd9365_dsi_6735_hd_drv;
extern LCM_DRIVER nt35532_dsi_6735_fhd_lcm_drv;
extern LCM_DRIVER nt35521_dsi_6735_hd_tps65132_drv;
extern LCM_DRIVER rm67200_dsi_6735_hd_drv;
extern LCM_DRIVER st7703_hd720_dsi_vdo_drv;
extern LCM_DRIVER st7703_hdplus_dsi_vdo_drv;
extern LCM_DRIVER otm1285_dsi_6735_hd_drv;
extern LCM_DRIVER otm1191a_dsi_6735_fhd_lcm_drv;
extern LCM_DRIVER nt35596_fhd_dsi_vdo_auo_lcm_drv;
/*QHD*/

/*FHD*/

#ifdef BUILD_LK
extern void mdelay(unsigned long msec);
#endif

#endif
