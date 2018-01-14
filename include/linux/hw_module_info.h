/* Copyright Statement:
 *
 * This software/firmware and related documentation ("tydtech Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to tydtech Inc. and/or its licensors.
 * Without the prior written permission of tydtech inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of tydtech Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* tydtech Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("TYDTECH SOFTWARE")
 * RECEIVED FROM TYDTECH AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. TYDTECH EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES TYDTECH PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE TYDTECH SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN TYDTECH SOFTWARE. TYDTECH SHALL ALSO NOT BE RESPONSIBLE FOR ANY TYDTECH
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND TYDTECH'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE TYDTECH SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT TYDTECH'S OPTION, TO REVISE OR REPLACE THE TYDTECH SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * TYDTECH FOR SUCH TYDTECH SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("tydtech Software")
 * have been modified by tydtech Inc. All revisions are subject to any receiver's
 * applicable license agreements with tydtech Inc.
 */

/******************************************************************************
 * file name: hw_module_info.h
 * author: liguo
 * email: lxm542006@163.com
 *
 * Copyright 2010 tydtech Co.,Ltd.
 ******************************************************************************/
#ifndef _HW_MODULE_INFO_H
#define _HW_MODULE_INFO_H

#include <linux/list.h>

typedef enum _hw_module_type
{
	HW_MODULE_TYPE_MIN = 0,
	HW_MODULE_TYPE_CTP,
	HW_MODULE_TYPE_LCM,
	HW_MODULE_TYPE_ALSPS,
	HW_MODULE_TYPE_GS,
	HW_MODULE_TYPE_FP,
	HW_MODULE_TYPE_MS,
	HW_MODULE_TYPE_GY,
	HW_MODULE_TYPE_MAIN_CAMERA,
	HW_MODULE_TYPE_SUB_CAMERA,
	HW_MODULE_TYPE_MAIN_2_CAMERA,
	HW_MODULE_TYPE_LEN,
	HW_MODULE_TYPE_FLASHLIGHT,
	HW_MODULE_TYPE_BATTERY,
	HW_MODULE_TYPE_MAX,
	HW_MODULE_TYPE_ALL = HW_MODULE_TYPE_MAX
}hw_module_type;

typedef struct _hw_module
{
	hw_module_type type;
	char *name;
}hw_module;

typedef struct _hw_module_info
{
	struct list_head link;
	hw_module_type type;
	int priority;
	int id;
	const char *name;
	char *vendor;
	char *more;
}hw_module_info;

enum{
	HW_MODULE_PRIORITY_LCM           = 5,
	HW_MODULE_PRIORITY_CTP           = 10,
	HW_MODULE_PRIORITY_MAIN_CAMERA   = 15,
	HW_MODULE_PRIORITY_SUB_CAMERA    = 20,
	HW_MODULE_PRIORITY_MAIN_2_CAMERA   = 25,
	HW_MODULE_PRIORITY_ALSPS         = 30,
	HW_MODULE_PRIORITY_GS            = 35,
	HW_MODULE_PRIORITY_MS            = 40,
	HW_MODULE_PRIORITY_GY            = 45,
	HW_MODULE_PRIORITY_LEN           = 50,
	HW_MODULE_PRIORITY_FLASHLIGHT    = 55,
	HW_MODULE_PRIORITY_BATTERY    = 60,
	HW_MODULE_PRIORITY_FP    = 65
};

extern int hw_module_info_add(hw_module_info *info);
extern int hw_module_info_del(hw_module_info *info);

#endif  // _HW_MODULE_INFO_H

