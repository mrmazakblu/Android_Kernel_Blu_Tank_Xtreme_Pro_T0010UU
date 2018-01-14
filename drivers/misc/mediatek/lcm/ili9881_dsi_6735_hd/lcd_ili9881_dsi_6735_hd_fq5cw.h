#ifndef __LCM_ILI9881_DSI_6735_HD_FQ5CW
#define __LCM_ILI9881_DSI_6735_HD_FQ5CW

#if defined(DROI_PRO_FQ5CW_TW)||defined(DROI_PRO_FQ5CW_TW2)||defined(DROI_PRO_FQ5CW_NJX)
#define TYD_LCD_USE_CUSTOM_ILI9881C_HD
#endif

#if defined(TYD_PRO_FQ5C_XF)||defined(TYD_PRO_FQ5C_GS)
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,3,{0x98,0x81,0x03}},
{0x01,1,{0x00}},
{0x02,1,{0x00}},
{0x03,1,{0x72}},
{0x04,1,{0x00}},
{0x05,1,{0x00}},
{0x06,1,{0x09}},
{0x07,1,{0x00}},
{0x08,1,{0x00}},
{0x09,1,{0x01}},
{0x0A,1,{0x00}},
{0x0B,1,{0x00}},
{0x0C,1,{0x01}},
{0x0D,1,{0x00}},
{0x0E,1,{0x00}},
{0x0F,1,{0x00}},
{0x10,1,{0x00}},
{0x11,1,{0x00}},
{0x12,1,{0x00}},
{0x13,1,{0x00}},
{0x14,1,{0x00}},
{0x15,1,{0x00}},
{0x16,1,{0x00}},
{0x17,1,{0x00}},
{0x18,1,{0x00}},
{0x19,1,{0x00}},
{0x1A,1,{0x00}},
{0x1B,1,{0x00}},
{0x1C,1,{0x00}},
{0x1D,1,{0x00}},
{0x1E,1,{0x40}},
{0x1F,1,{0x80}},
{0x20,1,{0x05}},
{0x21,1,{0x02}},
{0x22,1,{0x00}},
{0x23,1,{0x00}},
{0x24,1,{0x00}},
{0x25,1,{0x00}},
{0x26,1,{0x00}},
{0x27,1,{0x00}},
{0x28,1,{0x33}},
{0x29,1,{0x02}},
{0x2A,1,{0x00}},
{0x2B,1,{0x00}},
{0x2C,1,{0x00}},
{0x2D,1,{0x00}},
{0x2E,1,{0x00}},
{0x2F,1,{0x00}},
{0x30,1,{0x00}},
{0x31,1,{0x00}},
{0x32,1,{0x00}},
{0x33,1,{0x00}},
{0x34,1,{0x04}},
{0x35,1,{0x00}},
{0x36,1,{0x00}},
{0x37,1,{0x00}},
{0x38,1,{0x3c}},
{0x39,1,{0x00}},
{0x3A,1,{0x40}},
{0x3B,1,{0x40}},
{0x3C,1,{0x00}},
{0x3D,1,{0x00}},
{0x3E,1,{0x00}},
{0x3F,1,{0x00}},
{0x40,1,{0x00}},
{0x41,1,{0x00}},
{0x42,1,{0x00}},
{0x43,1,{0x00}},
{0x44,1,{0x00}},
{0x50,1,{0x01}},
{0x51,1,{0x23}},
{0x52,1,{0x45}},
{0x53,1,{0x67}},
{0x54,1,{0x89}},
{0x55,1,{0xAB}},
{0x56,1,{0x01}},
{0x57,1,{0x23}},
{0x58,1,{0x45}},
{0x59,1,{0x67}},
{0x5A,1,{0x89}},
{0x5B,1,{0xAB}},
{0x5C,1,{0xCD}},
{0x5D,1,{0xEF}},
{0x5E,1,{0x11}},
{0x5F,1,{0x01}},
{0x60,1,{0x00}},
{0x61,1,{0x15}},
{0x62,1,{0x14}},
{0x63,1,{0x0E}},
{0x64,1,{0x0F}},
{0x65,1,{0x0C}},
{0x66,1,{0x0D}},
{0x67,1,{0x06}},
{0x68,1,{0x02}},
{0x69,1,{0x07}},
{0x6A,1,{0x02}},
{0x6B,1,{0x02}},
{0x6C,1,{0x02}},
{0x6D,1,{0x02}},
{0x6E,1,{0x02}},
{0x6F,1,{0x02}},
{0x70,1,{0x02}},
{0x71,1,{0x02}},
{0x72,1,{0x02}},
{0x73,1,{0x02}},
{0x74,1,{0x02}},
{0x75,1,{0x01}},
{0x76,1,{0x00}},
{0x77,1,{0x14}},
{0x78,1,{0x15}},
{0x79,1,{0x0E}},
{0x7A,1,{0x0F}},
{0x7B,1,{0x0C}},
{0x7C,1,{0x0D}},
{0x7D,1,{0x06}},
{0x7E,1,{0x02}},
{0x7F,1,{0x07}},
{0x80,1,{0x02}},
{0x81,1,{0x02}},
{0x82,1,{0x02}},
{0x83,1,{0x02}},
{0x84,1,{0x02}},
{0x85,1,{0x02}},
{0x86,1,{0x02}},
{0x87,1,{0x02}},
{0x88,1,{0x02}},
{0x89,1,{0x02}},
{0x8A,1,{0x02}},
{0xFF,3,{0x98,0x81,0x04}},
{0x6C,1,{0x15}},
{0x6E,1,{0x2A}},             //VGHclamp15V
{0x6F,1,{0x35}},
{0x3A,1,{0x94}},
{0x8D,1,{0x15}},             //VGLclamp-12V
{0x87,1,{0xBA}},
{0x26,1,{0x76}},
{0xB2,1,{0xD1}},
{0xB5,1,{0x06}},
{0xFF,3,{0x98,0x81,0x01}},
{0x22,1,{0x0A}},        //BGR,01,SS
{0x31,1,{0x00}},        //zig-zaginversion
{0x53,1,{0x8b}},
{0x55,1,{0x8F}},        //VCOM1
{0x50,1,{0x9a}},       //VCOM2
{0x51,1,{0x9a}},        //VREG1OUT=4.5V
{0x60,1,{0x22}},       //22        //VREG2OUT=-4.5V
{0x61,1,{0x00}},                 //SDT
{0x62,1,{0x19}},        //VP255GammaP
{0x63,1,{0x10}},                //VP251

{0xA0,1,{0x08}},        //VP247
{0xA1,1,{0x11}},        //VP243
{0xA2,1,{0x1a}},        //VP239
{0xA3,1,{0x01}},        //VP231
{0xA4,1,{0x1e}},        //VP219
{0xA5,1,{0x2e}},        //VP203
{0xA6,1,{0x20}},        //VP175
{0xA7,1,{0x22}},        //VP144
{0xA8,1,{0x53}},        //VP111
{0xA9,1,{0x24}},        //VP80
{0xAA,1,{0x2f}},        //VP52
{0xAB,1,{0x4a}},        //VP36
{0xAC,1,{0x22}},        //VP24
{0xAD,1,{0x1d}},        //VP16
{0xAE,1,{0x52}},        //VP12
{0xAF,1,{0x28}},        //VP8
{0xB0,1,{0x25}},        //VP4
{0xB1,1,{0x49}},        //VP0
{0xB2,1,{0x63}},        //VN255GAMMAN
{0xB3,1,{0x30}},        //VN251

{0xC0,1,{0x08}},        //VN247
{0xC1,1,{0x15}},        //VN243
{0xC2,1,{0x1f}},        //VN239
{0xC3,1,{0x1a}},        //VN231
{0xC4,1,{0x03}},        //VN219
{0xC5,1,{0x18}},        //VN203
{0xC6,1,{0x0f}},        //VN175
{0xC7,1,{0x17}},        //VN144
{0xC8,1,{0x65}},        //VN111
{0xC9,1,{0x16}},        //VN80
{0xCA,1,{0x21}},
{0xCB,1,{0x64}},        //VN36
{0xCC,1,{0x16}},        //VN24
{0xCD,1,{0x14}},        //VN16
{0xCE,1,{0x49}},        //VN12
{0xCF,1,{0x1b}},        //VN8
{0xD0,1,{0x2a}},        //VN4
{0xD1,1,{0x56}},        //VN0
{0xD2,1,{0x64}},
{0xD3,1,{0x35}},

{0xFF,3,{0x98,0x81,0x00}},
//{0x36,1,{0x08}},
{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {}},
{0x29,1,{0x00}},
{REGFLAG_DELAY, 50, {}},

{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
#if defined(TYD_PRO_FQ5C_GS2)
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,3,{0x98,0x81,0x03}},
{0x01,1,{0x00}},
{0x02,1,{0x00}},
{0x03,1,{0x72}},
{0x04,1,{0x00}},
{0x05,1,{0x00}},
{0x06,1,{0x09}},
{0x07,1,{0x00}},
{0x08,1,{0x00}},
{0x09,1,{0x01}},
{0x0A,1,{0x00}},
{0x0B,1,{0x00}},
{0x0C,1,{0x01}},
{0x0D,1,{0x00}},
{0x0E,1,{0x00}},
{0x0F,1,{0x00}},
{0x10,1,{0x00}},
{0x11,1,{0x00}},
{0x12,1,{0x00}},
{0x13,1,{0x00}},
{0x14,1,{0x00}},
{0x15,1,{0x00}},
{0x16,1,{0x00}},
{0x17,1,{0x00}},
{0x18,1,{0x00}},
{0x19,1,{0x00}},
{0x1A,1,{0x00}},
{0x1B,1,{0x00}},
{0x1C,1,{0x00}},
{0x1D,1,{0x00}},
{0x1E,1,{0x40}},
{0x1F,1,{0x80}},
{0x20,1,{0x05}},
{0x21,1,{0x02}},
{0x22,1,{0x00}},
{0x23,1,{0x00}},
{0x24,1,{0x00}},
{0x25,1,{0x00}},
{0x26,1,{0x00}},
{0x27,1,{0x00}},
{0x28,1,{0x33}},
{0x29,1,{0x02}},
{0x2A,1,{0x00}},
{0x2B,1,{0x00}},
{0x2C,1,{0x00}},
{0x2D,1,{0x00}},
{0x2E,1,{0x00}},
{0x2F,1,{0x00}},
{0x30,1,{0x00}},
{0x31,1,{0x00}},
{0x32,1,{0x00}},
{0x33,1,{0x00}},
{0x34,1,{0x04}},
{0x35,1,{0x00}},
{0x36,1,{0x00}},
{0x37,1,{0x00}},
{0x38,1,{0x3c}},
{0x39,1,{0x00}},
{0x3A,1,{0x40}},
{0x3B,1,{0x40}},
{0x3C,1,{0x00}},
{0x3D,1,{0x00}},
{0x3E,1,{0x00}},
{0x3F,1,{0x00}},
{0x40,1,{0x00}},
{0x41,1,{0x00}},
{0x42,1,{0x00}},
{0x43,1,{0x00}},
{0x44,1,{0x00}},
{0x50,1,{0x01}},
{0x51,1,{0x23}},
{0x52,1,{0x45}},
{0x53,1,{0x67}},
{0x54,1,{0x89}},
{0x55,1,{0xab}},
{0x56,1,{0x01}},
{0x57,1,{0x23}},
{0x58,1,{0x45}},
{0x59,1,{0x67}},
{0x5A,1,{0x89}},
{0x5B,1,{0xab}},
{0x5C,1,{0xcd}},
{0x5D,1,{0xef}},
{0x5E,1,{0x11}},
{0x5F,1,{0x01}},
{0x60,1,{0x00}},
{0x61,1,{0x15}},
{0x62,1,{0x14}},
{0x63,1,{0x0E}},
{0x64,1,{0x0F}},
{0x65,1,{0x0C}},
{0x66,1,{0x0D}},
{0x67,1,{0x06}},
{0x68,1,{0x02}},
{0x69,1,{0x02}},
{0x6A,1,{0x02}},
{0x6B,1,{0x02}},
{0x6C,1,{0x02}},
{0x6D,1,{0x02}},
{0x6E,1,{0x07}},
{0x6F,1,{0x02}},
{0x70,1,{0x02}},
{0x71,1,{0x02}},
{0x72,1,{0x02}},
{0x73,1,{0x02}},
{0x74,1,{0x02}},
{0x75,1,{0x01}},
{0x76,1,{0x00}},
{0x77,1,{0x14}},
{0x78,1,{0x15}},
{0x79,1,{0x0E}},
{0x7A,1,{0x0F}},
{0x7B,1,{0x0C}},
{0x7C,1,{0x0D}},
{0x7D,1,{0x06}},
{0x7E,1,{0x02}},
{0x7F,1,{0x02}},
{0x80,1,{0x02}},
{0x81,1,{0x02}},
{0x82,1,{0x02}},
{0x83,1,{0x02}},
{0x84,1,{0x07}},
{0x85,1,{0x02}},
{0x86,1,{0x02}},
{0x87,1,{0x02}},
{0x88,1,{0x02}},
{0x89,1,{0x02}},
{0x8A,1,{0x02}},
{0xFF,3,{0x98,0x81,0x04}},
{0x6C,1,{0x15}},
{0x6E,1,{0x2A}},
{0x6F,1,{0x33}},//33
{0x3A,1,{0x94}},
{0x8D,1,{0x1A}},
{0x87,1,{0xBA}},
{0x26,1,{0x76}},
{0xB2,1,{0xD1}},
{0xB5,1,{0x06}},
{0xFF,3,{0x98,0x81,0x01}},
{0x22,1,{0x0A}},
{0x31,1,{0x00}},
{0x53,1,{0x92}},  //8f
{0x55,1,{0x8F}},
{0x50,1,{0xAE}},
{0x51,1,{0xAE}},
{0x60,1,{0x28}},

{0xA0,1,{0x0f}},
{0xA1,1,{0x21}},
{0xA2,1,{0x2f}},
{0xA3,1,{0x14}},
{0xA4,1,{0x17}},
{0xA5,1,{0x29}},
{0xA6,1,{0x1e}},
{0xA7,1,{0x1f}},
{0xA8,1,{0x89}},
{0xA9,1,{0x1c}},
{0xAA,1,{0x27}},
{0xAB,1,{0x73}},
{0xAC,1,{0x1B}},
{0xAD,1,{0x1B}},
{0xAE,1,{0x4F}},
{0xAF,1,{0x24}},
{0xB0,1,{0x2A}},
{0xB1,1,{0x4B}},
{0xB2,1,{0x58}},
{0xB3,1,{0x3F}},
{0xC0,1,{0x04}},
{0xC1,1,{0x22}},
{0xC2,1,{0x30}},
{0xC3,1,{0x14}},
{0xC4,1,{0x17}},
{0xC5,1,{0x2A}},
{0xC6,1,{0x1E}},
{0xC7,1,{0x1E}},
{0xC8,1,{0x8A}},
{0xC9,1,{0x1B}},
{0xCA,1,{0x28}},
{0xCB,1,{0x74}},
{0xCC,1,{0x1C}},
{0xCD,1,{0x1B}},
{0xCE,1,{0x4F}},
{0xCF,1,{0x24}},
{0xD0,1,{0x2A}},
{0xD1,1,{0x4B}},
{0xD2,1,{0x58}},
{0xD3,1,{0x3F}},
{0xFF,3,{0x98,0x81,0x00}},
{0x11,01,{0x00}},
{REGFLAG_DELAY, 120, {}},
{0x29,01,{0x00}},
{REGFLAG_DELAY, 20, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

#if defined(DROI_PRO_FQ5CW_TW)
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,03,{0x98,81,03}},

{0x01,01,{0x00}},
{0x02,01,{0x00}},
{0x03,01,{0x73}},
{0x04,01,{0x00}},
{0x05,01,{0x00}},
{0x06,01,{0x0A}},
{0x07,01,{0x00}},
{0x08,01,{0x00}},
{0x09,01,{0x01}},
{0x0a,01,{0x00}},
{0x0b,01,{0x00}},
{0x0c,01,{0x01}},
{0x0d,01,{0x00}},
{0x0e,01,{0x00}},
{0x0f,01,{0x1D}},
{0x10,01,{0x1D}},
{0x11,01,{0x00}},
{0x12,01,{0x00}},
{0x13,01,{0x00}},
{0x14,01,{0x00}},

{0x15,01,{0x04}},
{0x16,01,{0x01}},
{0x17,01,{0x01}},

{0x18,01,{0x00}},
{0x19,01,{0x00}},
{0x1a,01,{0x00}},
{0x1b,01,{0x00}},
{0x1c,01,{0x00}},
{0x1d,01,{0x00}},
{0x1e,01,{0x40}},
{0x1f,01,{0x80}},
{0x20,01,{0x06}},
{0x21,01,{0x02}},
{0x22,01,{0x00}},
{0x23,01,{0x00}},
{0x24,01,{0x00}},
{0x25,01,{0x00}},
{0x26,01,{0x00}},
{0x27,01,{0x00}},
{0x28,01,{0x33}},
{0x29,01,{0x03}},
{0x2a,01,{0x00}},
{0x2b,01,{0x00}},
{0x2c,01,{0x00}},
{0x2d,01,{0x00}},
{0x2e,01,{0x00}},
{0x2f,01,{0x00}},
{0x30,01,{0x00}},
{0x31,01,{0x00}},
{0x32,01,{0x00}},
{0x33,01,{0x00}},
{0x34,01,{0x04}},
{0x35,01,{0x00}},
{0x36,01,{0x00}},
{0x37,01,{0x00}},
{0x38,01,{0x3C}},
{0x39,01,{0x00}},
{0x3a,01,{0x40}},
{0x3b,01,{0x40}},
{0x3c,01,{0x00}},
{0x3d,01,{0x00}},
{0x3e,01,{0x00}},
{0x3f,01,{0x00}},
{0x40,01,{0x00}},
{0x41,01,{0x00}},
{0x42,01,{0x00}},
{0x43,01,{0x00}},
{0x44,01,{0x00}},

{0x50,01,{0x01}},

{0x51,01,{0x23}},

{0x52,01,{0x45}},

{0x53,01,{0x67}},

{0x54,01,{0x89}},

{0x55,01,{0xab}},

{0x56,01,{0x01}},

{0x57,01,{0x23}},

{0x58,01,{0x45}},

{0x59,01,{0x67}},

{0x5a,01,{0x89}},

{0x5b,01,{0xab}},

{0x5c,01,{0xcd}},

{0x5d,01,{0xef}},


{0x5e,01,{0x11}},
{0x5f,01,{0x01}},
{0x60,01,{0x00}},
{0x61,01,{0x15}},
{0x62,01,{0x14}},
{0x63,01,{0x0E}},
{0x64,01,{0x0F}},
{0x65,01,{0x0C}},
{0x66,01,{0x0D}},
{0x67,01,{0x06}},
{0x68,01,{0x02}},
{0x69,01,{0x07}},
{0x6a,01,{0x02}},
{0x6b,01,{0x02}},
{0x6c,01,{0x02}},
{0x6d,01,{0x02}},
{0x6e,01,{0x02}},
{0x6f,01,{0x02}},
{0x70,01,{0x02}},
{0x71,01,{0x02}},
{0x72,01,{0x02}},
{0x73,01,{0x02}},
{0x74,01,{0x02}},
{0x75,01,{0x01}},
{0x76,01,{0x00}},
{0x77,01,{0x14}},
{0x78,01,{0x15}},
{0x79,01,{0x0E}},
{0x7a,01,{0x0F}},
{0x7b,01,{0x0C}},
{0x7c,01,{0x0D}},
{0x7d,01,{0x06}},
{0x7e,01,{0x02}},
{0x7f,01,{0x07}},
{0x80,01,{0x02}},
{0x81,01,{0x02}},
{0x82,01,{0x02}},
{0x83,01,{0x02}},
{0x84,01,{0x02}},
{0x85,01,{0x02}},
{0x86,01,{0x02}},
{0x87,01,{0x02}},
{0x88,01,{0x02}},
{0x89,01,{0x02}},
{0x8A,01,{0x02}},

{0xFF,3,{0x98,0x81,0x04}},
{0x6C,1,{0x15}},
{0x6E,1,{0x2A}},
{0x6F,1,{0x35}},
{0x8D,1,{0x14}},
{0x87,1,{0xBA}},
{0x26,1,{0x76}},
{0xB2,1,{0xD1}},
{0xB5,1,{0x06}},
{0x7A,1,{0x01}},

{0x3A,1,{0x24}},
{0x35,1,{0x1F}},
{0x32,1,{0x05}},
{0x33,1,{0x00}},

{0xFF,3,{0x98,0x81,0x01}},
{0x22,1,{0x0a}},
{0x31,1,{0x00}},
{0x53,1,{0x90}},  //0x90
{0x55,1,{0xA2}},
{0x50,1,{0xB7}},  //96
{0x51,1,{0xB7}},  //96
{0x60,1,{0x22}},
{0x61,1,{0x00}},
{0x62,1,{0x19}},
{0x63,1,{0x00}},

{0xA0,1,{0x08}},
{0xA1,1,{0x1A}},
{0xA2,1,{0x27}},
{0xA3,1,{0x15}},
{0xA4,1,{0x17}},
{0xA5,1,{0x2A}},
{0xA6,1,{0x1E}},
{0xA7,1,{0x1F}},
{0xA8,1,{0x8B}},
{0xA9,1,{0x1B}},
{0xAA,1,{0x27}},
{0xAB,1,{0x78}},
{0xAC,1,{0x18}},
{0xAD,1,{0x18}},
{0xAE,1,{0x4C}},
{0xAF,1,{0x21}},
{0xB0,1,{0x27}},
{0xB1,1,{0x54}},
{0xB2,1,{0x67}},
{0xB3,1,{0x39}},

{0xC0,1,{0x08}},
{0xC1,1,{0x1A}},
{0xC2,1,{0x27}},
{0xC3,1,{0x15}},
{0xC4,1,{0x17}},
{0xC5,1,{0x2A}},
{0xC6,1,{0x1E}},
{0xC7,1,{0x1F}},
{0xC8,1,{0x8B}},
{0xC9,1,{0x1B}},
{0xCA,1,{0x27}},
{0xCB,1,{0x78}},
{0xCC,1,{0x18}},
{0xCD,1,{0x18}},
{0xCE,1,{0x4C}},
{0xCF,1,{0x21}},
{0xD0,1,{0x27}},
{0xD1,1,{0x54}},
{0xD2,1,{0x67}},
{0xD3,1,{0x39}},
{0x60,1,{0x22}},

{0xFF,3,{0x98,0x81,0x00}},
{0x36,1,{0x03}},

{0x11,01,{0x00}},
{REGFLAG_DELAY, 120, {}},
{0x29,01,{0x00}},
{REGFLAG_DELAY, 20, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

#if defined(DROI_PRO_FQ5CW_TW2)
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,3,{0x98,0x81,0x03}},
//GIP_1
{0x01,1,{0x00}},
{0x02,1,{0x00}},
{0x03,1,{0x56}},
{0x04,1,{0x13}},
{0x05,1,{0x00}},
{0x06,1,{0x06}},
{0x07,1,{0x01}},
{0x08,1,{0x00}},
{0x09,1,{0x30}},
{0x0a,1,{0x01}},
{0x0b,1,{0x00}},
{0x0c,1,{0x30}},
{0x0d,1,{0x01}},
{0x0e,1,{0x00}},
{0x0f,1,{0x18}},
{0x10,1,{0x18}},
{0x11,1,{0x00}},
{0x12,1,{0x00}},
{0x13,1,{0x00}},
{0x14,1,{0x00}},
{0x15,1,{0x00}},
{0x16,1,{0x00}},
{0x17,1,{0x00}},
{0x18,1,{0x00}},
{0x19,1,{0x00}},
{0x1a,1,{0x00}},
{0x1b,1,{0x00}},
{0x1c,1,{0x00}},
{0x1d,1,{0x00}},
{0x1e,1,{0x40}},
{0x1f,1,{0xc0}},
{0x20,1,{0x02}},
{0x21,1,{0x05}},
{0x22,1,{0x02}},
{0x23,1,{0x00}},
{0x24,1,{0x86}},
{0x25,1,{0x85}},
{0x26,1,{0x00}},
{0x27,1,{0x00}},
{0x28,1,{0x3B}},
{0x29,1,{0x03}},
{0x2a,1,{0x00}},
{0x2b,1,{0x00}},
{0x2c,1,{0x00}},
{0x2d,1,{0x00}},
{0x2e,1,{0x00}},
{0x2f,1,{0x00}},
{0x30,1,{0x00}},
{0x31,1,{0x00}},
{0x32,1,{0x00}},
{0x33,1,{0x00}},
{0x34,1,{0x00}},
{0x35,1,{0x00}},
{0x36,1,{0x00}},
{0x37,1,{0x00}},
{0x38,1,{0x00}},
{0x39,1,{0x00}},
{0x3a,1,{0x00}},
{0x3b,1,{0x00}},
{0x3c,1,{0x00}},
{0x3d,1,{0x00}},
{0x3e,1,{0x00}},
{0x3f,1,{0x00}},
{0x40,1,{0x00}},
{0x41,1,{0x00}},
{0x42,1,{0x00}},
{0x43,1,{0x00}},
{0x44,1,{0x00}},
//GIP_2,
{0x50,1,{0x01}},
{0x51,1,{0x23}},
{0x52,1,{0x45}},
{0x53,1,{0x67}},
{0x54,1,{0x89}},
{0x55,1,{0xab}},
{0x56,1,{0x01}},
{0x57,1,{0x23}},
{0x58,1,{0x45}},
{0x59,1,{0x67}},
{0x5a,1,{0x89}},
{0x5b,1,{0xab}},
{0x5c,1,{0xcd}},
{0x5d,1,{0xef}},
//GIP_3
{0x5e,1,{0x11}},
{0x5f,1,{0x08}},
{0x60,1,{0x00}},
{0x61,1,{0x01}},
{0x62,1,{0x02}},
{0x63,1,{0x02}},
{0x64,1,{0x0f}},
{0x65,1,{0x0e}},
{0x66,1,{0x0d}},
{0x67,1,{0x0c}},
{0x68,1,{0x02}},
{0x69,1,{0x02}},
{0x6a,1,{0x02}},
{0x6b,1,{0x02}},
{0x6c,1,{0x02}},
{0x6d,1,{0x02}},
{0x6e,1,{0x06}},
{0x6f,1,{0x02}},
{0x70,1,{0x02}},
{0x71,1,{0x02}},
{0x72,1,{0x02}},
{0x73,1,{0x02}},
{0x74,1,{0x02}},
{0x75,1,{0x06}},
{0x76,1,{0x00}},
{0x77,1,{0x01}},
{0x78,1,{0x02}},
{0x79,1,{0x02}},
{0x7a,1,{0x0f}},
{0x7b,1,{0x0e}},
{0x7c,1,{0x0d}},
{0x7d,1,{0x0c}},
{0x7e,1,{0x02}},
{0x7f,1,{0x02}},
{0x80,1,{0x02}},
{0x81,1,{0x02}},
{0x82,1,{0x02}},
{0x83,1,{0x02}},
{0x84,1,{0x08}},
{0x85,1,{0x02}},
{0x86,1,{0x02}},
{0x87,1,{0x02}},
{0x88,1,{0x02}},
{0x89,1,{0x02}},
{0x8A,1,{0x02}},


//CMD_Page 4
{0xFF,3,{0x98,0x81,0x04}},//page 4 ÏÂÌíŒÓ {0x00,1,{0x80}},ÎªMIPI4ÍšµÀ,{0x00,1,{0x00}},Îª3ÍšµÀ£¬²»ÐŽÄ¬ÈÏÎª4ÍšµÀ
//{0x00,1,{0x00}},
{0x6C,1,{0x15}},
{0x6E,1,{0x32}},
{0x6F,1,{0x45}},
{0x8D,1,{0x1A}},
{0x87,1,{0xBA}},
{0x26,1,{0x76}},
{0xB2,1,{0xD1}},
{0xB5,1,{0x07}},
{0x35,1,{0x1F}},
{0x3A,1,{0x24}},


//CMD_Page 1
{0xFF,3,{0x98,0x81,0x01}},
{0x22,1,{0x3a}},
{0x53,1,{0x85}},
{0x55,1,{0x5C}},
{0x50,1,{0x95}},
{0x51,1,{0x95}},
{0x31,1,{0x00}},
{0x60,1,{0x30}},
{0x61,1,{0x00}},
{0x62,1,{0x19}},
{0x63,1,{0x10}},
//VP25	Gamma P
{0xA0,1,{0x08}},
//VP25
{0xA1,1,{0x1B}},
//VP247
{0xA2,1,{0x26}},
//VP243
{0xA3,1,{0x13}},
//VP239
{0xA4,1,{0x11}},
//VP231
{0xA5,1,{0x24}},
//VP219
{0xA6,1,{0x19}},
//VP203
{0xA7,1,{0x1A}},
//VP175
{0xA8,1,{0x74}},
//VP144
{0xA9,1,{0x1B}},
//VP11
{0xAA,1,{0x26}},
//VP80
{0xAB,1,{0x68}},
//VP52
{0xAC,1,{0x1A}},
//VP36
{0xAD,1,{0x18}},
//VP24
{0xAE,1,{0x4B}},
//VP16
{0xAF,1,{0x21}},
//VP12,
{0xB0,1,{0x27}},
//VP8
{0xB1,1,{0x4C}},
//VP4
{0xB2,1,{0x60}},
//VP0
{0xB3,1,{0x39}},
//VN255 GAMMA N,
{0xC0,1,{0x08}},
//VN251
{0xC1,1,{0x14}},
//VN247
{0xC2,1,{0x21}},
//VN243
{0xC3,1,{0x0E}},
//VN239
{0xC4,1,{0x14}},
//VN231,
{0xC5,1,{0x27}},
//VN219
{0xC6,1,{0x1B}},
//VN203
{0xC7,1,{0x1D}},
//VN175
{0xC8,1,{0x86}},
//VN14
{0xC9,1,{0x1C}},
//VN11
{0xCA,1,{0x2A}},
//VN80
{0xCB,1,{0x84}},
//VN52
{0xCC,1,{0x1C}},
//VN36
{0xCD,1,{0x1C}},
//VN24,
{0xCE,1,{0x4F}},
//VN16
{0xCF,1,{0x22}},
//VN12
{0xD0,1,{0x27}},
//VN8
{0xD1,1,{0x5C}},
//VN4
{0xD2,1,{0x6A}},
//VN0
{0xD3,1,{0x39}},



//CMD_Page 0,
{0xFF,3,{0x98,0x81,0x00}},

{0x35,1,{0x00}},

{0x11,1,{0x00}},		// Sleep-Out
{REGFLAG_DELAY, 150,  {}},

{0x29,1,{0x00}},
{REGFLAG_DELAY, 50,  {}},
{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
#if defined(DROI_PRO_FQ5CW_NJX)
static struct LCM_setting_table lcm_initialization_setting[] = {
   //ili9881d_3L_HSD5.0_0NCELL_kalaide
    {0xFF, 3,{0x98, 0x81, 0x03}},
    {0x01, 1,{0x00}},
    {0x02, 1,{0x00}},
    {0x03, 1,{0x73}},//72
    {0x04, 1,{0x00}},
    {0x05, 1,{0x00}},
    {0x06, 1,{0x0A}},//09
    {0x07, 1,{0x00}},
    {0x08, 1,{0x00}},
    {0x09, 1,{0x01}},
    {0x0A, 1,{0x00}},
    {0x0B, 1,{0x00}},
    {0x0C, 1,{0x01}},
    {0x0D, 1,{0x00}},
    {0x0E, 1,{0x00}},
    {0x0F, 1,{0x1D}},//00
    {0x10, 1,{0x1D}},//00
    {0x11, 1,{0x00}},
    {0x12, 1,{0x00}},
    {0x13, 1,{0x00}},
    {0x14, 1,{0x00}},
    {0x15, 1,{0x00}},
    {0x16, 1,{0x00}},
    {0x17, 1,{0x00}},
    {0x18, 1,{0x00}},
    {0x19, 1,{0x00}},
    {0x1A, 1,{0x00}},
    {0x1B, 1,{0x00}},
    {0x1C, 1,{0x00}},
    {0x1D, 1,{0x00}},
    {0x1E, 1,{0x40}},
    {0x1F, 1,{0x80}},
    {0x20, 1,{0x06}},//05
    {0x21, 1,{0x02}},
    {0x22, 1,{0x00}},
    {0x23, 1,{0x00}},
    {0x24, 1,{0x00}},
    {0x25, 1,{0x00}},
    {0x26, 1,{0x00}},
    {0x27, 1,{0x00}},
    {0x28, 1,{0x33}},
    {0x29, 1,{0x03}},//02
    {0x2A, 1,{0x00}},
    {0x2B, 1,{0x00}},
    {0x2C, 1,{0x00}},
    {0x2D, 1,{0x00}},
    {0x2E, 1,{0x00}},
    {0x2F, 1,{0x00}},
    {0x30, 1,{0x00}},
    {0x31, 1,{0x00}},
    {0x32, 1,{0x00}},
    {0x33, 1,{0x00}},
    {0x34, 1,{0x04}},
    {0x35, 1,{0x00}},
    {0x36, 1,{0x00}},
    {0x37, 1,{0x00}},
    {0x38, 1,{0x3C}},
    {0x39, 1,{0x35}},//00
    {0x3A, 1,{0x01}},//40
    {0x3B, 1,{0x40}},//40
    {0x3C, 1,{0x00}},
    {0x3D, 1,{0x00}},
    {0x3E, 1,{0x00}},
    {0x3F, 1,{0x00}},
    {0x40, 1,{0x00}},
    {0x41, 1,{0x88}},//00
    {0x42, 1,{0x00}},
    {0x43, 1,{0x00}},
    {0x44, 1,{0x1F}},//00
    
    {0x50, 1,{0x01}},
    {0x51, 1,{0x23}},
    {0x52, 1,{0x45}},
    {0x53, 1,{0x67}},
    {0x54, 1,{0x89}},
    {0x55, 1,{0xAB}},
    {0x56, 1,{0x01}},
    {0x57, 1,{0x23}},
    {0x58, 1,{0x45}},
    {0x59, 1,{0x67}},
    {0x5A, 1,{0x89}},
    {0x5B, 1,{0xAB}},
    {0x5C, 1,{0xCD}},
    {0x5D, 1,{0xEF}},
    {0x5E, 1,{0x11}},
    {0x5F, 1,{0x01}},
    {0x60, 1,{0x00}},
    {0x61, 1,{0x15}},
    {0x62, 1,{0x14}},
    {0x63, 1,{0x0E}},
    {0x64, 1,{0x0F}},
    {0x65, 1,{0x0C}},
    {0x66, 1,{0x0D}},
    {0x67, 1,{0x06}},
    {0x68, 1,{0x02}},
    {0x69, 1,{0x07}},
    {0x6A, 1,{0x02}},
    {0x6B, 1,{0x02}},
    {0x6C, 1,{0x02}},
    {0x6D, 1,{0x02}},
    {0x6E, 1,{0x02}},
    {0x6F, 1,{0x02}},
    {0x70, 1,{0x02}},
    {0x71, 1,{0x02}},
    {0x72, 1,{0x02}},
    {0x73, 1,{0x02}},
    {0x74, 1,{0x02}},
    {0x75, 1,{0x01}},
    {0x76, 1,{0x00}},
    {0x77, 1,{0x14}},
    {0x78, 1,{0x15}},
    {0x79, 1,{0x0E}},
    {0x7A, 1,{0x0F}},
    {0x7B, 1,{0x0C}},
    {0x7C, 1,{0x0D}},
    {0x7D, 1,{0x06}},
    {0x7E, 1,{0x02}},
    {0x7F, 1,{0x07}},
    {0x80, 1,{0x02}},
    {0x81, 1,{0x02}},
    {0x82, 1,{0x02}},
    {0x83, 1,{0x02}},
    {0x84, 1,{0x02}},
    {0x85, 1,{0x02}},
    {0x86, 1,{0x02}},
    {0x87, 1,{0x02}},
    {0x88, 1,{0x02}},
    {0x89, 1,{0x02}},
    {0x8A, 1,{0x02}},

    {0xFF, 3,{0x98, 0x81, 0x04}},
    {0x00, 1,{0x80}},   //4lane 3lane 0x00
    {0x70, 1,{0x00}},
    {0x71, 1,{0x00}},
    {0x82, 1,{0x0F}},
    {0x84, 1,{0x0F}},
    {0x85, 1,{0x0D}},
    {0x32, 1,{0xAC}},
    {0x8C, 1,{0x80}},
    {0x3C, 1,{0xF5}},
    {0xB5, 1,{0x07}},
    {0x31, 1,{0x45}},
    {0x3A, 1,{0x24}},
    {0x88, 1,{0x33}},

    {0xFF, 3,{0x98, 0x81, 0x01}},
    {0x22, 1,{0x0A}},//0A
    {0x31, 1,{0x00}},
    {0x53, 1,{0x8A}},
    {0x55, 1,{0x8A}},
    {0x50, 1,{0x85}},
    {0x51, 1,{0x85}},
    {0x62, 1,{0x0D}},

    {0xA0, 1,{0x08}},
    {0xA1, 1,{0x1A}},
    {0xA2, 1,{0x27}},
    {0xA3, 1,{0x15}},
    {0xA4, 1,{0x17}},
    {0xA5, 1,{0x2A}},
    {0xA6, 1,{0x1E}},
    {0xA7, 1,{0x1F}},
    {0xA8, 1,{0x8B}},
    {0xA9, 1,{0x1B}},
    {0xAA, 1,{0x27}},
    {0xAB, 1,{0x78}},
    {0xAC, 1,{0x18}},
    {0xAD, 1,{0x18}},
    {0xAE, 1,{0x4C}},
    {0xAF, 1,{0x21}},
    {0xB0, 1,{0x27}},
    {0xB1, 1,{0x54}},
    {0xB2, 1,{0x67}},
    {0xB3, 1,{0x39}},
    {0xC0, 1,{0x08}},
    {0xC1, 1,{0x1A}},
    {0xC2, 1,{0x27}},
    {0xC3, 1,{0x15}},
    {0xC4, 1,{0x17}},
    {0xC5, 1,{0x2A}},
    {0xC6, 1,{0x1E}},
    {0xC7, 1,{0x1F}},
    {0xC8, 1,{0x8B}},
    {0xC9, 1,{0x1B}},
    {0xCA, 1,{0x27}},
    {0xCB, 1,{0x78}},
    {0xCC, 1,{0x18}},
    {0xCD, 1,{0x18}},
    {0xCE, 1,{0x4C}},
    {0xCF, 1,{0x21}},
    {0xD0, 1,{0x27}},
    {0xD1, 1,{0x54}},
    {0xD2, 1,{0x67}},
    {0xD3, 1,{0x39}},
    {0xFF, 3,{0x98, 0x81, 0x00}},
    {0x35, 1,{0x00}},

    {0x11,1,{0x00}},
    {REGFLAG_DELAY, 120, {}},
    {0x29,1,{0x00}},
    {REGFLAG_DELAY, 50, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
#endif