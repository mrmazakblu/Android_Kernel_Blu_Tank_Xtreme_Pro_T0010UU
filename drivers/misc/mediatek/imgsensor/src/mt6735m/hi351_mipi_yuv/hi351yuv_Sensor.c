
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
//#include <mach/mt6516_pll.h>
#include "kd_camera_typedef.h"

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "hi351yuv_Sensor.h"
#include "hi351yuv_Camera_Sensor_para.h"
#include "hi351yuv_CameraCustomized.h"

#define HI351_DEBUG
#ifdef HI351_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

MSDK_SENSOR_CONFIG_STRUCT HI351SensorConfigData;
#define SENSOR_CORE_PCLK	83200000	//48M PCLK Output 78000000

#define WINMO_USE 0
#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool HI351_VEDIO_MPEG4 = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4);

kal_uint8 HI351_Sleep_Mode;
kal_uint32 HI351_PV_dummy_pixels=616,HI351_PV_dummy_lines=20,HI351_isp_master_clock=260/*0*/;

//static HI351_SENSOR_INFO_ST HI351_sensor;
static HI351_OPERATION_STATE_ST HI351_op_state;

static kal_uint32 zoom_factor = 0;
static kal_bool HI351_gPVmode = KAL_TRUE; //PV size or Full size
static kal_bool HI351_VEDIO_encode_mode = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4)
static kal_bool HI351_sensor_cap_state = KAL_FALSE; //Preview or Capture

static kal_uint8 HI351_Banding_setting = AE_FLICKER_MODE_50HZ;  //Wonder add
//static kal_uint16  HI351_PV_Shutter = 0;
static kal_uint32  HI351_sensor_pclk=260;//520 //20110518

//static kal_uint32 HI351_pv_HI351_exposure_lines=0x017a00, HI351_cp_HI351_exposure_lines=0;
//static kal_uint16 HI351_Capture_Max_Gain16= 6*16;
//static kal_uint16 HI351_Capture_Gain16=0 ;
//static kal_uint16 HI351_Capture_Shutter=0;
//static kal_uint16 HI351_Capture_Extra_Lines=0;

//static kal_uint16  HI351_PV_Gain16 = 0;
//static kal_uint16  HI351_PV_Extra_Lines = 0;
kal_uint32 HI351_capture_pclk_in_M=520,HI351_preview_pclk_in_M=390;

//extern static CAMERA_DUAL_CAMERA_SENSOR_ENUM g_currDualSensorIdx;
//extern static char g_currSensorName[32];
//extern int kdModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name);

extern int iReadReg_Byte(u8 addr, u8 *buf, u8 i2cId);
extern int iWriteReg_Byte(u8 addr, u8 buf, u32 size, u16 i2cId);
//SENSOR_REG_STRUCT HI351SensorCCT[FACTORY_END_ADDR]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
//SENSOR_REG_STRUCT HI351SensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
//	camera_para.SENSOR.cct	SensorCCT	=> SensorCCT
//	camera_para.SENSOR.reg	SensorReg

BOOL HI351_set_param_banding(UINT16 para);

//extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff, u16 i2cId);
//extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes, u16 i2cId);
//extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
///*ergate-017*/
//extern int iWriteRegI2C_ext(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId, u16 speed);
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
kal_uint16 HI351_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
//	char puSendCmd[2] = {(char)(addr & 0xFF) ,(char)(para & 0xFF)};

	//iWriteRegI2C_ext(puSendCmd , 2,HI351_WRITE_ID_0, 50);
	//iWriteReg_Byte(addr, para, 1, HI351_WRITE_ID_0);

    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};

	return (kal_uint16)iWriteRegI2C(puSendCmd , 2,HI351_WRITE_ID_0);
}

kal_uint8 HI351_read_cmos_sensor(kal_uint8 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
	iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte,1,HI351_WRITE_ID_0);

    return get_byte;
//	kal_uint8 get_byte=0;
//    iReadRegI2C(addr, &get_byte,1, HI351_WRITE_ID_0);
//    return get_byte;
}

bool HI351_checkID(void)
{

	kal_uint16 sensor_id=0;
	sensor_id = HI351_read_cmos_sensor(0x04);
	printk("[Hi351] sensor id = 0x%x\n", sensor_id);
	if (HI351MIPI_SENSOR_ID != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;
	else
		return ERROR_NONE;
}


void HI351_Init_Cmds(void)
{
//	volatile signed char i;
//	kal_uint16 sensor_id=0;
	zoom_factor = 0;

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf1);
HI351_write_cmos_sensor(0x01, 0xf3);
HI351_write_cmos_sensor(0x01, 0xf1);

///////////////////////////////////////////
// 0 Page PLL setting
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x07, 0x25); //24/(5+1) = 4Mhz
HI351_write_cmos_sensor(0x08, 0x60); // 96Mhz
HI351_write_cmos_sensor(0x09, 0x82);
HI351_write_cmos_sensor(0x07, 0xa5);
HI351_write_cmos_sensor(0x07, 0xa5);
HI351_write_cmos_sensor(0x09, 0xa2);

HI351_write_cmos_sensor(0x0A, 0x01); // MCU hardware reset
HI351_write_cmos_sensor(0x0A, 0x00);
HI351_write_cmos_sensor(0x0A, 0x01);
HI351_write_cmos_sensor(0x0A, 0x00);

///////////////////////////////////////////
// 20 Page OTP/ROM LSC download select setting
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x20);
HI351_write_cmos_sensor(0x3a, 0x00);
HI351_write_cmos_sensor(0x3b, 0x00);
HI351_write_cmos_sensor(0x3c, 0x00);

///////////////////////////////////////////
// 30 Page MCU reset, enable setting
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x30, 0x86);
HI351_write_cmos_sensor(0x31, 0x00);
HI351_write_cmos_sensor(0x32, 0x0c);
HI351_write_cmos_sensor(0xe0, 0x00); //new version
HI351_write_cmos_sensor(0x10, 0x80); // mcu reset high
HI351_write_cmos_sensor(0x10, 0x8a); // mcu enable high
HI351_write_cmos_sensor(0x11, 0x08); // xdata memory reset high
HI351_write_cmos_sensor(0x11, 0x00); // xdata memory reset low

///////////////////////////////////////////
// Copy OTP register to Xdata.
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x07); // otp page
HI351_write_cmos_sensor(0x12, 0x01); // memory mux on(mcu)  //new version  just use no change  system setting
//108M
HI351_write_cmos_sensor(0x40, 0x35); // otp_cfg1 value is different by PLLMCLK
HI351_write_cmos_sensor(0x47, 0x0B); // otp_cfg8 value is different by PLLMCLK
//DPC reg in otp down to xdata
HI351_write_cmos_sensor(0x2E, 0x00); // OTP (DPC block) start addr h(OTP DPC -> xdata DPC)
HI351_write_cmos_sensor(0x2F, 0x20); // OTP (DPC block) start addr l
HI351_write_cmos_sensor(0x30, 0x00); // MCU (DPC block) sram addr h(OTP DPC -> xdata DPC)
HI351_write_cmos_sensor(0x31, 0xD6); // MCU (DPC block) sram addr l
HI351_write_cmos_sensor(0x32, 0x00); // Copy reg sram size h(DPC reg size)
HI351_write_cmos_sensor(0x33, 0xFF); // Copy reg sram size l
HI351_write_cmos_sensor(0x10, 0x02); // Copy mcu down set(OTP DPC -> xdata DPC)
HI351_write_cmos_sensor(0x8C, 0x08); //OTP DPC pos offset
HI351_write_cmos_sensor(0x8F, 0x20);
HI351_write_cmos_sensor(0x92, 0x00);
HI351_write_cmos_sensor(0x93, 0x54); //Full size normal No_flip case
HI351_write_cmos_sensor(0x94, 0x00);
HI351_write_cmos_sensor(0x95, 0x11); //Full size normal No_flip case
mdelay(50);
// Color ratio reg in otp down to xdata
HI351_write_cmos_sensor(0x2E, 0x03); // OTP (ColorRatio block) start addr h(OTP ColorRatio->xdata ColorRatio)
HI351_write_cmos_sensor(0x2F, 0x20); // OTP (ColorRatio block) start addr l
HI351_write_cmos_sensor(0x30, 0x20); // MCU(ColorRatio block) sram addr h(OTP ColorRatio->xdata ColorRatio)
HI351_write_cmos_sensor(0x31, 0xA6); // MCU (ColorRatio block) sram addr l
HI351_write_cmos_sensor(0x32, 0x01); // Copy reg sram size h(MCU reg size)
HI351_write_cmos_sensor(0x33, 0x00); // Copy reg sram size l
HI351_write_cmos_sensor(0x10, 0x02); // Copy mcu down set(OTP ColorRatio -> xdata ColorRatio)
mdelay(50);
HI351_write_cmos_sensor(0x12, 0x00); // memory mux off
HI351_write_cmos_sensor(0x98, 0x00); // dpc_mem_ctl1
HI351_write_cmos_sensor(0x97, 0x01); // otp_dpc_ctl1

///////////////////////////////////////////
// 30 Page MCU reset, enable setting
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x10, 0x0a);// mcu reset low  = mcu start!!, mcu clk div = 1/3

///////////////////////////////////////////
// 0 Page
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x0B, 0x02); //PLL lock time
HI351_write_cmos_sensor(0x10, 0x00); //Full
HI351_write_cmos_sensor(0x11, 0x90); //1 frame skip
HI351_write_cmos_sensor(0x13, 0x80);
HI351_write_cmos_sensor(0x14, 0x20);
HI351_write_cmos_sensor(0x15, 0x03);
HI351_write_cmos_sensor(0x17, 0x04); //Parallel, MIPI : 04, JPEG : 0c

HI351_write_cmos_sensor(0x20, 0x00);
HI351_write_cmos_sensor(0x21, 0x00);
HI351_write_cmos_sensor(0x22, 0x00);
HI351_write_cmos_sensor(0x23, 0x12);

HI351_write_cmos_sensor(0x24, 0x06);
HI351_write_cmos_sensor(0x25, 0x00);
HI351_write_cmos_sensor(0x26, 0x08);
HI351_write_cmos_sensor(0x27, 0x00);

HI351_write_cmos_sensor(0x03, 0x00); //Page 0
HI351_write_cmos_sensor(0x50, 0x00); //Hblank 20
HI351_write_cmos_sensor(0x51, 0x14);
HI351_write_cmos_sensor(0x52, 0x00); //Vblank 14
HI351_write_cmos_sensor(0x53, 0x0e);
//BLC
HI351_write_cmos_sensor(0x80, 0x02);
HI351_write_cmos_sensor(0x81, 0x87);
HI351_write_cmos_sensor(0x82, 0x28);
HI351_write_cmos_sensor(0x83, 0x08);
HI351_write_cmos_sensor(0x84, 0x8c);
HI351_write_cmos_sensor(0x85, 0x0c); //blc on
HI351_write_cmos_sensor(0x86, 0x00);
HI351_write_cmos_sensor(0x87, 0x00);
HI351_write_cmos_sensor(0x88, 0x98);
HI351_write_cmos_sensor(0x89, 0x10);
HI351_write_cmos_sensor(0x8a, 0x80);
HI351_write_cmos_sensor(0x8b, 0x00);
HI351_write_cmos_sensor(0x8e, 0x80);
HI351_write_cmos_sensor(0x8f, 0x0f);
HI351_write_cmos_sensor(0x90, 0x0f); //BLC_TIME_TH_ON
HI351_write_cmos_sensor(0x91, 0x0f); //BLC_TIME_TH_OFF
HI351_write_cmos_sensor(0x92, 0xa0); //BLC_AG_TH_ON
HI351_write_cmos_sensor(0x93, 0x90); //BLC_AG_TH_OFF
HI351_write_cmos_sensor(0x96, 0xfe); //BLC_OUT_TH
HI351_write_cmos_sensor(0x97, 0xE0); //BLC_OUT_TH
HI351_write_cmos_sensor(0x98, 0x20);
HI351_write_cmos_sensor(0xa1, 0x81); //odd_adj_out
HI351_write_cmos_sensor(0xa2, 0x83); //odd_adj_in
HI351_write_cmos_sensor(0xa3, 0x86); //odd_adj_dark
HI351_write_cmos_sensor(0xa5, 0x81); //even_adj_out
HI351_write_cmos_sensor(0xa6, 0x83); //even_adj_in
HI351_write_cmos_sensor(0xa7, 0x86); //even_adj_dark
HI351_write_cmos_sensor(0xbb, 0x20);

///////////////////////////////////////////
// 2 Page
///////////////////////////////////////////

HI351_write_cmos_sensor(0x03, 0x02);
HI351_write_cmos_sensor(0x10, 0x00);
HI351_write_cmos_sensor(0x13, 0x00);
HI351_write_cmos_sensor(0x14, 0x00);
HI351_write_cmos_sensor(0x15, 0x08);
HI351_write_cmos_sensor(0x1a, 0x00);//ncp adaptive off
HI351_write_cmos_sensor(0x1b, 0x00);
HI351_write_cmos_sensor(0x1c, 0xc0);
HI351_write_cmos_sensor(0x1d, 0x00);//MCU update bit[4]
HI351_write_cmos_sensor(0x20, 0x44);
HI351_write_cmos_sensor(0x21, 0x02);
HI351_write_cmos_sensor(0x22, 0x22);
HI351_write_cmos_sensor(0x23, 0x30);//clamp on 10 -30
HI351_write_cmos_sensor(0x24, 0x77);
HI351_write_cmos_sensor(0x2b, 0x00);
HI351_write_cmos_sensor(0x2c, 0x0C);
HI351_write_cmos_sensor(0x2d, 0x80);
HI351_write_cmos_sensor(0x2e, 0x00);
HI351_write_cmos_sensor(0x2f, 0x00);
HI351_write_cmos_sensor(0x30, 0x00);
HI351_write_cmos_sensor(0x31, 0xf0);
HI351_write_cmos_sensor(0x32, 0x22);
HI351_write_cmos_sensor(0x33, 0x42); //auto flicker
HI351_write_cmos_sensor(0x34, 0x30);
HI351_write_cmos_sensor(0x35, 0x00);
HI351_write_cmos_sensor(0x36, 0x08);
HI351_write_cmos_sensor(0x37, 0x40); //auto flicker
HI351_write_cmos_sensor(0x38, 0x14);
HI351_write_cmos_sensor(0x39, 0x02);
HI351_write_cmos_sensor(0x3a, 0x00);

HI351_write_cmos_sensor(0x3d, 0x70);
HI351_write_cmos_sensor(0x3e, 0x04);
HI351_write_cmos_sensor(0x3f, 0x00);
HI351_write_cmos_sensor(0x40, 0x01);
HI351_write_cmos_sensor(0x41, 0x8a);
HI351_write_cmos_sensor(0x42, 0x00);
HI351_write_cmos_sensor(0x43, 0x25);
HI351_write_cmos_sensor(0x44, 0x00);
HI351_write_cmos_sensor(0x46, 0x00);
HI351_write_cmos_sensor(0x47, 0x00);
HI351_write_cmos_sensor(0x48, 0x3C);
HI351_write_cmos_sensor(0x49, 0x10);
HI351_write_cmos_sensor(0x4a, 0x00);
HI351_write_cmos_sensor(0x4b, 0x10);
HI351_write_cmos_sensor(0x4c, 0x08);
HI351_write_cmos_sensor(0x4d, 0x70);
HI351_write_cmos_sensor(0x4e, 0x04);
HI351_write_cmos_sensor(0x4f, 0x38);
HI351_write_cmos_sensor(0x50, 0xa0);
HI351_write_cmos_sensor(0x51, 0x00);
HI351_write_cmos_sensor(0x52, 0x70);
HI351_write_cmos_sensor(0x53, 0x00);
HI351_write_cmos_sensor(0x54, 0xc0);
HI351_write_cmos_sensor(0x55, 0x40);
HI351_write_cmos_sensor(0x56, 0x11);
HI351_write_cmos_sensor(0x57, 0x00);
HI351_write_cmos_sensor(0x58, 0x10);
HI351_write_cmos_sensor(0x59, 0x0E);
HI351_write_cmos_sensor(0x5a, 0x00);
HI351_write_cmos_sensor(0x5b, 0x00);
HI351_write_cmos_sensor(0x5c, 0x00);
HI351_write_cmos_sensor(0x5d, 0x00);
HI351_write_cmos_sensor(0x60, 0x04);
HI351_write_cmos_sensor(0x61, 0xe2);
HI351_write_cmos_sensor(0x62, 0x00);
HI351_write_cmos_sensor(0x63, 0xc8);
HI351_write_cmos_sensor(0x64, 0x00);
HI351_write_cmos_sensor(0x65, 0x00);
HI351_write_cmos_sensor(0x66, 0x00);
HI351_write_cmos_sensor(0x67, 0x3f);
HI351_write_cmos_sensor(0x68, 0x3f);
HI351_write_cmos_sensor(0x69, 0x3f);
HI351_write_cmos_sensor(0x6a, 0x04);
HI351_write_cmos_sensor(0x6b, 0x38);
HI351_write_cmos_sensor(0x6c, 0x00);
HI351_write_cmos_sensor(0x6d, 0x00);
HI351_write_cmos_sensor(0x6e, 0x00);
HI351_write_cmos_sensor(0x6f, 0x00);
HI351_write_cmos_sensor(0x70, 0x00);
HI351_write_cmos_sensor(0x71, 0x50);
HI351_write_cmos_sensor(0x72, 0x05);
HI351_write_cmos_sensor(0x73, 0xa5);
HI351_write_cmos_sensor(0x74, 0x00);
HI351_write_cmos_sensor(0x75, 0x50);
HI351_write_cmos_sensor(0x76, 0x02);
HI351_write_cmos_sensor(0x77, 0xfa);
HI351_write_cmos_sensor(0x78, 0x01);
HI351_write_cmos_sensor(0x79, 0xb4);
HI351_write_cmos_sensor(0x7a, 0x01);
HI351_write_cmos_sensor(0x7b, 0xb8);
HI351_write_cmos_sensor(0x7c, 0x00);
HI351_write_cmos_sensor(0x7d, 0x00);
HI351_write_cmos_sensor(0x7e, 0x00);
HI351_write_cmos_sensor(0x7f, 0x00);
HI351_write_cmos_sensor(0xa0, 0x00);
HI351_write_cmos_sensor(0xa1, 0xEB);
HI351_write_cmos_sensor(0xa2, 0x02);
HI351_write_cmos_sensor(0xa3, 0x2D);
HI351_write_cmos_sensor(0xa4, 0x02);
HI351_write_cmos_sensor(0xa5, 0xB9);
HI351_write_cmos_sensor(0xa6, 0x05);
HI351_write_cmos_sensor(0xa7, 0xED);
HI351_write_cmos_sensor(0xa8, 0x00);
HI351_write_cmos_sensor(0xa9, 0xEB);
HI351_write_cmos_sensor(0xaa, 0x01);
HI351_write_cmos_sensor(0xab, 0xED);
HI351_write_cmos_sensor(0xac, 0x02);
HI351_write_cmos_sensor(0xad, 0x79);
HI351_write_cmos_sensor(0xae, 0x04);
HI351_write_cmos_sensor(0xaf, 0x2D);
HI351_write_cmos_sensor(0xb0, 0x00);
HI351_write_cmos_sensor(0xb1, 0x56);
HI351_write_cmos_sensor(0xb2, 0x01);
HI351_write_cmos_sensor(0xb3, 0x08);
HI351_write_cmos_sensor(0xb4, 0x00);
HI351_write_cmos_sensor(0xb5, 0x2B);
HI351_write_cmos_sensor(0xb6, 0x03);
HI351_write_cmos_sensor(0xb7, 0x2B);
HI351_write_cmos_sensor(0xb8, 0x00);
HI351_write_cmos_sensor(0xb9, 0x56);
HI351_write_cmos_sensor(0xba, 0x00);
HI351_write_cmos_sensor(0xbb, 0xC8);
HI351_write_cmos_sensor(0xbc, 0x00);
HI351_write_cmos_sensor(0xbd, 0x2B);
HI351_write_cmos_sensor(0xbe, 0x01);
HI351_write_cmos_sensor(0xbf, 0xAB);
HI351_write_cmos_sensor(0xc0, 0x00);
HI351_write_cmos_sensor(0xc1, 0x54);
HI351_write_cmos_sensor(0xc2, 0x01);
HI351_write_cmos_sensor(0xc3, 0x0A);
HI351_write_cmos_sensor(0xc4, 0x00);
HI351_write_cmos_sensor(0xc5, 0x29);
HI351_write_cmos_sensor(0xc6, 0x03);
HI351_write_cmos_sensor(0xc7, 0x2D);
HI351_write_cmos_sensor(0xc8, 0x00);
HI351_write_cmos_sensor(0xc9, 0x54);
HI351_write_cmos_sensor(0xca, 0x00);
HI351_write_cmos_sensor(0xcb, 0xCA);
HI351_write_cmos_sensor(0xcc, 0x00);
HI351_write_cmos_sensor(0xcd, 0x29);
HI351_write_cmos_sensor(0xce, 0x01);
HI351_write_cmos_sensor(0xcf, 0xAD);
HI351_write_cmos_sensor(0xd0, 0x10);
HI351_write_cmos_sensor(0xd1, 0x14);
HI351_write_cmos_sensor(0xd2, 0x20);
HI351_write_cmos_sensor(0xd3, 0x00);
HI351_write_cmos_sensor(0xd4, 0x0c); //DCDC_TIME_TH_ON
HI351_write_cmos_sensor(0xd5, 0x0c); //DCDC_TIME_TH_OFF
HI351_write_cmos_sensor(0xd6, 0x78); //DCDC_AG_TH_ON
HI351_write_cmos_sensor(0xd7, 0x70); //DCDC_AG_TH_OFF
HI351_write_cmos_sensor(0xE0, 0xf0);//ncp adaptive
HI351_write_cmos_sensor(0xE1, 0xf0);//ncp adaptive
HI351_write_cmos_sensor(0xE2, 0xf0);//ncp adaptive
HI351_write_cmos_sensor(0xE3, 0xf0);//ncp adaptive
HI351_write_cmos_sensor(0xE4, 0xd0);//ncp adaptive e1 -) d0
HI351_write_cmos_sensor(0xE5, 0x00);//ncp adaptive
HI351_write_cmos_sensor(0xE6, 0x00);
HI351_write_cmos_sensor(0xE7, 0x00);
HI351_write_cmos_sensor(0xE8, 0x00);
HI351_write_cmos_sensor(0xE9, 0x00);
HI351_write_cmos_sensor(0xEA, 0x15);
HI351_write_cmos_sensor(0xEB, 0x15);
HI351_write_cmos_sensor(0xEC, 0x15);
HI351_write_cmos_sensor(0xED, 0x05);
HI351_write_cmos_sensor(0xEE, 0x05);
HI351_write_cmos_sensor(0xEF, 0x65);
HI351_write_cmos_sensor(0xF0, 0x0c);
HI351_write_cmos_sensor(0xF3, 0x05);
HI351_write_cmos_sensor(0xF4, 0x0a);
HI351_write_cmos_sensor(0xF5, 0x05);
HI351_write_cmos_sensor(0xF6, 0x05);
HI351_write_cmos_sensor(0xF7, 0x15);
HI351_write_cmos_sensor(0xF8, 0x15);
HI351_write_cmos_sensor(0xF9, 0x15);
HI351_write_cmos_sensor(0xFA, 0x15);
HI351_write_cmos_sensor(0xFB, 0x15);
HI351_write_cmos_sensor(0xFC, 0x55);
HI351_write_cmos_sensor(0xFD, 0x55);
HI351_write_cmos_sensor(0xFE, 0x05);
HI351_write_cmos_sensor(0xFF, 0x00);
///////////////////////////////////////////
//3Page
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x03);
HI351_write_cmos_sensor(0x10, 0x00);
HI351_write_cmos_sensor(0x11, 0x64);
HI351_write_cmos_sensor(0x12, 0x00);
HI351_write_cmos_sensor(0x13, 0x32);
HI351_write_cmos_sensor(0x14, 0x02);
HI351_write_cmos_sensor(0x15, 0x51);
HI351_write_cmos_sensor(0x16, 0x02);
HI351_write_cmos_sensor(0x17, 0x59);
HI351_write_cmos_sensor(0x18, 0x00);
HI351_write_cmos_sensor(0x19, 0x97);
HI351_write_cmos_sensor(0x1a, 0x01);
HI351_write_cmos_sensor(0x1b, 0x7C);
HI351_write_cmos_sensor(0x1c, 0x00);
HI351_write_cmos_sensor(0x1d, 0x97);
HI351_write_cmos_sensor(0x1e, 0x01);
HI351_write_cmos_sensor(0x1f, 0x7C);
HI351_write_cmos_sensor(0x20, 0x00);
HI351_write_cmos_sensor(0x21, 0x97);
HI351_write_cmos_sensor(0x22, 0x00);
HI351_write_cmos_sensor(0x23, 0xe3); //cds 2 off time sunspot
HI351_write_cmos_sensor(0x24, 0x00);
HI351_write_cmos_sensor(0x25, 0x97);
HI351_write_cmos_sensor(0x26, 0x00);
HI351_write_cmos_sensor(0x27, 0xe3); //cds 2 off time  sunspot

HI351_write_cmos_sensor(0x28, 0x00);
HI351_write_cmos_sensor(0x29, 0x97);
HI351_write_cmos_sensor(0x2a, 0x00);
HI351_write_cmos_sensor(0x2b, 0xE6);
HI351_write_cmos_sensor(0x2c, 0x00);
HI351_write_cmos_sensor(0x2d, 0x97);
HI351_write_cmos_sensor(0x2e, 0x00);
HI351_write_cmos_sensor(0x2f, 0xE6);
HI351_write_cmos_sensor(0x30, 0x00);
HI351_write_cmos_sensor(0x31, 0x0a);
HI351_write_cmos_sensor(0x32, 0x03);
HI351_write_cmos_sensor(0x33, 0x31);
HI351_write_cmos_sensor(0x34, 0x00);
HI351_write_cmos_sensor(0x35, 0x0a);
HI351_write_cmos_sensor(0x36, 0x03);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x38, 0x00);
HI351_write_cmos_sensor(0x39, 0x0A);
HI351_write_cmos_sensor(0x3a, 0x01);
HI351_write_cmos_sensor(0x3b, 0xB0);
HI351_write_cmos_sensor(0x3c, 0x00);
HI351_write_cmos_sensor(0x3d, 0x0A);
HI351_write_cmos_sensor(0x3e, 0x01);
HI351_write_cmos_sensor(0x3f, 0xB0);
HI351_write_cmos_sensor(0x40, 0x00);
HI351_write_cmos_sensor(0x41, 0x04);
HI351_write_cmos_sensor(0x42, 0x00);
HI351_write_cmos_sensor(0x43, 0x1c);
HI351_write_cmos_sensor(0x44, 0x00);
HI351_write_cmos_sensor(0x45, 0x02);
HI351_write_cmos_sensor(0x46, 0x00);
HI351_write_cmos_sensor(0x47, 0x34);
HI351_write_cmos_sensor(0x48, 0x00);
HI351_write_cmos_sensor(0x49, 0x06);
HI351_write_cmos_sensor(0x4a, 0x00);
HI351_write_cmos_sensor(0x4b, 0x1a);
HI351_write_cmos_sensor(0x4c, 0x00);
HI351_write_cmos_sensor(0x4d, 0x06);
HI351_write_cmos_sensor(0x4e, 0x00);
HI351_write_cmos_sensor(0x4f, 0x1a);
HI351_write_cmos_sensor(0x50, 0x00);
HI351_write_cmos_sensor(0x51, 0x08);
HI351_write_cmos_sensor(0x52, 0x00);
HI351_write_cmos_sensor(0x53, 0x18);
HI351_write_cmos_sensor(0x54, 0x00);
HI351_write_cmos_sensor(0x55, 0x08);
HI351_write_cmos_sensor(0x56, 0x00);
HI351_write_cmos_sensor(0x57, 0x18);
HI351_write_cmos_sensor(0x58, 0x00);
HI351_write_cmos_sensor(0x59, 0x08);
HI351_write_cmos_sensor(0x5A, 0x00);
HI351_write_cmos_sensor(0x5b, 0x18);
HI351_write_cmos_sensor(0x5c, 0x00);
HI351_write_cmos_sensor(0x5d, 0x06);
HI351_write_cmos_sensor(0x5e, 0x00);
HI351_write_cmos_sensor(0x5f, 0x1c);
HI351_write_cmos_sensor(0x60, 0x00);
HI351_write_cmos_sensor(0x61, 0x00);
HI351_write_cmos_sensor(0x62, 0x00);
HI351_write_cmos_sensor(0x63, 0x00);
HI351_write_cmos_sensor(0x64, 0x00);
HI351_write_cmos_sensor(0x65, 0x00);
HI351_write_cmos_sensor(0x66, 0x00);
HI351_write_cmos_sensor(0x67, 0x00);
HI351_write_cmos_sensor(0x68, 0x00);
HI351_write_cmos_sensor(0x69, 0x02);
HI351_write_cmos_sensor(0x6A, 0x00);
HI351_write_cmos_sensor(0x6B, 0x1e);
HI351_write_cmos_sensor(0x6C, 0x00);
HI351_write_cmos_sensor(0x6D, 0x00);
HI351_write_cmos_sensor(0x6E, 0x00);
HI351_write_cmos_sensor(0x6F, 0x00);
HI351_write_cmos_sensor(0x70, 0x00);
HI351_write_cmos_sensor(0x71, 0x66);
HI351_write_cmos_sensor(0x72, 0x01);
HI351_write_cmos_sensor(0x73, 0x86);
HI351_write_cmos_sensor(0x74, 0x00);
HI351_write_cmos_sensor(0x75, 0x6B);
HI351_write_cmos_sensor(0x76, 0x00);
HI351_write_cmos_sensor(0x77, 0x93);
HI351_write_cmos_sensor(0x78, 0x01);
HI351_write_cmos_sensor(0x79, 0x84);
HI351_write_cmos_sensor(0x7a, 0x01);
HI351_write_cmos_sensor(0x7b, 0x88);
HI351_write_cmos_sensor(0x7c, 0x01);
HI351_write_cmos_sensor(0x7d, 0x84);
HI351_write_cmos_sensor(0x7e, 0x01);
HI351_write_cmos_sensor(0x7f, 0x88);
HI351_write_cmos_sensor(0x80, 0x01);
HI351_write_cmos_sensor(0x81, 0x13);
HI351_write_cmos_sensor(0x82, 0x01);
HI351_write_cmos_sensor(0x83, 0x3B);
HI351_write_cmos_sensor(0x84, 0x01);
HI351_write_cmos_sensor(0x85, 0x84);
HI351_write_cmos_sensor(0x86, 0x01);
HI351_write_cmos_sensor(0x87, 0x88);
HI351_write_cmos_sensor(0x88, 0x01);
HI351_write_cmos_sensor(0x89, 0x84);
HI351_write_cmos_sensor(0x8a, 0x01);
HI351_write_cmos_sensor(0x8b, 0x88);
HI351_write_cmos_sensor(0x8c, 0x01);
HI351_write_cmos_sensor(0x8d, 0x16);
HI351_write_cmos_sensor(0x8e, 0x01);
HI351_write_cmos_sensor(0x8f, 0x42);
HI351_write_cmos_sensor(0x90, 0x00);
HI351_write_cmos_sensor(0x91, 0x68);
HI351_write_cmos_sensor(0x92, 0x01);
HI351_write_cmos_sensor(0x93, 0x80);
HI351_write_cmos_sensor(0x94, 0x00);
HI351_write_cmos_sensor(0x95, 0x68);
HI351_write_cmos_sensor(0x96, 0x01);
HI351_write_cmos_sensor(0x97, 0x80);
HI351_write_cmos_sensor(0x98, 0x01);
HI351_write_cmos_sensor(0x99, 0x80);
HI351_write_cmos_sensor(0x9a, 0x00);
HI351_write_cmos_sensor(0x9b, 0x68);
HI351_write_cmos_sensor(0x9c, 0x01);
HI351_write_cmos_sensor(0x9d, 0x80);
HI351_write_cmos_sensor(0x9e, 0x00);
HI351_write_cmos_sensor(0x9f, 0x68);
HI351_write_cmos_sensor(0xa0, 0x00);
HI351_write_cmos_sensor(0xa1, 0x08);
HI351_write_cmos_sensor(0xa2, 0x00);
HI351_write_cmos_sensor(0xa3, 0x04);
HI351_write_cmos_sensor(0xa4, 0x00);
HI351_write_cmos_sensor(0xa5, 0x08);
HI351_write_cmos_sensor(0xa6, 0x00);
HI351_write_cmos_sensor(0xa7, 0x04);
HI351_write_cmos_sensor(0xa8, 0x00);
HI351_write_cmos_sensor(0xa9, 0x73);
HI351_write_cmos_sensor(0xaa, 0x00);
HI351_write_cmos_sensor(0xab, 0x64);
HI351_write_cmos_sensor(0xac, 0x00);
HI351_write_cmos_sensor(0xad, 0x73);
HI351_write_cmos_sensor(0xae, 0x00);
HI351_write_cmos_sensor(0xaf, 0x64);
HI351_write_cmos_sensor(0xc0, 0x00);
HI351_write_cmos_sensor(0xc1, 0x1d);
HI351_write_cmos_sensor(0xc2, 0x00);
HI351_write_cmos_sensor(0xc3, 0x2f);
HI351_write_cmos_sensor(0xc4, 0x00);
HI351_write_cmos_sensor(0xc5, 0x1d);
HI351_write_cmos_sensor(0xc6, 0x00);
HI351_write_cmos_sensor(0xc7, 0x2f);
HI351_write_cmos_sensor(0xc8, 0x00);
HI351_write_cmos_sensor(0xc9, 0x1f);
HI351_write_cmos_sensor(0xca, 0x00);
HI351_write_cmos_sensor(0xcb, 0x2d);
HI351_write_cmos_sensor(0xcc, 0x00);
HI351_write_cmos_sensor(0xcd, 0x1f);
HI351_write_cmos_sensor(0xce, 0x00);
HI351_write_cmos_sensor(0xcf, 0x2d);
HI351_write_cmos_sensor(0xd0, 0x00);
HI351_write_cmos_sensor(0xd1, 0x21);
HI351_write_cmos_sensor(0xd2, 0x00);
HI351_write_cmos_sensor(0xd3, 0x2b);
HI351_write_cmos_sensor(0xd4, 0x00);
HI351_write_cmos_sensor(0xd5, 0x21);
HI351_write_cmos_sensor(0xd6, 0x00);
HI351_write_cmos_sensor(0xd7, 0x2b);
HI351_write_cmos_sensor(0xd8, 0x00);
HI351_write_cmos_sensor(0xd9, 0x23);
HI351_write_cmos_sensor(0xdA, 0x00);
HI351_write_cmos_sensor(0xdB, 0x29);
HI351_write_cmos_sensor(0xdC, 0x00);
HI351_write_cmos_sensor(0xdD, 0x23);
HI351_write_cmos_sensor(0xdE, 0x00);
HI351_write_cmos_sensor(0xdF, 0x29);
HI351_write_cmos_sensor(0xe0, 0x00);
HI351_write_cmos_sensor(0xe1, 0x6B);
HI351_write_cmos_sensor(0xe2, 0x00);
HI351_write_cmos_sensor(0xe3, 0xE8);
HI351_write_cmos_sensor(0xe4, 0x00);
HI351_write_cmos_sensor(0xe5, 0xEB);
HI351_write_cmos_sensor(0xe6, 0x01);
HI351_write_cmos_sensor(0xe7, 0x7E);
HI351_write_cmos_sensor(0xe8, 0x00);
HI351_write_cmos_sensor(0xe9, 0x95);
HI351_write_cmos_sensor(0xea, 0x00);
HI351_write_cmos_sensor(0xeb, 0xF1);
HI351_write_cmos_sensor(0xec, 0x00);
HI351_write_cmos_sensor(0xed, 0xdd);
HI351_write_cmos_sensor(0xee, 0x00);
HI351_write_cmos_sensor(0xef, 0x00);

HI351_write_cmos_sensor(0xf0, 0x00);
HI351_write_cmos_sensor(0xf1, 0x34);
HI351_write_cmos_sensor(0xf2, 0x00);

///////////////////////////////////////////
// 10 Page
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x10);
HI351_write_cmos_sensor(0x10, 0x03); //YUV422-YUYV
HI351_write_cmos_sensor(0x12, 0x30); //0x30); //0220 test dyoffset on//0x10); //Y,DY offset Enb
HI351_write_cmos_sensor(0x13, 0x0a); //Bright2, Contrast Enb
HI351_write_cmos_sensor(0x20, 0x80);
HI351_write_cmos_sensor(0x48, 0x80); // contrast
HI351_write_cmos_sensor(0x50, 0x90); //0220 add, AGbgt//
HI351_write_cmos_sensor(0x60, 0x03); //Sat, Trans Enb
HI351_write_cmos_sensor(0x61, 0x80);
HI351_write_cmos_sensor(0x62, 0x80);
//Desat - Chroma
HI351_write_cmos_sensor(0x70, 0x0c);
HI351_write_cmos_sensor(0x71, 0x00);
HI351_write_cmos_sensor(0x72, 0xaf);
HI351_write_cmos_sensor(0x73, 0xcc);
HI351_write_cmos_sensor(0x74, 0x88);
HI351_write_cmos_sensor(0x75, 0x05);
HI351_write_cmos_sensor(0x76, 0x40);
HI351_write_cmos_sensor(0x77, 0x49);
HI351_write_cmos_sensor(0x78, 0x99);
HI351_write_cmos_sensor(0x79, 0x56);
HI351_write_cmos_sensor(0x7a, 0x66);
HI351_write_cmos_sensor(0x7b, 0x46);
HI351_write_cmos_sensor(0x7c, 0x66);
HI351_write_cmos_sensor(0x7d, 0x0e);
HI351_write_cmos_sensor(0x7e, 0x1e);
HI351_write_cmos_sensor(0x7f, 0x3c);

HI351_write_cmos_sensor(0xe0, 0xff); //don't touch
HI351_write_cmos_sensor(0xe1, 0x3f); //don't touch
HI351_write_cmos_sensor(0xe2, 0xff); //don't touch
HI351_write_cmos_sensor(0xe3, 0xff); //don't touch
HI351_write_cmos_sensor(0xe4, 0xf7); //don't touch
HI351_write_cmos_sensor(0xe5, 0x79); //don't touch
HI351_write_cmos_sensor(0xe6, 0xce); //don't touch
HI351_write_cmos_sensor(0xe7, 0x1f); //don't touch
HI351_write_cmos_sensor(0xe8, 0x5f); //don't touch
HI351_write_cmos_sensor(0xf0, 0x3f); //don't touch

///////////////////////////////////////////
// 11 page D-LPF
///////////////////////////////////////////
//DLPF
HI351_write_cmos_sensor(0x03, 0x11);
HI351_write_cmos_sensor(0x10, 0x13); //DLPF
HI351_write_cmos_sensor(0xf0, 0x40); //lpf_auto_ctl1
HI351_write_cmos_sensor(0xf2, 0x68); //LPF_AG_TH_ON
HI351_write_cmos_sensor(0xf3, 0x60); //LPF_AG_TH_OFF
HI351_write_cmos_sensor(0xf4, 0xfe); //lpf_out_th_h
HI351_write_cmos_sensor(0xf5, 0xfd); //lpf_out_th_lo

HI351_write_cmos_sensor(0xf6, 0x00); //lpf_ymean_th_hi
HI351_write_cmos_sensor(0xf7, 0x00); //lpf_ymean_th_lo

// STEVE Luminanace level setting
HI351_write_cmos_sensor(0x32, 0x8b);
HI351_write_cmos_sensor(0x33, 0x54);
HI351_write_cmos_sensor(0x34, 0x2c);
HI351_write_cmos_sensor(0x35, 0x29);
HI351_write_cmos_sensor(0x36, 0x18);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x38, 0x17);
///////////////////////////////////////////
// 12 page DPC / GBGR /LensDebulr
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x12);
HI351_write_cmos_sensor(0x12, 0x08);
HI351_write_cmos_sensor(0x2b, 0x08); //white
HI351_write_cmos_sensor(0x2c, 0x08); //mid_high
HI351_write_cmos_sensor(0x2d, 0x08); //mid_low
HI351_write_cmos_sensor(0x2e, 0x06); //dark

HI351_write_cmos_sensor(0x33, 0x09);
HI351_write_cmos_sensor(0x35, 0x03);
HI351_write_cmos_sensor(0x36, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x38, 0x02);

HI351_write_cmos_sensor(0x60, 0x21);
HI351_write_cmos_sensor(0x61, 0x0e);
HI351_write_cmos_sensor(0x62, 0x70);
HI351_write_cmos_sensor(0x63, 0x70);
HI351_write_cmos_sensor(0x65, 0x01);

HI351_write_cmos_sensor(0xE1, 0x58);
HI351_write_cmos_sensor(0xEC, 0x32);
HI351_write_cmos_sensor(0xEE, 0x03);

///////////////////////////////////////////
// 13 page YC2D LPF
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x13);

HI351_write_cmos_sensor(0x10, 0x33); //Don't touch
HI351_write_cmos_sensor(0xa0, 0x0f); //Don't touch

HI351_write_cmos_sensor(0xe1, 0x07);

///////////////////////////////////////////
// 14 page Sharpness
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x14);

HI351_write_cmos_sensor(0x10, 0x00); //Don't touch   //preview turn off sharpness  capture turn on //off: 00  on: 27
HI351_write_cmos_sensor(0x11, 0x02); //Don't touch
HI351_write_cmos_sensor(0x12, 0x40); //Don't touch
HI351_write_cmos_sensor(0x20, 0x82); //Don't touch
HI351_write_cmos_sensor(0x30, 0x82); //Don't touch
HI351_write_cmos_sensor(0x40, 0x84); //Don't touch
HI351_write_cmos_sensor(0x50, 0x84); //Don't touch

mdelay(10);

///////////////////////////////////////////
// 15 Page LSC off
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x15);
HI351_write_cmos_sensor(0x10, 0x82);


///////////////////////////////////////////
// 7 Page LSC data (STEVE 75p)
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x07);
HI351_write_cmos_sensor(0x12, 0x04);
HI351_write_cmos_sensor(0x34, 0x00);
HI351_write_cmos_sensor(0x35, 0x00);
HI351_write_cmos_sensor(0x13, 0x85);
HI351_write_cmos_sensor(0x13, 0x05);

//================ LSC set start
HI351_write_cmos_sensor(0x37, 0x34);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x38);
HI351_write_cmos_sensor(0x37, 0x3d);
HI351_write_cmos_sensor(0x37, 0x44);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x39);
HI351_write_cmos_sensor(0x37, 0x3d);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x34);
HI351_write_cmos_sensor(0x37, 0x37);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x36);
HI351_write_cmos_sensor(0x37, 0x39);
HI351_write_cmos_sensor(0x37, 0x3e);
HI351_write_cmos_sensor(0x37, 0x39);
HI351_write_cmos_sensor(0x37, 0x37);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x33);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x33);
HI351_write_cmos_sensor(0x37, 0x37);
HI351_write_cmos_sensor(0x37, 0x3c);
HI351_write_cmos_sensor(0x37, 0x3f);
HI351_write_cmos_sensor(0x37, 0x42);
HI351_write_cmos_sensor(0x37, 0x48);
HI351_write_cmos_sensor(0x37, 0x52);
HI351_write_cmos_sensor(0x37, 0x3a);
HI351_write_cmos_sensor(0x37, 0x37);
HI351_write_cmos_sensor(0x37, 0x34);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x36);
HI351_write_cmos_sensor(0x37, 0x3b);
HI351_write_cmos_sensor(0x37, 0x40);
HI351_write_cmos_sensor(0x37, 0x44);
HI351_write_cmos_sensor(0x37, 0x49);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x33);
HI351_write_cmos_sensor(0x37, 0x39);
HI351_write_cmos_sensor(0x37, 0x3e);
HI351_write_cmos_sensor(0x37, 0x41);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x33);
HI351_write_cmos_sensor(0x37, 0x39);
HI351_write_cmos_sensor(0x37, 0x3c);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x38);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x34);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x33);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x33);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x38);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x38);
HI351_write_cmos_sensor(0x37, 0x3b);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x38);
HI351_write_cmos_sensor(0x37, 0x3d);
HI351_write_cmos_sensor(0x37, 0x40);
HI351_write_cmos_sensor(0x37, 0x39);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x37);
HI351_write_cmos_sensor(0x37, 0x3c);
HI351_write_cmos_sensor(0x37, 0x41);
HI351_write_cmos_sensor(0x37, 0x46);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x3d);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x36);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x04);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x34);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x38);
HI351_write_cmos_sensor(0x37, 0x3d);
HI351_write_cmos_sensor(0x37, 0x44);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x28);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x39);
HI351_write_cmos_sensor(0x37, 0x3d);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1c);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x30);
HI351_write_cmos_sensor(0x37, 0x34);
HI351_write_cmos_sensor(0x37, 0x37);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1e);
HI351_write_cmos_sensor(0x37, 0x25);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x31);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x22);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2c);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x16);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x37, 0x01);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x0b);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x21);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x02);
HI351_write_cmos_sensor(0x37, 0x03);
HI351_write_cmos_sensor(0x37, 0x05);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x10);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x06);
HI351_write_cmos_sensor(0x37, 0x07);
HI351_write_cmos_sensor(0x37, 0x0a);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x12);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x2b);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x26);
HI351_write_cmos_sensor(0x37, 0x23);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x11);
HI351_write_cmos_sensor(0x37, 0x0e);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0c);
HI351_write_cmos_sensor(0x37, 0x0d);
HI351_write_cmos_sensor(0x37, 0x0f);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x2f);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x18);
HI351_write_cmos_sensor(0x37, 0x15);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x13);
HI351_write_cmos_sensor(0x37, 0x14);
HI351_write_cmos_sensor(0x37, 0x17);
HI351_write_cmos_sensor(0x37, 0x1a);
HI351_write_cmos_sensor(0x37, 0x1f);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x35);
HI351_write_cmos_sensor(0x37, 0x2d);
HI351_write_cmos_sensor(0x37, 0x2a);
HI351_write_cmos_sensor(0x37, 0x27);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x19);
HI351_write_cmos_sensor(0x37, 0x1b);
HI351_write_cmos_sensor(0x37, 0x1d);
HI351_write_cmos_sensor(0x37, 0x20);
HI351_write_cmos_sensor(0x37, 0x24);
HI351_write_cmos_sensor(0x37, 0x29);
HI351_write_cmos_sensor(0x37, 0x2e);
HI351_write_cmos_sensor(0x37, 0x32);
HI351_write_cmos_sensor(0x37, 0x36);
HI351_write_cmos_sensor(0x37, 0x39);

//0224
//END

//================ LSC set end

HI351_write_cmos_sensor(0x12, 0x00);
HI351_write_cmos_sensor(0x13, 0x00);

HI351_write_cmos_sensor(0x03, 0x15);
HI351_write_cmos_sensor(0x10, 0x83); // LSC ON

///////////////////////////////////////////
// 16 Page CMC
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x16);

HI351_write_cmos_sensor(0x10, 0x0f); //cmc
HI351_write_cmos_sensor(0x17, 0x2f); //CMC SIGN
HI351_write_cmos_sensor(0x60, 0x7d); //on: 7f off:7d//decolor function off

//automatic saturation
HI351_write_cmos_sensor(0x8a, 0x7f);
HI351_write_cmos_sensor(0x8b, 0x7f);
HI351_write_cmos_sensor(0x8c, 0x7f);
HI351_write_cmos_sensor(0x8d, 0x7f);
HI351_write_cmos_sensor(0x8e, 0x7f);
HI351_write_cmos_sensor(0x8f, 0x7f);
HI351_write_cmos_sensor(0x90, 0x7f);
HI351_write_cmos_sensor(0x91, 0x7f);
HI351_write_cmos_sensor(0x92, 0x7f);
HI351_write_cmos_sensor(0x93, 0x7f);
HI351_write_cmos_sensor(0x94, 0x7f);
HI351_write_cmos_sensor(0x95, 0x7f);
HI351_write_cmos_sensor(0x96, 0x7f);
HI351_write_cmos_sensor(0x97, 0x7f);
HI351_write_cmos_sensor(0x98, 0x7b);
HI351_write_cmos_sensor(0x99, 0x74);
HI351_write_cmos_sensor(0x9a, 0x6e);

//Dgain
HI351_write_cmos_sensor(0xa0, 0x81); //Manual WB gain enable
HI351_write_cmos_sensor(0xa1, 0x00);

HI351_write_cmos_sensor(0xa2, 0x6b); //R_dgain_byr
HI351_write_cmos_sensor(0xa3, 0x76); //B_dgain_byr

HI351_write_cmos_sensor(0xa6, 0xa0); //r max
HI351_write_cmos_sensor(0xa8, 0xa0); //b max
// Pre WB gain setting(after AWB setting)
HI351_write_cmos_sensor(0xF0, 0x01);//Pre WB gain enable Gain resolution_1x
HI351_write_cmos_sensor(0xF1, 0x40);//
HI351_write_cmos_sensor(0xF2, 0x40);//
HI351_write_cmos_sensor(0xF3, 0x40);//
HI351_write_cmos_sensor(0xF4, 0x40);//
///////////////////////////////////////////
// 17 Page Gamma
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x17);
HI351_write_cmos_sensor(0x10, 0x01);

///////////////////////////////////////////
// 18 Page Histogram
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x18);
HI351_write_cmos_sensor(0x10, 0x00);
HI351_write_cmos_sensor(0xc0, 0x01);
HI351_write_cmos_sensor(0xC4, 0x7a); //FLK200 = EXP100/2/Oneline
HI351_write_cmos_sensor(0xC5, 0x66); //FLK240 = EXP120/2/Oneline

///////////////////////////////////////////
// 20 Page AE
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x20);
HI351_write_cmos_sensor(0x10, 0x9f); //M 50hz//0xdf);//auto 50hz flicker select
HI351_write_cmos_sensor(0x12, 0x2d); //Dgain off
HI351_write_cmos_sensor(0x17, 0xa0);
HI351_write_cmos_sensor(0x1f, 0x1f);

HI351_write_cmos_sensor(0x03, 0x20); //Page 20
HI351_write_cmos_sensor(0x20, 0x00); //EXP Normal 33.33 fps
HI351_write_cmos_sensor(0x21, 0x18);
HI351_write_cmos_sensor(0x22, 0xac);
HI351_write_cmos_sensor(0x23, 0x68);
HI351_write_cmos_sensor(0x24, 0x00); //EXP Max 12.50 fps
HI351_write_cmos_sensor(0x25, 0x41);
HI351_write_cmos_sensor(0x26, 0xcb);
HI351_write_cmos_sensor(0x27, 0xc0);
HI351_write_cmos_sensor(0x28, 0x00); //EXPMin 24545.45 fps
HI351_write_cmos_sensor(0x29, 0x08);
HI351_write_cmos_sensor(0x2a, 0x98);
HI351_write_cmos_sensor(0x30, 0x08); //EXP100
HI351_write_cmos_sensor(0x31, 0x39);
HI351_write_cmos_sensor(0x32, 0x78);
HI351_write_cmos_sensor(0x33, 0x06); //EXP120
HI351_write_cmos_sensor(0x34, 0xd9);
HI351_write_cmos_sensor(0x35, 0x20);
HI351_write_cmos_sensor(0x36, 0x00); //EXP Unit
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x38, 0x98);
HI351_write_cmos_sensor(0x40, 0x00); //exp 12000
HI351_write_cmos_sensor(0x41, 0x08);
HI351_write_cmos_sensor(0x42, 0x39);

HI351_write_cmos_sensor(0x43, 0x04); //pga time th
HI351_write_cmos_sensor(0x51, 0xa0); //pga_max_total
HI351_write_cmos_sensor(0x52, 0x28); //pga_min_total

HI351_write_cmos_sensor(0x71, 0xd0); //DG MAX
HI351_write_cmos_sensor(0x72, 0x80); //DG MIN

HI351_write_cmos_sensor(0x80, 0x34); //ae target

///////////////////////////////////////////
// 30 page MCU Set
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x30); //system setting important
HI351_write_cmos_sensor(0x12, 0x00); //mcu iic setting
HI351_write_cmos_sensor(0x20, 0x08); //need to set before mcu page
HI351_write_cmos_sensor(0x50, 0x00);
HI351_write_cmos_sensor(0xe0, 0x02); //Don't touch
HI351_write_cmos_sensor(0xf0, 0x00);
HI351_write_cmos_sensor(0x11, 0x05); //B[0]: MCU holding
HI351_write_cmos_sensor(0x03, 0xc0);
HI351_write_cmos_sensor(0xe4, 0xA0); //delay

///////////////////////////////////////////
// 30 Page DMA address set
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x30); //DMA
HI351_write_cmos_sensor(0x7c, 0x2c); //Extra str
HI351_write_cmos_sensor(0x7d, 0xce);
HI351_write_cmos_sensor(0x7e, 0x2c); //Extra end
HI351_write_cmos_sensor(0x7f, 0xd1);
HI351_write_cmos_sensor(0x80, 0x24); //Outdoor str
HI351_write_cmos_sensor(0x81, 0x70);
HI351_write_cmos_sensor(0x82, 0x24); //Outdoor end
HI351_write_cmos_sensor(0x83, 0x73);
HI351_write_cmos_sensor(0x84, 0x21); //Indoor str
HI351_write_cmos_sensor(0x85, 0xa6);
HI351_write_cmos_sensor(0x86, 0x21); //Indoor end
HI351_write_cmos_sensor(0x87, 0xa9);
HI351_write_cmos_sensor(0x88, 0x27); //Dark1 str
HI351_write_cmos_sensor(0x89, 0x3a);
HI351_write_cmos_sensor(0x8a, 0x27); //Dark1 end
HI351_write_cmos_sensor(0x8b, 0x3d);
HI351_write_cmos_sensor(0x8c, 0x2a); //Dark2 str
HI351_write_cmos_sensor(0x8d, 0x04);
HI351_write_cmos_sensor(0x8e, 0x2a); //Dark2 end
HI351_write_cmos_sensor(0x8f, 0x07);

HI351_write_cmos_sensor(0x03, 0xC0);
HI351_write_cmos_sensor(0x2F, 0xf0); //DMA busy flag check
HI351_write_cmos_sensor(0x31, 0x20); //Delay before DMA write_setting
HI351_write_cmos_sensor(0x33, 0x20); //DMA full stuck mode
HI351_write_cmos_sensor(0x32, 0x01); //DMA on first

HI351_write_cmos_sensor(0x03, 0xC0);
HI351_write_cmos_sensor(0x2F, 0xf0); //DMA busy flag check
HI351_write_cmos_sensor(0x31, 0x20); //Delay before DMA write_setting
HI351_write_cmos_sensor(0x33, 0x20);
HI351_write_cmos_sensor(0x32, 0x01); //DMA on second

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x10, 0x13); // Preview2
HI351_write_cmos_sensor(0x01, 0xF0); // sleep off

///////////////////////////////////////////
// 30 page
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0xDE, 0x20); //OTP color ratio xdata address set
HI351_write_cmos_sensor(0xDF, 0xA5);
HI351_write_cmos_sensor(0x03, 0xE7);
HI351_write_cmos_sensor(0x1F, 0x18); //OTP color ratio Rg typical set = 6300
HI351_write_cmos_sensor(0x20, 0x9c);
HI351_write_cmos_sensor(0x21, 0x0F); //OTP color ratio Bg typical set = 3920
HI351_write_cmos_sensor(0x22, 0x50);

/////////////////////////////////////////////
// CD Page Adaptive Mode(Color ratio)
/////////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xCD);//
HI351_write_cmos_sensor(0x47, 0x00);//
HI351_write_cmos_sensor(0x12, 0x40);//
HI351_write_cmos_sensor(0x13, 0x40);//Ratio WB R gain min
HI351_write_cmos_sensor(0x14, 0x48);//Ratio WB R gain max
HI351_write_cmos_sensor(0x15, 0x40);//Ratio WB B gain min
HI351_write_cmos_sensor(0x16, 0x48);//Ratio WB B gain max
HI351_write_cmos_sensor(0x10, 0x38); // STEVE b9 -) 38 Disable

///////////////////////////////////////////
// 1F Page SSD
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x1f); //1F page
HI351_write_cmos_sensor(0x11, 0x00); //bit[5:4]: debug mode
HI351_write_cmos_sensor(0x12, 0x60);
HI351_write_cmos_sensor(0x13, 0x14);
HI351_write_cmos_sensor(0x14, 0x10);
HI351_write_cmos_sensor(0x15, 0x00);
HI351_write_cmos_sensor(0x20, 0x18); //ssd_x_start_pos
HI351_write_cmos_sensor(0x21, 0x14); //ssd_y_start_pos
HI351_write_cmos_sensor(0x22, 0x8C); //ssd_blk_width
HI351_write_cmos_sensor(0x23, 0x60); //ssd_blk_height  //ae & awb input data block  //full size use 9c //pre1 use 60
HI351_write_cmos_sensor(0x28, 0x18);
HI351_write_cmos_sensor(0x29, 0x02);
HI351_write_cmos_sensor(0x3B, 0x18);
HI351_write_cmos_sensor(0x3C, 0x8C);
HI351_write_cmos_sensor(0x10, 0x19); //SSD enable

///////////////////////////////////////////
// C4 Page MCU AE
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xc4);
HI351_write_cmos_sensor(0x11, 0x30); // ae speed B[7:6] 0 (SLOW) ~ 3 (FAST), 0x70 - 0x30
HI351_write_cmos_sensor(0x12, 0x10);
HI351_write_cmos_sensor(0x19, 0x4a); //0x30); //0x50); //17th//0x30); // band0 gain 40fps 0x2d
HI351_write_cmos_sensor(0x1a, 0x54); //0x38); //0x58); //17th//0x38); // band1 gain 20fps STEVE
HI351_write_cmos_sensor(0x1b, 0x6c); //0x4c); //0x60); //0x6c); //17th//0x4c); // band2 gain 12fps

HI351_write_cmos_sensor(0x1c, 0x04);
HI351_write_cmos_sensor(0x1d, 0x80);

HI351_write_cmos_sensor(0x1e, 0x00); // band1 min exposure time	1/33.33s // correction point
HI351_write_cmos_sensor(0x1f, 0x18);
HI351_write_cmos_sensor(0x20, 0xac);
HI351_write_cmos_sensor(0x21, 0x68);

HI351_write_cmos_sensor(0x22, 0x00); // band2 min exposure time	1/20s
HI351_write_cmos_sensor(0x23, 0x29);
HI351_write_cmos_sensor(0x24, 0x1f);
HI351_write_cmos_sensor(0x25, 0x58);

HI351_write_cmos_sensor(0x26, 0x00);// band3 min exposure time  1/12.5s
HI351_write_cmos_sensor(0x27, 0x41);
HI351_write_cmos_sensor(0x28, 0xcb);
HI351_write_cmos_sensor(0x29, 0xc0);

HI351_write_cmos_sensor(0x36, 0x22); // AE Yth

HI351_write_cmos_sensor(0x66, 0x00); //jktest 0131//manual 50hz
HI351_write_cmos_sensor(0x69, 0x00); //jktest 0131//manual 50hz //auto off

//HI351_write_cmos_sensor(0x03, 0x20);
//HI351_write_cmos_sensor(0x12, 0x2d); // STEVE 2d (AE digital gain OFF)

///////////////////////////////////////////
// c3 Page MCU AE Weight
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xc3);
HI351_write_cmos_sensor(0x10, 0x00);
HI351_write_cmos_sensor(0x38, 0xFF);
HI351_write_cmos_sensor(0x39, 0xFF);
//AE_CenterWeighted
HI351_write_cmos_sensor(0x70, 0x00);
HI351_write_cmos_sensor(0x71, 0x00);
HI351_write_cmos_sensor(0x72, 0x00);
HI351_write_cmos_sensor(0x73, 0x00);
HI351_write_cmos_sensor(0x74, 0x00);
HI351_write_cmos_sensor(0x75, 0x00);
HI351_write_cmos_sensor(0x76, 0x00);
HI351_write_cmos_sensor(0x77, 0x00);
HI351_write_cmos_sensor(0x78, 0x00);
HI351_write_cmos_sensor(0x79, 0x00);
HI351_write_cmos_sensor(0x7A, 0x00);
HI351_write_cmos_sensor(0x7B, 0x00);
HI351_write_cmos_sensor(0x7C, 0x11);
HI351_write_cmos_sensor(0x7D, 0x11);
HI351_write_cmos_sensor(0x7E, 0x11);
HI351_write_cmos_sensor(0x7F, 0x11);
HI351_write_cmos_sensor(0x80, 0x11);
HI351_write_cmos_sensor(0x81, 0x11);
HI351_write_cmos_sensor(0x82, 0x11);
HI351_write_cmos_sensor(0x83, 0x21);
HI351_write_cmos_sensor(0x84, 0x44);
HI351_write_cmos_sensor(0x85, 0x44);
HI351_write_cmos_sensor(0x86, 0x12);
HI351_write_cmos_sensor(0x87, 0x11);
HI351_write_cmos_sensor(0x88, 0x11);
HI351_write_cmos_sensor(0x89, 0x22);
HI351_write_cmos_sensor(0x8A, 0x64);
HI351_write_cmos_sensor(0x8B, 0x46);
HI351_write_cmos_sensor(0x8C, 0x22);
HI351_write_cmos_sensor(0x8D, 0x11);
HI351_write_cmos_sensor(0x8E, 0x21);
HI351_write_cmos_sensor(0x8F, 0x33);
HI351_write_cmos_sensor(0x90, 0x64);
HI351_write_cmos_sensor(0x91, 0x46);
HI351_write_cmos_sensor(0x92, 0x33);
HI351_write_cmos_sensor(0x93, 0x12);
HI351_write_cmos_sensor(0x94, 0x21);
HI351_write_cmos_sensor(0x95, 0x33);
HI351_write_cmos_sensor(0x96, 0x44);
HI351_write_cmos_sensor(0x97, 0x44);
HI351_write_cmos_sensor(0x98, 0x33);
HI351_write_cmos_sensor(0x99, 0x12);
HI351_write_cmos_sensor(0x9A, 0x21);
HI351_write_cmos_sensor(0x9B, 0x33);
HI351_write_cmos_sensor(0x9C, 0x33);
HI351_write_cmos_sensor(0x9D, 0x33);
HI351_write_cmos_sensor(0x9E, 0x33);
HI351_write_cmos_sensor(0x9F, 0x12);
HI351_write_cmos_sensor(0xA0, 0x11);
HI351_write_cmos_sensor(0xA1, 0x11);
HI351_write_cmos_sensor(0xA2, 0x11);
HI351_write_cmos_sensor(0xA3, 0x11);
HI351_write_cmos_sensor(0xA4, 0x11);
HI351_write_cmos_sensor(0xA5, 0x11);
HI351_write_cmos_sensor(0xE1, 0x29); //Outdoor AG Max
HI351_write_cmos_sensor(0xE2, 0x03);

///////////////////////////////////////////
// Capture Setting
///////////////////////////////////////////

HI351_write_cmos_sensor(0x03, 0xd5);
HI351_write_cmos_sensor(0x11, 0xb9); //manual sleep onoff
HI351_write_cmos_sensor(0x14, 0xfd); // STEVE EXPMIN x2
HI351_write_cmos_sensor(0x1e, 0x02); //capture clock set
HI351_write_cmos_sensor(0x86, 0x04); //preview clock set

HI351_write_cmos_sensor(0x1f, 0x00); //Capture Hblank  20
HI351_write_cmos_sensor(0x20, 0x14);
HI351_write_cmos_sensor(0x21, 0x08); //Capture oneline 2200  //hblank+hline  20+2180
HI351_write_cmos_sensor(0x22, 0x98);

HI351_write_cmos_sensor(0x8c, 0x00); //Preview Hblank  20
HI351_write_cmos_sensor(0x8d, 0x14);
HI351_write_cmos_sensor(0x92, 0x08); //Preview oneline 2200	//hblank+hline  20+2180
HI351_write_cmos_sensor(0x93, 0x98);
HI351_write_cmos_sensor(0x33, 0x00);

///////////////////////////////////////////
// C0 Page Firmware system
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xc0);
HI351_write_cmos_sensor(0x16, 0x81); //MCU main roof holding on

///////////////////////////////////////////
// DAWB control : Page Mode = 0xC5 ~ 0xC9
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xc5);
HI351_write_cmos_sensor(0x0e, 0x01); //burst start
HI351_write_cmos_sensor(0x10, 0x30);
HI351_write_cmos_sensor(0x11, 0xa1);
HI351_write_cmos_sensor(0x12, 0x17); //Near BP off
HI351_write_cmos_sensor(0x13, 0x0d);// high temp. option off
HI351_write_cmos_sensor(0x14, 0x13);
HI351_write_cmos_sensor(0x15, 0x04);
HI351_write_cmos_sensor(0x16, 0x0a);
HI351_write_cmos_sensor(0x17, 0x08);

HI351_write_cmos_sensor(0x18, 0x0a);
HI351_write_cmos_sensor(0x19, 0x03);
HI351_write_cmos_sensor(0x1a, 0xa0);//awb max ylvl
HI351_write_cmos_sensor(0x1b, 0x18);//awb min ylvl
HI351_write_cmos_sensor(0x1c, 0x0a);//awb frame skip when min max
HI351_write_cmos_sensor(0x1d, 0x40);
HI351_write_cmos_sensor(0x1e, 0x00);
HI351_write_cmos_sensor(0x1f, 0xF2);//sky limit

HI351_write_cmos_sensor(0x20, 0x00);
HI351_write_cmos_sensor(0x21, 0xa0);
HI351_write_cmos_sensor(0x22, 0x00);
HI351_write_cmos_sensor(0x23, 0xF2);//sky limit
HI351_write_cmos_sensor(0x24, 0x00);
HI351_write_cmos_sensor(0x25, 0xa0);
HI351_write_cmos_sensor(0x26, 0x01);
HI351_write_cmos_sensor(0x27, 0x09);

HI351_write_cmos_sensor(0x28, 0x00);
HI351_write_cmos_sensor(0x29, 0x73); //jktest 0131//0x64);
HI351_write_cmos_sensor(0x2a, 0x00);
HI351_write_cmos_sensor(0x2b, 0xe6);
HI351_write_cmos_sensor(0x2c, 0x00);
HI351_write_cmos_sensor(0x2d, 0x82); // dark angle max
HI351_write_cmos_sensor(0x2e, 0x00);
HI351_write_cmos_sensor(0x2f, 0x01);
HI351_write_cmos_sensor(0x30, 0xc9);
HI351_write_cmos_sensor(0x31, 0x08);
HI351_write_cmos_sensor(0x32, 0x00);
HI351_write_cmos_sensor(0x33, 0x02);
HI351_write_cmos_sensor(0x34, 0x08);
HI351_write_cmos_sensor(0x35, 0x82);
HI351_write_cmos_sensor(0x36, 0x00);
HI351_write_cmos_sensor(0x37, 0x09);
HI351_write_cmos_sensor(0x38, 0x1f);
HI351_write_cmos_sensor(0x39, 0xf0);
HI351_write_cmos_sensor(0x3a, 0x00);
HI351_write_cmos_sensor(0x3b, 0x12);
HI351_write_cmos_sensor(0x3c, 0x3f);
HI351_write_cmos_sensor(0x3d, 0xe0);
HI351_write_cmos_sensor(0x3e, 0x01);
HI351_write_cmos_sensor(0x3f, 0xb5);
HI351_write_cmos_sensor(0x40, 0xfd);
HI351_write_cmos_sensor(0x41, 0x00);
HI351_write_cmos_sensor(0x42, 0x02);
HI351_write_cmos_sensor(0x43, 0x0d);
HI351_write_cmos_sensor(0x44, 0x96);
HI351_write_cmos_sensor(0x45, 0x00);
HI351_write_cmos_sensor(0x46, 0x00);
HI351_write_cmos_sensor(0x47, 0x09);
HI351_write_cmos_sensor(0x48, 0x1f);
HI351_write_cmos_sensor(0x49, 0xf0);
HI351_write_cmos_sensor(0x4a, 0x00);
HI351_write_cmos_sensor(0x4b, 0x12);
HI351_write_cmos_sensor(0x4c, 0x3f);
HI351_write_cmos_sensor(0x4d, 0xe0);
HI351_write_cmos_sensor(0x4e, 0x08); //wb slop offset r
HI351_write_cmos_sensor(0x4f, 0x02); //wb slop offset b

HI351_write_cmos_sensor(0x50, 0x55);
HI351_write_cmos_sensor(0x51, 0x55);
HI351_write_cmos_sensor(0x52, 0x55);
HI351_write_cmos_sensor(0x53, 0x55);
HI351_write_cmos_sensor(0x54, 0x55);
HI351_write_cmos_sensor(0x55, 0x55);
HI351_write_cmos_sensor(0x56, 0x55);
HI351_write_cmos_sensor(0x57, 0x55);

HI351_write_cmos_sensor(0x58, 0x55);
HI351_write_cmos_sensor(0x59, 0x55);
HI351_write_cmos_sensor(0x5a, 0x55);
HI351_write_cmos_sensor(0x5b, 0x55);
HI351_write_cmos_sensor(0x5c, 0x55);
HI351_write_cmos_sensor(0x5d, 0x55);
HI351_write_cmos_sensor(0x5e, 0x55);
HI351_write_cmos_sensor(0x5f, 0x55);

HI351_write_cmos_sensor(0x60, 0x55);
HI351_write_cmos_sensor(0x61, 0x55);
HI351_write_cmos_sensor(0x62, 0x55);
HI351_write_cmos_sensor(0x63, 0x55);
HI351_write_cmos_sensor(0x64, 0x55);
HI351_write_cmos_sensor(0x65, 0x55);
HI351_write_cmos_sensor(0x66, 0x55);
HI351_write_cmos_sensor(0x67, 0x55);

HI351_write_cmos_sensor(0x68, 0x55);
HI351_write_cmos_sensor(0x69, 0x55);
HI351_write_cmos_sensor(0x6a, 0x55);
HI351_write_cmos_sensor(0x6b, 0x1f);
HI351_write_cmos_sensor(0x6c, 0x26);
HI351_write_cmos_sensor(0x6d, 0x2c);
HI351_write_cmos_sensor(0x6e, 0x33);
HI351_write_cmos_sensor(0x6f, 0x3c);

HI351_write_cmos_sensor(0x70, 0x49);
HI351_write_cmos_sensor(0x71, 0x53);
HI351_write_cmos_sensor(0x72, 0x5c);
HI351_write_cmos_sensor(0x73, 0x67);
HI351_write_cmos_sensor(0x74, 0x72);
HI351_write_cmos_sensor(0x75, 0x7d);
HI351_write_cmos_sensor(0x76, 0x86);
HI351_write_cmos_sensor(0x77, 0x72);

HI351_write_cmos_sensor(0x78, 0x62);
HI351_write_cmos_sensor(0x79, 0x54);
HI351_write_cmos_sensor(0x7a, 0x46);
HI351_write_cmos_sensor(0x7b, 0x3e);
HI351_write_cmos_sensor(0x7c, 0x3b);
HI351_write_cmos_sensor(0x7d, 0x38);
HI351_write_cmos_sensor(0x7e, 0x36);
HI351_write_cmos_sensor(0x7f, 0x35);

HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x81, 0x99);
HI351_write_cmos_sensor(0x82, 0x90);
HI351_write_cmos_sensor(0x83, 0x86);
HI351_write_cmos_sensor(0x84, 0x7a);
HI351_write_cmos_sensor(0x85, 0x6c);
HI351_write_cmos_sensor(0x86, 0x5e);
HI351_write_cmos_sensor(0x87, 0x52);

HI351_write_cmos_sensor(0x88, 0x4c);
HI351_write_cmos_sensor(0x89, 0x48);
HI351_write_cmos_sensor(0x8a, 0x46);
HI351_write_cmos_sensor(0x8b, 0x45);
HI351_write_cmos_sensor(0x8c, 0x1f);
HI351_write_cmos_sensor(0x8d, 0x26);
HI351_write_cmos_sensor(0x8e, 0x2b);
HI351_write_cmos_sensor(0x8f, 0x30);

HI351_write_cmos_sensor(0x90, 0x36);
HI351_write_cmos_sensor(0x91, 0x3c);
HI351_write_cmos_sensor(0x92, 0x47);
HI351_write_cmos_sensor(0x93, 0x52);
HI351_write_cmos_sensor(0x94, 0x5f);
HI351_write_cmos_sensor(0x95, 0x6d);
HI351_write_cmos_sensor(0x96, 0x7d);
HI351_write_cmos_sensor(0x97, 0x8f);

HI351_write_cmos_sensor(0x98, 0x81);
HI351_write_cmos_sensor(0x99, 0x78);
HI351_write_cmos_sensor(0x9a, 0x6d);
HI351_write_cmos_sensor(0x9b, 0x61);
HI351_write_cmos_sensor(0x9c, 0x55);
HI351_write_cmos_sensor(0x9d, 0x4b);
HI351_write_cmos_sensor(0x9e, 0x44);
HI351_write_cmos_sensor(0x9f, 0x41);

HI351_write_cmos_sensor(0xa0, 0x3e);
HI351_write_cmos_sensor(0xa1, 0x3c);
HI351_write_cmos_sensor(0xa2, 0x28);
HI351_write_cmos_sensor(0xa3, 0x2d);
HI351_write_cmos_sensor(0xa4, 0x37);
HI351_write_cmos_sensor(0xa5, 0x43);
HI351_write_cmos_sensor(0xa6, 0x48);
HI351_write_cmos_sensor(0xa7, 0x55);

HI351_write_cmos_sensor(0xa8, 0x69);//205degree b gain 8 code up
HI351_write_cmos_sensor(0xa9, 0x82);
HI351_write_cmos_sensor(0xaa, 0x00);
HI351_write_cmos_sensor(0xab, 0x00);
HI351_write_cmos_sensor(0xac, 0x90);
HI351_write_cmos_sensor(0xad, 0x90);
HI351_write_cmos_sensor(0xae, 0x00);//awb indoor taget r
HI351_write_cmos_sensor(0xaf, 0x00);//awb indoor taget r

HI351_write_cmos_sensor(0xb0, 0x00);
HI351_write_cmos_sensor(0xb1, 0x00);
HI351_write_cmos_sensor(0xb2, 0x00);
HI351_write_cmos_sensor(0xb3, 0x00);
HI351_write_cmos_sensor(0xb4, 0x00);
HI351_write_cmos_sensor(0xb5, 0x00);
HI351_write_cmos_sensor(0xb6, 0x00);//awb indoor taget b
HI351_write_cmos_sensor(0xb7, 0x00);//awb indoor taget b

HI351_write_cmos_sensor(0xb8, 0x00); //205degree [ indoor ]b gain 6 code up
HI351_write_cmos_sensor(0xb9, 0x00);
HI351_write_cmos_sensor(0xba, 0x00);
HI351_write_cmos_sensor(0xbb, 0x00);
HI351_write_cmos_sensor(0xbc, 0x00);
HI351_write_cmos_sensor(0xbd, 0x00);
HI351_write_cmos_sensor(0xbe, 0x00);
HI351_write_cmos_sensor(0xbf, 0x00);

HI351_write_cmos_sensor(0xc0, 0x00);
HI351_write_cmos_sensor(0xc1, 0x00);
HI351_write_cmos_sensor(0xc2, 0x00);
HI351_write_cmos_sensor(0xc3, 0x00);
HI351_write_cmos_sensor(0xc4, 0x00);
HI351_write_cmos_sensor(0xc5, 0x00);
HI351_write_cmos_sensor(0xc6, 0x00);
HI351_write_cmos_sensor(0xc7, 0x00);

HI351_write_cmos_sensor(0xc8, 0x00);
HI351_write_cmos_sensor(0xc9, 0x00);
HI351_write_cmos_sensor(0xca, 0x00);
HI351_write_cmos_sensor(0xcb, 0x00);
HI351_write_cmos_sensor(0xcc, 0x00);
HI351_write_cmos_sensor(0xcd, 0x00);
HI351_write_cmos_sensor(0xce, 0x00);
HI351_write_cmos_sensor(0xcf, 0x00);

HI351_write_cmos_sensor(0xd0, 0x00);
HI351_write_cmos_sensor(0xd1, 0x00);
HI351_write_cmos_sensor(0xd2, 0x0a);
HI351_write_cmos_sensor(0xd3, 0x0e);
HI351_write_cmos_sensor(0xd4, 0x14);
HI351_write_cmos_sensor(0xd5, 0x1e);
HI351_write_cmos_sensor(0xd6, 0x28);
HI351_write_cmos_sensor(0xd7, 0x22);

HI351_write_cmos_sensor(0xd8, 0x1e);
HI351_write_cmos_sensor(0xd9, 0x1b);
HI351_write_cmos_sensor(0xda, 0x18);
HI351_write_cmos_sensor(0xdb, 0x14);
HI351_write_cmos_sensor(0xdc, 0x10);
HI351_write_cmos_sensor(0xdd, 0x0d);
HI351_write_cmos_sensor(0xde, 0x0a);
HI351_write_cmos_sensor(0xdf, 0x0a);

HI351_write_cmos_sensor(0xe0, 0x0a);
HI351_write_cmos_sensor(0xe1, 0x0a);
HI351_write_cmos_sensor(0xe2, 0x28);
HI351_write_cmos_sensor(0xe3, 0x28);
HI351_write_cmos_sensor(0xe4, 0x28);
HI351_write_cmos_sensor(0xe5, 0x28);
HI351_write_cmos_sensor(0xe6, 0x28);
HI351_write_cmos_sensor(0xe7, 0x24);

HI351_write_cmos_sensor(0xe8, 0x20);
HI351_write_cmos_sensor(0xe9, 0x1c);
HI351_write_cmos_sensor(0xea, 0x18);
HI351_write_cmos_sensor(0xeb, 0x14);
HI351_write_cmos_sensor(0xec, 0x14);
HI351_write_cmos_sensor(0xed, 0x0a);
HI351_write_cmos_sensor(0xee, 0x0a);
HI351_write_cmos_sensor(0xef, 0x0a);

HI351_write_cmos_sensor(0xf0, 0x0a);
HI351_write_cmos_sensor(0xf1, 0x09);
HI351_write_cmos_sensor(0xf2, 0x08);
HI351_write_cmos_sensor(0xf3, 0x07);
HI351_write_cmos_sensor(0xf4, 0x07);
HI351_write_cmos_sensor(0xf5, 0x06);
HI351_write_cmos_sensor(0xf6, 0x06);
HI351_write_cmos_sensor(0xf7, 0x05);

HI351_write_cmos_sensor(0xf8, 0x64);
HI351_write_cmos_sensor(0xf9, 0x05);
HI351_write_cmos_sensor(0xfa, 0x32);
HI351_write_cmos_sensor(0xfb, 0x28);
HI351_write_cmos_sensor(0xfc, 0x64);
HI351_write_cmos_sensor(0xfd, 0x14);//indoor angle offset 26->14
HI351_write_cmos_sensor(0x0e, 0x00); //burst end
HI351_write_cmos_sensor(0x03, 0xc6);
HI351_write_cmos_sensor(0x0e, 0x01); //burst start
HI351_write_cmos_sensor(0x10, 0x1e);
HI351_write_cmos_sensor(0x11, 0x50);
HI351_write_cmos_sensor(0x12, 0x1e);// spec temp 1e -> 32 50%
HI351_write_cmos_sensor(0x13, 0x14);
HI351_write_cmos_sensor(0x14, 0x64); //96 ->64
HI351_write_cmos_sensor(0x15, 0x1e);
HI351_write_cmos_sensor(0x16, 0x04);
HI351_write_cmos_sensor(0x17, 0xf8);

HI351_write_cmos_sensor(0x18, 0x40);
HI351_write_cmos_sensor(0x19, 0xf0);
HI351_write_cmos_sensor(0x1a, 0x40);
HI351_write_cmos_sensor(0x1b, 0xf0);
HI351_write_cmos_sensor(0x1c, 0x08);
HI351_write_cmos_sensor(0x1d, 0x00);
HI351_write_cmos_sensor(0x1e, 0x34);
HI351_write_cmos_sensor(0x1f, 0x39);

HI351_write_cmos_sensor(0x20, 0x3F);
HI351_write_cmos_sensor(0x21, 0x46);
HI351_write_cmos_sensor(0x22, 0x4B);
HI351_write_cmos_sensor(0x23, 0x52);
HI351_write_cmos_sensor(0x24, 0x58);
HI351_write_cmos_sensor(0x25, 0x5F);
HI351_write_cmos_sensor(0x26, 0x66);
HI351_write_cmos_sensor(0x27, 0x6D);

HI351_write_cmos_sensor(0x28, 0x79);
HI351_write_cmos_sensor(0x29, 0x5F);
HI351_write_cmos_sensor(0x2a, 0x55);
HI351_write_cmos_sensor(0x2b, 0x4E);
HI351_write_cmos_sensor(0x2c, 0x47);
HI351_write_cmos_sensor(0x2d, 0x43);
HI351_write_cmos_sensor(0x2e, 0x40);
HI351_write_cmos_sensor(0x2f, 0x3D);

HI351_write_cmos_sensor(0x30, 0x3C);
HI351_write_cmos_sensor(0x31, 0x3B);
HI351_write_cmos_sensor(0x32, 0x3A);
HI351_write_cmos_sensor(0x33, 0x39);
HI351_write_cmos_sensor(0x34, 0x70);
HI351_write_cmos_sensor(0x35, 0x6B);
HI351_write_cmos_sensor(0x36, 0x67);
HI351_write_cmos_sensor(0x37, 0x64);

HI351_write_cmos_sensor(0x38, 0x60);
HI351_write_cmos_sensor(0x39, 0x5C);
HI351_write_cmos_sensor(0x3a, 0x58);
HI351_write_cmos_sensor(0x3b, 0x54);
HI351_write_cmos_sensor(0x3c, 0x51);
HI351_write_cmos_sensor(0x3d, 0x4C);
HI351_write_cmos_sensor(0x3e, 0x4A);
HI351_write_cmos_sensor(0x3f, 0x34);

HI351_write_cmos_sensor(0x40, 0x3B);
HI351_write_cmos_sensor(0x41, 0x41);
HI351_write_cmos_sensor(0x42, 0x45);
HI351_write_cmos_sensor(0x43, 0x4B);
HI351_write_cmos_sensor(0x44, 0x51);
HI351_write_cmos_sensor(0x45, 0x57);
HI351_write_cmos_sensor(0x46, 0x60);
HI351_write_cmos_sensor(0x47, 0x6A);

HI351_write_cmos_sensor(0x48, 0x72);
HI351_write_cmos_sensor(0x49, 0x79);
HI351_write_cmos_sensor(0x4a, 0x67);
HI351_write_cmos_sensor(0x4b, 0x60);
HI351_write_cmos_sensor(0x4c, 0x59);
HI351_write_cmos_sensor(0x4d, 0x55);
HI351_write_cmos_sensor(0x4e, 0x50);
HI351_write_cmos_sensor(0x4f, 0x4D);

HI351_write_cmos_sensor(0x50, 0x4A);
HI351_write_cmos_sensor(0x51, 0x46);
HI351_write_cmos_sensor(0x52, 0x44);
HI351_write_cmos_sensor(0x53, 0x42);
HI351_write_cmos_sensor(0x54, 0x41);
HI351_write_cmos_sensor(0x55, 0x50);
HI351_write_cmos_sensor(0x56, 0x5a);
HI351_write_cmos_sensor(0x57, 0x64);

HI351_write_cmos_sensor(0x58, 0x6e);
HI351_write_cmos_sensor(0x59, 0x78);
HI351_write_cmos_sensor(0x5a, 0x82);
HI351_write_cmos_sensor(0x5b, 0x8c);
HI351_write_cmos_sensor(0x5c, 0x96);
HI351_write_cmos_sensor(0x5d, 0x00);
HI351_write_cmos_sensor(0x5e, 0x00);
HI351_write_cmos_sensor(0x5f, 0x00);

HI351_write_cmos_sensor(0x60, 0x00);
HI351_write_cmos_sensor(0x61, 0x00);
HI351_write_cmos_sensor(0x62, 0x00);
HI351_write_cmos_sensor(0x63, 0x00);
HI351_write_cmos_sensor(0x64, 0x00);
HI351_write_cmos_sensor(0x65, 0x00);
HI351_write_cmos_sensor(0x66, 0x00);
HI351_write_cmos_sensor(0x67, 0x00);

HI351_write_cmos_sensor(0x68, 0x00);
HI351_write_cmos_sensor(0x69, 0x00);
HI351_write_cmos_sensor(0x6a, 0x00);
HI351_write_cmos_sensor(0x6b, 0x00);
HI351_write_cmos_sensor(0x6c, 0x00);
HI351_write_cmos_sensor(0x6d, 0x00);
HI351_write_cmos_sensor(0x6e, 0x00);
HI351_write_cmos_sensor(0x6f, 0x00);

HI351_write_cmos_sensor(0x70, 0x00);
HI351_write_cmos_sensor(0x71, 0x00);
HI351_write_cmos_sensor(0x72, 0x00);
HI351_write_cmos_sensor(0x73, 0x00);
HI351_write_cmos_sensor(0x74, 0x00);
HI351_write_cmos_sensor(0x75, 0x00);
HI351_write_cmos_sensor(0x76, 0x00);
HI351_write_cmos_sensor(0x77, 0x00);

HI351_write_cmos_sensor(0x78, 0x00);
HI351_write_cmos_sensor(0x79, 0x00);
HI351_write_cmos_sensor(0x7a, 0x00);
HI351_write_cmos_sensor(0x7b, 0x00);
HI351_write_cmos_sensor(0x7c, 0x00);
HI351_write_cmos_sensor(0x7d, 0x00);
HI351_write_cmos_sensor(0x7e, 0x00);
HI351_write_cmos_sensor(0x7f, 0x00);

HI351_write_cmos_sensor(0x80, 0x00);
HI351_write_cmos_sensor(0x81, 0x00);
HI351_write_cmos_sensor(0x82, 0x00);
HI351_write_cmos_sensor(0x83, 0x00);
HI351_write_cmos_sensor(0x84, 0x00);
HI351_write_cmos_sensor(0x85, 0x0a);
HI351_write_cmos_sensor(0x86, 0x0a);
HI351_write_cmos_sensor(0x87, 0x0a);

HI351_write_cmos_sensor(0x88, 0x0a);
HI351_write_cmos_sensor(0x89, 0x0a);
HI351_write_cmos_sensor(0x8a, 0x0a);
HI351_write_cmos_sensor(0x8b, 0x0a);
HI351_write_cmos_sensor(0x8c, 0x28);
HI351_write_cmos_sensor(0x8d, 0x28);
HI351_write_cmos_sensor(0x8e, 0x28);
HI351_write_cmos_sensor(0x8f, 0x28);

HI351_write_cmos_sensor(0x90, 0x1e);
HI351_write_cmos_sensor(0x91, 0x1e);
HI351_write_cmos_sensor(0x92, 0x0a);
HI351_write_cmos_sensor(0x93, 0x0a);
HI351_write_cmos_sensor(0x94, 0x0a);
HI351_write_cmos_sensor(0x95, 0x20);
HI351_write_cmos_sensor(0x96, 0x1e);
HI351_write_cmos_sensor(0x97, 0x1c);

HI351_write_cmos_sensor(0x98, 0x1a);
HI351_write_cmos_sensor(0x99, 0x18);
HI351_write_cmos_sensor(0x9a, 0x16);
HI351_write_cmos_sensor(0x9b, 0x14);
HI351_write_cmos_sensor(0x9c, 0x14);
HI351_write_cmos_sensor(0x9d, 0x14);
HI351_write_cmos_sensor(0x9e, 0x12);
HI351_write_cmos_sensor(0x9f, 0x12);

HI351_write_cmos_sensor(0xa0, 0x08);
HI351_write_cmos_sensor(0xa1, 0x08);
HI351_write_cmos_sensor(0xa2, 0x08);
HI351_write_cmos_sensor(0xa3, 0x07);
HI351_write_cmos_sensor(0xa4, 0x07);
HI351_write_cmos_sensor(0xa5, 0x07);
HI351_write_cmos_sensor(0xa6, 0x06);
HI351_write_cmos_sensor(0xa7, 0x06);

HI351_write_cmos_sensor(0xa8, 0x05);
HI351_write_cmos_sensor(0xa9, 0x05);
HI351_write_cmos_sensor(0xaa, 0x04);
HI351_write_cmos_sensor(0xab, 0x64);
HI351_write_cmos_sensor(0xac, 0x01);
HI351_write_cmos_sensor(0xad, 0x14);
HI351_write_cmos_sensor(0xae, 0x19);
HI351_write_cmos_sensor(0xaf, 0x64);//kjh out limit 64 -> 76 sky

HI351_write_cmos_sensor(0xb0, 0x14);
HI351_write_cmos_sensor(0xb1, 0x1e);
HI351_write_cmos_sensor(0xb2, 0x20); //50 -> 20 sky outdoor
HI351_write_cmos_sensor(0xb3, 0x32); //1e -> 32(50%)
HI351_write_cmos_sensor(0xb4, 0x14);
HI351_write_cmos_sensor(0xb5, 0x3c);
HI351_write_cmos_sensor(0xb6, 0x1e);
HI351_write_cmos_sensor(0xb7, 0x08);

HI351_write_cmos_sensor(0xb8, 0xd2);
HI351_write_cmos_sensor(0xb9, 0x40);
HI351_write_cmos_sensor(0xba, 0xf0);
HI351_write_cmos_sensor(0xbb, 0x40);
HI351_write_cmos_sensor(0xbc, 0xf0);
HI351_write_cmos_sensor(0xbd, 0x04);
HI351_write_cmos_sensor(0xbe, 0x00);
HI351_write_cmos_sensor(0xbf, 0xcd);
HI351_write_cmos_sensor(0x0e, 0x00); //burst end

/////////////////////////////////////////////
// CD Page (Color ratio)
/////////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xCD);//
HI351_write_cmos_sensor(0x47, 0x06);//
HI351_write_cmos_sensor(0x10, 0xB8);//enable

///////////////////////////////////////////
//Adaptive mode : Page Mode = 0xCF
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xcf);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x00);
HI351_write_cmos_sensor(0x11, 0x84); // STEVE 04 -) 84  //cmc + - , adaptive lsc
HI351_write_cmos_sensor(0x12, 0x01);
HI351_write_cmos_sensor(0x13, 0x01); //Y_LUM_MAX
HI351_write_cmos_sensor(0x14, 0x8a);
HI351_write_cmos_sensor(0x15, 0x30);
HI351_write_cmos_sensor(0x16, 0x80);

HI351_write_cmos_sensor(0x17, 0x00);  //Y_LUM middle 1
HI351_write_cmos_sensor(0x18, 0xba);
HI351_write_cmos_sensor(0x19, 0x99);
HI351_write_cmos_sensor(0x1a, 0xec);

HI351_write_cmos_sensor(0x1b, 0x00);  //Y_LUM middle 2
HI351_write_cmos_sensor(0x1c, 0x09);
HI351_write_cmos_sensor(0x1d, 0x1f);
HI351_write_cmos_sensor(0x1e, 0xf0);

HI351_write_cmos_sensor(0x1f, 0x00);  //Y_LUM min
HI351_write_cmos_sensor(0x20, 0x00);
HI351_write_cmos_sensor(0x21, 0x26);
HI351_write_cmos_sensor(0x22, 0x16);

HI351_write_cmos_sensor(0x23, 0x9a);  //CTEM high
HI351_write_cmos_sensor(0x24, 0x54);  //ctemp middler
HI351_write_cmos_sensor(0x25, 0x35);  //CTEM low

HI351_write_cmos_sensor(0x26, 0x50);  //YCON high
HI351_write_cmos_sensor(0x27, 0x20);  //YCON middle
HI351_write_cmos_sensor(0x28, 0x01);  //YCON low

HI351_write_cmos_sensor(0x29, 0x00); //Y_LUM max_TH
HI351_write_cmos_sensor(0x2a, 0x00);
HI351_write_cmos_sensor(0x2b, 0x00);
HI351_write_cmos_sensor(0x2c, 0x00);

HI351_write_cmos_sensor(0x2d, 0x00);  //Y_LUM middle1_TH
HI351_write_cmos_sensor(0x2e, 0x00);
HI351_write_cmos_sensor(0x2f, 0x00);
HI351_write_cmos_sensor(0x30, 0x00);

HI351_write_cmos_sensor(0x31, 0x00);  //Y_LUM middle_TH
HI351_write_cmos_sensor(0x32, 0x00);
HI351_write_cmos_sensor(0x33, 0x00);
HI351_write_cmos_sensor(0x34, 0x00);

HI351_write_cmos_sensor(0x35, 0x00); //Y_LUM min_TH
HI351_write_cmos_sensor(0x36, 0x00);
HI351_write_cmos_sensor(0x37, 0x00);
HI351_write_cmos_sensor(0x38, 0x00);

HI351_write_cmos_sensor(0x39, 0x00);  //CTEM high_TH
HI351_write_cmos_sensor(0x3a, 0x00);  //CTEM middle_TH
HI351_write_cmos_sensor(0x3b, 0x00); //CTEM low_TH

HI351_write_cmos_sensor(0x3c, 0x00); //YCON high_TH
HI351_write_cmos_sensor(0x3d, 0x00); //YCON middle_TH
HI351_write_cmos_sensor(0x3e, 0x00); //YCON low_TH

////////////////////////////////////////////
// CF Page Adaptive Y Target
///////////////////////////////////////////
HI351_write_cmos_sensor(0x3f, 0x30); //YLVL_00
HI351_write_cmos_sensor(0x40, 0x30); //YLVL_01
HI351_write_cmos_sensor(0x41, 0x30); //YLVL_02
HI351_write_cmos_sensor(0x42, 0x40); //YLVL_03
HI351_write_cmos_sensor(0x43, 0x40); //YLVL_04
HI351_write_cmos_sensor(0x44, 0x40); //YLVL_05
HI351_write_cmos_sensor(0x45, 0x40); //YLVL_06
HI351_write_cmos_sensor(0x46, 0x40); //YLVL_07
HI351_write_cmos_sensor(0x47, 0x40); //YLVL_08
HI351_write_cmos_sensor(0x48, 0x30); //YLVL_09
HI351_write_cmos_sensor(0x49, 0x30); //YLVL_10
HI351_write_cmos_sensor(0x4a, 0x30); //YLVL_11

///////////////////////////////////////////
// CF Page Adaptive Y Contrast (4b~56)
///////////////////////////////////////////
HI351_write_cmos_sensor(0x4b, 0x80);  //YCON_00
HI351_write_cmos_sensor(0x4c, 0x80);  //YCON_01
HI351_write_cmos_sensor(0x4d, 0x80);  //YCON_02
HI351_write_cmos_sensor(0x4e, 0x80);  //Contrast 3 //90
HI351_write_cmos_sensor(0x4f, 0x80);  //Contrast 4
HI351_write_cmos_sensor(0x50, 0x80);  //Contrast 5
HI351_write_cmos_sensor(0x51, 0x80);  //Contrast 6
HI351_write_cmos_sensor(0x52, 0x80);  //Contrast 7
HI351_write_cmos_sensor(0x53, 0x80);  //Contrast 8
HI351_write_cmos_sensor(0x54, 0x80);  //Contrast 9
HI351_write_cmos_sensor(0x55, 0x80);  //Contrast 10
HI351_write_cmos_sensor(0x56, 0x80);  //Contrast 11

///////////////////////////////////////////
// CF Page Adaptive Y OFFSET (57~62)
///////////////////////////////////////////
HI351_write_cmos_sensor(0x57, 0x00);
HI351_write_cmos_sensor(0x58, 0x00);
HI351_write_cmos_sensor(0x59, 0x00);
HI351_write_cmos_sensor(0x5a, 0x00);
HI351_write_cmos_sensor(0x5b, 0x00);
HI351_write_cmos_sensor(0x5c, 0x00);
HI351_write_cmos_sensor(0x5d, 0x00);
HI351_write_cmos_sensor(0x5e, 0x00);
HI351_write_cmos_sensor(0x5f, 0x00);
HI351_write_cmos_sensor(0x60, 0x00);
HI351_write_cmos_sensor(0x61, 0x00);
HI351_write_cmos_sensor(0x62, 0x00);
/////////////2//////////////////////
// CF~D0~D1 P2tive GAMMA
/////////////2//////////////////////


HI351_write_cmos_sensor(0x63, 0x00);   //GMA00
HI351_write_cmos_sensor(0x64, 0x02);
HI351_write_cmos_sensor(0x65, 0x06);
HI351_write_cmos_sensor(0x66, 0x0c);
HI351_write_cmos_sensor(0x67, 0x14);
HI351_write_cmos_sensor(0x68, 0x1e);
HI351_write_cmos_sensor(0x69, 0x28);
HI351_write_cmos_sensor(0x6a, 0x33);
HI351_write_cmos_sensor(0x6b, 0x43);
HI351_write_cmos_sensor(0x6c, 0x4f);
HI351_write_cmos_sensor(0x6d, 0x5d);
HI351_write_cmos_sensor(0x6e, 0x68);
HI351_write_cmos_sensor(0x6f, 0x70);
HI351_write_cmos_sensor(0x70, 0x78);
HI351_write_cmos_sensor(0x71, 0x7f);
HI351_write_cmos_sensor(0x72, 0x85);
HI351_write_cmos_sensor(0x73, 0x8b);
HI351_write_cmos_sensor(0x74, 0x91);
HI351_write_cmos_sensor(0x75, 0x97);
HI351_write_cmos_sensor(0x76, 0x9e);
HI351_write_cmos_sensor(0x77, 0xa3);
HI351_write_cmos_sensor(0x78, 0xad);
HI351_write_cmos_sensor(0x79, 0xb5);
HI351_write_cmos_sensor(0x7a, 0xbd);
HI351_write_cmos_sensor(0x7b, 0xca);
HI351_write_cmos_sensor(0x7c, 0xd7);
HI351_write_cmos_sensor(0x7d, 0xe1);
HI351_write_cmos_sensor(0x7e, 0xea);
HI351_write_cmos_sensor(0x7f, 0xf2);
HI351_write_cmos_sensor(0x80, 0xf6);
HI351_write_cmos_sensor(0x81, 0xf9);
HI351_write_cmos_sensor(0x82, 0xfc);
HI351_write_cmos_sensor(0x83, 0xfe);
HI351_write_cmos_sensor(0x84, 0xff);

HI351_write_cmos_sensor(0x85, 0x00);   //GMA01
HI351_write_cmos_sensor(0x86, 0x02);
HI351_write_cmos_sensor(0x87, 0x06);
HI351_write_cmos_sensor(0x88, 0x0c);
HI351_write_cmos_sensor(0x89, 0x14);
HI351_write_cmos_sensor(0x8a, 0x1e);
HI351_write_cmos_sensor(0x8b, 0x28);
HI351_write_cmos_sensor(0x8c, 0x33);
HI351_write_cmos_sensor(0x8d, 0x43);
HI351_write_cmos_sensor(0x8e, 0x4f);
HI351_write_cmos_sensor(0x8f, 0x5d);
HI351_write_cmos_sensor(0x90, 0x68);
HI351_write_cmos_sensor(0x91, 0x70);
HI351_write_cmos_sensor(0x92, 0x78);
HI351_write_cmos_sensor(0x93, 0x7f);
HI351_write_cmos_sensor(0x94, 0x85);
HI351_write_cmos_sensor(0x95, 0x8b);
HI351_write_cmos_sensor(0x96, 0x91);
HI351_write_cmos_sensor(0x97, 0x97);
HI351_write_cmos_sensor(0x98, 0x9e);
HI351_write_cmos_sensor(0x99, 0xa3);
HI351_write_cmos_sensor(0x9a, 0xad);
HI351_write_cmos_sensor(0x9b, 0xb5);
HI351_write_cmos_sensor(0x9c, 0xbd);
HI351_write_cmos_sensor(0x9d, 0xca);
HI351_write_cmos_sensor(0x9e, 0xd7);
HI351_write_cmos_sensor(0x9f, 0xe1);
HI351_write_cmos_sensor(0xa0, 0xea);
HI351_write_cmos_sensor(0xa1, 0xf2);
HI351_write_cmos_sensor(0xa2, 0xf6);
HI351_write_cmos_sensor(0xa3, 0xf9);
HI351_write_cmos_sensor(0xa4, 0xfc);
HI351_write_cmos_sensor(0xa5, 0xfe);
HI351_write_cmos_sensor(0xa6, 0xff);

HI351_write_cmos_sensor(0xa7, 0x00);   //GMA02
HI351_write_cmos_sensor(0xa8, 0x02);
HI351_write_cmos_sensor(0xa9, 0x06);
HI351_write_cmos_sensor(0xaa, 0x0c);
HI351_write_cmos_sensor(0xab, 0x14);
HI351_write_cmos_sensor(0xac, 0x1e);
HI351_write_cmos_sensor(0xad, 0x28);
HI351_write_cmos_sensor(0xae, 0x33);
HI351_write_cmos_sensor(0xaf, 0x43);
HI351_write_cmos_sensor(0xb0, 0x4f);
HI351_write_cmos_sensor(0xb1, 0x5d);
HI351_write_cmos_sensor(0xb2, 0x68);
HI351_write_cmos_sensor(0xb3, 0x70);
HI351_write_cmos_sensor(0xb4, 0x78);
HI351_write_cmos_sensor(0xb5, 0x7f);
HI351_write_cmos_sensor(0xb6, 0x85);
HI351_write_cmos_sensor(0xb7, 0x8b);
HI351_write_cmos_sensor(0xb8, 0x91);
HI351_write_cmos_sensor(0xb9, 0x97);
HI351_write_cmos_sensor(0xba, 0x9e);
HI351_write_cmos_sensor(0xbb, 0xa3);
HI351_write_cmos_sensor(0xbc, 0xad);
HI351_write_cmos_sensor(0xbd, 0xb5);
HI351_write_cmos_sensor(0xbe, 0xbd);
HI351_write_cmos_sensor(0xbf, 0xca);
HI351_write_cmos_sensor(0xc0, 0xd7);
HI351_write_cmos_sensor(0xc1, 0xe1);
HI351_write_cmos_sensor(0xc2, 0xea);
HI351_write_cmos_sensor(0xc3, 0xf2);
HI351_write_cmos_sensor(0xc4, 0xf6);
HI351_write_cmos_sensor(0xc5, 0xf9);
HI351_write_cmos_sensor(0xc6, 0xfc);
HI351_write_cmos_sensor(0xc7, 0xfe);
HI351_write_cmos_sensor(0xc8, 0xff);

HI351_write_cmos_sensor(0xc9, 0x00);//GMA03
HI351_write_cmos_sensor(0xca, 0x02);
HI351_write_cmos_sensor(0xcb, 0x06);
HI351_write_cmos_sensor(0xcc, 0x0c);
HI351_write_cmos_sensor(0xcd, 0x14);
HI351_write_cmos_sensor(0xce, 0x1e);
HI351_write_cmos_sensor(0xcf, 0x28);
HI351_write_cmos_sensor(0xd0, 0x33);
HI351_write_cmos_sensor(0xd1, 0x43);
HI351_write_cmos_sensor(0xd2, 0x4f);
HI351_write_cmos_sensor(0xd3, 0x5d);
HI351_write_cmos_sensor(0xd4, 0x68);
HI351_write_cmos_sensor(0xd5, 0x70);
HI351_write_cmos_sensor(0xd6, 0x78);
HI351_write_cmos_sensor(0xd7, 0x7F);
HI351_write_cmos_sensor(0xd8, 0x85);
HI351_write_cmos_sensor(0xd9, 0x8b);
HI351_write_cmos_sensor(0xda, 0x91);
HI351_write_cmos_sensor(0xdb, 0x97);
HI351_write_cmos_sensor(0xdc, 0x9e);
HI351_write_cmos_sensor(0xdd, 0xa3);
HI351_write_cmos_sensor(0xde, 0xad);
HI351_write_cmos_sensor(0xdf, 0xb5);
HI351_write_cmos_sensor(0xe0, 0xbd);
HI351_write_cmos_sensor(0xe1, 0xca);
HI351_write_cmos_sensor(0xe2, 0xd7);
HI351_write_cmos_sensor(0xe3, 0xe1);
HI351_write_cmos_sensor(0xe4, 0xea);
HI351_write_cmos_sensor(0xe5, 0xf2);
HI351_write_cmos_sensor(0xe6, 0xf6);
HI351_write_cmos_sensor(0xe7, 0xf9);
HI351_write_cmos_sensor(0xe8, 0xfc);
HI351_write_cmos_sensor(0xe9, 0xfe);
HI351_write_cmos_sensor(0xea, 0xff);

HI351_write_cmos_sensor(0xeb, 0x00);//GMA04
HI351_write_cmos_sensor(0xec, 0x02);
HI351_write_cmos_sensor(0xed, 0x06);
HI351_write_cmos_sensor(0xee, 0x0c);
HI351_write_cmos_sensor(0xef, 0x14);
HI351_write_cmos_sensor(0xf0, 0x1e);
HI351_write_cmos_sensor(0xf1, 0x28);
HI351_write_cmos_sensor(0xf2, 0x33);
HI351_write_cmos_sensor(0xf3, 0x43);
HI351_write_cmos_sensor(0xf4, 0x4f);
HI351_write_cmos_sensor(0xf5, 0x5d);
HI351_write_cmos_sensor(0xf6, 0x68);
HI351_write_cmos_sensor(0xf7, 0x70);
HI351_write_cmos_sensor(0xf8, 0x78);
HI351_write_cmos_sensor(0xf9, 0x7F);
HI351_write_cmos_sensor(0xfa, 0x85);
HI351_write_cmos_sensor(0xfb, 0x8b);
HI351_write_cmos_sensor(0xfc, 0x91);
HI351_write_cmos_sensor(0xfd, 0x97);
HI351_write_cmos_sensor(0x0e, 0x00); //burst end
HI351_write_cmos_sensor(0x03, 0xd0); //Page d0
HI351_write_cmos_sensor(0x0e, 0x01); //burst start
HI351_write_cmos_sensor(0x10, 0x9e);
HI351_write_cmos_sensor(0x11, 0xa3);
HI351_write_cmos_sensor(0x12, 0xad);
HI351_write_cmos_sensor(0x13, 0xb5);
HI351_write_cmos_sensor(0x14, 0xbd);
HI351_write_cmos_sensor(0x15, 0xca);
HI351_write_cmos_sensor(0x16, 0xd7);
HI351_write_cmos_sensor(0x17, 0xe1);
HI351_write_cmos_sensor(0x18, 0xea);
HI351_write_cmos_sensor(0x19, 0xf2);
HI351_write_cmos_sensor(0x1a, 0xf6);
HI351_write_cmos_sensor(0x1b, 0xf9);
HI351_write_cmos_sensor(0x1c, 0xfc);
HI351_write_cmos_sensor(0x1d, 0xfe);
HI351_write_cmos_sensor(0x1e, 0xff);

HI351_write_cmos_sensor(0x1f, 0x00);//GMA05
HI351_write_cmos_sensor(0x20, 0x02);
HI351_write_cmos_sensor(0x21, 0x06);
HI351_write_cmos_sensor(0x22, 0x0c);
HI351_write_cmos_sensor(0x23, 0x14);
HI351_write_cmos_sensor(0x24, 0x1e);
HI351_write_cmos_sensor(0x25, 0x28);
HI351_write_cmos_sensor(0x26, 0x33);
HI351_write_cmos_sensor(0x27, 0x43);
HI351_write_cmos_sensor(0x28, 0x4f);
HI351_write_cmos_sensor(0x29, 0x5d);
HI351_write_cmos_sensor(0x2a, 0x68);
HI351_write_cmos_sensor(0x2b, 0x70);
HI351_write_cmos_sensor(0x2c, 0x78);
HI351_write_cmos_sensor(0x2d, 0x7F);
HI351_write_cmos_sensor(0x2e, 0x85);
HI351_write_cmos_sensor(0x2f, 0x8b);
HI351_write_cmos_sensor(0x30, 0x91);
HI351_write_cmos_sensor(0x31, 0x97);
HI351_write_cmos_sensor(0x32, 0x9e);
HI351_write_cmos_sensor(0x33, 0xa3);
HI351_write_cmos_sensor(0x34, 0xad);
HI351_write_cmos_sensor(0x35, 0xb5);
HI351_write_cmos_sensor(0x36, 0xbd);
HI351_write_cmos_sensor(0x37, 0xca);
HI351_write_cmos_sensor(0x38, 0xd7);
HI351_write_cmos_sensor(0x39, 0xe1);
HI351_write_cmos_sensor(0x3a, 0xea);
HI351_write_cmos_sensor(0x3b, 0xf2);
HI351_write_cmos_sensor(0x3c, 0xf6);
HI351_write_cmos_sensor(0x3d, 0xf9);
HI351_write_cmos_sensor(0x3e, 0xfc);
HI351_write_cmos_sensor(0x3f, 0xfe);
HI351_write_cmos_sensor(0x40, 0xff);

HI351_write_cmos_sensor(0x41, 0x00);//GMA06
HI351_write_cmos_sensor(0x42, 0x02);
HI351_write_cmos_sensor(0x43, 0x06);
HI351_write_cmos_sensor(0x44, 0x0c);
HI351_write_cmos_sensor(0x45, 0x14);
HI351_write_cmos_sensor(0x46, 0x1e);
HI351_write_cmos_sensor(0x47, 0x28);
HI351_write_cmos_sensor(0x48, 0x33);
HI351_write_cmos_sensor(0x49, 0x43);
HI351_write_cmos_sensor(0x4a, 0x4f);
HI351_write_cmos_sensor(0x4b, 0x5d);
HI351_write_cmos_sensor(0x4c, 0x68);
HI351_write_cmos_sensor(0x4d, 0x70);
HI351_write_cmos_sensor(0x4e, 0x78);
HI351_write_cmos_sensor(0x4f, 0x7f);
HI351_write_cmos_sensor(0x50, 0x85);
HI351_write_cmos_sensor(0x51, 0x8b);
HI351_write_cmos_sensor(0x52, 0x91);
HI351_write_cmos_sensor(0x53, 0x97);
HI351_write_cmos_sensor(0x54, 0x9e);
HI351_write_cmos_sensor(0x55, 0xa3);
HI351_write_cmos_sensor(0x56, 0xad);
HI351_write_cmos_sensor(0x57, 0xb5);
HI351_write_cmos_sensor(0x58, 0xbd);
HI351_write_cmos_sensor(0x59, 0xca);
HI351_write_cmos_sensor(0x5a, 0xd7);
HI351_write_cmos_sensor(0x5b, 0xe1);
HI351_write_cmos_sensor(0x5c, 0xea);
HI351_write_cmos_sensor(0x5d, 0xf2);
HI351_write_cmos_sensor(0x5e, 0xf6);
HI351_write_cmos_sensor(0x5f, 0xf9);
HI351_write_cmos_sensor(0x60, 0xfc);
HI351_write_cmos_sensor(0x61, 0xfe);
HI351_write_cmos_sensor(0x62, 0xff);

HI351_write_cmos_sensor(0x63, 0x00);//GMA07
HI351_write_cmos_sensor(0x64, 0x02);
HI351_write_cmos_sensor(0x65, 0x06);
HI351_write_cmos_sensor(0x66, 0x0c);
HI351_write_cmos_sensor(0x67, 0x14);
HI351_write_cmos_sensor(0x68, 0x1e);
HI351_write_cmos_sensor(0x69, 0x28);
HI351_write_cmos_sensor(0x6a, 0x33);
HI351_write_cmos_sensor(0x6b, 0x43);
HI351_write_cmos_sensor(0x6c, 0x4f);
HI351_write_cmos_sensor(0x6d, 0x5d);
HI351_write_cmos_sensor(0x6e, 0x68);
HI351_write_cmos_sensor(0x6f, 0x70);
HI351_write_cmos_sensor(0x70, 0x78);
HI351_write_cmos_sensor(0x71, 0x7f);
HI351_write_cmos_sensor(0x72, 0x85);
HI351_write_cmos_sensor(0x73, 0x8b);
HI351_write_cmos_sensor(0x74, 0x91);
HI351_write_cmos_sensor(0x75, 0x97);
HI351_write_cmos_sensor(0x76, 0x9e);
HI351_write_cmos_sensor(0x77, 0xa3);
HI351_write_cmos_sensor(0x78, 0xad);
HI351_write_cmos_sensor(0x79, 0xb5);
HI351_write_cmos_sensor(0x7a, 0xbd);
HI351_write_cmos_sensor(0x7b, 0xca);
HI351_write_cmos_sensor(0x7c, 0xd7);
HI351_write_cmos_sensor(0x7d, 0xe1);
HI351_write_cmos_sensor(0x7e, 0xea);
HI351_write_cmos_sensor(0x7f, 0xf2);
HI351_write_cmos_sensor(0x80, 0xf6);
HI351_write_cmos_sensor(0x81, 0xf9);
HI351_write_cmos_sensor(0x82, 0xfc);
HI351_write_cmos_sensor(0x83, 0xfe);
HI351_write_cmos_sensor(0x84, 0xff);

HI351_write_cmos_sensor(0x85, 0x00);//GMA08
HI351_write_cmos_sensor(0x86, 0x02);
HI351_write_cmos_sensor(0x87, 0x06);
HI351_write_cmos_sensor(0x88, 0x0c);
HI351_write_cmos_sensor(0x89, 0x14);
HI351_write_cmos_sensor(0x8a, 0x1e);
HI351_write_cmos_sensor(0x8b, 0x28);
HI351_write_cmos_sensor(0x8c, 0x33);
HI351_write_cmos_sensor(0x8d, 0x43);
HI351_write_cmos_sensor(0x8e, 0x4f);
HI351_write_cmos_sensor(0x8f, 0x5d);
HI351_write_cmos_sensor(0x90, 0x68);
HI351_write_cmos_sensor(0x91, 0x70);
HI351_write_cmos_sensor(0x92, 0x78);
HI351_write_cmos_sensor(0x93, 0x7f);
HI351_write_cmos_sensor(0x94, 0x85);
HI351_write_cmos_sensor(0x95, 0x8b);
HI351_write_cmos_sensor(0x96, 0x91);
HI351_write_cmos_sensor(0x97, 0x97);
HI351_write_cmos_sensor(0x98, 0x9e);
HI351_write_cmos_sensor(0x99, 0xa3);
HI351_write_cmos_sensor(0x9a, 0xad);
HI351_write_cmos_sensor(0x9b, 0xb5);
HI351_write_cmos_sensor(0x9c, 0xbd);
HI351_write_cmos_sensor(0x9d, 0xca);
HI351_write_cmos_sensor(0x9e, 0xd7);
HI351_write_cmos_sensor(0x9f, 0xe1);
HI351_write_cmos_sensor(0xa0, 0xea);
HI351_write_cmos_sensor(0xa1, 0xf2);
HI351_write_cmos_sensor(0xa2, 0xf6);
HI351_write_cmos_sensor(0xa3, 0xf9);
HI351_write_cmos_sensor(0xa4, 0xfc);
HI351_write_cmos_sensor(0xa5, 0xfe);
HI351_write_cmos_sensor(0xa6, 0xff);

HI351_write_cmos_sensor(0xa7, 0x00);//GMA09
HI351_write_cmos_sensor(0xa8, 0x02);
HI351_write_cmos_sensor(0xa9, 0x06);
HI351_write_cmos_sensor(0xaa, 0x0c);
HI351_write_cmos_sensor(0xab, 0x14);
HI351_write_cmos_sensor(0xac, 0x1e);
HI351_write_cmos_sensor(0xad, 0x28);
HI351_write_cmos_sensor(0xae, 0x33);
HI351_write_cmos_sensor(0xaf, 0x43);
HI351_write_cmos_sensor(0xb0, 0x4f);
HI351_write_cmos_sensor(0xb1, 0x5d);
HI351_write_cmos_sensor(0xb2, 0x68);
HI351_write_cmos_sensor(0xb3, 0x70);
HI351_write_cmos_sensor(0xb4, 0x78);
HI351_write_cmos_sensor(0xb5, 0x7f);
HI351_write_cmos_sensor(0xb6, 0x85);
HI351_write_cmos_sensor(0xb7, 0x8b);
HI351_write_cmos_sensor(0xb8, 0x91);
HI351_write_cmos_sensor(0xb9, 0x97);
HI351_write_cmos_sensor(0xba, 0x9e);
HI351_write_cmos_sensor(0xbb, 0xa3);
HI351_write_cmos_sensor(0xbc, 0xad);
HI351_write_cmos_sensor(0xbd, 0xb5);
HI351_write_cmos_sensor(0xbe, 0xbd);
HI351_write_cmos_sensor(0xbf, 0xca);
HI351_write_cmos_sensor(0xc0, 0xd7);
HI351_write_cmos_sensor(0xc1, 0xe1);
HI351_write_cmos_sensor(0xc2, 0xea);
HI351_write_cmos_sensor(0xc3, 0xf2);
HI351_write_cmos_sensor(0xc4, 0xf6);
HI351_write_cmos_sensor(0xc5, 0xf9);
HI351_write_cmos_sensor(0xc6, 0xfc);
HI351_write_cmos_sensor(0xc7, 0xfe);
HI351_write_cmos_sensor(0xc8, 0xff);

HI351_write_cmos_sensor(0xc9, 0x00);//GMA10
HI351_write_cmos_sensor(0xca, 0x02);
HI351_write_cmos_sensor(0xcb, 0x06);
HI351_write_cmos_sensor(0xcc, 0x0c);
HI351_write_cmos_sensor(0xcd, 0x14);
HI351_write_cmos_sensor(0xce, 0x1e);
HI351_write_cmos_sensor(0xcf, 0x28);
HI351_write_cmos_sensor(0xd0, 0x33);
HI351_write_cmos_sensor(0xd1, 0x43);
HI351_write_cmos_sensor(0xd2, 0x4f);
HI351_write_cmos_sensor(0xd3, 0x5d);
HI351_write_cmos_sensor(0xd4, 0x68);
HI351_write_cmos_sensor(0xd5, 0x70);
HI351_write_cmos_sensor(0xd6, 0x78);
HI351_write_cmos_sensor(0xd7, 0x7f);
HI351_write_cmos_sensor(0xd8, 0x85);
HI351_write_cmos_sensor(0xd9, 0x8b);
HI351_write_cmos_sensor(0xda, 0x91);
HI351_write_cmos_sensor(0xdb, 0x97);
HI351_write_cmos_sensor(0xdc, 0x9e);
HI351_write_cmos_sensor(0xdd, 0xa3);
HI351_write_cmos_sensor(0xde, 0xad);
HI351_write_cmos_sensor(0xdf, 0xb5);
HI351_write_cmos_sensor(0xe0, 0xbd);
HI351_write_cmos_sensor(0xe1, 0xca);
HI351_write_cmos_sensor(0xe2, 0xd7);
HI351_write_cmos_sensor(0xe3, 0xe1);
HI351_write_cmos_sensor(0xe4, 0xea);
HI351_write_cmos_sensor(0xe5, 0xf2);
HI351_write_cmos_sensor(0xe6, 0xf6);
HI351_write_cmos_sensor(0xe7, 0xf9);
HI351_write_cmos_sensor(0xe8, 0xfc);
HI351_write_cmos_sensor(0xe9, 0xfe);
HI351_write_cmos_sensor(0xea, 0xff);

HI351_write_cmos_sensor(0xeb, 0x00);//GMA11
HI351_write_cmos_sensor(0xec, 0x02);
HI351_write_cmos_sensor(0xed, 0x06);
HI351_write_cmos_sensor(0xee, 0x0c);
HI351_write_cmos_sensor(0xef, 0x14);
HI351_write_cmos_sensor(0xf0, 0x1e);
HI351_write_cmos_sensor(0xf1, 0x28);
HI351_write_cmos_sensor(0xf2, 0x33);
HI351_write_cmos_sensor(0xf3, 0x43);
HI351_write_cmos_sensor(0xf4, 0x4f);
HI351_write_cmos_sensor(0xf5, 0x5d);
HI351_write_cmos_sensor(0xf6, 0x68);
HI351_write_cmos_sensor(0xf7, 0x70);
HI351_write_cmos_sensor(0xf8, 0x78);
HI351_write_cmos_sensor(0xf9, 0x7f);
HI351_write_cmos_sensor(0xfa, 0x85);
HI351_write_cmos_sensor(0xfb, 0x8b);
HI351_write_cmos_sensor(0xfc, 0x91);
HI351_write_cmos_sensor(0xfd, 0x97);
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
HI351_write_cmos_sensor(0x03, 0xd1);//Page d1
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x9e);
HI351_write_cmos_sensor(0x11, 0xa3);
HI351_write_cmos_sensor(0x12, 0xad);
HI351_write_cmos_sensor(0x13, 0xb5);
HI351_write_cmos_sensor(0x14, 0xbd);
HI351_write_cmos_sensor(0x15, 0xca);
HI351_write_cmos_sensor(0x16, 0xd7);
HI351_write_cmos_sensor(0x17, 0xe1);
HI351_write_cmos_sensor(0x18, 0xea);
HI351_write_cmos_sensor(0x19, 0xf2);
HI351_write_cmos_sensor(0x1a, 0xf6);
HI351_write_cmos_sensor(0x1b, 0xf9);
HI351_write_cmos_sensor(0x1c, 0xfc);
HI351_write_cmos_sensor(0x1d, 0xfe);
HI351_write_cmos_sensor(0x1e, 0xff);

///////////////////////////////////////////
// D1 Page Adaptive Y Target delta
///////////////////////////////////////////
HI351_write_cmos_sensor(0x1f, 0x80);//Y target delta 0
HI351_write_cmos_sensor(0x20, 0x80);//Y target delta 1
HI351_write_cmos_sensor(0x21, 0x80);//Y target delta 2
HI351_write_cmos_sensor(0x22, 0x80);//Y target delta 3
HI351_write_cmos_sensor(0x23, 0x80);//Y target delta 4
HI351_write_cmos_sensor(0x24, 0x80);//Y target delta 5
HI351_write_cmos_sensor(0x25, 0x80);//Y target delta 6
HI351_write_cmos_sensor(0x26, 0x80);//Y target delta 7
HI351_write_cmos_sensor(0x27, 0x80);//Y target delta 8
HI351_write_cmos_sensor(0x28, 0x80);//Y target delta 9
HI351_write_cmos_sensor(0x29, 0x80);//Y target delta 10
HI351_write_cmos_sensor(0x2a, 0x80);//Y target delta 11
///////////////////////////////////////////
// D1 Page Adaptive R/B saturation
///////////////////////////////////////////
//Cb
HI351_write_cmos_sensor(0x2b, 0x88); //SATB_00
HI351_write_cmos_sensor(0x2c, 0x88); //SATB_01
HI351_write_cmos_sensor(0x2d, 0x88); //SATB_02
HI351_write_cmos_sensor(0x2e, 0x98); //SATB_03
HI351_write_cmos_sensor(0x2f, 0xa0); //SATB_04
HI351_write_cmos_sensor(0x30, 0xa8); //SATB_05
HI351_write_cmos_sensor(0x31, 0x98); //SATB_06
HI351_write_cmos_sensor(0x32, 0xa0); //SATB_07
HI351_write_cmos_sensor(0x33, 0xa8); //SATB_08
HI351_write_cmos_sensor(0x34, 0x98); //SATB_09
HI351_write_cmos_sensor(0x35, 0x98); //SATB_10
HI351_write_cmos_sensor(0x36, 0x98); //SATB_11
//Cr
HI351_write_cmos_sensor(0x37, 0x88); //SATR_00
HI351_write_cmos_sensor(0x38, 0x88); //SATR_01
HI351_write_cmos_sensor(0x39, 0x88); //SATR_02
HI351_write_cmos_sensor(0x3a, 0x98); //SATR_03
HI351_write_cmos_sensor(0x3b, 0xa0); //SATR_04
HI351_write_cmos_sensor(0x3c, 0xa8); //SATR_05
HI351_write_cmos_sensor(0x3d, 0x98); //SATR_06
HI351_write_cmos_sensor(0x3e, 0xa0); //SATR_07
HI351_write_cmos_sensor(0x3f, 0xa8); //SATR_08
HI351_write_cmos_sensor(0x40, 0x98); //SATR_09
HI351_write_cmos_sensor(0x41, 0x98); //SATR_10
HI351_write_cmos_sensor(0x42, 0x98); //SATR_11

///////////////////////////////////////////
// D1 Page Adaptive CMC
///////////////////////////////////////////

HI351_write_cmos_sensor(0x43, 0x2f); //CMC_00
HI351_write_cmos_sensor(0x44, 0x68);
HI351_write_cmos_sensor(0x45, 0x29);
HI351_write_cmos_sensor(0x46, 0x01);
HI351_write_cmos_sensor(0x47, 0x19);
HI351_write_cmos_sensor(0x48, 0x6c);
HI351_write_cmos_sensor(0x49, 0x13);
HI351_write_cmos_sensor(0x4a, 0x13);
HI351_write_cmos_sensor(0x4b, 0x1e);
HI351_write_cmos_sensor(0x4c, 0x71);

HI351_write_cmos_sensor(0x4d, 0x2f); //CMC_01
HI351_write_cmos_sensor(0x4e, 0x68);
HI351_write_cmos_sensor(0x4f, 0x29);
HI351_write_cmos_sensor(0x50, 0x01);
HI351_write_cmos_sensor(0x51, 0x19);
HI351_write_cmos_sensor(0x52, 0x6c);
HI351_write_cmos_sensor(0x53, 0x13);
HI351_write_cmos_sensor(0x54, 0x13);
HI351_write_cmos_sensor(0x55, 0x1e);
HI351_write_cmos_sensor(0x56, 0x71);

HI351_write_cmos_sensor(0x57, 0x2f); //CMC_02
HI351_write_cmos_sensor(0x58, 0x68);
HI351_write_cmos_sensor(0x59, 0x29);
HI351_write_cmos_sensor(0x5a, 0x01);
HI351_write_cmos_sensor(0x5b, 0x19);
HI351_write_cmos_sensor(0x5c, 0x6c);
HI351_write_cmos_sensor(0x5d, 0x13);
HI351_write_cmos_sensor(0x5e, 0x13);
HI351_write_cmos_sensor(0x5f, 0x1e);
HI351_write_cmos_sensor(0x60, 0x71);

HI351_write_cmos_sensor(0x61, 0x2f); //CMC_03
HI351_write_cmos_sensor(0x62, 0x6a);
HI351_write_cmos_sensor(0x63, 0x32);
HI351_write_cmos_sensor(0x64, 0x08);
HI351_write_cmos_sensor(0x65, 0x1a);
HI351_write_cmos_sensor(0x66, 0x6c);
HI351_write_cmos_sensor(0x67, 0x12);
HI351_write_cmos_sensor(0x68, 0x03);
HI351_write_cmos_sensor(0x69, 0x30);
HI351_write_cmos_sensor(0x6a, 0x73);

HI351_write_cmos_sensor(0x6b, 0x2f);	//CMC_04
HI351_write_cmos_sensor(0x6c, 0x68);
HI351_write_cmos_sensor(0x6d, 0x29);
HI351_write_cmos_sensor(0x6e, 0x01);
HI351_write_cmos_sensor(0x6f, 0x17);
HI351_write_cmos_sensor(0x70, 0x6c);
HI351_write_cmos_sensor(0x71, 0x15);
HI351_write_cmos_sensor(0x72, 0x01);
HI351_write_cmos_sensor(0x73, 0x30);
HI351_write_cmos_sensor(0x74, 0x71);

HI351_write_cmos_sensor(0x75, 0x2f);	//CMC_05
HI351_write_cmos_sensor(0x76, 0x68);
HI351_write_cmos_sensor(0x77, 0x29);
HI351_write_cmos_sensor(0x78, 0x01);
HI351_write_cmos_sensor(0x79, 0x17);
HI351_write_cmos_sensor(0x7a, 0x6c);
HI351_write_cmos_sensor(0x7b, 0x15);
HI351_write_cmos_sensor(0x7c, 0x01);
HI351_write_cmos_sensor(0x7d, 0x30);
HI351_write_cmos_sensor(0x7e, 0x71);

HI351_write_cmos_sensor(0x7f, 0x2f); //CMC_06
HI351_write_cmos_sensor(0x80, 0x6a);
HI351_write_cmos_sensor(0x81, 0x3c);
HI351_write_cmos_sensor(0x82, 0x12);
HI351_write_cmos_sensor(0x83, 0x21);
HI351_write_cmos_sensor(0x84, 0x70);
HI351_write_cmos_sensor(0x85, 0x0f);
HI351_write_cmos_sensor(0x86, 0x06);
HI351_write_cmos_sensor(0x87, 0x2d);
HI351_write_cmos_sensor(0x88, 0x73);

HI351_write_cmos_sensor(0x89, 0x2f); //CMC_07
HI351_write_cmos_sensor(0x8a, 0x6a);
HI351_write_cmos_sensor(0x8b, 0x3c);
HI351_write_cmos_sensor(0x8c, 0x12);
HI351_write_cmos_sensor(0x8d, 0x21);
HI351_write_cmos_sensor(0x8e, 0x70);
HI351_write_cmos_sensor(0x8f, 0x0f);
HI351_write_cmos_sensor(0x90, 0x06);
HI351_write_cmos_sensor(0x91, 0x2d);
HI351_write_cmos_sensor(0x92, 0x73);

HI351_write_cmos_sensor(0x93, 0x2f); //CMC_08
HI351_write_cmos_sensor(0x94, 0x6a);
HI351_write_cmos_sensor(0x95, 0x3c);
HI351_write_cmos_sensor(0x96, 0x12);
HI351_write_cmos_sensor(0x97, 0x21);
HI351_write_cmos_sensor(0x98, 0x70);
HI351_write_cmos_sensor(0x99, 0x0f);
HI351_write_cmos_sensor(0x9a, 0x06);
HI351_write_cmos_sensor(0x9b, 0x2d);
HI351_write_cmos_sensor(0x9c, 0x73);

HI351_write_cmos_sensor(0x9d, 0x2f); //CMC_09
HI351_write_cmos_sensor(0x9e, 0x6a);
HI351_write_cmos_sensor(0x9f, 0x3c);
HI351_write_cmos_sensor(0xa0, 0x12);
HI351_write_cmos_sensor(0xa1, 0x21);
HI351_write_cmos_sensor(0xa2, 0x70);
HI351_write_cmos_sensor(0xa3, 0x0f);
HI351_write_cmos_sensor(0xa4, 0x06);
HI351_write_cmos_sensor(0xa5, 0x2d);
HI351_write_cmos_sensor(0xa6, 0x73);

HI351_write_cmos_sensor(0xa7, 0x2f); //CMC_10
HI351_write_cmos_sensor(0xa8, 0x6a);
HI351_write_cmos_sensor(0xa9, 0x3c);
HI351_write_cmos_sensor(0xaa, 0x12);
HI351_write_cmos_sensor(0xab, 0x21);
HI351_write_cmos_sensor(0xac, 0x70);
HI351_write_cmos_sensor(0xad, 0x0f);
HI351_write_cmos_sensor(0xae, 0x06);
HI351_write_cmos_sensor(0xaf, 0x2d);
HI351_write_cmos_sensor(0xb0, 0x73);

HI351_write_cmos_sensor(0xb1, 0x2f); //CMC_11
HI351_write_cmos_sensor(0xb2, 0x6a);
HI351_write_cmos_sensor(0xb3, 0x3c);
HI351_write_cmos_sensor(0xb4, 0x12);
HI351_write_cmos_sensor(0xb5, 0x21);
HI351_write_cmos_sensor(0xb6, 0x70);
HI351_write_cmos_sensor(0xb7, 0x0f);
HI351_write_cmos_sensor(0xb8, 0x06);
HI351_write_cmos_sensor(0xb9, 0x2d);
HI351_write_cmos_sensor(0xba, 0x73);

///////////////////////////////////////////
// D1~D2~D3 Page Adaptive Multi-CMC
///////////////////////////////////////////
//MCMC_00
HI351_write_cmos_sensor(0xbb, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0xbc, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0xbd, 0x73);//0_GAIN   //115 //R1 //Red
HI351_write_cmos_sensor(0xbe, 0x81);//0_HUE	  //-1
HI351_write_cmos_sensor(0xbf, 0x35);//0_CENTER //53
HI351_write_cmos_sensor(0xc0, 0x1a);//0_DELTA  //26
HI351_write_cmos_sensor(0xc1, 0x6c);//1_GAIN   //108 //r2 //Green
HI351_write_cmos_sensor(0xc2, 0x12);//1_HUE	  //+18
HI351_write_cmos_sensor(0xc3, 0x74); //jktest 0202//116//0x76);//1_CENTER //118
HI351_write_cmos_sensor(0xc4, 0x21); //jktest 0202//33//0x34);//1_DELTA  //52
HI351_write_cmos_sensor(0xc5, 0x88);//2_GAIN   //136 //r3 //Blue
HI351_write_cmos_sensor(0xc6, 0x8c);//2_HUE	  //-12
HI351_write_cmos_sensor(0xc7, 0xa9);//2_CENTER //169
HI351_write_cmos_sensor(0xc8, 0x10);//2_DELTA  //16
HI351_write_cmos_sensor(0xc9, 0x90);//t//0x57);//3_GAIN   //87 //r4 //yellow
HI351_write_cmos_sensor(0xca, 0x20);//0219//0x03);//0x88);//3_HUE	  //-8
HI351_write_cmos_sensor(0xcb, 0x4a); //jktest 0202//74//0x52);//3_CENTER //82
HI351_write_cmos_sensor(0xcc, 0x0b); //jktest 0202//11//0x16);//3_DELTA  //22
HI351_write_cmos_sensor(0xcd, 0x80); //0219//0x90);//0x8a);//t//0x80);//4_GAIN   //128 //r5 //tree green indoor
HI351_write_cmos_sensor(0xce, 0x0c); //0219//0x65);//t//0x30);//t//0x20);//t//0x0c);//4_HUE	  //12
HI351_write_cmos_sensor(0xcf, 0x5b);//t//0x57); //jktest 0202//87//0x76);//4_CENTER //118
HI351_write_cmos_sensor(0xd0, 0x16);//t//0x0d); //jktest 0202//12//0x1c);//4_DELTA  //28
HI351_write_cmos_sensor(0xd1, 0xb2);//5_GAIN   //178 //r6 //No use
HI351_write_cmos_sensor(0xd2, 0x8a);//5_HUE	  //-10
HI351_write_cmos_sensor(0xd3, 0x52);//5_CENTER //82
HI351_write_cmos_sensor(0xd4, 0x1c);//5_DELTA  //28
//MCMC_01
HI351_write_cmos_sensor(0xd5, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0xd6, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0xd7, 0x66);//0_GAIN  //102////r1
HI351_write_cmos_sensor(0xd8, 0x00);//0_HUE   //00
HI351_write_cmos_sensor(0xd9, 0x35);//0_CENTER //53
HI351_write_cmos_sensor(0xda, 0x13);//0_DELTA //19
HI351_write_cmos_sensor(0xdb, 0x67);//1_GAIN  //103 //r2
HI351_write_cmos_sensor(0xdc, 0x04);//1_HUE	 //04
HI351_write_cmos_sensor(0xdd, 0x6e);//1_CENTER //110
HI351_write_cmos_sensor(0xde, 0x16);//jktest 0202//22//0x1c);//1_DELTA //28
HI351_write_cmos_sensor(0xdf, 0x67);//2_GAIN  //103 //r3
HI351_write_cmos_sensor(0xe0, 0x8f);//2_HUE	 //-15
HI351_write_cmos_sensor(0xe1, 0xaf);//2_CENTER //175
HI351_write_cmos_sensor(0xe2, 0x1c);//2_DELTA //28
HI351_write_cmos_sensor(0xe3, 0x9a);//3_GAIN  //154//r4
HI351_write_cmos_sensor(0xe4, 0x86);//3_HUE	 //-6
HI351_write_cmos_sensor(0xe5, 0x4a);//jktest 0202//74//0x52);//3_CENTER //82
HI351_write_cmos_sensor(0xe6, 0x0a);//jktest 0202//10//0x1c);//3_DELTA //28
HI351_write_cmos_sensor(0xe7, 0x90);//4_GAIN  //144//r5
HI351_write_cmos_sensor(0xe8, 0x20);//t//0x94);//4_HUE	 //-20
HI351_write_cmos_sensor(0xe9, 0x5b);//t//0x57);//jktest 0202//87//0x93);//4_CENTER //147
HI351_write_cmos_sensor(0xea, 0x16);//t//0x0c);//jktest 0202//12//0x1c);//4_DELTA //28
HI351_write_cmos_sensor(0xeb, 0xb2);//5_GAIN  //178//r6
HI351_write_cmos_sensor(0xec, 0x8a);//5_HUE	 //-10
HI351_write_cmos_sensor(0xed, 0x52);//5_CENTER//82
HI351_write_cmos_sensor(0xee, 0x1d);//5_DELTA //29
//MCMC_02
HI351_write_cmos_sensor(0xef, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0xf0, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0xf1, 0x66);//0_GAIN
HI351_write_cmos_sensor(0xf2, 0x00);//0_HUE
HI351_write_cmos_sensor(0xf3, 0x35);//0_CENTER
HI351_write_cmos_sensor(0xf4, 0x13);//0_DELTA
HI351_write_cmos_sensor(0xf5, 0x67);//1_GAIN
HI351_write_cmos_sensor(0xf6, 0x04);//1_HUE
HI351_write_cmos_sensor(0xf7, 0x6e);//1_CENTER
HI351_write_cmos_sensor(0xf8, 0x1c);//1_DELTA
HI351_write_cmos_sensor(0xf9, 0x67);//2_GAIN
HI351_write_cmos_sensor(0xfa, 0x92);//2_HUE
HI351_write_cmos_sensor(0xfb, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0xfc, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0xfd, 0x9a);//3_GAIN
HI351_write_cmos_sensor(0x0e, 0x00); //burst end
HI351_write_cmos_sensor(0x03, 0xd2);//Page d2
HI351_write_cmos_sensor(0x0e, 0x01); //burst start
HI351_write_cmos_sensor(0x10, 0x86);//3_HUE
HI351_write_cmos_sensor(0x11, 0x35);//jktest 0202//0x52);//3_CENTER //R4
HI351_write_cmos_sensor(0x12, 0x13);//jktest 0202//0x1c);//3_DELTA
HI351_write_cmos_sensor(0x13, 0x90);//4_GAIN
HI351_write_cmos_sensor(0x14, 0x94);//4_HUE
HI351_write_cmos_sensor(0x15, 0x5b);//t//0x6e);//jktest 0202//0x93);//4_CENTER //R5
HI351_write_cmos_sensor(0x16, 0x16);//t//0x1c);//4_DELTA
HI351_write_cmos_sensor(0x17, 0x35);//jktest 0202//0xb2);//5_GAIN
HI351_write_cmos_sensor(0x18, 0x13);//jktest 0202//0x8a);//5_HUE
HI351_write_cmos_sensor(0x19, 0x52);//5_CENTER
HI351_write_cmos_sensor(0x1a, 0x1d);//5_DELTA
//MCMC_03
HI351_write_cmos_sensor(0x1b, 0x80); //GLB_GAIN
HI351_write_cmos_sensor(0x1c, 0x00); //GLB_HUE
HI351_write_cmos_sensor(0x1d, 0x70); //0_GAIN
HI351_write_cmos_sensor(0x1e, 0x87); //0_HUE
HI351_write_cmos_sensor(0x1f, 0x36); //0_CENTER
HI351_write_cmos_sensor(0x20, 0x0d); //0_DELTA
HI351_write_cmos_sensor(0x21, 0xb0); //1_GAIN
HI351_write_cmos_sensor(0x22, 0x10); //1_HUE
HI351_write_cmos_sensor(0x23, 0x6b); //1_CENTER
HI351_write_cmos_sensor(0x24, 0x1c); //1_DELTA
HI351_write_cmos_sensor(0x25, 0x70); //2_GAIN
HI351_write_cmos_sensor(0x26, 0x00); //2_HUE
HI351_write_cmos_sensor(0x27, 0xaf); //2_CENTER
HI351_write_cmos_sensor(0x28, 0x1c); //2_DELTA
HI351_write_cmos_sensor(0x29, 0x80); //3_GAIN
HI351_write_cmos_sensor(0x2a, 0x87); //3_HUE
HI351_write_cmos_sensor(0x2b, 0x51); //3_CENTER
HI351_write_cmos_sensor(0x2c, 0x1c); //3_DELTA
HI351_write_cmos_sensor(0x2d, 0xb0); //4_GAIN
HI351_write_cmos_sensor(0x2e, 0x10); //4_HUE
HI351_write_cmos_sensor(0x2f, 0x76); //4_CENTER
HI351_write_cmos_sensor(0x30, 0x1c); //4_DELTA
HI351_write_cmos_sensor(0x31, 0x80); //5_GAIN
HI351_write_cmos_sensor(0x32, 0x00); //5_HUE
HI351_write_cmos_sensor(0x33, 0x9a); //5_CENTER
HI351_write_cmos_sensor(0x34, 0x14); //5_DELTA

//MCMC_04
HI351_write_cmos_sensor(0x35, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0x36, 0x00);//GLB_HUE
HI351_write_cmos_sensor(0x37, 0x80);//0_GAIN
HI351_write_cmos_sensor(0x38, 0x84);//0_HUE
HI351_write_cmos_sensor(0x39, 0x32);//0_CENTER
HI351_write_cmos_sensor(0x3a, 0x0f);//0_DELTA
HI351_write_cmos_sensor(0x3b, 0x90);//1_GAIN
HI351_write_cmos_sensor(0x3c, 0x14);//1_HUE
HI351_write_cmos_sensor(0x3d, 0x6a);//1_CENTER
HI351_write_cmos_sensor(0x3e, 0x14);//1_DELTA
HI351_write_cmos_sensor(0x3f, 0x70);//2_GAIN
HI351_write_cmos_sensor(0x40, 0x8e);//2_HUE
HI351_write_cmos_sensor(0x41, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0x42, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0x43, 0x9a);//3_GAIN
HI351_write_cmos_sensor(0x44, 0x00);//3_HUE
HI351_write_cmos_sensor(0x45, 0x51);//3_CENTER
HI351_write_cmos_sensor(0x46, 0x18);//3_DELTA
HI351_write_cmos_sensor(0x47, 0xa9);//4_GAIN
HI351_write_cmos_sensor(0x48, 0x10);//4_HUE
HI351_write_cmos_sensor(0x49, 0x7c);//4_CENTER
HI351_write_cmos_sensor(0x4a, 0x18);//4_DELTA
HI351_write_cmos_sensor(0x4b, 0xa0);//5_GAIN
HI351_write_cmos_sensor(0x4c, 0x00);//5_HUE
HI351_write_cmos_sensor(0x4d, 0x99);//5_CENTER
HI351_write_cmos_sensor(0x4e, 0x1e);//5_DELTA

//MCMC_05
HI351_write_cmos_sensor(0x4f, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0x50, 0x00);//GLB_HUE
HI351_write_cmos_sensor(0x51, 0x78);//0_GAIN
HI351_write_cmos_sensor(0x52, 0x08);//0_HUE
HI351_write_cmos_sensor(0x53, 0x32);//0_CENTER
HI351_write_cmos_sensor(0x54, 0x0f);//0_DELTA
HI351_write_cmos_sensor(0x55, 0x90);//1_GAIN
HI351_write_cmos_sensor(0x56, 0x0a);//1_HUE
HI351_write_cmos_sensor(0x57, 0x6a);//1_CENTER
HI351_write_cmos_sensor(0x58, 0x14);//1_DELTA
HI351_write_cmos_sensor(0x59, 0x70);//2_GAIN
HI351_write_cmos_sensor(0x5a, 0x88);//2_HUE
HI351_write_cmos_sensor(0x5b, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0x5c, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0x5d, 0x9a);//3_GAIN
HI351_write_cmos_sensor(0x5e, 0x00);//3_HUE
HI351_write_cmos_sensor(0x5f, 0x4b);//3_CENTER
HI351_write_cmos_sensor(0x60, 0x18);//3_DELTA
HI351_write_cmos_sensor(0x61, 0x90);//4_GAIN
HI351_write_cmos_sensor(0x62, 0x10);//4_HUE
HI351_write_cmos_sensor(0x63, 0x7c);//4_CENTER
HI351_write_cmos_sensor(0x64, 0x18);//4_DELTA
HI351_write_cmos_sensor(0x65, 0xa0);//5_GAIN
HI351_write_cmos_sensor(0x66, 0x00);//5_HUE
HI351_write_cmos_sensor(0x67, 0x99);//5_CENTER
HI351_write_cmos_sensor(0x68, 0x1e);//5_DELTA
//MCMC_06
HI351_write_cmos_sensor(0x69, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0x6a, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0x6b, 0x73);//0_GAIN
HI351_write_cmos_sensor(0x6c, 0x85);//0_HUE
HI351_write_cmos_sensor(0x6d, 0x35);//0_CENTER
HI351_write_cmos_sensor(0x6e, 0x1a);//0_DELTA
HI351_write_cmos_sensor(0x6f, 0x6c);//1_GAIN
HI351_write_cmos_sensor(0x70, 0x12);//1_HUE
HI351_write_cmos_sensor(0x71, 0x74);//1_CENTER
HI351_write_cmos_sensor(0x72, 0x21);//1_DELTA
HI351_write_cmos_sensor(0x73, 0x88);//2_GAIN
HI351_write_cmos_sensor(0x74, 0x8c);//2_HUE
HI351_write_cmos_sensor(0x75, 0xa9);//2_CENTER
HI351_write_cmos_sensor(0x76, 0x10);//2_DELTA
HI351_write_cmos_sensor(0x77, 0x90);//3_GAIN
HI351_write_cmos_sensor(0x78, 0x20);//3_HUE
HI351_write_cmos_sensor(0x79, 0x4a);//3_CENTER
HI351_write_cmos_sensor(0x7a, 0x0b);//3_DELTA
HI351_write_cmos_sensor(0x7b, 0x80);//4_GAIN
HI351_write_cmos_sensor(0x7c, 0x0c);//4_HUE
HI351_write_cmos_sensor(0x7d, 0x5b);//4_CENTER
HI351_write_cmos_sensor(0x7e, 0x16);//4_DELTA
HI351_write_cmos_sensor(0x7f, 0xb2);//5_GAIN
HI351_write_cmos_sensor(0x80, 0x8a);//5_HUE
HI351_write_cmos_sensor(0x81, 0x52);//5_CENTER
HI351_write_cmos_sensor(0x82, 0x1c);//5_DELTA

//MCMC_07
HI351_write_cmos_sensor(0x83, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0x84, 0x82);//GLB_HUE
HI351_write_cmos_sensor(0x85, 0x80);//0_GAIN
HI351_write_cmos_sensor(0x86, 0x84);//0_HUE
HI351_write_cmos_sensor(0x87, 0x36);//0_CENTER
HI351_write_cmos_sensor(0x88, 0x13);//0_DELTA
HI351_write_cmos_sensor(0x89, 0x80);//1_GAIN
HI351_write_cmos_sensor(0x8a, 0x0c);//1_HUE
HI351_write_cmos_sensor(0x8b, 0x62);//1_CENTER
HI351_write_cmos_sensor(0x8c, 0x16);//1_DELTA
HI351_write_cmos_sensor(0x8d, 0x73);//2_GAIN
HI351_write_cmos_sensor(0x8e, 0x8a);//2_HUE
HI351_write_cmos_sensor(0x8f, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0x90, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0x91, 0x80);//3_GAIN
HI351_write_cmos_sensor(0x92, 0x86);//3_HUE
HI351_write_cmos_sensor(0x93, 0x4a);//3_CENTER
HI351_write_cmos_sensor(0x94, 0x0a);//3_DELTA
HI351_write_cmos_sensor(0x95, 0x80);//4_GAIN
HI351_write_cmos_sensor(0x96, 0x0c);//4_HUE
HI351_write_cmos_sensor(0x97, 0x5b);//4_CENTER
HI351_write_cmos_sensor(0x98, 0x16);//4_DELTA
HI351_write_cmos_sensor(0x99, 0xb2);//5_GAIN
HI351_write_cmos_sensor(0x9a, 0x8a);//5_HUE
HI351_write_cmos_sensor(0x9b, 0x52);//5_CENTER
HI351_write_cmos_sensor(0x9c, 0x1d);//5_DELTA
//MCMC_08
HI351_write_cmos_sensor(0x9d, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0x9e, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0x9f, 0x80);//0_GAIN
HI351_write_cmos_sensor(0xa0, 0x04);//0_HUE
HI351_write_cmos_sensor(0xa1, 0x36);//0_CENTER
HI351_write_cmos_sensor(0xa2, 0x13);//0_DELTA
HI351_write_cmos_sensor(0xa3, 0x80);//1_GAIN
HI351_write_cmos_sensor(0xa4, 0x0c);//1_HUE
HI351_write_cmos_sensor(0xa5, 0x62);//1_CENTER
HI351_write_cmos_sensor(0xa6, 0x10);//1_DELTA
HI351_write_cmos_sensor(0xa7, 0x73);//2_GAIN
HI351_write_cmos_sensor(0xa8, 0x8a);//2_HUE
HI351_write_cmos_sensor(0xa9, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0xaa, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0xab, 0x80);//3_GAIN
HI351_write_cmos_sensor(0xac, 0x86);//3_HUE
HI351_write_cmos_sensor(0xad, 0x35);//jktest 0202//0x51);//3_CENTER
HI351_write_cmos_sensor(0xae, 0x13);//jktest 0202//0x14);//3_DELTA
HI351_write_cmos_sensor(0xaf, 0x80);//4_GAIN
HI351_write_cmos_sensor(0xb0, 0x86);//jktest 0202//0x0c);//4_HUE
HI351_write_cmos_sensor(0xb1, 0x5b);//0x6e);//jktest 0202//0x76);//4_CENTER
HI351_write_cmos_sensor(0xb2, 0x16);//0x1c);//4_DELTA
HI351_write_cmos_sensor(0xb3, 0xb2);//5_GAIN
HI351_write_cmos_sensor(0xb4, 0x8a);//5_HUE
HI351_write_cmos_sensor(0xb5, 0x52);//5_CENTER
HI351_write_cmos_sensor(0xb6, 0x1d);//5_DELTA
//MCMC_09
HI351_write_cmos_sensor(0xb7, 0x80);//GLB_GAIN                                  HI351_write_cmos_sensor(0xb7, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0xb8, 0x01);//GLB_HUE                                   HI351_write_cmos_sensor(0xb8, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0xb9, 0x73);//0_GAIN   //115 //R1                       HI351_write_cmos_sensor(0xb9, 0x80);//0_GAIN
HI351_write_cmos_sensor(0xba, 0x81);//0_HUE	  //-1                             HI351_write_cmos_sensor(0xba, 0x04);//0_HUE
HI351_write_cmos_sensor(0xbb, 0x35);//0_CENTER //53                             HI351_write_cmos_sensor(0xbb, 0x36);//0_CENTER
HI351_write_cmos_sensor(0xbc, 0x1a);//0_DELTA  //26                             HI351_write_cmos_sensor(0xbc, 0x13);//0_DELTA
HI351_write_cmos_sensor(0xbd, 0x6c);//1_GAIN   //108 //r2                       HI351_write_cmos_sensor(0xbd, 0x80);//1_GAIN
HI351_write_cmos_sensor(0xbe, 0x12);//1_HUE	  //+18                            HI351_write_cmos_sensor(0xbe, 0x0c);//1_HUE
HI351_write_cmos_sensor(0xbf, 0x74); //jktest 0202//116//0x76);//1_CENTER //118 HI351_write_cmos_sensor(0xbf, 0x62);//1_CENTER
HI351_write_cmos_sensor(0xc0, 0x21); //jktest 0202//33//0x34);//1_DELTA  //52   HI351_write_cmos_sensor(0xc0, 0x10);//1_DELTA
HI351_write_cmos_sensor(0xc1, 0x88);//2_GAIN   //136 //r3                       HI351_write_cmos_sensor(0xc1, 0x73);//2_GAIN
HI351_write_cmos_sensor(0xc2, 0x8c);//2_HUE	  //-12                            HI351_write_cmos_sensor(0xc2, 0x8a);//2_HUE
HI351_write_cmos_sensor(0xc3, 0xa9);//2_CENTER //169                            HI351_write_cmos_sensor(0xc3, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0xc4, 0x10);//2_DELTA  //16                             HI351_write_cmos_sensor(0xc4, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0xc5, 0x90);//t//0x57);//3_GAIN   //87 //r4                        HI351_write_cmos_sensor(0xc5, 0x80);//3_GAIN
HI351_write_cmos_sensor(0xc6, 0x20);//0219//0x03);//t//0x88);//3_HUE	  //-8                             HI351_write_cmos_sensor(0xc6, 0x86);//3_HUE
HI351_write_cmos_sensor(0xc7, 0x4a); //jktest 0202//74//0x52);//3_CENTER //82   HI351_write_cmos_sensor(0xc7, 0x51);//3_CENTER
HI351_write_cmos_sensor(0xc8, 0x0b); //jktest 0202//11//0x16);//3_DELTA  //22   HI351_write_cmos_sensor(0xc8, 0x14);//3_DELTA
HI351_write_cmos_sensor(0xc9, 0x80);//0219//0x90);//t//0x80);//4_GAIN   //128 //r5                       HI351_write_cmos_sensor(0xc9, 0x80);//4_GAIN
HI351_write_cmos_sensor(0xca, 0x0c);//0219//0x65);//t//0x30);//t//0x0c);//4_HUE	  //12                             HI351_write_cmos_sensor(0xca, 0x0c);//4_HUE
HI351_write_cmos_sensor(0xcb, 0x5b);//t//0x57); //jktest 0202//87//0x76);//4_CENTER //118  HI351_write_cmos_sensor(0xcb, 0x76);//4_CENTER
HI351_write_cmos_sensor(0xcc, 0x16);//t//0x0d); //jktest 0202//12//0x1c);//4_DELTA  //28   HI351_write_cmos_sensor(0xcc, 0x1c);//4_DELTA
HI351_write_cmos_sensor(0xcd, 0xb2);//5_GAIN   //178 //r6                       HI351_write_cmos_sensor(0xcd, 0xb2);//5_GAIN
HI351_write_cmos_sensor(0xce, 0x8a);//5_HUE	  //-10                            HI351_write_cmos_sensor(0xce, 0x8a);//5_HUE
HI351_write_cmos_sensor(0xcf, 0x52);//5_CENTER //82                             HI351_write_cmos_sensor(0xcf, 0x52);//5_CENTER
HI351_write_cmos_sensor(0xd0, 0x1c);//5_DELTA  //28                             HI351_write_cmos_sensor(0xd0, 0x1d);//5_DELTA
//MCMC_10
HI351_write_cmos_sensor(0xd1, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0xd2, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0xd3, 0x80);//0_GAIN
HI351_write_cmos_sensor(0xd4, 0x04);//0_HUE
HI351_write_cmos_sensor(0xd5, 0x36);//0_CENTER
HI351_write_cmos_sensor(0xd6, 0x13);//0_DELTA
HI351_write_cmos_sensor(0xd7, 0x80);//1_GAIN
HI351_write_cmos_sensor(0xd8, 0x0c);//1_HUE
HI351_write_cmos_sensor(0xd9, 0x62);//1_CENTER
HI351_write_cmos_sensor(0xda, 0x10);//1_DELTA
HI351_write_cmos_sensor(0xdb, 0x73);//2_GAIN
HI351_write_cmos_sensor(0xdc, 0x8a);//2_HUE
HI351_write_cmos_sensor(0xdd, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0xde, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0xdf, 0x80);//3_GAIN
HI351_write_cmos_sensor(0xe0, 0x86);//3_HUE
HI351_write_cmos_sensor(0xe1, 0x4a);//jktest 0202//74//0x51);//3_CENTER
HI351_write_cmos_sensor(0xe2, 0x0a);//jktest 0202//10//0x14);//3_DELTA
HI351_write_cmos_sensor(0xe3, 0x80);//4_GAIN
HI351_write_cmos_sensor(0xe4, 0x0c);//4_HUE
HI351_write_cmos_sensor(0xe5, 0x5b);//t//0x57);//jktest 0202//87//0x76);//4_CENTER
HI351_write_cmos_sensor(0xe6, 0x16);//t//0x0c);//jktest 0202//12//0x1c);//4_DELTA
HI351_write_cmos_sensor(0xe7, 0xb2);//5_GAIN
HI351_write_cmos_sensor(0xe8, 0x8a);//5_HUE
HI351_write_cmos_sensor(0xe9, 0x52);//5_CENTER
HI351_write_cmos_sensor(0xea, 0x1d);//5_DELTA
//MCMC_11
HI351_write_cmos_sensor(0xeb, 0x80);//GLB_GAIN
HI351_write_cmos_sensor(0xec, 0x01);//GLB_HUE
HI351_write_cmos_sensor(0xed, 0x80);//0_GAIN
HI351_write_cmos_sensor(0xee, 0x04);//0_HUE
HI351_write_cmos_sensor(0xef, 0x36);//0_CENTER
HI351_write_cmos_sensor(0xf0, 0x13);//0_DELTA
HI351_write_cmos_sensor(0xf1, 0x80);//1_GAIN
HI351_write_cmos_sensor(0xf2, 0x0c);//1_HUE
HI351_write_cmos_sensor(0xf3, 0x62);//1_CENTER
HI351_write_cmos_sensor(0xf4, 0x10);//1_DELTA
HI351_write_cmos_sensor(0xf5, 0x73);//2_GAIN
HI351_write_cmos_sensor(0xf6, 0x8a);//2_HUE
HI351_write_cmos_sensor(0xf7, 0xaf);//2_CENTER
HI351_write_cmos_sensor(0xf8, 0x1c);//2_DELTA
HI351_write_cmos_sensor(0xf9, 0x80);//3_GAIN
HI351_write_cmos_sensor(0xfa, 0x86);//3_HUE
HI351_write_cmos_sensor(0xfb, 0x35);//jktest 0202//0x51);//3_CENTER
HI351_write_cmos_sensor(0xfc, 0x13);//jktest 0202//0x14);//3_DELTA
HI351_write_cmos_sensor(0xfd, 0x80);//4_GAIN
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
HI351_write_cmos_sensor(0x03, 0xd3);//Page d3
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x86);//4_HUE
HI351_write_cmos_sensor(0x11, 0x5b);//t//0x6e);//jktest 0202//0x76);//4_CENTER
HI351_write_cmos_sensor(0x12, 0x16);//t//0x1c);//4_DELTA
HI351_write_cmos_sensor(0x13, 0xb2);//5_GAIN
HI351_write_cmos_sensor(0x14, 0x8a);//5_HUE
HI351_write_cmos_sensor(0x15, 0x52);//5_CENTER
HI351_write_cmos_sensor(0x16, 0x1d);//5_DELTA

///////////////////////////////////////////
// D3 Page Adaptive LSC
///////////////////////////////////////////

HI351_write_cmos_sensor(0x17, 0x00); //LSC 00 ofs GB //00
HI351_write_cmos_sensor(0x18, 0x00); //LSC 00 ofs B
HI351_write_cmos_sensor(0x19, 0x00); //LSC 00 ofs R
HI351_write_cmos_sensor(0x1a, 0x00); //LSC 00 ofs GR

HI351_write_cmos_sensor(0x1b, 0x40); //0x40); //28set//0x20);//LSC 00 Gain GB
HI351_write_cmos_sensor(0x1c, 0x40); //0x40); //28set//0x20);//LSC 00 Gain B
HI351_write_cmos_sensor(0x1d, 0x40); //0x45); //28set//0x14); //LSC 00 Gain R
HI351_write_cmos_sensor(0x1e, 0x40); //0x40); //28set//0x20);//LSC 00 Gain GR

HI351_write_cmos_sensor(0x1f, 0x00); //LSC 01 ofs GB //01
HI351_write_cmos_sensor(0x20, 0x00); //LSC 01 ofs B
HI351_write_cmos_sensor(0x21, 0x00); //LSC 01 ofs R
HI351_write_cmos_sensor(0x22, 0x00); //LSC 01 ofs GR
HI351_write_cmos_sensor(0x23, 0x40); //0x40); //28set//0x20);//LSC 01 Gain GB
HI351_write_cmos_sensor(0x24, 0x40); //0x40); //28set//0x20);//LSC 01 Gain B
HI351_write_cmos_sensor(0x25, 0x40); //0x45); //28set//0x14); //LSC 01 Gain R
HI351_write_cmos_sensor(0x26, 0x40); //0x40); //28set//0x20);//LSC 01 Gain GR

HI351_write_cmos_sensor(0x27, 0x00); //LSC 02 ofs GB //02
HI351_write_cmos_sensor(0x28, 0x00); //LSC 02 ofs B
HI351_write_cmos_sensor(0x29, 0x00); //LSC 02 ofs R
HI351_write_cmos_sensor(0x2a, 0x00); //LSC 02 ofs GR
HI351_write_cmos_sensor(0x2b, 0x40); //0x40); //28set//0x20);//LSC 02 Gain GB
HI351_write_cmos_sensor(0x2c, 0x40); //0x40); //28set//0x20);//LSC 02 Gain B
HI351_write_cmos_sensor(0x2d, 0x40); //0x45); //28set//0x20); //LSC 02 Gain R
HI351_write_cmos_sensor(0x2e, 0x40); //0x40); //28set//0x20);//LSC 02 Gain GR

HI351_write_cmos_sensor(0x2f, 0x00); //LSC 03 ofs GB //03
HI351_write_cmos_sensor(0x30, 0x00); //LSC 03 ofs B
HI351_write_cmos_sensor(0x31, 0x00); //0308//0x04); //LSC 03 ofs R
HI351_write_cmos_sensor(0x32, 0x00); //LSC 03 ofs GR
HI351_write_cmos_sensor(0x33, 0x80); //LSC 03 Gain GB
HI351_write_cmos_sensor(0x34, 0x78); //LSC 03 Gain B
HI351_write_cmos_sensor(0x35, 0x78); //0308//0x34); //LSC 03 Gain R
HI351_write_cmos_sensor(0x36, 0x80); //LSC 03 Gain GR

HI351_write_cmos_sensor(0x37, 0x00); //LSC 04 ofs GB //04
HI351_write_cmos_sensor(0x38, 0x00); //LSC 04 ofs B
HI351_write_cmos_sensor(0x39, 0x00); //0308//0x04); //LSC 04 ofs R
HI351_write_cmos_sensor(0x3a, 0x00); //LSC 04 ofs GR
HI351_write_cmos_sensor(0x3b, 0x80); //LSC 04 Gain GB
HI351_write_cmos_sensor(0x3c, 0x78); //LSC 04 Gain B
HI351_write_cmos_sensor(0x3d, 0x78); //0308//0x34); //LSC 04 Gain R
HI351_write_cmos_sensor(0x3e, 0x80); //LSC 04 Gain GR

HI351_write_cmos_sensor(0x3f, 0x00); //LSC 05 ofs GB //05
HI351_write_cmos_sensor(0x40, 0x00); //LSC 05 ofs B
HI351_write_cmos_sensor(0x41, 0x00); //LSC 05 ofs R
HI351_write_cmos_sensor(0x42, 0x00); //LSC 05 ofs GR
HI351_write_cmos_sensor(0x43, 0x80); //LSC 05 Gain GB
HI351_write_cmos_sensor(0x44, 0x78); //LSC 05 Gain B
HI351_write_cmos_sensor(0x45, 0x78); //LSC 05 Gain R
HI351_write_cmos_sensor(0x46, 0x80);//LSC 05 Gain GR

HI351_write_cmos_sensor(0x47, 0x00); //LSC 06 ofs GB //06
HI351_write_cmos_sensor(0x48, 0x00); //LSC 06 ofs B
HI351_write_cmos_sensor(0x49, 0x00); //0308//0x04); //LSC 06 ofs R
HI351_write_cmos_sensor(0x4a, 0x00); //LSC 06 ofs GR
HI351_write_cmos_sensor(0x4b, 0x60);//78 LSC 06 Gain GB
HI351_write_cmos_sensor(0x4c, 0x60);//7c LSC 06 Gain B
HI351_write_cmos_sensor(0x4d, 0x60); //0308//0x34);//80 LSC 06 Gain R
HI351_write_cmos_sensor(0x4e, 0x60);//78 LSC 06 Gain GR

HI351_write_cmos_sensor(0x4f, 0x00); //LSC 07 ofs GB //07
HI351_write_cmos_sensor(0x50, 0x00); //LSC 07 ofs B
HI351_write_cmos_sensor(0x51, 0x00); //0308//0x04); //LSC 07 ofs R
HI351_write_cmos_sensor(0x52, 0x00); //LSC 07 ofs GR
HI351_write_cmos_sensor(0x53, 0x60);//78 LSC 07 Gain GB
HI351_write_cmos_sensor(0x54, 0x60);//7c LSC 07 Gain B
HI351_write_cmos_sensor(0x55, 0x60); //0308//0x34);//80 LSC 07 Gain R
HI351_write_cmos_sensor(0x56, 0x60);//78 LSC 07 Gain GR

HI351_write_cmos_sensor(0x57, 0x00); //LSC 08 ofs GB //08
HI351_write_cmos_sensor(0x58, 0x00); //LSC 08 ofs B
HI351_write_cmos_sensor(0x59, 0x00); //LSC 08 ofs R
HI351_write_cmos_sensor(0x5a, 0x00); //LSC 08 ofs GR
HI351_write_cmos_sensor(0x5b, 0x60); //78 LSC 08 Gain GB
HI351_write_cmos_sensor(0x5c, 0x60); //7c LSC 08 Gain B
HI351_write_cmos_sensor(0x5d, 0x70); //80 LSC 08 Gain R
HI351_write_cmos_sensor(0x5e, 0x60); //78 LSC 08 Gain GR

HI351_write_cmos_sensor(0x5f, 0x00); //LSC 09 ofs GB //09
HI351_write_cmos_sensor(0x60, 0x00); //LSC 09 ofs B
HI351_write_cmos_sensor(0x61, 0x00); //LSC 09 ofs R
HI351_write_cmos_sensor(0x62, 0x00); //LSC 09 ofs GR
HI351_write_cmos_sensor(0x63, 0xa0); //78 LSC 09 Gain GB
HI351_write_cmos_sensor(0x64, 0xa0); //7c LSC 09 Gain B
HI351_write_cmos_sensor(0x65, 0xa0); //80 LSC 09 Gain R
HI351_write_cmos_sensor(0x66, 0xa0); //78 LSC 09 Gain GR

HI351_write_cmos_sensor(0x67, 0x00); //LSC 10 ofs GB //10
HI351_write_cmos_sensor(0x68, 0x00); //LSC 10 ofs B
HI351_write_cmos_sensor(0x69, 0x00); //LSC 10 ofs R
HI351_write_cmos_sensor(0x6a, 0x00); //LSC 10 ofs GR
HI351_write_cmos_sensor(0x6b, 0xa0); //78 LSC 10 Gain GB
HI351_write_cmos_sensor(0x6c, 0xa0); //7c LSC 10 Gain B
HI351_write_cmos_sensor(0x6d, 0xa0); //80 LSC 10 Gain R
HI351_write_cmos_sensor(0x6e, 0xa0); //78 LSC 10 Gain GR

HI351_write_cmos_sensor(0x6f, 0x00); //LSC 11 ofs GB //11
HI351_write_cmos_sensor(0x70, 0x00); //LSC 11 ofs B
HI351_write_cmos_sensor(0x71, 0x00); //LSC 11 ofs R
HI351_write_cmos_sensor(0x72, 0x00); //LSC 11 ofs GR
HI351_write_cmos_sensor(0x73, 0xa0); //78 LSC 11 Gain GB
HI351_write_cmos_sensor(0x74, 0xa0); //7c LSC 11 Gain B
HI351_write_cmos_sensor(0x75, 0xa0); //80 LSC 11 Gain R
HI351_write_cmos_sensor(0x76, 0xa0); //78 LSC 11 Gain GR
///////////////////////////////////////////
// D3 Page OTP, ROM Select TH
///////////////////////////////////////////
HI351_write_cmos_sensor(0x77, 0x60); //2 ROM High
HI351_write_cmos_sensor(0x78, 0x20); //2 ROM Low
HI351_write_cmos_sensor(0x79, 0x60); //3 OTP High
HI351_write_cmos_sensor(0x7a, 0x40); //3 OTP Mid
HI351_write_cmos_sensor(0x7b, 0x20); //3 OTP Low
///////////////////////////////////////////
// D3 Page Adaptive DNP
///////////////////////////////////////////
HI351_write_cmos_sensor(0x7c, 0x00); //LSC EV max
HI351_write_cmos_sensor(0x7d, 0x00);
HI351_write_cmos_sensor(0x7e, 0x07);
HI351_write_cmos_sensor(0x7f, 0xf1);

HI351_write_cmos_sensor(0x80, 0x00); //LSC EV min
HI351_write_cmos_sensor(0x81, 0x00);
HI351_write_cmos_sensor(0x82, 0x07);
HI351_write_cmos_sensor(0x83, 0xf1);
HI351_write_cmos_sensor(0x84, 0x20); //CTEM max
HI351_write_cmos_sensor(0x85, 0x20); //CTEM min
HI351_write_cmos_sensor(0x86, 0x20); //Y STD max
HI351_write_cmos_sensor(0x87, 0x20); //Y STD min

HI351_write_cmos_sensor(0x88, 0x00); //LSC offset
HI351_write_cmos_sensor(0x89, 0x00);
HI351_write_cmos_sensor(0x8a, 0x00);
HI351_write_cmos_sensor(0x8b, 0x00);
HI351_write_cmos_sensor(0x8c, 0x80); //LSC gain
HI351_write_cmos_sensor(0x8d, 0x80);
HI351_write_cmos_sensor(0x8e, 0x80);
HI351_write_cmos_sensor(0x8f, 0x80);

HI351_write_cmos_sensor(0x90, 0x80); //DNP CB
HI351_write_cmos_sensor(0x91, 0x80); //DNP CR
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
///////////////////////////////////
//Page 0xD9 DMA EXTRA
///////////////////////////////////

HI351_write_cmos_sensor(0x03, 0xd9);
HI351_write_cmos_sensor(0x0e, 0x01); //BURST_START
HI351_write_cmos_sensor(0x10, 0x03);
HI351_write_cmos_sensor(0x11, 0x10);//Page 10
HI351_write_cmos_sensor(0x12, 0x61);//sat B
HI351_write_cmos_sensor(0x13, 0x80);
HI351_write_cmos_sensor(0x14, 0x62);//sat R
HI351_write_cmos_sensor(0x15, 0x80);
HI351_write_cmos_sensor(0x16, 0x03); //jktest 0131//0x40); //dyoffset
HI351_write_cmos_sensor(0x17, 0x00); //0x05); //17th//0x07); //0220 test dyoffset//0x00);

HI351_write_cmos_sensor(0x18, 0x03); //jktest 0131//0x40); //dyoffset
HI351_write_cmos_sensor(0x19, 0x00); //0x05); //0x00);
HI351_write_cmos_sensor(0x1a, 0x03);//Page 16 CMC
HI351_write_cmos_sensor(0x1b, 0x16);
HI351_write_cmos_sensor(0x1c, 0x30);//CMC11
HI351_write_cmos_sensor(0x1d, 0x7f);
HI351_write_cmos_sensor(0x1e, 0x31);//CMC12
HI351_write_cmos_sensor(0x1f, 0x42);

HI351_write_cmos_sensor(0x20, 0x32);//CMC13
HI351_write_cmos_sensor(0x21, 0x03);
HI351_write_cmos_sensor(0x22, 0x33);//CMC21
HI351_write_cmos_sensor(0x23, 0x22);
HI351_write_cmos_sensor(0x24, 0x34);//CMC22
HI351_write_cmos_sensor(0x25, 0x7b);
HI351_write_cmos_sensor(0x26, 0x35);//CMC23
HI351_write_cmos_sensor(0x27, 0x19);
HI351_write_cmos_sensor(0x28, 0x36);//CMC31
HI351_write_cmos_sensor(0x29, 0x01);
HI351_write_cmos_sensor(0x2a, 0x37);//CMC32
HI351_write_cmos_sensor(0x2b, 0x43);
HI351_write_cmos_sensor(0x2c, 0x38);//CMC33
HI351_write_cmos_sensor(0x2d, 0x84);
HI351_write_cmos_sensor(0x2e, 0x70);//GLBSATGAIN
HI351_write_cmos_sensor(0x2f, 0x80);

HI351_write_cmos_sensor(0x30, 0x71);//GLBROT
HI351_write_cmos_sensor(0x31, 0x00);
HI351_write_cmos_sensor(0x32, 0x72);//R1SATGAIN
HI351_write_cmos_sensor(0x33, 0x9b);
HI351_write_cmos_sensor(0x34, 0x73);//R1ROT
HI351_write_cmos_sensor(0x35, 0x05);
HI351_write_cmos_sensor(0x36, 0x74);//R1CENTERANG
HI351_write_cmos_sensor(0x37, 0x34);

HI351_write_cmos_sensor(0x38, 0x75);//R1DELTAANG
HI351_write_cmos_sensor(0x39, 0x1e);
HI351_write_cmos_sensor(0x3a, 0x76);//R2SATGAIN
HI351_write_cmos_sensor(0x3b, 0xa6);
HI351_write_cmos_sensor(0x3c, 0x77);//R2ROT
HI351_write_cmos_sensor(0x3d, 0x10);
HI351_write_cmos_sensor(0x3e, 0x78);//R2CENTERANG
HI351_write_cmos_sensor(0x3f, 0x69);

HI351_write_cmos_sensor(0x40, 0x79);//R2DELTAANG
HI351_write_cmos_sensor(0x41, 0x1e);
HI351_write_cmos_sensor(0x42, 0x7a);//R3SATGAIN
HI351_write_cmos_sensor(0x43, 0x80);
HI351_write_cmos_sensor(0x44, 0x7b);//R3ROT
HI351_write_cmos_sensor(0x45, 0x80);
HI351_write_cmos_sensor(0x46, 0x7c);//R3CENTERANG
HI351_write_cmos_sensor(0x47, 0xad);

HI351_write_cmos_sensor(0x48, 0x7d);//R3DELTAANG
HI351_write_cmos_sensor(0x49, 0x1e);
HI351_write_cmos_sensor(0x4a, 0x7e);//R4SATGAIN
HI351_write_cmos_sensor(0x4b, 0x98);
HI351_write_cmos_sensor(0x4c, 0x7f);//R4ROT
HI351_write_cmos_sensor(0x4d, 0x80);
HI351_write_cmos_sensor(0x4e, 0x80);//R4CENTERANG
HI351_write_cmos_sensor(0x4f, 0x51);

HI351_write_cmos_sensor(0x50, 0x81);//R4DELTANAG
HI351_write_cmos_sensor(0x51, 0x1e);
HI351_write_cmos_sensor(0x52, 0x82);//R5SATGAIN
HI351_write_cmos_sensor(0x53, 0x80);
HI351_write_cmos_sensor(0x54, 0x83);//R5ROT
HI351_write_cmos_sensor(0x55, 0x0c);
HI351_write_cmos_sensor(0x56, 0x84);//R5CENTERANG
HI351_write_cmos_sensor(0x57, 0x23);

HI351_write_cmos_sensor(0x58, 0x85);//R5DELTAANG
HI351_write_cmos_sensor(0x59, 0x1e);
HI351_write_cmos_sensor(0x5a, 0x86);//R6SATGAIN
HI351_write_cmos_sensor(0x5b, 0xb3);
HI351_write_cmos_sensor(0x5c, 0x87);//R6ROT
HI351_write_cmos_sensor(0x5d, 0x8a);
HI351_write_cmos_sensor(0x5e, 0x88);//R6CENTERANG
HI351_write_cmos_sensor(0x5f, 0x52);

HI351_write_cmos_sensor(0x60, 0x89);//R6DELTAANG
HI351_write_cmos_sensor(0x61, 0x1e);
HI351_write_cmos_sensor(0x62, 0x03);//Page 17 Gamma
HI351_write_cmos_sensor(0x63, 0x17);
HI351_write_cmos_sensor(0x64, 0x20);//GMMA0
HI351_write_cmos_sensor(0x65, 0x00);
HI351_write_cmos_sensor(0x66, 0x21);
HI351_write_cmos_sensor(0x67, 0x02);

HI351_write_cmos_sensor(0x68, 0x22);
HI351_write_cmos_sensor(0x69, 0x04);
HI351_write_cmos_sensor(0x6a, 0x23);
HI351_write_cmos_sensor(0x6b, 0x09);
HI351_write_cmos_sensor(0x6c, 0x24);
HI351_write_cmos_sensor(0x6d, 0x12);
HI351_write_cmos_sensor(0x6e, 0x25);
HI351_write_cmos_sensor(0x6f, 0x23);

HI351_write_cmos_sensor(0x70, 0x26);//GMMA6
HI351_write_cmos_sensor(0x71, 0x37);
HI351_write_cmos_sensor(0x72, 0x27);
HI351_write_cmos_sensor(0x73, 0x47);
HI351_write_cmos_sensor(0x74, 0x28);
HI351_write_cmos_sensor(0x75, 0x57);
HI351_write_cmos_sensor(0x76, 0x29);
HI351_write_cmos_sensor(0x77, 0x61);

HI351_write_cmos_sensor(0x78, 0x2a);
HI351_write_cmos_sensor(0x79, 0x6b);
HI351_write_cmos_sensor(0x7a, 0x2b);
HI351_write_cmos_sensor(0x7b, 0x71);
HI351_write_cmos_sensor(0x7c, 0x2c);
HI351_write_cmos_sensor(0x7d, 0x76);
HI351_write_cmos_sensor(0x7e, 0x2d);//GMMA13
HI351_write_cmos_sensor(0x7f, 0x7a);

HI351_write_cmos_sensor(0x80, 0x2e);
HI351_write_cmos_sensor(0x81, 0x7f);
HI351_write_cmos_sensor(0x82, 0x2f);
HI351_write_cmos_sensor(0x83, 0x84);
HI351_write_cmos_sensor(0x84, 0x30);
HI351_write_cmos_sensor(0x85, 0x88);
HI351_write_cmos_sensor(0x86, 0x31);
HI351_write_cmos_sensor(0x87, 0x8c);

HI351_write_cmos_sensor(0x88, 0x32);
HI351_write_cmos_sensor(0x89, 0x91);
HI351_write_cmos_sensor(0x8a, 0x33);
HI351_write_cmos_sensor(0x8b, 0x94);
HI351_write_cmos_sensor(0x8c, 0x34);
HI351_write_cmos_sensor(0x8d, 0x98);
HI351_write_cmos_sensor(0x8e, 0x35);
HI351_write_cmos_sensor(0x8f, 0x9f);

HI351_write_cmos_sensor(0x90, 0x36);
HI351_write_cmos_sensor(0x91, 0xa6);
HI351_write_cmos_sensor(0x92, 0x37);
HI351_write_cmos_sensor(0x93, 0xae);
HI351_write_cmos_sensor(0x94, 0x38);
HI351_write_cmos_sensor(0x95, 0xbb);
HI351_write_cmos_sensor(0x96, 0x39);
HI351_write_cmos_sensor(0x97, 0xc9);

HI351_write_cmos_sensor(0x98, 0x3a);
HI351_write_cmos_sensor(0x99, 0xd3);
HI351_write_cmos_sensor(0x9a, 0x3b);
HI351_write_cmos_sensor(0x9b, 0xdc);
HI351_write_cmos_sensor(0x9c, 0x3c);
HI351_write_cmos_sensor(0x9d, 0xe2);
HI351_write_cmos_sensor(0x9e, 0x3d);
HI351_write_cmos_sensor(0x9f, 0xe8);

HI351_write_cmos_sensor(0xa0, 0x3e);
HI351_write_cmos_sensor(0xa1, 0xed);
HI351_write_cmos_sensor(0xa2, 0x3f);
HI351_write_cmos_sensor(0xa3, 0xf4);
HI351_write_cmos_sensor(0xa4, 0x40);
HI351_write_cmos_sensor(0xa5, 0xfa);
HI351_write_cmos_sensor(0xa6, 0x41);//GMMA33
HI351_write_cmos_sensor(0xa7, 0xff);

HI351_write_cmos_sensor(0xa8, 0x03);//page 20 AE
HI351_write_cmos_sensor(0xa9, 0x20);
HI351_write_cmos_sensor(0xaa, 0x39);//??
HI351_write_cmos_sensor(0xab, 0x40);
HI351_write_cmos_sensor(0xac, 0x03);//Page 15 SHD
HI351_write_cmos_sensor(0xad, 0x15);
HI351_write_cmos_sensor(0xae, 0x24);//LSC OFFSET GB
HI351_write_cmos_sensor(0xaf, 0x00);

HI351_write_cmos_sensor(0xb0, 0x25);//LSC OFFSET B
HI351_write_cmos_sensor(0xb1, 0x00);
HI351_write_cmos_sensor(0xb2, 0x26);//LSC OFFSET R
HI351_write_cmos_sensor(0xb3, 0x00);
HI351_write_cmos_sensor(0xb4, 0x27);//LSC OFFSET GR
HI351_write_cmos_sensor(0xb5, 0x00);
HI351_write_cmos_sensor(0xb6, 0x28);//LSC GAIN GB
HI351_write_cmos_sensor(0xb7, 0x80);

HI351_write_cmos_sensor(0xb8, 0x29);//LSC GAIN B
HI351_write_cmos_sensor(0xb9, 0x80);
HI351_write_cmos_sensor(0xba, 0x2a);//LSC GAIN R
HI351_write_cmos_sensor(0xbb, 0x7a);
HI351_write_cmos_sensor(0xbc, 0x2b);//LSC GAIN GR
HI351_write_cmos_sensor(0xbd, 0x80);
HI351_write_cmos_sensor(0xbe, 0x11);//LSC CTL2
HI351_write_cmos_sensor(0xbf, 0x40);
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
///////////////////////////////////
// Page 0xDA(DMA Outdoor)
///////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xda);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x03);
HI351_write_cmos_sensor(0x11, 0x11);
HI351_write_cmos_sensor(0x12, 0x10);
HI351_write_cmos_sensor(0x13, 0x13); //Outdoor 1110
HI351_write_cmos_sensor(0x14, 0x11);
HI351_write_cmos_sensor(0x15, 0x08); //Outdoor 1111
HI351_write_cmos_sensor(0x16, 0x12);
HI351_write_cmos_sensor(0x17, 0x1e); //Outdoor 1112
HI351_write_cmos_sensor(0x18, 0x13);
HI351_write_cmos_sensor(0x19, 0x0e); //Outdoor 1113
HI351_write_cmos_sensor(0x1a, 0x14);
HI351_write_cmos_sensor(0x1b, 0x31); //Outdoor 1114
HI351_write_cmos_sensor(0x1c, 0x30);
HI351_write_cmos_sensor(0x1d, 0x20); //Outdoor 1130
HI351_write_cmos_sensor(0x1e, 0x31);
HI351_write_cmos_sensor(0x1f, 0x20); //Outdoor 1131
HI351_write_cmos_sensor(0x20, 0x32);
HI351_write_cmos_sensor(0x21, 0x52); //Outdoor 1132
HI351_write_cmos_sensor(0x22, 0x33);
HI351_write_cmos_sensor(0x23, 0x3b); //Outdoor 1133
HI351_write_cmos_sensor(0x24, 0x34);
HI351_write_cmos_sensor(0x25, 0x1d); //Outdoor 1134
HI351_write_cmos_sensor(0x26, 0x35);
HI351_write_cmos_sensor(0x27, 0x21); //Outdoor 1135
HI351_write_cmos_sensor(0x28, 0x36);
HI351_write_cmos_sensor(0x29, 0x1b); //Outdoor 1136
HI351_write_cmos_sensor(0x2a, 0x37);
HI351_write_cmos_sensor(0x2b, 0x21); //Outdoor 1137
HI351_write_cmos_sensor(0x2c, 0x38);
HI351_write_cmos_sensor(0x2d, 0x18); //Outdoor 1138
HI351_write_cmos_sensor(0x2e, 0x39);
HI351_write_cmos_sensor(0x2f, 0x18); //Outdoor 1139 R2 lvl1 gain
HI351_write_cmos_sensor(0x30, 0x3a);
HI351_write_cmos_sensor(0x31, 0x18); //Outdoor 113a R2 lvl2 gain
HI351_write_cmos_sensor(0x32, 0x3b);
HI351_write_cmos_sensor(0x33, 0x18); //Outdoor 113b R2 lvl3 gain
HI351_write_cmos_sensor(0x34, 0x3c);
HI351_write_cmos_sensor(0x35, 0x1a); //Outdoor 113c R2 lvl4 gain
HI351_write_cmos_sensor(0x36, 0x3d);
HI351_write_cmos_sensor(0x37, 0x1a); //Outdoor 113d R2 lvl5 gain
HI351_write_cmos_sensor(0x38, 0x3e);
HI351_write_cmos_sensor(0x39, 0x20); //Outdoor 113e R2 lvl6 gain
HI351_write_cmos_sensor(0x3a, 0x3f);
HI351_write_cmos_sensor(0x3b, 0x22); //Outdoor 113f R2 lvl7 gain
HI351_write_cmos_sensor(0x3c, 0x40);
HI351_write_cmos_sensor(0x3d, 0x20); //Outdoor 1140 R2 lvl8 gain
HI351_write_cmos_sensor(0x3e, 0x41);
HI351_write_cmos_sensor(0x3f, 0x10); //Outdoor 1141 R2 Lvl1 offset
HI351_write_cmos_sensor(0x40, 0x42);
HI351_write_cmos_sensor(0x41, 0x10); //Outdoor 1142 R2 Lvl2 offset
HI351_write_cmos_sensor(0x42, 0x43);
HI351_write_cmos_sensor(0x43, 0x10); //Outdoor 1143 R2 Lvl3 offset
HI351_write_cmos_sensor(0x44, 0x44);
HI351_write_cmos_sensor(0x45, 0x10); //Outdoor 1144 R2 Lvl4 offset
HI351_write_cmos_sensor(0x46, 0x45);
HI351_write_cmos_sensor(0x47, 0x18); //Outdoor 1145 R2 Lvl5 offset
HI351_write_cmos_sensor(0x48, 0x46);
HI351_write_cmos_sensor(0x49, 0x18); //Outdoor 1146 R2 Lvl6 offset
HI351_write_cmos_sensor(0x4a, 0x47);
HI351_write_cmos_sensor(0x4b, 0x40); //Outdoor 1147 R2 Lvl7 offset
HI351_write_cmos_sensor(0x4c, 0x48);
HI351_write_cmos_sensor(0x4d, 0x40); //Outdoor 1148 R2 Lvl8 offset
HI351_write_cmos_sensor(0x4e, 0x49);
HI351_write_cmos_sensor(0x4f, 0xe0); //Outdoor 1149 Lv1 h_clip
HI351_write_cmos_sensor(0x50, 0x4a);
HI351_write_cmos_sensor(0x51, 0xf0); //Outdoor 114a Lv2 h_clip
HI351_write_cmos_sensor(0x52, 0x4b);
HI351_write_cmos_sensor(0x53, 0xfc); //Outdoor 114b Lv3 h_clip
HI351_write_cmos_sensor(0x54, 0x4c);
HI351_write_cmos_sensor(0x55, 0xfc); //Outdoor 114c Lv4 h_clip
HI351_write_cmos_sensor(0x56, 0x4d);
HI351_write_cmos_sensor(0x57, 0xfc); //Outdoor 114d Lv5 h_clip
HI351_write_cmos_sensor(0x58, 0x4e);
HI351_write_cmos_sensor(0x59, 0xf0); //Outdoor 114e Lv6 h_clip
HI351_write_cmos_sensor(0x5a, 0x4f);
HI351_write_cmos_sensor(0x5b, 0xf0); //Outdoor 114f Lv7 h_clip
HI351_write_cmos_sensor(0x5c, 0x50);
HI351_write_cmos_sensor(0x5d, 0xf0); //Outdoor 1150 Lv8 h_clip
HI351_write_cmos_sensor(0x5e, 0x51);
HI351_write_cmos_sensor(0x5f, 0x68); //Outdoor 1151 color gain start
HI351_write_cmos_sensor(0x60, 0x52);
HI351_write_cmos_sensor(0x61, 0x68); //Outdoor 1152
HI351_write_cmos_sensor(0x62, 0x53);
HI351_write_cmos_sensor(0x63, 0x68); //Outdoor 1153
HI351_write_cmos_sensor(0x64, 0x54);
HI351_write_cmos_sensor(0x65, 0x68); //Outdoor 1154
HI351_write_cmos_sensor(0x66, 0x55);
HI351_write_cmos_sensor(0x67, 0x68); //Outdoor 1155
HI351_write_cmos_sensor(0x68, 0x56);
HI351_write_cmos_sensor(0x69, 0x68); //Outdoor 1156
HI351_write_cmos_sensor(0x6a, 0x57);
HI351_write_cmos_sensor(0x6b, 0x68); //Outdoor 1157
HI351_write_cmos_sensor(0x6c, 0x58);
HI351_write_cmos_sensor(0x6d, 0x68); //Outdoor 1158 color gain end
HI351_write_cmos_sensor(0x6e, 0x59);
HI351_write_cmos_sensor(0x6f, 0x70); //Outdoor 1159 color ofs lmt start
HI351_write_cmos_sensor(0x70, 0x5a);
HI351_write_cmos_sensor(0x71, 0x70); //Outdoor 115a
HI351_write_cmos_sensor(0x72, 0x5b);
HI351_write_cmos_sensor(0x73, 0x70); //Outdoor 115b
HI351_write_cmos_sensor(0x74, 0x5c);
HI351_write_cmos_sensor(0x75, 0x70); //Outdoor 115c
HI351_write_cmos_sensor(0x76, 0x5d);
HI351_write_cmos_sensor(0x77, 0x70); //Outdoor 115d
HI351_write_cmos_sensor(0x78, 0x5e);
HI351_write_cmos_sensor(0x79, 0x70); //Outdoor 115e
HI351_write_cmos_sensor(0x7a, 0x5f);
HI351_write_cmos_sensor(0x7b, 0x70); //Outdoor 115f
HI351_write_cmos_sensor(0x7c, 0x60);
HI351_write_cmos_sensor(0x7d, 0x70); //Outdoor 1160 color ofs lmt end
HI351_write_cmos_sensor(0x7e, 0x61);
HI351_write_cmos_sensor(0x7f, 0xc0); //Outdoor 1161
HI351_write_cmos_sensor(0x80, 0x62);
HI351_write_cmos_sensor(0x81, 0xf0); //Outdoor 1162
HI351_write_cmos_sensor(0x82, 0x63);
HI351_write_cmos_sensor(0x83, 0x80); //Outdoor 1163
HI351_write_cmos_sensor(0x84, 0x64);
HI351_write_cmos_sensor(0x85, 0x40); //Outdoor 1164
HI351_write_cmos_sensor(0x86, 0x65);
HI351_write_cmos_sensor(0x87, 0x80); //Outdoor 1165
HI351_write_cmos_sensor(0x88, 0x66);
HI351_write_cmos_sensor(0x89, 0x80); //Outdoor 1166
HI351_write_cmos_sensor(0x8a, 0x67);
HI351_write_cmos_sensor(0x8b, 0x80); //Outdoor 1167
HI351_write_cmos_sensor(0x8c, 0x68);
HI351_write_cmos_sensor(0x8d, 0x80); //Outdoor 1168
HI351_write_cmos_sensor(0x8e, 0x69);
HI351_write_cmos_sensor(0x8f, 0x40); //Outdoor 1169
HI351_write_cmos_sensor(0x90, 0x6a);
HI351_write_cmos_sensor(0x91, 0x80); //Outdoor 116a Imp Lv2 High Gain
HI351_write_cmos_sensor(0x92, 0x6b);
HI351_write_cmos_sensor(0x93, 0x80); //Outdoor 116b Imp Lv2 Middle Gain
HI351_write_cmos_sensor(0x94, 0x6c);
HI351_write_cmos_sensor(0x95, 0x80); //Outdoor 116c Imp Lv2 Low Gain
HI351_write_cmos_sensor(0x96, 0x6d);
HI351_write_cmos_sensor(0x97, 0x80); //Outdoor 116d
HI351_write_cmos_sensor(0x98, 0x6e);
HI351_write_cmos_sensor(0x99, 0x40); //Outdoor 116e
HI351_write_cmos_sensor(0x9a, 0x6f);
HI351_write_cmos_sensor(0x9b, 0x80); //Outdoor 116f Imp Lv3 Hi Gain
HI351_write_cmos_sensor(0x9c, 0x70);
HI351_write_cmos_sensor(0x9d, 0x60); //Outdoor 1170 Imp Lv3 Middle Gain
HI351_write_cmos_sensor(0x9e, 0x71);
HI351_write_cmos_sensor(0x9f, 0x40); //Outdoor 1171 Imp Lv3 Low Gain
HI351_write_cmos_sensor(0xa0, 0x72);
HI351_write_cmos_sensor(0xa1, 0x6e); //Outdoor 1172
HI351_write_cmos_sensor(0xa2, 0x73);
HI351_write_cmos_sensor(0xa3, 0x3a); //Outdoor 1173
HI351_write_cmos_sensor(0xa4, 0x74);
HI351_write_cmos_sensor(0xa5, 0x80); //Outdoor 1174 Imp Lv4 Hi Gain
HI351_write_cmos_sensor(0xa6, 0x75);
HI351_write_cmos_sensor(0xa7, 0x80); //Outdoor 1175 Imp Lv4 Middle Gain
HI351_write_cmos_sensor(0xa8, 0x76);
HI351_write_cmos_sensor(0xa9, 0x40); //Outdoor 1176 Imp Lv4 Low Gain
HI351_write_cmos_sensor(0xaa, 0x77);
HI351_write_cmos_sensor(0xab, 0x6e); //Outdoor 1177 Imp Lv5 Hi Th
HI351_write_cmos_sensor(0xac, 0x78);
HI351_write_cmos_sensor(0xad, 0x66); //Outdoor 1178 Imp Lv5 Middle Th
HI351_write_cmos_sensor(0xae, 0x79);
HI351_write_cmos_sensor(0xaf, 0x60); //Outdoor 1179 Imp Lv5 Hi Gain
HI351_write_cmos_sensor(0xb0, 0x7a);
HI351_write_cmos_sensor(0xb1, 0x30); //Outdoor 117a Imp Lv5 Middle Gain
HI351_write_cmos_sensor(0xb2, 0x7b);
HI351_write_cmos_sensor(0xb3, 0x20); //Outdoor 117b Imp Lv5 Low Gain
HI351_write_cmos_sensor(0xb4, 0x7c);
HI351_write_cmos_sensor(0xb5, 0x5c); //Outdoor 117c Imp Lv6 Hi Th
HI351_write_cmos_sensor(0xb6, 0x7d);
HI351_write_cmos_sensor(0xb7, 0x30); //Outdoor 117d Imp Lv6 Middle Th
HI351_write_cmos_sensor(0xb8, 0x7e);
HI351_write_cmos_sensor(0xb9, 0x60); //Outdoor 117e Imp Lv6 Hi Gain
HI351_write_cmos_sensor(0xba, 0x7f);
HI351_write_cmos_sensor(0xbb, 0x60); //Outdoor 117f Imp Lv6 Middle Gain
HI351_write_cmos_sensor(0xbc, 0x80);
HI351_write_cmos_sensor(0xbd, 0x20); //Outdoor 1180 Imp Lv6 Low Gain
HI351_write_cmos_sensor(0xbe, 0x81);
HI351_write_cmos_sensor(0xbf, 0x62); //Outdoor 1181
HI351_write_cmos_sensor(0xc0, 0x82);
HI351_write_cmos_sensor(0xc1, 0x26); //Outdoor 1182
HI351_write_cmos_sensor(0xc2, 0x83);
HI351_write_cmos_sensor(0xc3, 0x60); //Outdoor 1183 Imp Lv7 Hi Gain
HI351_write_cmos_sensor(0xc4, 0x84);
HI351_write_cmos_sensor(0xc5, 0x60); //Outdoor 1184 Imp Lv7 Middle Gain
HI351_write_cmos_sensor(0xc6, 0x85);
HI351_write_cmos_sensor(0xc7, 0x60); //Outdoor 1185 Imp Lv7 Low Gain
HI351_write_cmos_sensor(0xc8, 0x86);
HI351_write_cmos_sensor(0xc9, 0x62); //Outdoor 1186
HI351_write_cmos_sensor(0xca, 0x87);
HI351_write_cmos_sensor(0xcb, 0x26); //Outdoor 1187
HI351_write_cmos_sensor(0xcc, 0x88);
HI351_write_cmos_sensor(0xcd, 0x60); //Outdoor 1188
HI351_write_cmos_sensor(0xce, 0x89);
HI351_write_cmos_sensor(0xcf, 0x60); //Outdoor 1189
HI351_write_cmos_sensor(0xd0, 0x8a);
HI351_write_cmos_sensor(0xd1, 0x60); //Outdoor 118a
HI351_write_cmos_sensor(0xd2, 0x90);
HI351_write_cmos_sensor(0xd3, 0x00); //Outdoor 1190
HI351_write_cmos_sensor(0xd4, 0x91);
HI351_write_cmos_sensor(0xd5, 0x4e); //Outdoor 1191
HI351_write_cmos_sensor(0xd6, 0x92);
HI351_write_cmos_sensor(0xd7, 0x00); //Outdoor 1192
HI351_write_cmos_sensor(0xd8, 0x93);
HI351_write_cmos_sensor(0xd9, 0x16); //Outdoor 1193
HI351_write_cmos_sensor(0xda, 0x94);
HI351_write_cmos_sensor(0xdb, 0x01); //Outdoor 1194
HI351_write_cmos_sensor(0xdc, 0x95);
HI351_write_cmos_sensor(0xdd, 0x80); //Outdoor 1195
HI351_write_cmos_sensor(0xde, 0x96);
HI351_write_cmos_sensor(0xdf, 0x55); //Outdoor 1196
HI351_write_cmos_sensor(0xe0, 0x97);
HI351_write_cmos_sensor(0xe1, 0x8d); //Outdoor 1197
HI351_write_cmos_sensor(0xe2, 0xb0);
HI351_write_cmos_sensor(0xe3, 0x60); //Outdoor 11b0
HI351_write_cmos_sensor(0xe4, 0xb1);
HI351_write_cmos_sensor(0xe5, 0xb0); //Outdoor 11b1
HI351_write_cmos_sensor(0xe6, 0xb2);
HI351_write_cmos_sensor(0xe7, 0x88); //Outdoor 11b2
HI351_write_cmos_sensor(0xe8, 0xb3);
HI351_write_cmos_sensor(0xe9, 0x10); //Outdoor 11b3
HI351_write_cmos_sensor(0xea, 0xb4);
HI351_write_cmos_sensor(0xeb, 0x04); //Outdoor 11b4
HI351_write_cmos_sensor(0xec, 0x03);
HI351_write_cmos_sensor(0xed, 0x12);
HI351_write_cmos_sensor(0xee, 0x10);
HI351_write_cmos_sensor(0xef, 0x03); //Outdoor 1210
HI351_write_cmos_sensor(0xf0, 0x11);
HI351_write_cmos_sensor(0xf1, 0x29); //Outdoor 1211
HI351_write_cmos_sensor(0xf2, 0x12);
HI351_write_cmos_sensor(0xf3, 0x08); //Outdoor 1212
HI351_write_cmos_sensor(0xf4, 0x40);
HI351_write_cmos_sensor(0xf5, 0x33); //Outdoor 1240
HI351_write_cmos_sensor(0xf6, 0x41);
HI351_write_cmos_sensor(0xf7, 0x0a); //Outdoor 1241
HI351_write_cmos_sensor(0xf8, 0x42);
HI351_write_cmos_sensor(0xf9, 0x6a); //Outdoor 1242
HI351_write_cmos_sensor(0xfa, 0x43);
HI351_write_cmos_sensor(0xfb, 0x80); //Outdoor 1243
HI351_write_cmos_sensor(0xfc, 0x44);
HI351_write_cmos_sensor(0xfd, 0x02); //Outdoor 1244
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
HI351_write_cmos_sensor(0x03, 0xdb);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x45);
HI351_write_cmos_sensor(0x11, 0x0a); //Outdoor 1245
HI351_write_cmos_sensor(0x12, 0x46);
HI351_write_cmos_sensor(0x13, 0x80); //Outdoor 1246
HI351_write_cmos_sensor(0x14, 0x60);
HI351_write_cmos_sensor(0x15, 0x21); //Outdoor 1260
HI351_write_cmos_sensor(0x16, 0x61);
HI351_write_cmos_sensor(0x17, 0x0e); //Outdoor 1261
HI351_write_cmos_sensor(0x18, 0x62);
HI351_write_cmos_sensor(0x19, 0x70); //Outdoor 1262
HI351_write_cmos_sensor(0x1a, 0x63);
HI351_write_cmos_sensor(0x1b, 0x70); //Outdoor 1263
HI351_write_cmos_sensor(0x1c, 0x64);
HI351_write_cmos_sensor(0x1d, 0x14); //Outdoor 1264
HI351_write_cmos_sensor(0x1e, 0x65);
HI351_write_cmos_sensor(0x1f, 0x01); //Outdoor 1265
HI351_write_cmos_sensor(0x20, 0x68);
HI351_write_cmos_sensor(0x21, 0x0a); //Outdoor 1268
HI351_write_cmos_sensor(0x22, 0x69);
HI351_write_cmos_sensor(0x23, 0x04); //Outdoor 1269
HI351_write_cmos_sensor(0x24, 0x6a);
HI351_write_cmos_sensor(0x25, 0x0a); //Outdoor 126a
HI351_write_cmos_sensor(0x26, 0x6b);
HI351_write_cmos_sensor(0x27, 0x0a); //Outdoor 126b
HI351_write_cmos_sensor(0x28, 0x6c);
HI351_write_cmos_sensor(0x29, 0x24); //Outdoor 126c
HI351_write_cmos_sensor(0x2a, 0x6d);
HI351_write_cmos_sensor(0x2b, 0x01); //Outdoor 126d
HI351_write_cmos_sensor(0x2c, 0x70);
HI351_write_cmos_sensor(0x2d, 0x01); //Outdoor 1270
HI351_write_cmos_sensor(0x2e, 0x71);
HI351_write_cmos_sensor(0x2f, 0x3d); //Outdoor 1271
HI351_write_cmos_sensor(0x30, 0x80);
HI351_write_cmos_sensor(0x31, 0x80); //Outdoor 1280
HI351_write_cmos_sensor(0x32, 0x81);
HI351_write_cmos_sensor(0x33, 0x88); //Outdoor 1281
HI351_write_cmos_sensor(0x34, 0x82);
HI351_write_cmos_sensor(0x35, 0x08); //Outdoor 1282
HI351_write_cmos_sensor(0x36, 0x83);
HI351_write_cmos_sensor(0x37, 0x0c); //Outdoor 1283
HI351_write_cmos_sensor(0x38, 0x84);
HI351_write_cmos_sensor(0x39, 0x90); //Outdoor 1284
HI351_write_cmos_sensor(0x3a, 0x85);
HI351_write_cmos_sensor(0x3b, 0x92); //Outdoor 1285
HI351_write_cmos_sensor(0x3c, 0x86);
HI351_write_cmos_sensor(0x3d, 0x20); //Outdoor 1286
HI351_write_cmos_sensor(0x3e, 0x87);
HI351_write_cmos_sensor(0x3f, 0x00); //Outdoor 1287
HI351_write_cmos_sensor(0x40, 0x88);
HI351_write_cmos_sensor(0x41, 0x70); //Outdoor 1288
HI351_write_cmos_sensor(0x42, 0x89);
HI351_write_cmos_sensor(0x43, 0xaa); //Outdoor 1289
HI351_write_cmos_sensor(0x44, 0x8a);
HI351_write_cmos_sensor(0x45, 0x50); //Outdoor 128a
HI351_write_cmos_sensor(0x46, 0x8b);
HI351_write_cmos_sensor(0x47, 0x10); //Outdoor 128b
HI351_write_cmos_sensor(0x48, 0x8c);
HI351_write_cmos_sensor(0x49, 0x04); //Outdoor 128c
HI351_write_cmos_sensor(0x4a, 0x8d);
HI351_write_cmos_sensor(0x4b, 0x02); //Outdoor 128d
HI351_write_cmos_sensor(0x4c, 0xe6);
HI351_write_cmos_sensor(0x4d, 0xff); //Outdoor 12e6
HI351_write_cmos_sensor(0x4e, 0xe7);
HI351_write_cmos_sensor(0x4f, 0x18); //Outdoor 12e7
HI351_write_cmos_sensor(0x50, 0xe8);
HI351_write_cmos_sensor(0x51, 0x20); //Outdoor 12e8
HI351_write_cmos_sensor(0x52, 0xe9);
HI351_write_cmos_sensor(0x53, 0x06); //Outdoor 12e9
HI351_write_cmos_sensor(0x54, 0x03);
HI351_write_cmos_sensor(0x55, 0x13);
HI351_write_cmos_sensor(0x56, 0x10);
HI351_write_cmos_sensor(0x57, 0x33); //Outdoor 1310
HI351_write_cmos_sensor(0x58, 0x20);
HI351_write_cmos_sensor(0x59, 0x20); //Outdoor 1320
HI351_write_cmos_sensor(0x5a, 0x21);
HI351_write_cmos_sensor(0x5b, 0x30); //Outdoor 1321
HI351_write_cmos_sensor(0x5c, 0x22);
HI351_write_cmos_sensor(0x5d, 0x36); //Outdoor 1322
HI351_write_cmos_sensor(0x5e, 0x23);
HI351_write_cmos_sensor(0x5f, 0x6a); //Outdoor 1323
HI351_write_cmos_sensor(0x60, 0x24);
HI351_write_cmos_sensor(0x61, 0xa0); //Outdoor 1324
HI351_write_cmos_sensor(0x62, 0x25);
HI351_write_cmos_sensor(0x63, 0xc0); //Outdoor 1325
HI351_write_cmos_sensor(0x64, 0x26);
HI351_write_cmos_sensor(0x65, 0xe0); //Outdoor 1326
HI351_write_cmos_sensor(0x66, 0x27);
HI351_write_cmos_sensor(0x67, 0x02); //Outdoor 1327
HI351_write_cmos_sensor(0x68, 0x28);
HI351_write_cmos_sensor(0x69, 0x03); //Outdoor 1328
HI351_write_cmos_sensor(0x6a, 0x29);
HI351_write_cmos_sensor(0x6b, 0x03); //Outdoor 1329
HI351_write_cmos_sensor(0x6c, 0x2a);
HI351_write_cmos_sensor(0x6d, 0x10); //Outdoor 132a
HI351_write_cmos_sensor(0x6e, 0x2b);
HI351_write_cmos_sensor(0x6f, 0x10); //Outdoor 132b
HI351_write_cmos_sensor(0x70, 0x2c);
HI351_write_cmos_sensor(0x71, 0x04); //Outdoor 132c
HI351_write_cmos_sensor(0x72, 0x2d);
HI351_write_cmos_sensor(0x73, 0x03); //Outdoor 132d
HI351_write_cmos_sensor(0x74, 0x2e);
HI351_write_cmos_sensor(0x75, 0x03); //Outdoor 132e
HI351_write_cmos_sensor(0x76, 0x2f);
HI351_write_cmos_sensor(0x77, 0x18); //Outdoor 132f
HI351_write_cmos_sensor(0x78, 0x30);
HI351_write_cmos_sensor(0x79, 0x03); //Outdoor 1330
HI351_write_cmos_sensor(0x7a, 0x31);
HI351_write_cmos_sensor(0x7b, 0x03); //Outdoor 1331
HI351_write_cmos_sensor(0x7c, 0x32);
HI351_write_cmos_sensor(0x7d, 0x03); //Outdoor 1332
HI351_write_cmos_sensor(0x7e, 0x33);
HI351_write_cmos_sensor(0x7f, 0x40); //Outdoor 1333
HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x81, 0x80); //Outdoor 1334
HI351_write_cmos_sensor(0x82, 0x35);
HI351_write_cmos_sensor(0x83, 0x00); //Outdoor 1335
HI351_write_cmos_sensor(0x84, 0x36);
HI351_write_cmos_sensor(0x85, 0xf0); //Outdoor 1336
HI351_write_cmos_sensor(0x86, 0xa0);
HI351_write_cmos_sensor(0x87, 0x07); //Outdoor 13a0
HI351_write_cmos_sensor(0x88, 0xa8);
HI351_write_cmos_sensor(0x89, 0x24); //Outdoor 13a8 Cb_Filter
HI351_write_cmos_sensor(0x8a, 0xa9);
HI351_write_cmos_sensor(0x8b, 0x24); //Outdoor 13a9 Cr_Filter
HI351_write_cmos_sensor(0x8c, 0xaa);
HI351_write_cmos_sensor(0x8d, 0x20); //Outdoor 13aa
HI351_write_cmos_sensor(0x8e, 0xab);
HI351_write_cmos_sensor(0x8f, 0x02); //Outdoor 13ab
HI351_write_cmos_sensor(0x90, 0xc0);
HI351_write_cmos_sensor(0x91, 0x27); //Outdoor 13c0
HI351_write_cmos_sensor(0x92, 0xc2);
HI351_write_cmos_sensor(0x93, 0x08); //Outdoor 13c2
HI351_write_cmos_sensor(0x94, 0xc3);
HI351_write_cmos_sensor(0x95, 0x08); //Outdoor 13c3
HI351_write_cmos_sensor(0x96, 0xc4);
HI351_write_cmos_sensor(0x97, 0x40); //Outdoor 13c4
HI351_write_cmos_sensor(0x98, 0xc5);
HI351_write_cmos_sensor(0x99, 0x38); //Outdoor 13c5
HI351_write_cmos_sensor(0x9a, 0xc6);
HI351_write_cmos_sensor(0x9b, 0xf0); //Outdoor 13c6
HI351_write_cmos_sensor(0x9c, 0xc7);
HI351_write_cmos_sensor(0x9d, 0x10); //Outdoor 13c7
HI351_write_cmos_sensor(0x9e, 0xc8);
HI351_write_cmos_sensor(0x9f, 0x44); //Outdoor 13c8
HI351_write_cmos_sensor(0xa0, 0xc9);
HI351_write_cmos_sensor(0xa1, 0x87); //Outdoor 13c9
HI351_write_cmos_sensor(0xa2, 0xca);
HI351_write_cmos_sensor(0xa3, 0xff); //Outdoor 13ca
HI351_write_cmos_sensor(0xa4, 0xcb);
HI351_write_cmos_sensor(0xa5, 0x20); //Outdoor 13cb
HI351_write_cmos_sensor(0xa6, 0xcc);
HI351_write_cmos_sensor(0xa7, 0x61); //Outdoor 13cc
HI351_write_cmos_sensor(0xa8, 0xcd);
HI351_write_cmos_sensor(0xa9, 0x87); //Outdoor 13cd
HI351_write_cmos_sensor(0xaa, 0xce);
HI351_write_cmos_sensor(0xab, 0x8a); //Outdoor 13ce
HI351_write_cmos_sensor(0xac, 0xcf);
HI351_write_cmos_sensor(0xad, 0xa5); //Outdoor 13cf
HI351_write_cmos_sensor(0xae, 0x03);
HI351_write_cmos_sensor(0xaf, 0x14);
HI351_write_cmos_sensor(0xb0, 0x11);
HI351_write_cmos_sensor(0xb1, 0x02); //Outdoor 1410
HI351_write_cmos_sensor(0xb2, 0x11);
HI351_write_cmos_sensor(0xb3, 0x02); //Outdoor 1411 TOP L_clip
HI351_write_cmos_sensor(0xb4, 0x12);
HI351_write_cmos_sensor(0xb5, 0x40); //Outdoor 1412
HI351_write_cmos_sensor(0xb6, 0x13);
HI351_write_cmos_sensor(0xb7, 0x98); //Outdoor 1413
HI351_write_cmos_sensor(0xb8, 0x14);
HI351_write_cmos_sensor(0xb9, 0x3a); //Outdoor 1414
HI351_write_cmos_sensor(0xba, 0x15);
HI351_write_cmos_sensor(0xbb, 0x40); //Outdoor 1415 Positive High Gain
HI351_write_cmos_sensor(0xbc, 0x16);
HI351_write_cmos_sensor(0xbd, 0x40); //Outdoor 1416 Positive Middel Gain
HI351_write_cmos_sensor(0xbe, 0x17);
HI351_write_cmos_sensor(0xbf, 0x40); //Outdoor 1417 Positive Low Gain
HI351_write_cmos_sensor(0xc0, 0x18);
HI351_write_cmos_sensor(0xc1, 0x68); //Outdoor 1418  Negative High Gain
HI351_write_cmos_sensor(0xc2, 0x19);
HI351_write_cmos_sensor(0xc3, 0x70); //Outdoor 1419  Negative Middle Gain
HI351_write_cmos_sensor(0xc4, 0x1a);
HI351_write_cmos_sensor(0xc5, 0x70); //Outdoor 141a  Negative Low Gain
HI351_write_cmos_sensor(0xc6, 0x20);
HI351_write_cmos_sensor(0xc7, 0x82); //Outdoor 1420  s_diff L_clip
HI351_write_cmos_sensor(0xc8, 0x21);
HI351_write_cmos_sensor(0xc9, 0x03); //Outdoor 1421
HI351_write_cmos_sensor(0xca, 0x22);
HI351_write_cmos_sensor(0xcb, 0x05); //Outdoor 1422
HI351_write_cmos_sensor(0xcc, 0x23);
HI351_write_cmos_sensor(0xcd, 0x07); //Outdoor 1423
HI351_write_cmos_sensor(0xce, 0x24);
HI351_write_cmos_sensor(0xcf, 0x0a); //Outdoor 1424
HI351_write_cmos_sensor(0xd0, 0x25);
HI351_write_cmos_sensor(0xd1, 0x46); //Outdoor 1425
HI351_write_cmos_sensor(0xd2, 0x26);
HI351_write_cmos_sensor(0xd3, 0x32); //Outdoor 1426
HI351_write_cmos_sensor(0xd4, 0x27);
HI351_write_cmos_sensor(0xd5, 0x1e); //Outdoor 1427
HI351_write_cmos_sensor(0xd6, 0x28);
HI351_write_cmos_sensor(0xd7, 0x10); //Outdoor 1428
HI351_write_cmos_sensor(0xd8, 0x29);
HI351_write_cmos_sensor(0xd9, 0x00); //Outdoor 1429
HI351_write_cmos_sensor(0xda, 0x2a);
HI351_write_cmos_sensor(0xdb, 0x18); //Outdoor 142a
HI351_write_cmos_sensor(0xdc, 0x2b);
HI351_write_cmos_sensor(0xdd, 0x18); //Outdoor 142b
HI351_write_cmos_sensor(0xde, 0x2c);
HI351_write_cmos_sensor(0xdf, 0x18); //Outdoor 142c
HI351_write_cmos_sensor(0xe0, 0x2d);
HI351_write_cmos_sensor(0xe1, 0x30); //Outdoor 142d
HI351_write_cmos_sensor(0xe2, 0x2e);
HI351_write_cmos_sensor(0xe3, 0x30); //Outdoor 142e
HI351_write_cmos_sensor(0xe4, 0x2f);
HI351_write_cmos_sensor(0xe5, 0x30); //Outdoor 142f
HI351_write_cmos_sensor(0xe6, 0x30);
HI351_write_cmos_sensor(0xe7, 0x82); //Outdoor 1430 Ldiff_L_cip
HI351_write_cmos_sensor(0xe8, 0x31);
HI351_write_cmos_sensor(0xe9, 0x02); //Outdoor 1431
HI351_write_cmos_sensor(0xea, 0x32);
HI351_write_cmos_sensor(0xeb, 0x04); //Outdoor 1432
HI351_write_cmos_sensor(0xec, 0x33);
HI351_write_cmos_sensor(0xed, 0x04); //Outdoor 1433
HI351_write_cmos_sensor(0xee, 0x34);
HI351_write_cmos_sensor(0xef, 0x0a); //Outdoor 1434
HI351_write_cmos_sensor(0xf0, 0x35);
HI351_write_cmos_sensor(0xf1, 0x46); //Outdoor 1435
HI351_write_cmos_sensor(0xf2, 0x36);
HI351_write_cmos_sensor(0xf3, 0x32); //Outdoor 1436
HI351_write_cmos_sensor(0xf4, 0x37);
HI351_write_cmos_sensor(0xf5, 0x28); //Outdoor 1437
HI351_write_cmos_sensor(0xf6, 0x38);
HI351_write_cmos_sensor(0xf7, 0x12); //Outdoor 1438
HI351_write_cmos_sensor(0xf8, 0x39);
HI351_write_cmos_sensor(0xf9, 0x00); //Outdoor 1439
HI351_write_cmos_sensor(0xfa, 0x3a);
HI351_write_cmos_sensor(0xfb, 0x20); //Outdoor 143a
HI351_write_cmos_sensor(0xfc, 0x3b);
HI351_write_cmos_sensor(0xfd, 0x30); //Outdoor 143b
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end

HI351_write_cmos_sensor(0x03, 0xdc);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x3c);
HI351_write_cmos_sensor(0x11, 0x20); //Outdoor 143c
HI351_write_cmos_sensor(0x12, 0x3d);
HI351_write_cmos_sensor(0x13, 0x18); //Outdoor 143d
HI351_write_cmos_sensor(0x14, 0x3e);
HI351_write_cmos_sensor(0x15, 0x28); //Outdoor 143e
HI351_write_cmos_sensor(0x16, 0x3f);
HI351_write_cmos_sensor(0x17, 0x14); //Outdoor 143f
HI351_write_cmos_sensor(0x18, 0x40);
HI351_write_cmos_sensor(0x19, 0x84); //Outdoor 1440  Mdiff Low Clip
HI351_write_cmos_sensor(0x1a, 0x41);
HI351_write_cmos_sensor(0x1b, 0x10); //Outdoor 1441
HI351_write_cmos_sensor(0x1c, 0x42);
HI351_write_cmos_sensor(0x1d, 0x70); //Outdoor 1442
HI351_write_cmos_sensor(0x1e, 0x43);
HI351_write_cmos_sensor(0x1f, 0x20); //Outdoor 1443
HI351_write_cmos_sensor(0x20, 0x44);
HI351_write_cmos_sensor(0x21, 0x18); //Outdoor 1444
HI351_write_cmos_sensor(0x22, 0x45);
HI351_write_cmos_sensor(0x23, 0x18); //Outdoor 1445
HI351_write_cmos_sensor(0x24, 0x46);
HI351_write_cmos_sensor(0x25, 0x10); //Outdoor 1446
HI351_write_cmos_sensor(0x26, 0x47);
HI351_write_cmos_sensor(0x27, 0x18); //Outdoor 1447
HI351_write_cmos_sensor(0x28, 0x48);
HI351_write_cmos_sensor(0x29, 0x0a); //Outdoor 1448
HI351_write_cmos_sensor(0x2a, 0x49);
HI351_write_cmos_sensor(0x2b, 0x10); //Outdoor 1449
HI351_write_cmos_sensor(0x2c, 0x50);
HI351_write_cmos_sensor(0x2d, 0x84); //Outdoor 1450 Hdiff Low Clip
HI351_write_cmos_sensor(0x2e, 0x51);
HI351_write_cmos_sensor(0x2f, 0x30); //Outdoor 1451 hclip
HI351_write_cmos_sensor(0x30, 0x52);
HI351_write_cmos_sensor(0x31, 0xb0); //Outdoor 1452
HI351_write_cmos_sensor(0x32, 0x53);
HI351_write_cmos_sensor(0x33, 0x37); //Outdoor 1453
HI351_write_cmos_sensor(0x34, 0x54);
HI351_write_cmos_sensor(0x35, 0x33); //Outdoor 1454
HI351_write_cmos_sensor(0x36, 0x65);
HI351_write_cmos_sensor(0x37, 0x33); //Outdoor 1455
HI351_write_cmos_sensor(0x38, 0x66);
HI351_write_cmos_sensor(0x39, 0x33); //Outdoor 1456
HI351_write_cmos_sensor(0x3a, 0x57);
HI351_write_cmos_sensor(0x3b, 0x10); //Outdoor 1457
HI351_write_cmos_sensor(0x3c, 0x68);
HI351_write_cmos_sensor(0x3d, 0x20); //Outdoor 1458
HI351_write_cmos_sensor(0x3e, 0x69);
HI351_write_cmos_sensor(0x3f, 0x20); //Outdoor 1459
HI351_write_cmos_sensor(0x40, 0x60);
HI351_write_cmos_sensor(0x41, 0x01); //Outdoor 1460
HI351_write_cmos_sensor(0x42, 0x61);
HI351_write_cmos_sensor(0x43, 0xa0); //Outdoor 1461
HI351_write_cmos_sensor(0x44, 0x62);
HI351_write_cmos_sensor(0x45, 0x98); //Outdoor 1462
HI351_write_cmos_sensor(0x46, 0x63);
HI351_write_cmos_sensor(0x47, 0xe4); //Outdoor 1463
HI351_write_cmos_sensor(0x48, 0x64);
HI351_write_cmos_sensor(0x49, 0xa4); //Outdoor 1464
HI351_write_cmos_sensor(0x4a, 0x65);
HI351_write_cmos_sensor(0x4b, 0x7d); //Outdoor 1465
HI351_write_cmos_sensor(0x4c, 0x66);
HI351_write_cmos_sensor(0x4d, 0x4b); //Outdoor 1466
HI351_write_cmos_sensor(0x4e, 0x70);
HI351_write_cmos_sensor(0x4f, 0x10); //Outdoor 1470
HI351_write_cmos_sensor(0x50, 0x71);
HI351_write_cmos_sensor(0x51, 0x10); //Outdoor 1471
HI351_write_cmos_sensor(0x52, 0x72);
HI351_write_cmos_sensor(0x53, 0x10); //Outdoor 1472
HI351_write_cmos_sensor(0x54, 0x73);
HI351_write_cmos_sensor(0x55, 0x10); //Outdoor 1473
HI351_write_cmos_sensor(0x56, 0x74);
HI351_write_cmos_sensor(0x57, 0x10); //Outdoor 1474
HI351_write_cmos_sensor(0x58, 0x75);
HI351_write_cmos_sensor(0x59, 0x10); //Outdoor 1475
HI351_write_cmos_sensor(0x5a, 0x76);
HI351_write_cmos_sensor(0x5b, 0x38); //Outdoor 1476    green sharp pos High
HI351_write_cmos_sensor(0x5c, 0x77);
HI351_write_cmos_sensor(0x5d, 0x38); //Outdoor 1477    green sharp pos Middle
HI351_write_cmos_sensor(0x5e, 0x78);
HI351_write_cmos_sensor(0x5f, 0x38); //Outdoor 1478    green sharp pos Low
HI351_write_cmos_sensor(0x60, 0x79);
HI351_write_cmos_sensor(0x61, 0x70); //Outdoor 1479    green sharp nega High
HI351_write_cmos_sensor(0x62, 0x7a);
HI351_write_cmos_sensor(0x63, 0x70); //Outdoor 147a    green sharp nega Middle
HI351_write_cmos_sensor(0x64, 0x7b);
HI351_write_cmos_sensor(0x65, 0x70); //Outdoor 147b    green sharp nega Low

HI351_write_cmos_sensor(0x66, 0x03);
HI351_write_cmos_sensor(0x67, 0x10); //10 page
HI351_write_cmos_sensor(0x68, 0x60);
HI351_write_cmos_sensor(0x69, 0x01); //1060
HI351_write_cmos_sensor(0x6a, 0x70);
HI351_write_cmos_sensor(0x6b, 0x0c); //1070
HI351_write_cmos_sensor(0x6c, 0x71);
HI351_write_cmos_sensor(0x6d, 0x00); //1071
HI351_write_cmos_sensor(0x6e, 0x72);
HI351_write_cmos_sensor(0x6f, 0x7a); //1072
HI351_write_cmos_sensor(0x70, 0x73);
HI351_write_cmos_sensor(0x71, 0x28); //1073
HI351_write_cmos_sensor(0x72, 0x74);
HI351_write_cmos_sensor(0x73, 0x14); //1074
HI351_write_cmos_sensor(0x74, 0x75);
HI351_write_cmos_sensor(0x75, 0x0d); //1075
HI351_write_cmos_sensor(0x76, 0x76);
HI351_write_cmos_sensor(0x77, 0x40); //1076
HI351_write_cmos_sensor(0x78, 0x77);
HI351_write_cmos_sensor(0x79, 0x49); //1077
HI351_write_cmos_sensor(0x7a, 0x78);
HI351_write_cmos_sensor(0x7b, 0x99); //1078
HI351_write_cmos_sensor(0x7c, 0x79);
HI351_write_cmos_sensor(0x7d, 0x4c); //1079
HI351_write_cmos_sensor(0x7e, 0x7a);
HI351_write_cmos_sensor(0x7f, 0xcc); //107a
HI351_write_cmos_sensor(0x80, 0x7b);
HI351_write_cmos_sensor(0x81, 0x49); //107b
HI351_write_cmos_sensor(0x82, 0x7c);
HI351_write_cmos_sensor(0x83, 0x99); //107c
HI351_write_cmos_sensor(0x84, 0x7d);
HI351_write_cmos_sensor(0x85, 0x14); //107d
HI351_write_cmos_sensor(0x86, 0x7e);
HI351_write_cmos_sensor(0x87, 0x28); //107e
HI351_write_cmos_sensor(0x88, 0x7f);
HI351_write_cmos_sensor(0x89, 0x50); //107f

HI351_write_cmos_sensor(0x8a, 0x03); //16 page
HI351_write_cmos_sensor(0x8b, 0x16);
HI351_write_cmos_sensor(0x8c, 0x8a); //168a
HI351_write_cmos_sensor(0x8d, 0x68);
HI351_write_cmos_sensor(0x8e, 0x8b); //168b
HI351_write_cmos_sensor(0x8f, 0x7c);
HI351_write_cmos_sensor(0x90, 0x8c); //168c
HI351_write_cmos_sensor(0x91, 0x7f);
HI351_write_cmos_sensor(0x92, 0x8d); //168d
HI351_write_cmos_sensor(0x93, 0x7f);
HI351_write_cmos_sensor(0x94, 0x8e); //168e
HI351_write_cmos_sensor(0x95, 0x7f);
HI351_write_cmos_sensor(0x96, 0x8f); //168f
HI351_write_cmos_sensor(0x97, 0x7f);
HI351_write_cmos_sensor(0x98, 0x90); //1690
HI351_write_cmos_sensor(0x99, 0x7f);
HI351_write_cmos_sensor(0x9a, 0x91); //1691
HI351_write_cmos_sensor(0x9b, 0x7f);
HI351_write_cmos_sensor(0x9c, 0x92); //1692
HI351_write_cmos_sensor(0x9d, 0x7f);

HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
//////////////////
// dd Page (DMA Indoor)
//////////////////
HI351_write_cmos_sensor(0x03, 0xdd);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x03);
HI351_write_cmos_sensor(0x11, 0x11);
HI351_write_cmos_sensor(0x12, 0x10);
HI351_write_cmos_sensor(0x13, 0x13); //Indoor 1110
HI351_write_cmos_sensor(0x14, 0x11);
HI351_write_cmos_sensor(0x15, 0x08); //Indoor 1111
HI351_write_cmos_sensor(0x16, 0x12);
HI351_write_cmos_sensor(0x17, 0x1e); //Indoor 1112
HI351_write_cmos_sensor(0x18, 0x13);
HI351_write_cmos_sensor(0x19, 0x0e); //Indoor 1113
HI351_write_cmos_sensor(0x1a, 0x14);
HI351_write_cmos_sensor(0x1b, 0x31); //Indoor 1114
HI351_write_cmos_sensor(0x1c, 0x30);
HI351_write_cmos_sensor(0x1d, 0x20); //Indoor 1130
HI351_write_cmos_sensor(0x1e, 0x31);
HI351_write_cmos_sensor(0x1f, 0x20); //Indoor 1131
HI351_write_cmos_sensor(0x20, 0x32);
HI351_write_cmos_sensor(0x21, 0x52); //Indoor 1132
HI351_write_cmos_sensor(0x22, 0x33);
HI351_write_cmos_sensor(0x23, 0x3b); //Indoor 1133
HI351_write_cmos_sensor(0x24, 0x34);
HI351_write_cmos_sensor(0x25, 0x1d); //Indoor 1134
HI351_write_cmos_sensor(0x26, 0x35);
HI351_write_cmos_sensor(0x27, 0x21); //Indoor 1135
HI351_write_cmos_sensor(0x28, 0x36);
HI351_write_cmos_sensor(0x29, 0x1b); //Indoor 1136
HI351_write_cmos_sensor(0x2a, 0x37);
HI351_write_cmos_sensor(0x2b, 0x21); //Indoor 1137
HI351_write_cmos_sensor(0x2c, 0x38);
HI351_write_cmos_sensor(0x2d, 0x18); //Indoor 1138
HI351_write_cmos_sensor(0x2e, 0x39);
HI351_write_cmos_sensor(0x2f, 0x18); //Indoor 1139 R2 lvl1 gain
HI351_write_cmos_sensor(0x30, 0x3a);
HI351_write_cmos_sensor(0x31, 0x18); //Indoor 113a R2 lvl2 gain
HI351_write_cmos_sensor(0x32, 0x3b);
HI351_write_cmos_sensor(0x33, 0x18); //Indoor 113b R2 lvl3 gain
HI351_write_cmos_sensor(0x34, 0x3c);
HI351_write_cmos_sensor(0x35, 0x1a); //Indoor 113c R2 lvl4 gain
HI351_write_cmos_sensor(0x36, 0x3d);
HI351_write_cmos_sensor(0x37, 0x1a); //Indoor 113d R2 lvl5 gain
HI351_write_cmos_sensor(0x38, 0x3e);
HI351_write_cmos_sensor(0x39, 0x20); //Indoor 113e R2 lvl6 gain
HI351_write_cmos_sensor(0x3a, 0x3f);
HI351_write_cmos_sensor(0x3b, 0x22); //Indoor 113f R2 lvl7 gain
HI351_write_cmos_sensor(0x3c, 0x40);
HI351_write_cmos_sensor(0x3d, 0x20); //Indoor 1140 R2 lvl8 gain
HI351_write_cmos_sensor(0x3e, 0x41);
HI351_write_cmos_sensor(0x3f, 0x10); //Indoor 1141 R2 Lvl1 offset
HI351_write_cmos_sensor(0x40, 0x42);
HI351_write_cmos_sensor(0x41, 0x10); //Indoor 1142 R2 Lvl2 offset
HI351_write_cmos_sensor(0x42, 0x43);
HI351_write_cmos_sensor(0x43, 0x10); //Indoor 1143 R2 Lvl3 offset
HI351_write_cmos_sensor(0x44, 0x44);
HI351_write_cmos_sensor(0x45, 0x10); //Indoor 1144 R2 Lvl4 offset
HI351_write_cmos_sensor(0x46, 0x45);
HI351_write_cmos_sensor(0x47, 0x18); //Indoor 1145 R2 Lvl5 offset
HI351_write_cmos_sensor(0x48, 0x46);
HI351_write_cmos_sensor(0x49, 0x18); //Indoor 1146 R2 Lvl6 offset
HI351_write_cmos_sensor(0x4a, 0x47);
HI351_write_cmos_sensor(0x4b, 0x40); //Indoor 1147 R2 Lvl7 offset
HI351_write_cmos_sensor(0x4c, 0x48);
HI351_write_cmos_sensor(0x4d, 0x40); //Indoor 1148 R2 Lvl8 offset
HI351_write_cmos_sensor(0x4e, 0x49);
HI351_write_cmos_sensor(0x4f, 0xe0); //Indoor 1149 Lv1 h_clip
HI351_write_cmos_sensor(0x50, 0x4a);
HI351_write_cmos_sensor(0x51, 0xf0); //Indoor 114a Lv2 h_clip
HI351_write_cmos_sensor(0x52, 0x4b);
HI351_write_cmos_sensor(0x53, 0xfc); //Indoor 114b Lv3 h_clip
HI351_write_cmos_sensor(0x54, 0x4c);
HI351_write_cmos_sensor(0x55, 0xfc); //Indoor 114c Lv4 h_clip
HI351_write_cmos_sensor(0x56, 0x4d);
HI351_write_cmos_sensor(0x57, 0xfc); //Indoor 114d Lv5 h_clip
HI351_write_cmos_sensor(0x58, 0x4e);
HI351_write_cmos_sensor(0x59, 0xf0); //Indoor 114e Lv6 h_clip
HI351_write_cmos_sensor(0x5a, 0x4f);
HI351_write_cmos_sensor(0x5b, 0xf0); //Indoor 114f Lv7 h_clip
HI351_write_cmos_sensor(0x5c, 0x50);
HI351_write_cmos_sensor(0x5d, 0xf0); //Indoor 1150 Lv8 h_clip
HI351_write_cmos_sensor(0x5e, 0x51);
HI351_write_cmos_sensor(0x5f, 0x68); //Indoor 1151 color gain start
HI351_write_cmos_sensor(0x60, 0x52);
HI351_write_cmos_sensor(0x61, 0x68); //Indoor 1152
HI351_write_cmos_sensor(0x62, 0x53);
HI351_write_cmos_sensor(0x63, 0x68); //Indoor 1153
HI351_write_cmos_sensor(0x64, 0x54);
HI351_write_cmos_sensor(0x65, 0x68); //Indoor 1154
HI351_write_cmos_sensor(0x66, 0x55);
HI351_write_cmos_sensor(0x67, 0x68); //Indoor 1155
HI351_write_cmos_sensor(0x68, 0x56);
HI351_write_cmos_sensor(0x69, 0x68); //Indoor 1156
HI351_write_cmos_sensor(0x6a, 0x57);
HI351_write_cmos_sensor(0x6b, 0x68); //Indoor 1157
HI351_write_cmos_sensor(0x6c, 0x58);
HI351_write_cmos_sensor(0x6d, 0x68); //Indoor 1158 color gain end
HI351_write_cmos_sensor(0x6e, 0x59);
HI351_write_cmos_sensor(0x6f, 0x70); //Indoor 1159 color ofs lmt start
HI351_write_cmos_sensor(0x70, 0x5a);
HI351_write_cmos_sensor(0x71, 0x70); //Indoor 115a
HI351_write_cmos_sensor(0x72, 0x5b);
HI351_write_cmos_sensor(0x73, 0x70); //Indoor 115b
HI351_write_cmos_sensor(0x74, 0x5c);
HI351_write_cmos_sensor(0x75, 0x70); //Indoor 115c
HI351_write_cmos_sensor(0x76, 0x5d);
HI351_write_cmos_sensor(0x77, 0x70); //Indoor 115d
HI351_write_cmos_sensor(0x78, 0x5e);
HI351_write_cmos_sensor(0x79, 0x70); //Indoor 115e
HI351_write_cmos_sensor(0x7a, 0x5f);
HI351_write_cmos_sensor(0x7b, 0x70); //Indoor 115f
HI351_write_cmos_sensor(0x7c, 0x60);
HI351_write_cmos_sensor(0x7d, 0x70); //Indoor 1160 color ofs lmt end
HI351_write_cmos_sensor(0x7e, 0x61);
HI351_write_cmos_sensor(0x7f, 0xc0); //Indoor 1161
HI351_write_cmos_sensor(0x80, 0x62);
HI351_write_cmos_sensor(0x81, 0xf0); //Indoor 1162
HI351_write_cmos_sensor(0x82, 0x63);
HI351_write_cmos_sensor(0x83, 0x80); //Indoor 1163
HI351_write_cmos_sensor(0x84, 0x64);
HI351_write_cmos_sensor(0x85, 0x40); //Indoor 1164
HI351_write_cmos_sensor(0x86, 0x65);
HI351_write_cmos_sensor(0x87, 0x80); //Indoor 1165
HI351_write_cmos_sensor(0x88, 0x66);
HI351_write_cmos_sensor(0x89, 0x80); //Indoor 1166
HI351_write_cmos_sensor(0x8a, 0x67);
HI351_write_cmos_sensor(0x8b, 0x80); //Indoor 1167
HI351_write_cmos_sensor(0x8c, 0x68);
HI351_write_cmos_sensor(0x8d, 0x80); //Indoor 1168
HI351_write_cmos_sensor(0x8e, 0x69);
HI351_write_cmos_sensor(0x8f, 0x40); //Indoor 1169
HI351_write_cmos_sensor(0x90, 0x6a);
HI351_write_cmos_sensor(0x91, 0x80); //Indoor 116a Imp Lv2 High Gain
HI351_write_cmos_sensor(0x92, 0x6b);
HI351_write_cmos_sensor(0x93, 0x80); //Indoor 116b Imp Lv2 Middle Gain
HI351_write_cmos_sensor(0x94, 0x6c);
HI351_write_cmos_sensor(0x95, 0x80); //Indoor 116c Imp Lv2 Low Gain
HI351_write_cmos_sensor(0x96, 0x6d);
HI351_write_cmos_sensor(0x97, 0x80); //Indoor 116d
HI351_write_cmos_sensor(0x98, 0x6e);
HI351_write_cmos_sensor(0x99, 0x40); //Indoor 116e
HI351_write_cmos_sensor(0x9a, 0x6f);
HI351_write_cmos_sensor(0x9b, 0x80); //Indoor 116f Imp Lv3 Hi Gain
HI351_write_cmos_sensor(0x9c, 0x70);
HI351_write_cmos_sensor(0x9d, 0x60); //Indoor 1170 Imp Lv3 Middle Gain
HI351_write_cmos_sensor(0x9e, 0x71);
HI351_write_cmos_sensor(0x9f, 0x40); //Indoor 1171 Imp Lv3 Low Gain
HI351_write_cmos_sensor(0xa0, 0x72);
HI351_write_cmos_sensor(0xa1, 0x6e); //Indoor 1172
HI351_write_cmos_sensor(0xa2, 0x73);
HI351_write_cmos_sensor(0xa3, 0x3a); //Indoor 1173
HI351_write_cmos_sensor(0xa4, 0x74);
HI351_write_cmos_sensor(0xa5, 0x80); //Indoor 1174 Imp Lv4 Hi Gain
HI351_write_cmos_sensor(0xa6, 0x75);
HI351_write_cmos_sensor(0xa7, 0x80); //Indoor 1175 Imp Lv4 Middle Gain
HI351_write_cmos_sensor(0xa8, 0x76);
HI351_write_cmos_sensor(0xa9, 0x40); //Indoor 1176 Imp Lv4 Low Gain
HI351_write_cmos_sensor(0xaa, 0x77);
HI351_write_cmos_sensor(0xab, 0x6e); //Indoor 1177 Imp Lv5 Hi Th
HI351_write_cmos_sensor(0xac, 0x78);
HI351_write_cmos_sensor(0xad, 0x66); //Indoor 1178 Imp Lv5 Middle Th
HI351_write_cmos_sensor(0xae, 0x79);
HI351_write_cmos_sensor(0xaf, 0x60); //Indoor 1179 Imp Lv5 Hi Gain
HI351_write_cmos_sensor(0xb0, 0x7a);
HI351_write_cmos_sensor(0xb1, 0x30); //Indoor 117a Imp Lv5 Middle Gain
HI351_write_cmos_sensor(0xb2, 0x7b);
HI351_write_cmos_sensor(0xb3, 0x20); //Indoor 117b Imp Lv5 Low Gain
HI351_write_cmos_sensor(0xb4, 0x7c);
HI351_write_cmos_sensor(0xb5, 0x5c); //Indoor 117c Imp Lv6 Hi Th
HI351_write_cmos_sensor(0xb6, 0x7d);
HI351_write_cmos_sensor(0xb7, 0x30); //Indoor 117d Imp Lv6 Middle Th
HI351_write_cmos_sensor(0xb8, 0x7e);
HI351_write_cmos_sensor(0xb9, 0x60); //Indoor 117e Imp Lv6 Hi Gain
HI351_write_cmos_sensor(0xba, 0x7f);
HI351_write_cmos_sensor(0xbb, 0x60); //Indoor 117f Imp Lv6 Middle Gain
HI351_write_cmos_sensor(0xbc, 0x80);
HI351_write_cmos_sensor(0xbd, 0x20); //Indoor 1180 Imp Lv6 Low Gain
HI351_write_cmos_sensor(0xbe, 0x81);
HI351_write_cmos_sensor(0xbf, 0x62); //Indoor 1181
HI351_write_cmos_sensor(0xc0, 0x82);
HI351_write_cmos_sensor(0xc1, 0x26); //Indoor 1182
HI351_write_cmos_sensor(0xc2, 0x83);
HI351_write_cmos_sensor(0xc3, 0x60); //Indoor 1183 Imp Lv7 Hi Gain
HI351_write_cmos_sensor(0xc4, 0x84);
HI351_write_cmos_sensor(0xc5, 0x60); //Indoor 1184 Imp Lv7 Middle Gain
HI351_write_cmos_sensor(0xc6, 0x85);
HI351_write_cmos_sensor(0xc7, 0x60); //Indoor 1185 Imp Lv7 Low Gain
HI351_write_cmos_sensor(0xc8, 0x86);
HI351_write_cmos_sensor(0xc9, 0x62); //Indoor 1186
HI351_write_cmos_sensor(0xca, 0x87);
HI351_write_cmos_sensor(0xcb, 0x26); //Indoor 1187
HI351_write_cmos_sensor(0xcc, 0x88);
HI351_write_cmos_sensor(0xcd, 0x60); //Indoor 1188
HI351_write_cmos_sensor(0xce, 0x89);
HI351_write_cmos_sensor(0xcf, 0x60); //Indoor 1189
HI351_write_cmos_sensor(0xd0, 0x8a);
HI351_write_cmos_sensor(0xd1, 0x60); //Indoor 118a
HI351_write_cmos_sensor(0xd2, 0x90);
HI351_write_cmos_sensor(0xd3, 0x00); //Indoor 1190
HI351_write_cmos_sensor(0xd4, 0x91);
HI351_write_cmos_sensor(0xd5, 0x4e); //Indoor 1191
HI351_write_cmos_sensor(0xd6, 0x92);
HI351_write_cmos_sensor(0xd7, 0x00); //Indoor 1192
HI351_write_cmos_sensor(0xd8, 0x93);
HI351_write_cmos_sensor(0xd9, 0x16); //Indoor 1193
HI351_write_cmos_sensor(0xda, 0x94);
HI351_write_cmos_sensor(0xdb, 0x01); //Indoor 1194
HI351_write_cmos_sensor(0xdc, 0x95);
HI351_write_cmos_sensor(0xdd, 0x80); //Indoor 1195
HI351_write_cmos_sensor(0xde, 0x96);
HI351_write_cmos_sensor(0xdf, 0x55); //Indoor 1196
HI351_write_cmos_sensor(0xe0, 0x97);
HI351_write_cmos_sensor(0xe1, 0x8d); //Indoor 1197
HI351_write_cmos_sensor(0xe2, 0xb0);
HI351_write_cmos_sensor(0xe3, 0x60); //Indoor 11b0
HI351_write_cmos_sensor(0xe4, 0xb1);
HI351_write_cmos_sensor(0xe5, 0xb0); //Indoor 11b1
HI351_write_cmos_sensor(0xe6, 0xb2);
HI351_write_cmos_sensor(0xe7, 0x88); //Indoor 11b2
HI351_write_cmos_sensor(0xe8, 0xb3);
HI351_write_cmos_sensor(0xe9, 0x10); //Indoor 11b3
HI351_write_cmos_sensor(0xea, 0xb4);
HI351_write_cmos_sensor(0xeb, 0x04); //Indoor 11b4
HI351_write_cmos_sensor(0xec, 0x03);
HI351_write_cmos_sensor(0xed, 0x12);
HI351_write_cmos_sensor(0xee, 0x10);
HI351_write_cmos_sensor(0xef, 0x03); //Indoor 1210
HI351_write_cmos_sensor(0xf0, 0x11);
HI351_write_cmos_sensor(0xf1, 0x29); //Indoor 1211
HI351_write_cmos_sensor(0xf2, 0x12);
HI351_write_cmos_sensor(0xf3, 0x08); //Indoor 1212
HI351_write_cmos_sensor(0xf4, 0x40);
HI351_write_cmos_sensor(0xf5, 0x33); //Indoor 1240
HI351_write_cmos_sensor(0xf6, 0x41);
HI351_write_cmos_sensor(0xf7, 0x0a); //Indoor 1241
HI351_write_cmos_sensor(0xf8, 0x42);
HI351_write_cmos_sensor(0xf9, 0x6a); //Indoor 1242
HI351_write_cmos_sensor(0xfa, 0x43);
HI351_write_cmos_sensor(0xfb, 0x80); //Indoor 1243
HI351_write_cmos_sensor(0xfc, 0x44);
HI351_write_cmos_sensor(0xfd, 0x02); //Indoor 1244
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
HI351_write_cmos_sensor(0x03, 0xde);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x45);
HI351_write_cmos_sensor(0x11, 0x0a); //Indoor 1245
HI351_write_cmos_sensor(0x12, 0x46);
HI351_write_cmos_sensor(0x13, 0x80); //Indoor 1246
HI351_write_cmos_sensor(0x14, 0x60);
HI351_write_cmos_sensor(0x15, 0x21); //Indoor 1260
HI351_write_cmos_sensor(0x16, 0x61);
HI351_write_cmos_sensor(0x17, 0x0e); //Indoor 1261
HI351_write_cmos_sensor(0x18, 0x62);
HI351_write_cmos_sensor(0x19, 0x70); //Indoor 1262
HI351_write_cmos_sensor(0x1a, 0x63);
HI351_write_cmos_sensor(0x1b, 0x70); //Indoor 1263
HI351_write_cmos_sensor(0x1c, 0x64);
HI351_write_cmos_sensor(0x1d, 0x14); //Indoor 1264
HI351_write_cmos_sensor(0x1e, 0x65);
HI351_write_cmos_sensor(0x1f, 0x01); //Indoor 1265
HI351_write_cmos_sensor(0x20, 0x68);
HI351_write_cmos_sensor(0x21, 0x0a); //Indoor 1268
HI351_write_cmos_sensor(0x22, 0x69);
HI351_write_cmos_sensor(0x23, 0x04); //Indoor 1269
HI351_write_cmos_sensor(0x24, 0x6a);
HI351_write_cmos_sensor(0x25, 0x0a); //Indoor 126a
HI351_write_cmos_sensor(0x26, 0x6b);
HI351_write_cmos_sensor(0x27, 0x0a); //Indoor 126b
HI351_write_cmos_sensor(0x28, 0x6c);
HI351_write_cmos_sensor(0x29, 0x24); //Indoor 126c
HI351_write_cmos_sensor(0x2a, 0x6d);
HI351_write_cmos_sensor(0x2b, 0x01); //Indoor 126d
HI351_write_cmos_sensor(0x2c, 0x70);
HI351_write_cmos_sensor(0x2d, 0x01); //Indoor 1270
HI351_write_cmos_sensor(0x2e, 0x71);
HI351_write_cmos_sensor(0x2f, 0x3d); //Indoor 1271
HI351_write_cmos_sensor(0x30, 0x80);
HI351_write_cmos_sensor(0x31, 0x80); //Indoor 1280
HI351_write_cmos_sensor(0x32, 0x81);
HI351_write_cmos_sensor(0x33, 0x88); //Indoor 1281
HI351_write_cmos_sensor(0x34, 0x82);
HI351_write_cmos_sensor(0x35, 0x08); //Indoor 1282
HI351_write_cmos_sensor(0x36, 0x83);
HI351_write_cmos_sensor(0x37, 0x0c); //Indoor 1283
HI351_write_cmos_sensor(0x38, 0x84);
HI351_write_cmos_sensor(0x39, 0x90); //Indoor 1284
HI351_write_cmos_sensor(0x3a, 0x85);
HI351_write_cmos_sensor(0x3b, 0x92); //Indoor 1285
HI351_write_cmos_sensor(0x3c, 0x86);
HI351_write_cmos_sensor(0x3d, 0x20); //Indoor 1286
HI351_write_cmos_sensor(0x3e, 0x87);
HI351_write_cmos_sensor(0x3f, 0x00); //Indoor 1287
HI351_write_cmos_sensor(0x40, 0x88);
HI351_write_cmos_sensor(0x41, 0x70); //Indoor 1288
HI351_write_cmos_sensor(0x42, 0x89);
HI351_write_cmos_sensor(0x43, 0xaa); //Indoor 1289
HI351_write_cmos_sensor(0x44, 0x8a);
HI351_write_cmos_sensor(0x45, 0x50); //Indoor 128a
HI351_write_cmos_sensor(0x46, 0x8b);
HI351_write_cmos_sensor(0x47, 0x10); //Indoor 128b
HI351_write_cmos_sensor(0x48, 0x8c);
HI351_write_cmos_sensor(0x49, 0x04); //Indoor 128c
HI351_write_cmos_sensor(0x4a, 0x8d);
HI351_write_cmos_sensor(0x4b, 0x02); //Indoor 128d
HI351_write_cmos_sensor(0x4c, 0xe6);
HI351_write_cmos_sensor(0x4d, 0xff); //Indoor 12e6
HI351_write_cmos_sensor(0x4e, 0xe7);
HI351_write_cmos_sensor(0x4f, 0x18); //Indoor 12e7
HI351_write_cmos_sensor(0x50, 0xe8);
HI351_write_cmos_sensor(0x51, 0x20); //Indoor 12e8
HI351_write_cmos_sensor(0x52, 0xe9);
HI351_write_cmos_sensor(0x53, 0x06); //Indoor 12e9
HI351_write_cmos_sensor(0x54, 0x03);
HI351_write_cmos_sensor(0x55, 0x13);
HI351_write_cmos_sensor(0x56, 0x10);
HI351_write_cmos_sensor(0x57, 0x33); //Indoor 1310
HI351_write_cmos_sensor(0x58, 0x20);
HI351_write_cmos_sensor(0x59, 0x20); //Indoor 1320
HI351_write_cmos_sensor(0x5a, 0x21);
HI351_write_cmos_sensor(0x5b, 0x30); //Indoor 1321
HI351_write_cmos_sensor(0x5c, 0x22);
HI351_write_cmos_sensor(0x5d, 0x36); //Indoor 1322
HI351_write_cmos_sensor(0x5e, 0x23);
HI351_write_cmos_sensor(0x5f, 0x6a); //Indoor 1323
HI351_write_cmos_sensor(0x60, 0x24);
HI351_write_cmos_sensor(0x61, 0xa0); //Indoor 1324
HI351_write_cmos_sensor(0x62, 0x25);
HI351_write_cmos_sensor(0x63, 0xc0); //Indoor 1325
HI351_write_cmos_sensor(0x64, 0x26);
HI351_write_cmos_sensor(0x65, 0xe0); //Indoor 1326
HI351_write_cmos_sensor(0x66, 0x27);
HI351_write_cmos_sensor(0x67, 0x02); //Indoor 1327
HI351_write_cmos_sensor(0x68, 0x28);
HI351_write_cmos_sensor(0x69, 0x03); //Indoor 1328
HI351_write_cmos_sensor(0x6a, 0x29);
HI351_write_cmos_sensor(0x6b, 0x03); //Indoor 1329
HI351_write_cmos_sensor(0x6c, 0x2a);
HI351_write_cmos_sensor(0x6d, 0x10); //Indoor 132a
HI351_write_cmos_sensor(0x6e, 0x2b);
HI351_write_cmos_sensor(0x6f, 0x10); //Indoor 132b
HI351_write_cmos_sensor(0x70, 0x2c);
HI351_write_cmos_sensor(0x71, 0x04); //Indoor 132c
HI351_write_cmos_sensor(0x72, 0x2d);
HI351_write_cmos_sensor(0x73, 0x03); //Indoor 132d
HI351_write_cmos_sensor(0x74, 0x2e);
HI351_write_cmos_sensor(0x75, 0x03); //Indoor 132e
HI351_write_cmos_sensor(0x76, 0x2f);
HI351_write_cmos_sensor(0x77, 0x18); //Indoor 132f
HI351_write_cmos_sensor(0x78, 0x30);
HI351_write_cmos_sensor(0x79, 0x03); //Indoor 1330
HI351_write_cmos_sensor(0x7a, 0x31);
HI351_write_cmos_sensor(0x7b, 0x03); //Indoor 1331
HI351_write_cmos_sensor(0x7c, 0x32);
HI351_write_cmos_sensor(0x7d, 0x03); //Indoor 1332
HI351_write_cmos_sensor(0x7e, 0x33);
HI351_write_cmos_sensor(0x7f, 0x40); //Indoor 1333
HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x81, 0x80); //Indoor 1334
HI351_write_cmos_sensor(0x82, 0x35);
HI351_write_cmos_sensor(0x83, 0x00); //Indoor 1335
HI351_write_cmos_sensor(0x84, 0x36);
HI351_write_cmos_sensor(0x85, 0xf0); //Indoor 1336
HI351_write_cmos_sensor(0x86, 0xa0);
HI351_write_cmos_sensor(0x87, 0x07); //Indoor 13a0
HI351_write_cmos_sensor(0x88, 0xa8);
HI351_write_cmos_sensor(0x89, 0x24); //Indoor 13a8 Cb_Filter
HI351_write_cmos_sensor(0x8a, 0xa9);
HI351_write_cmos_sensor(0x8b, 0x24); //Indoor 13a9 Cr_Filter
HI351_write_cmos_sensor(0x8c, 0xaa);
HI351_write_cmos_sensor(0x8d, 0x20); //Indoor 13aa
HI351_write_cmos_sensor(0x8e, 0xab);
HI351_write_cmos_sensor(0x8f, 0x02); //Indoor 13ab
HI351_write_cmos_sensor(0x90, 0xc0);
HI351_write_cmos_sensor(0x91, 0x27); //Indoor 13c0
HI351_write_cmos_sensor(0x92, 0xc2);
HI351_write_cmos_sensor(0x93, 0x08); //Indoor 13c2
HI351_write_cmos_sensor(0x94, 0xc3);
HI351_write_cmos_sensor(0x95, 0x08); //Indoor 13c3
HI351_write_cmos_sensor(0x96, 0xc4);
HI351_write_cmos_sensor(0x97, 0x40); //Indoor 13c4
HI351_write_cmos_sensor(0x98, 0xc5);
HI351_write_cmos_sensor(0x99, 0x38); //Indoor 13c5
HI351_write_cmos_sensor(0x9a, 0xc6);
HI351_write_cmos_sensor(0x9b, 0xf0); //Indoor 13c6
HI351_write_cmos_sensor(0x9c, 0xc7);
HI351_write_cmos_sensor(0x9d, 0x10); //Indoor 13c7
HI351_write_cmos_sensor(0x9e, 0xc8);
HI351_write_cmos_sensor(0x9f, 0x44); //Indoor 13c8
HI351_write_cmos_sensor(0xa0, 0xc9);
HI351_write_cmos_sensor(0xa1, 0x87); //Indoor 13c9
HI351_write_cmos_sensor(0xa2, 0xca);
HI351_write_cmos_sensor(0xa3, 0xff); //Indoor 13ca
HI351_write_cmos_sensor(0xa4, 0xcb);
HI351_write_cmos_sensor(0xa5, 0x20); //Indoor 13cb
HI351_write_cmos_sensor(0xa6, 0xcc);
HI351_write_cmos_sensor(0xa7, 0x61); //Indoor 13cc
HI351_write_cmos_sensor(0xa8, 0xcd);
HI351_write_cmos_sensor(0xa9, 0x87); //Indoor 13cd
HI351_write_cmos_sensor(0xaa, 0xce);
HI351_write_cmos_sensor(0xab, 0x8a); //Indoor 13ce
HI351_write_cmos_sensor(0xac, 0xcf);
HI351_write_cmos_sensor(0xad, 0xa5); //Indoor 13cf
HI351_write_cmos_sensor(0xae, 0x03);
HI351_write_cmos_sensor(0xaf, 0x14);
HI351_write_cmos_sensor(0xb0, 0x11);
HI351_write_cmos_sensor(0xb1, 0x02); //Indoor 1410
HI351_write_cmos_sensor(0xb2, 0x11);
HI351_write_cmos_sensor(0xb3, 0x02); //Indoor 1411 TOP L_clip
HI351_write_cmos_sensor(0xb4, 0x12);
HI351_write_cmos_sensor(0xb5, 0x40); //Indoor 1412
HI351_write_cmos_sensor(0xb6, 0x13);
HI351_write_cmos_sensor(0xb7, 0x98); //Indoor 1413
HI351_write_cmos_sensor(0xb8, 0x14);
HI351_write_cmos_sensor(0xb9, 0x3a); //Indoor 1414
HI351_write_cmos_sensor(0xba, 0x15);
HI351_write_cmos_sensor(0xbb, 0x40); //Indoor 1415  Positive High Gain
HI351_write_cmos_sensor(0xbc, 0x16);
HI351_write_cmos_sensor(0xbd, 0x40); //Indoor 1416  Positive Middle Gain
HI351_write_cmos_sensor(0xbe, 0x17);
HI351_write_cmos_sensor(0xbf, 0x40); //Indoor 1417  Positive Low Gain
HI351_write_cmos_sensor(0xc0, 0x18);
HI351_write_cmos_sensor(0xc1, 0x68); //Indoor 1418  Negative High Gain
HI351_write_cmos_sensor(0xc2, 0x19);
HI351_write_cmos_sensor(0xc3, 0x70); //Indoor 1419  Negative Middle Gain
HI351_write_cmos_sensor(0xc4, 0x1a);
HI351_write_cmos_sensor(0xc5, 0x70); //Indoor 141a  Negative Low Gain
HI351_write_cmos_sensor(0xc6, 0x20);
HI351_write_cmos_sensor(0xc7, 0x82); //Indoor 1420  s_diff L_clip
HI351_write_cmos_sensor(0xc8, 0x21);
HI351_write_cmos_sensor(0xc9, 0x03); //Indoor 1421
HI351_write_cmos_sensor(0xca, 0x22);
HI351_write_cmos_sensor(0xcb, 0x05); //Indoor 1422
HI351_write_cmos_sensor(0xcc, 0x23);
HI351_write_cmos_sensor(0xcd, 0x07); //Indoor 1423
HI351_write_cmos_sensor(0xce, 0x24);
HI351_write_cmos_sensor(0xcf, 0x0a); //Indoor 1424
HI351_write_cmos_sensor(0xd0, 0x25);
HI351_write_cmos_sensor(0xd1, 0x46); //Indoor 1425
HI351_write_cmos_sensor(0xd2, 0x26);
HI351_write_cmos_sensor(0xd3, 0x32); //Indoor 1426
HI351_write_cmos_sensor(0xd4, 0x27);
HI351_write_cmos_sensor(0xd5, 0x1e); //Indoor 1427
HI351_write_cmos_sensor(0xd6, 0x28);
HI351_write_cmos_sensor(0xd7, 0x10); //Indoor 1428
HI351_write_cmos_sensor(0xd8, 0x29);
HI351_write_cmos_sensor(0xd9, 0x00); //Indoor 1429
HI351_write_cmos_sensor(0xda, 0x2a);
HI351_write_cmos_sensor(0xdb, 0x18); //Indoor 142a
HI351_write_cmos_sensor(0xdc, 0x2b);
HI351_write_cmos_sensor(0xdd, 0x18); //Indoor 142b
HI351_write_cmos_sensor(0xde, 0x2c);
HI351_write_cmos_sensor(0xdf, 0x18); //Indoor 142c
HI351_write_cmos_sensor(0xe0, 0x2d);
HI351_write_cmos_sensor(0xe1, 0x30); //Indoor 142d
HI351_write_cmos_sensor(0xe2, 0x2e);
HI351_write_cmos_sensor(0xe3, 0x30); //Indoor 142e
HI351_write_cmos_sensor(0xe4, 0x2f);
HI351_write_cmos_sensor(0xe5, 0x30); //Indoor 142f
HI351_write_cmos_sensor(0xe6, 0x30);
HI351_write_cmos_sensor(0xe7, 0x82); //Indoor 1430 Ldiff_L_cip
HI351_write_cmos_sensor(0xe8, 0x31);
HI351_write_cmos_sensor(0xe9, 0x02); //Indoor 1431
HI351_write_cmos_sensor(0xea, 0x32);
HI351_write_cmos_sensor(0xeb, 0x04); //Indoor 1432
HI351_write_cmos_sensor(0xec, 0x33);
HI351_write_cmos_sensor(0xed, 0x04); //Indoor 1433
HI351_write_cmos_sensor(0xee, 0x34);
HI351_write_cmos_sensor(0xef, 0x0a); //Indoor 1434
HI351_write_cmos_sensor(0xf0, 0x35);
HI351_write_cmos_sensor(0xf1, 0x46); //Indoor 1435
HI351_write_cmos_sensor(0xf2, 0x36);
HI351_write_cmos_sensor(0xf3, 0x32); //Indoor 1436
HI351_write_cmos_sensor(0xf4, 0x37);
HI351_write_cmos_sensor(0xf5, 0x28); //Indoor 1437
HI351_write_cmos_sensor(0xf6, 0x38);
HI351_write_cmos_sensor(0xf7, 0x12); //Indoor 1438
HI351_write_cmos_sensor(0xf8, 0x39);
HI351_write_cmos_sensor(0xf9, 0x00); //Indoor 1439
HI351_write_cmos_sensor(0xfa, 0x3a);
HI351_write_cmos_sensor(0xfb, 0x20); //Indoor 143a
HI351_write_cmos_sensor(0xfc, 0x3b);
HI351_write_cmos_sensor(0xfd, 0x30); //Indoor 143b
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end

HI351_write_cmos_sensor(0x03, 0xdf);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x3c);
HI351_write_cmos_sensor(0x11, 0x20); //Indoor 143c
HI351_write_cmos_sensor(0x12, 0x3d);
HI351_write_cmos_sensor(0x13, 0x18); //Indoor 143d
HI351_write_cmos_sensor(0x14, 0x3e);
HI351_write_cmos_sensor(0x15, 0x28); //Indoor 143e
HI351_write_cmos_sensor(0x16, 0x3f);
HI351_write_cmos_sensor(0x17, 0x14); //Indoor 143f
HI351_write_cmos_sensor(0x18, 0x40);
HI351_write_cmos_sensor(0x19, 0x84); //Indoor 1440  Mdiff Low Clip
HI351_write_cmos_sensor(0x1a, 0x41);
HI351_write_cmos_sensor(0x1b, 0x10); //Indoor 1441
HI351_write_cmos_sensor(0x1c, 0x42);
HI351_write_cmos_sensor(0x1d, 0x70); //Indoor 1442
HI351_write_cmos_sensor(0x1e, 0x43);
HI351_write_cmos_sensor(0x1f, 0x20); //Indoor 1443
HI351_write_cmos_sensor(0x20, 0x44);
HI351_write_cmos_sensor(0x21, 0x18); //Indoor 1444
HI351_write_cmos_sensor(0x22, 0x45);
HI351_write_cmos_sensor(0x23, 0x18); //Indoor 1445
HI351_write_cmos_sensor(0x24, 0x46);
HI351_write_cmos_sensor(0x25, 0x10); //Indoor 1446
HI351_write_cmos_sensor(0x26, 0x47);
HI351_write_cmos_sensor(0x27, 0x18); //Indoor 1447
HI351_write_cmos_sensor(0x28, 0x48);
HI351_write_cmos_sensor(0x29, 0x0a); //Indoor 1448
HI351_write_cmos_sensor(0x2a, 0x49);
HI351_write_cmos_sensor(0x2b, 0x10); //Indoor 1449
HI351_write_cmos_sensor(0x2c, 0x50);
HI351_write_cmos_sensor(0x2d, 0x84); //Indoor 1450 Hdiff Low Clip
HI351_write_cmos_sensor(0x2e, 0x51);
HI351_write_cmos_sensor(0x2f, 0x30); //Indoor 1451 hclip
HI351_write_cmos_sensor(0x30, 0x52);
HI351_write_cmos_sensor(0x31, 0xb0); //Indoor 1452
HI351_write_cmos_sensor(0x32, 0x53);
HI351_write_cmos_sensor(0x33, 0x37); //Indoor 1453
HI351_write_cmos_sensor(0x34, 0x54);
HI351_write_cmos_sensor(0x35, 0x33); //Indoor 1454
HI351_write_cmos_sensor(0x36, 0x65);
HI351_write_cmos_sensor(0x37, 0x33); //Indoor 1455
HI351_write_cmos_sensor(0x38, 0x66);
HI351_write_cmos_sensor(0x39, 0x33); //Indoor 1456
HI351_write_cmos_sensor(0x3a, 0x57);
HI351_write_cmos_sensor(0x3b, 0x10); //Indoor 1457
HI351_write_cmos_sensor(0x3c, 0x68);
HI351_write_cmos_sensor(0x3d, 0x20); //Indoor 1458
HI351_write_cmos_sensor(0x3e, 0x69);
HI351_write_cmos_sensor(0x3f, 0x20); //Indoor 1459
HI351_write_cmos_sensor(0x40, 0x60);
HI351_write_cmos_sensor(0x41, 0x01); //Indoor 1460
HI351_write_cmos_sensor(0x42, 0x61);
HI351_write_cmos_sensor(0x43, 0xa0); //Indoor 1461
HI351_write_cmos_sensor(0x44, 0x62);
HI351_write_cmos_sensor(0x45, 0x98); //Indoor 1462
HI351_write_cmos_sensor(0x46, 0x63);
HI351_write_cmos_sensor(0x47, 0xe4); //Indoor 1463
HI351_write_cmos_sensor(0x48, 0x64);
HI351_write_cmos_sensor(0x49, 0xa4); //Indoor 1464
HI351_write_cmos_sensor(0x4a, 0x65);
HI351_write_cmos_sensor(0x4b, 0x7d); //Indoor 1465
HI351_write_cmos_sensor(0x4c, 0x66);
HI351_write_cmos_sensor(0x4d, 0x4b); //Indoor 1466
HI351_write_cmos_sensor(0x4e, 0x70);
HI351_write_cmos_sensor(0x4f, 0x10); //Indoor 1470
HI351_write_cmos_sensor(0x50, 0x71);
HI351_write_cmos_sensor(0x51, 0x10); //Indoor 1471
HI351_write_cmos_sensor(0x52, 0x72);
HI351_write_cmos_sensor(0x53, 0x10); //Indoor 1472
HI351_write_cmos_sensor(0x54, 0x73);
HI351_write_cmos_sensor(0x55, 0x10); //Indoor 1473
HI351_write_cmos_sensor(0x56, 0x74);
HI351_write_cmos_sensor(0x57, 0x10); //Indoor 1474
HI351_write_cmos_sensor(0x58, 0x75);
HI351_write_cmos_sensor(0x59, 0x10); //Indoor 1475
HI351_write_cmos_sensor(0x5a, 0x76);
HI351_write_cmos_sensor(0x5b, 0x38); //Indoor 1476    green sharp pos High
HI351_write_cmos_sensor(0x5c, 0x77);
HI351_write_cmos_sensor(0x5d, 0x38); //Indoor 1477    green sharp pos Middle
HI351_write_cmos_sensor(0x5e, 0x78);
HI351_write_cmos_sensor(0x5f, 0x38); //Indoor 1478    green sharp pos Low
HI351_write_cmos_sensor(0x60, 0x79);
HI351_write_cmos_sensor(0x61, 0x70); //Indoor 1479    green sharp nega High
HI351_write_cmos_sensor(0x62, 0x7a);
HI351_write_cmos_sensor(0x63, 0x70); //Indoor 147a    green sharp nega Middle
HI351_write_cmos_sensor(0x64, 0x7b);
HI351_write_cmos_sensor(0x65, 0x70); //Indoor 147b    green sharp nega Low
HI351_write_cmos_sensor(0x66, 0x03);
HI351_write_cmos_sensor(0x67, 0x10); //10 page
HI351_write_cmos_sensor(0x68, 0x60);
HI351_write_cmos_sensor(0x69, 0x01); //1060
HI351_write_cmos_sensor(0x6a, 0x70);
HI351_write_cmos_sensor(0x6b, 0x0c); //1070
HI351_write_cmos_sensor(0x6c, 0x71);
HI351_write_cmos_sensor(0x6d, 0x00); //1071
HI351_write_cmos_sensor(0x6e, 0x72);
HI351_write_cmos_sensor(0x6f, 0x83); //1072
HI351_write_cmos_sensor(0x70, 0x73);
HI351_write_cmos_sensor(0x71, 0x99); //1073
HI351_write_cmos_sensor(0x72, 0x74);
HI351_write_cmos_sensor(0x73, 0x1b); //1074
HI351_write_cmos_sensor(0x74, 0x75);
HI351_write_cmos_sensor(0x75, 0x0b); //1075
HI351_write_cmos_sensor(0x76, 0x76);
HI351_write_cmos_sensor(0x77, 0x3c); //1076
HI351_write_cmos_sensor(0x78, 0x77);
HI351_write_cmos_sensor(0x79, 0x43); //1077
HI351_write_cmos_sensor(0x7a, 0x78);
HI351_write_cmos_sensor(0x7b, 0x33); //1078
HI351_write_cmos_sensor(0x7c, 0x79);
HI351_write_cmos_sensor(0x7d, 0x4c); //1079
HI351_write_cmos_sensor(0x7e, 0x7a);
HI351_write_cmos_sensor(0x7f, 0xcc); //107a
HI351_write_cmos_sensor(0x80, 0x7b);
HI351_write_cmos_sensor(0x81, 0x49); //107b
HI351_write_cmos_sensor(0x82, 0x7c);
HI351_write_cmos_sensor(0x83, 0x99); //107c
HI351_write_cmos_sensor(0x84, 0x7d);
HI351_write_cmos_sensor(0x85, 0x0e); //107d
HI351_write_cmos_sensor(0x86, 0x7e);
HI351_write_cmos_sensor(0x87, 0x1e); //107e
HI351_write_cmos_sensor(0x88, 0x7f);
HI351_write_cmos_sensor(0x89, 0x3c); //107f

HI351_write_cmos_sensor(0x8a, 0x03); //16 page
HI351_write_cmos_sensor(0x8b, 0x16);
HI351_write_cmos_sensor(0x8c, 0x8a); //168a
HI351_write_cmos_sensor(0x8d, 0x68);
HI351_write_cmos_sensor(0x8e, 0x8b); //168b
HI351_write_cmos_sensor(0x8f, 0x7c);
HI351_write_cmos_sensor(0x90, 0x8c); //168c
HI351_write_cmos_sensor(0x91, 0x7f);
HI351_write_cmos_sensor(0x92, 0x8d); //168d
HI351_write_cmos_sensor(0x93, 0x7f);
HI351_write_cmos_sensor(0x94, 0x8e); //168e
HI351_write_cmos_sensor(0x95, 0x7f);
HI351_write_cmos_sensor(0x96, 0x8f); //168f
HI351_write_cmos_sensor(0x97, 0x7f);
HI351_write_cmos_sensor(0x98, 0x90); //1690
HI351_write_cmos_sensor(0x99, 0x7f);
HI351_write_cmos_sensor(0x9a, 0x91); //1691
HI351_write_cmos_sensor(0x9b, 0x7f);
HI351_write_cmos_sensor(0x9c, 0x92); //1692
HI351_write_cmos_sensor(0x9d, 0x7f);
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
//////////////////
// e0 Page (DMA Dark1)
//////////////////
//Page 0xe0
HI351_write_cmos_sensor(0x03, 0xe0);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x03);
HI351_write_cmos_sensor(0x11, 0x11);
HI351_write_cmos_sensor(0x12, 0x10);
HI351_write_cmos_sensor(0x13, 0x13); //Dark1 1110
HI351_write_cmos_sensor(0x14, 0x11);
HI351_write_cmos_sensor(0x15, 0x08); //Dark1 1111
HI351_write_cmos_sensor(0x16, 0x12);
HI351_write_cmos_sensor(0x17, 0x1e); //Dark1 1112
HI351_write_cmos_sensor(0x18, 0x13);
HI351_write_cmos_sensor(0x19, 0x0e); //Dark1 1113
HI351_write_cmos_sensor(0x1a, 0x14);
HI351_write_cmos_sensor(0x1b, 0x31); //Dark1 1114
HI351_write_cmos_sensor(0x1c, 0x30);
HI351_write_cmos_sensor(0x1d, 0x20); //Dark1 1130
HI351_write_cmos_sensor(0x1e, 0x31);
HI351_write_cmos_sensor(0x1f, 0x20); //Dark1 1131
HI351_write_cmos_sensor(0x20, 0x32);
HI351_write_cmos_sensor(0x21, 0x52); //Dark1 1132
HI351_write_cmos_sensor(0x22, 0x33);
HI351_write_cmos_sensor(0x23, 0x3b); //Dark1 1133
HI351_write_cmos_sensor(0x24, 0x34);
HI351_write_cmos_sensor(0x25, 0x1d); //Dark1 1134
HI351_write_cmos_sensor(0x26, 0x35);
HI351_write_cmos_sensor(0x27, 0x21); //Dark1 1135
HI351_write_cmos_sensor(0x28, 0x36);
HI351_write_cmos_sensor(0x29, 0x1b); //Dark1 1136
HI351_write_cmos_sensor(0x2a, 0x37);
HI351_write_cmos_sensor(0x2b, 0x21); //Dark1 1137
HI351_write_cmos_sensor(0x2c, 0x38);
HI351_write_cmos_sensor(0x2d, 0x18); //Dark1 1138
HI351_write_cmos_sensor(0x2e, 0x39);
HI351_write_cmos_sensor(0x2f, 0x18); //Dark1 1139 R2 lvl1 gain
HI351_write_cmos_sensor(0x30, 0x3a);
HI351_write_cmos_sensor(0x31, 0x20); //Dark1 113a R2 lvl2 gain
HI351_write_cmos_sensor(0x32, 0x3b);
HI351_write_cmos_sensor(0x33, 0x28); //Dark1 113b R2 lvl3 gain
HI351_write_cmos_sensor(0x34, 0x3c);
HI351_write_cmos_sensor(0x35, 0x20); //Dark1 113c R2 lvl4 gain
HI351_write_cmos_sensor(0x36, 0x3d);
HI351_write_cmos_sensor(0x37, 0x20); //Dark1 113d R2 lvl5 gain
HI351_write_cmos_sensor(0x38, 0x3e);
HI351_write_cmos_sensor(0x39, 0x1e); //Dark1 113e R2 lvl6 gain
HI351_write_cmos_sensor(0x3a, 0x3f);
HI351_write_cmos_sensor(0x3b, 0x1e); //Dark1 113f R2 lvl7 gain
HI351_write_cmos_sensor(0x3c, 0x40);
HI351_write_cmos_sensor(0x3d, 0x1e); //Dark1 1140 R2 lvl8 gain
HI351_write_cmos_sensor(0x3e, 0x41);
HI351_write_cmos_sensor(0x3f, 0x10); //Dark1 1141 R2 Lvl1 offset
HI351_write_cmos_sensor(0x40, 0x42);
HI351_write_cmos_sensor(0x41, 0x10); //Dark1 1142 R2 Lvl2 offset
HI351_write_cmos_sensor(0x42, 0x43);
HI351_write_cmos_sensor(0x43, 0x20); //Dark1 1143 R2 Lvl3 offset
HI351_write_cmos_sensor(0x44, 0x44);
HI351_write_cmos_sensor(0x45, 0x2a); //Dark1 1144 R2 Lvl4 offset
HI351_write_cmos_sensor(0x46, 0x45);
HI351_write_cmos_sensor(0x47, 0x30); //Dark1 1145 R2 Lvl5 offset
HI351_write_cmos_sensor(0x48, 0x46);
HI351_write_cmos_sensor(0x49, 0x50); //Dark1 1146 R2 Lvl6 offset
HI351_write_cmos_sensor(0x4a, 0x47);
HI351_write_cmos_sensor(0x4b, 0x40); //Dark1 1147 R2 Lvl7 offset
HI351_write_cmos_sensor(0x4c, 0x48);
HI351_write_cmos_sensor(0x4d, 0x40); //Dark1 1148 R2 Lvl8 offset
HI351_write_cmos_sensor(0x4e, 0x49);
HI351_write_cmos_sensor(0x4f, 0x60); //Dark1 1149 Lv1 h_clip
HI351_write_cmos_sensor(0x50, 0x4a);
HI351_write_cmos_sensor(0x51, 0x80); //Dark1 114a Lv2 h_clip
HI351_write_cmos_sensor(0x52, 0x4b);
HI351_write_cmos_sensor(0x53, 0xa0); //Dark1 114b Lv3 h_clip
HI351_write_cmos_sensor(0x54, 0x4c);
HI351_write_cmos_sensor(0x55, 0xa0); //Dark1 114c Lv4 h_clip
HI351_write_cmos_sensor(0x56, 0x4d);
HI351_write_cmos_sensor(0x57, 0xa0); //Dark1 114d Lv5 h_clip
HI351_write_cmos_sensor(0x58, 0x4e);
HI351_write_cmos_sensor(0x59, 0x90); //Dark1 114e Lv6 h_clip
HI351_write_cmos_sensor(0x5a, 0x4f);
HI351_write_cmos_sensor(0x5b, 0x90); //Dark1 114f Lv7 h_clip
HI351_write_cmos_sensor(0x5c, 0x50);
HI351_write_cmos_sensor(0x5d, 0x90); //Dark1 1150 Lv8 h_clip
HI351_write_cmos_sensor(0x5e, 0x51);
HI351_write_cmos_sensor(0x5f, 0x68); //Dark1 1151 color gain start
HI351_write_cmos_sensor(0x60, 0x52);
HI351_write_cmos_sensor(0x61, 0x68); //Dark1 1152
HI351_write_cmos_sensor(0x62, 0x53);
HI351_write_cmos_sensor(0x63, 0x68); //Dark1 1153
HI351_write_cmos_sensor(0x64, 0x54);
HI351_write_cmos_sensor(0x65, 0x68); //Dark1 1154
HI351_write_cmos_sensor(0x66, 0x55);
HI351_write_cmos_sensor(0x67, 0x68); //Dark1 1155
HI351_write_cmos_sensor(0x68, 0x56);
HI351_write_cmos_sensor(0x69, 0x68); //Dark1 1156
HI351_write_cmos_sensor(0x6a, 0x57);
HI351_write_cmos_sensor(0x6b, 0x68); //Dark1 1157
HI351_write_cmos_sensor(0x6c, 0x58);
HI351_write_cmos_sensor(0x6d, 0x68); //Dark1 1158 color gain end
HI351_write_cmos_sensor(0x6e, 0x59);
HI351_write_cmos_sensor(0x6f, 0x70); //Dark1 1159 color ofs lmt start
HI351_write_cmos_sensor(0x70, 0x5a);
HI351_write_cmos_sensor(0x71, 0x70); //Dark1 115a
HI351_write_cmos_sensor(0x72, 0x5b);
HI351_write_cmos_sensor(0x73, 0x70); //Dark1 115b
HI351_write_cmos_sensor(0x74, 0x5c);
HI351_write_cmos_sensor(0x75, 0x70); //Dark1 115c
HI351_write_cmos_sensor(0x76, 0x5d);
HI351_write_cmos_sensor(0x77, 0x70); //Dark1 115d
HI351_write_cmos_sensor(0x78, 0x5e);
HI351_write_cmos_sensor(0x79, 0x70); //Dark1 115e
HI351_write_cmos_sensor(0x7a, 0x5f);
HI351_write_cmos_sensor(0x7b, 0x70); //Dark1 115f
HI351_write_cmos_sensor(0x7c, 0x60);
HI351_write_cmos_sensor(0x7d, 0x70); //Dark1 1160 color ofs lmt end
HI351_write_cmos_sensor(0x7e, 0x61);
HI351_write_cmos_sensor(0x7f, 0xc0); //Dark1 1161
HI351_write_cmos_sensor(0x80, 0x62);
HI351_write_cmos_sensor(0x81, 0xf0); //Dark1 1162
HI351_write_cmos_sensor(0x82, 0x63);
HI351_write_cmos_sensor(0x83, 0x80); //Dark1 1163
HI351_write_cmos_sensor(0x84, 0x64);
HI351_write_cmos_sensor(0x85, 0x40); //Dark1 1164
HI351_write_cmos_sensor(0x86, 0x65);
HI351_write_cmos_sensor(0x87, 0x40); //Dark1 1165
HI351_write_cmos_sensor(0x88, 0x66);
HI351_write_cmos_sensor(0x89, 0x40); //Dark1 1166
HI351_write_cmos_sensor(0x8a, 0x67);
HI351_write_cmos_sensor(0x8b, 0x40); //Dark1 1167
HI351_write_cmos_sensor(0x8c, 0x68);
HI351_write_cmos_sensor(0x8d, 0x80); //Dark1 1168
HI351_write_cmos_sensor(0x8e, 0x69);
HI351_write_cmos_sensor(0x8f, 0x40); //Dark1 1169
HI351_write_cmos_sensor(0x90, 0x6a);
HI351_write_cmos_sensor(0x91, 0x40); //Dark1 116a Imp Lv2 High Gain
HI351_write_cmos_sensor(0x92, 0x6b);
HI351_write_cmos_sensor(0x93, 0x40); //Dark1 116b Imp Lv2 Middle Gain
HI351_write_cmos_sensor(0x94, 0x6c);
HI351_write_cmos_sensor(0x95, 0x40); //Dark1 116c Imp Lv2 Low Gain
HI351_write_cmos_sensor(0x96, 0x6d);
HI351_write_cmos_sensor(0x97, 0x80); //Dark1 116d
HI351_write_cmos_sensor(0x98, 0x6e);
HI351_write_cmos_sensor(0x99, 0x40); //Dark1 116e
HI351_write_cmos_sensor(0x9a, 0x6f);
HI351_write_cmos_sensor(0x9b, 0x50); //Dark1 116f Imp Lv3 Hi Gain
HI351_write_cmos_sensor(0x9c, 0x70);
HI351_write_cmos_sensor(0x9d, 0x50); //Dark1 1170 Imp Lv3 Middle Gain
HI351_write_cmos_sensor(0x9e, 0x71);
HI351_write_cmos_sensor(0x9f, 0x50); //Dark1 1171 Imp Lv3 Low Gain
HI351_write_cmos_sensor(0xa0, 0x72);
HI351_write_cmos_sensor(0xa1, 0x6e); //Dark1 1172
HI351_write_cmos_sensor(0xa2, 0x73);
HI351_write_cmos_sensor(0xa3, 0x3a); //Dark1 1173
HI351_write_cmos_sensor(0xa4, 0x74);
HI351_write_cmos_sensor(0xa5, 0x50); //Dark1 1174 Imp Lv4 Hi Gain
HI351_write_cmos_sensor(0xa6, 0x75);
HI351_write_cmos_sensor(0xa7, 0x50); //Dark1 1175 Imp Lv4 Middle Gain
HI351_write_cmos_sensor(0xa8, 0x76);
HI351_write_cmos_sensor(0xa9, 0x50); //Dark1 1176 Imp Lv4 Low Gain
HI351_write_cmos_sensor(0xaa, 0x77);
HI351_write_cmos_sensor(0xab, 0x6e); //Dark1 1177 Imp Lv5 Hi Th
HI351_write_cmos_sensor(0xac, 0x78);
HI351_write_cmos_sensor(0xad, 0x66); //Dark1 1178 Imp Lv5 Middle Th
HI351_write_cmos_sensor(0xae, 0x79);
HI351_write_cmos_sensor(0xaf, 0x40); //Dark1 1179 Imp Lv5 Hi Gain
HI351_write_cmos_sensor(0xb0, 0x7a);
HI351_write_cmos_sensor(0xb1, 0x40); //Dark1 117a Imp Lv5 Middle Gain
HI351_write_cmos_sensor(0xb2, 0x7b);
HI351_write_cmos_sensor(0xb3, 0x40); //Dark1 117b Imp Lv5 Low Gain
HI351_write_cmos_sensor(0xb4, 0x7c);
HI351_write_cmos_sensor(0xb5, 0x5c); //Dark1 117c Imp Lv6 Hi Th
HI351_write_cmos_sensor(0xb6, 0x7d);
HI351_write_cmos_sensor(0xb7, 0x30); //Dark1 117d Imp Lv6 Middle Th
HI351_write_cmos_sensor(0xb8, 0x7e);
HI351_write_cmos_sensor(0xb9, 0x40); //Dark1 117e Imp Lv6 Hi Gain
HI351_write_cmos_sensor(0xba, 0x7f);
HI351_write_cmos_sensor(0xbb, 0x40); //Dark1 117f Imp Lv6 Middle Gain
HI351_write_cmos_sensor(0xbc, 0x80);
HI351_write_cmos_sensor(0xbd, 0x40); //Dark1 1180 Imp Lv6 Low Gain
HI351_write_cmos_sensor(0xbe, 0x81);
HI351_write_cmos_sensor(0xbf, 0x62); //Dark1 1181
HI351_write_cmos_sensor(0xc0, 0x82);
HI351_write_cmos_sensor(0xc1, 0x26); //Dark1 1182
HI351_write_cmos_sensor(0xc2, 0x83);
HI351_write_cmos_sensor(0xc3, 0x40); //Dark1 1183 Imp Lv7 Hi Gain
HI351_write_cmos_sensor(0xc4, 0x84);
HI351_write_cmos_sensor(0xc5, 0x40); //Dark1 1184 Imp Lv7 Middle Gain
HI351_write_cmos_sensor(0xc6, 0x85);
HI351_write_cmos_sensor(0xc7, 0x40); //Dark1 1185 Imp Lv7 Low Gain
HI351_write_cmos_sensor(0xc8, 0x86);
HI351_write_cmos_sensor(0xc9, 0x62); //Dark1 1186
HI351_write_cmos_sensor(0xca, 0x87);
HI351_write_cmos_sensor(0xcb, 0x26); //Dark1 1187
HI351_write_cmos_sensor(0xcc, 0x88);
HI351_write_cmos_sensor(0xcd, 0x30); //Dark1 1188
HI351_write_cmos_sensor(0xce, 0x89);
HI351_write_cmos_sensor(0xcf, 0x30); //Dark1 1189
HI351_write_cmos_sensor(0xd0, 0x8a);
HI351_write_cmos_sensor(0xd1, 0x30); //Dark1 118a
HI351_write_cmos_sensor(0xd2, 0x90);
HI351_write_cmos_sensor(0xd3, 0x00); //Dark1 1190
HI351_write_cmos_sensor(0xd4, 0x91);
HI351_write_cmos_sensor(0xd5, 0x4e); //Dark1 1191
HI351_write_cmos_sensor(0xd6, 0x92);
HI351_write_cmos_sensor(0xd7, 0x00); //Dark1 1192
HI351_write_cmos_sensor(0xd8, 0x93);
HI351_write_cmos_sensor(0xd9, 0x16); //Dark1 1193
HI351_write_cmos_sensor(0xda, 0x94);
HI351_write_cmos_sensor(0xdb, 0x01); //Dark1 1194
HI351_write_cmos_sensor(0xdc, 0x95);
HI351_write_cmos_sensor(0xdd, 0x80); //Dark1 1195
HI351_write_cmos_sensor(0xde, 0x96);
HI351_write_cmos_sensor(0xdf, 0x55); //Dark1 1196
HI351_write_cmos_sensor(0xe0, 0x97);
HI351_write_cmos_sensor(0xe1, 0x8d); //Dark1 1197
HI351_write_cmos_sensor(0xe2, 0xb0);
HI351_write_cmos_sensor(0xe3, 0x60); //Dark1 11b0
HI351_write_cmos_sensor(0xe4, 0xb1);
HI351_write_cmos_sensor(0xe5, 0xb0); //Dark1 11b1
HI351_write_cmos_sensor(0xe6, 0xb2);
HI351_write_cmos_sensor(0xe7, 0x88); //Dark1 11b2
HI351_write_cmos_sensor(0xe8, 0xb3);
HI351_write_cmos_sensor(0xe9, 0x10); //Dark1 11b3
HI351_write_cmos_sensor(0xea, 0xb4);
HI351_write_cmos_sensor(0xeb, 0x04); //Dark1 11b4
HI351_write_cmos_sensor(0xec, 0x03);
HI351_write_cmos_sensor(0xed, 0x12);
HI351_write_cmos_sensor(0xee, 0x10);
HI351_write_cmos_sensor(0xef, 0x03); //Dark1 1210
HI351_write_cmos_sensor(0xf0, 0x11);
HI351_write_cmos_sensor(0xf1, 0x29); //Dark1 1211
HI351_write_cmos_sensor(0xf2, 0x12);
HI351_write_cmos_sensor(0xf3, 0x04); //Dark1 1212
HI351_write_cmos_sensor(0xf4, 0x40);
HI351_write_cmos_sensor(0xf5, 0x33); //Dark1 1240
HI351_write_cmos_sensor(0xf6, 0x41);
HI351_write_cmos_sensor(0xf7, 0x0a); //Dark1 1241
HI351_write_cmos_sensor(0xf8, 0x42);
HI351_write_cmos_sensor(0xf9, 0x6a); //Dark1 1242
HI351_write_cmos_sensor(0xfa, 0x43);
HI351_write_cmos_sensor(0xfb, 0x80); //Dark1 1243
HI351_write_cmos_sensor(0xfc, 0x44);
HI351_write_cmos_sensor(0xfd, 0x02); //Dark1 1244
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
HI351_write_cmos_sensor(0x03, 0xe1);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x45);
HI351_write_cmos_sensor(0x11, 0x0a); //Dark1 1245
HI351_write_cmos_sensor(0x12, 0x46);
HI351_write_cmos_sensor(0x13, 0x80); //Dark1 1246
HI351_write_cmos_sensor(0x14, 0x60);
HI351_write_cmos_sensor(0x15, 0x21); //Dark1 1260
HI351_write_cmos_sensor(0x16, 0x61);
HI351_write_cmos_sensor(0x17, 0x0e); //Dark1 1261
HI351_write_cmos_sensor(0x18, 0x62);
HI351_write_cmos_sensor(0x19, 0x70); //Dark1 1262
HI351_write_cmos_sensor(0x1a, 0x63);
HI351_write_cmos_sensor(0x1b, 0x70); //Dark1 1263
HI351_write_cmos_sensor(0x1c, 0x64);
HI351_write_cmos_sensor(0x1d, 0x14); //Dark1 1264
HI351_write_cmos_sensor(0x1e, 0x65);
HI351_write_cmos_sensor(0x1f, 0x01); //Dark1 1265
HI351_write_cmos_sensor(0x20, 0x68);
HI351_write_cmos_sensor(0x21, 0x0a); //Dark1 1268
HI351_write_cmos_sensor(0x22, 0x69);
HI351_write_cmos_sensor(0x23, 0x04); //Dark1 1269
HI351_write_cmos_sensor(0x24, 0x6a);
HI351_write_cmos_sensor(0x25, 0x0a); //Dark1 126a
HI351_write_cmos_sensor(0x26, 0x6b);
HI351_write_cmos_sensor(0x27, 0x0a); //Dark1 126b
HI351_write_cmos_sensor(0x28, 0x6c);
HI351_write_cmos_sensor(0x29, 0x24); //Dark1 126c
HI351_write_cmos_sensor(0x2a, 0x6d);
HI351_write_cmos_sensor(0x2b, 0x01); //Dark1 126d
HI351_write_cmos_sensor(0x2c, 0x70);
HI351_write_cmos_sensor(0x2d, 0x01); //Dark1 1270
HI351_write_cmos_sensor(0x2e, 0x71);
HI351_write_cmos_sensor(0x2f, 0x3d); //Dark1 1271
HI351_write_cmos_sensor(0x30, 0x80);
HI351_write_cmos_sensor(0x31, 0x80); //Dark1 1280
HI351_write_cmos_sensor(0x32, 0x81);
HI351_write_cmos_sensor(0x33, 0x88); //Dark1 1281
HI351_write_cmos_sensor(0x34, 0x82);
HI351_write_cmos_sensor(0x35, 0x08); //Dark1 1282
HI351_write_cmos_sensor(0x36, 0x83);
HI351_write_cmos_sensor(0x37, 0x0c); //Dark1 1283
HI351_write_cmos_sensor(0x38, 0x84);
HI351_write_cmos_sensor(0x39, 0x90); //Dark1 1284
HI351_write_cmos_sensor(0x3a, 0x85);
HI351_write_cmos_sensor(0x3b, 0x92); //Dark1 1285
HI351_write_cmos_sensor(0x3c, 0x86);
HI351_write_cmos_sensor(0x3d, 0x20); //Dark1 1286
HI351_write_cmos_sensor(0x3e, 0x87);
HI351_write_cmos_sensor(0x3f, 0x00); //Dark1 1287
HI351_write_cmos_sensor(0x40, 0x88);
HI351_write_cmos_sensor(0x41, 0x70); //Dark1 1288
HI351_write_cmos_sensor(0x42, 0x89);
HI351_write_cmos_sensor(0x43, 0xaa); //Dark1 1289
HI351_write_cmos_sensor(0x44, 0x8a);
HI351_write_cmos_sensor(0x45, 0x50); //Dark1 128a
HI351_write_cmos_sensor(0x46, 0x8b);
HI351_write_cmos_sensor(0x47, 0x10); //Dark1 128b
HI351_write_cmos_sensor(0x48, 0x8c);
HI351_write_cmos_sensor(0x49, 0x04); //Dark1 128c
HI351_write_cmos_sensor(0x4a, 0x8d);
HI351_write_cmos_sensor(0x4b, 0x02); //Dark1 128d
HI351_write_cmos_sensor(0x4c, 0xe6);
HI351_write_cmos_sensor(0x4d, 0xff); //Dark1 12e6
HI351_write_cmos_sensor(0x4e, 0xe7);
HI351_write_cmos_sensor(0x4f, 0x18); //Dark1 12e7
HI351_write_cmos_sensor(0x50, 0xe8);
HI351_write_cmos_sensor(0x51, 0x20); //Dark1 12e8
HI351_write_cmos_sensor(0x52, 0xe9);
HI351_write_cmos_sensor(0x53, 0x06); //Dark1 12e9
HI351_write_cmos_sensor(0x54, 0x03);
HI351_write_cmos_sensor(0x55, 0x13);
HI351_write_cmos_sensor(0x56, 0x10);
HI351_write_cmos_sensor(0x57, 0x31); //Dark1 1310
HI351_write_cmos_sensor(0x58, 0x20);
HI351_write_cmos_sensor(0x59, 0x20); //Dark1 1320
HI351_write_cmos_sensor(0x5a, 0x21);
HI351_write_cmos_sensor(0x5b, 0x30); //Dark1 1321
HI351_write_cmos_sensor(0x5c, 0x22);
HI351_write_cmos_sensor(0x5d, 0x36); //Dark1 1322
HI351_write_cmos_sensor(0x5e, 0x23);
HI351_write_cmos_sensor(0x5f, 0x6a); //Dark1 1323
HI351_write_cmos_sensor(0x60, 0x24);
HI351_write_cmos_sensor(0x61, 0xa0); //Dark1 1324
HI351_write_cmos_sensor(0x62, 0x25);
HI351_write_cmos_sensor(0x63, 0xc0); //Dark1 1325
HI351_write_cmos_sensor(0x64, 0x26);
HI351_write_cmos_sensor(0x65, 0xe0); //Dark1 1326
HI351_write_cmos_sensor(0x66, 0x27);
HI351_write_cmos_sensor(0x67, 0x02); //Dark1 1327
HI351_write_cmos_sensor(0x68, 0x28);
HI351_write_cmos_sensor(0x69, 0x03); //Dark1 1328
HI351_write_cmos_sensor(0x6a, 0x29);
HI351_write_cmos_sensor(0x6b, 0x03); //Dark1 1329
HI351_write_cmos_sensor(0x6c, 0x2a);
HI351_write_cmos_sensor(0x6d, 0x02); //Dark1 132a
HI351_write_cmos_sensor(0x6e, 0x2b);
HI351_write_cmos_sensor(0x6f, 0x04); //Dark1 132b
HI351_write_cmos_sensor(0x70, 0x2c);
HI351_write_cmos_sensor(0x71, 0x04); //Dark1 132c
HI351_write_cmos_sensor(0x72, 0x2d);
HI351_write_cmos_sensor(0x73, 0x03); //Dark1 132d
HI351_write_cmos_sensor(0x74, 0x2e);
HI351_write_cmos_sensor(0x75, 0x03); //Dark1 132e
HI351_write_cmos_sensor(0x76, 0x2f);
HI351_write_cmos_sensor(0x77, 0x14); //Dark1 132f
HI351_write_cmos_sensor(0x78, 0x30);
HI351_write_cmos_sensor(0x79, 0x03); //Dark1 1330
HI351_write_cmos_sensor(0x7a, 0x31);
HI351_write_cmos_sensor(0x7b, 0x03); //Dark1 1331
HI351_write_cmos_sensor(0x7c, 0x32);
HI351_write_cmos_sensor(0x7d, 0x03); //Dark1 1332
HI351_write_cmos_sensor(0x7e, 0x33);
HI351_write_cmos_sensor(0x7f, 0x40); //Dark1 1333
HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x81, 0x80); //Dark1 1334
HI351_write_cmos_sensor(0x82, 0x35);
HI351_write_cmos_sensor(0x83, 0x00); //Dark1 1335
HI351_write_cmos_sensor(0x84, 0x36);
HI351_write_cmos_sensor(0x85, 0xf0); //Dark1 1336
HI351_write_cmos_sensor(0x86, 0xa0);
HI351_write_cmos_sensor(0x87, 0x07); //Dark1 13a0
HI351_write_cmos_sensor(0x88, 0xa8);
HI351_write_cmos_sensor(0x89, 0x24); //Dark1 13a8 Cb_Filter
HI351_write_cmos_sensor(0x8a, 0xa9);
HI351_write_cmos_sensor(0x8b, 0x24); //Dark1 13a9 Cr_Filter
HI351_write_cmos_sensor(0x8c, 0xaa);
HI351_write_cmos_sensor(0x8d, 0x20); //Dark1 13aa
HI351_write_cmos_sensor(0x8e, 0xab);
HI351_write_cmos_sensor(0x8f, 0x02); //Dark1 13ab
HI351_write_cmos_sensor(0x90, 0xc0);
HI351_write_cmos_sensor(0x91, 0x27); //Dark1 13c0
HI351_write_cmos_sensor(0x92, 0xc2);
HI351_write_cmos_sensor(0x93, 0x08); //Dark1 13c2
HI351_write_cmos_sensor(0x94, 0xc3);
HI351_write_cmos_sensor(0x95, 0x08); //Dark1 13c3
HI351_write_cmos_sensor(0x96, 0xc4);
HI351_write_cmos_sensor(0x97, 0x40); //Dark1 13c4
HI351_write_cmos_sensor(0x98, 0xc5);
HI351_write_cmos_sensor(0x99, 0x38); //Dark1 13c5
HI351_write_cmos_sensor(0x9a, 0xc6);
HI351_write_cmos_sensor(0x9b, 0xf0); //Dark1 13c6
HI351_write_cmos_sensor(0x9c, 0xc7);
HI351_write_cmos_sensor(0x9d, 0x10); //Dark1 13c7
HI351_write_cmos_sensor(0x9e, 0xc8);
HI351_write_cmos_sensor(0x9f, 0x44); //Dark1 13c8
HI351_write_cmos_sensor(0xa0, 0xc9);
HI351_write_cmos_sensor(0xa1, 0x87); //Dark1 13c9
HI351_write_cmos_sensor(0xa2, 0xca);
HI351_write_cmos_sensor(0xa3, 0xff); //Dark1 13ca
HI351_write_cmos_sensor(0xa4, 0xcb);
HI351_write_cmos_sensor(0xa5, 0x20); //Dark1 13cb
HI351_write_cmos_sensor(0xa6, 0xcc);
HI351_write_cmos_sensor(0xa7, 0x61); //Dark1 13cc
HI351_write_cmos_sensor(0xa8, 0xcd);
HI351_write_cmos_sensor(0xa9, 0x87); //Dark1 13cd
HI351_write_cmos_sensor(0xaa, 0xce);
HI351_write_cmos_sensor(0xab, 0x8a); //Dark1 13ce
HI351_write_cmos_sensor(0xac, 0xcf);
HI351_write_cmos_sensor(0xad, 0xa5); //Dark1 13cf
HI351_write_cmos_sensor(0xae, 0x03);
HI351_write_cmos_sensor(0xaf, 0x14);
HI351_write_cmos_sensor(0xb0, 0x11);
HI351_write_cmos_sensor(0xb1, 0x04); //Dark1 1410
HI351_write_cmos_sensor(0xb2, 0x11);
HI351_write_cmos_sensor(0xb3, 0x04); //Dark1 1411 TOP L_clip
HI351_write_cmos_sensor(0xb4, 0x12);
HI351_write_cmos_sensor(0xb5, 0x40); //Dark1 1412
HI351_write_cmos_sensor(0xb6, 0x13);
HI351_write_cmos_sensor(0xb7, 0x98); //Dark1 1413
HI351_write_cmos_sensor(0xb8, 0x14);
HI351_write_cmos_sensor(0xb9, 0x3a); //Dark1 1414
HI351_write_cmos_sensor(0xba, 0x15);
HI351_write_cmos_sensor(0xbb, 0x30); //Dark1 1415
HI351_write_cmos_sensor(0xbc, 0x16);
HI351_write_cmos_sensor(0xbd, 0x30); //Dark1 1416
HI351_write_cmos_sensor(0xbe, 0x17);
HI351_write_cmos_sensor(0xbf, 0x30); //Dark1 1417
HI351_write_cmos_sensor(0xc0, 0x18);
HI351_write_cmos_sensor(0xc1, 0x38); //Dark1 1418  Negative High Gain
HI351_write_cmos_sensor(0xc2, 0x19);
HI351_write_cmos_sensor(0xc3, 0x40); //Dark1 1419  Negative Middle Gain
HI351_write_cmos_sensor(0xc4, 0x1a);
HI351_write_cmos_sensor(0xc5, 0x40); //Dark1 141a  Negative Low Gain
HI351_write_cmos_sensor(0xc6, 0x20);
HI351_write_cmos_sensor(0xc7, 0x84); //Dark1 1420  s_diff L_clip
HI351_write_cmos_sensor(0xc8, 0x21);
HI351_write_cmos_sensor(0xc9, 0x03); //Dark1 1421
HI351_write_cmos_sensor(0xca, 0x22);
HI351_write_cmos_sensor(0xcb, 0x05); //Dark1 1422
HI351_write_cmos_sensor(0xcc, 0x23);
HI351_write_cmos_sensor(0xcd, 0x07); //Dark1 1423
HI351_write_cmos_sensor(0xce, 0x24);
HI351_write_cmos_sensor(0xcf, 0x0a); //Dark1 1424
HI351_write_cmos_sensor(0xd0, 0x25);
HI351_write_cmos_sensor(0xd1, 0x46); //Dark1 1425
HI351_write_cmos_sensor(0xd2, 0x26);
HI351_write_cmos_sensor(0xd3, 0x32); //Dark1 1426
HI351_write_cmos_sensor(0xd4, 0x27);
HI351_write_cmos_sensor(0xd5, 0x1e); //Dark1 1427
HI351_write_cmos_sensor(0xd6, 0x28);
HI351_write_cmos_sensor(0xd7, 0x10); //Dark1 1428
HI351_write_cmos_sensor(0xd8, 0x29);
HI351_write_cmos_sensor(0xd9, 0x00); //Dark1 1429
HI351_write_cmos_sensor(0xda, 0x2a);
HI351_write_cmos_sensor(0xdb, 0x18); //Dark1 142a
HI351_write_cmos_sensor(0xdc, 0x2b);
HI351_write_cmos_sensor(0xdd, 0x18); //Dark1 142b
HI351_write_cmos_sensor(0xde, 0x2c);
HI351_write_cmos_sensor(0xdf, 0x18); //Dark1 142c
HI351_write_cmos_sensor(0xe0, 0x2d);
HI351_write_cmos_sensor(0xe1, 0x30); //Dark1 142d
HI351_write_cmos_sensor(0xe2, 0x2e);
HI351_write_cmos_sensor(0xe3, 0x30); //Dark1 142e
HI351_write_cmos_sensor(0xe4, 0x2f);
HI351_write_cmos_sensor(0xe5, 0x30); //Dark1 142f
HI351_write_cmos_sensor(0xe6, 0x30);
HI351_write_cmos_sensor(0xe7, 0x84); //Dark1 1430 Ldiff_L_cip
HI351_write_cmos_sensor(0xe8, 0x31);
HI351_write_cmos_sensor(0xe9, 0x02); //Dark1 1431
HI351_write_cmos_sensor(0xea, 0x32);
HI351_write_cmos_sensor(0xeb, 0x04); //Dark1 1432
HI351_write_cmos_sensor(0xec, 0x33);
HI351_write_cmos_sensor(0xed, 0x04); //Dark1 1433
HI351_write_cmos_sensor(0xee, 0x34);
HI351_write_cmos_sensor(0xef, 0x0a); //Dark1 1434
HI351_write_cmos_sensor(0xf0, 0x35);
HI351_write_cmos_sensor(0xf1, 0x46); //Dark1 1435
HI351_write_cmos_sensor(0xf2, 0x36);
HI351_write_cmos_sensor(0xf3, 0x32); //Dark1 1436
HI351_write_cmos_sensor(0xf4, 0x37);
HI351_write_cmos_sensor(0xf5, 0x28); //Dark1 1437
HI351_write_cmos_sensor(0xf6, 0x38);
HI351_write_cmos_sensor(0xf7, 0x12); //Dark1 1438
HI351_write_cmos_sensor(0xf8, 0x39);
HI351_write_cmos_sensor(0xf9, 0x00); //Dark1 1439
HI351_write_cmos_sensor(0xfa, 0x3a);
HI351_write_cmos_sensor(0xfb, 0x20); //Dark1 143a
HI351_write_cmos_sensor(0xfc, 0x3b);
HI351_write_cmos_sensor(0xfd, 0x20); //Dark1 143b
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end

HI351_write_cmos_sensor(0x03, 0xe2);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x3c);
HI351_write_cmos_sensor(0x11, 0x20); //Dark1 143c
HI351_write_cmos_sensor(0x12, 0x3d);
HI351_write_cmos_sensor(0x13, 0x10); //Dark1 143d
HI351_write_cmos_sensor(0x14, 0x3e);
HI351_write_cmos_sensor(0x15, 0x18); //Dark1 143e
HI351_write_cmos_sensor(0x16, 0x3f);
HI351_write_cmos_sensor(0x17, 0x14); //Dark1 143f
HI351_write_cmos_sensor(0x18, 0x40);
HI351_write_cmos_sensor(0x19, 0x04); //Dark11440  Mdiff Low Clip off
HI351_write_cmos_sensor(0x1a, 0x41);
HI351_write_cmos_sensor(0x1b, 0x10); //Dark1 1441
HI351_write_cmos_sensor(0x1c, 0x42);
HI351_write_cmos_sensor(0x1d, 0x70); //Dark1 1442
HI351_write_cmos_sensor(0x1e, 0x43);
HI351_write_cmos_sensor(0x1f, 0x20); //Dark1 1443
HI351_write_cmos_sensor(0x20, 0x44);
HI351_write_cmos_sensor(0x21, 0x10); //Dark1 1444
HI351_write_cmos_sensor(0x22, 0x45);
HI351_write_cmos_sensor(0x23, 0x0c); //Dark1 1445
HI351_write_cmos_sensor(0x24, 0x46);
HI351_write_cmos_sensor(0x25, 0x10); //Dark1 1446
HI351_write_cmos_sensor(0x26, 0x47);
HI351_write_cmos_sensor(0x27, 0x18); //Dark1 1447
HI351_write_cmos_sensor(0x28, 0x48);
HI351_write_cmos_sensor(0x29, 0x0a); //Dark1 1448
HI351_write_cmos_sensor(0x2a, 0x49);
HI351_write_cmos_sensor(0x2b, 0x10); //Dark1 1449
HI351_write_cmos_sensor(0x2c, 0x50);
HI351_write_cmos_sensor(0x2d, 0x85); //Dark1 1450 Hdiff Low Clip
HI351_write_cmos_sensor(0x2e, 0x51);
HI351_write_cmos_sensor(0x2f, 0x30); //Dark1 1451 hclip
HI351_write_cmos_sensor(0x30, 0x52);
HI351_write_cmos_sensor(0x31, 0xb0); //Dark1 1452
HI351_write_cmos_sensor(0x32, 0x53);
HI351_write_cmos_sensor(0x33, 0x37); //Dark1 1453
HI351_write_cmos_sensor(0x34, 0x54);
HI351_write_cmos_sensor(0x35, 0x33); //Dark1 1454
HI351_write_cmos_sensor(0x36, 0x55);
HI351_write_cmos_sensor(0x37, 0x33); //Dark1 1455
HI351_write_cmos_sensor(0x38, 0x56);
HI351_write_cmos_sensor(0x39, 0x33); //Dark1 1456
HI351_write_cmos_sensor(0x3a, 0x57);
HI351_write_cmos_sensor(0x3b, 0x10); //Dark1 1457
HI351_write_cmos_sensor(0x3c, 0x58);
HI351_write_cmos_sensor(0x3d, 0x14); //Dark1 1458
HI351_write_cmos_sensor(0x3e, 0x59);
HI351_write_cmos_sensor(0x3f, 0x10); //Dark1 1459
HI351_write_cmos_sensor(0x40, 0x60);
HI351_write_cmos_sensor(0x41, 0x01); //Dark1 1460
HI351_write_cmos_sensor(0x42, 0x61);
HI351_write_cmos_sensor(0x43, 0xa0); //Dark1 1461
HI351_write_cmos_sensor(0x44, 0x62);
HI351_write_cmos_sensor(0x45, 0x98); //Dark1 1462
HI351_write_cmos_sensor(0x46, 0x63);
HI351_write_cmos_sensor(0x47, 0xe4); //Dark1 1463
HI351_write_cmos_sensor(0x48, 0x64);
HI351_write_cmos_sensor(0x49, 0xa4); //Dark1 1464
HI351_write_cmos_sensor(0x4a, 0x65);
HI351_write_cmos_sensor(0x4b, 0x7d); //Dark1 1465
HI351_write_cmos_sensor(0x4c, 0x66);
HI351_write_cmos_sensor(0x4d, 0x4b); //Dark1 1466
HI351_write_cmos_sensor(0x4e, 0x70);
HI351_write_cmos_sensor(0x4f, 0x10); //Dark1 1470
HI351_write_cmos_sensor(0x50, 0x71);
HI351_write_cmos_sensor(0x51, 0x10); //Dark1 1471
HI351_write_cmos_sensor(0x52, 0x72);
HI351_write_cmos_sensor(0x53, 0x10); //Dark1 1472
HI351_write_cmos_sensor(0x54, 0x73);
HI351_write_cmos_sensor(0x55, 0x10); //Dark1 1473
HI351_write_cmos_sensor(0x56, 0x74);
HI351_write_cmos_sensor(0x57, 0x10); //Dark1 1474
HI351_write_cmos_sensor(0x58, 0x75);
HI351_write_cmos_sensor(0x59, 0x10); //Dark1 1475
HI351_write_cmos_sensor(0x5a, 0x76);
HI351_write_cmos_sensor(0x5b, 0x38); //Dark1 1476    green sharp pos High
HI351_write_cmos_sensor(0x5c, 0x77);
HI351_write_cmos_sensor(0x5d, 0x38); //Dark1 1477    green sharp pos Middle
HI351_write_cmos_sensor(0x5e, 0x78);
HI351_write_cmos_sensor(0x5f, 0x38); //Dark1 1478    green sharp pos Low
HI351_write_cmos_sensor(0x60, 0x79);
HI351_write_cmos_sensor(0x61, 0x70); //Dark1 1479    green sharp nega High
HI351_write_cmos_sensor(0x62, 0x7a);
HI351_write_cmos_sensor(0x63, 0x70); //Dark1 147a    green sharp nega Middle
HI351_write_cmos_sensor(0x64, 0x7b);
HI351_write_cmos_sensor(0x65, 0x70); //Dark1 147b    green sharp nega Low

HI351_write_cmos_sensor(0x66, 0x03);
HI351_write_cmos_sensor(0x67, 0x10); //10 page		//saturation if on more vivid
HI351_write_cmos_sensor(0x68, 0x60);
HI351_write_cmos_sensor(0x69, 0x01); //1060 on : 03, off : 01
HI351_write_cmos_sensor(0x6a, 0x70);
HI351_write_cmos_sensor(0x6b, 0x0c); //1070
HI351_write_cmos_sensor(0x6c, 0x71);
HI351_write_cmos_sensor(0x6d, 0x05); //1071
HI351_write_cmos_sensor(0x6e, 0x72);
HI351_write_cmos_sensor(0x6f, 0x5f); //1072
HI351_write_cmos_sensor(0x70, 0x73);
HI351_write_cmos_sensor(0x71, 0x33); //1073
HI351_write_cmos_sensor(0x72, 0x74);
HI351_write_cmos_sensor(0x73, 0x1b); //1074
HI351_write_cmos_sensor(0x74, 0x75);
HI351_write_cmos_sensor(0x75, 0x03); //1075
HI351_write_cmos_sensor(0x76, 0x76);
HI351_write_cmos_sensor(0x77, 0x20); //1076
HI351_write_cmos_sensor(0x78, 0x77);
HI351_write_cmos_sensor(0x79, 0x33); //1077
HI351_write_cmos_sensor(0x7a, 0x78);
HI351_write_cmos_sensor(0x7b, 0x33); //1078
HI351_write_cmos_sensor(0x7c, 0x79);
HI351_write_cmos_sensor(0x7d, 0x46); //1079
HI351_write_cmos_sensor(0x7e, 0x7a);
HI351_write_cmos_sensor(0x7f, 0x66); //107a
HI351_write_cmos_sensor(0x80, 0x7b);
HI351_write_cmos_sensor(0x81, 0x43); //107b
HI351_write_cmos_sensor(0x82, 0x7c);
HI351_write_cmos_sensor(0x83, 0x33); //107c
HI351_write_cmos_sensor(0x84, 0x7d);
HI351_write_cmos_sensor(0x85, 0x0e); //107d
HI351_write_cmos_sensor(0x86, 0x7e);
HI351_write_cmos_sensor(0x87, 0x1e); //107e
HI351_write_cmos_sensor(0x88, 0x7f);
HI351_write_cmos_sensor(0x89, 0x3c); //107f

HI351_write_cmos_sensor(0x8a, 0x03); //16 page  //dark condition reduce colo noise
HI351_write_cmos_sensor(0x8b, 0x16);
HI351_write_cmos_sensor(0x8c, 0x8a); //168a
HI351_write_cmos_sensor(0x8d, 0x68);
HI351_write_cmos_sensor(0x8e, 0x8b); //168b
HI351_write_cmos_sensor(0x8f, 0x7c);
HI351_write_cmos_sensor(0x90, 0x8c); //168c
HI351_write_cmos_sensor(0x91, 0x7f);
HI351_write_cmos_sensor(0x92, 0x8d); //168d
HI351_write_cmos_sensor(0x93, 0x7f);
HI351_write_cmos_sensor(0x94, 0x8e); //168e
HI351_write_cmos_sensor(0x95, 0x7f);
HI351_write_cmos_sensor(0x96, 0x8f); //168f
HI351_write_cmos_sensor(0x97, 0x7f);
HI351_write_cmos_sensor(0x98, 0x90); //1690
HI351_write_cmos_sensor(0x99, 0x7f);
HI351_write_cmos_sensor(0x9a, 0x91); //1691
HI351_write_cmos_sensor(0x9b, 0x7f);
HI351_write_cmos_sensor(0x9c, 0x92); //1692
HI351_write_cmos_sensor(0x9d, 0x7f);
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
//end

//////////////////
// e3 Page (DMA Dark2)
//////////////////

HI351_write_cmos_sensor(0x03, 0xe3);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x03);//Dark2 Page11
HI351_write_cmos_sensor(0x11, 0x11);
HI351_write_cmos_sensor(0x12, 0x10);
HI351_write_cmos_sensor(0x13, 0x1f); //Dark2 1110
HI351_write_cmos_sensor(0x14, 0x11);
HI351_write_cmos_sensor(0x15, 0x2a); //Dark2 1111
HI351_write_cmos_sensor(0x16, 0x12);
HI351_write_cmos_sensor(0x17, 0x1c); //Dark2 1112
HI351_write_cmos_sensor(0x18, 0x13);
HI351_write_cmos_sensor(0x19, 0x1c); //Dark2 1113
HI351_write_cmos_sensor(0x1a, 0x14);
HI351_write_cmos_sensor(0x1b, 0x3a); //Dark2 1114
HI351_write_cmos_sensor(0x1c, 0x30);
HI351_write_cmos_sensor(0x1d, 0x20); //Dark2 1130
HI351_write_cmos_sensor(0x1e, 0x31);
HI351_write_cmos_sensor(0x1f, 0x20); //Dark2 1131
HI351_write_cmos_sensor(0x20, 0x32);
HI351_write_cmos_sensor(0x21, 0x40); //Dark2 1132 :Lum level1
HI351_write_cmos_sensor(0x22, 0x33);
HI351_write_cmos_sensor(0x23, 0x28); //Dark2 1133 :Lum level2
HI351_write_cmos_sensor(0x24, 0x34);
HI351_write_cmos_sensor(0x25, 0x1a); //Dark2 1134 :Lum level3
HI351_write_cmos_sensor(0x26, 0x35);
HI351_write_cmos_sensor(0x27, 0x14); //Dark2 1135 :Lum level4
HI351_write_cmos_sensor(0x28, 0x36);
HI351_write_cmos_sensor(0x29, 0x0c); //Dark2 1136 :Lum level5
HI351_write_cmos_sensor(0x2a, 0x37);
HI351_write_cmos_sensor(0x2b, 0x0a); //Dark2 1137 :Lum level6
HI351_write_cmos_sensor(0x2c, 0x38);
HI351_write_cmos_sensor(0x2d, 0x00); //Dark2 1138 :Lum level7
HI351_write_cmos_sensor(0x2e, 0x39);
HI351_write_cmos_sensor(0x2f, 0x8a); //Dark2 1139 gain 1
HI351_write_cmos_sensor(0x30, 0x3a);
HI351_write_cmos_sensor(0x31, 0x8a); //Dark2 113a
HI351_write_cmos_sensor(0x32, 0x3b);
HI351_write_cmos_sensor(0x33, 0x8a); //Dark2 113b
HI351_write_cmos_sensor(0x34, 0x3c);
HI351_write_cmos_sensor(0x35, 0x8a); //Dark2 113c
HI351_write_cmos_sensor(0x36, 0x3d);
HI351_write_cmos_sensor(0x37, 0x8a); //Dark2 113d
HI351_write_cmos_sensor(0x38, 0x3e);
HI351_write_cmos_sensor(0x39, 0x8a); //Dark2 113e
HI351_write_cmos_sensor(0x3a, 0x3f);
HI351_write_cmos_sensor(0x3b, 0x8a); //Dark2 113f
HI351_write_cmos_sensor(0x3c, 0x40);
HI351_write_cmos_sensor(0x3d, 0x8a); //Dark2 1140 gain 8
HI351_write_cmos_sensor(0x3e, 0x41);
HI351_write_cmos_sensor(0x3f, 0x40); //Dark2 1141 offset 1
HI351_write_cmos_sensor(0x40, 0x42);
HI351_write_cmos_sensor(0x41, 0x10); //Dark2 1142 offset 2
HI351_write_cmos_sensor(0x42, 0x43);
HI351_write_cmos_sensor(0x43, 0x10); //Dark2 1143 offset 3
HI351_write_cmos_sensor(0x44, 0x44);
HI351_write_cmos_sensor(0x45, 0x10); //Dark2 1144 offset 4
HI351_write_cmos_sensor(0x46, 0x45);
HI351_write_cmos_sensor(0x47, 0x10); //Dark2 1145 offset 5
HI351_write_cmos_sensor(0x48, 0x46);
HI351_write_cmos_sensor(0x49, 0x10); //Dark2 1146 offset 6
HI351_write_cmos_sensor(0x4a, 0x47);
HI351_write_cmos_sensor(0x4b, 0x10); //Dark2 1147 offset 7
HI351_write_cmos_sensor(0x4c, 0x48);
HI351_write_cmos_sensor(0x4d, 0x10); //Dark2 1148 offset 8
HI351_write_cmos_sensor(0x4e, 0x49);
HI351_write_cmos_sensor(0x4f, 0x40); //Dark2 1149 high_clip_start
HI351_write_cmos_sensor(0x50, 0x4a);
HI351_write_cmos_sensor(0x51, 0x40); //Dark2 114a
HI351_write_cmos_sensor(0x52, 0x4b);
HI351_write_cmos_sensor(0x53, 0x40); //Dark2 114b
HI351_write_cmos_sensor(0x54, 0x4c);
HI351_write_cmos_sensor(0x55, 0x40); //Dark2 114c
HI351_write_cmos_sensor(0x56, 0x4d);
HI351_write_cmos_sensor(0x57, 0x40); //Dark2 114d
HI351_write_cmos_sensor(0x58, 0x4e);
HI351_write_cmos_sensor(0x59, 0x40); //Dark2 114e Lv 6 h_clip
HI351_write_cmos_sensor(0x5a, 0x4f);
HI351_write_cmos_sensor(0x5b, 0x40); //Dark2 114f Lv 7 h_clip
HI351_write_cmos_sensor(0x5c, 0x50);
HI351_write_cmos_sensor(0x5d, 0x40); //Dark2 1150 clip 8
HI351_write_cmos_sensor(0x5e, 0x51);
HI351_write_cmos_sensor(0x5f, 0xf0); //Dark2 1151 color gain start
HI351_write_cmos_sensor(0x60, 0x52);
HI351_write_cmos_sensor(0x61, 0xf0); //Dark2 1152
HI351_write_cmos_sensor(0x62, 0x53);
HI351_write_cmos_sensor(0x63, 0xf0); //Dark2 1153
HI351_write_cmos_sensor(0x64, 0x54);
HI351_write_cmos_sensor(0x65, 0xf0); //Dark2 1154
HI351_write_cmos_sensor(0x66, 0x55);
HI351_write_cmos_sensor(0x67, 0xf0); //Dark2 1155
HI351_write_cmos_sensor(0x68, 0x56);
HI351_write_cmos_sensor(0x69, 0xf0); //Dark2 1156
HI351_write_cmos_sensor(0x6a, 0x57);
HI351_write_cmos_sensor(0x6b, 0xf0); //Dark2 1157
HI351_write_cmos_sensor(0x6c, 0x58);
HI351_write_cmos_sensor(0x6d, 0xf0); //Dark2 1158 color gain end
HI351_write_cmos_sensor(0x6e, 0x59);
HI351_write_cmos_sensor(0x6f, 0xf8); //Dark2 1159 color ofs lmt start
HI351_write_cmos_sensor(0x70, 0x5a);
HI351_write_cmos_sensor(0x71, 0xf8); //Dark2 115a
HI351_write_cmos_sensor(0x72, 0x5b);
HI351_write_cmos_sensor(0x73, 0xf8); //Dark2 115b
HI351_write_cmos_sensor(0x74, 0x5c);
HI351_write_cmos_sensor(0x75, 0xf8); //Dark2 115c
HI351_write_cmos_sensor(0x76, 0x5d);
HI351_write_cmos_sensor(0x77, 0xf8); //Dark2 115d
HI351_write_cmos_sensor(0x78, 0x5e);
HI351_write_cmos_sensor(0x79, 0xf8); //Dark2 115e
HI351_write_cmos_sensor(0x7a, 0x5f);
HI351_write_cmos_sensor(0x7b, 0xf8); //Dark2 115f
HI351_write_cmos_sensor(0x7c, 0x60);
HI351_write_cmos_sensor(0x7d, 0xf8); //Dark2 1160 color ofs lmt end
HI351_write_cmos_sensor(0x7e, 0x61);
HI351_write_cmos_sensor(0x7f, 0xc0); //Dark2 1161
HI351_write_cmos_sensor(0x80, 0x62);
HI351_write_cmos_sensor(0x81, 0xf0); //Dark2 1162
HI351_write_cmos_sensor(0x82, 0x63);
HI351_write_cmos_sensor(0x83, 0x80); //Dark2 1163
HI351_write_cmos_sensor(0x84, 0x64);
HI351_write_cmos_sensor(0x85, 0x40); //Dark2 1164
HI351_write_cmos_sensor(0x86, 0x65);
HI351_write_cmos_sensor(0x87, 0x02); //Dark2 1165 : lmp_1_gain_h
HI351_write_cmos_sensor(0x88, 0x66);
HI351_write_cmos_sensor(0x89, 0x02); //Dark2 1166 : lmp_1_gain_m
HI351_write_cmos_sensor(0x8a, 0x67);
HI351_write_cmos_sensor(0x8b, 0x02); //Dark2 1167 : lmp_1_gain_l
HI351_write_cmos_sensor(0x8c, 0x68);
HI351_write_cmos_sensor(0x8d, 0x80); //Dark2 1168
HI351_write_cmos_sensor(0x8e, 0x69);
HI351_write_cmos_sensor(0x8f, 0x40); //Dark2 1169
HI351_write_cmos_sensor(0x90, 0x6a);
HI351_write_cmos_sensor(0x91, 0x01); //Dark2 116a : lmp_2_gain_h
HI351_write_cmos_sensor(0x92, 0x6b);
HI351_write_cmos_sensor(0x93, 0x01); //Dark2 116b : lmp_2_gain_m
HI351_write_cmos_sensor(0x94, 0x6c);
HI351_write_cmos_sensor(0x95, 0x01); //Dark2 116c : lmp_2_gain_l
HI351_write_cmos_sensor(0x96, 0x6d);
HI351_write_cmos_sensor(0x97, 0x80); //Dark2 116d
HI351_write_cmos_sensor(0x98, 0x6e);
HI351_write_cmos_sensor(0x99, 0x40); //Dark2 116e
HI351_write_cmos_sensor(0x9a, 0x6f);
HI351_write_cmos_sensor(0x9b, 0x01); //Dark2 116f : lmp_3_gain_h
HI351_write_cmos_sensor(0x9c, 0x70);
HI351_write_cmos_sensor(0x9d, 0x01); //Dark2 1170 : lmp_3_gain_m
HI351_write_cmos_sensor(0x9e, 0x71);
HI351_write_cmos_sensor(0x9f, 0x01); //Dark2 1171 : lmp_3_gain_l
HI351_write_cmos_sensor(0xa0, 0x72);
HI351_write_cmos_sensor(0xa1, 0x6e); //Dark2 1172
HI351_write_cmos_sensor(0xa2, 0x73);
HI351_write_cmos_sensor(0xa3, 0x3a); //Dark2 1173
HI351_write_cmos_sensor(0xa4, 0x74);
HI351_write_cmos_sensor(0xa5, 0x01); //Dark2 1174 : lmp_4_gain_h
HI351_write_cmos_sensor(0xa6, 0x75);
HI351_write_cmos_sensor(0xa7, 0x01); //Dark2 1175 : lmp_4_gain_m
HI351_write_cmos_sensor(0xa8, 0x76);
HI351_write_cmos_sensor(0xa9, 0x01); //Dark2 1176 : lmp_4_gain_l
HI351_write_cmos_sensor(0xaa, 0x77);
HI351_write_cmos_sensor(0xab, 0x6e); //Dark2 1177
HI351_write_cmos_sensor(0xac, 0x78);
HI351_write_cmos_sensor(0xad, 0x3a); //Dark2 1178
HI351_write_cmos_sensor(0xae, 0x79);
HI351_write_cmos_sensor(0xaf, 0x01); //Dark2 1179 : lmp_5_gain_h
HI351_write_cmos_sensor(0xb0, 0x7a);
HI351_write_cmos_sensor(0xb1, 0x01); //Dark2 117a : lmp_5_gain_m
HI351_write_cmos_sensor(0xb2, 0x7b);
HI351_write_cmos_sensor(0xb3, 0x01); //Dark2 117b : lmp_5_gain_l
HI351_write_cmos_sensor(0xb4, 0x7c);
HI351_write_cmos_sensor(0xb5, 0x5c); //Dark2 117c
HI351_write_cmos_sensor(0xb6, 0x7d);
HI351_write_cmos_sensor(0xb7, 0x30); //Dark2 117d
HI351_write_cmos_sensor(0xb8, 0x7e);
HI351_write_cmos_sensor(0xb9, 0x01); //Dark2 117e : lmp_6_gain_h
HI351_write_cmos_sensor(0xba, 0x7f);
HI351_write_cmos_sensor(0xbb, 0x01); //Dark2 117f : lmp_6_gain_m
HI351_write_cmos_sensor(0xbc, 0x80);
HI351_write_cmos_sensor(0xbd, 0x01); //Dark2 1180 : lmp_6_gain_l
HI351_write_cmos_sensor(0xbe, 0x81);
HI351_write_cmos_sensor(0xbf, 0x62); //Dark2 1181
HI351_write_cmos_sensor(0xc0, 0x82);
HI351_write_cmos_sensor(0xc1, 0x26); //Dark2 1182
HI351_write_cmos_sensor(0xc2, 0x83);
HI351_write_cmos_sensor(0xc3, 0x01); //Dark2 1183 : lmp_7_gain_h
HI351_write_cmos_sensor(0xc4, 0x84);
HI351_write_cmos_sensor(0xc5, 0x01); //Dark2 1184 : lmp_7_gain_m
HI351_write_cmos_sensor(0xc6, 0x85);
HI351_write_cmos_sensor(0xc7, 0x01); //Dark2 1185 : lmp_7_gain_l
HI351_write_cmos_sensor(0xc8, 0x86);
HI351_write_cmos_sensor(0xc9, 0x62); //Dark2 1186
HI351_write_cmos_sensor(0xca, 0x87);
HI351_write_cmos_sensor(0xcb, 0x26); //Dark2 1187
HI351_write_cmos_sensor(0xcc, 0x88);
HI351_write_cmos_sensor(0xcd, 0x01); //Dark2 1188 : lmp_8_gain_h
HI351_write_cmos_sensor(0xce, 0x89);
HI351_write_cmos_sensor(0xcf, 0x01); //Dark2 1189 : lmp_8_gain_m
HI351_write_cmos_sensor(0xd0, 0x8a);
HI351_write_cmos_sensor(0xd1, 0x01); //Dark2 118a : lmp_8_gain_l
HI351_write_cmos_sensor(0xd2, 0x90);
HI351_write_cmos_sensor(0xd3, 0x00); //Dark2 1190
HI351_write_cmos_sensor(0xd4, 0x91);
HI351_write_cmos_sensor(0xd5, 0x4e); //Dark2 1191
HI351_write_cmos_sensor(0xd6, 0x92);
HI351_write_cmos_sensor(0xd7, 0x00); //Dark2 1192
HI351_write_cmos_sensor(0xd8, 0x93);
HI351_write_cmos_sensor(0xd9, 0x16); //Dark2 1193
HI351_write_cmos_sensor(0xda, 0x94);
HI351_write_cmos_sensor(0xdb, 0x01); //Dark2 1194
HI351_write_cmos_sensor(0xdc, 0x95);
HI351_write_cmos_sensor(0xdd, 0x80); //Dark2 1195
HI351_write_cmos_sensor(0xde, 0x96);
HI351_write_cmos_sensor(0xdf, 0x55); //Dark2 1196
HI351_write_cmos_sensor(0xe0, 0x97);
HI351_write_cmos_sensor(0xe1, 0x8d); //Dark2 1197
HI351_write_cmos_sensor(0xe2, 0xb0);
HI351_write_cmos_sensor(0xe3, 0x30); //Dark2 11b0
HI351_write_cmos_sensor(0xe4, 0xb1);
HI351_write_cmos_sensor(0xe5, 0x90); //Dark2 11b1
HI351_write_cmos_sensor(0xe6, 0xb2);
HI351_write_cmos_sensor(0xe7, 0x08); //Dark2 11b2
HI351_write_cmos_sensor(0xe8, 0xb3);
HI351_write_cmos_sensor(0xe9, 0x00); //Dark2 11b3
HI351_write_cmos_sensor(0xea, 0xb4);
HI351_write_cmos_sensor(0xeb, 0x04); //Dark2 11b4
HI351_write_cmos_sensor(0xec, 0x03);
HI351_write_cmos_sensor(0xed, 0x12);
HI351_write_cmos_sensor(0xee, 0x10);
HI351_write_cmos_sensor(0xef, 0x03); //Dark2 1210
HI351_write_cmos_sensor(0xf0, 0x11);
HI351_write_cmos_sensor(0xf1, 0x29); //Dark2 1211
HI351_write_cmos_sensor(0xf2, 0x12);
HI351_write_cmos_sensor(0xf3, 0x03); //Dark2 1212
HI351_write_cmos_sensor(0xf4, 0x40);
HI351_write_cmos_sensor(0xf5, 0x02); //Dark2 1240
HI351_write_cmos_sensor(0xf6, 0x41);
HI351_write_cmos_sensor(0xf7, 0x0a); //Dark2 1241
HI351_write_cmos_sensor(0xf8, 0x42);
HI351_write_cmos_sensor(0xf9, 0x6a); //Dark2 1242
HI351_write_cmos_sensor(0xfa, 0x43);
HI351_write_cmos_sensor(0xfb, 0x80); //Dark2 1243
HI351_write_cmos_sensor(0xfc, 0x44);
HI351_write_cmos_sensor(0xfd, 0x02); //Dark2 1244
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end
HI351_write_cmos_sensor(0x03, 0xe4);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x45);
HI351_write_cmos_sensor(0x11, 0x0a); //Dark2 1245
HI351_write_cmos_sensor(0x12, 0x46);
HI351_write_cmos_sensor(0x13, 0x80); //Dark2 1246
HI351_write_cmos_sensor(0x14, 0x60);
HI351_write_cmos_sensor(0x15, 0x02); //Dark2 1260
HI351_write_cmos_sensor(0x16, 0x61);
HI351_write_cmos_sensor(0x17, 0x04); //Dark2 1261
HI351_write_cmos_sensor(0x18, 0x62);
HI351_write_cmos_sensor(0x19, 0x4b); //Dark2 1262
HI351_write_cmos_sensor(0x1a, 0x63);
HI351_write_cmos_sensor(0x1b, 0x41); //Dark2 1263
HI351_write_cmos_sensor(0x1c, 0x64);
HI351_write_cmos_sensor(0x1d, 0x14); //Dark2 1264
HI351_write_cmos_sensor(0x1e, 0x65);
HI351_write_cmos_sensor(0x1f, 0x00); //Dark2 1265
HI351_write_cmos_sensor(0x20, 0x68);
HI351_write_cmos_sensor(0x21, 0x0a); //Dark2 1268
HI351_write_cmos_sensor(0x22, 0x69);
HI351_write_cmos_sensor(0x23, 0x04); //Dark2 1269
HI351_write_cmos_sensor(0x24, 0x6a);
HI351_write_cmos_sensor(0x25, 0x0a); //Dark2 126a
HI351_write_cmos_sensor(0x26, 0x6b);
HI351_write_cmos_sensor(0x27, 0x0a); //Dark2 126b
HI351_write_cmos_sensor(0x28, 0x6c);
HI351_write_cmos_sensor(0x29, 0x24); //Dark2 126c
HI351_write_cmos_sensor(0x2a, 0x6d);
HI351_write_cmos_sensor(0x2b, 0x01); //Dark2 126d
HI351_write_cmos_sensor(0x2c, 0x70);
HI351_write_cmos_sensor(0x2d, 0x18); //Dark2 1270
HI351_write_cmos_sensor(0x2e, 0x71);
HI351_write_cmos_sensor(0x2f, 0xbf); //Dark2 1271
HI351_write_cmos_sensor(0x30, 0x80);
HI351_write_cmos_sensor(0x31, 0x64); //Dark2 1280
HI351_write_cmos_sensor(0x32, 0x81);
HI351_write_cmos_sensor(0x33, 0xb1); //Dark2 1281
HI351_write_cmos_sensor(0x34, 0x82);
HI351_write_cmos_sensor(0x35, 0x2c); //Dark2 1282
HI351_write_cmos_sensor(0x36, 0x83);
HI351_write_cmos_sensor(0x37, 0x02); //Dark2 1283
HI351_write_cmos_sensor(0x38, 0x84);
HI351_write_cmos_sensor(0x39, 0x30); //Dark2 1284
HI351_write_cmos_sensor(0x3a, 0x85);
HI351_write_cmos_sensor(0x3b, 0x90); //Dark2 1285
HI351_write_cmos_sensor(0x3c, 0x86);
HI351_write_cmos_sensor(0x3d, 0x10); //Dark2 1286
HI351_write_cmos_sensor(0x3e, 0x87);
HI351_write_cmos_sensor(0x3f, 0x01); //Dark2 1287
HI351_write_cmos_sensor(0x40, 0x88);
HI351_write_cmos_sensor(0x41, 0x3a); //Dark2 1288
HI351_write_cmos_sensor(0x42, 0x89);
HI351_write_cmos_sensor(0x43, 0x90); //Dark2 1289
HI351_write_cmos_sensor(0x44, 0x8a);
HI351_write_cmos_sensor(0x45, 0x0e); //Dark2 128a
HI351_write_cmos_sensor(0x46, 0x8b);
HI351_write_cmos_sensor(0x47, 0x0c); //Dark2 128b
HI351_write_cmos_sensor(0x48, 0x8c);
HI351_write_cmos_sensor(0x49, 0x05); //Dark2 128c
HI351_write_cmos_sensor(0x4a, 0x8d);
HI351_write_cmos_sensor(0x4b, 0x03); //Dark2 128d
HI351_write_cmos_sensor(0x4c, 0xe6);
HI351_write_cmos_sensor(0x4d, 0xff); //Dark2 12e6
HI351_write_cmos_sensor(0x4e, 0xe7);
HI351_write_cmos_sensor(0x4f, 0x18); //Dark2 12e7
HI351_write_cmos_sensor(0x50, 0xe8);
HI351_write_cmos_sensor(0x51, 0x0a); //Dark2 12e8
HI351_write_cmos_sensor(0x52, 0xe9);
HI351_write_cmos_sensor(0x53, 0x06); //Dark2 12e9
HI351_write_cmos_sensor(0x54, 0x03);
HI351_write_cmos_sensor(0x55, 0x13);
HI351_write_cmos_sensor(0x56, 0x10);
HI351_write_cmos_sensor(0x57, 0x3f); //Dark2 1310
HI351_write_cmos_sensor(0x58, 0x20);
HI351_write_cmos_sensor(0x59, 0x20); //Dark2 1320
HI351_write_cmos_sensor(0x5a, 0x21);
HI351_write_cmos_sensor(0x5b, 0x30); //Dark2 1321
HI351_write_cmos_sensor(0x5c, 0x22);
HI351_write_cmos_sensor(0x5d, 0x36); //Dark2 1322
HI351_write_cmos_sensor(0x5e, 0x23);
HI351_write_cmos_sensor(0x5f, 0x6a); //Dark2 1323
HI351_write_cmos_sensor(0x60, 0x24);
HI351_write_cmos_sensor(0x61, 0xa0); //Dark2 1324
HI351_write_cmos_sensor(0x62, 0x25);
HI351_write_cmos_sensor(0x63, 0xc0); //Dark2 1325
HI351_write_cmos_sensor(0x64, 0x26);
HI351_write_cmos_sensor(0x65, 0xe0); //Dark2 1326
HI351_write_cmos_sensor(0x66, 0x27);
HI351_write_cmos_sensor(0x67, 0x00); //Dark2 1327 lum 0
HI351_write_cmos_sensor(0x68, 0x28);
HI351_write_cmos_sensor(0x69, 0x00); //Dark2 1328
HI351_write_cmos_sensor(0x6a, 0x29);
HI351_write_cmos_sensor(0x6b, 0x00); //Dark2 1329
HI351_write_cmos_sensor(0x6c, 0x2a);
HI351_write_cmos_sensor(0x6d, 0x00); //Dark2 132a
HI351_write_cmos_sensor(0x6e, 0x2b);
HI351_write_cmos_sensor(0x6f, 0x00); //Dark2 132b
HI351_write_cmos_sensor(0x70, 0x2c);
HI351_write_cmos_sensor(0x71, 0x00); //Dark2 132c
HI351_write_cmos_sensor(0x72, 0x2d);
HI351_write_cmos_sensor(0x73, 0x00); //Dark2 132d
HI351_write_cmos_sensor(0x74, 0x2e);
HI351_write_cmos_sensor(0x75, 0x00); //Dark2 132e lum7
HI351_write_cmos_sensor(0x76, 0x2f);
HI351_write_cmos_sensor(0x77, 0x04); //Dark2 132f weight skin
HI351_write_cmos_sensor(0x78, 0x30);
HI351_write_cmos_sensor(0x79, 0x04); //Dark2 1330 weight blue
HI351_write_cmos_sensor(0x7a, 0x31);
HI351_write_cmos_sensor(0x7b, 0x04); //Dark2 1331 weight green
HI351_write_cmos_sensor(0x7c, 0x32);
HI351_write_cmos_sensor(0x7d, 0x04); //Dark2 1332 weight strong color
HI351_write_cmos_sensor(0x7e, 0x33);
HI351_write_cmos_sensor(0x7f, 0x10); //Dark2 1333
HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x81, 0x10); //Dark2 1334
HI351_write_cmos_sensor(0x82, 0x35);
HI351_write_cmos_sensor(0x83, 0x00); //Dark2 1335
HI351_write_cmos_sensor(0x84, 0x36);
HI351_write_cmos_sensor(0x85, 0x80); //Dark2 1336
HI351_write_cmos_sensor(0x86, 0xa0);
HI351_write_cmos_sensor(0x87, 0x07); //Dark2 13a0
HI351_write_cmos_sensor(0x88, 0xa8);
HI351_write_cmos_sensor(0x89, 0x30); //Dark2 13a8 Dark2 Cb-filter
HI351_write_cmos_sensor(0x8a, 0xa9);
HI351_write_cmos_sensor(0x8b, 0x30); //Dark2 13a9 Dark2 Cr-filter
HI351_write_cmos_sensor(0x8c, 0xaa);
HI351_write_cmos_sensor(0x8d, 0x30); //Dark2 13aa
HI351_write_cmos_sensor(0x8e, 0xab);
HI351_write_cmos_sensor(0x8f, 0x02); //Dark2 13ab
HI351_write_cmos_sensor(0x90, 0xc0);
HI351_write_cmos_sensor(0x91, 0x27); //Dark2 13c0
HI351_write_cmos_sensor(0x92, 0xc2);
HI351_write_cmos_sensor(0x93, 0x08); //Dark2 13c2
HI351_write_cmos_sensor(0x94, 0xc3);
HI351_write_cmos_sensor(0x95, 0x08); //Dark2 13c3
HI351_write_cmos_sensor(0x96, 0xc4);
HI351_write_cmos_sensor(0x97, 0x46); //Dark2 13c4
HI351_write_cmos_sensor(0x98, 0xc5);
HI351_write_cmos_sensor(0x99, 0x78); //Dark2 13c5
HI351_write_cmos_sensor(0x9a, 0xc6);
HI351_write_cmos_sensor(0x9b, 0xf0); //Dark2 13c6
HI351_write_cmos_sensor(0x9c, 0xc7);
HI351_write_cmos_sensor(0x9d, 0x10); //Dark2 13c7
HI351_write_cmos_sensor(0x9e, 0xc8);
HI351_write_cmos_sensor(0x9f, 0x44); //Dark2 13c8
HI351_write_cmos_sensor(0xa0, 0xc9);
HI351_write_cmos_sensor(0xa1, 0x87); //Dark2 13c9
HI351_write_cmos_sensor(0xa2, 0xca);
HI351_write_cmos_sensor(0xa3, 0xff); //Dark2 13ca
HI351_write_cmos_sensor(0xa4, 0xcb);
HI351_write_cmos_sensor(0xa5, 0x20); //Dark2 13cb
HI351_write_cmos_sensor(0xa6, 0xcc);
HI351_write_cmos_sensor(0xa7, 0x61); //Dark2 13cc skin range_cb_l
HI351_write_cmos_sensor(0xa8, 0xcd);
HI351_write_cmos_sensor(0xa9, 0x87); //Dark2 13cd skin range_cb_h
HI351_write_cmos_sensor(0xaa, 0xce);
HI351_write_cmos_sensor(0xab, 0x8a); //Dark2 13ce skin range_cr_l
HI351_write_cmos_sensor(0xac, 0xcf);
HI351_write_cmos_sensor(0xad, 0xa5); //Dark2 13cf skin range_cr_h
HI351_write_cmos_sensor(0xae, 0x03);
HI351_write_cmos_sensor(0xaf, 0x14);
HI351_write_cmos_sensor(0xb0, 0x11);
HI351_write_cmos_sensor(0xb1, 0x03); //Dark2 1410
HI351_write_cmos_sensor(0xb2, 0x11);
HI351_write_cmos_sensor(0xb3, 0x03); //Dark2 1411
HI351_write_cmos_sensor(0xb4, 0x12);
HI351_write_cmos_sensor(0xb5, 0x40); //Dark2 1412 Top H_Clip
HI351_write_cmos_sensor(0xb6, 0x13);
HI351_write_cmos_sensor(0xb7, 0x88); //Dark2 1413
HI351_write_cmos_sensor(0xb8, 0x14);
HI351_write_cmos_sensor(0xb9, 0x34); //Dark2 1414
HI351_write_cmos_sensor(0xba, 0x15);
HI351_write_cmos_sensor(0xbb, 0x00); //Dark2 1415 sharp positive ya
HI351_write_cmos_sensor(0xbc, 0x16);
HI351_write_cmos_sensor(0xbd, 0x00); //Dark2 1416 sharp positive mi
HI351_write_cmos_sensor(0xbe, 0x17);
HI351_write_cmos_sensor(0xbf, 0x00); //Dark2 1417 sharp positive low
HI351_write_cmos_sensor(0xc0, 0x18);
HI351_write_cmos_sensor(0xc1, 0x10); //Dark2 1418 sharp negative ya
HI351_write_cmos_sensor(0xc2, 0x19);
HI351_write_cmos_sensor(0xc3, 0x10); //Dark2 1419 sharp negative mi
HI351_write_cmos_sensor(0xc4, 0x1a);
HI351_write_cmos_sensor(0xc5, 0x10); //Dark2 141a sharp negative low
HI351_write_cmos_sensor(0xc6, 0x20);
HI351_write_cmos_sensor(0xc7, 0x83); //Dark2 1420
HI351_write_cmos_sensor(0xc8, 0x21);
HI351_write_cmos_sensor(0xc9, 0x03); //Dark2 1421
HI351_write_cmos_sensor(0xca, 0x22);
HI351_write_cmos_sensor(0xcb, 0x05); //Dark2 1422
HI351_write_cmos_sensor(0xcc, 0x23);
HI351_write_cmos_sensor(0xcd, 0x07); //Dark2 1423
HI351_write_cmos_sensor(0xce, 0x24);
HI351_write_cmos_sensor(0xcf, 0x0a); //Dark2 1424
HI351_write_cmos_sensor(0xd0, 0x25);
HI351_write_cmos_sensor(0xd1, 0x46); //Dark2 1425
HI351_write_cmos_sensor(0xd2, 0x26);
HI351_write_cmos_sensor(0xd3, 0x32); //Dark2 1426
HI351_write_cmos_sensor(0xd4, 0x27);
HI351_write_cmos_sensor(0xd5, 0x1e); //Dark2 1427
HI351_write_cmos_sensor(0xd6, 0x28);
HI351_write_cmos_sensor(0xd7, 0x19); //Dark2 1428
HI351_write_cmos_sensor(0xd8, 0x29);
HI351_write_cmos_sensor(0xd9, 0x00); //Dark2 1429
HI351_write_cmos_sensor(0xda, 0x2a);
HI351_write_cmos_sensor(0xdb, 0x10); //Dark2 142a
HI351_write_cmos_sensor(0xdc, 0x2b);
HI351_write_cmos_sensor(0xdd, 0x10); //Dark2 142b
HI351_write_cmos_sensor(0xde, 0x2c);
HI351_write_cmos_sensor(0xdf, 0x10); //Dark2 142c
HI351_write_cmos_sensor(0xe0, 0x2d);
HI351_write_cmos_sensor(0xe1, 0x80); //Dark2 142d
HI351_write_cmos_sensor(0xe2, 0x2e);
HI351_write_cmos_sensor(0xe3, 0x80); //Dark2 142e
HI351_write_cmos_sensor(0xe4, 0x2f);
HI351_write_cmos_sensor(0xe5, 0x80); //Dark2 142f
HI351_write_cmos_sensor(0xe6, 0x30);
HI351_write_cmos_sensor(0xe7, 0x83); //Dark2 1430
HI351_write_cmos_sensor(0xe8, 0x31);
HI351_write_cmos_sensor(0xe9, 0x02); //Dark2 1431
HI351_write_cmos_sensor(0xea, 0x32);
HI351_write_cmos_sensor(0xeb, 0x04); //Dark2 1432
HI351_write_cmos_sensor(0xec, 0x33);
HI351_write_cmos_sensor(0xed, 0x04); //Dark2 1433
HI351_write_cmos_sensor(0xee, 0x34);
HI351_write_cmos_sensor(0xef, 0x0a); //Dark2 1434
HI351_write_cmos_sensor(0xf0, 0x35);
HI351_write_cmos_sensor(0xf1, 0x46); //Dark2 1435
HI351_write_cmos_sensor(0xf2, 0x36);
HI351_write_cmos_sensor(0xf3, 0x32); //Dark2 1436
HI351_write_cmos_sensor(0xf4, 0x37);
HI351_write_cmos_sensor(0xf5, 0x28); //Dark2 1437
HI351_write_cmos_sensor(0xf6, 0x38);
HI351_write_cmos_sensor(0xf7, 0x12); //Dark2 1438
HI351_write_cmos_sensor(0xf8, 0x39);
HI351_write_cmos_sensor(0xf9, 0x00); //Dark2 1439
HI351_write_cmos_sensor(0xfa, 0x3a);
HI351_write_cmos_sensor(0xfb, 0x18); //Dark2 143a dr gain
HI351_write_cmos_sensor(0xfc, 0x3b);
HI351_write_cmos_sensor(0xfd, 0x20); //Dark2 143b
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end

HI351_write_cmos_sensor(0x03, 0xe5);
HI351_write_cmos_sensor(0x0e, 0x01); //burst_start
HI351_write_cmos_sensor(0x10, 0x3c);
HI351_write_cmos_sensor(0x11, 0x18); //Dark2 143c
HI351_write_cmos_sensor(0x12, 0x3d);
HI351_write_cmos_sensor(0x13, 0x20); //Dark2 143d nor gain
HI351_write_cmos_sensor(0x14, 0x3e);
HI351_write_cmos_sensor(0x15, 0x22); //Dark2 143e
HI351_write_cmos_sensor(0x16, 0x3f);
HI351_write_cmos_sensor(0x17, 0x10); //Dark2 143f
HI351_write_cmos_sensor(0x18, 0x40);
HI351_write_cmos_sensor(0x19, 0x03); //Dark2 1440
HI351_write_cmos_sensor(0x1a, 0x41);
HI351_write_cmos_sensor(0x1b, 0x12); //Dark2 1441
HI351_write_cmos_sensor(0x1c, 0x42);
HI351_write_cmos_sensor(0x1d, 0xb0); //Dark2 1442
HI351_write_cmos_sensor(0x1e, 0x43);
HI351_write_cmos_sensor(0x1f, 0x20); //Dark2 1443
HI351_write_cmos_sensor(0x20, 0x44);
HI351_write_cmos_sensor(0x21, 0x0a); //Dark2 1444
HI351_write_cmos_sensor(0x22, 0x45);
HI351_write_cmos_sensor(0x23, 0x0a); //Dark2 1445
HI351_write_cmos_sensor(0x24, 0x46);
HI351_write_cmos_sensor(0x25, 0x0a); //Dark2 1446
HI351_write_cmos_sensor(0x26, 0x47);
HI351_write_cmos_sensor(0x27, 0x08); //Dark2 1447
HI351_write_cmos_sensor(0x28, 0x48);
HI351_write_cmos_sensor(0x29, 0x08); //Dark2 1448
HI351_write_cmos_sensor(0x2a, 0x49);
HI351_write_cmos_sensor(0x2b, 0x08); //Dark2 1449
HI351_write_cmos_sensor(0x2c, 0x50);
HI351_write_cmos_sensor(0x2d, 0x03); //Dark2 1450
HI351_write_cmos_sensor(0x2e, 0x51);
HI351_write_cmos_sensor(0x2f, 0x32); //Dark2 1451
HI351_write_cmos_sensor(0x30, 0x52);
HI351_write_cmos_sensor(0x31, 0x40); //Dark2 1452
HI351_write_cmos_sensor(0x32, 0x53);
HI351_write_cmos_sensor(0x33, 0x19); //Dark2 1453
HI351_write_cmos_sensor(0x34, 0x54);
HI351_write_cmos_sensor(0x35, 0x60); //Dark2 1454
HI351_write_cmos_sensor(0x36, 0x55);
HI351_write_cmos_sensor(0x37, 0x60); //Dark2 1455
HI351_write_cmos_sensor(0x38, 0x56);
HI351_write_cmos_sensor(0x39, 0x60); //Dark2 1456
HI351_write_cmos_sensor(0x3a, 0x57);
HI351_write_cmos_sensor(0x3b, 0x20); //Dark2 1457
HI351_write_cmos_sensor(0x3c, 0x58);
HI351_write_cmos_sensor(0x3d, 0x20); //Dark2 1458
HI351_write_cmos_sensor(0x3e, 0x59);
HI351_write_cmos_sensor(0x3f, 0x20); //Dark2 1459
HI351_write_cmos_sensor(0x40, 0x60);
HI351_write_cmos_sensor(0x41, 0x03); //Dark2 1460 skin opt en
HI351_write_cmos_sensor(0x42, 0x61);
HI351_write_cmos_sensor(0x43, 0xa0); //Dark2 1461
HI351_write_cmos_sensor(0x44, 0x62);
HI351_write_cmos_sensor(0x45, 0x98); //Dark2 1462
HI351_write_cmos_sensor(0x46, 0x63);
HI351_write_cmos_sensor(0x47, 0xe4); //Dark2 1463 skin_std_th_h
HI351_write_cmos_sensor(0x48, 0x64);
HI351_write_cmos_sensor(0x49, 0xa4); //Dark2 1464 skin_std_th_l
HI351_write_cmos_sensor(0x4a, 0x65);
HI351_write_cmos_sensor(0x4b, 0x7d); //Dark2 1465 sharp_std_th_h
HI351_write_cmos_sensor(0x4c, 0x66);
HI351_write_cmos_sensor(0x4d, 0x4b); //Dark2 1466 sharp_std_th_l
HI351_write_cmos_sensor(0x4e, 0x70);
HI351_write_cmos_sensor(0x4f, 0x10); //Dark2 1470
HI351_write_cmos_sensor(0x50, 0x71);
HI351_write_cmos_sensor(0x51, 0x10); //Dark2 1471
HI351_write_cmos_sensor(0x52, 0x72);
HI351_write_cmos_sensor(0x53, 0x10); //Dark2 1472
HI351_write_cmos_sensor(0x54, 0x73);
HI351_write_cmos_sensor(0x55, 0x10); //Dark2 1473
HI351_write_cmos_sensor(0x56, 0x74);
HI351_write_cmos_sensor(0x57, 0x10); //Dark2 1474
HI351_write_cmos_sensor(0x58, 0x75);
HI351_write_cmos_sensor(0x59, 0x10); //Dark2 1475
HI351_write_cmos_sensor(0x5a, 0x76);
HI351_write_cmos_sensor(0x5b, 0x28); //Dark2 1476
HI351_write_cmos_sensor(0x5c, 0x77);
HI351_write_cmos_sensor(0x5d, 0x28); //Dark2 1477
HI351_write_cmos_sensor(0x5e, 0x78);
HI351_write_cmos_sensor(0x5f, 0x28); //Dark2 1478
HI351_write_cmos_sensor(0x60, 0x79);
HI351_write_cmos_sensor(0x61, 0x28); //Dark2 1479
HI351_write_cmos_sensor(0x62, 0x7a);
HI351_write_cmos_sensor(0x63, 0x28); //Dark2 147a
HI351_write_cmos_sensor(0x64, 0x7b);
HI351_write_cmos_sensor(0x65, 0x28); //Dark2 147b
HI351_write_cmos_sensor(0x66, 0x03);
HI351_write_cmos_sensor(0x67, 0x10); //10 page		//saturation if on more vivid
HI351_write_cmos_sensor(0x68, 0x60);
HI351_write_cmos_sensor(0x69, 0x01); //1060 on : 03, off : 01
HI351_write_cmos_sensor(0x6a, 0x70);
HI351_write_cmos_sensor(0x6b, 0x0c); //1070
HI351_write_cmos_sensor(0x6c, 0x71);
HI351_write_cmos_sensor(0x6d, 0x08); //1071
HI351_write_cmos_sensor(0x6e, 0x72);
HI351_write_cmos_sensor(0x6f, 0xed); //1072
HI351_write_cmos_sensor(0x70, 0x73);
HI351_write_cmos_sensor(0x71, 0x00); //1073
HI351_write_cmos_sensor(0x72, 0x74);
HI351_write_cmos_sensor(0x73, 0x36); //1074
HI351_write_cmos_sensor(0x74, 0x75);
HI351_write_cmos_sensor(0x75, 0x08); //1075
HI351_write_cmos_sensor(0x76, 0x76);
HI351_write_cmos_sensor(0x77, 0x13); //1076
HI351_write_cmos_sensor(0x78, 0x77);
HI351_write_cmos_sensor(0x79, 0x20); //1077
HI351_write_cmos_sensor(0x7a, 0x78);
HI351_write_cmos_sensor(0x7b, 0x00); //1078
HI351_write_cmos_sensor(0x7c, 0x79);
HI351_write_cmos_sensor(0x7d, 0x40); //1079
HI351_write_cmos_sensor(0x7e, 0x7a);
HI351_write_cmos_sensor(0x7f, 0x00); //107a
HI351_write_cmos_sensor(0x80, 0x7b);
HI351_write_cmos_sensor(0x81, 0x39); //107b
HI351_write_cmos_sensor(0x82, 0x7c);
HI351_write_cmos_sensor(0x83, 0x99); //107c
HI351_write_cmos_sensor(0x84, 0x7d);
HI351_write_cmos_sensor(0x85, 0x0e); //107d
HI351_write_cmos_sensor(0x86, 0x7e);
HI351_write_cmos_sensor(0x87, 0x1e); //107e
HI351_write_cmos_sensor(0x88, 0x7f);
HI351_write_cmos_sensor(0x89, 0x3c); //107f

HI351_write_cmos_sensor(0x8a, 0x03); //16 page  //dark condition reduce colo noise
HI351_write_cmos_sensor(0x8b, 0x16);
HI351_write_cmos_sensor(0x8c, 0x8a); //168a
HI351_write_cmos_sensor(0x8d, 0x60);
HI351_write_cmos_sensor(0x8e, 0x8b); //168b
HI351_write_cmos_sensor(0x8f, 0x68);
HI351_write_cmos_sensor(0x90, 0x8c); //168c
HI351_write_cmos_sensor(0x91, 0x6a);
HI351_write_cmos_sensor(0x92, 0x8d); //168d
HI351_write_cmos_sensor(0x93, 0x74);
HI351_write_cmos_sensor(0x94, 0x8e); //168e
HI351_write_cmos_sensor(0x95, 0x7a);
HI351_write_cmos_sensor(0x96, 0x8f); //168f
HI351_write_cmos_sensor(0x97, 0x7f);
HI351_write_cmos_sensor(0x98, 0x90); //1690
HI351_write_cmos_sensor(0x99, 0x7f);
HI351_write_cmos_sensor(0x9a, 0x91); //1691
HI351_write_cmos_sensor(0x9b, 0x7f);
HI351_write_cmos_sensor(0x9c, 0x92); //1692
HI351_write_cmos_sensor(0x9d, 0x7f);
HI351_write_cmos_sensor(0x0e, 0x00); //burst_end

// DMA END

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xF1); //Sleep mode on

HI351_write_cmos_sensor(0x03, 0xc0);
HI351_write_cmos_sensor(0x16, 0x80); //MCU main roof holding off

HI351_write_cmos_sensor(0x03, 0xC0);
HI351_write_cmos_sensor(0x33, 0x01); //DMA hand shake mode set
HI351_write_cmos_sensor(0x32, 0x01); //DMA off
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x11, 0x04); //Bit[0]: MCU hold off

HI351_write_cmos_sensor(0x03, 0xc0);
HI351_write_cmos_sensor(0xe1, 0x00);

HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x25, 0x0e);
HI351_write_cmos_sensor(0x25, 0x1e);
///////////////////////////////////////////
// 1F Page SSD
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x1f); //1F page
HI351_write_cmos_sensor(0x11, 0x00); //bit[5:4]: debug mode
HI351_write_cmos_sensor(0x12, 0x60);
HI351_write_cmos_sensor(0x13, 0x14);
HI351_write_cmos_sensor(0x14, 0x10);
HI351_write_cmos_sensor(0x15, 0x00);
HI351_write_cmos_sensor(0x20, 0x18); //ssd_x_start_pos
HI351_write_cmos_sensor(0x21, 0x14); //ssd_y_start_pos
HI351_write_cmos_sensor(0x22, 0x8C); //ssd_blk_width
HI351_write_cmos_sensor(0x23, 0x9A); //for Oneline 2200 0x9C -> 0x9A,ssd_blk_height
HI351_write_cmos_sensor(0x28, 0x18);
HI351_write_cmos_sensor(0x29, 0x02);
HI351_write_cmos_sensor(0x3B, 0x18);
HI351_write_cmos_sensor(0x3C, 0x8C);
HI351_write_cmos_sensor(0x10, 0x19); //SSD enable

HI351_write_cmos_sensor(0x03, 0xc4); //AE en
HI351_write_cmos_sensor(0x66, 0x08); //AE stat
HI351_write_cmos_sensor(0x67, 0x00); //AE stat
HI351_write_cmos_sensor(0x10, 0xff); //AE ON & Reset
HI351_write_cmos_sensor(0x03, 0xc3); //AE Static en
HI351_write_cmos_sensor(0x10, 0x84);

mdelay(20);

///////////////////////////////////////////
// 30 Page DMA address set
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x30); //DMA
HI351_write_cmos_sensor(0x7c, 0x2c); //Extra str
HI351_write_cmos_sensor(0x7d, 0xce);
HI351_write_cmos_sensor(0x7e, 0x2d); //Extra end
HI351_write_cmos_sensor(0x7f, 0xbb);
HI351_write_cmos_sensor(0x80, 0x24); //Outdoor str
HI351_write_cmos_sensor(0x81, 0x70);
HI351_write_cmos_sensor(0x82, 0x27); //Outdoor end
HI351_write_cmos_sensor(0x83, 0x39);
HI351_write_cmos_sensor(0x84, 0x21); //Indoor str
HI351_write_cmos_sensor(0x85, 0xa6);
HI351_write_cmos_sensor(0x86, 0x24); //Indoor end
HI351_write_cmos_sensor(0x87, 0x6f);
HI351_write_cmos_sensor(0x88, 0x27); //Dark1 str
HI351_write_cmos_sensor(0x89, 0x3a);
HI351_write_cmos_sensor(0x8a, 0x2a); //Dark1 end
HI351_write_cmos_sensor(0x8b, 0x03);
HI351_write_cmos_sensor(0x8c, 0x2a); //Dark2 str
HI351_write_cmos_sensor(0x8d, 0x04);
HI351_write_cmos_sensor(0x8e, 0x2c); //Dark2 end
HI351_write_cmos_sensor(0x8f, 0xcd);



/////////////////////////////////////////////
// CD Page (Color ratio)
/////////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0xCD);//
HI351_write_cmos_sensor(0x10, 0x38);//ColorRatio disable


HI351_write_cmos_sensor(0x03, 0xc9); //AWB Start Point
HI351_write_cmos_sensor(0x2a, 0x00);
HI351_write_cmos_sensor(0x2b, 0xb2);
HI351_write_cmos_sensor(0x2c, 0x00);
HI351_write_cmos_sensor(0x2d, 0x82);
HI351_write_cmos_sensor(0x2e, 0x00);
HI351_write_cmos_sensor(0x2f, 0xb2);
HI351_write_cmos_sensor(0x30, 0x00);
HI351_write_cmos_sensor(0x31, 0x82);

HI351_write_cmos_sensor(0x03, 0xc5); //AWB en
HI351_write_cmos_sensor(0x10, 0xb1);

HI351_write_cmos_sensor(0x03, 0xcf); //Adative en
HI351_write_cmos_sensor(0x10, 0xaf); //

///////////////////////////////////////////
// 48 Page MIPI setting
///////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x48);
HI351_write_cmos_sensor(0x09, 0xa6); //MIPI CLK
HI351_write_cmos_sensor(0x10, 0x1C); //MIPI ON
HI351_write_cmos_sensor(0x11, 0x00); //Normal Mode //[4] '1' contenous, '0'non-contenous
HI351_write_cmos_sensor(0x14, 0x50); //Skew
HI351_write_cmos_sensor(0x16, 0x04);

HI351_write_cmos_sensor(0x1a, 0x11);
HI351_write_cmos_sensor(0x1b, 0x0d); //Short Packet
HI351_write_cmos_sensor(0x1c, 0x01); //Control DP
HI351_write_cmos_sensor(0x1d, 0x0f); //Control DN
HI351_write_cmos_sensor(0x1e, 0x09);
//HI351_write_cmos_sensor(0x1f, 0x04); // 20120305 LSW 0x07);
HI351_write_cmos_sensor(0x1f, 0x05);
HI351_write_cmos_sensor(0x20, 0x00);
HI351_write_cmos_sensor(0x24, 0x1e); //Bayer8 : 2a, Bayer10 : 2b, YUV : 1e

HI351_write_cmos_sensor(0x30, 0x00); //2048*2
HI351_write_cmos_sensor(0x31, 0x05);
//HI351_write_cmos_sensor(0x32, 0x0f); // Tclk zero
//HI351_write_cmos_sensor(0x32, 0x0b); //tck 00 384Mbps 0b
//HI351_write_cmos_sensor(0x34, 0x03); // Tclk prepare 384Mbps 03
HI351_write_cmos_sensor(0x34, 0x04); // Tclk prepare 432Mbps
//HI351_write_cmos_sensor(0x35, 0x04); //20120305 LSW 0x06(default)

HI351_write_cmos_sensor(0x39, 0x03); //Drivability 00

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x0c, 0xf0); //Parallel Line off

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x11, 0x80); // STEVE 0frame skip, XY flip
HI351_write_cmos_sensor(0x01, 0xf0); //sleep off

HI351_write_cmos_sensor(0x03, 0xC0);
HI351_write_cmos_sensor(0x33, 0x00);
HI351_write_cmos_sensor(0x32, 0x01); //DMA on

//////////////////////////////////////////////
// Delay
//////////////////////////////////////////////
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x03, 0x00);


}

#if 0
kal_uint32 HI351_read_shutter(void)
{
	//kal_uint8 temp_reg0, temp_reg1, temp_reg2;
	kal_uint32 shutter = 0;


	return shutter;
}

static void HI351_write_shutter(kal_uint16 shutter)
{

}
#endif

void HI351_night_mode(kal_bool enable)
{

if (HI351_sensor_cap_state == KAL_TRUE)
{
return ;
}

if (enable)
{
if (HI351_VEDIO_encode_mode == KAL_TRUE)
{
HI351_write_cmos_sensor(0x03, 0x10);
HI351_write_cmos_sensor(0x41, 0x20);
}
else
{
HI351_write_cmos_sensor(0x03, 0x10);
HI351_write_cmos_sensor(0x41, 0x20);
}
}
else
{
if (HI351_VEDIO_encode_mode == KAL_TRUE)
{
HI351_write_cmos_sensor(0x03, 0x10);
HI351_write_cmos_sensor(0x41, 0x00);
}
else
{
HI351_write_cmos_sensor(0x03, 0x10);
HI351_write_cmos_sensor(0x41, 0x00);
}
}
}

#if 0
/* Register setting from capture to preview. */
static void HI351_set_XGA_mode(void)
{

	return;

}
#endif

void HI351_Initial_Cmds(void)
{
	//kal_uint16 i,cnt;
    HI351_Init_Cmds();
}

static void HI351_set_mirror_flip(kal_uint8 image_mirror);

UINT32 HI351Open(void)
{
	volatile signed char i;
	kal_uint32 sensor_id=0;
//	kal_uint8 temp_sccb_addr = 0;

	HI351_write_cmos_sensor(0x03, 0x00);
	HI351_write_cmos_sensor(0x01, 0xf1);
	HI351_write_cmos_sensor(0x01, 0xf3);
	HI351_write_cmos_sensor(0x01, 0xf1);

	HI351_write_cmos_sensor(0x01, 0xf1);
	HI351_write_cmos_sensor(0x01, 0xf3);
	HI351_write_cmos_sensor(0x01, 0xf1);

    do{
        for (i=0; i < 3; i++)
        {

            sensor_id = HI351_read_cmos_sensor(0x04);

            if (sensor_id == HI351MIPI_SENSOR_ID)
            {
#ifdef HI351_DEBUG
                printk("[HI351YUV]:Read Sensor ID succ:0x%x\n", sensor_id);
#endif
                break;
            }
        }

        mdelay(20);
    }while(0);


    if (sensor_id != HI351MIPI_SENSOR_ID)
	{

#ifdef HI351_DEBUG
	    printk("[HI351YUV]:Read Sensor ID fail:0x%x\n", sensor_id);
#endif
		return ERROR_SENSOR_CONNECT_FAIL;
	}

#ifdef HI351_DEBUG
        printk("[HI351YUV]:Read Sensor ID pass:0x%x\n", sensor_id);
#endif

	HI351_Initial_Cmds();
    HI351_set_mirror_flip(3);

	return ERROR_NONE;
}

UINT32 HI351Close(void)
{
	//CISModulePowerOn(FALSE);
	//kdModulePowerOn((CAMERA_DUAL_CAMERA_SENSOR_ENUM) g_currDualSensorIdx, g_currSensorName,false, CAMERA_HW_DRVNAME);
	return ERROR_NONE;
}

static void HI351_set_mirror_flip(kal_uint8 image_mirror)
{
    kal_uint8 HI351_HV_Mirror;

    HI351_write_cmos_sensor(0x03,0x00);
	HI351_HV_Mirror = (HI351_read_cmos_sensor(0x11) & 0xfc);

    switch (image_mirror) {
    case IMAGE_NORMAL:
    	HI351_HV_Mirror |= 0x00;
        break;
    case IMAGE_H_MIRROR:
        HI351_HV_Mirror |= 0x02;
        break;
    case IMAGE_V_MIRROR:
        HI351_HV_Mirror |= 0x01;
        break;
    case IMAGE_HV_MIRROR:
        HI351_HV_Mirror |= 0x03;
        break;
    default:
        break;
    }
	HI351_write_cmos_sensor(0x11, HI351_HV_Mirror);


}

UINT32 HI351Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
kal_uint16  iStartX = 0, iStartY = 0;

HI351_sensor_cap_state = KAL_FALSE;
HI351_sensor_pclk=390;

HI351_gPVmode = KAL_TRUE;

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf1); //Sleep on

HI351_write_cmos_sensor(0x03, 0x30); //DMA&Adaptive Off
HI351_write_cmos_sensor(0x36, 0xa3);
mdelay(10);

HI351_write_cmos_sensor(0x03, 0xc4); //AE off
HI351_write_cmos_sensor(0x10, 0x60);
HI351_write_cmos_sensor(0x03, 0xc5); //AWB off
HI351_write_cmos_sensor(0x10, 0x30);

//preview mode off
HI351_write_cmos_sensor(0x03, 0x15);  //Shading
HI351_write_cmos_sensor(0x10, 0x81);  //
HI351_write_cmos_sensor(0x20, 0x08);  //Shading Width 2048
HI351_write_cmos_sensor(0x21, 0x00);
HI351_write_cmos_sensor(0x22, 0x06);  //Shading Height 768
HI351_write_cmos_sensor(0x23, 0x00);
HI351_write_cmos_sensor(0x03, 0x19);
HI351_write_cmos_sensor(0x10, 0x00);//MODE_ZOOM
HI351_write_cmos_sensor(0x11, 0x00);//MODE_ZOOM2
HI351_write_cmos_sensor(0x12, 0x06);//ZOOM_CONFIG
HI351_write_cmos_sensor(0x13, 0x01);//Test Setting

HI351_write_cmos_sensor(0x20, 0x05);//ZOOM_DST_WIDTH_H
HI351_write_cmos_sensor(0x21, 0x00);//ZOOM_DST_WIDTH_L
HI351_write_cmos_sensor(0x22, 0x03);//ZOOM_DST_HEIGHT_H
HI351_write_cmos_sensor(0x23, 0xc0);//ZOOM_DST_HEIGHT_L
HI351_write_cmos_sensor(0x24, 0x00);//ZOOM_WIN_STX_H
HI351_write_cmos_sensor(0x25, 0x00);//ZOOM_WIN_STX_L	 // STEVE00 3-) 1
HI351_write_cmos_sensor(0x26, 0x00);//ZOOM_WIN_STY_H
HI351_write_cmos_sensor(0x27, 0x00);//ZOOM_WIN_STY_L
HI351_write_cmos_sensor(0x28, 0x05);//ZOOM_WIN_ENX_H
HI351_write_cmos_sensor(0x29, 0x00);//ZOOM_WIN_ENX_L	 //STEVE00 83 -) 81
HI351_write_cmos_sensor(0x2a, 0x03);//ZOOM_WIN_ENY_H
HI351_write_cmos_sensor(0x2b, 0xc0);//ZOOM_WIN_ENY_L
HI351_write_cmos_sensor(0x2c, 0x0c);//ZOOM_VER_STEP_H
HI351_write_cmos_sensor(0x2d, 0xcc);//ZOOM_VER_STEP_L
HI351_write_cmos_sensor(0x2e, 0x0c);//ZOOM_HOR_STEP_H
HI351_write_cmos_sensor(0x2f, 0xcc);//ZOOM_HOR_STEP_L
HI351_write_cmos_sensor(0x30, 0x7c);//ZOOM_FIFO_DELAY


HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x10, 0x10); //Sub1/2 + Pre1
HI351_write_cmos_sensor(0x11, 0x80); // STEVE 0 skip Fix Frame Off, XY Flip
HI351_write_cmos_sensor(0x13, 0x80); //Fix AE Set Off
HI351_write_cmos_sensor(0x14, 0x20);
HI351_write_cmos_sensor(0x17, 0x04); // for Preview

HI351_write_cmos_sensor(0x20, 0x00); //Start Height
HI351_write_cmos_sensor(0x21, 0x04);
HI351_write_cmos_sensor(0x22, 0x00); //Start Width
HI351_write_cmos_sensor(0x23, 0x0a);

HI351_write_cmos_sensor(0x24, 0x06);
HI351_write_cmos_sensor(0x25, 0x00);
HI351_write_cmos_sensor(0x26, 0x08);
HI351_write_cmos_sensor(0x27, 0x00);

HI351_write_cmos_sensor(0x03, 0xFE);
HI351_write_cmos_sensor(0xFE, 0x2c); //Delay 10ms //important
mdelay(10);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x20, 0x00);
HI351_write_cmos_sensor(0x21, 0x01); //preview row start set.

HI351_write_cmos_sensor(0x03, 0x14);
HI351_write_cmos_sensor(0x10, 0x00);

HI351_write_cmos_sensor(0x03, 0x20); //Page 20
HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x12, 0x2d);
HI351_write_cmos_sensor(0x71, 0x80);//digital gain max
HI351_write_cmos_sensor(0x72, 0x80);//digital gain min

HI351_write_cmos_sensor(0x24, 0x00); //EXP Max 12.50 fps
HI351_write_cmos_sensor(0x25, 0x41);
HI351_write_cmos_sensor(0x26, 0xcb);
HI351_write_cmos_sensor(0x27, 0xc0);
HI351_write_cmos_sensor(0x28, 0x00); //EXPMin 24545.45 fps
HI351_write_cmos_sensor(0x29, 0x08);
HI351_write_cmos_sensor(0x2a, 0x98);

HI351_write_cmos_sensor(0x3c, 0x00); //EXP Fix 30.01 fps
HI351_write_cmos_sensor(0x3d, 0x1b);
HI351_write_cmos_sensor(0x3e, 0x75);
HI351_write_cmos_sensor(0x3f, 0xb0);

HI351_write_cmos_sensor(0x30, 0x08); //EXP100
HI351_write_cmos_sensor(0x31, 0x39);
HI351_write_cmos_sensor(0x32, 0x78);
HI351_write_cmos_sensor(0x33, 0x06); //EXP120
HI351_write_cmos_sensor(0x34, 0xd9);
HI351_write_cmos_sensor(0x35, 0x20);
HI351_write_cmos_sensor(0x36, 0x00); //EXP Unit
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x38, 0x98);

HI351_write_cmos_sensor(0x51, 0xf0); //ag max

HI351_write_cmos_sensor(0x03, 0xc4);
HI351_write_cmos_sensor(0x19, 0x4a); //Bnd0 Gain
HI351_write_cmos_sensor(0x1e, 0x00); //Bnd0 33.3fps
HI351_write_cmos_sensor(0x1f, 0x18);
HI351_write_cmos_sensor(0x20, 0xac);
HI351_write_cmos_sensor(0x21, 0x68);
HI351_write_cmos_sensor(0x1a, 0x54); //Bnd1 Gain
HI351_write_cmos_sensor(0x22, 0x00); //Bnd1 20fps
HI351_write_cmos_sensor(0x23, 0x29);
HI351_write_cmos_sensor(0x24, 0x1f);
HI351_write_cmos_sensor(0x25, 0x58);
HI351_write_cmos_sensor(0x1b, 0x5c); //Bnd2 Gain
HI351_write_cmos_sensor(0x26, 0x00); //Bnd2 12.5fps
HI351_write_cmos_sensor(0x27, 0x41);
HI351_write_cmos_sensor(0x28, 0xcb);
HI351_write_cmos_sensor(0x29, 0xc0);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x13, 0x80); //Fix AE Set off
HI351_write_cmos_sensor(0x11, 0x90); //Fix Off

HI351_write_cmos_sensor(0x03, 0x48);
HI351_write_cmos_sensor(0x10, 0x1C); //MIPI On
HI351_write_cmos_sensor(0x16, 0x04);
HI351_write_cmos_sensor(0x30, 0x00); //1024 * 2
HI351_write_cmos_sensor(0x31, 0x08);

HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x36, 0x28); //Preview set //important

HI351_write_cmos_sensor(0x03, 0xFE);
HI351_write_cmos_sensor(0xFE, 0x2c); //Delay 10ms
mdelay(10);

HI351_write_cmos_sensor(0x03, 0x11);
HI351_write_cmos_sensor(0xf0, 0x20); //jktest 0203//for longhceer aw551//0x0d); // STEVE Dark mode for Sawtooth

HI351_write_cmos_sensor(0x03, 0xc4); //AE en
HI351_write_cmos_sensor(0x10, 0xef); //JH guide 0xea); //50hz//0xe1);//auto

HI351_write_cmos_sensor(0x03, 0xFE);
HI351_write_cmos_sensor(0xFE, 0x2c); //Delay 20ms
mdelay(20);

HI351_write_cmos_sensor(0x03, 0xc5); //AWB en
HI351_write_cmos_sensor(0x10, 0xb1);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf0); //sleep off

HI351_write_cmos_sensor(0x03, 0xcf); //Adaptive On
HI351_write_cmos_sensor(0x10, 0xaf);

HI351_write_cmos_sensor(0x03, 0xc0);
HI351_write_cmos_sensor(0x33, 0x00);
HI351_write_cmos_sensor(0x32, 0x01); //DMA On

image_window->GrabStartX = iStartX;
image_window->GrabStartY = iStartY;
image_window->ExposureWindowWidth = HI351_IMAGE_SENSOR_PV_WIDTH - 16;
image_window->ExposureWindowHeight = HI351_IMAGE_SENSOR_PV_HEIGHT - 12;
msleep(10);
// copy sensor_config_data
memcpy(&HI351SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
return ERROR_NONE;
}

UINT32 HI351Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
//kal_uint8 temp_AE_reg;
//kal_uint8 CLK_DIV_REG = 0;

/* 2M FULL Mode */
HI351_gPVmode = KAL_FALSE;

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf1); //Sleep on

HI351_write_cmos_sensor(0x03, 0x30); //DMA & Adaptive Off
HI351_write_cmos_sensor(0x36, 0xa3);//delay 10ms
mdelay(10);

HI351_write_cmos_sensor(0x03, 0xc4); //AE off
HI351_write_cmos_sensor(0x10, 0x68);
HI351_write_cmos_sensor(0x03, 0xc5); //AWB off
HI351_write_cmos_sensor(0x10, 0x30);

HI351_write_cmos_sensor(0x03, 0x19); //Scaler Off
HI351_write_cmos_sensor(0x10, 0x00);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x10, 0x00); //Full
HI351_write_cmos_sensor(0x14, 0x20);
HI351_write_cmos_sensor(0x17, 0x05); // for Full


HI351_write_cmos_sensor(0x20, 0x00); //Start Height
HI351_write_cmos_sensor(0x21, 0x04);
HI351_write_cmos_sensor(0x22, 0x00); //Start Width
HI351_write_cmos_sensor(0x23, 0x0a);

HI351_write_cmos_sensor(0x24, 0x06);
HI351_write_cmos_sensor(0x25, 0x00);
HI351_write_cmos_sensor(0x26, 0x08);
HI351_write_cmos_sensor(0x27, 0x00);

HI351_write_cmos_sensor(0x03, 0xFE);
HI351_write_cmos_sensor(0xFE, 0x2c); //Delay 10ms
mdelay(10);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x20, 0x00);
HI351_write_cmos_sensor(0x21, 0x03); //preview row start set.

HI351_write_cmos_sensor(0x03, 0x14);
HI351_write_cmos_sensor(0x10, 0x27); //sharp on

HI351_write_cmos_sensor(0x03, 0x15); //Shading
HI351_write_cmos_sensor(0x10, 0x83); // 00 - 83 LSC ON
HI351_write_cmos_sensor(0x20, 0x08);
HI351_write_cmos_sensor(0x21, 0x00);
HI351_write_cmos_sensor(0x22, 0x06);
HI351_write_cmos_sensor(0x23, 0x00);

HI351_write_cmos_sensor(0x03, 0x48); //MIPI Setting
HI351_write_cmos_sensor(0x10, 0x1C);
HI351_write_cmos_sensor(0x16, 0x04);
HI351_write_cmos_sensor(0x30, 0x00); //2048 * 2
HI351_write_cmos_sensor(0x31, 0x10);


HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x36, 0x29); //Capture

HI351_write_cmos_sensor(0x03, 0xFE);
HI351_write_cmos_sensor(0xFE, 0x2c);
mdelay(20);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf0); //sleep off

image_window->GrabStartX=1;
image_window->GrabStartY=1;
image_window->ExposureWindowWidth=HI351_IMAGE_SENSOR_FULL_WIDTH - 16;
image_window->ExposureWindowHeight=HI351_IMAGE_SENSOR_FULL_HEIGHT - 12;

mdelay(10);
// copy sensor_config_data
memcpy(&HI351SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

return ERROR_NONE;
}

UINT32 HI351GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorFullWidth=HI351_FULL_GRAB_WIDTH;
	pSensorResolution->SensorFullHeight=HI351_FULL_GRAB_HEIGHT;
	pSensorResolution->SensorPreviewWidth=HI351_PV_GRAB_WIDTH;
	pSensorResolution->SensorPreviewHeight=HI351_PV_GRAB_HEIGHT;
	pSensorResolution->SensorVideoWidth=HI351_PV_GRAB_WIDTH;
	pSensorResolution->SensorVideoHeight=HI351_PV_GRAB_HEIGHT;


	return ERROR_NONE;
}

UINT32 HI351GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	pSensorInfo->SensorPreviewResolutionX=HI351_PV_GRAB_WIDTH;
	pSensorInfo->SensorPreviewResolutionY=HI351_PV_GRAB_HEIGHT;
	pSensorInfo->SensorFullResolutionX=HI351_FULL_GRAB_WIDTH;
	pSensorInfo->SensorFullResolutionY=HI351_FULL_GRAB_HEIGHT;

	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;

	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;

	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 1;
#ifdef MIPI_INTERFACE
	pSensorInfo->SensroInterfaceType = SENSOR_INTERFACE_TYPE_MIPI;
#else
	pSensorInfo->SensroInterfaceType = SENSOR_INTERFACE_TYPE_PARALLEL;
#endif

	pSensorInfo->CaptureDelayFrame = 2;
	pSensorInfo->PreviewDelayFrame = 3;
	pSensorInfo->VideoDelayFrame = 20;
	pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
//		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
		pSensorInfo->SensorClockFreq=24;
		pSensorInfo->SensorClockDividCount=	3;
		pSensorInfo->SensorClockRisingCount= 0;
		pSensorInfo->SensorClockFallingCount= 2;
		pSensorInfo->SensorPixelClockCount= 3;
		pSensorInfo->SensorDataLatchCount= 2;
                /*ergate-004*/
		pSensorInfo->SensorGrabStartX = HI351_PV_GRAB_START_X;//0;
		pSensorInfo->SensorGrabStartY = HI351_PV_GRAB_START_Y;//0;
	#ifdef MIPI_INTERFACE
        pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;
        pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
        pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
        pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
        pSensorInfo->SensorPacketECCOrder = 1;
	#endif
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
//		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
		pSensorInfo->SensorClockFreq=24;
		pSensorInfo->SensorClockDividCount=	3;
		pSensorInfo->SensorClockRisingCount= 0;
		pSensorInfo->SensorClockFallingCount= 2;
		pSensorInfo->SensorPixelClockCount= 3;
		pSensorInfo->SensorDataLatchCount= 2;
                /*ergate-004*/
		pSensorInfo->SensorGrabStartX = HI351_FULL_GRAB_START_X;//0;
		pSensorInfo->SensorGrabStartY = HI351_FULL_GRAB_START_Y;//0;
	#ifdef MIPI_INTERFACE
        pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;
        pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
        pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
        pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
        pSensorInfo->SensorPacketECCOrder = 1;
	#endif
		break;
		default:
		pSensorInfo->SensorClockFreq=24;
		pSensorInfo->SensorClockDividCount=3;
		pSensorInfo->SensorClockRisingCount=0;
		pSensorInfo->SensorClockFallingCount=2;
		pSensorInfo->SensorPixelClockCount=3;
		pSensorInfo->SensorDataLatchCount=2;
                /*ergate-004*/
		pSensorInfo->SensorGrabStartX = HI351_PV_GRAB_START_X;//0;
		pSensorInfo->SensorGrabStartY = HI351_PV_GRAB_START_Y;//0;
		break;
	}
	//HI351_PixelClockDivider=pSensorInfo->SensorPixelClockCount;
	memcpy(pSensorConfigData, &HI351SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}

UINT32 HI351Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch (ScenarioId)
	{
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
//	case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
	HI351Preview(pImageWindow, pSensorConfigData);
	break;

	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
//	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
	HI351Capture(pImageWindow, pSensorConfigData);
	break;

	default:
	return ERROR_INVALID_SCENARIO_ID;
	}
	return TRUE;
}

BOOL HI351_set_param_wb(UINT16 para)
{
	switch (para)
	{
    case AWB_MODE_AUTO:
		HI351_write_cmos_sensor(0x03, 0xc5); /*Page c5*/
		HI351_write_cmos_sensor(0x11, 0xa4); /*adaptive on*/
		HI351_write_cmos_sensor(0x12, 0x97); /*adaptive on*/
		HI351_write_cmos_sensor(0x26, 0x00); /*indoor agl max*/
		HI351_write_cmos_sensor(0x27, 0xeb);
		HI351_write_cmos_sensor(0x28, 0x00);  /*indoor agl min*/
		HI351_write_cmos_sensor(0x29, 0x55);
		HI351_write_cmos_sensor(0x03, 0xc6); /*Page c6*/
		HI351_write_cmos_sensor(0x18, 0x46); /*RgainMin*/
		HI351_write_cmos_sensor(0x19, 0x90); /*RgainMax*/
		HI351_write_cmos_sensor(0x1a, 0x40); /*BgainMin*/
		HI351_write_cmos_sensor(0x1b, 0xa4); /*BgainMax*/
		HI351_write_cmos_sensor(0x03, 0xc5);
		HI351_write_cmos_sensor(0x10, 0xb1);
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy//YINGTIAN
		HI351_write_cmos_sensor(0x03, 0xc5); /*Page c5*/
		HI351_write_cmos_sensor(0x11, 0xa0); /*adaptive off*/
		HI351_write_cmos_sensor(0x12, 0x07); /*adaptive off*/
		HI351_write_cmos_sensor(0x26, 0x00); /*indoor agl max*/
		HI351_write_cmos_sensor(0x27, 0xff);
		HI351_write_cmos_sensor(0x28, 0x00);  /*indoor agl min*/
		HI351_write_cmos_sensor(0x29, 0xf0);
		HI351_write_cmos_sensor(0x03, 0xc6); /*Page c6*/
		HI351_write_cmos_sensor(0x18, 0x58); /*indoor R gain Min*/
		HI351_write_cmos_sensor(0x19, 0x78); /*indoor R gain Max*/
		HI351_write_cmos_sensor(0x1a, 0x60); /*indoor B gain Min*/
		HI351_write_cmos_sensor(0x1b, 0x78); /*indoor B gain Max*/
		HI351_write_cmos_sensor(0x03, 0xc5);
		HI351_write_cmos_sensor(0x10, 0xb1);
        break;
    case AWB_MODE_DAYLIGHT: //sunny//RIGUANG
		HI351_write_cmos_sensor(0x03, 0xc5); /*Page c5*/
		HI351_write_cmos_sensor(0x11, 0xa0); /*adaptive off*/
		HI351_write_cmos_sensor(0x12, 0x07); /*adaptive off*/
		HI351_write_cmos_sensor(0x26, 0x00); /*indoor agl max*/
		HI351_write_cmos_sensor(0x27, 0xf0);
		HI351_write_cmos_sensor(0x28, 0x00);  /*indoor agl min*/
		HI351_write_cmos_sensor(0x29, 0xc8);
		HI351_write_cmos_sensor(0x03, 0xc6); /*Page c6*/
		HI351_write_cmos_sensor(0x18, 0x58); /*indoor R gain Min*/
		HI351_write_cmos_sensor(0x19, 0x5b); /*indoor R gain Max*/
		HI351_write_cmos_sensor(0x1a, 0x76); /*indoor B gain Min*/
		HI351_write_cmos_sensor(0x1b, 0x7a); /*indoor B gain Max*/
		HI351_write_cmos_sensor(0x03, 0xc5);
		HI351_write_cmos_sensor(0x10, 0xb1);
        break;
    case AWB_MODE_INCANDESCENT: //office//BAICHIDENG
		HI351_write_cmos_sensor(0x03, 0xc5); /*Page c5*/
		HI351_write_cmos_sensor(0x11, 0xa0); /*adaptive off*/
		HI351_write_cmos_sensor(0x12, 0x07); /*adaptive off*/
		HI351_write_cmos_sensor(0x26, 0x00); /*indoor agl max*/
		HI351_write_cmos_sensor(0x27, 0x82);
		HI351_write_cmos_sensor(0x28, 0x00);  /*indoor agl min*/
		HI351_write_cmos_sensor(0x29, 0x50);
		HI351_write_cmos_sensor(0x03, 0xc6); /*Page c6*/
		HI351_write_cmos_sensor(0x18, 0x44); /*indoor R gain Min*/
		HI351_write_cmos_sensor(0x19, 0x90); /*indoor R gain Max*/
		HI351_write_cmos_sensor(0x1a, 0x40); /*indoor B gain Min*/
		HI351_write_cmos_sensor(0x1b, 0xa4); /*indoor B gain Max*/
		HI351_write_cmos_sensor(0x03, 0xc5);
		HI351_write_cmos_sensor(0x10, 0xb1);
        break;
    case AWB_MODE_TUNGSTEN: //home
        break;
    case AWB_MODE_FLUORESCENT://YINGGUANGDENG
		HI351_write_cmos_sensor(0x03, 0xc5); /*Page c5*/
		HI351_write_cmos_sensor(0x11, 0xa0); /*adaptive off*/
		HI351_write_cmos_sensor(0x12, 0x07); /*adaptive off*/
		HI351_write_cmos_sensor(0x26, 0x00); /*indoor agl max*/
		HI351_write_cmos_sensor(0x27, 0xb4);
		HI351_write_cmos_sensor(0x28, 0x00);  /*indoor agl min*/
		HI351_write_cmos_sensor(0x29, 0x82);
		HI351_write_cmos_sensor(0x03, 0xc6); /*Page c6*/
		HI351_write_cmos_sensor(0x18, 0x44); /*indoor R gain Min*/
		HI351_write_cmos_sensor(0x19, 0x90); /*indoor R gain Max*/
		HI351_write_cmos_sensor(0x1a, 0x40); /*indoor B gain Min*/
		HI351_write_cmos_sensor(0x1b, 0xa4); /*indoor B gain Max*/
		HI351_write_cmos_sensor(0x03, 0xc5);
		HI351_write_cmos_sensor(0x10, 0xb1);
        break;
	default:
	    return FALSE;
	}
	return TRUE;



}

BOOL HI351_set_param_effect(UINT16 para)
{

kal_uint32 ret = KAL_TRUE;
  switch (para)
  {
	case MEFFECT_OFF:
		HI351_write_cmos_sensor(0x03, 0x10);
		HI351_write_cmos_sensor(0x11, 0x03);
		HI351_write_cmos_sensor(0x12, 0xf0); /*constant off	*/
		HI351_write_cmos_sensor(0x42, 0x00); /*cb_offset	   */
		HI351_write_cmos_sensor(0x43, 0x00); /*cr_offset	   */
		HI351_write_cmos_sensor(0x44, 0x80); /*cb_constant   */
		HI351_write_cmos_sensor(0x45, 0x80); /*cr_constant   */
	break;

	case MEFFECT_SEPIA:
		HI351_write_cmos_sensor(0x03, 0x10);
		HI351_write_cmos_sensor(0x11, 0x03); /*solar off		  */
		HI351_write_cmos_sensor(0x12, 0xf3); /*constant on  */
		HI351_write_cmos_sensor(0x42, 0x00); /*cb_offset		  */
		HI351_write_cmos_sensor(0x43, 0x00); /*cr_offset		  */
		HI351_write_cmos_sensor(0x44, 0x60); /*cb_constant  */
		HI351_write_cmos_sensor(0x45, 0xa3); /*cr_constant  */
	break;

	case MEFFECT_NEGATIVE:
		HI351_write_cmos_sensor(0x03, 0x10);
		HI351_write_cmos_sensor(0x11, 0x03); /*solar off		 */
		HI351_write_cmos_sensor(0x12, 0xf8); /*negative on	*/
		HI351_write_cmos_sensor(0x42, 0x00); /*cb_offset		 */
		HI351_write_cmos_sensor(0x43, 0x00); /*cr_offset		 */
		HI351_write_cmos_sensor(0x44, 0x80); /*cb_constant	*/
		HI351_write_cmos_sensor(0x45, 0x80); /*cr_constant	*/
	break;

	case MEFFECT_SEPIAGREEN:
		HI351_write_cmos_sensor(0x03, 0x10);
		HI351_write_cmos_sensor(0x11, 0x03); /*solar off		  */
		HI351_write_cmos_sensor(0x12, 0xf3); /*constant on  */
		HI351_write_cmos_sensor(0x42, 0x00); /*cb_offset		  */
		HI351_write_cmos_sensor(0x43, 0x00); /*cr_offset		  */
		HI351_write_cmos_sensor(0x44, 0x60); /*cb_constant  */
		HI351_write_cmos_sensor(0x45, 0xa3); /*cr_constant  */
		HI351_write_cmos_sensor(0x03, 0x14); /*			  */
		HI351_write_cmos_sensor(0x80, 0x20); /*effect_ctl1_off */
	break;

	case MEFFECT_SEPIABLUE:
		HI351_write_cmos_sensor(0x03, 0x10);
		HI351_write_cmos_sensor(0x11, 0x03); /*solar off		  */
		HI351_write_cmos_sensor(0x12, 0xf3); /*constant on  */
		HI351_write_cmos_sensor(0x42, 0x00); /*cb_offset		  */
		HI351_write_cmos_sensor(0x43, 0x00); /*cr_offset		  */
		HI351_write_cmos_sensor(0x44, 0xc0); /*cb_constant  */
		HI351_write_cmos_sensor(0x45, 0x80); /*cr_constant  */
		HI351_write_cmos_sensor(0x03, 0x14); /*			  */
		HI351_write_cmos_sensor(0x80, 0x20); /*effect_ctl1_off */
	break;

	case MEFFECT_MONO:
		HI351_write_cmos_sensor(0x03, 0x10);
		HI351_write_cmos_sensor(0x11, 0x03); /*solar off		*/
		HI351_write_cmos_sensor(0x12, 0xf3); /*constant on	*/
		HI351_write_cmos_sensor(0x42, 0x00); /*cb_offset		*/
		HI351_write_cmos_sensor(0x43, 0x00); /*cr_offset		*/
		HI351_write_cmos_sensor(0x44, 0x80); /*cb_constant	*/
		HI351_write_cmos_sensor(0x45, 0x80); /*cr_constant	*/
	break;

	default:
	ret = FALSE;
	}

	return ret;



}

BOOL HI351_set_param_banding(UINT16 para)
{
 //   kal_uint8 banding;

	switch (para)
	{
	case AE_FLICKER_MODE_50HZ:
		HI351_Banding_setting = AE_FLICKER_MODE_50HZ;
		HI351_write_cmos_sensor(0x03,0x20);
		HI351_write_cmos_sensor(0x10,0x97);
	break;
	case AE_FLICKER_MODE_60HZ:
		HI351_Banding_setting = AE_FLICKER_MODE_60HZ;
		HI351_write_cmos_sensor(0x03,0x20);
		HI351_write_cmos_sensor(0x10,0x87);
	break;
	default:
	return FALSE;
	}

    return TRUE;
}

BOOL HI351_set_param_exposure(UINT16 para)
{


  HI351_write_cmos_sensor(0x03,0x00);
  //HI351_write_cmos_sensor(0x12,(HI351_read_cmos_sensor(0x12)|0x10));//make sure the Yoffset control is opened.

  switch (para)
  {
  case AE_EV_COMP_n13:
	  HI351_write_cmos_sensor(0x60,0x04);
	  break;
  case AE_EV_COMP_n10:
	  HI351_write_cmos_sensor(0x60,0x04);
	  break;
  case AE_EV_COMP_n07:
	  HI351_write_cmos_sensor(0x60,0x04);
	  break;
  case AE_EV_COMP_n03:
	  HI351_write_cmos_sensor(0x60,0x04);
	  break;
  case AE_EV_COMP_00:
	  HI351_write_cmos_sensor(0x60,0x00);
	  break;
  case AE_EV_COMP_03:
	  HI351_write_cmos_sensor(0x40,0x10);
	  break;
  case AE_EV_COMP_07:
	  HI351_write_cmos_sensor(0x40,0x20);
	  break;
  case AE_EV_COMP_10:
	  HI351_write_cmos_sensor(0x40,0x30);
	  break;
  case AE_EV_COMP_13:
	  HI351_write_cmos_sensor(0x40,0x40);
	  break;
  default:
	  return FALSE;
  }


    return TRUE;

}


UINT32 HI351YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
  if (HI351_op_state.sensor_cap_state == KAL_TRUE)	/* Don't need it when capture mode. */
	{
		return KAL_TRUE;
	}

	switch (iCmd)
	{
	case FID_SCENE_MODE:
	if (iPara == SCENE_MODE_OFF)
	{
	HI351_night_mode(0);
	}
	else if (iPara == SCENE_MODE_NIGHTSCENE)
	{
	HI351_night_mode(1);
	}
	break;

	case FID_AWB_MODE:
	HI351_set_param_wb(iPara);
	break;

	case FID_COLOR_EFFECT:
	HI351_set_param_effect(iPara);
	break;

	case FID_AE_EV:
	HI351_set_param_exposure(iPara);
	break;

	case FID_AE_FLICKER:
	HI351_set_param_banding(iPara);
	break;

	case FID_AE_SCENE_MODE:
	break;

	case FID_ZOOM_FACTOR:
	zoom_factor = iPara;
	break;

	default:
	break;
	}
	return TRUE;
}

UINT32 HI351YUVSetVideoMode(UINT16 u2FrameRate)
{


if(u2FrameRate==30)
{
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf1); //Sleep on

HI351_write_cmos_sensor(0x03, 0x30); //DMA&Adaptive Off
HI351_write_cmos_sensor(0x36, 0xa3);

HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x36, 0xa3); //DMA&Adaptive Off

HI351_write_cmos_sensor(0x03, 0xc4); //AE off
HI351_write_cmos_sensor(0x10, 0x6a);
HI351_write_cmos_sensor(0x03, 0xc5); //AWB off
HI351_write_cmos_sensor(0x10, 0x00);
mdelay(10);
//DELAY 10
//preview mode off
//HI351_write_cmos_sensor(0x03, 0x30);
//HI351_write_cmos_sensor(0x36, 0x29);

HI351_write_cmos_sensor(0x03, 0x15);  //Shading
HI351_write_cmos_sensor(0x10, 0x81);  //
HI351_write_cmos_sensor(0x20, 0x08);  //Shading Width 2048
HI351_write_cmos_sensor(0x21, 0x00);
HI351_write_cmos_sensor(0x22, 0x03);  //Shading Height 768
HI351_write_cmos_sensor(0x23, 0x00);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x10, 0x11); //Sub+Pre1
HI351_write_cmos_sensor(0x17, 0x04); //clk 1/1

HI351_write_cmos_sensor(0x20, 0x00); //Start of Height
HI351_write_cmos_sensor(0x21, 0x01);
HI351_write_cmos_sensor(0x22, 0x00); //Start of Width
HI351_write_cmos_sensor(0x23, 0x0a);

HI351_write_cmos_sensor(0x03, 0x19);
HI351_write_cmos_sensor(0x10, 0x00);//MODE_ZOOM
HI351_write_cmos_sensor(0x11, 0x00);//MODE_ZOOM2
HI351_write_cmos_sensor(0x12, 0x06);//ZOOM_CONFIG
HI351_write_cmos_sensor(0x13, 0x01);//Test Setting
HI351_write_cmos_sensor(0x20, 0x05);//ZOOM_DST_WIDTH_H
HI351_write_cmos_sensor(0x21, 0x00);//ZOOM_DST_WIDTH_L
HI351_write_cmos_sensor(0x22, 0x01);//ZOOM_DST_HEIGHT_H
HI351_write_cmos_sensor(0x23, 0xe0);//ZOOM_DST_HEIGHT_L
HI351_write_cmos_sensor(0x24, 0x00);//ZOOM_WIN_STX_H
HI351_write_cmos_sensor(0x25, 0x00);//ZOOM_WIN_STX_L	 // STEVE00 3-) 1
HI351_write_cmos_sensor(0x26, 0x00);//ZOOM_WIN_STY_H
HI351_write_cmos_sensor(0x27, 0x00);//ZOOM_WIN_STY_L
HI351_write_cmos_sensor(0x28, 0x05);//ZOOM_WIN_ENX_H
HI351_write_cmos_sensor(0x29, 0x00);//ZOOM_WIN_ENX_L	 //STEVE00 83 -) 81
HI351_write_cmos_sensor(0x2a, 0x01);//ZOOM_WIN_ENY_H
HI351_write_cmos_sensor(0x2b, 0xe0);//ZOOM_WIN_ENY_L
HI351_write_cmos_sensor(0x2c, 0x0c);//ZOOM_VER_STEP_H
HI351_write_cmos_sensor(0x2d, 0xcc);//ZOOM_VER_STEP_L
HI351_write_cmos_sensor(0x2e, 0x0c);//ZOOM_HOR_STEP_H
HI351_write_cmos_sensor(0x2f, 0xcc);//ZOOM_HOR_STEP_L
HI351_write_cmos_sensor(0x30, 0x7c);//ZOOM_FIFO_DELAY
mdelay(10);

HI351_write_cmos_sensor(0x03, 0x20); //Page 20
HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x12, 0x6d);
HI351_write_cmos_sensor(0x71, 0xf0);//digital gain max
HI351_write_cmos_sensor(0x72, 0xf0);//digital gain min

HI351_write_cmos_sensor(0x24, 0x00); //EXP Max 33.33 fps
HI351_write_cmos_sensor(0x25, 0x18);
HI351_write_cmos_sensor(0x26, 0xac);
HI351_write_cmos_sensor(0x27, 0x68);
HI351_write_cmos_sensor(0x28, 0x00); //EXPMin 24545.45 fps
HI351_write_cmos_sensor(0x29, 0x08);
HI351_write_cmos_sensor(0x2a, 0x98);

HI351_write_cmos_sensor(0x3c, 0x00); //EXP Fix 30.01 fps
HI351_write_cmos_sensor(0x3d, 0x1b);
HI351_write_cmos_sensor(0x3e, 0x75);
HI351_write_cmos_sensor(0x3f, 0xb0);

HI351_write_cmos_sensor(0x30, 0x08); //EXP100
HI351_write_cmos_sensor(0x31, 0x39);
HI351_write_cmos_sensor(0x32, 0x78);
HI351_write_cmos_sensor(0x33, 0x06); //EXP120
HI351_write_cmos_sensor(0x34, 0xd9);
HI351_write_cmos_sensor(0x35, 0x20);
HI351_write_cmos_sensor(0x36, 0x00); //EXP Unit
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x38, 0x98);

HI351_write_cmos_sensor(0x51, 0xf0); //ag max

HI351_write_cmos_sensor(0x03, 0xc4);
HI351_write_cmos_sensor(0x19, 0x24); //Bnd0 Gain
HI351_write_cmos_sensor(0x1e, 0x00); //Bnd0 100fps
HI351_write_cmos_sensor(0x1f, 0x08);
HI351_write_cmos_sensor(0x20, 0x39);
HI351_write_cmos_sensor(0x21, 0x78);
HI351_write_cmos_sensor(0x1a, 0x24); //Bnd1 Gain
HI351_write_cmos_sensor(0x22, 0x00); //Bnd1 50fps
HI351_write_cmos_sensor(0x23, 0x10);
HI351_write_cmos_sensor(0x24, 0x72);
HI351_write_cmos_sensor(0x25, 0xf0);
HI351_write_cmos_sensor(0x1b, 0x24); //Bnd2 Gain
HI351_write_cmos_sensor(0x26, 0x00); //Bnd2 33.3fps
HI351_write_cmos_sensor(0x27, 0x18);
HI351_write_cmos_sensor(0x28, 0xac);
HI351_write_cmos_sensor(0x29, 0x68);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x13, 0xa8); //Fix AE Set on
HI351_write_cmos_sensor(0x11, 0x94); //Fix On

mdelay(10);
//DELAY 10


HI351_write_cmos_sensor(0x03, 0xcf); //Adaptive On
HI351_write_cmos_sensor(0x10, 0xaf);

HI351_write_cmos_sensor(0x03, 0xc4); //AE en
HI351_write_cmos_sensor(0x10, 0xef);

mdelay(20);
//DELAY 20

HI351_write_cmos_sensor(0x03, 0xc5); //AWB en
HI351_write_cmos_sensor(0x10, 0xb1);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf0); //sleep off

HI351_write_cmos_sensor(0x03, 0xc0);
HI351_write_cmos_sensor(0x33, 0x00);
HI351_write_cmos_sensor(0x32, 0x01); //DMA On
}
else if(u2FrameRate==15)
{
HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf1); //Sleep on

HI351_write_cmos_sensor(0x03, 0x30); //DMA&Adaptive Off
HI351_write_cmos_sensor(0x36, 0xa3);

HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x03, 0x30);
HI351_write_cmos_sensor(0x36, 0xa3); //DMA&Adaptive Off

HI351_write_cmos_sensor(0x03, 0xc4); //AE off
HI351_write_cmos_sensor(0x10, 0x6a);
HI351_write_cmos_sensor(0x03, 0xc5); //AWB off
HI351_write_cmos_sensor(0x10, 0x00);
mdelay(20);
//DELAY 10
//preview mode off
//HI351_write_cmos_sensor(0x03, 0x30);
//HI351_write_cmos_sensor(0x36, 0x29);

HI351_write_cmos_sensor(0x03, 0x15);  //Shading
HI351_write_cmos_sensor(0x10, 0x81);  //
HI351_write_cmos_sensor(0x20, 0x08);  //Shading Width 2048
HI351_write_cmos_sensor(0x21, 0x00);
HI351_write_cmos_sensor(0x22, 0x03);  //Shading Height 768
HI351_write_cmos_sensor(0x23, 0x00);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x10, 0x11); //Sub+Pre1
HI351_write_cmos_sensor(0x17, 0x04); //clk 1/1

HI351_write_cmos_sensor(0x20, 0x00); //Start of Height
HI351_write_cmos_sensor(0x21, 0x01);
HI351_write_cmos_sensor(0x22, 0x00); //Start of Width
HI351_write_cmos_sensor(0x23, 0x0a);

HI351_write_cmos_sensor(0x03, 0x19);
HI351_write_cmos_sensor(0x10, 0x00);//MODE_ZOOM
HI351_write_cmos_sensor(0x11, 0x00);//MODE_ZOOM2
HI351_write_cmos_sensor(0x12, 0x06);//ZOOM_CONFIG
HI351_write_cmos_sensor(0x13, 0x01);//Test Setting
HI351_write_cmos_sensor(0x20, 0x05);//ZOOM_DST_WIDTH_H
HI351_write_cmos_sensor(0x21, 0x00);//ZOOM_DST_WIDTH_L
HI351_write_cmos_sensor(0x22, 0x01);//ZOOM_DST_HEIGHT_H
HI351_write_cmos_sensor(0x23, 0xe0);//ZOOM_DST_HEIGHT_L
HI351_write_cmos_sensor(0x24, 0x00);//ZOOM_WIN_STX_H
HI351_write_cmos_sensor(0x25, 0x00);//ZOOM_WIN_STX_L	 // STEVE00 3-) 1
HI351_write_cmos_sensor(0x26, 0x00);//ZOOM_WIN_STY_H
HI351_write_cmos_sensor(0x27, 0x00);//ZOOM_WIN_STY_L
HI351_write_cmos_sensor(0x28, 0x05);//ZOOM_WIN_ENX_H
HI351_write_cmos_sensor(0x29, 0x00);//ZOOM_WIN_ENX_L	 //STEVE00 83 -) 81
HI351_write_cmos_sensor(0x2a, 0x01);//ZOOM_WIN_ENY_H
HI351_write_cmos_sensor(0x2b, 0xe0);//ZOOM_WIN_ENY_L
HI351_write_cmos_sensor(0x2c, 0x0c);//ZOOM_VER_STEP_H
HI351_write_cmos_sensor(0x2d, 0xcc);//ZOOM_VER_STEP_L
HI351_write_cmos_sensor(0x2e, 0x0c);//ZOOM_HOR_STEP_H
HI351_write_cmos_sensor(0x2f, 0xcc);//ZOOM_HOR_STEP_L
HI351_write_cmos_sensor(0x30, 0x7c);//ZOOM_FIFO_DELAY
//HI351_write_cmos_sensor(SENSOR_WRITE_DELAY,0x0a); //delay 10ms

HI351_write_cmos_sensor(0x03, 0x20); //Page 20
HI351_write_cmos_sensor(0x80, 0x34);
HI351_write_cmos_sensor(0x12, 0x6d);
HI351_write_cmos_sensor(0x71, 0xf0);//digital gain max
HI351_write_cmos_sensor(0x72, 0xf0);//digital gain min

HI351_write_cmos_sensor(0x24, 0x00); //EXP Max 33.33 fps
HI351_write_cmos_sensor(0x25, 0x18);
HI351_write_cmos_sensor(0x26, 0xac);
HI351_write_cmos_sensor(0x27, 0x68);
HI351_write_cmos_sensor(0x28, 0x00); //EXPMin 24545.45 fps
HI351_write_cmos_sensor(0x29, 0x08);
HI351_write_cmos_sensor(0x2a, 0x98);

HI351_write_cmos_sensor(0x3c, 0x00); //EXP Fix 30.01 fps
HI351_write_cmos_sensor(0x3d, 0x36);
HI351_write_cmos_sensor(0x3e, 0xeb);
HI351_write_cmos_sensor(0x3f, 0x60);

HI351_write_cmos_sensor(0x30, 0x08); //EXP100
HI351_write_cmos_sensor(0x31, 0x39);
HI351_write_cmos_sensor(0x32, 0x78);
HI351_write_cmos_sensor(0x33, 0x06); //EXP120
HI351_write_cmos_sensor(0x34, 0xd9);
HI351_write_cmos_sensor(0x35, 0x20);
HI351_write_cmos_sensor(0x36, 0x00); //EXP Unit
HI351_write_cmos_sensor(0x37, 0x08);
HI351_write_cmos_sensor(0x38, 0x98);

HI351_write_cmos_sensor(0x51, 0xf0); //ag max

HI351_write_cmos_sensor(0x03, 0xc4);
HI351_write_cmos_sensor(0x19, 0x24); //Bnd0 Gain
HI351_write_cmos_sensor(0x1e, 0x00); //Bnd0 100fps
HI351_write_cmos_sensor(0x1f, 0x08);
HI351_write_cmos_sensor(0x20, 0x39);
HI351_write_cmos_sensor(0x21, 0x78);
HI351_write_cmos_sensor(0x1a, 0x24); //Bnd1 Gain
HI351_write_cmos_sensor(0x22, 0x00); //Bnd1 50fps
HI351_write_cmos_sensor(0x23, 0x10);
HI351_write_cmos_sensor(0x24, 0x72);
HI351_write_cmos_sensor(0x25, 0xf0);
HI351_write_cmos_sensor(0x1b, 0x24); //Bnd2 Gain
HI351_write_cmos_sensor(0x26, 0x00); //Bnd2 33.3fps
HI351_write_cmos_sensor(0x27, 0x18);
HI351_write_cmos_sensor(0x28, 0xac);
HI351_write_cmos_sensor(0x29, 0x68);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x13, 0xa8); //Fix AE Set on
HI351_write_cmos_sensor(0x11, 0x94); //Fix On

mdelay(10);
//DELAY 10


HI351_write_cmos_sensor(0x03, 0xcf); //Adaptive On
HI351_write_cmos_sensor(0x10, 0xaf);

HI351_write_cmos_sensor(0x03, 0xc4); //AE en
HI351_write_cmos_sensor(0x10, 0xef);

mdelay(20);
//DELAY 20

HI351_write_cmos_sensor(0x03, 0xc5); //AWB en
HI351_write_cmos_sensor(0x10, 0xb1);

HI351_write_cmos_sensor(0x03, 0x00);
HI351_write_cmos_sensor(0x01, 0xf0); //sleep off

HI351_write_cmos_sensor(0x03, 0xc0);
HI351_write_cmos_sensor(0x33, 0x00);
HI351_write_cmos_sensor(0x32, 0x01); //DMA On
}
else
{
	//printk("Videomode error \n");
}
HI351_VEDIO_encode_mode = KAL_TRUE;

return TRUE;
}

UINT32 HI351GetSensorID(UINT32 *sensorID)
{

	int retry=3;

	printk("HI351GetSensorID \n");

	do{
		*sensorID=HI351_read_cmos_sensor(0x04);

		if(*sensorID == HI351MIPI_SENSOR_ID)
        	break;

		SENSORDB("HI351GetSensorID Read Sendor ID Fail:0x%04x \n",*sensorID);
		retry--;
	}while(retry>0);

	if(*sensorID!=HI351MIPI_SENSOR_ID){

		*sensorID=0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	else{
		SENSORDB("HI351 Read Sendor ID SUCCESS:0x%04x \n",*sensorID);
		return ERROR_NONE;
	}

}
UINT32 HI351FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 u2Temp = 0;

	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;

//	PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
//	MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
//	MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
//	MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;


	switch (FeatureId)
	{
	case SENSOR_FEATURE_GET_RESOLUTION:
	*pFeatureReturnPara16++=HI351_FULL_GRAB_WIDTH;
	*pFeatureReturnPara16=HI351_FULL_GRAB_HEIGHT;
	*pFeatureParaLen=4;
	break;

	case SENSOR_FEATURE_GET_PERIOD:
	*pFeatureReturnPara16++=HI351_PV_PERIOD_PIXEL_NUMS+HI351_PV_dummy_pixels;
	*pFeatureReturnPara16=HI351_PV_PERIOD_LINE_NUMS+HI351_PV_dummy_lines;
	*pFeatureParaLen=4;
	break;

	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
	*pFeatureReturnPara32 = HI351_sensor_pclk/10;
	*pFeatureParaLen=4;
	break;

	case SENSOR_FEATURE_SET_ESHUTTER:
	u2Temp = *pFeatureData16;
	break;

	case SENSOR_FEATURE_SET_NIGHTMODE:
	HI351_night_mode((BOOL) *pFeatureData16);
	break;

	case SENSOR_FEATURE_SET_GAIN:
	break;

	case SENSOR_FEATURE_SET_FLASHLIGHT:
	break;

	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
	HI351_isp_master_clock=*pFeatureData32;
	break;

	case SENSOR_FEATURE_SET_REGISTER:
	HI351_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
	break;

	case SENSOR_FEATURE_GET_REGISTER:
	pSensorRegData->RegData = HI351_read_cmos_sensor(pSensorRegData->RegAddr);
	break;

	case SENSOR_FEATURE_GET_CONFIG_PARA:
	memcpy(pSensorConfigData, &HI351SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	*pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
	break;

	case SENSOR_FEATURE_SET_CCT_REGISTER:
	case SENSOR_FEATURE_GET_CCT_REGISTER:
	case SENSOR_FEATURE_SET_ENG_REGISTER:
	case SENSOR_FEATURE_GET_ENG_REGISTER:
	case SENSOR_FEATURE_GET_REGISTER_DEFAULT:

	case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
	case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
	case SENSOR_FEATURE_GET_GROUP_INFO:
	case SENSOR_FEATURE_GET_ITEM_INFO:
	case SENSOR_FEATURE_SET_ITEM_INFO:
	case SENSOR_FEATURE_GET_ENG_INFO:
	break;

	case SENSOR_FEATURE_GET_GROUP_COUNT:
	*pFeatureReturnPara32++=0;
	*pFeatureParaLen=4;
	break;

	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
	// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
	// if EEPROM does not exist in camera module.
	*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
	*pFeatureParaLen=4;
	break;

	case SENSOR_FEATURE_SET_YUV_CMD:
	HI351YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
	break;

	case SENSOR_FEATURE_SET_VIDEO_MODE:
	HI351YUVSetVideoMode(*pFeatureData16);
	break;

	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		 HI351GetSensorID(pFeatureData32);
		 break;

	default:
	break;
	}
	return ERROR_NONE;
}

SENSOR_FUNCTION_STRUCT	SensorFuncHI351=
{
	HI351Open,
	HI351GetInfo,
	HI351GetResolution,
	HI351FeatureControl,
	HI351Control,
	HI351Close
};

UINT32 HI351_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	if (pfFunc!=NULL)
	*pfFunc=&SensorFuncHI351;

	return ERROR_NONE;
}

