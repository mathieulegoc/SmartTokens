/*
    Pretty LAYer for ChibiOS/RT - Copyright (C) 2014 Rocco Marco Guglielmi

    This file is part of PLAY for ChibiOS/RT.

    PLAY is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    PLAY is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    Special thanks to Giovanni Di Sirio for teachings, his moral support and
    friendship. Note that some or every piece of this file could be part of
    the ChibiOS project that is intellectual property of Giovanni Di Sirio.
    Please refer to ChibiOS/RT license before use this file.
 */

/**
 * @file    lsm6ds0.h
 * @brief   LSM6DS0 MEMS interface module header.
 *
 * @{
 */

#ifndef _LSM6DS0_H_
#define _LSM6DS0_H_


#include <jendefs.h>
#include <AppHardwareApi.h>
#include "Config.h"
#include "robot_functions.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    LSM6DS0 register names
 * @{
 */


/******************************************************************************/
/*                                                                            */
/*                        LSM6DS0 on board MEMS                               */
/*                                                                            */
/******************************************************************************/

#define LSM6DS0_ADDRESS_SD0_LOW     0x6A // address pin low (GND), default for InvenSense evaluation board
#define LSM6DS0_ADDRESS_SD0_HIGH    0x6B // address pin high (VCC)
#define LSM6DS0_ADDRESS  		   LSM6DS0_ADDRESS_SD0_LOW

/*****************  Bit definition for I2C/SPI communication  *****************/
#define  SUB                         ((uint8)0x7F)            /*!< SUB[6:0] Sub-registers address Mask */
#define  SUB_0                       ((uint8)0x01)            /*!< bit 0 */
#define  SUB_1                       ((uint8)0x02)            /*!< bit 1 */
#define  SUB_2                       ((uint8)0x08)            /*!< bit 3 */
#define  SUB_4                       ((uint8)0x10)            /*!< bit 4 */
#define  SUB_5                       ((uint8)0x20)            /*!< bit 5 */
#define  SUB_6                       ((uint8)0x40)            /*!< bit 6 */

#define  SUB_MSB                     ((uint8)0x80)            /*!< Multiple data read\write bit */

/*****************  Bit definition for Registers Addresses  *******************/
#define  ACT_THS                     ((uint8)0x04)            /*!< Activity threshold register */
#define  ACT_DUR                     ((uint8)0x05)            /*!< Inactivity duration register */
#define  INT_GEN_CFG_XL              ((uint8)0x06)            /*!< Accelerometer interrupt generator configuration register */
#define  INT_GEN_THS_X_XL            ((uint8)0x07)            /*!< Accelerometer X-axis interrupt threshold register */
#define  INT_GEN_THS_Y_XL            ((uint8)0x08)            /*!< Accelerometer Y-axis interrupt threshold register */
#define  INT_GEN_THS_Z_XL            ((uint8)0x09)            /*!< Accelerometer Z-axis interrupt threshold register */
#define  INT_GEN_DUR_XL              ((uint8)0x0A)            /*!< Accelerometer interrupt duration register */
#define  REFERENCE_G                 ((uint8)0x0B)            /*!< Gyroscope reference value register for digital high-pass filter */
#define  INT_CTRL                    ((uint8)0x0C)            /*!< INT pin control register */
#define  WHO_AM_I                    ((uint8)0x0F)            /*!< Who_AM_I register */
#define  CTRL_REG1_G                 ((uint8)0x10)            /*!< Gyroscope control register 1 */
#define  CTRL_REG2_G                 ((uint8)0x11)            /*!< Gyroscope control register 2 */
#define  CTRL_REG3_G                 ((uint8)0x12)            /*!< Gyroscope control register 3 */
#define  ORIENT_CFG_G                ((uint8)0x13)            /*!< Gyroscope sign and orientation register */
#define  INT_GEN_SRC_G               ((uint8)0x14)            /*!< Gyroscope interrupt source register */
#define  OUT_TEMP_L                  ((uint8)0x15)            /*!< Temperature data output low register */
#define  OUT_TEMP_H                  ((uint8)0x16)            /*!< Temperature data output high register */
#define  STATUS_REG1                 ((uint8)0x17)            /*!< Status register 1 */
#define  OUT_X_L_G                   ((uint8)0x18)            /*!< Gyroscope X-axis low output register */
#define  OUT_X_H_G                   ((uint8)0x19)            /*!< Gyroscope X-axis high output register */
#define  OUT_Y_L_G                   ((uint8)0x1A)            /*!< Gyroscope Y-axis low output register */
#define  OUT_Y_H_G                   ((uint8)0x1B)            /*!< Gyroscope Y-axis high output register */
#define  OUT_Z_L_G                   ((uint8)0x1C)            /*!< Gyroscope Z-axis low output register */
#define  OUT_Z_H_G                   ((uint8)0x1D)            /*!< Gyroscope Z-axis high output register */
#define  CTRL_REG4                   ((uint8)0x1E)            /*!< Control register 4 */
#define  CTRL_REG5_XL                ((uint8)0x1F)            /*!< Accelerometer Control Register 5 */
#define  CTRL_REG6_XL                ((uint8)0x20)            /*!< Accelerometer Control Register 6 */
#define  CTRL_REG7_XL                ((uint8)0x21)            /*!< Accelerometer Control Register 7 */
#define  CTRL_REG8                   ((uint8)0x22)            /*!< Control register 8 */
#define  CTRL_REG9                   ((uint8)0x23)            /*!< Control register 9 */
#define  CTRL_REG10                  ((uint8)0x24)            /*!< Control register 10 */
#define  INT_GEN_SRC_XL              ((uint8)0x26)            /*!< Accelerometer interrupt source register */
#define  STATUS_REG2                 ((uint8)0x27)            /*!< Status register */
#define  OUT_X_L_XL                  ((uint8)0x28)            /*!< Accelerometer X-axis low output register */
#define  OUT_X_H_XL                  ((uint8)0x29)            /*!< Accelerometer X-axis high output register */
#define  OUT_Y_L_XL                  ((uint8)0x2A)            /*!< Accelerometer Y-axis low output register */
#define  OUT_Y_H_XL                  ((uint8)0x2B)            /*!< Accelerometer Y-axis high output register */
#define  OUT_Z_L_XL                  ((uint8)0x2C)            /*!< Accelerometer Z-axis low output register */
#define  OUT_Z_H_XL                  ((uint8)0x2D)            /*!< Accelerometer Z-axis high output register */
#define  FIFO_CTRL                   ((uint8)0x2E)            /*!< FIFO control register */
#define  FIFO_SRC                    ((uint8)0x2F)            /*!< FIFO status control register */
#define  INT_GEN_CFG_G               ((uint8)0x30)            /*!< Gyroscope interrupt generator configuration register */
#define  INT_GEN_THS_XH_G            ((uint8)0x31)            /*!< Gyroscope X-axis low interrupt generator threshold registers */
#define  INT_GEN_THS_XL_G            ((uint8)0x32)            /*!< Gyroscope X-axis high interrupt generator threshold registers  */
#define  INT_GEN_THS_YH_G            ((uint8)0x33)            /*!< Gyroscope Y-axis low interrupt generator threshold registers */
#define  INT_GEN_THS_YL_G            ((uint8)0x34)            /*!< Gyroscope Y-axis high interrupt generator threshold registers */
#define  INT_GEN_THS_ZH_G            ((uint8)0x35)            /*!< Gyroscope Z-axis low interrupt generator threshold registers */
#define  INT_GEN_THS_ZL_G            ((uint8)0x36)            /*!< Gyroscope Z-axis high interrupt generator threshold registers */
#define  INT_GEN_DUR_G               ((uint8)0x37)            /*!< Gyroscope interrupt generator duration register */

/*****************  Bit definition for Registers Addresses  *******************/

#define LSM6DS0_DEVICE_RESET_BIT		0
//CTRL_REG1_G
//Gyro fullscale selection
#define LSM6DS0_GYRO_FS_SEL_BIT			3
#define LSM6DS0_GYRO_FS_SEL_LENGTH		2

//Gyroscope output data rate selection
#define LSM6DS0_GYRO_ODR_SEL_BIT		0
#define LSM6DS0_GYRO_ODR_SEL_LENGTH		8

//Accelerometer fullscale selection
#define LSM6DS0_ACCEL_FS_SEL_BIT		3
#define LSM6DS0_ACCEL_FS_SEL_LENGTH		2

//Accelerometer output data rate selection
#define LSM6DS0_ACCEL_ODR_SEL_BIT		5
#define LSM6DS0_ACCEL_ODR_SEL_LENGTH	3

//Gyroscope low Power Mode
#define LSM6DS0_GYRO_LP_MODE_BIT		7
#define LSM6DS0_GYRO_LP_MODE_LENGTH		1

//Self Test
#define LSM6DS0_GYRO_ST_BIT				2
#define LSM6DS0_ACCEL_ST_BIT			0

/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @name    Generic LSM6DS0 data structures and types
 * @{
 */

/**
 * @brief  Accelerometer and Gyroscope Slave Address
 */
typedef enum {
  SAD_GND = 0x6A,                             /*!< LSM6DS0 Slave Address when SA1 is to GND */
  SAD_VCC = 0x6B                              /*!< LSM6DS0 Slave Address when SA1 is to VCC */
}ImuSlaveAddress;

/**
 * @brief  Accelerometer and Gyroscope Block Data Update
 */
typedef enum
{
  BDU_CONTINOUS = 0x00,                       /*!< Continuos Update */
  BDU_BLOCKED   = 0x40                        /*!< Single Update: output registers not updated until MSB and LSB reading */
}ImuBlockDataUpdate;

/**
 * @brief  Accelerometer and Gyroscope Endianness
 */
typedef enum
{
  END_LITTLE = 0x00,                          /*!< Little Endian: data LSB @ lower address */
  END_BIG    = 0x20                           /*!< Big Endian: data MSB @ lower address */
}ImuEndianness;
/** @}  */

/**
 * @name    Accelerometer data structures and types
 * @{
 */

/**
 * @brief  Accelerometer Decimation Mode
 */
typedef enum {
  ACC_DEC_DISABLED = 0x00,                    /*!< NO decimation */
  ACC_DEC_X2       = 0x40,                    /*!< Decimation update every 2 sample */
  ACC_DEC_X4       = 0x80,                    /*!< Decimation update every 4 sample */
  ACC_DEC_X8       = 0xC0                     /*!< Decimation update every 8 sample */
}AccelDecimationMode;

/**
 * @brief   Accelerometer Axes Enabling
 */
typedef enum{
  ACC_AE_DISABLED = 0x00,                     /*!< Axes all disabled */
  ACC_AE_X        = 0x08,                     /*!< Only X-axis enabled */
  ACC_AE_Y        = 0x10,                     /*!< Only Y-axis enabled */
  ACC_AE_XY       = 0x18,                     /*!< X & Y axes enabled */
  ACC_AE_Z        = 0x20,                     /*!< Only Z-axis enabled */
  ACC_AE_XZ       = 0x28,                     /*!< X & Z axes enabled  */
  ACC_AE_YZ       = 0x30,                     /*!< Y & Z axes enabled  */
  ACC_AE_XYZ      = 0x38                      /*!< All axes enabled */
}AccelAxesEnable;

/**
 * @brief  Accelerometer Output Data Rate
 */
typedef enum {
  ACC_ODR_PD    = 0x0,                       /*!< Power down */
  ACC_ODR_10Hz  = 0x1,                       /*!< Output Data Rate = 10 Hz */
  ACC_ODR_50Hz  = 0x2,                       /*!< Output Data Rate = 50 Hz */
  ACC_ODR_119Hz = 0x3,                       /*!< Output Data Rate = 119 Hz */
  ACC_ODR_238Hz = 0x4,                       /*!< Output Data Rate = 238 Hz */
  ACC_ODR_476Hz = 0x5,                       /*!< Output Data Rate = 476 Hz */
  ACC_ODR_952Hz = 0x6                        /*!< Output Data Rate = 952 Hz */
}AccelODR;

/**
 * @brief  Accelerometer Full Scale
 */
typedef enum {
  ACC_FS_2G  = 0x00,                          /*!< ±2 g m/s^2 */
  ACC_FS_4G  = 0x10,                          /*!< ±4 g m/s^2 */
  ACC_FS_8G  = 0x18,                          /*!< ±8 g m/s^2 */
  ACC_FS_16G = 0x08                           /*!< ±16 g m/s^2 */
}AccelFullscale;

/**
 * @brief  Accelerometer Antialiasing filter Bandwidth Selection
 */
typedef enum{
  ACC_BW_408Hz    = 0x00,                     /*!< AA filter bandwidth = 408 Hz  */
  ACC_BW_211Hz    = 0x01,                     /*!< AA filter bandwidth = 211 Hz */
  ACC_BW_105Hz    = 0x02,                     /*!< AA filter bandwidth = 105 Hz */
  ACC_BW_50Hz     = 0x03,                     /*!< AA filter bandwidth = 50 Hz */
  ACC_BW_ACCORDED = 0x04                     /*!< AA filter bandwidth chosen by ODR selection */
}AccelAntialiasing;

/**
 * @brief  Accelerometer High Resolution mode
 */
typedef enum
{
  ACC_HR_Disabled = 0x00,                     /*!< High resolution output mode disabled, FDS bypassed */
  ACC_HR_EN_9     = 0xC4,                     /*!< High resolution output mode enabled, LP cutoff = ODR/9, FDS enabled */
  ACC_HR_EN_50    = 0x84,                     /*!< High resolution output mode enabled, LP cutoff = ODR/50, FDS enabled */
  ACC_HR_EN_100   = 0xA4,                     /*!< High resolution output mode enabled, LP cutoff = ODR/100, FDS enabled */
  ACC_HR_EN_400   = 0xE4,                     /*!< High resolution output mode enabled, LP cutoff = ODR/400, FDS enabled */
}AccelHighResMode;

/**
 * @brief  HP filter for interrupt
 */
typedef enum
{
  ACC_HPIS1_BYPASSED = 0x00,                  /*!< High-pass filter bypassed */
  ACC_HPIS1_ENABLED  = 0x01                   /*!< High-pass filter enabled for accelerometer interrupt function on interrupt */
}HighPassFilter;

/**
 * @brief  Gyroscope Output Data Rate
 */
typedef enum {
  GYRO_ODR_PD             = 0x00,             /*!< Power down */
  GYRO_ODR_14_9Hz_CO_5Hz  = 0x20,             /*!< Output Data Rate = 14.9 Hz, CutOff = 5Hz */
  GYRO_ODR_59_5Hz_CO_16Hz = 0x40,             /*!< Output Data Rate = 59.5 Hz, CutOff = 16Hz */
  GYRO_ODR_119Hz_CO_14Hz  = 0x60,             /*!< Output Data Rate = 119 Hz, CutOff = 14Hz */
  GYRO_ODR_119Hz_CO_31Hz  = 0x61,             /*!< Output Data Rate = 119 Hz, CutOff = 31Hz */
  GYRO_ODR_238Hz_CO_14Hz  = 0x80,             /*!< Output Data Rate = 238 Hz, CutOff = 14Hz */
  GYRO_ODR_238Hz_CO_29Hz  = 0x81,             /*!< Output Data Rate = 328 Hz, CutOff = 29Hz */
  GYRO_ODR_238Hz_CO_63Hz  = 0x82,             /*!< Output Data Rate = 238 Hz, CutOff = 63Hz */
  GYRO_ODR_238Hz_CO_78Hz  = 0x83,             /*!< Output Data Rate = 476 Hz, CutOff = 78Hz */
  GYRO_ODR_476Hz_CO_21Hz  = 0xA0,             /*!< Output Data Rate = 476 Hz, CutOff = 21Hz */
  GYRO_ODR_476Hz_CO_28Hz  = 0xA1,             /*!< Output Data Rate = 238 Hz, CutOff = 28Hz */
  GYRO_ODR_476Hz_CO_57Hz  = 0xA2,             /*!< Output Data Rate = 476 Hz, CutOff = 57Hz */
  GYRO_ODR_476Hz_CO_100Hz = 0xA3,             /*!< Output Data Rate = 476 Hz, CutOff = 100Hz */
  GYRO_ODR_952Hz_CO_33Hz  = 0xC0,             /*!< Output Data Rate = 952 Hz, CutOff = 33Hz */
  GYRO_ODR_952Hz_CO_40Hz  = 0xC1,             /*!< Output Data Rate = 952 Hz, CutOff = 40Hz */
  GYRO_ODR_952Hz_CO_58Hz  = 0xC2,             /*!< Output Data Rate = 952 Hz, CutOff = 58Hz */
  GYRO_ODR_952Hz_CO_100Hz = 0xC3              /*!< Output Data Rate = 952 Hz, CutOff = 100Hz */
}GyroODR;

/**
 * @brief  Gyroscope Full Scale
 */
typedef enum {
  GYRO_FS_245DSP  = 0x00,                     /*!< ±245 degrees per second */
  GYRO_FS_500DSP  = 0x08,                     /*!< ±500 degrees per second */
  GYRO_FS_2000DSP = 0x18                      /*!< ±2000 degrees per second */
}GyroFullscale;

/**
 * @brief  Gyroscope Output Selection
 */
typedef enum {
  GYRO_OUT_SEL_BYPASS    = 0x00,              /*!< Output not filtered */
  GYRO_OUT_SEL_FILTERED  = 0x01,              /*!< Output filtered */
}GyroFilter;

/**
 * @brief  Gyroscope Interrupt Selection
 */
typedef enum {
  GYRO_INT_SEL_BYPASS    = 0x00,              /*!< Interrupt generator signal not filtered */
  GYRO_INT_SEL_FILTERED  = 0x08,              /*!< Interrupt generator signal filtered */
}GyroInterrupt;

/**
 * @brief  Gyroscope Low Power Mode
 */
typedef enum {
  GYRO_LP_MODE_HIGH_PERFORMANCE = 0x00,       /*!< High performance */
  GYRO_LP_MODE_LOW_POWER        = 0x80,       /*!< Low power */
}GyroLowPowerMode;

/**
 * @brief  Gyroscope High Pass Filter Cutoff Selection
 */
typedef enum {
  GYRO_HPCF_DISABLED = 0x00,                  /*!< HP filter disabled  */
  GYRO_HPCF_0        = 0x40,                  /*!< Config 0 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_1        = 0x41,                  /*!< Config 1 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_2        = 0x42,                  /*!< Config 2 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_3        = 0x43,                  /*!< Config 3 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_4        = 0x44,                  /*!< Config 4 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_5        = 0x45,                  /*!< Config 5 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_6        = 0x46,                  /*!< Config 6 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_7        = 0x47,                  /*!< Config 7 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_8        = 0x48,                  /*!< Config 8 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_9        = 0x49,                  /*!< Config 9 refer to table 48 of DOcID025604 Rev 3  */
  GYRO_HPCF_10       = 0x4A                   /*!< Config 10 refer to table 48 of DOcID025604 Rev 3  */
}GyroHighPassCutoff;

/**
 * @brief   Gyroscope Axes Enabling
 */
typedef enum{
  GYRO_AE_DISABLED = 0x00,                    /*!< Axes all disabled */
  GYRO_AE_X        = 0x08,                    /*!< Only X-axis enabled */
  GYRO_AE_Y        = 0x10,                    /*!< Only Y-axis enabled */
  GYRO_AE_XY       = 0x18,                    /*!< X & Y axes enabled */
  GYRO_AE_Z        = 0x20,                    /*!< Only Z-axis enabled */
  GYRO_AE_XZ       = 0x28,                    /*!< X & Z axes enabled  */
  GYRO_AE_YZ       = 0x30,                    /*!< Y & Z axes enabled  */
  GYRO_AE_XYZ      = 0x38                     /*!< All axes enabled */
}GyroAxesEnable;

/**
 * @brief  Gyroscope Decimation Mode
 */
typedef enum {
  GYRO_DEC_DISABLED = 0x00,                   /*!< NO decimation */
  GYRO_DEC_X2       = 0x40,                   /*!< Decimation update every 2 sample */
  GYRO_DEC_X4       = 0x80,                   /*!< Decimation update every 4 sample */
  GYRO_DEC_X8       = 0xC0                    /*!< Decimation update every 8 sample */
}GyroDecimationMode;

/**
 * @brief  Gyroscope Sleep Mode
 */
typedef enum {
  GYRO_SLP_DISABLED = 0x00,                   /*!< Gyroscope sleep mode disabled */
  GYRO_SLP_ENABLED  = 0x40                    /*!< Gyroscope sleep mode enabled */
}GyroSleepMode;

/*********************************************************************************/
/**
 * @brief   Gyroscope configuration structure.
 */
/*********************************************************************************/
PUBLIC int16 xAccelOffset;
PUBLIC int16 yAccelOffset;
PUBLIC int16 zAccelOffset;

PUBLIC int16 xGyroOffset;
PUBLIC int16 yGyroOffset;
PUBLIC int16 zGyroOffset;
PUBLIC int16 temperature;
PUBLIC float abias[3];
PUBLIC float gbias[3];

GyroFullscale gScale;
AccelFullscale aScale;

bool accelEnabled;
bool gyroEnabled;

// gRes, aRes, and mRes store the current resolution for each sensor.
// Units of these values would be DPS (or g's or Gs's) per ADC tick.
// This value is calculated as (sensor scale) / (2^15).
float gRes, aRes;

PUBLIC void initializeLSM6DS0(bool useAccel, bool useGyro);
PUBLIC void calibrateLSM6DS0(float *gbias, float *abias);
PUBLIC bool testConnection();
PUBLIC void initGyro();
PUBLIC void initAccel();
PUBLIC void setAccelODR(uint8 rate);
PUBLIC void setGyroODR(uint8 rate);
PUBLIC void setFullScaleGyroRange(uint8 range);
PUBLIC void setAccelSelfTest(bool enabled);
PUBLIC void setGyroSelfTest(bool enabled);
PUBLIC void setFullScaleAccelRange(uint8 range);
PUBLIC bool getAcceleration(int16* x, int16* y, int16* z);
PUBLIC int16 getAccelerationX();
PUBLIC int16 getAccelerationY();
PUBLIC int16 getAccelerationZ();
PUBLIC int16 getTemperature();
PUBLIC void getRotation(int16* x, int16* y, int16* z);
PUBLIC int16 getRotationX();
PUBLIC int16 getRotationY();
PUBLIC int16 getRotationZ();
PUBLIC void resetLSM6DS0();
PUBLIC uint8 getIMUDeviceID();
PUBLIC void calcGyroRes();
PUBLIC void calcAccelRes();

#endif /* _LSM6DS0_H_ */
