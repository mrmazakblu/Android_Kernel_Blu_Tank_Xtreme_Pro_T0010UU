#ifndef __LCD_NT35523_DSI_HD_PF5_H__
#define __LCD_NT35523_DSI_HD_PF5_H__

#if defined(DROI_PRO_PF5_A7)
#define SUPORT_ADC_CHECK
#define DROI_LCD_USE_CUSTOM_NT35523_HD
#endif

#if defined(DROI_PRO_PF5_A7)
static struct LCM_setting_table lcm_initialization_setting_0400[] = {
	// hongli LD050HF1L01+NT35523 20161124 glass:PANDA	
	{0xFF,4,{0xAA,0x55,0xA5,0x80}},
	{0xF3,1,{0xC0}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
    {0xC0,9,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02}},
    {0xC8,1,{0x80}},

    {0xB1,2,{0xE8,0x21}},
    {0xB5,2,{0x05,0x00}},
    {0xBB,2,{0x93,0x93}},
    {0xBC,2,{0x0F,0x00}},
    {0xBD,4,{0x11,0x30,0x10,0x10}},
                  
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
    {0xCE,1,{0x00}},
    {0xD7,2,{0x00,0xFF}},
    {0xB7,2,{0x00,0x6C}},

    {0xCE,1,{0x00}},
    {0xCA,1,{0x03}},

    {0xB3,2,{0x23,0x23}},
    {0xB4,2,{0x23,0x23}},
    {0xC3,2,{0x5A,0x5A}},
    {0xC4,2,{0x5A,0x5A}},
    {0xC2,2,{0x5A,0x5A}},
    {0xB9,2,{0x34,0x34}},
    {0xBA,2,{0x34,0x34}},
    {0xBC,2,{0x50,0x00}},
    {0xBD,2,{0x50,0x00}},    
    {0xBE,2,{0x00,0x71}},
    {0xBF,2,{0x00,0x71}},

    {0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
    {0xB0,1,{0x40}},
    {0xD1,16,{0x00,0x00,0x00,0x1B,0x00,0x40,0x00,0x5B,0x00,0x71,0x00,0x97,0x00,0xB5,0x00,0xE6}},
    {0xD2,16,{0x01,0x0D,0x01,0x4B,0x01,0x7C,0x01,0xC8,0x02,0x03,0x02,0x05,0x02,0x3A,0x02,0x72}},
    {0xD3,16,{0x02,0x95,0x02,0xC5,0x02,0xE7,0x03,0x15,0x03,0x32,0x03,0x57,0x03,0x71,0x03,0x95}},
    {0xD4,4,{0x03,0xB0,0x03,0xB3}},

    {0xF0,5,{0x55,0xAA,0x52,0x08,0x03}}, 
    {0xB0,4,{0x00,0x00,0x00,0x00}},
    {0xB1,4,{0x00,0x00,0x00,0x00}}, 
    {0xB2,7,{0x00,0x00,0x0A,0x06,0x00,0xF0,0x5B}},
    {0xB3,7,{0x00,0x00,0x09,0x06,0x00,0xF0,0x5B}},
    {0xB6,10,{0xF0,0x05,0x06,0x03,0x00,0x00,0x00,0x00,0x10,0x10}},
    {0xB7,10,{0xF0,0x05,0x07,0x03,0x00,0x00,0x00,0x00,0x10,0x10}}, 
    {0xBC,7,{0xC5,0x03,0x00,0x08,0x00,0xF0,0x7F}},
    {0xC4,2,{0x00,0x00}},

    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},  
    {0xB0,4,{0x33,0x04,0x00,0x01}},
    {0xB1,2,{0x40,0x00}},  
    {0xB2,3,{0x03,0x02,0x22}},
    {0xB3,4,{0x83,0x23,0x42,0x9A}},
    {0xB4,4,{0xC5,0x35,0x77,0x53}},
    {0xB5,7,{0x4C,0xE5,0x31,0x33,0x33,0xA3,0x0A}},
    {0xB6,6,{0x00,0x00,0xD5,0x31,0x77,0x53}},
    {0xB9,5,{0x00,0x00,0x00,0x05,0x00}},
    {0xC0,5,{0x35,0x33,0x33,0x50,0x05}},
    {0xC6,4,{0x00,0x00,0x00,0x00}},
    {0xCE,2,{0xF0,0x1F}},  

    {0xD2,5,{0x00,0x25,0x02,0x00,0x00}},
    {0xE7,2,{0xE8,0xFF}},
    {0xE8,2,{0xFF,0xFF}},
    {0xE9,1,{0x00}},
    {0xEA,1,{0xAA}},
    {0xEB,1,{0xAA}},
    {0xEC,1,{0xAA}},
    {0xEE,1,{0xAA}},

    {0xF0,5,{0x55,0xAA,0x52,0x08,0x06}}, 
    {0xB0,5,{0x7D,0x4A,0x7D,0x7D,0x7D}},
    {0xB1,5,{0x7D,0x7D,0x42,0x5d,0x7D}},  
    {0xB2,5,{0x7D,0x63,0x61,0x7D,0x7D}},   
    {0xB3,5,{0x5f,0x72,0x7d,0x7D,0x7D}},   
    {0xB4,2,{0x7D,0x7D}},                
    {0xB5,5,{0x7D,0x48,0x7D,0x7D,0x7D}},
    {0xB6,5,{0x7D,0x7D,0x40,0x5c,0x7D}},          
    {0xB7,5,{0x7D,0x62,0x60,0x7D,0x7D}},        
    {0xB8,5,{0x5e,0x72,0x7d,0x7D,0x7D}},        
    {0xB9,2,{0x7D,0x7D}},
    
	{0x35,1,{0x00}},
	//{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
    //{0xEE,4,{0x87,0x78,0x02,0x40}},
    //{0xEF,3,{0x00,0xFF,0xFF}},
    
	{REGFLAG_DELAY, 100, {}},
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 100, {}},
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_initialization_setting_0800[] = {
	//zhanjie LD050HF1L01+NT35523 20161124 glass:PANDA
	{0xFF,4,{0xAA,0x55,0xA5,0x80}},
	{0xF3,1,{0xC0}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
    {0xC0,9,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02}},
    {0xC8,1,{0x80}},
    {0xC6,2,{0x41,0x18}},

    {0xB1,2,{0xE8,0x21}},
    {0xB5,2,{0x05,0x00}},
    {0xBB,2,{0x93,0x93}},
    {0xBC,2,{0x0F,0x00}},
    {0xBD,4,{0x11,0x30,0x10,0x10}},
                  
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
    {0xC0,1,{0x00}},
    {0xB5,2,{0x08,0x08}},
    {0xD7,2,{0x00,0xFF}},
    {0xB7,2,{0x00,0x6C}},

    {0xCE,1,{0x00}},
    {0xCA,1,{0x03}},

    {0xB3,2,{0x23,0x23}},
    {0xB4,2,{0x23,0x23}},
    {0xC3,2,{0x5A,0x5A}},
    {0xC4,2,{0x5A,0x5A}},
    {0xC2,2,{0x5A,0x5A}},
    {0xB9,2,{0x34,0x34}},
    {0xBA,2,{0x34,0x34}},
    {0xBC,2,{0x50,0x00}},
    {0xBD,2,{0x50,0x00}},    
    {0xBE,2,{0x00,0x71}},
    {0xBF,2,{0x00,0x71}},

    {0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
    {0xB0,1,{0x40}},
    {0xD1,16,{0x00,0x0A,0x00,0x1a,0x00,0x42,0x00,0x5e,0x00,0x71,0x00,0x91,0x00,0xb8,0x00,0xE8}},
    {0xD2,16,{0x01,0x0f,0x01,0x4c,0x01,0x7d,0x01,0xC8,0x02,0x03,0x02,0x05,0x02,0x3b,0x02,0x73}},
    {0xD3,16,{0x02,0x98,0x02,0xCb,0x02,0xef,0x03,0x1e,0x03,0x44,0x03,0x60,0x03,0x68,0x03,0x6d}},
    {0xD4,4,{0x03,0x6f,0x03,0xAD}},
/*
    {0xD1,16,{0x00,0x00,0x00,0x19,0x00,0x3b,0x00,0x57,0x00,0x6e,0x00,0x8a,0x00,0xac,0x00,0xE1}},
    {0xD2,16,{0x01,0x08,0x01,0x45,0x01,0x76,0x01,0xC2,0x02,0x00,0x02,0x02,0x02,0x38,0x02,0x72}},
    {0xD3,16,{0x02,0x96,0x02,0xCb,0x02,0xf1,0x03,0x22,0x03,0x4a,0x03,0x6f,0x03,0x8b,0x03,0xa7}},
    {0xD4,4,{0x03,0xB2,0x03,0xB3}},
*/
/*
    {0xD1,16,{0x00,0x00,0x00,0x1B,0x00,0x40,0x00,0x5B,0x00,0x71,0x00,0x97,0x00,0xB5,0x00,0xE6}},
    {0xD2,16,{0x01,0x0D,0x01,0x4B,0x01,0x7C,0x01,0xC8,0x02,0x03,0x02,0x05,0x02,0x3A,0x02,0x72}},
    {0xD3,16,{0x02,0x95,0x02,0xC5,0x02,0xE7,0x03,0x15,0x03,0x32,0x03,0x57,0x03,0x71,0x03,0x95}},
    {0xD4,4,{0x03,0xB0,0x03,0xB3}},
*/
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x03}}, 
    {0xB0,4,{0x00,0x00,0x00,0x00}},
    {0xB1,4,{0x00,0x00,0x00,0x00}}, 
    {0xB2,7,{0x00,0x00,0x0A,0x06,0x00,0xF0,0x5B}},
    {0xB3,7,{0x00,0x00,0x09,0x06,0x00,0xF0,0x5B}},
    {0xB6,10,{0xF0,0x05,0x06,0x03,0x00,0x00,0x00,0x00,0x10,0x10}},
    {0xB7,10,{0xF0,0x05,0x07,0x03,0x00,0x00,0x00,0x00,0x10,0x10}}, 
    {0xBC,7,{0xC5,0x03,0x00,0x08,0x00,0xF0,0x7F}},
    {0xC4,2,{0x00,0x00}},

    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},  
    {0xB0,4,{0x33,0x04,0x00,0x01}},
    {0xB1,2,{0x40,0x00}},  
    {0xB2,3,{0x03,0x02,0x22}},
    {0xB3,4,{0x83,0x23,0x42,0x9A}},
    {0xB4,4,{0xC5,0x35,0x77,0x53}},
    {0xB5,7,{0x4C,0xE5,0x31,0x33,0x33,0xA3,0x0A}},
    {0xB6,6,{0x00,0x00,0xD5,0x31,0x77,0x53}},
    {0xB9,5,{0x00,0x00,0x00,0x05,0x00}},
    {0xC0,5,{0x35,0x33,0x33,0x50,0x05}},
    {0xC6,4,{0x00,0x00,0x00,0x00}},
    {0xCE,2,{0xF0,0x1F}},  

    {0xD2,5,{0x00,0x25,0x02,0x00,0x00}},
    {0xE7,2,{0xE8,0xFF}},
    {0xE8,2,{0xFF,0xFF}},
    {0xE9,1,{0x00}},
    {0xEA,1,{0xAA}},
    {0xEB,1,{0xAA}},
    {0xEC,1,{0xAA}},
    {0xEE,1,{0xAA}},

    {0xF0,5,{0x55,0xAA,0x52,0x08,0x06}}, 
    {0xB0,5,{0x7D,0x4A,0x7D,0x7D,0x7D}},
    {0xB1,5,{0x7D,0x7D,0x42,0x5d,0x7D}},  
    {0xB2,5,{0x7D,0x63,0x61,0x7D,0x7D}},   
    {0xB3,5,{0x5f,0x72,0x7d,0x7D,0x7D}},   
    {0xB4,2,{0x7D,0x7D}},                
    {0xB5,5,{0x7D,0x48,0x7D,0x7D,0x7D}},
    {0xB6,5,{0x7D,0x7D,0x40,0x5c,0x7D}},          
    {0xB7,5,{0x7D,0x62,0x60,0x7D,0x7D}},        
    {0xB8,5,{0x5e,0x72,0x7d,0x7D,0x7D}},        
    {0xB9,2,{0x7D,0x7D}},
    
	{0x35,1,{0x00}},
	//{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
    //{0xEE,4,{0x87,0x78,0x02,0x40}},
    //{0xEF,3,{0x00,0xFF,0xFF}},
    
	{REGFLAG_DELAY, 100, {}},
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 100, {}},
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

#endif /*__LCD_NT35523_DSI_HD_PF5_H__*/