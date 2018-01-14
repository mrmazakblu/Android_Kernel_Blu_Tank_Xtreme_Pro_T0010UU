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

//s_add new sensor driver here
//export funtions
/*IMX*/
UINT32 IMX219_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 IMX214_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 IMX179_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 IMX135_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 IMX190_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
/*OV*/
UINT32 OV9760MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 OV5675_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 OV5648MIPISensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
/*S5K*/
UINT32 S5K3L8_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
/*HI*/
UINT32 HI841_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
/*GC*/
UINT32 GC2355_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC2235_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC2755_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC2365MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC2375MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC8024MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC5005MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
/*SP*/

//! Add Sensor Init function here
//! Note:
//! 1. Add by the resolution from ""large to small"", due to large sensor
//!    will be possible to be main sensor.
//!    This can avoid I2C error during searching sensor.
//! 2. This file should be the same as mediatek\custom\common\hal\imgsensor\src\sensorlist.cpp
ACDK_KD_SENSOR_INIT_FUNCTION_STRUCT kdSensorList[MAX_NUM_OF_SUPPORT_SENSOR+1] =
{
/*IMX*/
#if defined(IMX219_MIPI_RAW)
    {IMX219_SENSOR_ID, SENSOR_DRVNAME_IMX219_MIPI_RAW, IMX219_MIPI_RAW_SensorInit},
#endif
#if defined(IMX214_MIPI_RAW)
    {IMX214_SENSOR_ID, SENSOR_DRVNAME_IMX214_MIPI_RAW, IMX214_MIPI_RAW_SensorInit},
#endif
#if defined(IMX179_MIPI_RAW)
    {IMX179_SENSOR_ID, SENSOR_DRVNAME_IMX179_MIPI_RAW,IMX179_MIPI_RAW_SensorInit},
#endif
#if defined(IMX135_MIPI_RAW)
    {IMX135_SENSOR_ID, SENSOR_DRVNAME_IMX135_MIPI_RAW, IMX135_MIPI_RAW_SensorInit},
#endif
#if defined(IMX190_MIPI_RAW)
    {IMX190_SENSOR_ID, SENSOR_DRVNAME_IMX190_MIPI_RAW, IMX190_MIPI_RAW_SensorInit},
#endif
/*OV (OmniVision)*/
#if defined(OV9760_MIPI_RAW)
    {OV9760MIPI_SENSOR_ID,SENSOR_DRVNAME_OV9760_MIPI_RAW,OV9760MIPI_RAW_SensorInit},
#endif
#if defined(OV5675_MIPI_RAW)
    {OV5675MIPI_SENSOR_ID, SENSOR_DRVNAME_OV5675_MIPI_RAW, OV5675_MIPI_RAW_SensorInit},
#endif
#if defined(OV5648_MIPI_RAW)
    {OV5648MIPI_SENSOR_ID, SENSOR_DRVNAME_OV5648_MIPI_RAW, OV5648MIPISensorInit},
#endif
/*S5K*/
#if defined(S5K3L8_MIPI_RAW)
    {S5K3L8_SENSOR_ID, SENSOR_DRVNAME_S5K3L8_MIPI_RAW, S5K3L8_MIPI_RAW_SensorInit},
#endif
/*HI*/
#if defined(HI841_MIPI_RAW)
    {HI841_SENSOR_ID, SENSOR_DRVNAME_HI841_MIPI_RAW,HI841_MIPI_RAW_SensorInit},
#endif
/*GC*/
#if defined(GC2355_MIPI_RAW)
    {GC2355_SENSOR_ID, SENSOR_DRVNAME_GC2355_MIPI_RAW,GC2355_MIPI_RAW_SensorInit},
#endif
#if defined(GC2235_RAW)
    {GC2235_SENSOR_ID, SENSOR_DRVNAME_GC2235_RAW, GC2235_RAW_SensorInit},
#endif
#if defined(GC2755_MIPI_RAW)
    {GC2755_SENSOR_ID, SENSOR_DRVNAME_GC2755_MIPI_RAW,GC2755_MIPI_RAW_SensorInit},
#endif
#if defined(GC2365_MIPI_RAW)
    {GC2365MIPI_SENSOR_ID, SENSOR_DRVNAME_GC2365MIPI_RAW,GC2365MIPI_RAW_SensorInit},
#endif
#if defined(GC2375_MIPI_RAW)
    {GC2375MIPI_SENSOR_ID,SENSOR_DRVNAME_GC2375MIPI_RAW,GC2375MIPI_RAW_SensorInit},
#endif
#if defined(GC8024_MIPI_RAW)
    {GC8024MIPI_SENSOR_ID, SENSOR_DRVNAME_GC8024MIPI_RAW,GC8024MIPI_RAW_SensorInit},
#endif
#if defined(GC5005_MIPI_RAW)
    {GC5005MIPI_SENSOR_ID, SENSOR_DRVNAME_GC5005MIPI_RAW,GC5005MIPI_RAW_SensorInit},
#endif
/*  ADD sensor driver before this line */
	{0, {0}
	 , NULL}
	,			/* end of list */
};

/* e_add new sensor driver here */
