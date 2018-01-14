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

#ifndef _LEDS_SW_H
#define _LEDS_SW_H

/******************************************************************************
 *  LED & Backlight type defination
 *****************************************************************************/

enum mt65xx_led_type {
	MT65XX_LED_TYPE_RED = 0,
	MT65XX_LED_TYPE_GREEN,
	MT65XX_LED_TYPE_BLUE,
	MT65XX_LED_TYPE_JOGBALL,
	MT65XX_LED_TYPE_KEYBOARD,
	MT65XX_LED_TYPE_BUTTON,
	MT65XX_LED_TYPE_LCD,
	MT65XX_LED_TYPE_TOTAL,
};

enum mt65xx_led_mode {
	MT65XX_LED_MODE_NONE,
	MT65XX_LED_MODE_PWM,
	MT65XX_LED_MODE_GPIO,
	MT65XX_LED_MODE_PMIC,
	MT65XX_LED_MODE_CUST_LCM,
	MT65XX_LED_MODE_CUST_BLS_PWM
};

/******************************************************************************
 *  for backlight
 *****************************************************************************/

/* backlight call back function */
typedef int (*cust_brightness_set) (int level, int div);
typedef int (*cust_set_brightness) (int level);

/* 10bit backlight level */
#define LED_INCREASE_LED_LEVEL_MTKPATCH
#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
#define MT_LED_INTERNAL_LEVEL_BIT_CNT 10
#endif

/******************************************************************************
 *  for PMIC
 *****************************************************************************/

enum mt65xx_led_pmic {
	MT65XX_LED_PMIC_LCD_ISINK = 0,
	MT65XX_LED_PMIC_NLED_ISINK0,
	MT65XX_LED_PMIC_NLED_ISINK1,
	MT65XX_LED_PMIC_NLED_ISINK2,
	MT65XX_LED_PMIC_NLED_ISINK3
};

enum MT65XX_PMIC_ISINK_MODE {
	ISINK_PWM_MODE = 0,
	ISINK_BREATH_MODE = 1,
	ISINK_REGISTER_MODE = 2
};

enum MT65XX_PMIC_ISINK_STEP {
	ISINK_0 = 0,		/* 4mA */
	ISINK_1 = 1,		/* 8mA */
	ISINK_2 = 2,		/* 12mA */
	ISINK_3 = 3,		/* 16mA */
	ISINK_4 = 4,		/* 20mA */
	ISINK_5 = 5		/* 24mA */
};

enum MT65XX_PMIC_ISINK_FSEL {
	/* 32K clock */
	ISINK_1KHZ = 0,
	ISINK_200HZ = 4,
	ISINK_5HZ = 199,
	ISINK_2HZ = 499,
	ISINK_1HZ = 999,
	ISINK_05HZ = 1999,
	ISINK_02HZ = 4999,
	ISINK_01HZ = 9999,
	/* 2M clock */
	ISINK_2M_20KHZ = 2,
	ISINK_2M_1KHZ = 61,
	ISINK_2M_200HZ = 311,
	ISINK_2M_5HZ = 12499,
	ISINK_2M_2HZ = 31249,
	ISINK_2M_1HZ = 62499
};

/******************************************************************************
 *  for PWM
 *****************************************************************************/


#endif				/* _LEDS_SW_H */
