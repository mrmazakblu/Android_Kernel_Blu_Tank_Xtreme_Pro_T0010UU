#ifndef __LCD_JD9365_DSI_HD_F6_H__
#define __LCD_JD9365_DSI_HD_F6_H__

#if defined(DROI_PRO_F6_JF)
//#define SUPORT_ADC_CHECK
#define TYD_LCD_USE_CUSTOM_JD9365_HD
#endif

#if defined(DROI_PRO_F6_JF)
static struct LCM_setting_table lcm_initialization_setting[] = {
//C_JD9365+F055A15-601_720x1280_JD5001_Column_G2.2 glass:CMI
{0xE0,1,{0x00}},

{0xE1,1,{0x93}},
{0xE2,1,{0x65}},
{0xE3,1,{0xF8}},

{0xE0,1,{0x01}},

{0x00,1,{0x00}},
{0x01,1,{0xA9}},
{0x03,1,{0x00}},
{0x04,1,{0xB6}},

{0x17,1,{0x00}},
{0x18,1,{0xBF}},
{0x19,1,{0x01}},
{0x1A,1,{0x00}},
{0x1B,1,{0xBF}},
{0x1C,1,{0x01}},

{0x1F,1,{0x7C}},
{0x20,1,{0x26}},
{0x21,1,{0x26}},
{0x22,1,{0x4E}},

{0x37,1,{0x09}},

{0x38,1,{0x04}},
{0x39,1,{0x08}},
{0x3A,1,{0x1F}},
{0x3B,1,{0x1F}},
{0x3C,1,{0x78}},
{0x3D,1,{0xFF}},
{0x3E,1,{0xFF}},
{0x3F,1,{0x00}},

{0x40,1,{0x04}},
{0x41,1,{0xA0}},

{0x43,1,{0x0F}},
{0x44,1,{0x0A}},
{0x45,1,{0x24}},

{0x55,1,{0x01}},
{0x56,1,{0x01}},
{0x57,1,{0x69}},
{0x58,1,{0x0A}},
{0x59,1,{0x4A}},
{0x5A,1,{0x2E}},
{0x5B,1,{0x1A}},
{0x5C,1,{0x19}},

{0x5D,1,{0x7C}},
{0x5E,1,{0x63}},
{0x5F,1,{0x53}},
{0x60,1,{0x47}},
{0x61,1,{0x44}},
{0x62,1,{0x36}},
{0x63,1,{0x3B}},
{0x64,1,{0x25}},
{0x65,1,{0x3D}},
{0x66,1,{0x3B}},
{0x67,1,{0x39}},
{0x68,1,{0x56}},
{0x69,1,{0x44}},
{0x6A,1,{0x4A}},
{0x6B,1,{0x3A}},
{0x6C,1,{0x36}},
{0x6D,1,{0x28}},
{0x6E,1,{0x12}},
{0x6F,1,{0x08}},
{0x70,1,{0x7C}},
{0x71,1,{0x63}},
{0x72,1,{0x53}},
{0x73,1,{0x47}},
{0x74,1,{0x44}},
{0x75,1,{0x36}},
{0x76,1,{0x3B}},
{0x77,1,{0x25}},
{0x78,1,{0x3D}},
{0x79,1,{0x3B}},
{0x7A,1,{0x39}},
{0x7B,1,{0x56}},
{0x7C,1,{0x44}},
{0x7D,1,{0x4A}},
{0x7E,1,{0x3A}},
{0x7F,1,{0x36}},
{0x80,1,{0x28}},
{0x81,1,{0x12}},
{0x82,1,{0x08}},

{0xE0,1,{0x02}},

{0x00,1,{0x57}},
{0x01,1,{0x77}},
{0x02,1,{0x44}},
{0x03,1,{0x46}},
{0x04,1,{0x48}},
{0x05,1,{0x4A}},
{0x06,1,{0x4C}},
{0x07,1,{0x4E}},
{0x08,1,{0x50}},
{0x09,1,{0x55}},
{0x0A,1,{0x52}},
{0x0B,1,{0x55}},
{0x0C,1,{0x55}},
{0x0D,1,{0x55}},
{0x0E,1,{0x55}},
{0x0F,1,{0x55}},
{0x10,1,{0x55}},
{0x11,1,{0x55}},
{0x12,1,{0x55}},
{0x13,1,{0x40}},
{0x14,1,{0x55}},
{0x15,1,{0x55}},

{0x16,1,{0x57}},
{0x17,1,{0x77}},
{0x18,1,{0x45}},
{0x19,1,{0x47}},
{0x1A,1,{0x49}},
{0x1B,1,{0x4B}},
{0x1C,1,{0x4D}},
{0x1D,1,{0x4F}},
{0x1E,1,{0x51}},
{0x1F,1,{0x55}},
{0x20,1,{0x53}},
{0x21,1,{0x55}},
{0x22,1,{0x55}},
{0x23,1,{0x55}},
{0x24,1,{0x55}},
{0x25,1,{0x55}},
{0x26,1,{0x55}},
{0x27,1,{0x55}},
{0x28,1,{0x55}},
{0x29,1,{0x41}},
{0x2A,1,{0x55}},
{0x2B,1,{0x55}},

{0x2C,1,{0x57}},
{0x2D,1,{0x77}},
{0x2E,1,{0x4F}},
{0x2F,1,{0x4D}},
{0x30,1,{0x4B}},
{0x31,1,{0x49}},
{0x32,1,{0x47}},
{0x33,1,{0x45}},
{0x34,1,{0x41}},
{0x35,1,{0x55}},
{0x36,1,{0x53}},
{0x37,1,{0x55}},
{0x38,1,{0x55}},
{0x39,1,{0x55}},
{0x3A,1,{0x55}},
{0x3B,1,{0x55}},
{0x3C,1,{0x55}},
{0x3D,1,{0x55}},
{0x3E,1,{0x55}},
{0x3F,1,{0x51}},
{0x40,1,{0x55}},
{0x41,1,{0x55}},

{0x42,1,{0x57}},
{0x43,1,{0x77}},
{0x44,1,{0x4E}},
{0x45,1,{0x4C}},
{0x46,1,{0x4A}},
{0x47,1,{0x48}},
{0x48,1,{0x46}},
{0x49,1,{0x44}},
{0x4A,1,{0x40}},
{0x4B,1,{0x55}},
{0x4C,1,{0x52}},
{0x4D,1,{0x55}},
{0x4E,1,{0x55}},
{0x4F,1,{0x55}},
{0x50,1,{0x55}},
{0x51,1,{0x55}},
{0x52,1,{0x55}},
{0x53,1,{0x55}},
{0x54,1,{0x55}},
{0x55,1,{0x50}},
{0x56,1,{0x55}},
{0x57,1,{0x55}},

{0x58,1,{0x40}},
{0x59,1,{0x00}},
{0x5A,1,{0x00}},
{0x5B,1,{0x10}},
{0x5C,1,{0x09}},
{0x5D,1,{0x30}},
{0x5E,1,{0x01}},
{0x5F,1,{0x02}},
{0x60,1,{0x30}},
{0x61,1,{0x03}},
{0x62,1,{0x04}},
{0x63,1,{0x06}},
{0x64,1,{0x6A}},
{0x65,1,{0x75}},
{0x66,1,{0x0F}},
{0x67,1,{0xB3}},
{0x68,1,{0x0B}},
{0x69,1,{0x06}},
{0x6A,1,{0x6A}},
{0x6B,1,{0x10}},
{0x6C,1,{0x00}},
{0x6D,1,{0x04}},
{0x6E,1,{0x04}},
{0x6F,1,{0x88}},
{0x70,1,{0x00}},
{0x71,1,{0x00}},
{0x72,1,{0x06}},
{0x73,1,{0x7B}},
{0x74,1,{0x00}},
{0x75,1,{0xBC}},
{0x76,1,{0x00}},
{0x77,1,{0x05}},
{0x78,1,{0x2E}},
{0x79,1,{0x00}},
{0x7A,1,{0x00}},
{0x7B,1,{0x00}},
{0x7C,1,{0x00}},
{0x7D,1,{0x03}},
{0x7E,1,{0x7B}},

{0xE0,1,{0x04}},
{0x09,1,{0x10}},
{0x2B,1,{0x2B}},
{0x2E,1,{0x44}},

{0xE0,1,{0x00}},
{0xE6,1,{0x02}},
{0xE7,1,{0x02}},
{0x35,1,{0x00}},

//SLP OUT
{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {}},
{0x29,1,{0x00}},
{REGFLAG_DELAY, 5, {}},
};
#endif

#endif
