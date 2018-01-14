/*****************************************************************************
 *
 * Filename:
 * ---------
 *     GC8034mipi_Sensor.c
 *
 * Project:
 * --------
 *     ALPS
 *
 * Description:
 * ------------
 *     Source code of Sensor driver
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
//#include <asm/system.h>
//#include <linux/xlog.h>
#include "kd_camera_typedef.h"
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "gc8034mipi_Sensor.h"

/****************************Modify Following Strings for Debug****************************/
#define PFX "GC8034_camera_sensor"
#define LOG_1 LOG_INF("GC8034,MIPI 4LANE\n")
#define LOG_2 LOG_INF("preview 1632*1224@30fps,216Mbps/lane; video 1632*1224@30fps,720Mbps/lane; capture 8M@30fps,216Mbps/lane\n")
/****************************   Modify end    *******************************************/

#define GC8034_DEFAULT_DUMMY_PIXEL_NUMS   0x1f0 // HB
#define GC8034_DEFAULT_DUMMY_LINE_NUMS    0x10 //VB

#define LOG_INF(format, args...)    pr_debug(PFX "[%s] " format, __FUNCTION__, ##args)

#define IMAGE_NORMAL_MIRROR_CUSTOME
//#define IMAGE_HV_MIRROR_CUSTOME
//#define IMAGE_H_MIRROR_CUSTOME
//#define IMAGE_V_MIRROR_CUSTOME

#ifdef IMAGE_NORMAL_MIRROR_CUSTOME
#define MIRROR 		  0xc0
#define FullStartY     0x08
#define FullStartX     0x09
#endif

#ifdef IMAGE_HV_MIRROR_CUSTOME
#define MIRROR  	  0xc3
#define FullStartY     0x09
#define FullStartX     0x08

#endif
#ifdef IMAGE_NORMAL_MIRROR_CUSTOME
#define MIRROR 		  0xc0
#define FullStartY     0x08
#define FullStartX     0x09
#endif

#ifdef IMAGE_H_MIRROR_CUSTOME
#define MIRROR 		  0xc1
#define FullStartY     0x08
#define FullStartX     0x08

#endif

#ifdef IMAGE_V_MIRROR_CUSTOME
#define MIRROR 		  0xc2
#define FullStartY     0x09
#define FullStartX     0x09

#endif



#define ANALOG_GAIN_1 64   // 1.00x
#define ANALOG_GAIN_2 88   // 1.38x
#define ANALOG_GAIN_3 125  // 1.95x
#define ANALOG_GAIN_4 173  // 2.70x
#define ANALOG_GAIN_5 243 // 3.80x
#define ANALOG_GAIN_6 345  // 5.40x
#define ANALOG_GAIN_7 490 //7.66x
#define ANALOG_GAIN_8 684 // 10.69x
#define ANALOG_GAIN_9 962  //15.03x
#define ANALOG_GAIN_10 1450  // 22.66x
#define ANALOG_GAIN_11 2072  // 32.38x
#define ANALOG_GAIN_12 2900  // 45.31x
#define ANALOG_GAIN_13 4144  // 64.76x

kal_bool GC8034DuringTestPattern = KAL_FALSE;

//static kal_uint8 PreorCap = 0;
static kal_uint8 gainlevel = 0;
static kal_uint8 Val09[9] =
{0x10,0x00,0xe0,0x70,0x80,0x70,0x70,0xb0,0xb0};
static kal_uint8 Val20[9] =
{0x75,0x73,0x74,0x72,0x76,0x74,0x76,0x71,0x70};
static kal_uint8 Val33[9] =
{0xa3,0xa3,0xa4,0xa2,0xa3,0xa2,0xa1,0x9b,0x9a};
static kal_uint8 Valdf[9] =
{0x07,0x0e,0x0d,0x0e,0x08,0x04,0x0a,0x0e,0x0d};
static kal_uint8 Vale7[9] =
{0x15,0x21,0x21,0x2a,0x22,0x26,0x25,0x31,0x39};
static kal_uint8 Vale8[9] =
{0x1b,0x30,0x2b,0x35,0x2b,0x35,0x35,0x31,0x3b};
static kal_uint8 Vale9[9] =
{0x1a,0x18,0x15,0x21,0x22,0x22,0x25,0x37,0x39};
static kal_uint8 Valea[9] =
{0x20,0x18,0x1c,0x30,0x26,0x25,0x25,0x31,0x34};
static kal_uint8 Valeb[9] =
{0x50,0x4b,0x50,0x4c,0x53,0x4e,0x50,0x4c,0x49};
static kal_uint8 Valec[9] =
{0x6a,0x76,0x74,0x72,0x6b,0x68,0x66,0x60,0x61};
static kal_uint8 Valed[9] =
{0x9c,0x99,0x9f,0xa0,0x9b,0x99,0x93,0x8b,0x83};
static kal_uint8 Valee[9] =
{0xee,0xdb,0xe2,0xf9,0xd8,0xd1,0xc2,0xbc,0xb1};


//#define GC8034OTP_ENABLE
#ifdef GC8034OTP_ENABLE
	#define GC8034OTP_FOR_CUSTOMER
	#define GC8034OTP_DEBUG
#endif

static DEFINE_SPINLOCK(imgsensor_drv_lock);
static imgsensor_info_struct imgsensor_info = {
    .sensor_id = GC8034MIPI_SENSOR_ID,        //record sensor id defined in Kd_imgsensor.h

	.checksum_value = 0xb8666fa5,		//checksum value for Camera Auto Test

	.pre = {
		.pclk = 80000000,				//record different mode's pclk
		.linelength = 2136,				//record different mode's linelength
        .framelength = 1250,            //record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width = 3264,		//record different mode's width of grabwindow
		.grabwindow_height = 2448,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 80000000,
		.linelength = 2136,
        .framelength = 1250,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,
	},
	.cap1 = {							//capture for PIP 24fps relative information, capture1 mode must use same framelength, linelength with Capture mode for shutter calculate
        .pclk = 80000000,
		.linelength = 2136,
        .framelength = 1250,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,	//less than 13M(include 13M),cap1 max framerate is 24fps,16M max framerate is 20fps, 20M max framerate is 15fps
	},
	.normal_video = {
		.pclk = 80000000,
		.linelength = 2136,
        .framelength = 1250,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 80000000,
		.linelength = 2136,
        .framelength = 1250,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 1200,
	},
	.slim_video = {
		.pclk = 80000000,
		.linelength = 2136,
        .framelength = 1250,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,

	},
	.margin = 0,			//sensor framelength & shutter margin
    .min_shutter = 1,        //min shutter
	.max_frame_length = 0x7fff,//max framelength by sensor register's limitation
	.ae_shut_delay_frame = 0,	//shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2
	.ae_sensor_gain_delay_frame = 0,//sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2
	.ae_ispGain_delay_frame = 2,//isp gain delay frame for AE cycle
	.ihdr_support = 0,	  //1, support; 0,not support
	.ihdr_le_firstline = 0,  //1,le first ; 0, se first
	.sensor_mode_num = 5,	  //support sensor mode num

	.cap_delay_frame = 2,		//enter capture delay frame num
	.pre_delay_frame = 2, 		//enter preview delay frame num
	.video_delay_frame = 2,		//enter video delay frame num
	.hs_video_delay_frame = 2,	//enter high speed video  delay frame num
	.slim_video_delay_frame = 2,//enter slim video delay frame num

    .isp_driving_current = ISP_DRIVING_6MA, //mclk driving current
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,//sensor_interface_type
	.mipi_sensor_type = MIPI_OPHY_NCSI2, //0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2
    .mipi_settle_delay_mode = MIPI_SETTLEDELAY_AUTO,//0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_R,//sensor output first pixel color
	.mclk = 24,//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
	.mipi_lane_num = SENSOR_MIPI_2_LANE,//mipi lane num
	.i2c_addr_table = {0x6e,0xff},//record sensor support all write id addr, only supprt 4must end with 0xff
};


static imgsensor_struct imgsensor = {
	.mirror = IMAGE_HV_MIRROR, //IMAGE_NORMAL,				//mirrorflip information
	.sensor_mode = IMGSENSOR_MODE_INIT, //IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video,High Speed Video, Slim Video
    .shutter = 0x3ED,                    //current shutter
    .gain = 0x40,                        //current gain
	.dummy_pixel = 0,					//current dummypixel
	.dummy_line = 0,					//current dummyline
	.current_fps = 300,  //full size current fps : 24fps for PIP, 30fps for Normal or ZSD
	.autoflicker_en = KAL_FALSE,  //auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker
	.test_pattern = KAL_FALSE,		//test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,//current scenario id
	.ihdr_en = 0, //sensor need support LE, SE with HDR feature
	.i2c_write_id = 0x6e,//record current sensor's i2c write id
};


/* Sensor output window information */
static SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[5] =
{
	{ 3264, 2448,	 0,    0, 3264, 2448, 3264,  2448,	  0,	0, 3264,  2448, 	 0,    0, 3264,  2448}, // Preview
	{ 3264, 2448,	 0,    0, 3264, 2448, 3264,  2448,	  0,	0, 3264,  2448, 	 0,    0, 3264,  2448}, // capture
	{ 3264, 2448,	 0,    0, 3264, 2448, 3264,  2448,	  0,	0, 3264,  2448, 	 0,    0, 3264,  2448}, // video
	{ 3264, 2448,	 0,    0, 3264, 2448, 3264,  2448,	  0,	0, 3264,  2448, 	 0,    0, 3264,  2448}, // hight speed video
	{ 3264, 2448,	 0,    0, 3264, 2448, 3264,  2448,	  0,	0, 3264,  2448, 	 0,    0, 3264,  2448}  // slim video
 };




static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[1] = {(char)(addr & 0xFF) };
	iReadRegI2C(pu_send_cmd, 1, (u8*)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;

}

static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
#if 1
		char pu_send_cmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
		iWriteRegI2C(pu_send_cmd, 2, imgsensor.i2c_write_id);
#else
		iWriteReg((u16)addr, (u32)para, 2, imgsensor.i2c_write_id);
#endif

}



#define DD_PARAM_QTY 		350
#define WINDOW_WIDTH  		0x0cd4 //3284 max effective pixels
#define WINDOW_HEIGHT 		0x09a0 //2462
#define REG_ROM_START 		0x4e
#ifdef GC8034OTP_FOR_CUSTOMER
#define RG_TYPICAL    		0x0400
#define BG_TYPICAL			0x0400
#define INFO_ROM_START		0x70
#define INFO_WIDTH       	0x08
#define WB_ROM_START      	0x5f
#define WB_WIDTH          	0x04
#define GOLDEN_ROM_START  	0x67  //golden R/G ratio
#define GOLDEN_WIDTH      	0x04
#define LSC_NUM      		99//0x63 //(7+2)*(9+2)
#endif

#ifdef GC8034OTP_ENABLE
static kal_uint8 LSC_ADDR[4]={0x0e,0x20,0x1a,0x88};
#endif
/*
#define LSC_GROUP1_PAGE 0x03
#define LSC_GROUP1_ADDR 0x220
#define LSC_GROUP2_PAGE 0x06
#define LSC_GROUP2_ADDR 0x288

static kal_uint8 LSC_ADDR[4]={LSC_GROUP1_PAGE<<2+LSC_GROUP1_ADDR>>8,0x20,0x1a,0x88};
*/
#ifdef GC8034OTP_ENABLE
typedef struct otp_gc8034
{
	kal_uint16 dd_cnt;
	kal_uint16 dd_flag;
#ifdef GC8034OTP_FOR_CUSTOMER
	kal_uint16 module_id;
	kal_uint16 lens_id;
	kal_uint16 vcm_id;
	kal_uint16 vcm_driver_id;
	kal_uint16 year;
	kal_uint16 month;
	kal_uint16 day;
	kal_uint16 rg_gain;
	kal_uint16 bg_gain;
	kal_uint16 wb_flag;
	kal_uint16 golden_flag;
	kal_uint16 golden_rg;
	kal_uint16 golden_bg;
	kal_uint16 lsc_flag;// 0:Empty 1:Success 2:Invalid
#endif
	kal_uint16 reg_page[5];
	kal_uint16 reg_addr[5];
	kal_uint16 reg_value[5];
	kal_uint16 reg_flag;
	kal_uint16 reg_num;

}gc8034_otp;

static gc8034_otp gc8034_otp_info;

typedef enum{
	otp_page0=0,
	otp_page1,
	otp_page2,
	otp_page3,
	otp_page4,
	otp_page5,
	otp_page6,
	otp_page7,
	otp_page8,
	otp_page9,
}otp_page;

typedef enum{
	otp_close=0,
	otp_open,
}otp_state;

static kal_uint8 gc8034_read_otp(kal_uint8 addr)
{
	kal_uint8 value;
	kal_uint8 regd4;
	kal_uint16 realaddr = addr * 8;

    LOG_INF("E!\n");

	regd4 = read_cmos_sensor(0xd4);  //d4OTP enable[7]  OTP page select[2],[1:0]OTP address select high bit [9:8]

	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xd4,(regd4&0xfc)+((realaddr>>8)&0x03));
	write_cmos_sensor(0xd5,realaddr&0xff);  //OTP address select low bit [7:0]
	mdelay(1);
	write_cmos_sensor(0xf3,0x20);
	value = read_cmos_sensor(0xd7);

	return value;
}

static void gc8034_read_otp_group(kal_uint8 addr,kal_uint8* buff,int size)
{
	kal_uint8 i;
	kal_uint8 regd4,regf4,page;
	kal_uint16 realaddr = addr * 8;
	regd4 = read_cmos_sensor(0xd4);
	regf4 = read_cmos_sensor(0xf4);
	page = regd4&0x3c;
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xd4,(regd4&0xfc)+((realaddr>>8)&0x03));
	write_cmos_sensor(0xd5,realaddr&0xff);
	mdelay(1);
	write_cmos_sensor(0xf3,0x20);
	write_cmos_sensor(0xf4,regf4|0x02);
	write_cmos_sensor(0xf3,0x80);

	for(i=0;i<size;i++)
	{
		buff[i] = read_cmos_sensor(0xd7);
    }
	write_cmos_sensor(0xf3,0x00);
	write_cmos_sensor(0xf4,regf4&0xfd);
}


static void gc8034_select_page_otp(otp_page otp_select_page)
{
	kal_uint8 page;

	write_cmos_sensor(0xfe,0x00);
	page = read_cmos_sensor(0xd4);
	page = (page & 0xc3) | (((kal_uint8)otp_select_page<<2)&0x3c);

	/*
	switch(otp_select_page)
	{
	case otp_page0:
		page = page & 0xc3;
		break;
	case otp_page1:
		page = page & 0xc3 | 0x04;
		break;
	case otp_page2:
		page = page & 0xc3 | 0x08;
		break;
	case otp_page3:
		page = page & 0xc3 | 0x0c;
		break;
	case otp_page4:
		page = page & 0xc3 | 0x10;
		break;
	case otp_page5:
		page = page & 0xc3 | 0x14;
		break;
	case otp_page6:
		page = page & 0xc3 | 0x18;
		break;
	case otp_page7:
		page = page & 0xc3 | 0x1c;
		break;
	case otp_page8:
		page = page & 0xc3 | 0x20;
		break;
	case otp_page9:
		page = page & 0xc3 | 0x24;
		break;
	default:
		break;
	}
	*/
	mdelay(5);
	write_cmos_sensor(0xd4,page);

}

static void gc8034_gcore_read_otp_info(void)
{
	kal_uint8 flagdd;//,flag_Package;
	kal_uint16 i;
	kal_uint8 total_number=0;
	kal_uint8 temp;//,ddchecksum;
#ifdef GC8034OTP_FOR_CUSTOMER
	kal_uint8 flag_wb,index,flag_Module,flag_lsc;//,flag_golden;
	kal_uint16 j,idx;
	kal_uint8 ddtempbuff[80*4];
	kal_uint16 check;
	kal_uint8 info[8];
	kal_uint8 wb[4];
	kal_uint8 lsc[99*4];
	kal_uint16 lscr[99];
	kal_uint16 lscg[99];
	kal_uint16 lscb[99];
	kal_uint8 lscaddr;
	kal_uint8 golden[4];
#endif
	kal_uint8 regd4 = 0,regf4 = 0,page = 0;
	kal_uint16 realaddr = 0;


	memset(&gc8034_otp_info,0,sizeof(gc8034_otp));
	/*TODO*/
	gc8034_select_page_otp(otp_page0);
	flagdd = gc8034_read_otp(0x0b);
	LOG_INF("GC8034_OTP_DD : flag_dd = 0x%x\n",flagdd);
	//flag_Package= gc8034_read_otp(0x00);

	//DD
	switch(flagdd&0x03)
	{
	case 0x00:
		LOG_INF("GC8034_OTP_DD is Empty !!\n");
		gc8034_otp_info.dd_flag = 0x00;
		break;
	case 0x01:
		LOG_INF("GC8034_OTP_DD is Valid!!\n");
		total_number = gc8034_read_otp(0x0c) + gc8034_read_otp(0x0d);
		LOG_INF("GC8034_OTP : total_number = %d\n",total_number);
#ifdef GC8034OTP_DEBUG
		//gc8034_read_otp_group(0x0e, ddtempbuff, total_number * 4);
		realaddr = 0x0e * 8;
		regd4 = read_cmos_sensor(0xd4);
		regf4 = read_cmos_sensor(0xf4);
		page = regd4&0x3c;
		write_cmos_sensor(0xfe,0x00);
		write_cmos_sensor(0xd4,(regd4&0xfc)+((realaddr>>8)&0x03));
		write_cmos_sensor(0xd5,realaddr&0xff);
		mdelay(1);
		write_cmos_sensor(0xf3,0x20);
		write_cmos_sensor(0xf4,regf4|0x02);
		write_cmos_sensor(0xf3,0x80);

		for(i=0;i<total_number*4;i++)
		{
			if((i + 14)%0x80 == 0)
			{
				page+=4;
				write_cmos_sensor(0xf3,0x00);
				write_cmos_sensor(0xf4,regf4&0xfd);
				write_cmos_sensor(0xd4,(regd4&0xc0)|page);
				write_cmos_sensor(0xd5,0x00);
				mdelay(1);
				write_cmos_sensor(0xf3,0x20);
				write_cmos_sensor(0xf4,regf4|0x02);
				write_cmos_sensor(0xf3,0x80);
			}
			ddtempbuff[i] = read_cmos_sensor(0xd7);
		}
		write_cmos_sensor(0xf3,0x00);
		write_cmos_sensor(0xf4,regf4&0xfd);
		/*DD position*/
		for(i=0; i<total_number; i++)
		{
			LOG_INF("GC8034_OTP_DD:index = %d, x = %x, y = %x, checksum = %x, type = %x \n ",\
				i, (((kal_uint16)ddtempbuff[4 * i + 1] & 0x0f) << 8) + ddtempbuff[4 * i],\
				((kal_uint16)ddtempbuff[4 * i + 2] << 4) + ((ddtempbuff[4 * i + 1] & 0xf0) >> 4),\
				ddtempbuff[4 * i + 3]&0x80,ddtempbuff[4 * i + 3]&0x03);
			mdelay(2);
		}
#endif
		gc8034_otp_info.dd_cnt = total_number;
		gc8034_otp_info.dd_flag = 0x01;
		break;
	case 0x02:
	case 0x03:
		LOG_INF("GC8034_OTP_DD is Invalid !!\n");
		gc8034_otp_info.dd_flag = 0x02;
		break;
	default :
		break;
	}


#ifdef GC8034OTP_FOR_CUSTOMER
    gc8034_select_page_otp(otp_page9);
	flag_Module=gc8034_read_otp(0x6f);
	flag_wb = gc8034_read_otp(0x5e);
	//flag_golden = gc8034_read_otp(0x5e) & 0xf0;
	LOG_INF("GC8034_OTP : flag_Module = 0x%x , flag_wb = 0x%x\n",flag_Module,flag_wb);

//INFO&WB
	for(index=0;index<2;index++)
	{
		switch((flag_Module<<(2 * index))&0x0c)
		{
		case 0x00:
			LOG_INF("GC8034_OTP_INFO group %d is Empty !!\n", index + 1);
			break;
		case 0x04:
			LOG_INF("GC8034_OTP_INFO group %d is Valid !!\n", index + 1);
			check = 0;
			gc8034_read_otp_group(INFO_ROM_START + index * INFO_WIDTH, &info[0], INFO_WIDTH);
			for (i = 0; i < INFO_WIDTH - 1; i++)
			{
				check += info[i];
			}
			if ((check % 255 + 1) == info[INFO_WIDTH-1])
			{
				gc8034_otp_info.module_id = info[0];
				gc8034_otp_info.lens_id = info[1];
				gc8034_otp_info.vcm_driver_id = info[2];
				gc8034_otp_info.vcm_id = info[3];
				gc8034_otp_info.year = info[4];
				gc8034_otp_info.month = info[5];
				gc8034_otp_info.day = info[6];
			}
			else
			{
				LOG_INF("GC8034_OTP_INFO Check sum %d Error !!\n", index + 1);
			}
			break;
		case 0x08:
		case 0x0c:
			LOG_INF("GC8034_OTP_INFO group %d is Invalid !!\n", index + 1);
			break;
		default :
			break;
		}

		switch((flag_wb<<(2 * index))&0x0c)
		{
		case 0x00:
			LOG_INF("GC8034_OTP_WB group %d is Empty !!\n", index + 1);
			gc8034_otp_info.wb_flag = gc8034_otp_info.wb_flag|0x00;
			break;
		case 0x04:
			LOG_INF("GC8034_OTP_WB group %d is Valid !!\n", index + 1);
			check = 0;
			gc8034_read_otp_group(WB_ROM_START + index * WB_WIDTH, &wb[0], WB_WIDTH);
			for (i = 0; i < WB_WIDTH - 1; i++)
			{
				check += wb[i];
			}
			if ((check % 255 + 1) == wb[WB_WIDTH - 1])
			{
				gc8034_otp_info.rg_gain = (wb[0]|((wb[1]&0xf0)<<4)) > 0 ? (wb[0]|((wb[1]&0xf0)<<4)) : 0x400;
				gc8034_otp_info.bg_gain = (((wb[1]&0x0f)<<8)|wb[2]) > 0 ? (((wb[1]&0x0f)<<8)|wb[2]) : 0x400;
				gc8034_otp_info.wb_flag = gc8034_otp_info.wb_flag|0x01;
			}
			else
			{
				LOG_INF("GC8034_OTP_WB Check sum %d Error !!\n", index + 1);
			}
			break;
		case 0x08:
		case 0x0c:
			LOG_INF("GC8034_OTP_WB group %d is Invalid !!\n", index + 1);
			gc8034_otp_info.wb_flag = gc8034_otp_info.wb_flag|0x02;
			break;
		default :
			break;
		}

		switch((flag_wb<<(2 * index))&0xc0)
		{
		case 0x00:
			LOG_INF("GC8034_OTP_GOLDEN group %d is Empty !!\n", index + 1);
			gc8034_otp_info.golden_flag = gc8034_otp_info.golden_flag|0x00;
			break;
		case 0x40:
			LOG_INF("GC8034_OTP_GOLDEN group %d is Valid !!\n", index + 1);
			check = 0;
			gc8034_read_otp_group(GOLDEN_ROM_START + index * GOLDEN_WIDTH, &golden[0], GOLDEN_WIDTH);
			for (i = 0; i < GOLDEN_WIDTH - 1; i++)
			{
				check += golden[i];
			}
			if ((check % 255 + 1) == golden[GOLDEN_WIDTH - 1])
			{
				gc8034_otp_info.golden_rg = (golden[0]|((golden[1]&0xf0)<<4)) > 0 ? (golden[0]|((golden[1]&0xf0)<<4)) : RG_TYPICAL;
				gc8034_otp_info.golden_bg = (((golden[1]&0x0f)<<8)|golden[2]) > 0 ? (((golden[1]&0x0f)<<8)|golden[2]) : BG_TYPICAL;
				gc8034_otp_info.golden_flag = gc8034_otp_info.golden_flag|0x01;
			}
			else
			{
				LOG_INF("GC8034_OTP_GOLDEN Check sum %d Error !!\n", index + 1);
			}
			break;
		case 0x80:
		case 0xc0:
			LOG_INF("GC8034_OTP_GOLDEN group %d is Invalid !!\n", index + 1);
			gc8034_otp_info.golden_flag = gc8034_otp_info.golden_flag|0x02;
			break;
		default :
			break;
		}
	}

#ifdef GC8034OTP_DEBUG	/*lsc*/
	gc8034_select_page_otp(otp_page3);
	flag_lsc = gc8034_read_otp(0x43);
	for(index=0;index<2;index++)
	{

      switch((flag_lsc<<(2 * index))&0x0c)
		{
		case 0x00:
			 LOG_INF("GC8034_OTP_LSC group %d is Empty !\n",index + 1);
			 break;
		case 0x04:
             LOG_INF("GC8034_OTP_LSC group %d is Valid !\n",index + 1);
			 if(index==0)
		 	 {
		 		gc8034_select_page_otp(otp_page3);
				lscaddr = 0x44;
		 	 }
			 else
			 {
			 	gc8034_select_page_otp(otp_page6);
				lscaddr = 0x51;
			 }
				//gc8034_read_otp_group(lscaddr, &lsc[0], 99*4);
				realaddr = lscaddr * 8;
				regd4 = read_cmos_sensor(0xd4);
				regf4 = read_cmos_sensor(0xf4);
				page = regd4&0x3c;
				write_cmos_sensor(0xfe,0x00);
				write_cmos_sensor(0xd4,(regd4&0xfc)+((realaddr>>8)&0x03));
				write_cmos_sensor(0xd5,realaddr&0xff);
				mdelay(1);
				write_cmos_sensor(0xf3,0x20);
				write_cmos_sensor(0xf4,regf4|0x02);
				write_cmos_sensor(0xf3,0x80);

				for(i=0;i<99*4;i++)
				{
					if((i + lscaddr )%0x80 == 0)
					{
						page+=4;
						write_cmos_sensor(0xf3,0x00);
						write_cmos_sensor(0xf4,regf4&0xfd);
						write_cmos_sensor(0xd4,(regd4&0xc0)|page);
						write_cmos_sensor(0xd5,0x00);
						mdelay(1);
						write_cmos_sensor(0xf3,0x20);
						write_cmos_sensor(0xf4,regf4|0x02);
						write_cmos_sensor(0xf3,0x80);
					}
					lsc[i] = read_cmos_sensor(0xd7);
				}
				write_cmos_sensor(0xf3,0x00);
				write_cmos_sensor(0xf4,regf4&0xfd);

			for(i=0;i<9;i++)
			{
				for(j=0;j<11;j++)
				{
					idx = i*11 + j;
					lscr[idx] = ((lsc[4*idx+3]&0x3f)<<4)+((lsc[4*idx+2]&0xf0)>>4);
					lscg[idx] = ((lsc[4*idx+2]&0x0f)<<6)+((lsc[4*idx+1]&0xfc)>>2);
					lscb[idx] = ((lsc[4*idx+1]&0x03)<<8)+lsc[4*idx];
				}
			}
			LOG_INF("GC8034_OTP_LSC data R channel\n");
			for(i=0;i<9;i++)
			{
				LOG_INF("%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",lscr[i*11],lscr[i*11+1],lscr[i*11+2],\
					lscr[i*11+3],lscr[i*11+4],lscr[i*11+5],lscr[i*11+6],lscr[i*11+7],lscr[i*11+8],lscr[i*11+9],lscr[i*11+10]);
			}
			LOG_INF("GC8034_OTP_LSC data G channel\n");
			for(i=0;i<9;i++)
			{
				LOG_INF("%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",lscg[i*11],lscg[i*11+1],lscg[i*11+2],\
					lscg[i*11+3],lscg[i*11+4],lscg[i*11+5],lscg[i*11+6],lscg[i*11+7],lscg[i*11+8],lscg[i*11+9],lscg[i*11+10]);
			}
			LOG_INF("GC8034_OTP_LSC data B channel\n");
			for(i=0;i<9;i++)
			{
				LOG_INF("%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",lscb[i*11],lscb[i*11+1],lscb[i*11+2],\
					lscb[i*11+3],lscb[i*11+4],lscb[i*11+5],lscb[i*11+6],lscb[i*11+7],lscb[i*11+8],lscb[i*11+9],lscb[i*11+10]);
			}
			 break;
		case 0x08:
		case 0x0c:
			LOG_INF("GC8034_OTP_LSC group %d is Invalid !!\n", index + 1);
			break;
		default :
			break;
      	}
	}

#endif
	/*print otp information*/
	LOG_INF("GC8034_OTP_INFO:module_id=0x%x\n",gc8034_otp_info.module_id);
	LOG_INF("GC8034_OTP_INFO:lens_id=0x%x\n",gc8034_otp_info.lens_id);
	LOG_INF("GC8034_OTP_INFO:vcm_id=0x%x\n",gc8034_otp_info.vcm_id);
	LOG_INF("GC8034_OTP_INFO:vcm_driver_id=0x%x\n",gc8034_otp_info.vcm_driver_id);
	LOG_INF("GC8034_OTP_INFO:data=%d-%d-%d\n",gc8034_otp_info.year,gc8034_otp_info.month,gc8034_otp_info.day);
	LOG_INF("GC8034_OTP_WB:r/g=0x%x\n",gc8034_otp_info.rg_gain);
	LOG_INF("GC8034_OTP_WB:b/g=0x%x\n",gc8034_otp_info.bg_gain);
	LOG_INF("GC8034_OTP_GOLDEN:golden_rg=0x%x\n",gc8034_otp_info.golden_rg);
	LOG_INF("GC8034_OTP_GOLDEN:golden_bg=0x%x\n",gc8034_otp_info.golden_bg);
#endif

	/*chip regs*/
	gc8034_select_page_otp(otp_page2);
	gc8034_otp_info.reg_flag = gc8034_read_otp(0x4e);

	if(gc8034_otp_info.reg_flag==1)
	{
		for(i=0;i<5;i++)
		{
		temp = gc8034_read_otp(0x4f+5*i);
		for(j=0;j<2;j++)
			{
			if(((temp>>(4*j+3))&0x01)==0x01)
				{
				gc8034_otp_info.reg_page[gc8034_otp_info.reg_num] = (temp>>(4*j))&0x03;
				gc8034_otp_info.reg_addr[gc8034_otp_info.reg_num] = gc8034_read_otp(0x50+5*i+2*j);
				gc8034_otp_info.reg_value[gc8034_otp_info.reg_num] = gc8034_read_otp(0x50+5*i+2*j+1);
				gc8034_otp_info.reg_num++;
				}
			}
		}
	}

}


static void gc8034_gcore_update_dd(void)
{
	kal_uint8 state;//dd_num,
	//dd_num=gc8034_otp_info.dd_cnt;

	/*TODO*/
	if(0x01 ==gc8034_otp_info.dd_flag)
	{
	   LOG_INF("GC8034_OTP_AUTO_DD start !\n");
	         write_cmos_sensor(0xf2,0x01);
			 write_cmos_sensor(0xf4,0x88);
			 write_cmos_sensor(0xfe,0x00);
	         write_cmos_sensor(0x79,0x2e);//enable
	         write_cmos_sensor(0x7b,gc8034_otp_info.dd_cnt);
	  		 write_cmos_sensor(0x7e,0x00);
	  		 write_cmos_sensor(0x7f,0x70);//dd_base_addr
	   		 write_cmos_sensor(0x6e,0x01); //[0]auto_check_mode
	   		 write_cmos_sensor(0xbd,0xd4);
			 write_cmos_sensor(0xbe,0x9c);
			 write_cmos_sensor(0xbf,0xa0);  //otp_win_heigth&width 3284*2464
			 write_cmos_sensor(0xfe,0x01);
			 write_cmos_sensor(0xbe,0x00); //allow dd write to sram
			 write_cmos_sensor(0xa9,0x01); //clear ram
			 write_cmos_sensor(0xfe,0x00);
	  		 write_cmos_sensor(0xf2,0x41);  //start auto load

			 while((state = read_cmos_sensor(0x6e)))
	  		 {
	  		 	if((state|0xef)!=0xff)
					break;
				else
					mdelay(10);
	  		 }

	  		 write_cmos_sensor(0xfe,0x01);
	  		 write_cmos_sensor(0xbe,0x01);
	  		 write_cmos_sensor(0xfe,0x00);
			 write_cmos_sensor(0x79,0x00);

	}

}
#endif
#ifdef GC8034OTP_FOR_CUSTOMER
static void gc8034_gcore_update_wb(void)
{
	kal_uint16 r_gain_current = 0 , g_gain_current = 0 , b_gain_current = 0 , base_gain = 0;
	kal_uint16 r_gain = 1024 , g_gain = 1024 , b_gain = 1024 ;
	kal_uint16 rg_typical,bg_typical;

	if(0x02==gc8034_otp_info.golden_flag)
	{
		return;
	}
	if(0x00==(gc8034_otp_info.golden_flag&0x01))
	{
		rg_typical=RG_TYPICAL;
		bg_typical=BG_TYPICAL;
	}
	if(0x01==(gc8034_otp_info.golden_flag&0x01))
	{
		rg_typical=gc8034_otp_info.golden_rg;
		bg_typical=gc8034_otp_info.golden_bg;
		LOG_INF("GC8034_OTP_UPDATE_AWB:rg_typical = 0x%x , bg_typical = 0x%x\n",rg_typical,bg_typical);
	}

	if(0x01==(gc8034_otp_info.wb_flag&0x01))
	{
		r_gain_current = 2048 * rg_typical/gc8034_otp_info.rg_gain;
		b_gain_current = 2048 * bg_typical/gc8034_otp_info.bg_gain;
		g_gain_current = 2048;

		base_gain = (r_gain_current<b_gain_current) ? r_gain_current : b_gain_current;
		base_gain = (base_gain<g_gain_current) ? base_gain : g_gain_current;
		LOG_INF("GC8034_OTP_UPDATE_AWB:r_gain_current = 0x%x , b_gain_current = 0x%x , base_gain = 0x%x \n",r_gain_current,b_gain_current,base_gain);

		r_gain = 0x400 * r_gain_current / base_gain;
		g_gain = 0x400 * g_gain_current / base_gain;
		b_gain = 0x400 * b_gain_current / base_gain;
		LOG_INF("GC8034_OTP_UPDATE_AWB:r_gain = 0x%x , g_gain = 0x%x , b_gain = 0x%x \n",r_gain,g_gain,b_gain);

		/*TODO*/
		write_cmos_sensor(0xfe,0x01);
		write_cmos_sensor(0x84,g_gain>>3);
		write_cmos_sensor(0x85,r_gain>>3);
		write_cmos_sensor(0x86,b_gain>>3);
		write_cmos_sensor(0x87,g_gain>>3);
		write_cmos_sensor(0x88,((g_gain&0x07) << 4) + (r_gain&0x07));
		write_cmos_sensor(0x89,((b_gain&0x07) << 4) + (g_gain&0x07));
		write_cmos_sensor(0xfe,0x00);

	}

}

static void gc8034_gcore_update_lsc(void)
{
    kal_uint8 flag_lsc,index,state;

	gc8034_select_page_otp(otp_page3);
	flag_lsc = gc8034_read_otp(0x43);
	LOG_INF("GC8034_OTP_LSC : flag_lsc = 0x%x\n",flag_lsc);

	for(index=0;index<2;index++)
	{

      switch((flag_lsc<<(2 * index))&0x0c)
		{
		case 0x00:
			 LOG_INF("GC8034_OTP_LSC group %d is Empty !\n",index + 1);
			 gc8034_otp_info.lsc_flag = gc8034_otp_info.lsc_flag|0x00;
			 break;
		case 0x04:
             LOG_INF("GC8034_OTP_LSC group %d is Valid !\n",index + 1);
			 gc8034_otp_info.lsc_flag = gc8034_otp_info.lsc_flag|0x01;
			 write_cmos_sensor(0xf2,0x01);
			 write_cmos_sensor(0xf4,0x88);
			 write_cmos_sensor(0xfe,0x00);
	         write_cmos_sensor(0x78,0x9b);//8a//row and col
	         write_cmos_sensor(0x79,0x0d);//enable
	  		 write_cmos_sensor(0x7a,LSC_NUM);//50//lsc_num
	  		 write_cmos_sensor(0x7c,LSC_ADDR[index*2]);//0c//otp_lsc_page[4:2]
	   		 write_cmos_sensor(0x7d,LSC_ADDR[index*2+1]);//00//otp_lsc_base_addr[7:0]
	   		 write_cmos_sensor(0x6e,0x01);//otp_auto_check_mode
	  		 write_cmos_sensor(0xfe,0x01);
	  		 write_cmos_sensor(0xcf,0x00);
			 write_cmos_sensor(0xc9,0x01); //clear ram
	  	     write_cmos_sensor(0xf2,0x41);
	  		 write_cmos_sensor(0xfe,0x00);

	  		 while((state = read_cmos_sensor(0x6e)))
	  		 {
	  		 	if((state|0xdf)!=0xff)
					break;
				else
					mdelay(10);
	  		 }

	  		 write_cmos_sensor(0xfe,0x01);
	  		 write_cmos_sensor(0xcf,0x01);
	  		 write_cmos_sensor(0xfe,0x00);
			 write_cmos_sensor(0x79,0x00);
			 // mdelay(5);
			 break;
		case 0x08:
		case 0x0c:
			LOG_INF("GC8034_OTP_LSC group %d is Invalid !!\n", index + 1);
			gc8034_otp_info.lsc_flag = gc8034_otp_info.lsc_flag|0x02;
			break;
		default :
			break;
		}


    }


}

#endif
#ifdef GC8034OTP_ENABLE
static void gc8034_gcore_update_chipversion(void)
{
	kal_uint8 i;

	LOG_INF("GC8034_OTP_UPDATE_CHIPVERSION:reg_num = %d\n",gc8034_otp_info.reg_num);

	write_cmos_sensor(0xfe,0x00);
	if(gc8034_otp_info.reg_flag)
	{
		for(i=0;i<gc8034_otp_info.reg_num;i++)
		{
			write_cmos_sensor(0xfe,gc8034_otp_info.reg_page[i]);
			write_cmos_sensor(gc8034_otp_info.reg_addr[i] ,gc8034_otp_info.reg_value[i]);
			LOG_INF("GC8034_OTP_UPDATE_CHIP_VERSION:{0x%x,0x%x}!!\n",gc8034_otp_info.reg_addr[i],gc8034_otp_info.reg_value[i]);
		}
	}
}

static void gc8034_gcore_update_otp(void)
{
	gc8034_gcore_update_dd();
#ifdef GC8034OTP_FOR_CUSTOMER
	gc8034_gcore_update_wb();
    gc8034_gcore_update_lsc();

#endif
	gc8034_gcore_update_chipversion();

}

static void gc8034_gcore_enable_otp(otp_state state)
{
	kal_uint8 otp_clk,otp_en;
	otp_clk = read_cmos_sensor(0xf2);
	otp_en= read_cmos_sensor(0xf4);
	if(state)
	{
		otp_clk = otp_clk | 0x01;
		otp_en = otp_en | 0x88;
		mdelay(5);
		write_cmos_sensor(0xf2,otp_clk);	// 0xf2[0]:OTP_CLK_en
		write_cmos_sensor(0xf4,otp_en);	// 0xf4[3]:OTP_en

		LOG_INF("GC8034_OTP: Enable OTP!\n");
	}
	else
	{
		otp_en = otp_en & 0xf7;
		otp_clk = otp_clk & 0xf7;
		mdelay(5);
		write_cmos_sensor(0xf4,otp_en);
		write_cmos_sensor(0xf2,otp_clk);

		LOG_INF("GC8034_OTP: Disable OTP!\n");
	}

}


static void gc8034_gcore_identify_otp(void)
{
/*
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xf2,0x00);
	write_cmos_sensor(0xf4,0x00);
	write_cmos_sensor(0xf5,0x19);
	write_cmos_sensor(0xf6,0x44);
	write_cmos_sensor(0xf7,0x95);
	write_cmos_sensor(0xf8,0x63);
	write_cmos_sensor(0xf9,0x00);
	write_cmos_sensor(0xfa,0x45);
	write_cmos_sensor(0xfc,0xee);
*/

	gc8034_gcore_enable_otp(otp_open);
	gc8034_gcore_read_otp_info();
	gc8034_gcore_update_otp();
	gc8034_gcore_enable_otp(otp_close);
}
#endif
static void set_dummy(void)
{

 	kal_uint32 hb = 0;
	kal_uint32 vb = 0;
	LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line, imgsensor.dummy_pixel);

	hb = imgsensor.dummy_pixel + GC8034_DEFAULT_DUMMY_PIXEL_NUMS;
	vb = imgsensor.dummy_line + GC8034_DEFAULT_DUMMY_LINE_NUMS;	//dummy line and shutter need 4X-align


	//Set HB, don't modify
	//write_cmos_sensor(0x05, (hb >> 8)& 0xFF);
	//write_cmos_sensor(0x06, hb & 0xFF);

	//Set VB
	//write_cmos_sensor(0x07, (vb >> 8) & 0xFF);
	//write_cmos_sensor(0x08, vb & 0xFF);

//  end
}    /* set_dummy  */

static kal_uint32 return_sensor_id(void)
{
    return ((read_cmos_sensor(0xf0) << 8) | read_cmos_sensor(0xf1));
}

static void set_max_framerate(UINT16 framerate,kal_bool min_framelength_en)
{
    //kal_int16 dummy_line;
    kal_uint32 frame_length = imgsensor.frame_length;
    //unsigned long flags;

    frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
    spin_lock(&imgsensor_drv_lock);
    imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length;
    imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;

    if (imgsensor.frame_length > imgsensor_info.max_frame_length)
    {
        imgsensor.frame_length = imgsensor_info.max_frame_length;
        imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
    }
    if (min_framelength_en)
        imgsensor.min_frame_length = imgsensor.frame_length;
    spin_unlock(&imgsensor_drv_lock);
    set_dummy();
}    /*    set_max_framerate  */



/*************************************************************************
* FUNCTION
*    set_shutter
*
* DESCRIPTION
*    This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*    iShutter : exposured lines
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
    unsigned long flags;
	kal_uint16 shutter_temp;
    //kal_uint16 realtime_fps;
    //kal_uint32 frame_length;
    LOG_INF("Enter set_shutter!\n");
    spin_lock_irqsave(&imgsensor_drv_lock, flags);
    imgsensor.shutter = shutter;
    spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

    // if shutter bigger than frame_length, should extend frame length first
    spin_lock(&imgsensor_drv_lock);
    if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
        imgsensor.frame_length = shutter + imgsensor_info.margin;
    else
        imgsensor.frame_length = imgsensor.min_frame_length;
    if (imgsensor.frame_length > imgsensor_info.max_frame_length)
        imgsensor.frame_length = imgsensor_info.max_frame_length;
    spin_unlock(&imgsensor_drv_lock);
    shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
    shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	/*if (imgsensor.autoflicker_en)
		{     realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
              if(realtime_fps >= 297 && realtime_fps <= 305)
                    set_max_framerate(296,0);
              else if(realtime_fps >= 147 && realtime_fps <= 150)
                    set_max_framerate(146,0);
              else
                   set_max_framerate(realtime_fps,0);

         }*/


	// Update Shutter
	if(shutter > 16383) shutter = 16383;
	if(shutter < 1) shutter = 1;
	//* 2
	shutter_temp=shutter*2;

	/*if(shutter_temp <= 0x14)
  	{
		write_cmos_sensor(0xfe, 0x00);
		write_cmos_sensor(0xd1, 0xad);
  	}
  	else
	{
		write_cmos_sensor(0xfe, 0x00);
		write_cmos_sensor(0xd1, 0xaa);
  	}*/

	//Update Shutter
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x03, (shutter_temp>>8) & 0x7F);
	write_cmos_sensor(0x04, shutter_temp & 0xFF);

    LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);

}    /*    set_shutter */


#if 0
static kal_uint16 gain2reg(const kal_uint16 gain)
{
   /* kal_uint16 reg_gain = 0x0000;
	kal_uint16 GC8034_GAIN_BASE = 256;

   // reg_gain = ((gain / BASEGAIN) << 4) + ((gain % BASEGAIN) * 16 / BASEGAIN);
    reg_gain = gain*GC8034_GAIN_BASE/0x40;
    return (kal_uint16)reg_gain;*/
    return 0;
}
#endif
/*************************************************************************
* FUNCTION
*    set_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    iGain : sensor global gain(base: 0x40)
*
* RETURNS
*    the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 iReg,temp;
	LOG_INF("Enter set_gain!\n");
	iReg = gain;

	if(iReg < 0x40)
		iReg = 0x40;

	if((ANALOG_GAIN_1<= iReg)&&(iReg < ANALOG_GAIN_2))
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x00);

	temp = iReg*256/64;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 1x, GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 0;
	}
	else if((ANALOG_GAIN_2<= iReg)&&(iReg < ANALOG_GAIN_3))
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x01);

	temp = 256*iReg/ANALOG_GAIN_2;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 1.375x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 1;
	}
	else if((ANALOG_GAIN_3<= iReg)&&(iReg < ANALOG_GAIN_4))
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x02);

	temp = 256*iReg/ANALOG_GAIN_3;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 1.891x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 2;
	}
	else if((ANALOG_GAIN_4<= iReg)&&(iReg < ANALOG_GAIN_5))
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x03);

	temp = 256*iReg/ANALOG_GAIN_4;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 2.625x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 3;
	}
	else if((ANALOG_GAIN_5<= iReg)&&(iReg < ANALOG_GAIN_6))
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x04);

	temp = 256*iReg/ANALOG_GAIN_5;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 3.734x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 4;
	}
	//else
	else if((ANALOG_GAIN_6<= iReg)&&(iReg < ANALOG_GAIN_7))
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x05);

	temp = 256*iReg/ANALOG_GAIN_6;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 5;
	}
	else if((ANALOG_GAIN_7<= iReg)&&(iReg < ANALOG_GAIN_8))
	{
   	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x06);

	temp = 256*iReg/ANALOG_GAIN_7;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 6;
	}

	else if((ANALOG_GAIN_8<= iReg)&&(iReg < ANALOG_GAIN_9))
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x07);

	temp = 256*iReg/ANALOG_GAIN_8;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 7;
	}
	else
	{
	//analog gain
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0xb6,0x08);

	temp = 256*iReg/ANALOG_GAIN_9;
	write_cmos_sensor(0xb1,temp>>8);
	write_cmos_sensor(0xb2,(temp&0xff));
	LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp);
	gainlevel = 8;
	}
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x09,Val09[gainlevel]);
	write_cmos_sensor(0x20,Val20[gainlevel]);
	write_cmos_sensor(0x33,Val33[gainlevel]);
	write_cmos_sensor(0xfe,0x01);
	write_cmos_sensor(0xdf,Valdf[gainlevel]);
	write_cmos_sensor(0xe7,Vale7[gainlevel]);
	write_cmos_sensor(0xe8,Vale8[gainlevel]);
	write_cmos_sensor(0xe9,Vale9[gainlevel]);
	write_cmos_sensor(0xea,Valea[gainlevel]);
	write_cmos_sensor(0xeb,Valeb[gainlevel]);
	write_cmos_sensor(0xec,Valec[gainlevel]);
	write_cmos_sensor(0xed,Valed[gainlevel]);
	write_cmos_sensor(0xee,Valee[gainlevel]);
	write_cmos_sensor(0xfe,0x00);
    /*else if((ANALOG_GAIN_10<= iReg)&&(iReg < ANALOG_GAIN_11))
   {
		write_cmos_sensor(0xfe,0x00);
		//analog gain
		write_cmos_sensor(0xb6,0x09);
		//write_cmos_sensor(0x20,0x54);
		//write_cmos_sensor(0x33,0x84);
		//write_cmos_sensor(0x09,0xd0);
		temp = 64*iReg/ANALOG_GAIN_10;
		write_cmos_sensor(0xb1,temp>>6);
		write_cmos_sensor(0xb2,(temp<<2)&0xfc);
		LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp)

   }
	else if((ANALOG_GAIN_11<= iReg)&&(iReg < ANALOG_GAIN_12))
   {
		write_cmos_sensor(0xfe,0x00);
		//analog gain
		write_cmos_sensor(0xb6,0x0a);
		//write_cmos_sensor(0x20,0x54);
		//write_cmos_sensor(0x33,0x84);
		//write_cmos_sensor(0x09,0xd0);
		temp = 64*iReg/ANALOG_GAIN_11;
		write_cmos_sensor(0xb1,temp>>6);
		write_cmos_sensor(0xb2,(temp<<2)&0xfc);
		LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp)

   }
	else if((ANALOG_GAIN_12<= iReg)&&(iReg < ANALOG_GAIN_13))
   {
		write_cmos_sensor(0xfe,0x00);
		//analog gain
		write_cmos_sensor(0xb6,0x0b);
		//write_cmos_sensor(0x20,0x54);
		//write_cmos_sensor(0x33,0x84);
		//write_cmos_sensor(0x09,0xd0);
		temp = 64*iReg/ANALOG_GAIN_12;
		write_cmos_sensor(0xb1,temp>>6);
		write_cmos_sensor(0xb2,(temp<<2)&0xfc);
		LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp)

   }
	else
   {
	     write_cmos_sensor(0xfe,0x00);
		//analog gain
	    write_cmos_sensor(0xb6,0x0c);
	   //write_cmos_sensor(0x20,0x54);
		//write_cmos_sensor(0x33,0x84);
		//write_cmos_sensor(0x09,0xd0);
	    temp = 64*iReg/ANALOG_GAIN_13;
	    write_cmos_sensor(0xb1,temp>>6);
	    write_cmos_sensor(0xb2,(temp<<2)&0xfc);
	    LOG_INF("GC8034MIPI analogic gain 5.250x , GC8034MIPI add pregain = %d\n",temp);*/


	return gain;

}    /*    set_gain  */

static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
    LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n",le,se,gain);

}


#if 0
static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

switch (image_mirror)
	{
		case IMAGE_NORMAL://IMAGE_NORMAL:
			write_cmos_sensor(0x17,0x00);//bit[1][0]

			break;
		case IMAGE_H_MIRROR://IMAGE_H_MIRROR:
			write_cmos_sensor(0x17,0x01);
			break;
		case IMAGE_V_MIRROR://IMAGE_V_MIRROR:
			write_cmos_sensor(0x17,0x10);
			break;
		case IMAGE_HV_MIRROR://IMAGE_HV_MIRROR:
			write_cmos_sensor(0x17,0x11);
			break;
	}

}
#endif
/*************************************************************************
* FUNCTION
*    night_mode
*
* DESCRIPTION
*    This function night mode of sensor.
*
* PARAMETERS
*    bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
		LOG_INF("Enter nigh_mode!\n");
/*No Need to implement this function*/
}    /*    night_mode    */

static void sensor_init(void)
{
	printk("%s build is %s-%s and the location of this log is %s %d\n",__FILE__,__DATE__,__TIME__,__func__,__LINE__);
	LOG_INF("E");
	LOG_INF("Enter init_setting!\n");
	/*SYS*/
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x3f,0x00);
	write_cmos_sensor(0xf2,0x00);
	write_cmos_sensor(0xf4,0x90);//80 2017.5.18
	write_cmos_sensor(0xf5,0x3d);//19 2017.5.18
	write_cmos_sensor(0xf6,0x44);
	write_cmos_sensor(0xf7,0x95); //pll enable
	write_cmos_sensor(0xf8,0x63); //pll mode
	write_cmos_sensor(0xf9,0x00);
	write_cmos_sensor(0xfa,0x42);//45 2017.5.18
	write_cmos_sensor(0xfc,0xee);

	/*Cisctl&Analog*/

	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x03,0x08);
	write_cmos_sensor(0x04,0xc6);
	write_cmos_sensor(0x05,0x02);
	write_cmos_sensor(0x06,0x16);
	write_cmos_sensor(0x07,0x00);
	write_cmos_sensor(0x08,0x10);
	write_cmos_sensor(0x0a,0x3a); //row start
	write_cmos_sensor(0x0b,0x00);
	write_cmos_sensor(0x0c,0x04); //col start
	write_cmos_sensor(0x0d,0x09);
	write_cmos_sensor(0x0e,0xa0); //win_height 2464
	write_cmos_sensor(0x0f,0x0c);
	write_cmos_sensor(0x10,0xd4); //win_width 3284
	write_cmos_sensor(0x17,MIRROR);
	write_cmos_sensor(0x18,0x02);
	write_cmos_sensor(0x19,0x17);
	write_cmos_sensor(0x1a,0x09);
	write_cmos_sensor(0x1d,0x13);
	write_cmos_sensor(0x1e,0x70);
	write_cmos_sensor(0x1f,0xa0);//a0 2017.5.18
	write_cmos_sensor(0x21,0x24);
	write_cmos_sensor(0x25,0x00);
	write_cmos_sensor(0x28,0x56);
	write_cmos_sensor(0xca,0x06);
	write_cmos_sensor(0xcb,0x00);
	write_cmos_sensor(0xcc,0x3d);
	write_cmos_sensor(0xce,0x40);
	write_cmos_sensor(0xcf,0xb3);
	write_cmos_sensor(0xd0,0x19);
	write_cmos_sensor(0xd1,0xaa);
	write_cmos_sensor(0xd2,0xdb);
	write_cmos_sensor(0xd8,0x98); //dacin offset
	write_cmos_sensor(0xd9,0xff); //rampr width
	write_cmos_sensor(0xda,0x0f);
	write_cmos_sensor(0xdb,0x18); //ramps width
	write_cmos_sensor(0xdc,0x09);
	write_cmos_sensor(0xe4,0xa9);
	write_cmos_sensor(0xe5,0x12); //08 2017.5.17
	write_cmos_sensor(0xe6,0x10); //ramps offset
	write_cmos_sensor(0xfe,0x02);
	write_cmos_sensor(0x59,0x08);
	write_cmos_sensor(0x5a,0x0c); //08 2017.5.17
	write_cmos_sensor(0x5b,0x08);
	write_cmos_sensor(0x5c,0x20);

	/*Gamma*/
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x09,0x40);
	write_cmos_sensor(0x20,0x72);
	write_cmos_sensor(0x33,0xa5);
	write_cmos_sensor(0xfe,0x01);
	write_cmos_sensor(0xdf,0x0a);
	write_cmos_sensor(0xe7,0x1e);
	write_cmos_sensor(0xe8,0x1d);
	write_cmos_sensor(0xe9,0x0e);
	write_cmos_sensor(0xea,0x13);
	write_cmos_sensor(0xeb,0x4a);
	write_cmos_sensor(0xec,0x6a);
	write_cmos_sensor(0xed,0xa6);
	write_cmos_sensor(0xee,0xd4);

	/*ISP*/
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x80,0x13);//11 2017.5.18
	write_cmos_sensor(0x84,0x01);
	write_cmos_sensor(0x88,0x03);
	write_cmos_sensor(0x89,0x03);
	write_cmos_sensor(0x8d,0x03);
	write_cmos_sensor(0x8f,0x14);
	write_cmos_sensor(0xad,0x00);//	2017.5.18

	/*Crop window*/

	write_cmos_sensor(0x90,0x01);
	write_cmos_sensor(0x92,FullStartY); //crop y
	write_cmos_sensor(0x94,FullStartX); //crop x
	write_cmos_sensor(0x95,0x09);
	write_cmos_sensor(0x96,0x90);
	write_cmos_sensor(0x97,0x0c);
	write_cmos_sensor(0x98,0xc0);

	/*Gain*/
	write_cmos_sensor(0xb0,0x90);
	write_cmos_sensor(0xb1,0x01);
	write_cmos_sensor(0xb2,0x00);
	write_cmos_sensor(0xb6,0x00);

	/*BLK*/

	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x40,0x22);
	write_cmos_sensor(0x43,0x00); //add_offset
	write_cmos_sensor(0x4e,0x0f); //00 2017.5.18//row_bits[15:8]
	write_cmos_sensor(0x4f,0xf0); //3c 2017.5.18//row_bits[7:0]
	write_cmos_sensor(0x58,0x80); //dark current ratio
	write_cmos_sensor(0x59,0x80);
	write_cmos_sensor(0x5a,0x80);
	write_cmos_sensor(0x5b,0x80);
	write_cmos_sensor(0x5c,0x00);
	write_cmos_sensor(0x5d,0x00);
	write_cmos_sensor(0x5e,0x00);
	write_cmos_sensor(0x5f,0x00);

	/*WB offset*/

	write_cmos_sensor(0xfe,0x01);
	write_cmos_sensor(0xbf,0x40);

	/*Dark Sun*/

	write_cmos_sensor(0xfe,0x01);
	write_cmos_sensor(0x68,0x77);

	/*DPC*/

	write_cmos_sensor(0xfe,0x01);
	write_cmos_sensor(0x60,0x15);
	write_cmos_sensor(0x61,0x10);
	write_cmos_sensor(0x62,0x60);
	write_cmos_sensor(0x63,0x48);
	write_cmos_sensor(0x64,0x02);

	/*LSC*/

	write_cmos_sensor(0xfe,0x01);
	write_cmos_sensor(0xa0,0x10); //[6]segment_width[8],0x[5:4]segment_height[9:8]
	write_cmos_sensor(0xa8,0x60); //segment_height[7:0]
	write_cmos_sensor(0xa2,0xd1); //height_ratio[7:0]
	write_cmos_sensor(0xc8,0x5b); //[7:4]height_ratio[11:8]
	write_cmos_sensor(0xa1,0xb8); //segment_width[7:0]
	write_cmos_sensor(0xa3,0x91); //width_ratio[7:0]
	write_cmos_sensor(0xc0,0x50); //[7:4]width_ratio[11:8]
	write_cmos_sensor(0xd0,0x05); //segment_width_end[11:8]
	write_cmos_sensor(0xd1,0xb2); //segment_width_end[7:0]
	write_cmos_sensor(0xd2,0x1f); //col_segment
	write_cmos_sensor(0xd3,0x00); //row_num_start[7:0]
	write_cmos_sensor(0xd4,0x00); //[5:4]row_num_start[9:8] [3:0]col_seg_start
	write_cmos_sensor(0xd5,0x00); //[7:2]col_num_start[7:2]
	write_cmos_sensor(0xd6,0x00); //[2:0]col_num_start[10:8]
	write_cmos_sensor(0xd7,0x00); //row_seg_start
	write_cmos_sensor(0xd8,0x00); //col_cal_start[7:0]
	write_cmos_sensor(0xd9,0x00); //[2:0]col_cal_start[10:8]

	/*ABB*/
	write_cmos_sensor(0xfe,0x01);
	write_cmos_sensor(0x20,0x02);
	write_cmos_sensor(0x21,0x02);
	write_cmos_sensor(0x23,0x43);

	/*MIPI*/
	write_cmos_sensor(0xfe,0x03);
	write_cmos_sensor(0x01,0x07);
	write_cmos_sensor(0x02,0x07);//03 2017.5.18
	write_cmos_sensor(0x03,0x92);
	write_cmos_sensor(0x04,0x80);
	write_cmos_sensor(0x11,0x2b);
	write_cmos_sensor(0x12,0xf0); //lwc 1632*5/4
	write_cmos_sensor(0x13,0x0f);
	write_cmos_sensor(0x15,0x12);
	write_cmos_sensor(0x16,0x29);
	write_cmos_sensor(0x17,0xff);
	write_cmos_sensor(0x18,0x01);
	write_cmos_sensor(0x19,0xaa);
	write_cmos_sensor(0x1a,0x02);
	write_cmos_sensor(0x21,0x0c);//05 2017.5.17
	write_cmos_sensor(0x22,0x0c);//05 2017.5.17
	write_cmos_sensor(0x23,0x2c);//16 2017.5.17
	write_cmos_sensor(0x24,0x00);
	write_cmos_sensor(0x25,0x1c);//12 2071.5.17
	write_cmos_sensor(0x26,0x0b);//07 2071.5.17
	write_cmos_sensor(0x29,0x0e);//07 2071.5.17
	write_cmos_sensor(0x2a,0x0e);//08 2071.5.17
	write_cmos_sensor(0x2b,0x0b);//07 2071.5.17
	write_cmos_sensor(0xfe,0x00);
}    /*    sensor_init  */

static void preview_setting(void)
{
	LOG_INF("Enter preview_setting!\n");
	/*SYS*/
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x3f,0x91);
	write_cmos_sensor(0xfe,0x00);
}    /*    preview_setting  */

static void capture_setting(kal_uint16 currefps)
{
	LOG_INF("E! currefps:%d\n",currefps);
	LOG_INF("Enter capture_setting!\n");
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x3f,0x91);
	write_cmos_sensor(0xfe,0x00);
	LOG_INF("E! currefps:%d\n",currefps);
	LOG_INF("Enter capture_setting!\n");
}

static void normal_video_setting(kal_uint16 currefps)
{
	LOG_INF("E! currefps:%d\n",currefps);
	LOG_INF("Enter normal_video_setting!\n");
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x3f,0x91);
	write_cmos_sensor(0xfe,0x00);
}

static void hs_video_setting(void)
{
	LOG_INF("E\n");
	LOG_INF("Enter hs_video_setting!\n");
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x3f,0x91);
	write_cmos_sensor(0xfe,0x00);
}

static void slim_video_setting(void)
{
	LOG_INF("E\n");
	LOG_INF("Enter slim_video_setting!\n");
	write_cmos_sensor(0xfe,0x00);
	write_cmos_sensor(0x3f,0x91);
	write_cmos_sensor(0xfe,0x00);
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
    LOG_INF("enable: %d\n", enable);


  /*  if(enable)
    {
     	write_cmos_sensor(0xfe, 0x00);
       write_cmos_sensor(0x8c,0x01); //bit[0]: 1 enable test pattern, 0 disable test pattern
    }
	else
	{
	 	write_cmos_sensor(0xfe, 0x00);
	   write_cmos_sensor(0x8c,0x00);//bit[0]: 1 enable test pattern, 0 disable test pattern
	}*/
    spin_lock(&imgsensor_drv_lock);
    imgsensor.test_pattern = enable;
    spin_unlock(&imgsensor_drv_lock);
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*    get_imgsensor_id
*
* DESCRIPTION
*    This function get the sensor ID
*
* PARAMETERS
*    *sensorID : return the sensor ID
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
    kal_uint8 i = 0;
    kal_uint8 retry = 2;

    while (imgsensor_info.i2c_addr_table[i] != 0xff) {
        spin_lock(&imgsensor_drv_lock);
        imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
        spin_unlock(&imgsensor_drv_lock);
        do {
            *sensor_id = return_sensor_id();
            if (*sensor_id == imgsensor_info.sensor_id) {
                LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,*sensor_id);
                return ERROR_NONE;
            }
            LOG_INF("Read sensor id fail, write id: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id,*sensor_id);
            retry--;
        } while(retry > 0);
        i++;
        retry = 2;
    }
    if (*sensor_id != imgsensor_info.sensor_id) {
        // if Sensor ID is not correct, Must set *sensor_id to 0xFFFFFFFF
        *sensor_id = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*    open
*
* DESCRIPTION
*    This function initialize the registers of CMOS sensor
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
    kal_uint8 i = 0;
    kal_uint8 retry = 2;
    kal_uint32 sensor_id = 0;
    LOG_1;

    while (imgsensor_info.i2c_addr_table[i] != 0xff) {
        spin_lock(&imgsensor_drv_lock);
        imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
        spin_unlock(&imgsensor_drv_lock);
        do {
            sensor_id = return_sensor_id();
            if (sensor_id == imgsensor_info.sensor_id) {
                LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
                break;
            }
            LOG_INF("Read sensor id fail, write id: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
            retry--;
        } while(retry > 0);
        i++;
        if (sensor_id == imgsensor_info.sensor_id)
            break;
        retry = 2;
    }
    if (imgsensor_info.sensor_id != sensor_id)
        return ERROR_SENSOR_CONNECT_FAIL;

    /* initail sequence write in  */
    sensor_init();
#ifdef GC8034OTP_ENABLE
	gc8034_gcore_identify_otp();

#endif
    spin_lock(&imgsensor_drv_lock);

    imgsensor.autoflicker_en= KAL_FALSE;
    imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
    imgsensor.pclk = imgsensor_info.pre.pclk;
    imgsensor.frame_length = imgsensor_info.pre.framelength;
    imgsensor.line_length = imgsensor_info.pre.linelength;
    imgsensor.min_frame_length = imgsensor_info.pre.framelength;
    imgsensor.dummy_pixel = 0;
    imgsensor.dummy_line = 0;
    imgsensor.ihdr_en = 0;
    imgsensor.test_pattern = KAL_FALSE;
    imgsensor.current_fps = imgsensor_info.pre.max_framerate;
    spin_unlock(&imgsensor_drv_lock);
    GC8034DuringTestPattern = KAL_FALSE;

    return ERROR_NONE;
}    /*    open  */

/*************************************************************************
* FUNCTION
*    close
*
* DESCRIPTION
*
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
    LOG_INF("E\n");

    /*No Need to implement this function*/

    return ERROR_NONE;
}    /*    close  */

/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*    This function start the sensor preview.
*
* PARAMETERS
*    *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
    imgsensor.pclk = imgsensor_info.pre.pclk;
    //imgsensor.video_mode = KAL_FALSE;
    imgsensor.line_length = imgsensor_info.pre.linelength;
    imgsensor.frame_length = imgsensor_info.pre.framelength;
    imgsensor.min_frame_length = imgsensor_info.pre.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    preview_setting();
	//set_mirror_flip(sensor_config_data->SensorImageMirror);
    return ERROR_NONE;
}    /*    preview   */

/*************************************************************************
* FUNCTION
*    capture
*
* DESCRIPTION
*    This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                          MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
	LOG_INF("Enter capture!\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
    if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {//PIP capture: 24fps for less than 13M, 20fps for 16M,15fps for 20M
        imgsensor.pclk = imgsensor_info.cap1.pclk;
        imgsensor.line_length = imgsensor_info.cap1.linelength;
        imgsensor.frame_length = imgsensor_info.cap1.framelength;
        imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
        imgsensor.autoflicker_en = KAL_FALSE;
    } else {
        if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
            LOG_INF("Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",imgsensor.current_fps,imgsensor_info.cap.max_framerate/10);
        imgsensor.pclk = imgsensor_info.cap.pclk;
        imgsensor.line_length = imgsensor_info.cap.linelength;
        imgsensor.frame_length = imgsensor_info.cap.framelength;
        imgsensor.min_frame_length = imgsensor_info.cap.framelength;
        imgsensor.autoflicker_en = KAL_FALSE;
    }
    spin_unlock(&imgsensor_drv_lock);
    capture_setting(imgsensor.current_fps);
	//set_mirror_flip(sensor_config_data->SensorImageMirror);
    return ERROR_NONE;
}    /* capture() */

static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
	LOG_INF("Enter normal_video_setting!\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
    imgsensor.pclk = imgsensor_info.normal_video.pclk;
    imgsensor.line_length = imgsensor_info.normal_video.linelength;
    imgsensor.frame_length = imgsensor_info.normal_video.framelength;
    imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
    //imgsensor.current_fps = 300;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    normal_video_setting(imgsensor.current_fps);
    return ERROR_NONE;
}    /*    normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
	LOG_INF("Enter hs_video!\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
    imgsensor.pclk = imgsensor_info.hs_video.pclk;
    //imgsensor.video_mode = KAL_TRUE;
    imgsensor.line_length = imgsensor_info.hs_video.linelength;
    imgsensor.frame_length = imgsensor_info.hs_video.framelength;
    imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
    imgsensor.dummy_line = 0;
    imgsensor.dummy_pixel = 0;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    hs_video_setting();
    return ERROR_NONE;
}    /*    hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
	LOG_INF("Enter slim_video!\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
    imgsensor.pclk = imgsensor_info.slim_video.pclk;
    imgsensor.line_length = imgsensor_info.slim_video.linelength;
    imgsensor.frame_length = imgsensor_info.slim_video.framelength;
    imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
    imgsensor.dummy_line = 0;
    imgsensor.dummy_pixel = 0;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    slim_video_setting();
	//set_mirror_flip(sensor_config_data->SensorImageMirror);

    return ERROR_NONE;
}    /*    slim_video     */

static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
    LOG_INF("E\n");
    sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
    sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;

    sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
    sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;

    sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
    sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;


    sensor_resolution->SensorHighSpeedVideoWidth     = imgsensor_info.hs_video.grabwindow_width;
    sensor_resolution->SensorHighSpeedVideoHeight     = imgsensor_info.hs_video.grabwindow_height;

    sensor_resolution->SensorSlimVideoWidth     = imgsensor_info.slim_video.grabwindow_width;
    sensor_resolution->SensorSlimVideoHeight     = imgsensor_info.slim_video.grabwindow_height;
    return ERROR_NONE;
}    /*    get_resolution    */

static kal_uint32 get_info(MSDK_SCENARIO_ID_ENUM scenario_id,
                      MSDK_SENSOR_INFO_STRUCT *sensor_info,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
     LOG_INF("Enter get_info!\n");
    LOG_INF("scenario_id = %d\n", scenario_id);


    //sensor_info->SensorVideoFrameRate = imgsensor_info.normal_video.max_framerate/10; /* not use */
    //sensor_info->SensorStillCaptureFrameRate= imgsensor_info.cap.max_framerate/10; /* not use */
    //imgsensor_info->SensorWebCamCaptureFrameRate= imgsensor_info.v.max_framerate; /* not use */

    sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
    sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
    sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
    sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    sensor_info->SensorInterruptDelayLines = 4; /* not use */
    sensor_info->SensorResetActiveHigh = FALSE; /* not use */
    sensor_info->SensorResetDelayCount = 5; /* not use */

    sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
    sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
    sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
    sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;

    sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
    sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
    sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
    sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
    sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;

    sensor_info->SensorMasterClockSwitch = 0; /* not use */
    sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

    sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;          /* The frame of setting shutter default 0 for TG int */
    sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;    /* The frame of setting sensor gain */
    sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;
    sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
    sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
    sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;

    sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
    sensor_info->SensorClockFreq = imgsensor_info.mclk;
    sensor_info->SensorClockDividCount = 3; /* not use */
    sensor_info->SensorClockRisingCount = 0;
    sensor_info->SensorClockFallingCount = 2; /* not use */
    sensor_info->SensorPixelClockCount = 3; /* not use */
    sensor_info->SensorDataLatchCount = 2; /* not use */

    sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
    sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
    sensor_info->SensorWidthSampling = 0;  // 0 is default 1x
    sensor_info->SensorHightSampling = 0;    // 0 is default 1x
    sensor_info->SensorPacketECCOrder = 1;

    switch (scenario_id) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;

            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

            sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;

            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;

            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;

            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

            break;
        default:
            sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
            break;
    }

    return ERROR_NONE;
}    /*    get_info  */

static kal_uint32 control(MSDK_SCENARIO_ID_ENUM scenario_id, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("Enter control!\n");
    LOG_INF("scenario_id = %d\n", scenario_id);
    spin_lock(&imgsensor_drv_lock);
    imgsensor.current_scenario_id = scenario_id;
    spin_unlock(&imgsensor_drv_lock);
    switch (scenario_id) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            preview(image_window, sensor_config_data);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            capture(image_window, sensor_config_data);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            normal_video(image_window, sensor_config_data);
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            hs_video(image_window, sensor_config_data);
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            slim_video(image_window, sensor_config_data);
            break;
        default:
            LOG_INF("Error ScenarioId setting");
            preview(image_window, sensor_config_data);
            return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
}    /* control() */

static kal_uint32 set_video_mode(UINT16 framerate)
{//This Function not used after ROME
    LOG_INF("framerate = %d\n ", framerate);
    // SetVideoMode Function should fix framerate
    if (framerate == 0)
        // Dynamic frame rate
        return ERROR_NONE;
    spin_lock(&imgsensor_drv_lock);
    if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
        imgsensor.current_fps = 296;
    else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
        imgsensor.current_fps = 146;
    else
        imgsensor.current_fps = framerate;
    spin_unlock(&imgsensor_drv_lock);
    set_max_framerate(imgsensor.current_fps,1);

    return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
    LOG_INF("enable = %d, framerate = %d \n", enable, framerate);
    spin_lock(&imgsensor_drv_lock);
    if (enable) //enable auto flicker
        imgsensor.autoflicker_en = KAL_TRUE;
    else //Cancel Auto flick
        imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    return ERROR_NONE;
}

static kal_uint32 set_max_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
{
    kal_uint32 frame_length;

    LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

    switch (scenario_id) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
            imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            if(framerate == 0)
                return ERROR_NONE;
            frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength) : 0;
            imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        	  if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
                frame_length = imgsensor_info.cap1.pclk / framerate * 10 / imgsensor_info.cap1.linelength;
                spin_lock(&imgsensor_drv_lock);
		            imgsensor.dummy_line = (frame_length > imgsensor_info.cap1.framelength) ? (frame_length - imgsensor_info.cap1.framelength) : 0;
		            imgsensor.frame_length = imgsensor_info.cap1.framelength + imgsensor.dummy_line;
		            imgsensor.min_frame_length = imgsensor.frame_length;
		            spin_unlock(&imgsensor_drv_lock);
            } else {
        		    if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
                    LOG_INF("Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",framerate,imgsensor_info.cap.max_framerate/10);
                frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
                spin_lock(&imgsensor_drv_lock);
		            imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
		            imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
		            imgsensor.min_frame_length = imgsensor.frame_length;
		            spin_unlock(&imgsensor_drv_lock);
            }
            set_dummy();
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
            imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength): 0;
            imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
            break;
        default:  //coding with  preview scenario by default
            frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
            imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
            LOG_INF("error scenario_id = %d, we use preview scenario \n", scenario_id);
            break;
    }
    return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{
    LOG_INF("scenario_id = %d\n", scenario_id);

    switch (scenario_id) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            *framerate = imgsensor_info.pre.max_framerate;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            *framerate = imgsensor_info.normal_video.max_framerate;
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            *framerate = imgsensor_info.cap.max_framerate;
            break;
        case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            *framerate = imgsensor_info.hs_video.max_framerate;
            break;
        case MSDK_SCENARIO_ID_SLIM_VIDEO:
            *framerate = imgsensor_info.slim_video.max_framerate;
            break;
        default:
            break;
    }

    return ERROR_NONE;
}



static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
                             UINT8 *feature_para,UINT32 *feature_para_len)
{
    UINT16 *feature_return_para_16=(UINT16 *) feature_para;
    UINT16 *feature_data_16=(UINT16 *) feature_para;
    UINT32 *feature_return_para_32=(UINT32 *) feature_para;
    UINT32 *feature_data_32=(UINT32 *) feature_para;
    unsigned long long *feature_data=(unsigned long long *) feature_para;
    //unsigned long long *feature_return_para=(unsigned long long *) feature_para;

    SENSOR_WINSIZE_INFO_STRUCT *wininfo;
    MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data=(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

    LOG_INF("feature_id = %d\n", feature_id);
    switch (feature_id) {
        case SENSOR_FEATURE_GET_PERIOD:
            *feature_return_para_16++ = imgsensor.line_length;
            *feature_return_para_16 = imgsensor.frame_length;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            *feature_return_para_32 = imgsensor.pclk;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            set_shutter(*feature_data);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            night_mode((BOOL) *feature_data);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            set_gain((UINT16) *feature_data);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *feature_return_para_32=LENS_DRIVER_ID_DO_NOT_CARE;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            set_video_mode(*feature_data);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            get_imgsensor_id(feature_return_para_32);
            break;
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            set_auto_flicker_mode((BOOL)*feature_data_16,*(feature_data_16+1));
            break;
        case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
            set_max_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*feature_data, *(feature_data+1));
            break;
        case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
            get_default_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*(feature_data), (MUINT32 *)(uintptr_t)(*(feature_data+1)));
            break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            set_test_pattern_mode((BOOL)*feature_data);
            break;
        case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: //for factory mode auto testing
            *feature_return_para_32 = imgsensor_info.checksum_value;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_SET_FRAMERATE:
            LOG_INF("current fps :%d\n", (UINT32)*feature_data);
            spin_lock(&imgsensor_drv_lock);
            imgsensor.current_fps = *feature_data;
            spin_unlock(&imgsensor_drv_lock);
            break;
        case SENSOR_FEATURE_SET_HDR:
            LOG_INF("ihdr enable :%d\n", (BOOL)*feature_data);
            spin_lock(&imgsensor_drv_lock);
            imgsensor.ihdr_en = (BOOL)*feature_data;
            spin_unlock(&imgsensor_drv_lock);
            break;
        case SENSOR_FEATURE_GET_CROP_INFO:
            LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n", (UINT32)*feature_data);

            wininfo = (SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

            switch (*feature_data_32) {
                case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[1],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[2],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[3],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_SLIM_VIDEO:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[4],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
                default:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[0],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
            }
            break;
        case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
            LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",(UINT16)*feature_data,(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
            ihdr_write_shutter_gain((UINT16)*feature_data,(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
            break;
        default:
            break;
    }

    return ERROR_NONE;
}    /*    feature_control()  */

static SENSOR_FUNCTION_STRUCT sensor_func = {
    open,
    get_info,
    get_resolution,
    feature_control,
    control,
    close
};

UINT32 GC8034MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&sensor_func;
    return ERROR_NONE;
}    /*    GC8034MIPI_RAW_SensorInit    */
