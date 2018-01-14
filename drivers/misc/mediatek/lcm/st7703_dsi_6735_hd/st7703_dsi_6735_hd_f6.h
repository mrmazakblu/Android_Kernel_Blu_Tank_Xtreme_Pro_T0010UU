#ifndef __LCD_ST7703_DSI_HD_F6_H__
#define __LCD_ST7703_DSI_HD_F6_H__

#if defined(DROI_PRO_F6_BPZN3)
//#define SUPORT_ADC_CHECK
#define DROI_LCD_USE_CUSTOM_ST7703_HD
#endif

#if defined(DROI_PRO_F6_BPZN3)
static struct LCM_setting_table lcm_initialization_setting[] = {
	// LCD2: guangtai IC: ST7703, glass: TM(tianma) 5.5HD
	{REGFLAG_DELAY, 50, {0}},
	{0xb9,3,{0xF1,0x12,0x83}},
	{0xba,27,{0x32,0x81,0x05,0xF9,0x0E,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x25,0x00,0x91,0x0A,0x00,0x00,0x02,0x4F,0xD1,0x00,0x00,0x37}},
	{0xb8,1,{0x26}},
	{0xb3,10,{0x0C,0x10,0x0A,0x50,0x03,0xFF,0x00,0x00,0x00,0x00}},
	{0xc0,9,{0x73,0x73,0x50,0x50,0x00,0x00,0x08,0x70,0x00}},
	{0xbc,1,{0x4f}},
	{0xcc,2,{0x0b}},
	{0xb4,1,{0x00}},
	{0xb2,3,{0xc8,0x12,0x30}},
	{0xe3,14,{0x07,0x07,0x0B,0x0B,0x03,0x0B,0x00,0x00,0x00,0x00,0xFF,0x80,0xC0,0x10}},
	{0xc1,12,{0x73,0x00,0x05,0x05,0x77,0xF1,0xCC,0xDD,0x67,0x77,0x33,0x33}},
	{0xb5,2,{0x09,0x09}},
	{0xb6,2,{0x85,0x85}},
	{0xbf,3,{0x02,0x11,0x00}},
	{0xe9,63,{0xC2,0x10,0x09,0x08,0xFE,0x08,0xB0,0x12,0x31,0x23,0x3F,0x85,0x08,0xB0,0x37,0x14,0x08,0x00,0x30,0x00,0x00,0x00,0x08,0x00,0x30,0x00,0x00,0x00,0x20,0x64,0x02,0x88,0x88,0x88,0x88,0x88,0x88,0x84,0x8F,0x31,0x75,0x13,0x88,0x88,0x88,0x88,0x88,0x88,0x85,0x8F,0x00,0x00,0x00,0x01,0x85,0x02,0xB1,0x47,0x00,0x3C,0x00,0x00,0x00}},
	{0xea,61,{0x00,0x1A,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x57,0x13,0x31,0x88,0x88,0x88,0x88,0x88,0x88,0x58,0xF8,0x46,0x02,0x20,0x88,0x88,0x88,0x88,0x88,0x88,0x84,0xF8,0x00,0x23,0x10,0x00,0x34,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x14,0x00,0x20,0x30,0x02,0xB1,0x00,0x00,0x00,0x00}},
	{0xe0,34,{0x00,0x03,0x04,0x2f,0x3e,0x3f,0x30,0x2a,0x05,0x0D,0x0d,0x10,0x11,0x0f,0x12,0x0f,0x14,0x00,0x03,0x04,0x2f,0x3e,0x3f,0x30,0x2a,0x05,0x0D,0x0d,0x10,0x11,0x0f,0x12,0x0f,0x14}},
	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 250, {0}},
	{0x29,1,{0x00}},  // Display On
	{REGFLAG_DELAY, 50, {0}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
#endif